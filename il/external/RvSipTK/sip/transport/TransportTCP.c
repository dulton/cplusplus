
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
 *                              TransportTCP.c
 *
 * This c file contains implementations for the transport layer.
 * it contain functions for opening and closing sockets, functions for sending and
 * receiving messages and functions for identifying the destination host and it's
 * ip address.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Yaniv Saltoon                 05/08/01
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "_SipTransport.h"
#include "TransportTCP.h"
#include "TransportMsgBuilder.h"
#include "_SipTransport.h"
#include "TransportCallbacks.h"
#include "_SipCommonCore.h"

#if (RV_TLS_TYPE != RV_TLS_NONE)
#include "rvtls.h"
#include "TransportTLS.h"
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#include <sys/poll.h>
/*
 * COMMENTS: Socket HW binding to Ehernet Interface
 */
#include <net/if.h>
#include <errno.h>
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                              MACRO DEFINITIONS                        */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV TcpHandleConnectEvent(IN TransportConnection* pConn,
                                                   IN RvBool             error);

static RvStatus RVCALLCONV TcpHandleAcceptEvent(
                                       IN TransportConnection* pConn,
                                       IN RvSocket*            pSocket);

static RvStatus RVCALLCONV TcpHandleCloseEvent(
                                       IN TransportConnection   *pConn);

static RvStatus RVCALLCONV TCPHandleReadWriteEvents(IN TransportConnection*   pConn,
                                                     IN RvSelectEvents         event);
static RvBool TcpSaftyShutdownNoCloseTimerEvHandler(IN void    *pContext,
                                                    IN RvTimer *timerInfo);

static RvStatus TcpShutdownGracefully(IN TransportConnection *pConn);

/*-------------------------------------------------------------------------*/
/*                      FUNCTIONS IMPLEMENTATION                           */
/*-------------------------------------------------------------------------*/

/******************************************************************************
 * TransportTCPOpen
 * ----------------------------------------------------------------------------
 * General: opens connection to destination address
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransportMgr   - the pointer to the transport.
 *         pConn           - The pointer to the connection.
 *         bConnect        - If RV_FALSE, the socket will not be connected.
 *                           In this case TransportTCPConnect should be called.
 *         clientLocalPort - Port, to which the socket will be bound.
 *                           This parameter is actual for client connections.
 *                           Listening connections use port of their Local
 *                           Address objects.
 *****************************************************************************/
RvStatus RVCALLCONV TransportTCPOpen (IN TransportMgr*        pTransportMgr,
                                      IN TransportConnection* pConn,
                                      IN RvBool               bConnect,
                                      IN RvUint16             clientLocalPort)
{
    SipTransportAddress    localAddress;
    RvStatus               rv = RV_OK;
    RvStatus               crv = RV_OK;
    TransportMgrLocalAddr* pLocalAddr;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)					
    unsigned char          tos = 0;
    int                    value = 0;
    SipTransportAddress    destAddress;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    pLocalAddr = (TransportMgrLocalAddr*)(pConn->hLocalAddr);

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)					
    tos = pLocalAddr->tos;
    destAddress = pConn->destAddress;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    /* Open new socket from core */
    localAddress = ((TransportMgrLocalAddr*)(pConn->hLocalAddr))->addr;

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportTCPOpen - pConn 0x%p: Openning new TCP connection type=%s, bConnect=%d, clientLocalPort=%d",
               pConn,SipCommonConvertEnumToStrConnectionType(pConn->eConnectionType),
               bConnect, clientLocalPort))

    if (NULL != pConn->pSocket)
    {
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportTCPOpen - pConn 0x%p: socket is not -1",pConn));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return RV_ERROR_UNKNOWN;
    }

    /* Use provided port for client connections */
    if (pConn->eConnectionType==RVSIP_TRANSPORT_CONN_TYPE_CLIENT)
    {
        RvAddressSetIpPort(&localAddress.addr, clientLocalPort);
    }

/* SPIRENT_BEGIN */

#if defined(UPDATED_BY_SPIRENT)					

#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&destAddress.addr) &&
        (RvUint32)UNDEFINED == RvAddressGetIpv6Scope(&destAddress.addr))
    {
        RvAddressSetIpv6Scope(&destAddress.addr,RvAddressGetIpv6Scope(&localAddress.addr));
        // set traffic class for IPv6 according to TOS byte from configuration
        if (localAddress.addr.addrtype == RV_ADDRESS_TYPE_IPV6) {
            localAddress.addr.data.ipv6.flowinfo = ((unsigned long) tos) << 4;
        }
        if (destAddress.addr.addrtype == RV_ADDRESS_TYPE_IPV6) {
            destAddress.addr.data.ipv6.flowinfo = ((unsigned long) tos) << 4;
        }
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
#endif /* defined(UPDATED_BY_SPIRENT) */ 

/* SPIRENT_END */


    crv = RvSocketConstruct(RvAddressGetType(&localAddress.addr),
                            RvSocketProtocolTcp,
                            pTransportMgr->pLogMgr,
                            &pConn->ccSocket);
    if (RV_OK != crv)
    {
        TransportMgrLocalAddrUnlock(pLocalAddr);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportTCPOpen - pConn 0x%p: Failed to open socket for the new connection, crv=%d",
                   pConn,RvErrorGetCode(crv)))
        return RV_ERROR_UNKNOWN;
    }
    pConn->pSocket = &pConn->ccSocket;

    rv = TransportMgrSetSocketDefaultAttrb(pConn->pSocket,pTransportMgr,RvSocketProtocolTcp);
    if (RV_OK != rv)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTCPOpen - pConn 0x%p: Socket %d: Fail to set a default attribute to socket", pConn, pConn->ccSocket));
    }
    
	crv = RvSocketReuseAddr(pConn->pSocket,pTransportMgr->pLogMgr);
    if (RV_OK != crv)
    {
        TransportMgrLocalAddrUnlock(pLocalAddr);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
			"TransportTCPOpen - pConn 0x%p: Failed to set the reuse address option, crv=%d",pConn,RvErrorGetCode(crv)))
		RvSocketDestruct(pConn->pSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
        pConn->pSocket = NULL;
        return RV_ERROR_UNKNOWN;
		
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Socket HW binding to Ethernet Interface
 */
#if defined(UPDATED_BY_SPIRENT)
    Spirent_RvSocketBindDevice(pConn->pSocket,pLocalAddr->if_name,pTransportMgr->pLogMgr);
#endif  //#if defined(UPDATED_BY_SPIRENT)
/* SPIRENT_END */

    crv = RvSocketBind(pConn->pSocket,&localAddress.addr,NULL,pTransportMgr->pLogMgr);
    if (RV_OK != crv)
    {
        TransportMgrLocalAddrUnlock(pLocalAddr);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportTCPOpen - pConn 0x%p: Failed to bind socket for the new connection, crv=%d",pConn,RvErrorGetCode(crv)))
        RvSocketDestruct(pConn->pSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
        pConn->pSocket = NULL;
        return RV_ERROR_UNKNOWN;
    }

    if (RV_TRUE == TRANSPORT_CONNECTION_TOS_VALID(pConn->typeOfService))
    {
        rv = RvSocketSetTypeOfService(pConn->pSocket,pConn->typeOfService,pConn->pTransportMgr->pLogMgr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportTCPOpen - pConn 0x%p: Failed to set TypeOfService=%d for socket %d, rv=%d",
                pConn,pConn->typeOfService,pConn->ccSocket,RvErrorGetCode(rv)));
            rv = RV_OK;
        }
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (localAddress.addr.addrtype == RV_ADDRESS_TYPE_IPV6) {
        value = 1; // send flow information
        RvSocketSetSockOpt(pConn->pSocket, IPPROTO_IPV6, IPV6_FLOWINFO_SEND, (void *) &value, sizeof(value));
    }
#endif // RV_IPV6
#endif  //#if defined(UPDATED_BY_SPIRENT)
/* SPIRENT_END */


    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportTCPOpen - pConn 0x%p: Socket %d was opened for connection",pConn, pConn->ccSocket))

    if (pConn->eConnectionType == RVSIP_TRANSPORT_CONN_TYPE_CLIENT)
    {
        RvAddress ccLocalAddress;

        /* Get port, to which the socket was bound */
        crv = RvSocketGetLocalAddress(pConn->pSocket, pTransportMgr->pLogMgr,
                                      &ccLocalAddress);
        if (RV_OK != crv)
        {
            rv = CRV2RV(crv);
            TransportMgrLocalAddrUnlock(pLocalAddr);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportTCPOpen - pConn 0x%p: RvSocketGetLocalAddress failed, crv=%d, rv=%d:%s",
                pConn,RvErrorGetCode(crv),rv,SipCommonStatus2Str(rv)));
            RvSocketDestruct(pConn->pSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
            pConn->pSocket = NULL;
            return rv;
        }
        pConn->localPort = RvAddressGetIpPort(&ccLocalAddress);

        if (RV_TRUE == bConnect)
        {
            rv = TransportTCPConnect(pConn);
            if (RV_OK != rv)
            {
                /* There MUST'NT be CRV2RV(crv)!!! The redundant zeros the erroneous rv */
				/* and turns it to RV_OK */ 
                TransportMgrLocalAddrUnlock(pLocalAddr);
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportTCPOpen - pConn 0x%p: TransportTCPConnect failed, rv=%d:%s",
                    pConn,rv,SipCommonStatus2Str(rv)));
                RvSocketDestruct(pConn->pSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
                pConn->pSocket = NULL;
                return rv;
            }
        }

    }
    else
    {
        crv = RvSocketListen(pConn->pSocket,(RvUint32)pTransportMgr->maxConnections,pTransportMgr->pLogMgr);
        if (crv != RV_OK)
        {
            TransportMgrLocalAddrUnlock(pLocalAddr);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                      "TransportTCPOpen - pConn 0x%p: RvSocketListen failed for socket %d (crv=%d)",pConn,*pConn->pSocket,RvErrorGetCode(crv)));
            RvSocketDestruct(pConn->pSocket,RV_FALSE,NULL,pTransportMgr->pLogMgr);
            pConn->pSocket = NULL;
            return RV_ERROR_UNKNOWN;
        }
        /* for servers start listening on the socket */
        pConn->ccEvent = RvSelectAccept;
        rv = TransportConnectionRegisterSelectEvents(pConn,RV_FALSE,RV_TRUE);
        if (rv != RV_OK)
        {
            TransportMgrLocalAddrUnlock(pLocalAddr);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                      "TransportTCPOpen - pConn 0x%p: Failed to register on events for socket %d (crv=%d)",pConn,*pConn->pSocket,RvErrorGetCode(crv)));
            TransportMgrSocketDestruct(pTransportMgr,
                pConn->pSocket, RV_FALSE /*bCleanSocket*/,
                ((pConn->bFdAllocated==RV_TRUE)?&pConn->ccSelectFd:NULL));
            pConn->pSocket = NULL;
            return RV_ERROR_UNKNOWN;
        }

    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportTCPOpen - pConn 0x%p: Socket %d was opened successfully",pConn,pConn->ccSocket));
    TransportMgrLocalAddrUnlock(pLocalAddr);

    return rv;
}

/******************************************************************************
 * TransportTCPConnect
 * ----------------------------------------------------------------------------
 * General: connects socket of client connection to destination address
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pConn - The pointer to the connection.
 *****************************************************************************/
RvStatus RVCALLCONV TransportTCPConnect (IN TransportConnection* pConn)
{
    RvStatus            rv, crv;
    TransportMgr*       pTransportMgr = pConn->pTransportMgr;
    SipTransportAddress destAddress;

    /* Ensure the Connection is in correct state */
    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT != pConn->eConnectionType)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTCPConnect - pConn 0x%p: not CLIENT connection",pConn));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (NULL == pConn->pSocket)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTCPConnect - pConn 0x%p: connection is not bound to socket",pConn));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Prepare destination address to be used for connection */
    destAddress = pConn->destAddress;
    /* If destination address is of IPv6 type, but it has no scope ID,
       use the scope ID of the local address. */
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&pConn->destAddress.addr) &&
        (RvUint32)UNDEFINED == RvAddressGetIpv6Scope(&pConn->destAddress.addr))
    {
        RvAddress ccLocalAddress;
        memset(&ccLocalAddress,0,sizeof(RvAddress));
        crv = RvSocketGetLocalAddress(pConn->pSocket, pTransportMgr->pLogMgr,
                                      &ccLocalAddress);
        if (RV_OK != crv)
        {
            rv = CRV2RV(crv);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportTCPConnect - pConn 0x%p: RvSocketGetLocalAddress failed, crv=%d, rv=%d:%s",
                pConn,RvErrorGetCode(crv),rv,SipCommonStatus2Str(rv)));
            return rv;
        }
        RvAddressSetIpv6Scope(&destAddress.addr,RvAddressGetIpv6Scope(&ccLocalAddress));
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* Move the Connection to the CONNECTING state */
    rv = TransportConnectionChangeState(pConn,
            RVSIP_TRANSPORT_CONN_STATE_CONNECTING,
            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED, RV_TRUE, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTCPConnect- Connection 0x%p: failed to chage state to CONNECTING (rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        TransportConnectionClose(pConn, RV_TRUE /*bCloseTlsAsTcp*/);
        return rv;
    }

    /* Connect the Connection socket to the peer */
    /* nucleus and windows: first register then connect.
       Other OSs first connect and then register */
#if (RV_OS_TYPE != RV_OS_TYPE_WIN32) && (RV_OS_TYPE != RV_OS_TYPE_NUCLEUS)  && (RV_OS_TYPE != RV_OS_TYPE_SYMBIAN)
    crv = RvSocketConnect(pConn->pSocket,&destAddress.addr,pTransportMgr->pLogMgr);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		if (RV_OK == crv) {
			RvSocketSetLinger(pConn->pSocket, 0, pTransportMgr->pLogMgr);
		}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
    if (RV_OK == crv)
#endif
    {
        /* trigger the events and callbacks */
        pConn->ccEvent = RvSelectConnect;
        rv = TransportConnectionRegisterSelectEvents(pConn,RV_FALSE,RV_TRUE);
        if (rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportTCPConnect - pConn 0x%p: TransportConnectionRegisterSelectEvents failed for socket %d (rv=%d)",
                pConn,pConn->ccSocket,rv));
            return rv;
        }
    }
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32) || (RV_OS_TYPE == RV_OS_TYPE_NUCLEUS) || (RV_OS_TYPE == RV_OS_TYPE_SYMBIAN)
    crv = RvSocketConnect(pConn->pSocket,&destAddress.addr,pTransportMgr->pLogMgr);
#endif
    if (RV_OK != crv)
    {
        if (RvSelectConnect == pConn->ccEvent)
        {
            pConn->ccEvent = 0;
            TransportConnectionRegisterSelectEvents(pConn,RV_FALSE,RV_FALSE);
        }
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTCPConnect - pConn 0x%p: RvSocketConnect failed for socket %d (rv=%d)",
            pConn,pConn->ccSocket,RvErrorGetCode(crv)));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/************************************************************************************
 * TransportTCPSend
 * ----------------------------------------------------------------------------------
 * General: send the data over the connection. The data is kept inside list.
 *           each connection has its own list of outgoing messages and when the connection
 *           sends the message successfully it notifies its owner so that the owner
 *           can decide whether to free the page of the message or not.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTCPSend (TransportConnection  *pConn)
{
    RvStatus                status      = RV_OK;
    RvChar*                 buffer      = NULL;
    RvInt32                 size        = 0;
    RvSize_t                sent        = 0;
    SipTransportSendInfo    *msg        = NULL;
    SipTransportSendInfo    msgInfo;
    RLIST_ITEM_HANDLE       listElement = NULL;
    RvBool                  bSendMsg    = RV_TRUE;
    RvBool                  bDiscardMsg = RV_FALSE;
    RvStatus                crv         = RV_OK;

    if (pConn == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    if( (RV_TRUE == pConn->isDeleted) ||
#if (RV_TLS_TYPE != RV_TLS_NONE) /* on TLS it is not sufficient that the connection is connected,
                                     we also want to know that handshake is completed */
       (pConn->eTransport == RVSIP_TRANSPORT_TLS && pConn->eTlsState != RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED) ||
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
       (pConn->eTransport == RVSIP_TRANSPORT_TCP && pConn->eState != RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED))
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTCPSend - pConn 0x%p: Can not send state: (%s/%s)",pConn,
                    SipCommonConvertEnumToStrConnectionState(pConn->eState),
                    TransportConnectionGetTlsStateName(pConn->eTlsState)));
        /* Connection is or will be deleted or disconnected
        Error should be returned */
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RLIST_GetHead(pConn->pTransportMgr->hMsgToSendPoolList,pConn->hMsgToSendList,&listElement);

    while (listElement != NULL) 
    {
        /*move all over the list and send everything*/
        msg = (SipTransportSendInfo*)listElement;

		if(msg == NULL)
		{
            RvLogExcep(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
				"TransportTCPSend - pConn: 0x%p msg==NULL, Invalid Parameter",pConn));
            return RV_ERROR_BADPARAM;
		}

        RvMutexLock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);

        if ((RvUint32)msg->msgSize+1 > pConn->pTransportMgr->maxBufSize)
        { /* +1 stands for '\0', used for message logging */
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                       "TransportTCPSend - pConn 0x%p: The transport's buffer (%d bytes) is shorter than the message to send (%d bytes)",
                       pConn, pConn->pTransportMgr->maxBufSize-1, msg->msgSize));
            RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }

        /* copying the whole message into the send buffer */
        status = RPOOL_CopyToExternal(pConn->pTransportMgr->hMsgPool,
                                        msg->msgToSend,
                                        0,
                                        (void*)(pConn->pTransportMgr->sendBuffer),
                                        msg->msgSize);
        if (status != RV_OK)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                       "TransportTCPSend - pConn: 0x%p Can not copy buffer",pConn));

            RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
            return status;
        }

        /* Find the part of buffer, that was not sent yet */
        buffer = (RvChar*)pConn->pTransportMgr->sendBuffer;
        buffer += msg->currPos;
        size = msg->msgSize - msg->currPos;

        /* printing the buffer */
        buffer[msg->msgSize] = '\0';
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        {
            RvChar      localIP[RVSIP_TRANSPORT_LEN_STRING_IP];
            RvUint16    localPort;
            RvChar      ipremote[RVSIP_TRANSPORT_LEN_STRING_IP];
            RvStatus    rv;

            rv = TransportMgrLocalAddressGetIPandPortStrings(pConn->pTransportMgr,
                &((TransportMgrLocalAddr*)pConn->hLocalAddr)->addr.addr,
                RV_FALSE, RV_TRUE, localIP, &localPort);
            if (RV_OK != rv)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTCPSend - pConn 0x%p: Failed to get IP and Port strings (rv=%d)",pConn,rv));
            }

            RvLogDebug(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
                       "TransportTCPSend - pConn 0x%p: %s message 0x%p Sent, %s:%d->%s:%d, size=%d",
                       pConn,
                       SipCommonConvertEnumToStrTransportType(pConn->eTransport),
                       (msg!=NULL) ? msg->msgToSend:NULL,
                       localIP,
                       localPort,
                       RvAddressGetString(&pConn->destAddress.addr,RVSIP_TRANSPORT_LEN_STRING_IP,ipremote),
                       RvAddressGetIpPort(&pConn->destAddress.addr),
                       size));
        }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

        TransportMsgBuilderPrintMsg(pConn->pTransportMgr,
                                    (RvChar*)pConn->pTransportMgr->sendBuffer,
                                    SIP_TRANSPORT_MSG_BUILDER_OUTGOING,
                                    RV_FALSE);
        bSendMsg = RV_TRUE;
        /* Get permition from the application to send the message */
        bSendMsg = TransportCallbackMsgToSendEv(
                            pConn->pTransportMgr,
                            buffer, size);
        TransportCallbackBufferToSend(
                pConn->pTransportMgr,
                (TransportMgrLocalAddr*)pConn->hLocalAddr,
                &pConn->destAddress, pConn, buffer, size, &bDiscardMsg);
        if (RV_TRUE == bSendMsg && RV_FALSE == bDiscardMsg)
        {
            switch (pConn->eTransport)
            {
            case RVSIP_TRANSPORT_TCP:
                /* Do the actual send of buffer */
                crv = RvSocketSendBuffer(pConn->pSocket,
                                   (RvUint8*)buffer,
                                   size,
                                   NULL,
                                   pConn->pTransportMgr->pLogMgr,
                                   &sent);
                break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
            case RVSIP_TRANSPORT_TLS:
                crv = RvTLSSessionSendBufferEx(pConn->pTlsSession,
                                               buffer,
                                               size,
                                               &pConn->pTripleLock->hLock,
                                               pConn->pTransportMgr->pLogMgr,
                                               &pConn->eTlsEvents,
                                               &sent);
                if (RV_TLS_ERROR_WILL_BLOCK == RvErrorGetCode(crv))
                {
                    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                              "TransportTCPSend - pConn 0x%p: message sending is TLS blocked.", pConn));
                    /* If message sending was blocked, it should be resent
                    again. Otherwise OpenSSL will return error. In order
                    to prevent removal of message before retrial to send it
                    made as a result of message owner termination, mark
                    the message DONT_REMOVE flag. */
                    msg->bDontRemove = RV_TRUE;
                    /* On BLOCK we have to register select events,needed for
                    completion of the blocked operation. RvTLSTranslateTLSEvents()
                    called from TransportTlsCallOnThread() will tell us
                    which events are needed. */
                    TransportTlsCallOnThread(pConn,RV_TRUE/*bAddClose*/);
                    RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
                    return RV_OK;
                }
                else if (RV_TLS_ERROR_SHUTDOWN == RvErrorGetCode(crv))
                {
                    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                              "TransportTCPSend - pConn 0x%p: Got shutdown event. starting shutdown procedure.", pConn));
                    TransportTlsClose(pConn);
                    sent = (RvSize_t)UNDEFINED;
                }
                else if (RV_OK != crv)
                {
                    sent = (RvSize_t)UNDEFINED;
                }
                else
                {/* write() succeeded */
                    /* Register TLS_READ event - listen for incoming messages*/
                    pConn->eTlsEvents |= RV_TLS_READ_EV;
                    TransportTlsCallOnThread(pConn, RV_TRUE);
                }
                break;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
            default:
                break;
            }

            /*check error on send*/
            if ((RvInt32)sent < 0 || crv != RV_OK)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                          "TransportTCPSend - pConn 0x%p: failure in sending message.", pConn));
                /*should announce transaction for error*/
                RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
                /*page is freed already - when the transaction destructed*/
                listElement = NULL;
#ifdef RV_SIP_IMS_ON
                if (NULL != pConn->pSecOwner)
                {
                    TransportCallbackMsgSendFailureEv(pConn->pTransportMgr,
                        msg->msgToSend, NULL/*pLocalAddr*/, pConn,
                        pConn->pSecOwner);
                }
#endif /*#ifdef RV_SIP_IMS_ON*/
                return RV_ERROR_UNKNOWN;
            }
        }
        else
        {
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                      "TransportTCPSend - pConn 0x%p: message was not sent due to application request.", pConn));
        }


        /* update the buffer pointer */
        msg->currPos += (RvInt32)sent;

        /* Free resources, consumed by message, if the buffer was successfully
        sent, or it was discarded in according to the application request */
        if (msg->currPos == msg->msgSize || RV_FALSE == bSendMsg || RV_TRUE == bDiscardMsg)
        {

            RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
            /*remove from list*/
            if(msg->bFreeMsg == RV_TRUE && msg->msgToSend != NULL_PAGE)
            {
                RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,msg->msgToSend);
                msg->msgToSend = NULL_PAGE;
            }
            msgInfo = *msg; /*copy before removing*/
            RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
            RLIST_Remove(pConn->pTransportMgr->hMsgToSendPoolList,pConn->hMsgToSendList,listElement);
            RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
            listElement = NULL;

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting outgoing TCP messages 
*/
#if defined(UPDATED_BY_SPIRENT)
            if ( msg->msgSize == 0 && RV_TRUE == bSendMsg && RV_FALSE == bDiscardMsg )
            {
                RvSipRawMessageCounterHandlersRun(SPIRENT_SIP_RAW_MESSAGE_OUTGOING);
            }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                      "TransportTCPSend - pConn 0x%p: Message sent successfully. size send=%d", pConn,sent));

            /*should announce owner for successful sending*/
            
            if(msgInfo.hOwner != NULL)
            {
                 TransportConnectionNotifyStatus(pConn,msgInfo.hOwner,&msgInfo,
                                            RVSIP_TRANSPORT_CONN_STATUS_MSG_SENT,RV_FALSE);
            }

            /*go to check is there any other message to send*/
        }
        else
        {
            /* just wait since not all the buffer was not sent yet*/
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                       "TransportTCPSend - pConn 0x%p: sent only %d bytes ",pConn, sent));

            /* if only part was sent remember the buffer */
            /* trigger the write event for the next chunk of data */
            switch (pConn->eTransport)
            {
            case RVSIP_TRANSPORT_TCP:
                pConn->ccEvent |= RvSelectWrite;
                TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);
                break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
            case RVSIP_TRANSPORT_TLS:
                pConn->eTlsEvents |= RV_TLS_WRITE_EV;
                TransportTlsCallOnThread(pConn, RV_TRUE/*bAddClose*/);
                break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
            default:
                break;
            }

            RvMutexUnlock(&pConn->pTransportMgr->udpBufferLock,pConn->pTransportMgr->pLogMgr);
            return RV_OK;
        }
        RLIST_GetHead(pConn->pTransportMgr->hMsgToSendPoolList,pConn->hMsgToSendList,&listElement);
    }
    return RV_OK;
}

/************************************************************************************
 * TransportTCPClose
 * ----------------------------------------------------------------------------------
 * General: Close the connection.
 *          if the status of the connection is closing then close the connection 
 *          immediatelly.
 *          otherwise, call shutdown to tell the other side to close also, and wait for
 *          closed event to occur on the socket and just then, close the connection
 *          and remove from the list.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTCPClose (IN TransportConnection *pConn)
{
    RvStatus        rv = RV_OK;

    if (pConn == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    if(pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TERMINATED)
    {
        return RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
               "TransportTCPClose - connection 0x%p: (socket:%d) request to close connection at state %s",
               pConn,pConn->ccSocket,
               SipCommonConvertEnumToStrConnectionState(pConn->eState)));

    /* we call shut down on connected sockets. otherwise we close immediately*/
    if ((RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED == pConn->eState) &&
        (pConn->pSocket != NULL))
    {
       RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "TransportTCPClose - connection 0x%p: Shutting down ", pConn));
       
       /* make sure we know that we are the ones that asked for a connection closure*/
       pConn->bClosedByLocal = RV_TRUE;
       
       /*don't allow new owners*/
       TransportConnectionHashRemove(pConn);

       rv = TransportConnectionChangeState(pConn,
                                      RVSIP_TRANSPORT_CONN_STATE_CLOSING,
                                      RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                                      RV_TRUE,RV_TRUE);

	   if (rv != RV_OK  &&  pConn->eState!=RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED)
	   {
		   RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "TransportTCPClose - connection 0x%p: was unable to handle state change event.", pConn));
		   return rv;

	   }
       
       /* If the Shutdown failed the socket is destructed inside the following function */
       TcpShutdownGracefully(pConn);
#define TCP_SHUTDOWN_NO_CLOSE_TIMER 5000
       /* 2.2.3  - setting a shut down timer. if close event does not arrive after 5 seconds*/
       if (!SipCommonCoreRvTimerStarted(&pConn->saftyTimer))
       {
           SipCommonCoreRvTimerStartEx(
               &pConn->saftyTimer,
               pConn->pTransportMgr->pSelect,
               TCP_SHUTDOWN_NO_CLOSE_TIMER,
               TcpSaftyShutdownNoCloseTimerEvHandler,
               pConn);
       }
    }
    else /*close immediately*/
    {
        if(pConn->isDeleted != RV_TRUE)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTCPClose - connection 0x%p: (socket:%d) will be marked as 'deleted', immediate close request", pConn,pConn->ccSocket));

            if(pConn->pSocket != NULL)
            {
                rv = TransportMgrSocketDestruct(pConn->pTransportMgr,
                        pConn->pSocket, RV_TRUE/*bCleanSocket*/,
                        ((pConn->bFdAllocated==RV_TRUE)?&pConn->ccSelectFd:NULL));
                pConn->pSocket      = NULL;
                pConn->bFdAllocated = RV_FALSE;
                pConn->ccEvent      = (RvSelectEvents)0;
                if (rv != RV_OK)
                {
                    RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                        "TransportTCPClose - connection 0x%p: (socket:%d) TransportMgrSocketDestruct failed(rv=%d:%s)",
                        pConn,pConn->ccSocket,rv,SipCommonStatus2Str(rv)));
                }
            }
            pConn->isDeleted = RV_TRUE;
        }
    }


    if (pConn->isDeleted == RV_TRUE)
    {
        if (pConn->usageCounter == 0)
        {
            /*add connection to the termination queue*/
            TransportConnectionTerminate(pConn,RVSIP_TRANSPORT_CONN_REASON_UNDEFINED);
        }
    }
    
    return rv;
}

/******************************************************************************
 * TransportTcpWriteQueueEvent
 * ----------------------------------------------------------------------------
 * General:      Treats the WRITE event, raised by Select Engine for the
 *               connection. If there are elements in the Connection list of
 *               outgoing messages, the messages, kept in the elements will be
 *               sent.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the Connection object.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV TransportTcpWriteQueueEvent (
                                        IN TransportConnection* pConn)
{
    RvStatus status;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTcpWriteQueueEvent - connection 0x%p, socket %d Set pConn->bWriteEventInQueue to RV_FALSE",pConn,pConn->ccSocket));
    pConn->bWriteEventInQueue = RV_FALSE;

    if (pConn->pSocket == NULL)
    {
        RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTcpWriteQueueEvent - connection 0x%p : Cannot handle %s write queue event, socket fd is illegal",
            pConn, SipCommonConvertEnumToStrTransportType(pConn->eTransport)));
        return RV_ERROR_UNKNOWN;
    }

    /* lets see, is the connection still good for sending?,
       if we closed it, it surly can not send */
    if (RV_TRUE == pConn->bClosedByLocal)
    {
        RvBool bConnectionClosing = RV_FALSE;

        if (RVSIP_TRANSPORT_CONN_STATE_CLOSING == pConn->eState ||
            RVSIP_TRANSPORT_CONN_STATE_CLOSED  == pConn->eState)
        {
            bConnectionClosing = RV_TRUE;
        }
#if (RV_TLS_TYPE != RV_TLS_NONE)
        if (RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED == pConn->eTlsState)
        {
            bConnectionClosing = RV_TRUE;
        }
#endif/*(RV_TLS_TYPE != RV_TLS_NONE)*/
        if (RV_TRUE == bConnectionClosing)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTcpWriteQueueEvent - connection 0x%p : the connection is closing, will not send. tcp state=%s, tls state=%s",
                pConn,
                SipCommonConvertEnumToStrConnectionState(pConn->eState),
                TransportConnectionGetTlsStateName(pConn->eTlsState)));
            return RV_OK;
        }
    }

    status = TransportTCPSend(pConn);
    if (status != RV_OK)
    {
        /*notify all transaction on Error*/
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                   "TransportTcpWriteQueueEvent - connection 0x%p (sock %d): Failed to send %s msg",
                   pConn,pConn->ccSocket,SipCommonConvertEnumToStrTransportType(pConn->eTransport)));

        TransportConnectionNotifyStatus(pConn,NULL,NULL,
                                        RVSIP_TRANSPORT_CONN_STATUS_ERROR,RV_FALSE);
        status = TransportConnectionDisconnect(pConn);
    }
    return status;
}

/******************************************************************************
 * TransportTcpDisconnectQueueEvent
 * ----------------------------------------------------------------------------
 * General:      Treats the DISCONNECT event, picked up from the Transport
 *               Processing Queue (TPQ):initiates the Connection object freeing
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the connection event.
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportTcpDisconnectQueueEvent(
                                    IN TransportConnection* pConn)

{
    RvStatus status = RV_OK;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportTcpDisconnectQueueEvent - connection 0x%p, socket %d received disconnect event (setting bCloseEventInQueue to RV_FALSE)",pConn,pConn->ccSocket));
    pConn->bCloseEventInQueue = RV_FALSE;

    if ((pConn->usageCounter == 1) && (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CLOSING))
    {
        if (pConn->pSocket != NULL)
        {
            status = TransportMgrSocketDestruct(pConn->pTransportMgr,
                        pConn->pSocket, RV_FALSE/*bCleanSocket*/,
                        ((pConn->bFdAllocated==RV_TRUE)?&pConn->ccSelectFd:NULL));
            if (status != RV_OK)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTcpDisconnectQueueEvent - Failed to destruct socket of Connection 0x%p ",pConn));
                
            }
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTcpDisconnectQueueEvent - conn 0x%p socket %d removed ",pConn,pConn->ccSocket));
            pConn->pSocket      = NULL;
            pConn->bFdAllocated = RV_FALSE;
            pConn->ccEvent      = (RvSelectEvents )0;
        }
        TransportConnectionChangeState(pConn,RVSIP_TRANSPORT_CONN_STATE_CLOSED,
                                       RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,RV_FALSE,RV_TRUE);

        /*remove the connection from the connection list*/
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTcpDisconnectQueueEvent - removing connection 0x%p from list",pConn));

        TransportConnectionTerminate(pConn,RVSIP_TRANSPORT_CONN_REASON_UNDEFINED);

    }
    else
    {
        /*change the status. In this case, when the connection is closed from the remote
         side we can immediately close the connection and remove from the list */
        TransportConnectionHashRemove(pConn);
#if (RV_TLS_TYPE != RV_TLS_NONE)
        /* It is possible that TLS handshake was completed, but TLS connected state was not assumed.
           In that case we need to notify the application that it no longer needs the application
           object, if there was one */
        if (RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED == pConn->eTlsState ||
            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY     == pConn->eTlsState)
        {
            TransportCallbackTLSStateChange(pConn,
                                    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED,
                                    RVSIP_TRANSPORT_CONN_REASON_DISCONNECTED,
                                    RV_FALSE, RV_TRUE);
        }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

        TransportConnectionChangeState(pConn,RVSIP_TRANSPORT_CONN_STATE_CLOSED,
                                       RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,RV_FALSE,RV_TRUE);

        TransportConnectionDecUsageCounter(pConn);
        status = TransportTCPClose(pConn);

        if (status != RV_OK)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                      "TransportTcpDisconnectQueueEvent - connection 0x%p couldn't be destruct ",pConn));

        }
    }
    return status;
}

/******************************************************************************
 * TransportTcpConnectedQueueEvent
 * ----------------------------------------------------------------------------
 * General:      Treats the CONNECTED event, raised by Select Engine: moves
 *               the Connection object to the CONNECTED state.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the connection object.
 *          error - RV_TRUE, if Select Engine handled some error on connection
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportTcpConnectedQueueEvent(
                                IN TransportConnection* pConn,
                                IN RvBool               error)
{
	RvStatus rv; 

    if (error)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTcpConnectedQueueEvent - conn: 0x%p (socket = %d), Connect event Error , Notifying owners (error = %d)",
            pConn, pConn->ccSocket, error));
        /*notify owners about the connection error*/
        TransportConnectionNotifyStatus(pConn,NULL,NULL,
                                        RVSIP_TRANSPORT_CONN_STATUS_ERROR,RV_FALSE);
        TransportConnectionDisconnect(pConn);
        return RV_OK;
    }

    rv = TransportConnectionChangeState(pConn,
            RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED,
            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED, RV_FALSE, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTcpConnectedQueueEvent - Connection 0x%p: failed to chage state to CONNECTED (rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        TransportConnectionDisconnect(pConn);
        return rv;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* letting the application know of the new TLS sequence */
    if (RVSIP_TRANSPORT_TLS == pConn->eTransport)
    {
        /* stop listening on events other than close until application tells us to handshake */
        pConn->ccEvent = (RvSelectEvents)0;
        TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);

        /* letting the application know of the new TLS sequence
           and exchange handles*/
        TransportCallbackTLSSeqStarted(pConn);

        if (NULL != pConn->pTransportMgr->appEvHandlers.pfnEvTlsStateChanged)
        {
            /* Telling the application we are ready for handshake*/
            rv = TransportCallbackTLSStateChange(pConn,
                            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY,
                            RVSIP_TRANSPORT_CONN_REASON_CLIENT_CONNECTED,
                            RV_FALSE, RV_TRUE);
            if (RV_OK != rv)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                    "TransportTcpConnectedQueueEvent - Connection 0x%p: failed to report HANDSHAKE_READY state(rv=%d:%s). Close the connection",
                    pConn, rv, SipCommonStatus2Str(rv)));
                TransportTlsClose(pConn);
                return rv;
            }
            return RV_OK;
        }
        else
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTcpConnectedQueueEvent - Connection 0x%p can not report Tls state change. Terminating connection",pConn));
            TransportConnectionNotifyStatus(
                           pConn,
                           NULL,
                           NULL,
                           RVSIP_TRANSPORT_CONN_STATUS_ERROR,
                           RV_FALSE);
            TransportTlsClose(pConn);
            return RV_OK;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    /*after the connection made , we can send the data*/
    pConn->ccEvent = RvSelectRead | RvSelectWrite;
    rv = TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);
	if (rv != RV_OK)
	{
		RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
			"TransportTcpConnectedQueueEvent - conn: 0x%p, failed to register read/write events",pConn));
		return rv;
	}

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTcpConnectedQueueEvent - conn: 0x%p, Waiting for write event",pConn));
    return RV_OK;
}

/************************************************************************************
 * TransportTcpHandleWriteEvent
 * ----------------------------------------------------------------------------------
 * General: Handles write event for TCP
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTcpConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTcpHandleWriteEvent(IN TransportConnection   *pConn)
{
    RvStatus rv = RV_OK;
    void*    ev = NULL;

    /* If previous WRITE_EVENT was not peeked out from TPQ -> no write was
       made. Therefore just ignore the current write event, raised by select.
       In addition - remove write event from requested events. */
    if (RV_TRUE == pConn->bWriteEventInQueue)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "TransportTcpHandleWriteEvent - connection 0x%p: pConn->bWriteEventInQueue is RV_TRUE. The event is not inserted into the queue",pConn));

        /* removing the write event to prevent an endless loop */
        pConn->ccEvent = RvSelectRead;
        rv = TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);
        return rv;
    }

    rv = TransportConnectionAddWriteEventToTPQ(pConn, &ev);

    /* If TransportConnectionAddWriteEventToTPQ() succeeded, the WRITE was
       added successfully to the TPQ. Remove registration for write event until
       write() will be performed. The last occurs on peek WRITE from TPQ. */
    if (RV_OK == rv)
    {
        pConn->ccEvent = RvSelectRead;
        rv = TransportConnectionRegisterSelectEvents(
                pConn, RV_TRUE /*bAddClose*/, RV_FALSE /*bFirstCall*/);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTcpHandleWriteEvent - connection 0x%p: failed to register select events(rv=%d:%s)",
                pConn, rv, SipCommonStatus2Str(rv)));
            TransportProcessingQueueFreeEvent(
                (RvSipTransportMgrHandle)pConn->pTransportMgr,
                (TransportProcessingQueueEvent*)ev);
            return rv;
        }
    }

    return RV_OK;
}

/************************************************************************************
 * TransportTcpEventCallback
 * ----------------------------------------------------------------------------------
 * General: Treats the events that may occur on a connection (Accept, Connect, Write, Read,
 *          close).
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   selectEngine - select engine which we registered on
 *          fd      - file descriptor corresponding with registered socket
 *          event   - The type of the event
 *          error   - indicates whether an error occurred in the low level
 * Output:  (-)
 ***********************************************************************************/
void TransportTcpEventCallback(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool          error)
{

    RvStatus                        status          = RV_OK;
	RvStatus						crv				= RV_OK;
    TransportConnection*            pConn           = RV_GET_STRUCT(TransportConnection,ccSelectFd,fd);
    TransportMgr*                   pTransportMgr   = NULL;
    RvSocket                        *pSocket        = NULL;

    RV_UNUSED_ARG(selectEngine);

    pTransportMgr = pConn->pTransportMgr;
    pSocket       = RvFdGetSocket(fd);
    status        = TransportConnectionLockEvent(pConn);
    if (status != RV_OK)
    {
        return;
    }

    if (*pSocket != pConn->ccSocket)
    {
        TransportConnectionUnLockEvent(pConn);
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTcpEventCallback - connection 0x%p (sock:%d): does not match socket from callback %d", pConn, pConn->ccSocket,*pSocket));
        return;
    }
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportTcpEventCallback - connection 0x%p (sock:%d): Event callback with event: %d, err=%d", pConn, pConn->ccSocket,selectEvent,error));

    if (pConn->pSocket ==  NULL)
    {
        TransportConnectionUnLockEvent(pConn);
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportTcpEventCallback - connection 0x%p (sock:%d): Cannot handle event: %d , socket fd is illegal", pConn, pConn->ccSocket,selectEvent));
        return;
    }

    /* Treat the events on the given socket */
    if (selectEvent & RvSelectAccept)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        do {
#endif
/* SPIRENT_END */
        status = TcpHandleAcceptEvent(pConn,pSocket);
        if (status != RV_OK)
        {
        /* Out of resources return code
            should be prevented when processing accept */
            status = RV_ERROR_UNKNOWN;
        }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            if ( status != RV_OK ) break;
			/* check if there is more to accept */
            {
               int retval=0;
               struct pollfd poll_fd;
               poll_fd.fd      = *pSocket;
               poll_fd.events  = POLLIN;
               poll_fd.revents = 0;
               
               do {
                  retval = poll ( &poll_fd, 1, 0 );
               } while(retval==-1 && errno==EINTR);
               
               if ( !(poll_fd.revents & POLLIN) ) break;
            }
        } while (1);
#endif
/* SPIRENT_END */
    }
    else if (selectEvent & RvSelectConnect)
    {
        status = TransportConnectionIncUsageCounter(pConn);
        if (status == RV_OK)
        {
            status = TcpHandleConnectEvent(pConn,error);
            if (status != RV_OK)
            {
                TransportConnectionDecUsageCounter(pConn);
            }
        }
    }
    else if (selectEvent & RvSelectClose)
    {
        status = TcpHandleCloseEvent(pConn);
    }
    else if (selectEvent & RvSelectWrite && error == RV_TRUE)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTcpEventCallback - connection 0x%p (sock:%d): write event with error, closing socket", pConn, pConn->ccSocket));
#if (RV_TLS_TYPE != RV_TLS_NONE)
        if (RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED != pConn->eTlsState)
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
        {
            TransportConnectionNotifyStatus(pConn,NULL,NULL,
                RVSIP_TRANSPORT_CONN_STATUS_ERROR,RV_TRUE);
        }
        status = TcpHandleCloseEvent(pConn);
    }
    /* for read and write events we need to check if the event is TCP or TLS
    since they will be handled differently */
    else if (selectEvent & RvSelectWrite || selectEvent & RvSelectRead)
    {
        switch (pConn->eTransport)
        {
        case RVSIP_TRANSPORT_TCP:
            status = TCPHandleReadWriteEvents(pConn,selectEvent);
            break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            status = TransportTLSHandleReadWriteEvents(pConn,selectEvent);
            break;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
        default:
            break;
        }
    }
    else
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTcpEventCallback - connection 0x%p: Can not match event 0x%p", pConn, selectEvent));
        TransportConnectionUnLockEvent(pConn);
        return;
    }

    if(status != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTcpEventCallback - connection 0x%p: Error handling event %d", pConn, selectEvent));

        if (RV_ERROR_OUTOFRESOURCES == status)
        {
            RLIST_ITEM_HANDLE oorListLocation = NULL;
/*OOR fix storing*/
			/*Since we encountered out of resource error, first we need to stop receiving events for this particular
			  socket.*/
			if (RvFdIsClosedByPeer(&pConn->ccSelectFd) == RV_TRUE)
			{
				RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
					"TransportTcpEventCallback - connection 0x%p: FD (0x%p) was closed by TCP peer trying to remove it from select engine.", pConn, &pConn->ccSelectFd));
				
				/*If the FD is closed by peer we can't update the ccEvent of this FD so we need to remove this FD
				  A new FD will be constructed upon resource availablity*/
				if (RV_TRUE == pConn->bFdAllocated)
                {
					/*After removing the FD we won't get any new messages*/
                    crv = RvSelectRemove(pConn->pTransportMgr->pSelect,&pConn->ccSelectFd);
                    RvFdDestruct(&pConn->ccSelectFd);
                    pConn->bFdAllocated = RV_FALSE;
					/*If we didn't managed to remove this FD we need to terminate the connection explicitly*/
                    if (RV_OK != crv)
                    {
                       TransportConnectionTerminate(pConn, RVSIP_TRANSPORT_CONN_REASON_ERROR);
                    }
                }
			}
			else
			{
				RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
					"TransportTcpEventCallback - connection 0x%p: FD (0x%p) is valid, trying to update it to stop receiving new events.", pConn, &pConn->ccSelectFd));
				/*The FD was not closed by peer so we need to update it to NOT receiveing any new events*/
				pConn->ccEvent = 0;
				crv = RvSelectUpdate(pConn->pTransportMgr->pSelect,
					&pConn->ccSelectFd,
					0,
					NULL);				
			}

			/* Mark connection for OOR */
            pConn->oorEvents |= selectEvent;
            /* Add connection to list of connection that suffered from oor, Only if this connection is not in the list alredy*/
			/* If we invoked this callback from the recovery mechanism and we still have OOR problem, we don't need to 
			   Insert this connection to the OOR list again.*/
			if (pConn->oorListLocation == NULL)
			{
				RvMutexLock(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
				status = RLIST_InsertTail(pTransportMgr->oorListPool,pTransportMgr->oorList,&oorListLocation);
				RvMutexUnlock(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
				if (RV_OK != status)
				{
					TransportConnectionUnLockEvent(pConn);
					RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
						"TransportTcpEventCallback - pConn=0x%p, RLIST_InsertTail returned %d error when adding OOR element",pConn,status));
					return;
				}
				else 
				{
					RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
						"TransportTcpEventCallback - connection 0x%p: was inserted to OOR list in location 0x%p.", pConn, oorListLocation));
				}

				*((TransportConnection **)(oorListLocation)) = pConn;
				pConn->oorListLocation = oorListLocation;
			}
			else 
			{
				RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
						"TransportTcpEventCallback - connection 0x%p: was inserted to OOR list location 0x%p.", pConn, pConn->oorListLocation));
			}
            
        }
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportTcpEventCallback - connection 0x%p (sock:%d): Event callback handled succesfully. event=%d", pConn, pConn->ccSocket,selectEvent));
    }

    TransportConnectionUnLockEvent(pConn);

    return;
}

/******************************************************************************
 * TransportTCPReceive
 * ----------------------------------------------------------------------------
 * General: reads data from Connection socket
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn      - The Connection object.
 * Output:  recvBuf    - Buffer into which the data should be read from socket.
 *          pRecvBytes - Number of received bytes.
 *****************************************************************************/
RvStatus RVCALLCONV TransportTCPReceive(
                                IN  TransportConnection* pConn,
                                OUT RvChar*              recvBuf,
                                OUT RvSize_t*            pRecvBytes)
{
    RvStatus crv;

    crv = RvSocketReceiveBuffer(pConn->pSocket, (RvUint8*)recvBuf,
                                pConn->pTransportMgr->maxBufSize-1,
                                pConn->pTransportMgr->pLogMgr, pRecvBytes,
                                NULL);
    if (RV_OK != crv)
    {
        RvStatus status;
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTCPReceive - Connection 0x%p: can not read from socket(%d), terminating connection", 
            pConn,*(pConn->pSocket)));
        /* An error during TCP socket read indicates that the connection is invalid */
		status = TransportTCPClose(pConn);
		if (status != RV_OK) 
		{
		    RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
			    "TransportTCPReceive - Connection 0x%p: failed to close", pConn));
        }
	    return CRV2RV(crv);
    }
    return RV_OK;
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/


/************************************************************************************
 * TcpHandleCloseEvent
 * ----------------------------------------------------------------------------------
 * General: This function is used in the multi threaded SIP stack configuration
 *            when TCP connection close event is discovered. It sends the DISCONNECTED_EVENT
 *            event to the processing queue.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
static RvStatus RVCALLCONV TcpHandleCloseEvent(IN TransportConnection   *pConn)
{
    RvStatus                        rv = RV_OK;
    TransportProcessingQueueEvent*  ev = NULL;

    /* Warn if there is message that was not received completely */
    if (pConn->recvInfo.hMsgReceived != NULL_PAGE)
    {
        RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                     "TcpHandleCloseEvent - connection 0x%p, socket %d: there is not complete message in RECV buffer",
                     pConn, pConn->ccSocket));
    }

    if (pConn->originalBuffer != NULL)
    {
        /* Some data was read but still have not been processed
        due to out-of-resource of receive buffers. The disconnect should
        be postpone until recovery from the out-of-resource.
        Note that it is supposed that after 'close' event
        no more 'read events can be received on the socket.*/
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                      "TcpHandleCloseEvent - connection 0x%p, socket %d: originalBuffer=0x%p returning OOR",
                      pConn,pConn->ccSocket,pConn->originalBuffer));
        return RV_ERROR_OUTOFRESOURCES;
    }
    if (pConn->bCloseEventInQueue == RV_TRUE)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                      "TcpHandleCloseEvent - connection 0x%p, socket %d: Close event in queue, returning OK",pConn,pConn->ccSocket));
        return RV_OK;
    }
    /*kill timers on server connections*/
    SipCommonCoreRvTimerCancel(&pConn->hServerTimer);

    SipCommonCoreRvTimerCancel(&pConn->saftyTimer);

    /*change the status . in this case, when the connection is closed from the remote
    side we can immediately close the connection and remove from the list */
    TransportConnectionHashRemove(pConn);

    /* stop listening on events */
    pConn->ccEvent  = 0;
    TransportConnectionRegisterSelectEvents(pConn,RV_FALSE,RV_FALSE);

    rv = TransportProcessingQueueAllocateEvent(
            (RvSipTransportMgrHandle)pConn->pTransportMgr,
            (RvSipTransportConnectionHandle)pConn, DISCONNECTED_EVENT,
            RV_FALSE/*bAllocateRcvdBuff*/, &ev);
    if (rv != RV_OK)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TcpHandleCloseEvent - connection 0x%p, socket %d: failed to allcoated an event. (rv=%d)",
            pConn,pConn->ccSocket,rv));

        return rv;
    }
    ev->typeSpecific.disconnected.pConn = pConn;
    rv = TransportConnectionIncUsageCounter(pConn);
    if (rv != RV_OK)
    {
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
        return rv;
    }
    rv = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
    if (rv != RV_OK)
    {
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
        TransportConnectionDecUsageCounter(pConn);
        return rv;
    }
    /*mark that there is a close event in the queue*/
    pConn->bCloseEventInQueue = RV_TRUE;
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TcpHandleCloseEvent - connection 0x%p, socket %d: close event inserted to queue, bCloseEventInQueue=RV_TRUE.",pConn,pConn->ccSocket));

    return rv;
}


/************************************************************************************
* TcpHandleConnectEvent
* ----------------------------------------------------------------------------------
* General: Treats the connect event that occurs when a client wants to connect to server
*            and this is the response from the remote server
*
* Return Value: RvStatus
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   pConn        - Pointer to the relevant connection
*          socket        - Pointer to the relevant connection info structure.
*           error        - if error was generated
* Output:  none
***********************************************************************************/
static RvStatus RVCALLCONV TcpHandleConnectEvent(IN TransportConnection *pConn,
                                                  IN RvBool              error)

{
    RvStatus                        status=0;
    TransportProcessingQueueEvent*  ev;

    if ((!error) && (pConn->eState != RVSIP_TRANSPORT_CONN_STATE_CONNECTING))
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TcpHandleConnectEvent - Cannot handle connect event for connection 0x%p , error %d, state %s",
            pConn,error, SipCommonConvertEnumToStrConnectionState(pConn->eState)));

        return (RV_ERROR_UNKNOWN);
    }

    /* Stop listening to events until the CONEECT_EVENT is handled */
    pConn->ccEvent = (RvSelectEvents)0;
    TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);

    status = TransportProcessingQueueAllocateEvent(
                (RvSipTransportMgrHandle)pConn->pTransportMgr,
                (RvSipTransportConnectionHandle)pConn, CONNECTED_EVENT,
                RV_FALSE/*bAllocateRcvdBuff*/, &ev);
    if (status != RV_OK)
    {
        pConn->oorConnectIsError = error;
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TcpHandleConnectEvent - connection 0x%p: Cannot allocate queue connect event (rv=%d:%s)",
            pConn, status, SipCommonStatus2Str(status)));
        return status;
    }

    ev->typeSpecific.connected.error = error;
    ev->typeSpecific.connected.pConn = pConn;

    if (!error)
    {
        /* mark the connection as open */
        pConn->eState = RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED;
        /*after the connection made , we can send the data*/
    }


    RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TcpHandleConnectEvent - processing connect event for connection 0x%p , socket %d",
            pConn,pConn->ccSocket));

    status = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
    if (status != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
             "TcpHandleConnectEvent - Failed to insert event into the processing queue"));
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
    }

    /* there was no OOR, we can reset the OOR connect error to false */
    pConn->oorConnectIsError = RV_FALSE;

    return status;
}


/************************************************************************************
 * TcpHandleAcceptEvent
 * ----------------------------------------------------------------------------------
 * General: treats the accept event occurs when new incoming request arrived.
 *            this function is related only to TCP server.
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          socket         - the socket arrived.
 * Output:
 ***********************************************************************************/
static RvStatus RVCALLCONV TcpHandleAcceptEvent(
                                         IN TransportConnection* pTcpConInfo,
                                         IN RvSocket*            pSocket)

{
    RvStatus                        rv              = RV_OK;
    TransportConnection*            pNewTCPConn     = NULL;
    RvSipTransportConnectionHandle  hTcpConn        = NULL;
    RvStatus                        crv             = RV_OK;
    TransportMgr*                   pTransportMgr   = pTcpConInfo->pTransportMgr;
    RvBool                          bNewConnCreated = RV_FALSE;
    RvBool                          bDrop           = RV_FALSE;
    /*accept can be only on a server connection*/
    if(pTcpConInfo->eConnectionType != RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TcpHandleAcceptEvent - Trying to accept on connection that is not a server, returning (rv=0)"));

        return RV_OK;
    }

    /* open TCP server */
    rv = TransportConnectionConstruct(pTransportMgr,
                                      RV_TRUE,
                                      RVSIP_TRANSPORT_CONN_TYPE_SERVER,
                                      pTcpConInfo->eTransport,
                                      pTcpConInfo->hLocalAddr,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &bNewConnCreated,
                                      &hTcpConn);

    if (rv != RV_OK)
    {

        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TcpHandleAcceptEvent - Failed to construct new connection"));
        RvSocketDontAccept(pSocket, pTransportMgr->pLogMgr);
        return RV_ERROR_UNKNOWN;
    }

    pNewTCPConn = (TransportConnection*)hTcpConn;

    crv = RvSocketAccept(pSocket,pTransportMgr->pLogMgr,&pNewTCPConn->ccSocket,&pNewTCPConn->destAddress.addr);
    if (crv != RV_OK)
    {
        /*Failed to get valid socket*/
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TcpHandleAcceptEvent - RvSocketAccept Failed (from socket %d). Connection is being closed.",*pSocket));

        TransportTCPClose(pNewTCPConn);
        return RV_ERROR_UNKNOWN;
    }
    pNewTCPConn->pSocket = &pNewTCPConn->ccSocket;
    pNewTCPConn->destAddress.eTransport = pNewTCPConn->eTransport;
    pNewTCPConn->typeOfService = pTcpConInfo->typeOfService;
#ifdef RV_SIP_IMS_ON
    pNewTCPConn->pSecOwner = ((TransportMgrLocalAddr*)(pTcpConInfo->hLocalAddr))->pSecOwner;
#endif /*#ifdef RV_SIP_IMS_ON*/

    if (TRANSPORT_CONNECTION_TOS_UNDEFINED != pNewTCPConn->typeOfService)
    {
        rv = RvSocketSetTypeOfService(pNewTCPConn->pSocket,pNewTCPConn->typeOfService,pNewTCPConn->pTransportMgr->pLogMgr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TcpHandleAcceptEvent - pConn 0x%p: Failed to set TypeOfService=%d for socket %d, rv=%d",
                pNewTCPConn,pNewTCPConn->typeOfService,pNewTCPConn->ccSocket,RvErrorGetCode(rv)));
            rv = RV_OK;
        }
    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TcpHandleAcceptEvent - pConn 0x%p: Socket %d opened for connection",pNewTCPConn, pNewTCPConn->ccSocket))

    /* Call to ConnectionIncoming callback in order to get application approval
    to read and send messages on the connection */
    TransportCallbackConnCreated(pNewTCPConn->pTransportMgr, pNewTCPConn, &bDrop);
    if (RV_TRUE == bDrop)
    {
        RvLogDebug(pNewTCPConn->pTransportMgr->pLogSrc,(pNewTCPConn->pTransportMgr->pLogSrc,
            "TcpHandleAcceptEvent - pConn 0x%p: Connection is being closed in according to Application request",pNewTCPConn))
        TransportTCPClose(pNewTCPConn);
        return RV_OK;
    }

    /* Register the Connection to CLOSE event in order to ensure Connection close,
    initiated by the Application from StateChange callback below */
    rv = TransportConnectionRegisterSelectEvents(pNewTCPConn,RV_TRUE,RV_TRUE/*bFirstCall*/);
	if (rv != RV_OK)
	{
		RvLogDebug(pNewTCPConn->pTransportMgr->pLogSrc,(pNewTCPConn->pTransportMgr->pLogSrc,
            "TcpHandleAcceptEvent - pConn 0x%p: failed to register select events",
			pNewTCPConn))
			TransportTCPClose(pNewTCPConn);
        return rv;
	}

    rv = TransportConnectionChangeState(pNewTCPConn,
            RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED,
            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED, RV_FALSE, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pNewTCPConn->pTransportMgr->pLogSrc,(pNewTCPConn->pTransportMgr->pLogSrc,
            "TcpHandleAcceptEvent - Connection 0x%p: failed to chage state to CONNECTED (rv=%d:%s)",
            pNewTCPConn, rv, SipCommonStatus2Str(rv)));
        TransportTCPClose(pNewTCPConn);
        return rv;
    }

    /* If the Connection state was changed from the Callback, it means the Connection was closed, */
    if (RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED != pNewTCPConn->eState)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TcpHandleAcceptEvent - New Connection 0x%p was closed by application. %s -> %s",
            pNewTCPConn,
            SipCommonConvertEnumToStrConnectionState(RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED),
            SipCommonConvertEnumToStrConnectionState(pNewTCPConn->eState)));
        return RV_OK;
    }
    
    /* set the ability to get read and close events on the connection */
    if (RVSIP_TRANSPORT_TCP == pNewTCPConn->eTransport)
    {
        pNewTCPConn->ccEvent = RvSelectRead;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_TRANSPORT_TLS == pNewTCPConn->eTransport)
    {
        TransportCallbackTLSSeqStarted(pNewTCPConn);

        if (NULL != pTransportMgr->appEvHandlers.pfnEvTlsStateChanged)
        {
            rv = TransportCallbackTLSStateChange(pNewTCPConn,
                            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY,
                            RVSIP_TRANSPORT_CONN_REASON_SERVER_CONNECTED,
                            RV_FALSE, RV_TRUE);
            if (RV_OK != rv)
            {
                RvLogError(pNewTCPConn->pTransportMgr->pLogSrc ,(pNewTCPConn->pTransportMgr->pLogSrc ,
                    "TcpHandleAcceptEvent - Connection 0x%p: failed to report HANDSHAKE_READY state(rv=%d:%s). Close the connection",
                    pNewTCPConn, rv, SipCommonStatus2Str(rv)));
                TransportTlsClose(pNewTCPConn);
                return rv;
            }

            /* making sure the socket was not closed during the handshake,
               if so, we return*/
            if (NULL == pNewTCPConn->pSocket)
            {
                return RV_OK;
            }
            /* making sure we know when the connection is closed from the other side */
            rv = TransportConnectionRegisterSelectEvents(pNewTCPConn,RV_TRUE,RV_FALSE/*bFirstCall*/);
			if (rv != RV_OK)
			{
				RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
					"TcpHandleAcceptEvent - Connection 0x%p failed to register select events, closing connection",pNewTCPConn));
				TransportTlsClose(pNewTCPConn);
				return rv;
			}
        }
        else
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TcpHandleAcceptEvent - Connection 0x%p can not report Tls state change. Terminating connection",pNewTCPConn));

            TransportTlsClose(pNewTCPConn);
            return RV_ERROR_UNKNOWN;
        }
        return RV_OK;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
    /* register the events for the connection */
    TransportConnectionRegisterSelectEvents(pNewTCPConn,RV_TRUE,RV_FALSE/*bFirstCall*/);

    return RV_OK;
}

/************************************************************************************
 * TCPHandleReadWriteEvents
 * ----------------------------------------------------------------------------------
 * General: Handles read and write event for TCP (similar function exists for TLS)
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTcpConn - Pointer to the relevant connection.
 *          event    - The event to be treated
 * Output:
 ***********************************************************************************/
static RvStatus RVCALLCONV TCPHandleReadWriteEvents(IN TransportConnection*   pConn,
                                                     IN RvSelectEvents         event)
{
    RvStatus                        status      = RV_OK;

    if (event & RvSelectWrite)
    {
        status = TransportTcpHandleWriteEvent(pConn);
    }

    else if (event & RvSelectRead)
    {
        if (pConn->eState != RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED &&
            pConn->eState != RVSIP_TRANSPORT_CONN_STATE_CLOSING)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TCPHandleReadWriteEvents - connection 0x%p: read event in not CONNECTED/CLOSING state",
                pConn));

            /* Empty TCP socket */
            RvSocketClean(pConn->pSocket,pConn->pTransportMgr->pLogMgr);

            /* Re-register socket events */
            status = TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);
		    if (status != RV_OK)
		    {
			    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
				    "TCPHandleReadWriteEvents - connection 0x%p: failed to register socket events",
				    pConn));
			    return status;
		    }
            return RV_OK;
        }
        status = TransportConnectionReceive(pConn);
    }
    return status;
}

/***************************************************************************
 * TcpSaftyShutdownNoCloseTimerEvHandler
 * ------------------------------------------------------------------------
 * General: this function is set on calling shutdown and if close event
 *          does not arrive, it will simulate it
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pContext - The connection using this timer.
 ***************************************************************************/
static RvBool TcpSaftyShutdownNoCloseTimerEvHandler(IN void    *pContext,
                                                    IN RvTimer *timerInfo)
{
    RvStatus rv;
    TransportConnection* pConn = (TransportConnection*)pContext;
    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pConn->saftyTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc  ,
            "TcpSaftyShutdownNoCloseTimerEvHandler - Connection 0x%p : timer expired but was already released. do nothing...",
            pConn));
        
        TransportConnectionUnLockEvent(pConn);
        return RV_FALSE;
    }
    SipCommonCoreRvTimerExpired(&pConn->saftyTimer);
    SipCommonCoreRvTimerCancel(&pConn->saftyTimer);

    /* the timer expired after we got a close event from the net */
    if (pConn->bCloseEventInQueue == RV_TRUE)
    {
        TransportConnectionUnLockEvent(pConn);
        return RV_FALSE;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TcpSaftyShutdownNoCloseTimerEvHandler- Connection 0x%p - timer expired, Simulating close event ",pConn));   
           
    TcpHandleCloseEvent(pConn);

    TransportConnectionUnLockEvent(pConn);
    return RV_FALSE;
}

/***************************************************************************
 * TcpShutdownGracefully
 * ------------------------------------------------------------------------
 * General: This function shutdowns a connection gracefully, by reading the
 *          left the bytes in the socket before closing the socket 
 *          completely.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pConn - Pointer to the relevant connection.
 ***************************************************************************/
static RvStatus TcpShutdownGracefully(IN TransportConnection *pConn)
{
    RvStatus rv;

    /* 1. trying and shutdown gracefully */
    
    /* Update the socket events before closing it for preventing an error */
    pConn->ccEvent = RvSelectRead;
    TransportConnectionRegisterSelectEvents(pConn,RV_TRUE,RV_FALSE);
    rv = RvSocketShutdown(pConn->pSocket,RV_FALSE, pConn->pTransportMgr->pLogMgr);
    
    /* 2.1 if shutdown failed - kill the socket*/
    if (rv != RV_OK)
    {
        TransportMgrSocketDestruct(
            pConn->pTransportMgr, pConn->pSocket, RV_TRUE/*bCleanSocket*/,
            ((pConn->bFdAllocated==RV_TRUE)?&pConn->ccSelectFd:NULL));
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TcpShutdownGracefully - connection 0x%p: (socket:%d) marked as 'deleted' after shutdown", pConn,pConn->ccSocket));
        pConn->pSocket      = NULL;
        pConn->isDeleted    = RV_TRUE;
        pConn->bFdAllocated = RV_FALSE;
        pConn->ccEvent      = (RvSelectEvents )0;
    }

    /* 2.2 if shutting down gracefully, try and read what ever is left on the socket */
    else if (RVSIP_TRANSPORT_TCP ==  pConn->eTransport)
    {
        RvSize_t leftOverBytes   = 0;
        RvStatus crv            = RV_OK;
        /* 2.2.1 if there are any messages on the socket, we try to read them, if no resources are left,
        there is nothing we can do here, the message will be ignored*/
        
        crv = RvSocketGetBytesAvailable(pConn->pSocket,pConn->pTransportMgr->pLogMgr,&leftOverBytes);
        if (RV_OK == crv && leftOverBytes > 0 && RVSIP_TRANSPORT_TCP == pConn->eTransport)
        {
            /* read leftovers */
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TcpShutdownGracefully - connection 0x%p: (socket:%d) Reading leftovers", pConn,pConn->ccSocket));
            TransportConnectionReceive(pConn);
        }
        
        /* 2.2.2 if we cant read anything new from the socket kill it.
        this if is executed if the socket is not valid*/
        else if (RV_OK != crv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TcpShutdownGracefully - connection 0x%p: (socket:%d) could not get bytes avalable, removing socket", 
                pConn,pConn->ccSocket));

            TransportMgrSocketDestruct(pConn->pTransportMgr,
                pConn->pSocket, RV_TRUE/*bCleanSocket*/,
                ((pConn->bFdAllocated==RV_TRUE)?&pConn->ccSelectFd:NULL));
            pConn->pSocket = NULL;
            pConn->isDeleted = RV_TRUE;
            pConn->bFdAllocated = RV_FALSE;
            pConn->ccEvent      = (RvSelectEvents )0;
        }
    }

    return rv;
}

