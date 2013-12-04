#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVision 1996.                                                     *
* Last Revision:                                                       *
*********************************************************************************
*/


/*********************************************************************************
 *                              TransportUDP.c
 *
 * This c file contains implementations for udp transport functions
 * ip address.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                 08/12/02
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"

#include "TransportUDP.h"
#include "TransportProcessingQueue.h"
#include "TransportMsgBuilder.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

#if defined(UPDATED_BY_SPIRENT_ABACUS)
#define SPIRENT_READ_UNBLOCKING
#define SPIRENT_READ_TIME_LIMIT (10)
#endif

#include "rvexternal.h"

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/******************************************************************************
 * TransportUDPOpen
 * ----------------------------------------------------------------------------
 * General: creates UDP socket, binds it to the specified address and
 *          starts to listen on it (register it to READ event).
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the transport info structure
 *          pLocalAddress - local address, socket for which should be opened.
 *                          It contains IP and port for binding
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPOpen(IN TransportMgr*          pTransportMgr,
                                     IN TransportMgrLocalAddr* pLocalAddress)
{
    RvStatus rv, crv;

    pLocalAddress->pSocket = NULL;
    pLocalAddress->ccSocket = RV_INVALID_SOCKET;

    /* Create socket */
    crv = RvSocketConstruct(RvAddressGetType(&(pLocalAddress->addr.addr)),
                            RvSocketProtocolUdp,
                            pTransportMgr->pLogMgr,
                            &pLocalAddress->ccSocket);
    if (RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPOpen (Local Address 0x%p): can't open UDP socket (crv=%d,rv=%d:%s)",
            pLocalAddress, crv, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Socket HW binding to Ethernet Interface
 */
#if defined(UPDATED_BY_SPIRENT)
    Spirent_RvSocketBindDevice(&pLocalAddress->ccSocket,pLocalAddress->if_name,pTransportMgr->pLogMgr);
#endif  //#if defined(UPDATED_BY_SPIRENT)
/* SPIRENT_END */

    /* Bind socket */
    crv = RvSocketBind(&pLocalAddress->ccSocket, &(pLocalAddress->addr.addr),
                       NULL /*portRange*/, pTransportMgr->pLogMgr);
    if (RV_OK != crv)
    {
        RvSocketDestruct(&pLocalAddress->ccSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
        pLocalAddress->ccSocket = RV_INVALID_SOCKET;
        rv = CRV2RV(crv);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPOpen (Local Address 0x%p): can't bind UDP socket %d (crv=%d,rv=%d:%s)",
            pLocalAddress, pLocalAddress->ccSocket, crv, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = TransportMgrSetSocketDefaultAttrb(
            &pLocalAddress->ccSocket, pTransportMgr, RvSocketProtocolUdp);
    if (RV_OK != rv)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPOpen (Local Address 0x%p): failed to set default attributes to socket %d (crv=%d,rv=%d:%s)",
            pLocalAddress, pLocalAddress->ccSocket, crv, rv, SipCommonStatus2Str(rv)));
    }
    pLocalAddress->pSocket = &pLocalAddress->ccSocket;

    /* Register socket to READ events */
    crv = RvFdConstruct(&pLocalAddress->ccSelectFd, pLocalAddress->pSocket,
                        pTransportMgr->pLogMgr);
    if(RV_OK != crv)
    {
        pLocalAddress->ccSelectFd.fd = RV_INVALID_SOCKET;
        rv = CRV2RV(crv);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPOpen (Local Address 0x%p): failed to create FD for socket %d (crv=%d,rv=%d:%s)",
            pLocalAddress, *pLocalAddress->pSocket, crv, rv, SipCommonStatus2Str(rv)));
        TransportUDPClose(pTransportMgr, pLocalAddress);
        return rv;
    }
    crv = RvSelectAdd(pTransportMgr->pSelect, &pLocalAddress->ccSelectFd,
                      RvSelectRead, TransportUdpEventCallback);
    if(RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPOpen (Local Address 0x%p): failed to register READ event for socket %d (crv=%d,rv=%d:%s)",
            pLocalAddress, *pLocalAddress->pSocket, crv, rv, SipCommonStatus2Str(rv)));
        TransportUDPClose(pTransportMgr, pLocalAddress);
        return rv;
    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportUDPOpen  (Local Address 0x%p, socket %d): Socket was opened successfully",
        pLocalAddress, *pLocalAddress->pSocket));

    return RV_OK;
}

/******************************************************************************
 * TransportUDPClose
 * ----------------------------------------------------------------------------
 * General: Closes UDP address - closes network socket, unregisters it from
 *          Select Engine, destructs FileDescriptor
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport manager
 *          pLocalAddr      - pointer to the address to be closed
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPClose(
                                    IN TransportMgr             *pTransportMgr,
                                    IN TransportMgrLocalAddr    *pLocalAddr)
{
    /* Close the socket */
    if (NULL != pLocalAddr->pSocket)
    {
        RvStatus rv;

        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPClose: Closing socket=%d", pLocalAddr->ccSocket));

        rv = TransportMgrSocketDestruct(pTransportMgr, pLocalAddr->pSocket,
                RV_FALSE/*bCleanSocket*/, &pLocalAddr->ccSelectFd);
        if (rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportUDPClose: Failed to destruct socket=0x%p", pLocalAddr->ccSocket));
            pLocalAddr->pSocket = NULL;
            return rv;
        }
        pLocalAddr->pSocket = NULL;
    }

    return RV_OK;
}

#if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES)
/******************************************************************************
 * TransportUDPReopen
 * ----------------------------------------------------------------------------
 * General: closes, opens and binds socket to the same IP:Port.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport manager
 *          pLocalAddr      - pointer to the address to be closed
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPReopen(
                            IN TransportMgr             *pTransportMgr,
                            IN TransportMgrLocalAddr    *pLocalAddr)
{
    RvStatus rv;
    RvSocket closedSocket = pLocalAddr->ccSocket;

    rv = TransportUDPClose(pTransportMgr, pLocalAddr);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPReopen: Failed to close socket=0x%p (rv=%d:%s)",
            pLocalAddr->ccSocket, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = TransportUDPOpen(pTransportMgr, pLocalAddr);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportUDPReopen: Failed to open socket=0x%p (rv=%d:%s)",
            closedSocket, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportUDPReopen (Local Address 0x%p): socket %d was replaced by socket %d",
        pLocalAddr, closedSocket, pLocalAddr->ccSocket));
    return RV_OK;
}
#endif /* #if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES) */

/************************************************************************************
 * TransportUdpEventCallback
 * ----------------------------------------------------------------------------------
 * General:  Callback function that is called from the LI package whenever we are getting event from there.
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * input   : selectEngine   - Events engine of this fd
 *           fd             - File descriptor that this event occured on
 *           selectEvent    - Event that happened
 *           error          - RV_TRUE if an error occured
 * Output:  RvStatus
 ***********************************************************************************/
void TransportUdpEventCallback(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error)
{
    RvSize_t                bytesRecv;
    SipTransportAddress     recvFromAddress;
    RvSocket*               pSock;
    RvStatus                crv = RV_OK;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#ifdef SPIRENT_READ_UNBLOCKING
    unsigned int           time_first;
    unsigned int           time_last;
#endif
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvChar                 buf[RVSIP_TRANSPORT_LEN_STRING_IP];
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
    RvStatus                retCode = RV_OK;

    TransportMgrLocalAddr*  pLocalAddr  = RV_GET_STRUCT(TransportMgrLocalAddr,ccSelectFd,fd);
    TransportMgr*           pMgr        =pLocalAddr->pMgr;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    buf[0] = '\0';
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    pSock = RvFdGetSocket(fd);

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "TransportUdpEventCallback - sock %d: notification of network events", (RvInt)(*pSock)));

    memset(&recvFromAddress, 0, sizeof(recvFromAddress)); 
    
    recvFromAddress.eTransport = RVSIP_TRANSPORT_UDP;

    switch (selectEvent)
    {
    case RvSelectRead:
        {
            TransportProcessingQueueEvent *ev             = NULL;
            RvUint8                       *recvBuffer     = NULL;
            RvStatus rv;

            /* Error checking */
            if (error == RV_TRUE)
            {
                RvLogExcep(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: Got RvSelectRead with error. Received message is aborted.",(RvInt)(*pSock)));
                break;
            }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#ifdef SPIRENT_READ_UNBLOCKING
            time_first=time_last=getMilliSeconds();
            while(1)
            {                                        
                if ( ul_after(time_last,(time_first + SPIRENT_READ_TIME_LIMIT)) ) 
                    break;
#else
	    while(1)
	    {
#endif  /* SPIRENT_READ_UNBLOCKING */
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

            retCode = TransportProcessingQueueAllocateEvent(
                            (RvSipTransportMgrHandle)pMgr,
                            NULL, MESSAGE_RCVD_EVENT ,RV_TRUE, &ev);

            if (retCode != RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: Failed to allocate resources for processing read event",(RvInt)(*pSock)));
                if (RV_ERROR_OUTOFRESOURCES == retCode)
                {
                    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "TransportUdpEventCallback sock %d: Stop Processing UDP events until OOR recovery",(RvInt)(*pSock)));
                    RvMutexLock(&pMgr->hRcvBufferAllocLock,pMgr->pLogMgr);
                    if (RV_FALSE == pMgr->notEnoughBuffersInPoolForUdp)
                    {
                        TransportMgrSelectUpdateForAllUdpAddrs(pMgr,(RvSelectEvents)0);
                        pMgr->notEnoughBuffersInPoolForUdp = RV_TRUE;
                    }
                    RvMutexUnlock(&pMgr->hRcvBufferAllocLock,pMgr->pLogMgr);
                }
                break;
            }
            recvBuffer = ev->typeSpecific.msgReceived.receivedBuffer;

            rv = TransportMgrLocalAddrLock(pLocalAddr);
            if (RV_OK != rv)
            {
                RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: Local address probably was closed",(RvInt)(*pSock)));
                TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pMgr,ev);
                break;
            }

            /* receiving message*/
            crv = RvSocketReceiveBuffer(pSock,recvBuffer,pMgr->maxBufSize,pMgr->pLogMgr,&bytesRecv,&recvFromAddress.addr);

            if(crv != RV_OK)
            {
                if (RV_ERROR_INSUFFICIENT_BUFFER == RvErrorGetCode(crv))
                {
                    RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "TransportUdpEventCallback - sock %d: Receive failure, due to insuffficient buffer",(RvInt)(*pSock)));
                }
                else
                {
                    RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "TransportUdpEventCallback - sock %d: Receive failure.",(RvInt)(*pSock)));
#if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES)
                    TransportUDPReopen(pMgr, pLocalAddr);
#endif /* #if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES) */
                }
                TransportMgrLocalAddrUnlock(pLocalAddr);
                TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pMgr,ev);
                break;
            }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            else
            {
               if(bytesRecv == 0)
               {
                    TransportMgrLocalAddrUnlock(pLocalAddr);
                    TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pMgr,ev);                    
                    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                          "TransportUdpEventCallback - sock %d: no more data left --> exit the callback.", (RvInt)(*pSock)));
                    break;
               }
            }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


            if (bytesRecv+1 > (RvSize_t)pMgr->maxBufSize)
            {
                TransportMgrLocalAddrUnlock(pLocalAddr);
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: The UDP buffer is not big enough.",(RvInt)(*pSock)));
                TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pMgr,ev);
                return;
            }
            recvBuffer[bytesRecv] = '\0';
            recvFromAddress.eTransport = RVSIP_TRANSPORT_UDP;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
            {
#if (RV_LOGMASK != RV_LOGLEVEL_DEBUG)
                RvChar localipbuff[RVSIP_TRANSPORT_LEN_STRING_IP];

                RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                           "TransportUdpEventCallback - sock %d: Recv new UDP message. %s:%d<-%s:%d, size=%d",
                           (RvInt)(*pSock),
                           RvAddressGetString(&pLocalAddr->addr.addr, RVSIP_TRANSPORT_LEN_STRING_IP, localipbuff),
                           RvAddressGetIpPort(&pLocalAddr->addr.addr),
                           RvAddressGetString(&recvFromAddress.addr, RVSIP_TRANSPORT_LEN_STRING_IP, buf),
                           RvAddressGetIpPort(&recvFromAddress.addr),
                           bytesRecv));
#endif
                /* printing the buffer */
                TransportMsgBuilderPrintMsg(pMgr,
                                            (RvChar *)recvBuffer,
                                            SIP_TRANSPORT_MSG_BUILDER_INCOMING,
                                            RV_FALSE);
            }
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

            TransportMgrLocalAddrUnlock(pLocalAddr);

            ev->typeSpecific.msgReceived.hLocalAddr     = (RvSipTransportLocalAddrHandle )pLocalAddr;
            ev->typeSpecific.msgReceived.bytesRecv      = (RvInt32)bytesRecv;
            ev->typeSpecific.msgReceived.rcvFromAddr    = recvFromAddress;
            ev->typeSpecific.msgReceived.eConnTransport = RVSIP_TRANSPORT_UDP;

            retCode = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pMgr,ev);
            if (retCode != RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: Failed to insert event into the processing queue", (RvInt)(*pSock)));
                TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pMgr,ev);
                break;
            }
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#ifdef SPIRENT_READ_UNBLOCKING
                time_last=getMilliSeconds();
             } /* while(1) */
#else
	    } /* while(1) */
#endif
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
            }
            break;

        case RvSelectWrite:
            {
                /* finished sending message */
                RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "TransportUdpEventCallback - sock %d: Got RvSelectWrite event from select module",(RvInt)(*pSock)));
            }

            break;

        case RvSelectAccept:
        case RvSelectConnect:
        case RvSelectClose:
        default:
            RvLogExcep(pMgr->pLogSrc,(pMgr->pLogSrc,
                "TransportUdpEventCallback - sock %d: Got unknown event %d", (RvInt)(*pSock),selectEvent));


    }
    RV_UNUSED_ARG(selectEngine);

    return;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"

int RvSipUdpPortIsAvailable(RvSipTransportMgrHandle            transportMgr,
                            RvSipTransportAddr *pLocalAddr, 
                            int port) {
  
  int ret=0;

  if(port==0) return 1;
  
  if(pLocalAddr && port>0) {
    
    RvStatus rv=0;
    
    TransportMgrLocalAddr   tmLocalAddr;
    TransportMgr*              pTransportMgr = (TransportMgr*)transportMgr;
    
    memset(&tmLocalAddr,0,sizeof(tmLocalAddr));
    
    rv = TransportMgrInitLocalAddressStructure(pTransportMgr,
					       pLocalAddr->eTransportType, pLocalAddr->eAddrType,
					       pLocalAddr->strIP, port, 
					       pLocalAddr->if_name,
					       pLocalAddr->vdevblock,
					       &tmLocalAddr);
    if(rv != RV_OK) {
      dprintf("%s - Failed to initialize strucuture for local %d address %s:%d (rv=%d)",
        __FUNCTION__,
        (int)(pLocalAddr->eTransportType),
        pLocalAddr->strIP,port,rv);
    } else {
      
      RvSocket    sock=RV_INVALID_SOCKET;
      RvStatus crv=0;
      RvInt addrType=RV_ADDRESS_TYPE_IPV4;
      
      if(pLocalAddr->eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) {
        addrType=RV_ADDRESS_TYPE_IPV6;
      }
      
      /* Create socket */
      crv = RvSocketConstruct(addrType,
        RvSocketProtocolUdp,
        pTransportMgr->pLogMgr,
        &sock);
      
      if (RV_OK != crv || sock==RV_INVALID_SOCKET)
      {
        rv = CRV2RV(crv);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
          "%s (Local Address %s): can't open UDP socket (crv=%d,rv=%d:%s)",
          __FUNCTION__,pLocalAddr->strIP, crv, rv, SipCommonStatus2Str(rv)));
        dprintf("%s (Local Address %s): can't open UDP socket (crv=%d,rv=%d:%s)",
          __FUNCTION__,pLocalAddr->strIP, crv, rv, SipCommonStatus2Str(rv));
        return 0;
      }

      Spirent_RvSocketBindDevice(&sock, pLocalAddr->if_name, pTransportMgr->pLogMgr);
      
      /* Bind socket */
      crv = RvSocketBind(&sock, &(tmLocalAddr.addr.addr),
        NULL /*portRange*/, pTransportMgr->pLogMgr);
      if (crv == RV_OK) {
        ret=1;
      }
      
      RvSocketDestruct(&sock, RV_FALSE, NULL, pTransportMgr->pLogMgr);
    }
  }

  return ret;
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif

