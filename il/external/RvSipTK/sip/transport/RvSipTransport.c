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
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              RvSipTransport.c
 *
 * This c file contains implementations for the transport layer API.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *   Tamar Barzuza                    Jan 2002
 *   Sarit Galanos                  Jan 2003
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "rvhost.h"
#include "_SipTransport.h"
#include "_SipTransportTypes.h"
#include "_SipCommonCore.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransport.h"
#include "RvSipTransportTypes.h"
#include "TransportMsgBuilder.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#include "AdsRpool.h"
#include "TransportDNS.h"
#include "TransportMgrObject.h"
#include "TransportProcessingQueue.h"
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* TransportError handlers 
*/
#if defined(UPDATED_BY_SPIRENT)

static RvSipTransportErrorEvHandlers SIP_TransportErrorEvHandlers = {NULL};

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                          MACROS DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV TransportInjectMsg(
                        IN  RvSipTransportMgrHandle            hTransportMgr,
                        IN  RvChar                        *pMsgBuffer,
                        IN  RvUint32                       totalMsgLength,
                        IN  RvSipMsgHandle                  hMsg,
                        IN  RvSipTransportConnectionHandle  hConn,
                        IN  RvSipTransportMsgAddrCfg     *pAddressInfo);

static RvStatus RVCALLCONV SetInjectAddresses(
                        IN TransportMgr*                  pTransportMgr,
                        IN TransportProcessingQueueEvent* ev,
                        IN RvSipTransportConnectionHandle hConn,
                        IN RvSipTransportMsgAddrCfg*     pAddressInfo);
#endif /*#ifndef RV_SIP_PRIMITIVES */

static RvBool RVCALLCONV IsValidLocalAddress(
							 IN  TransportMgr					*pTransportMgr,
							 IN  RvSipTransportAddr             *pAddressDetails,
							 IN  RvUint32                        addrStructSize);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransportConvertStringToIp
 * ------------------------------------------------------------------------
 * General: Converts an ip from string format to binary format
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr     - Handle to the transport manager
 *          pszIpAsString    - a NULL terminated string representing an ip address
 *                             (d.d.d.d for ipv4, x:x:x:x:x:x:x:x for ipv6)
 *                             Note: do not use [] for ipv6 addresses.
 *             eAddressType     - represent the type of address ipv6/ipv4
 * Output:  pIpAsBinary      - the ip address represented in binary.
 *                             (16 bytes for ipv6, 4 bytes for ipv4)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertStringToIp (IN  RvSipTransportMgrHandle   hTransportMgr,
                                                            IN  RvChar*                  pszIpAsString,
                                                            IN  RvSipTransportAddressType eAddressType,
                                                            OUT RvUint8*                  pIpAsBinary)
{
    TransportMgr*  pTransportMgr   = (TransportMgr*)hTransportMgr;
    RvAddress      address;
    RvUint32       ipv4            = 0;
#if(RV_NET_TYPE & RV_NET_IPV6)
    const RvUint8* pipv6           = NULL;
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    if (NULL == pszIpAsString)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    switch (eAddressType)
    {
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:
        {
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&address);
            if (NULL == RvAddressSetString(pszIpAsString,&address))
            {

                return RV_ERROR_BADPARAM;
            }
            ipv4 = RvAddressIpv4GetIp(RvAddressGetIpv4(&address));
            memcpy(pIpAsBinary,&ipv4,sizeof(ipv4));
        }
    break;
#if(RV_NET_TYPE & RV_NET_IPV6)
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:
        {
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&address);
            if (NULL == RvAddressSetString(pszIpAsString,&address))
            {    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                            "RvSipTransportConvertStringToIp - string contain an invalid IP address"));
                return RV_ERROR_BADPARAM;
            }
            pipv6 = RvAddressIpv6GetIp(RvAddressGetIpv6(&address));
            memcpy(pIpAsBinary,pipv6,RVSIP_TRANSPORT_LEN_BYTES_IPV6);
        }
    break;
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    default:
        return RV_ERROR_BADPARAM;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConvertIpToString
 * ------------------------------------------------------------------------
 * General: Converts an ip from binary format to string format
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr     - Handle to the transport manager
 *          pIpAsBinary      - the ip address represented in binary.
 *                             (16 bytes for ipv6, 4 bytes for ipv4)
 *          stringLen        - the size of the buffer.
 *             eAddressType     - represent the type of address ipv6/ipv4
 * Output:  pszIpAsString    - a NULL terminated string representing an ip address
 *                             (d.d.d.d for ipv4, x:x:x:x:x:x:x:x for ipv6)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertIpToString (IN  RvSipTransportMgrHandle   hTransportMgr,
                                                            IN  RvUint8*                  pIpAsBinary,
                                                            IN  RvSipTransportAddressType eAddressType,
                                                            IN  RvInt32                  stringLen,
                                                            OUT RvChar*                  pszIpAsString)
{
    TransportMgr*  pTransportMgr = (TransportMgr*)hTransportMgr;
    RvStatus       rv           = RV_OK;
    RvAddress       address;

    if (NULL == pszIpAsString)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    switch (eAddressType)
    {
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:
        {
			RvUint32 ipVal;
			memcpy(&ipVal, pIpAsBinary, sizeof(ipVal));
            RvAddressConstructIpv4(&address,ipVal,0);
            memcpy(&address.data.ipv4.ip,pIpAsBinary,4);
        }
    break;
#if(RV_NET_TYPE & RV_NET_IPV6)
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:
        {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            RvAddressConstructIpv6(&address,pIpAsBinary,0,0,0);
#else
            RvAddressConstructIpv6(&address,pIpAsBinary,0,0);
#endif
/* SPIRENT_END */
        }
    break;
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    default:
        return RV_ERROR_BADPARAM;
    }
    RvAddressGetString(&address,stringLen,pszIpAsString);

    return rv;
}

/***************************************************************************
 * RvSipTransportMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for the transport events.
 * Return Value: RV_ERROR_NULLPTR - The pointer to the handlers struct is NULL.
 *                 RV_OK - The event handlers were successfully set.
 *                 RV_ERROR_BADPARAM - Size of struct is negative.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *            hAppTransportMgr - An application handle. This handle will be supplied
 *                        with the transport callback for message-receievd
 *                        and message to send.
 *          pHandlers   - Pointer to structure containing application event
 *                        handler pointers.
 *          evHandlerStructSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetEvHandlers(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipAppTransportMgrHandle       hAppTransportMgr,
                        IN RvSipTransportMgrEvHandlers     *pHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransportMgr       *pTransportMgr;
    RvLogSource*       logId;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransportMgrEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransportMgrEvHandlers);

    pTransportMgr = (TransportMgr *)hTransportMgr;

    if ((NULL == hTransportMgr) ||
        (NULL == pHandlers) )
    {
        return RV_ERROR_NULLPTR;
    }

    RvMutexLock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    logId = pTransportMgr->pLogSrc;

    if (evHandlerStructSize == 0)
    {
        RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
        return RV_ERROR_BADPARAM;
    }

    /* The manager signs in the application manager:
       Set application's callback*/
    memset(&(pTransportMgr->appEvHandlers), 0, sizeof(RvSipTransportMgrEvHandlers));
    memcpy(&(pTransportMgr->appEvHandlers), pHandlers, minStructSize);

    /* Set the application manager to the transaction manager */
    if(hAppTransportMgr != NULL)
    {
        pTransportMgr->hAppTransportMgr = hAppTransportMgr;
    }

    RvLogDebug(logId ,(logId ,
              "RvSipTransportMgrSetEvHandlers - application event handlers were successfully set to the transport manager"));
    RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the handle to the application transport manger object.
 *          You can also supply this handle when calling
 *          RvSipTransportMgrSetEvHandlers().
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the stack transport manager.
 *           hAppTransportMgr - The application transport manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetAppMgrHandle(
                                   IN RvSipTransportMgrHandle      hTransportMgr,
                                   IN RvSipAppTransportMgrHandle   hAppTransportMgr)
{
    TransportMgr  *pTransportMgr;
    if(hTransportMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTransportMgr = (TransportMgr *)hTransportMgr;

    RvMutexLock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    pTransportMgr->hAppTransportMgr = hAppTransportMgr;

    RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransportMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application transport manger object.
 *          You set this handle in the stack using the
 *          RvSipTransportMgrSetEvHandlers() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 * Output:     phAppTransportMgr - The application transport manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrGetAppMgrHandle(
                         IN RvSipTransportMgrHandle        hTransportMgr,
                         OUT RvSipAppTransportMgrHandle*   phAppTransportMgr)
{
    TransportMgr  *pTransportMgr;
    if(hTransportMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTransportMgr = (TransportMgr *)hTransportMgr;

    RvMutexLock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    *phAppTransportMgr = pTransportMgr->hAppTransportMgr;

    RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransportMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transport
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr   - Handle to the transport manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrGetStackInstance(
                                   IN RvSipTransportMgrHandle hTransportMgr,
                                   OUT  void*                *phStackInstance)
{
    TransportMgr *pTransportMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = (TransportMgr *)hTransportMgr;
    *phStackInstance = pTransportMgr->hStack;
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportMgrSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for the entire stack that will be used
 *			by all objects. This function is equivalent to the outbound address
 *			that is being set in configuration except that now the address
 *			can be updated during the lifetime of the stack.
 *
 *   		Note that unless there are specific reasons to use this function, it is
 *			recommended to set this address only in the construction of the stack
 *			through the configuration from the following reasons:
 *			1. Setting the outbound address during the time that transactions
 *			   are alive and send messages might cause the address not to be
 *			   updated completely.
 *			2. Even if it is updated completely, changing the outbound address
 *			   while there is an active transaction that sends messages (using a transmitter)
 *			   will not change the destination. This means that if the first
 *			   message was sent to the old outbound proxy address, the retransmissions
 *			   will be sent to the same old address.
 *			3. This function allocates dynamic memory to hold the address.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr   - Handle to the transport manager.
 *            pOutboundAddr   - A valid pointer that holds the new outbound address.
 *            sizeOfCfg       - The size of the outbound proxy configuration structure.
 * Output:    None
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetOutboundAddress(
                                   IN RvSipTransportMgrHandle          hTransportMgr,
                                   IN RvSipTransportOutboundProxyCfg   *pOutboundAddr,
								   IN RvInt32                          sizeOfCfg)
{
    TransportMgr  *pTransportMgr;
	RvInt32	      length;
	RvChar		  *pStrIpHolder = NULL;
	RvChar		  *pStrHostNameHolder = NULL;
#ifdef RV_SIGCOMP_ON
	RvChar        *pSigcompIdHolder = NULL;
#endif

	RV_UNUSED_ARG(sizeOfCfg);
    if(hTransportMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTransportMgr = (TransportMgr *)hTransportMgr;

    RvMutexLock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);

	/* saving the old strings of the outbound address to be freed later on */
	pStrIpHolder = pTransportMgr->outboundAddr.strIp;
	pStrHostNameHolder = pTransportMgr->outboundAddr.strHostName;
#ifdef RV_SIGCOMP_ON
	pSigcompIdHolder = pTransportMgr->outboundAddr.strSigcompId;
#endif

	/* initializing the outbound structure */
	memset(&pTransportMgr->outboundAddr,0,sizeof(TransportOutboundAddr));

	/* setting the ip address */
    if(pOutboundAddr->strIpAddress != NULL &&
       strcmp(pOutboundAddr->strIpAddress, "")                 != 0    &&
       strcmp(pOutboundAddr->strIpAddress, IPV4_LOCAL_ADDRESS) != 0    &&
       memcmp(pOutboundAddr->strIpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) != 0)
    {
        length = (RvInt32)strlen(pOutboundAddr->strIpAddress) + 1;

        if (RV_OK != RvMemoryAlloc(NULL, length, pTransportMgr->pLogMgr, (void*)&pTransportMgr->outboundAddr.strIp))
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                       "RvSipTransportMgrSetOutboundAddress - Failed to allocate outbound proxy ip"));
			RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }
        strcpy(pTransportMgr->outboundAddr.strIp, pOutboundAddr->strIpAddress);
		/* freeing the already allocated memory */
		if (pStrIpHolder != NULL)
		{
			RvMemoryFree((void*)pStrIpHolder, pTransportMgr->pLogMgr);
		}
    }
    pTransportMgr->outboundAddr.port      = (RvUint16)pOutboundAddr->port;
    pTransportMgr->outboundAddr.transport = pOutboundAddr->eTransport;
#ifdef RV_SIGCOMP_ON
    pTransportMgr->outboundAddr.compType     = pOutboundAddr->eCompression;
	length = (RvInt32)strlen(pOutboundAddr->strSigcompId) + 1;
	if (RV_OK != RvMemoryAlloc(NULL, length, pTransportMgr->pLogMgr, (void*)&pTransportMgr->outboundAddr.strSigcompId))
	{
		RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
			"RvSipTransportMgrSetOutboundAddress - Failed to allocate outbound proxy sigcomp-id"));
		RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
		return RV_ERROR_OUTOFRESOURCES;
	}
	strcpy(pTransportMgr->outboundAddr.strSigcompId, pOutboundAddr->strSigcompId);
	/* freeing the already allocated memory */
	if (pSigcompIdHolder != NULL)
	{
		RvMemoryFree((void*)pSigcompIdHolder, pTransportMgr->pLogMgr);
	}
#endif

    /*if there is no outbound ip see if there is a domain name*/
    if(pTransportMgr->outboundAddr.strIp == NULL)
    {
        if(pOutboundAddr->strHostName != NULL &&
           strcmp(pOutboundAddr->strHostName,"") != 0)
        {
            length = (RvInt32)strlen(pOutboundAddr->strHostName) + 1;

            if (RV_OK != RvMemoryAlloc(NULL, length, pTransportMgr->pLogMgr, (void*)&pTransportMgr->outboundAddr.strHostName))
            {
                RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                          "RvSipTransportMgrSetOutboundAddress - Failed to allocate outbound proxy name"));
                RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
				return RV_ERROR_OUTOFRESOURCES;
            }
            strcpy(pTransportMgr->outboundAddr.strHostName, pOutboundAddr->strHostName);
			/* freeing the already allocated memory */
			if (pStrHostNameHolder != NULL)
			{
				RvMemoryFree((void*)pStrHostNameHolder, pTransportMgr->pLogMgr);
			}
        }
    }

    RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    return RV_OK;

}


/*-----------------------------------------------------------------------*/
/*                PERSISTENT CONNECTION API                              */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransportMgrCreateConnection
 * ------------------------------------------------------------------------
 * General: Constructs a new un-initialized connection and attach the supplied
 *          owner to the connection. The new connection assumes the IDLE state.
 *          Calling the RvSipTransportConnectionInit() function at this state
 *          will initialize the connection and will cause the connection to move
 *          to the READY state.
 *          NOTE: This function does not connect the connection. In order to
 *          connect the connection you must first initialize it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hOwner     - An handle to the connection owner.
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the Event handler structure
 * Output:  phConn     - handle to a newly creates connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrCreateConnection(
     IN  RvSipTransportMgrHandle                 hTransportMgr,
     IN  RvSipTransportConnectionOwnerHandle     hOwner,
     IN  RvSipTransportConnectionEvHandlers      *pEvHandlers,
     IN  RvInt32                                sizeofEvHandlers,
     OUT RvSipTransportConnectionHandle          *phConn)
{
    RvStatus rv;
    TransportConnection* pConn;
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;
    RvSipTransportConnectionEvHandlers        evHandlers;
    memset(&evHandlers,0, sizeof(RvSipTransportConnectionEvHandlers));

    if(hTransportMgr == NULL || phConn == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
             "RvSipTransportMgrCreateConnection - About to create a new connection"));


    if(pEvHandlers != NULL)
    {
        memcpy(&evHandlers,pEvHandlers,sizeofEvHandlers);
    }


    rv = TransportConnectionAllocate(pTransportMgr,phConn);
    if(rv != RV_OK)
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportMgrCreateConnection - Failed to allocate new connection"));
        return rv;
    }
    pConn = (TransportConnection*)*phConn;

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        TransportConnectionFree(pConn);
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionAttachOwner(pConn,evHandlers.pfnConnStateChangedEvHandler,
                                        evHandlers.pfnConnStausEvHandler,hOwner);
    if(rv != RV_OK)
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportMgrCreateConnection - Failed to attach connection owner to connection 0x%p",pConn));
        TransportConnectionFree(pConn);
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        *phConn = NULL;
        return rv;
    }

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
             "RvSipTransportMgrCreateConnection - A new connection 0x%p was constructed succesfully",pConn));

    return RV_OK;
}



/***************************************************************************
 * RvSipTransportConnectionInit
 * ------------------------------------------------------------------------
 * General: Initializes a connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection to be initialized.
 *          pCfg  - The configuration to use when initializing the connection.
 *          sizeofCfg - The size of the configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionInit(
                        IN RvSipTransportConnectionHandle      hConn,
                        IN RvSipTransportConnectionCfg        *pCfg,
                        IN RvInt32                             sizeofCfg)
{
    RvStatus                      rv         = RV_OK;
    TransportConnection*          pConn      = (TransportConnection*)hConn;
    TransportMgr*                 pTransportMgr;
    RvBool                        bIsIpV4    = RV_TRUE;
    SipTransportAddress           destAddr;
    RvSipTransportAddressType     eAddrTypeSrc = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    RvSipTransportAddressType     eAddrTypeDst = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    RvSipTransportLocalAddrHandle hLocalAddr = NULL;

    if(hConn==NULL || pCfg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionInit - Initializing connection 0x%p (transport=%s,localAddr=%s:%d,destAddr=%s:%d, strHostName=%s)",
                                              pConn,
                                              SipCommonConvertEnumToStrTransportType(pCfg->eTransportType),
                                              (pCfg->strLocalIp == NULL)?"None":pCfg->strLocalIp,
                                              pCfg->localPort,
                                              (pCfg->strDestIp == NULL)?"None":pCfg->strDestIp,
                                              pCfg->destPort,
                                              (pCfg->strHostName== NULL)?"None":pCfg->strHostName));

    /*check that the connection is idle*/
    if(pConn->pHashElement != NULL ||
       RVSIP_TRANSPORT_CONN_TYPE_UNDEFINED != pConn->eConnectionType)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed, A connection can be initalized only once (Connection type is %d)",
                  pConn->eConnectionType));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_UNKNOWN;
    }

    if(pCfg->strDestIp == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed, destination IP cannot be NULL"));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_BADPARAM;
    }

    if(pCfg->eTransportType != RVSIP_TRANSPORT_TCP &&
       pCfg->eTransportType != RVSIP_TRANSPORT_TLS &&
       pCfg->eTransportType != RVSIP_TRANSPORT_SCTP)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed, Invalid transport type"));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_BADPARAM;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(pCfg->eTransportType == RVSIP_TRANSPORT_TLS &&
       pCfg->strHostName == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed, host name cannot be NULL"));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_BADPARAM;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

    /*fixing local and remote port*/
    if(pCfg->localPort == 0)
    {
        pCfg->localPort = (RvUint16)((pCfg->eTransportType==RVSIP_TRANSPORT_TCP ||
                                      pCfg->eTransportType==RVSIP_TRANSPORT_SCTP)?
                                     5060:5061);
    }
    if(pCfg->destPort == 0)
    {
        pCfg->destPort = (RvUint16)((pCfg->eTransportType==RVSIP_TRANSPORT_TCP ||
                                     pCfg->eTransportType==RVSIP_TRANSPORT_SCTP)?
                                    5060:5061);
    }
    if(pCfg->strDestIp[0] == '[')
    {
        bIsIpV4      = RV_FALSE;
        eAddrTypeDst = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }
    if(pCfg->strLocalIp != NULL && pCfg->strLocalIp[0] == '[')
    {
        eAddrTypeSrc = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }
    /* Ensure that both Source and Destination IPs are of same type.
       Esxception is made for SCTP addresses. */
    if(pCfg->strLocalIp != NULL && RVSIP_TRANSPORT_SCTP != pCfg->eTransportType)
    {
        if((pCfg->strLocalIp[0] == '['  && pCfg->strDestIp[0] != '[') ||
           (pCfg->strLocalIp[0] != '['  && pCfg->strDestIp[0] == '['))
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                "RvSipTransportConnectionInit - Failed - both Local and Destination IPs should be of same type. IPv6 addresses should be surrounded by brackets"));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_ERROR_BADPARAM;
        }
    }

    rv = TransportMgrConvertString2Address(pCfg->strDestIp,&destAddr,bIsIpV4,RV_TRUE);
    if(rv != RV_OK)
    {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed to convert string dest ip to address (rv=%d)",rv));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }

    destAddr.eTransport = pCfg->eTransportType;
    RvAddressSetIpPort(&destAddr.addr,pCfg->destPort);

    /*find handle of local address*/
    if(pCfg->strLocalIp == NULL)
    {
        rv = SipTransportLocalAddressGetDefaultAddrByType(
            (RvSipTransportMgrHandle)pTransportMgr,
            pCfg->eTransportType, eAddrTypeDst, &hLocalAddr);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed to find local address that matches the dest address (rv=%d)",rv));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return rv;

        }
    }
    else
    {
        rv = SipTransportLocalAddressGetHandleByAddress(
            (RvSipTransportMgrHandle)pTransportMgr,pCfg->eTransportType,
            eAddrTypeSrc, pCfg->strLocalIp, pCfg->localPort, 
#if defined(UPDATED_BY_SPIRENT)
	    pCfg->if_name,
	    pCfg->vdevblock,
#endif 
	    &hLocalAddr);

        /* SCTP Local Address of IPv6 type can hold IPv4 addresses. 
           If the IPv4 Local Address was not found, try to find IPv6 addr. */
#if (RV_NET_TYPE & RV_NET_SCTP)
        if (RV_ERROR_NOT_FOUND == rv  &&
            RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eAddrTypeSrc)
        {
            eAddrTypeSrc = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
            rv = SipTransportLocalAddressGetHandleByAddress(
                (RvSipTransportMgrHandle)pTransportMgr,pCfg->eTransportType,
                eAddrTypeSrc, pCfg->strLocalIp, pCfg->localPort, 
#if defined(UPDATED_BY_SPIRENT)
		pCfg->if_name,
		pCfg->vdevblock,
#endif
		&hLocalAddr);
        }
#endif
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed Invalid local address (rv=%d)",rv));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return rv;
        }
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(pCfg->eTransportType == RVSIP_TRANSPORT_TLS)
    {
        rv = SipTransportConnectionSetHostStringName((RvSipTransportConnectionHandle )pConn,pCfg->strHostName);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                "RvSipTransportConnectionInit - Failed cant set the host name for TLS post connection assertion (rv=%d)",rv));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return rv;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE) */

    rv = TransportConnectionInit(pConn,RVSIP_TRANSPORT_CONN_TYPE_CLIENT,
                                 pCfg->eTransportType,hLocalAddr,&destAddr,RV_TRUE);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionInit - Failed to initialize connection 0x%p (rv=%d)",pConn,rv));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    if (RVSIP_TRANSPORT_SCTP == pCfg->eTransportType)
    {
        SipTransportConnectionSetSctpMsgSendingParams(
                                        (RvSipTransportConnectionHandle)pConn,
                                        &pCfg->sctpMsgSendParams);
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    RV_UNUSED_ARG(sizeofCfg);

    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionConnect
 * ------------------------------------------------------------------------
 * General: connecting a connection.
 *          Note - the connection must be in the idle state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection to connect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionConnect(
                        IN RvSipTransportConnectionHandle hConn)
{

    RvStatus rv;
    TransportConnection*      pConn = (TransportConnection*)hConn;
    TransportMgr*             pTransportMgr;

    if(hConn==NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
             "RvSipTransportConnectionConnect - connection 0x%p: About to connect",pConn));

    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT != pConn->eConnectionType)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
             "RvSipTransportConnectionConnect - connection 0x%p is not client (type %d)",
             pConn, pConn->eConnectionType));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_BADPARAM;
    }

    if(pConn->eState != RVSIP_TRANSPORT_CONN_STATE_READY)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionConnect - Failed, connection 0x%p cannot connect on state %s",
                  pConn,SipCommonConvertEnumToStrConnectionState(pConn->eState)));
         TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_UNKNOWN;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionConnect - connecting connection 0x%p",pConn));


    rv = TransportConnectionOpen(pConn, RV_TRUE/*bConnect*/,
            0/*clientLocalPort -> local port will be choosen by OS*/);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "RvSipTransportConnectionConnect - Failed to connect connection 0x%p",pConn))
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionAttachOwner
 * ------------------------------------------------------------------------
 * General: Attach a new owner to the supplied connection together with a set
 *          of callback functions that will be used to notify this owner about
 *          connection states and statuses. You can use this function only if
 *          the connection is connected or in the process to be connected.
 *          You cannot attach an owner to a connection that started its
 *          disconnection process.
 *          Notes:
 *              The connection will not disconnect as long as it has owners
 *          attached to it.
 *              Application can't attach owner to incoming connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to the connection.
 *          hOwner - The owner handle
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the event handler tructure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionAttachOwner(
            IN  RvSipTransportConnectionHandle            hConn,
            IN  RvSipTransportConnectionOwnerHandle       hOwner,
            IN  RvSipTransportConnectionEvHandlers        *pEvHandlers,
            IN  RvInt32                                   sizeofEvHandlers)
{

    RvStatus             rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;

    if(hConn==NULL || hOwner==NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /* Don't give chance to application to affect on Server Connection life
    cycle. The connection should be terminated by client on Transaction end */
    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT != pConn->eConnectionType)
    {
        RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionAttachOwner - Conn: 0x%p - illegal action for server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SipTransportConnectionAttachOwner(hConn,hOwner,pEvHandlers,sizeofEvHandlers);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionAttachOwner - Conn: 0x%p - SipTransportConnectionAttachOwner() failed (rv=%d:%s)",
            pConn,rv,SipCommonStatus2Str (rv)));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    RV_UNUSED_ARG(sizeofEvHandlers);

    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionDetachOwner
 * ------------------------------------------------------------------------
 * General: detach an owner from a connection.
 *          If the connection is left with no other owners it will be closed.
 *          If the same owner attached more then once the first matching owner
 *          will be removed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection.
 *          hOwner - The owner handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionDetachOwner(
            IN  RvSipTransportConnectionHandle            hConn,
            IN  RvSipTransportConnectionOwnerHandle       hOwner)
{

    RvStatus             rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;

    if(hConn==NULL || hOwner==NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;


    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionDetachOwner - Detaching owner 0x%p from connection 0x%p",hOwner,pConn));

    rv = TransportConnectionDetachOwner(pConn,hOwner);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "RvSipTransportConnectionDetachOwner - Failed Detach owner 0x%p from connection 0x%p",hOwner,pConn))
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}
/***************************************************************************
 * RvSipTransportConnectionTerminate
 * ------------------------------------------------------------------------
 * General: Closes the connection.
 *          Note: If there are still messages waiting on this
 *                connection they will be lost.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTerminate(
            IN  RvSipTransportConnectionHandle            hConn)
{

    RvStatus             rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;

    if(hConn==NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTerminate - Conn: 0x%p - can't terminate multi-server connection. Close local address instead",
            pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionTerminate - Start termination proceedure for connection 0x%p",pConn));

    if (RVSIP_TRANSPORT_CONN_STATE_IDLE == pConn->eState)
    {
        rv = TransportConnectionTerminate(pConn,RVSIP_TRANSPORT_CONN_REASON_UNDEFINED);
    }
    else
    {
        rv = TransportConnectionDisconnect(pConn);
    }
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "RvSipTransportConnectionTerminate - Failed for connection 0x%p",pConn))
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionGetCurrentState
 * ------------------------------------------------------------------------
 * General: retrieves the connection current state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  peState    - The connection current state
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetCurrentState(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionState  *peState)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL || peState == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *peState = RVSIP_TRANSPORT_CONN_STATE_UNDEFINED;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    *peState = pConn->eState;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetCurrentState - connection 0x%p - state = %s",pConn,
              SipCommonConvertEnumToStrConnectionState(pConn->eState)));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionIsTcpClient
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED.
 *          Use RvSipTransportConnectionGetType instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  peState    - The connection current state
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionIsTcpClient(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvBool                         *pbIsClient)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL || pbIsClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    switch (pConn->eConnectionType)
    {
    case RVSIP_TRANSPORT_CONN_TYPE_CLIENT:
        *pbIsClient = RV_TRUE;
        break;
    case RVSIP_TRANSPORT_CONN_TYPE_SERVER:
    case RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER:
        *pbIsClient = RV_FALSE;
        break;
    default:
        rv = RV_ERROR_UNKNOWN;
        break;
    }

    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionIsTcpClient - connection 0x%p: failed to identify connection side",pConn));
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionIsTcpClient - connection 0x%p: returning is client=%d",pConn, *pbIsClient));
    }

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
}

/***************************************************************************
 * RvSipTransportConnectionGetType
 * ------------------------------------------------------------------------
 * General: Returns the type of the connection (Client, Server, Mutliserver).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn       - Handle to the connection.
 * Output:  peConnectionType - type of the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetType(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionType    *peConnectionType)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    RvStatus              rv;

    if(hConn == NULL || peConnectionType == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peConnectionType = pConn->eConnectionType;

    TransportConnectionUnLockAPI(pConn);
    return rv;
}

/***************************************************************************
 * RvSipTransportConnectionGetNumOfOwners
 * ------------------------------------------------------------------------
 * General: retrieves the number of connection owners.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  pNumOfOwners    - Number of connection owners.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetNumOfOwners(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvInt32                *pNumOfOwners)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL || pNumOfOwners == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *pNumOfOwners = 0;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetNumOfOwners - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    *pNumOfOwners = pConn->numOfOwners;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetNumOfOwners - connection 0x%p - num of owners = %d",pConn,*pNumOfOwners));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionGetTransportType
 * ------------------------------------------------------------------------
 * General: retrieves the connection transport (TCP/TLS)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  peTransport - The connection transport
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetTransportType(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvSipTransport          *peTransport)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL || peTransport == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    *peTransport = pConn->eTransport;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetTransportType - connection 0x%p - Transport=%s",
               pConn, SipCommonConvertEnumToStrTransportType(*peTransport)));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}


/***************************************************************************
 * RvSipTransportConnectionEnable
 * ------------------------------------------------------------------------
 * General: Inserts a connection object to the hash so that persistent objects
 *          will be able to use it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionEnable(
                                        IN  RvSipTransportConnectionHandle hConn)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionEnable - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    switch (pConn->eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_CONNECTING:
    case RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED:
    case RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED:
        break;
    default:
         RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnable - Failed for connection 0x%p - can't insert connection to the hash at state %s",
              pConn,SipCommonConvertEnumToStrConnectionState(pConn->eState)));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
         return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pConn->pHashElement != NULL)
    {
         RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnable - connection 0x%p - already enabled",
              hConn));

         TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
         return RV_OK;
    }
    /*insert connection to the hash*/
    rv = TransportConnectionHashInsert(hConn, RV_FALSE);
    if(rv != RV_OK)
    {
         RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnable - connection 0x%p - Failed to insert connection to hash (rv=%d)",
              pConn,rv));

            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
         return rv;
    }
    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnable - connection 0x%p - enabled - connection was inserted connection to hash",
              pConn));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionDisable
 * ------------------------------------------------------------------------
 * General: Removes a connection object from the hash so that persistent objects
 *          will not be able to use it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionDisable(
                                        IN  RvSipTransportConnectionHandle hConn)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionDisable - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pConn->pHashElement == NULL && pConn->pHashAliasElement == NULL)
    {
         RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionDisable - connection 0x%p - already disabled",
              hConn));

         TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
         return RV_OK;
    }
    /*remove connection to the hash*/
    TransportConnectionHashRemove(pConn);
    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionDisable - connection 0x%p - disabled - connection was removed from hash",
              pConn));

       TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionIsEnabled
 * ------------------------------------------------------------------------
 * General: Returns whether the connection is enabled (in the hash) or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output   pbIsEnabled  - RV_TRUE if the connection is enabled. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionIsEnabled(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvBool             *pbIsEnabled)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL || pbIsEnabled == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionIsEnabled - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pConn->pHashElement == NULL)
    {
         RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionIsEnabled - connection 0x%p - is disabled",pConn));
         *pbIsEnabled = RV_FALSE;
    }
    else
    {
         RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionIsEnabled - connection 0x%p - is enabled", pConn));
         *pbIsEnabled = RV_TRUE;
    }

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: retrieves local address from a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *            hConn       - the connection handle that its address we seek
 * Output:  pAddress    - A previously allocated buffer to where the local address
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - a pointer to the variable to hold the port
 *          peAddressType - ipv6/ipv4
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetLocalAddress(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *pPort,
#if defined(UPDATED_BY_SPIRENT)
                                        OUT RvChar             *if_name,
					OUT RvUint16           *vdevblock,
#endif
                                        OUT RvSipTransportAddressType *peAddressType)
{
    TransportMgr*        pTransportMgr;
    TransportConnection* pConn          = (TransportConnection*)hConn;
    RvStatus            rv             = RV_OK;

    if (NULL == pConn || NULL == strAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "RvSipTransportConnectionGetLocalAddress - Getting local address for connection 0x%p",pConn));

    rv =  TransportConnectionGetAddress(pConn,RV_FALSE,strAddress,pPort,
#if defined(UPDATED_BY_SPIRENT)
					if_name,
					vdevblock,
#endif
             peAddressType);

    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionGetLocalAddress - Failed to get local address for connection 0x%p (rv=%d)",pConn, rv));
    }
       TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
}

/***************************************************************************
 * RvSipTransportConnectionGetRemoteAddress
 * ------------------------------------------------------------------------
 * General: retrieves remote address from a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *            hConn       - the connection handle that its address we seek
 * Output:  srtAddress    - A previously allocated space for the address.
 *                        4 bytes for IPv4 addresses 16 bytes for IPv6 (or addresses that
 *                        might be IPv6).
 *          pPort       - a pointer to the variable to hold the port
 *          peAddressType - ipv6/ipv4
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetRemoteAddress(
                                IN  RvSipTransportConnectionHandle hConn,
                                OUT RvChar             *strAddress,
                                OUT RvUint16           *pPort,
                                OUT RvSipTransportAddressType *peAddressType)
{
    TransportMgr*        pTransportMgr;
    TransportConnection* pConn          = (TransportConnection*)hConn;
    RvStatus            rv             = RV_OK;
#if defined(UPDATED_BY_SPIRENT)
    RvChar bogus_if_name[RVSIP_TRANSPORT_IFNAME_SIZE]="";
    RvUint16 bogus_vdevblock=0;
#endif

    if (NULL == pConn || NULL == strAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetRemoteAddress - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetRemoteAddress - Getting remote address for connection 0x%p",pConn));

    rv =  TransportConnectionGetAddress(pConn,RV_TRUE,strAddress,pPort,
#if defined(UPDATED_BY_SPIRENT)
					bogus_if_name,
					&bogus_vdevblock,
#endif
         peAddressType);

    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionGetRemoteAddress - Failed to get remote address for connection 0x%p (rv=%d)",pConn, rv));
    }
       TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
}

/***************************************************************************
 * RvSipTransportTlsEngineConstruct
 * ------------------------------------------------------------------------
 * General: Constructs a TLS engine.
 *          Tls Engine is an entity that holds together a number of characteristics
 *          related to TLS sessions. When making a TLS handshake you have to
 *          provide an engine. The handshake parameters will be derived from the
 *          engines parameters.
 *          For example, you can create a "TLS client" engine by calling
 *          RvSipTransportTlsEngineAddTrastedCA() after an engine has been
 *          constructed.
 *          Once an engine has been constructed it can be used to preform TLS
 *          handshakes. A handshake that uses an engine will 'inherit' it TLS
 *          characteristics, (e.g. TLS version)
 *
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          pTlsEngineCfg - a pointer to a configuration struct the holds data
 *          for the TLS engine.
 *          sizeofCfg - The size of the configuration structure
 * Output:  phTlsEngine     - a newly creates Tls engine.
 ***************************************************************************/
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineConstruct(
                        IN RvBool                           reqClientCert,
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineCfg*      pTlsEngineCfg,
                        IN RvInt32                         sizeofCfg,
                        OUT RvSipTransportTlsEngineHandle*  phTlsEngine)
#else
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineConstruct(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineCfg*      pTlsEngineCfg,
                        IN RvInt32                         sizeofCfg,
                        OUT RvSipTransportTlsEngineHandle*  phTlsEngine)

#endif
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv = RV_OK;
    RvPrivKeyType       eCoreTlsKeyType;
    RvTLSMethod         eCoreTlsMethod;
    SipTlsEngine        *psipTlsEngine = NULL;
#endif
    TransportMgr* pTransportMgr = (TransportMgr *)hTransportMgr;

    if (NULL == pTransportMgr)
    {
        return RV_ERROR_BADPARAM;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (NULL == pTransportMgr || NULL == pTlsEngineCfg || NULL == phTlsEngine)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineConstruct - starting to construct Tls Engine"));
    *phTlsEngine = NULL;

    if (NULL == pTransportMgr->tlsMgr.hraEngines)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - No TLS engines were allocated"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_UNDEFINED != pTlsEngineCfg->ePrivateKeyType)
    {
        ConvertSipTLSKeyToCoreTlsKey(pTlsEngineCfg->ePrivateKeyType, &eCoreTlsKeyType);
    }
    else
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - ePrivateKeyType is undefined"));
        return RV_ERROR_BADPARAM;
    }
    ConvertSipTlsMethodToCoreTLSMethod(pTlsEngineCfg->eTlsMethod,&eCoreTlsMethod);

    rv = RA_Alloc(pTransportMgr->tlsMgr.hraEngines,(RA_Element *)&psipTlsEngine);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - Tls Engine failed to get space from RA"));
        return rv;
    }

    rv = RvMutexConstruct(pTransportMgr->pLogMgr,&psipTlsEngine->engineLock);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - Tls Engine failed to construct mutex"));
        RA_DeAlloc(pTransportMgr->tlsMgr.hraEngines, (RA_Element)psipTlsEngine);
        return CRV2RV(rv);
    }

#if defined(UPDATED_BY_SPIRENT)
    rv = RvTLSEngineConstruct(
                              reqClientCert,
                              eCoreTlsMethod,
                              pTlsEngineCfg->strPrivateKey,
                              eCoreTlsKeyType,
                              pTlsEngineCfg->privateKeyLen,
                              pTlsEngineCfg->strCert,
                              pTlsEngineCfg->certLen,
                              pTlsEngineCfg->certDepth,
                              &psipTlsEngine->engineLock,
                              pTransportMgr->pLogMgr,
                              &psipTlsEngine->tlsEngine);
#else
    rv = RvTLSEngineConstruct(
                              eCoreTlsMethod,
                              pTlsEngineCfg->strPrivateKey,
                              eCoreTlsKeyType,
                              pTlsEngineCfg->privateKeyLen,
                              pTlsEngineCfg->strCert,
                              pTlsEngineCfg->certLen,
                              pTlsEngineCfg->certDepth,
                              &psipTlsEngine->engineLock,
                              pTransportMgr->pLogMgr,
                              &psipTlsEngine->tlsEngine);
#endif    

    if (RV_OK != rv)
    {
        RvMutexDestruct(&psipTlsEngine->engineLock,pTransportMgr->pLogMgr);
        RA_DeAlloc(pTransportMgr->tlsMgr.hraEngines,(RA_Element)psipTlsEngine);
        *phTlsEngine = NULL;

        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - Tls Engine failed to construct"));
        return CRV2RV(rv);
    }
    *phTlsEngine = (RvSipTransportTlsEngineHandle)psipTlsEngine;
    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc,
              "RvSipTransportTlsEngineConstruct - Tls Engine constructed successfully (eng=0x%p)", psipTlsEngine));
    RV_UNUSED_ARG(sizeofCfg);

    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    if (NULL != phTlsEngine)
    {
        *phTlsEngine = NULL;
    }

    RV_UNUSED_ARG(pTlsEngineCfg);
    RV_UNUSED_ARG(sizeofCfg);

	RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineConstruct - Tls is NOT supported"));

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportConnectionGetCurrentTlsState
 * ------------------------------------------------------------------------
 * General: retrieves the connection current TLS state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  peState    - The connection current TLS state
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetCurrentTlsState(
                        IN RvSipTransportConnectionHandle     hConn,
                        OUT RvSipTransportConnectionTlsState *peState)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv    = RV_OK;

    if(hConn == NULL || peState == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *peState = RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    *peState = pConn->eTlsState;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetCurrentTlsState - connection 0x%p - tlsState = %s",pConn,
              TransportConnectionGetTlsStateName(pConn->eTlsState)));


       TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    if (NULL != peState)
    {
        *peState = RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED;
    }
    RV_UNUSED_ARG(hConn);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportTlsEngineAddCertificateToChain
 * ------------------------------------------------------------------------
 * General: Adds a TLS certificate to chain of certificate. The engine holds
 *          a chain of certificates needed for its approval (usually ending
 *          with a self signed certificate).
 *          The engine will display the chain of certificates during handshakes
 *          in which it is required to present certificates.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 *          strCert       - The certificate encoded as ASN.1 string representation .
 *          certLen       - The length of the certificate
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineAddCertificateToChain(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        IN RvChar                          *strCert,
                        IN RvInt32                          certLen)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportMgr       *pTransportMgr  = (TransportMgr *)hTransportMgr;
    RvStatus           rv              = RV_OK;
    SipTlsEngine*      pSipEngine     = (SipTlsEngine*)hTlsEngine;

    if (NULL == pTransportMgr || NULL == hTlsEngine || NULL == strCert)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineAddCertificateToChain - Engine 0x%p: starting to add a certificate to engine",hTlsEngine));

    rv = RvTLSEngineAddCertificate(&pSipEngine->tlsEngine,
                                   strCert,
                                   certLen,
                                   &pSipEngine->engineLock,
                                   pTransportMgr->pLogMgr);

    if (RV_OK != rv)
    {

        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineAddCertificateToChain - Engine: 0x%p - failed to add certificate",hTlsEngine));
        return CRV2RV(rv);
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineAddCertificateToChain - Engine 0x%p: certificate added successfully",hTlsEngine));
    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hTlsEngine);
    RV_UNUSED_ARG(strCert);
    RV_UNUSED_ARG(certLen);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

}

/***************************************************************************
 * RvSipTransportTlsEngineAddTrustedCA
 * ------------------------------------------------------------------------
 * General: Adds a trusted certificate authority to an engine.
 *          After that function is used the engine will approve all certificates
 *          issued by the CA.
 *          A CA (Certificate Authority) is an entity that issues certificates.
 *          Most TLS clients on the net trust one or more CAs and approve only
 *          certificates that were issued by those CAs. After adding a trusted CA
 *          to an engine you can use it as a "TLS client" engine and use that
 *          connection on handshakes in which you request the other side of the
 *          connection to display it certificates.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 *          strCert       - The certificate encoded as ASN.1 string representation .
 *          certLen       - The length of the certificate
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineAddTrustedCA(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        IN RvChar                          *strCert,
                        IN RvInt32                          certLen)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportMgr       *pTransportMgr    = (TransportMgr *)hTransportMgr;
    RvStatus           rv              = RV_OK;
    SipTlsEngine*       pSipEngine     = (SipTlsEngine*)hTlsEngine;

    if (NULL == pTransportMgr || NULL == hTlsEngine || NULL == strCert)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineAddTrustedCA - Engine 0x%p: starting to add a trusted CA to engine",hTlsEngine));

    rv = RvTLSEngineAddAutorityCertificate(&pSipEngine->tlsEngine,
                                   strCert,
                                   certLen,
                                   &pSipEngine->engineLock,
                                   pTransportMgr->pLogMgr);

    if (RV_OK != rv)
    {

        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineAddTrustedCA - Engine: 0x%p - failed to add CA",hTlsEngine));
        return CRV2RV(rv);
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineAddTrustedCA - Engine 0x%p: CA added successfully",hTlsEngine));
    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hTlsEngine);
    RV_UNUSED_ARG(strCert);
    RV_UNUSED_ARG(certLen);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

}

/***************************************************************************
 * RvSipTransportTlsEngineGetUnderlyingCtx
 * ------------------------------------------------------------------------
 * General: gets the TLS engine's underlying SSL_CTX* pointer. that pointer 
 *          can be used to aplly direct OpenSSL API on the ctx.
 *          Once an engine has been exposed, the application can set 
 *          different settings to it, but it is not allowed to interfere 
 *          with it's I/O operation's (e.g. accept a session with it)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 * Output:  pUnderlyingCTX - the exposed SSL_CTX*
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineGetUnderlyingCtx(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        OUT void**                          pUnderlyingCTX)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportMgr*       pTransportMgr   = (TransportMgr *)hTransportMgr;
    RvStatus            crv              = RV_OK;
    SipTlsEngine*       pSipEngine      = (SipTlsEngine*)hTlsEngine;
    SSL_CTX*            pCtx            = NULL;
    
    if (NULL == pTransportMgr || NULL == hTlsEngine || NULL == pUnderlyingCTX)
    {
        return RV_ERROR_NULLPTR;
    }
    

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineGetUnderlyingCtx - Engine 0x%p: exposing engine",hTlsEngine));


    crv = RvTLSEngineExpose(&pSipEngine->tlsEngine,
                           &pSipEngine->engineLock,
                           pTransportMgr->pLogMgr,
                           &pCtx);

    if (RV_OK != crv)
    {

        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineGetUnderlyingCtx - Engine: 0x%p - could not expose engine",hTlsEngine));
        return CRV2RV(crv);
    }
    *pUnderlyingCTX = pCtx;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineGetUnderlyingCtx - Engine 0x%p: exposed as 0x%p",hTlsEngine,*pUnderlyingCTX));
    return crv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hTransportMgr)
    RV_UNUSED_ARG(hTlsEngine)
    RV_UNUSED_ARG(pUnderlyingCTX)
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

}

/***************************************************************************
 * RvSipTransportTlsEngineCheckPrivateKey
 * ------------------------------------------------------------------------
 * General: Checks the consistency of a private key with the corresponding
 *          certificate loaded into the engine. If more than one key/certificate pair
 *          (RSA/DSA) is installed, the last item installed will be checked. If e.g.
 *          the last item was a RSA certificate or key, the RSA key/certificate
 *          pair will be checked.
 *          This is a utility function for the application to make sure the key and
 *          certificate were loaded correctly into the engine.
 * Return Value: RV_OK - the key and certificate match
 *               RV_ERROR_UNKNOWN - the key and certificate does not match
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A Tls engine.
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineCheckPrivateKey(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine
                        )
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportMgr       *pTransportMgr  = (TransportMgr *)hTransportMgr;
    RvStatus           rv              = RV_OK;
    SipTlsEngine*       pSipEngine     = (SipTlsEngine*)hTlsEngine;

    if (NULL == pTransportMgr || NULL == hTlsEngine)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportTlsEngineCheckPrivateKey - Engine 0x%p: checking key<->certificate match",hTlsEngine));


    rv = RvTLSEngineCheckPrivateKey(&pSipEngine->tlsEngine,
                                   &pSipEngine->engineLock,
                                   pTransportMgr->pLogMgr);

    if (RV_OK != rv)
    {

        RvLogWarning(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineCheckPrivateKey - Engine: 0x%p - key and certificate does not match",hTlsEngine));
        return CRV2RV(rv);
    }
    else
    {
        RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsEngineCheckPrivateKey - Engine: 0x%p - key and certificate match",hTlsEngine));
    }
    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hTlsEngine);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportConnectionTlsHandshake
 * ------------------------------------------------------------------------
 * General: starts TLS negotiation on a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start the handshake
 *        hEngine     - The TLS engine that will be associated with the connection.
 *                    The connection will "inherit" the engine's parameters.
 *        eHandshakeSide - the TLS handshake side that the
 *                    connection will play on the TLS handshake. Using
 *                    the default enumeration will set the handshake side to be
 *                    client for TCP clients and Server for TCP servers
 *        pfnVerifyCertEvHandler - callback to check certificates that arrived
 *                    during handshake.
 *                    Client handshake side: NULL means use the default callback, valid
 *                                           certificates will be approved and invalid
 *                                           certificates will be rejected, causing a handshake
 *                                           failure.
 *                                           Using a function here overrides that default.
 *                    Server handshake side: NULL means no client certificates.
 *                                           Using a function here will require
 *                                           client certificate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsHandshake(
                 IN RvSipTransportConnectionHandle         hConnection,
                 IN RvSipTransportTlsEngineHandle          hEngine,
                 IN RvSipTransportTlsHandshakeSide         eHandshakeSide,
                 IN RvSipTransportVerifyCertificateEv      pfnVerifyCertEvHandler)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus               rv          = RV_OK;
    TransportConnection    *pConn = (TransportConnection *)hConnection;

    if (NULL == pConn || NULL == hEngine)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsHandshake - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsHandshake - hConnection=0x%p. can only perform handshake on a TLS connection",
                  hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionTlsHandshake - hConnection=0x%p,hEngine=0x%p,eHandshakeSide=%d,pfnVerifyCertEvHandler=0x%p starting handshake procedure",
              hConnection,hEngine,eHandshakeSide,pfnVerifyCertEvHandler));

    if (RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY != pConn->eTlsState)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsHandshake - TLS handshake can only start in state handshake ready state=%s (rv=%d)",
            TransportConnectionGetTlsStateName(pConn->eTlsState), 
            RV_ERROR_ILLEGAL_ACTION));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if (TRANSPORT_TLS_HANDSHAKE_IDLE != pConn->eTlsHandShakeState)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsHandshake - TLS handshake han already started (rv=%d)", RV_ERROR_ILLEGAL_ACTION));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* allowing the application to over ride tls hand shake side */
    if (RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_DEFAULT != eHandshakeSide)
    {
        pConn->eTlsHandshakeSide = eHandshakeSide;
    }

    /* setting callback by the following rools:
       Client: NULL means default, if there is a function -> set it as callback
       Server: NULL means no CB, if there is a function -> set it as callback
    */
    if (NULL == pfnVerifyCertEvHandler &&
        RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_CLIENT == pConn->eTlsHandshakeSide)
    {
        pConn->pfnVerifyCertEvHandler = TransportTlsDefaultCertificateVerificationCB;
    }
    if (NULL != pfnVerifyCertEvHandler)
    {
        pConn->pfnVerifyCertEvHandler = pfnVerifyCertEvHandler;
    }
    pConn->pTlsEngine = (SipTlsEngine*)hEngine;

    rv = TransportTLSFullHandshakeAndAllocation(pConn);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsHandshake - TLS handshake failed (rv=%d)", rv));
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hConnection);
    RV_UNUSED_ARG(hEngine);
    RV_UNUSED_ARG(eHandshakeSide);
    RV_UNUSED_ARG(pfnVerifyCertEvHandler);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

#if RV_TLS_ENABLE_RENEGOTIATION
/***************************************************************************
 * RvSipTransportConnectionTlsRenegotiate
 * ------------------------------------------------------------------------
 * General: starts TLS renegotiation on a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start renegotiation
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsRenegotiate(
                 IN RvSipTransportConnectionHandle         hConnection)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus               rv          = RV_OK;
    TransportConnection    *pConn = (TransportConnection *)hConnection;

    if (NULL == pConn)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsRenegotiate - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsRenegotiate - hConnection=0x%p. can only perform renegotiation on a TLS connection",
                  hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED != pConn->eTlsState)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsRenegotiate - TLS renegotiation is allowed in tls connected state only. state=%s (rv=%d)",
                   TransportConnectionGetTlsStateName(pConn->eTlsState), 
                   RV_ERROR_ILLEGAL_ACTION));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = TransportTlsRenegotiate(pConn);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsRenegotiate - Connection 0x%p fail renegotiation.", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;

    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "RvSipTransportConnectionTlsRenegotiate - Connection 0x%p: renegotiation was initiated",
        pConn));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hConnection);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportConnectionTlsRenegotiationInProgress
 * ------------------------------------------------------------------------
 * General: checks if the TLS connection is being renegotiated
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start renegotiation
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsRenegotiationInProgress(
                 IN RvSipTransportConnectionHandle  hConnection,
                 OUT RvBool*                        pbInProgress)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus                rv = RV_OK, crv;
    TransportConnection    *pConn = (TransportConnection *)hConnection;

    if (NULL==pConn || NULL==pbInProgress)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsRenegotiate - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsRenegotiate - hConnection=0x%p. can only perform renegotiation on a TLS connection",
                  hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED != pConn->eTlsState)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsRenegotiate - TLS renegotiation is allowed in tls connected state only. state=%s (rv=%d)",
                   TransportConnectionGetTlsStateName(pConn->eTlsState), 
                   RV_ERROR_ILLEGAL_ACTION));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    crv = RvTLSSessionRenegotiationInProgress(
                    pConn->pTlsSession, &pConn->pTripleLock->hLock,
                    pConn->pTransportMgr->pLogMgr, pbInProgress);

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return CRV2RV(crv);
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hConnection);
	RV_UNUSED_ARG(pbInProgress);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}
#endif /*RV_TLS_ENABLE_RENEGOTIATION*/

/***************************************************************************
 * RvSipTransportConnectionGetAppHandle
 * ------------------------------------------------------------------------
 * General: retrieves the connection's application handle
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn          - The connection handle
 * Output:  phAppHandle - The connection application handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetAppHandle(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionAppHandle *phAppHandle)
{
    TransportConnection*  pConn         = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr = NULL;
    RvStatus             rv            = RV_OK;

    if(hConn == NULL || phAppHandle == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    *phAppHandle = pConn->hAppHandle;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetAppHandle - connection 0x%p - returning 0x%p",pConn,
              pConn->hAppHandle));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
}

/***************************************************************************
 * RvSipTransportConnectionSetAppHandle
 * ------------------------------------------------------------------------
 * General: sets the connection's application handle
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn         - The connection handle
 *      :   hAppHandle - The connection application handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetAppHandle(
                        IN  RvSipTransportConnectionHandle  hConn,
                        IN  RvSipTransportConnectionAppHandle hAppHandle)
{
    TransportConnection*  pConn         = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr = NULL;
    RvStatus             rv            = RV_OK;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    pConn->hAppHandle = hAppHandle;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionSetAppHandle - connection 0x%p - setting app handle to 0x%p",pConn,
              pConn->hAppHandle));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
}

/******************************************************************************
 * RvSipTransportConnectionGetSecOwner
 * ----------------------------------------------------------------------------
 * General: Retrieves the security owner of the connection.
 *          The security owner is a security-object which is implemented in
 *          the RADVISION IMS SIP Security module.
 *          The IMS application should use this function when working with
 *          the Security module. The application should call this function from
 *          the RvSipTransportMsgReceivedEv() callback for a message received
 *          on a connection. This is to verify the identity of the message
 *          sender against the identity of the user with which the security
 *          owner established the protection. Calling this function from
 *          the RvSipTransportMsgReceivedEv() callback prevents further message
 *          processing in case the check fails.
 *          If the function returns NULL in the ppSecOwner parameter,this means
 *          that the message was not protected by the IMS security-object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn      - The connection handle
 * Output:  ppSecOwner - The pointer to the Security Object,
 *                       owning the Connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetSecOwner(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT void**                          ppSecOwner)
{
#ifdef RV_SIP_IMS_ON
    TransportConnection*  pConn         = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr = NULL;
    RvStatus             rv            = RV_OK;
    
    if(hConn == NULL || ppSecOwner == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    pTransportMgr = pConn->pTransportMgr;
    
    *ppSecOwner = pConn->pSecOwner;
    
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "RvSipTransportConnectionGetSecOwner - connection 0x%p - returning %p",
        pConn, pConn->pSecOwner));
    
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
#else
    RV_UNUSED_ARG(hConn);
    RV_UNUSED_ARG(ppSecOwner);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /*#ifdef RV_SIP_IMS_ON*/
}

/***************************************************************************
 * RvSipTransportConnectionTlsGetEncodedCert
 * ------------------------------------------------------------------------
 * General: Retrieves a last certificate, received from the connection peer.
 *          If the allocated buffer is insufficient, the length of the buffer
 *          needed will be inserted in pCertLen.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection - connection on to get cert of
 *          pCertLen    - The allocated cert buffer len
 * OutPut:  pCertLen    - The real size of the certificate.
 *          strCert     - an allocated buffer to hold the certificate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsGetEncodedCert(
                     IN    RvSipTransportConnectionHandle   hConnection,
                     INOUT RvInt32                         *pCertLen,
                     OUT   RvChar                          *strCert)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus               rv          = RV_OK;
    TransportConnection    *pConn       = (TransportConnection *)hConnection;
    RvInt                  realCertLen = 0;

    if (NULL == pConn || NULL == pCertLen)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - hConnection=0x%p. can only get certificate from a TLS connection",
            hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p getting encoded certificate", pConn));

    rv = RvTLSSessionGetCertificateLength(
                                          pConn->pTlsSession,
                                          &pConn->pTripleLock->hLock,
                                          pConn->pTransportMgr->pLogMgr,
                                          &realCertLen
                                          );
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p failed to get the certificate length", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return CRV2RV(rv);
    }

    if (0 == realCertLen)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p does not hold a certificate.", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (realCertLen > *pCertLen)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p insufficient buffer for certificate. (needed len=%d)", pConn, realCertLen));
        *pCertLen = realCertLen;
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    *pCertLen = realCertLen;

    rv = RvTLSSessionGetCertificate(pConn->pTlsSession,
        &pConn->pTripleLock->hLock,
        pConn->pTransportMgr->pLogMgr,
        strCert);

    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p failed to get the certificate", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return CRV2RV(rv);
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "RvSipTransportConnectionTlsGetEncodedCert - Connection 0x%p certificate retrieved", pConn));
    /* code here */
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    if (NULL != pCertLen)
    {
        *pCertLen = -1;
    }
    RV_UNUSED_ARG(hConnection);
    RV_UNUSED_ARG(strCert);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportConnectionTlsGetUnderlyingSsl
 * ------------------------------------------------------------------------
 * General: gets the TLS session's underlying SSL* pointer. that pointer 
 *          can be used to aplly direct OpenSSL API on the session.
 *          Once a session has been exposed, the application can set 
 *          different settings to it, but it is not allowed to interfere 
 *          with it's I/O operation's
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConnection - connection on to get cert of
 * OutPut:  pUnderlyingSSL    - the connection's underlying SSL session.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsGetUnderlyingSsl(
                     IN    RvSipTransportConnectionHandle   hConnection,
                     OUT   void**                           pUnderlyingSSL)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus                crv         = RV_OK;
    TransportConnection*    pConn       = (TransportConnection *)hConnection;
    SSL*                    pUnderlying = NULL;

    if (NULL == pConn || NULL == pUnderlyingSSL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransportConnectionLockAPI(pConn))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetUnderlyingSsl - Conn: 0x%p - illegal action for multi-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionTlsGetUnderlyingSsl - hConnection=0x%p. can only get underlying SSL from a TLS connection",
            hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionTlsGetUnderlyingSsl - Connection 0x%p getting underlying session", pConn));
    crv = RvTLSSessionExpose(pConn->pTlsSession,
                            &pConn->pTripleLock->hLock,
                            pConn->pTransportMgr->pLogMgr,
                            &pUnderlying);
    if (RV_OK != crv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportConnectionTlsGetUnderlyingSsl - Connection 0x%p can not expose underlying session", pConn));
        TransportConnectionUnLockAPI(pConn);
        return CRV2RV(crv);
    }
    *pUnderlyingSSL = pUnderlying;
    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionTlsGetUnderlyingSsl - Connection 0x%p exposed underlying session as 0x%p", pConn,*pUnderlyingSSL));
  
    TransportConnectionUnLockAPI(pConn); 
    return RV_OK;
#else
    RV_UNUSED_ARG(pUnderlyingSSL)
    RV_UNUSED_ARG(hConnection)
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipTransportTlsEncodeCert
 * ------------------------------------------------------------------------
 * General: Encodes a certificate to a buffer in DER(ASN.1) format.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCert - the certificate to encode
 *          pCertLen - the buffer length
 * Output:  strCert - the certificate encoded into Asn.1 format
 *       :  pCertLen    - The length of the certificate. (bytes)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEncodeCert(
                        IN    RvSipTransportTlsCertificate   hCert,
                        INOUT RvInt32                       *pCertLen,
                        OUT   RvChar                        *strCert)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvInt                  realCertLen = 0;
    RvStatus               rv          = RV_OK;

    if (NULL == hCert || NULL == pCertLen)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RvTLSGetCertificateLength((void *)hCert,&realCertLen);
    if (RV_OK != rv)
    {
        /* failed to get the cert len */
        return CRV2RV(rv);
    }

    if (realCertLen > *pCertLen)
    {
        /* insufficient buffer */
        *pCertLen = realCertLen;
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    *pCertLen = realCertLen;

    rv = RvTLSGetCertificate((void*)hCert,strCert);

    if (RV_OK != rv)
    {
        /* failure in getting the cert */
        return CRV2RV(rv);
    }

    return rv;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    if (NULL != pCertLen)
    {
        *pCertLen = -1;
    }
    RV_UNUSED_ARG(hCert);
    RV_UNUSED_ARG(strCert);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}

/***************************************************************************
 * RvSipTransportTlsGetSubjectAltDNS
 * ------------------------------------------------------------------------
 * General: The function retrieves a list of the subjectAltNames that are
 *          in a DNS format, from the TLS certificate.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection - The TLS connection.
 *          pBuffer    - A buffer allocated by application, to fill with the 
 *                       function output subjectAltNames list.
 *          pBufferLen - Pointer to an integer containing the size of buffer. 
 * Output:  pBuffer    - The buffer containing a zero-separated list of DNS names
 *          pBufferLen - Contains the requested buffer size (The real size that 
 *                       was used for the list - if there were enough resources,
 *                       or the expected buffer size - in case of out of resource case)
 *          NumOfNamesInList - Pointer to an integer containing the number of
 *                       SubjAltNames in the output list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsGetSubjectAltDNS(
                        IN    RvSipTransportConnectionHandle   hConnection,
                        INOUT RvChar*                          pBuffer,
                        INOUT RvInt32*                         pBufferLen,
                        OUT   RvInt32*                         pNumOfNamesInList)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus                crv         = RV_OK;
    TransportConnection*    pConn       = (TransportConnection *)hConnection;

    if (NULL == pConn || NULL == pBuffer || NULL == pBufferLen || NULL == pNumOfNamesInList)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransportConnectionLockAPI(pConn))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    
    if (RVSIP_TRANSPORT_TLS != pConn->eTransport)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportTlsGetSubjectAltDNS - hConnection=0x%p. can only get SubjAltName from a TLS connection",
            hConnection));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "RvSipTransportTlsGetSubjectAltDNS - Connection 0x%p getting SubjAltName", pConn));

    crv = RvTLSSessionGetSubjectAltDNS(pConn->pTlsSession, 
                                       pBuffer, 
                                       (RvSize_t*)pBufferLen, 
                                       (RvSize_t*)pNumOfNamesInList, 
                                       pConn->pTransportMgr->pLogMgr);
    if (RV_OK != crv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "RvSipTransportTlsGetSubjectAltDNS - Connection 0x%p Failed to get SubjectAltDNS", pConn));
        TransportConnectionUnLockAPI(pConn);
        return CRV2RV(crv);
    }

    
    TransportConnectionUnLockAPI(pConn); 
    return RV_OK;
#else
    RV_UNUSED_ARG(pNumOfNamesInList)
    RV_UNUSED_ARG(pBufferLen)
    RV_UNUSED_ARG(pBuffer)
    RV_UNUSED_ARG(hConnection)
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}
/***************************************************************************
 * RvSipTransportTlsGetCertVerificationError
 * ------------------------------------------------------------------------
 * General: retrieves an error string in the the verification callback
 * Return Value: the error string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCert - the handle of the certificate
 * Output:  strError - the error string
 ***************************************************************************/
RVAPI RvChar* RVCALLCONV RvSipTransportTlsGetCertVerificationError(
                           IN    RvSipTransportTlsCertificate   hCert,
                           OUT   RvChar                       **strError)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvTLSGetCertificateVerificationError((void*)hCert,strError);
    return *strError;
#else  /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    RV_UNUSED_ARG(hCert);
    RV_UNUSED_ARG(strError);

    return "Undefined";
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
}


/***************************************************************************
 * RvSipTransportSendObjectEvent
 * ------------------------------------------------------------------------
 * General: Sends an event through the event queue.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr - The transport manager handle.
 *          pObj          - Pointer to the object to be terminated.
 *          pEventInfo    - Pointer to an allocated uninitialised structure
 *                          used for queueing object events.
 *          reason        - event reason
 *          func          - event callback function - this function will be
 *                          called when the event will be poped
 *                          from the event queue.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportSendObjectEvent(
                                    IN RvSipTransportMgrHandle          hTransportMgr,
                                    IN void*                            pObj,
                                    IN RvSipTransportObjEventInfo*      pEventInfo,
                                    IN RvInt32                          reason,
                                    IN RvSipTransportObjectEventHandler func)
{
    TransportMgr       *pTransportMgr = (TransportMgr *)hTransportMgr;


    if (NULL == pTransportMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportSendObjectEvent - event on object 0x%p is sent to events queue",pObj));
#define APP_OBJ_LOG_STR "App Event"
#define APP_OBJ_NAME_STR "App Obj"

    return SipTransportSendTerminationObjectEvent(hTransportMgr, pObj, pEventInfo, reason, func,
                                       RV_FALSE,APP_OBJ_LOG_STR,APP_OBJ_NAME_STR);
}


/*-----------------------------------------------------------------------*/
/*                           DNS FUNCTIONS                               */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipTransportDNSListGetUsedHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host name element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - last used host name element
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListGetUsedHostElement(
    IN  RvSipTransportMgrHandle                pTransportMgr,
    IN  RvSipTransportDNSListHandle            pDnsList,
    OUT RvSipTransportDNSHostNameElement      *pHostElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
     return TransportDNSListGetUsedHostElement((TransportMgr *)pTransportMgr,
         (TransportDNSList *)pDnsList,pHostElement);
#else
     RV_UNUSED_ARG(pTransportMgr);
     RV_UNUSED_ARG(pDnsList);
     RV_UNUSED_ARG(pHostElement);
     return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipTransportDNSListSetUsedSRVElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListSetUsedSRVElement(
      IN  RvSipTransportMgrHandle      hTransportMgr,
      IN  RvSipTransportDNSListHandle  hDnsList,
      IN RvSipTransportDNSSRVElement*  pSRVElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    return TransportDNSListSetUsedSRVElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,pSRVElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pSRVElement);
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}


/***************************************************************************
 * RvSipTransportDNSListSetUsedHostElement
 * ------------------------------------------------------------------------
 * General: sets host name element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListSetUsedHostElement(
      IN  RvSipTransportMgrHandle          hTransportMgr,
      IN  RvSipTransportDNSListHandle      hDnsList,
      IN RvSipTransportDNSHostNameElement* pHostElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    return TransportDNSListSetUsedHostElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,pHostElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pHostElement);
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}



/***************************************************************************
 * RvSipTransportDNSListGetUsedSRVElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element, used to produce the host name list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pSRVElement     - last used SRV element
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListGetUsedSRVElement(
    IN  RvSipTransportMgrHandle                pTransportMgr,
    IN  RvSipTransportDNSListHandle            pDnsList,
    OUT RvSipTransportDNSSRVElement            *pSRVElement)
 {
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
     return TransportDNSListGetUsedSRVElement((TransportMgr *)pTransportMgr,
         (TransportDNSList *)pDnsList,pSRVElement);
#else
     RV_UNUSED_ARG(pTransportMgr);
     RV_UNUSED_ARG(pDnsList);
     RV_UNUSED_ARG(pSRVElement);
     return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipTransportDNSListGetSrvElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element from the SRV list of the DNS list object
 * according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN, RV_ERROR_INVALID_HANDLE or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *             pRelative        - relative SRV element. Used when location
 *                            is 'next' or 'previous'
 * Output:  pSrvElement     - found element
 *            pRelative        - new relative SRV element for get consequent
 *                            RvSipTransportDNSListGetSrvElement function
 *                            calls.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListGetSrvElement (
                                           IN     RvSipTransportMgrHandle           hTransportMgr,
                                           IN     RvSipTransportDNSListHandle        hDnsList,
                                           IN     RvSipListLocation                    location,
                                           INOUT  void                                **pRelative,
                                           OUT    RvSipTransportDNSSRVElement        *pSrvElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pRelative || NULL == pSrvElement ||
        NULL == ((TransportDNSList*)hDnsList)->hSrvNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListGetSrvElement((TransportMgr*)hTransportMgr,
                                         (TransportDNSList*)hDnsList,
                                         location,
                                         pRelative,
                                         pSrvElement);
#else
    if (pRelative != NULL)
    {
        *pRelative   = NULL;
    }
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(location);
    RV_UNUSED_ARG(pSrvElement);

    return RV_ERROR_UNKNOWN;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

}

/***************************************************************************
 * RvSipTransportDNSListConstruct
 * ------------------------------------------------------------------------
 * General: The function allocates and fills the TransportDNSList structure 
 *          and returns handle to the TransportDNSList object,
 *          and appropriate error code or RV_OK.
 *          Receives as input memory pool, page and list pool where lists
 *          element and the Transport DNS List object itself will be allocated.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to the transport manager
 *          hMemPool        - Handle of the memory pool.
 *          hMemPage        - Handle of the memory page
 *          hListsPool      - Handle of the list pool
 *          maxElementsInSingleDnsList - maximum num. of elements in a single list. *                            
 * Output:  phDnsList       - Handle to a DNS list object. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListConstruct (
                           IN  RvSipTransportMgrHandle      hTransportMgr,
                           IN  HRPOOL                       hMemPool,
                           IN  RvUint32                     maxElementsInSingleDnsList,
                           OUT RvSipTransportDNSListHandle  *phDnsList)
{
    HPAGE tempPage = NULL;
    RvStatus rv = RV_OK;

    if (NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == phDnsList)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = RPOOL_GetPage(hMemPool, 0, &tempPage);
    if(rv != RV_OK)
    {
        RvLogError(((TransportMgr*)hTransportMgr)->pLogSrc ,(((TransportMgr*)hTransportMgr)->pLogSrc ,
            "RvSipTransportDNSListConstruct - failed to allocate a new page (rv=%d)", rv));
        return rv;
    }

    rv = TransportDNSListConstruct(hMemPool, tempPage, maxElementsInSingleDnsList, 
                                    RV_TRUE, (TransportDNSList**)phDnsList);
    if(rv != RV_OK)
    {
        RPOOL_FreePage(hMemPool, tempPage);
        RvLogError(((TransportMgr*)hTransportMgr)->pLogSrc ,(((TransportMgr*)hTransportMgr)->pLogSrc ,
            "RvSipTransportDNSListConstruct - failed to construct the list (rv=%d)", rv));
        return rv;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipTransportDNSListDestruct
 * ------------------------------------------------------------------------
 * General: The function destructs the TransportDNSList structure.
 *          Note - Use this function only if you used RvSipTransportDNSListConstruct
 *          to construct the list
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to the transport manager
 *          hDnsList       - Handle to a DNS list object. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListDestruct (
                           IN  RvSipTransportMgrHandle      hTransportMgr,
                           IN  RvSipTransportDNSListHandle  hDnsList)
{
    TransportDNSList* pList = (TransportDNSList*)hDnsList;
    TransportMgr*     pMgr = (TransportMgr*)hTransportMgr;

    
    if (NULL == pMgr || NULL == hDnsList)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(RV_FALSE == pList->bAppList)
    {
        RvLogExcep(pMgr->pLogSrc, (pMgr->pLogSrc,
            "RvSipTransportDNSListDestruct: can not destruct a non-application list 0x%p", hDnsList));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    RPOOL_FreePage(pList->hMemPool, pList->hMemPage);
    RvLogDebug(pMgr->pLogSrc, (pMgr->pLogSrc,
        "RvSipTransportDNSListDestruct: dns-list 0x%p was destructed", hDnsList));
        
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportDNSListGetHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host element from the host elements list of the DNS
 * list object according to specified by input location.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *            pRelative        - relative host name element for 'next' or 'previous'
 *                            locations
 * Output:  pHostElement    - found element
 *            pRelative        - new relative host name element for consequent
 *            call to the RvSipTransportDNSListGetHostElement function.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListGetHostElement (
                                           IN  RvSipTransportMgrHandle          hTransportMgr,
                                           IN  RvSipTransportDNSListHandle        hDnsList,
                                           IN  RvSipListLocation                location,
                                           INOUT  void                            **pRelative,
                                           OUT RvSipTransportDNSHostNameElement    *pHostElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pRelative || NULL == pHostElement ||
        NULL == ((TransportDNSList*)hDnsList)->hHostNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListGetHostElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,location,pRelative,pHostElement);
#else
    if (pRelative != NULL)
    {
        *pRelative   = NULL;
    }
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(location);
    RV_UNUSED_ARG(pHostElement);

    return RV_ERROR_UNKNOWN;
#endif
}

/***************************************************************************
 * RvSipTransportDnsGetEnumResult
 * ------------------------------------------------------------------------
 * General: This function retrieves the result of an ENUM NAPTR query
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr  - transport mgr
 *           hDnsList       - DNS list to set record to
 * Output  : ppEnumRes       - pointer to ENUM string
 ***************************************************************************/
RVAPI RvStatus RvSipTransportDnsGetEnumResult(
				IN  RvSipTransportMgrHandle       hTransportMgr,
				IN  RvSipTransportDNSListHandle   hDnsList,
				OUT RvChar	                    **ppEnumRes)
{

	if(hTransportMgr == NULL || hDnsList == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(ppEnumRes == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDnsGetEnumResult((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,ppEnumRes);
}

/***************************************************************************
 * RvSipTransportDNSListRemoveTopmostSrvElement
 * ------------------------------------------------------------------------
 * General: removes topmost SRV element from the SRV elements list of the
 * DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListRemoveTopmostSrvElement(
                                          IN RvSipTransportMgrHandle     hTransportMgr,
                                          IN RvSipTransportDNSListHandle hDnsList)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == ((TransportDNSList*)hDnsList)->hSrvNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListRemoveTopmostSrvElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);

    return RV_ERROR_UNKNOWN;
#endif

}

/***************************************************************************
 * RvSipTransportDNSListRemoveTopmostHostElement
 * ------------------------------------------------------------------------
 * General: removes topmost host element from the head of the host elements
 * list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListRemoveTopmostHostElement(
                                        IN RvSipTransportMgrHandle     hTransportMgr,
                                        IN RvSipTransportDNSListHandle hDnsList)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == ((TransportDNSList*)hDnsList)->hHostNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListRemoveTopmostHostElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);

    return RV_ERROR_UNKNOWN;
#endif
}


/***************************************************************************
 * RvSipTransportDNSListPopSrvElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost SRV name element from the SRV
 * elements list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 * Output:  pSrvElement     - retrieved element
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPopSrvElement (
                                  IN  RvSipTransportMgrHandle       hTransportMgr,
                                  IN  RvSipTransportDNSListHandle   hDnsList,
                                  OUT RvSipTransportDNSSRVElement  *pSrvElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pSrvElement ||
        NULL == ((TransportDNSList*)hDnsList)->hSrvNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListPopSrvElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,pSrvElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pSrvElement);

    return RV_ERROR_UNKNOWN;
#endif
}

/***************************************************************************
 * RvSipTransportDNSListPopHostElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost host element from the list of host
 * elements in the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - element
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPopHostElement (
                                       IN  RvSipTransportMgrHandle           hTransportMgr,
                                       IN  RvSipTransportDNSListHandle       hDnsList,
                                       OUT RvSipTransportDNSHostNameElement *pHostElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pHostElement ||
        NULL == ((TransportDNSList*)hDnsList)->hHostNamesList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListPopHostElement((TransportMgr*)hTransportMgr, (TransportDNSList*)hDnsList,pHostElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pHostElement);

    return RV_ERROR_UNKNOWN;
#endif

}

/***************************************************************************
 * RvSipTransportDNSListPushSrvElement
 * ------------------------------------------------------------------------
 * General: adds single SRV element to the head of the SRV names list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pSrvElement        - SRV element structure to be added to the list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPushSrvElement(
                                    IN RvSipTransportMgrHandle        hTransportMgr,
                                    IN RvSipTransportDNSListHandle    hDnsList,
                                    IN RvSipTransportDNSSRVElement   *pSrvElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    TransportMgr*          pTransportMgr = (TransportMgr*)hTransportMgr;
    TransportDNSList*      pDnsList      = (TransportDNSList*)hDnsList;
    RvUint32               Count4, Count6;

    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pSrvElement)
    {
        return RV_ERROR_BADPARAM;
    }

    TransportMgrCounterLocalAddrsGet(pTransportMgr,pSrvElement->protocol,&Count4,&Count6);
    if (0 == Count4 && 0 == Count6)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportDNSListPushSrvElement - cannot push SRV element. stack holds no %s addresses (rv=%d)",
                  SipCommonConvertEnumToStrTransportType(pSrvElement->protocol),
                  RV_ERROR_ILLEGAL_ACTION));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return TransportDNSListPushSrvElement(pTransportMgr,pDnsList,pSrvElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pSrvElement);

    return RV_ERROR_UNKNOWN;
#endif

}

/***************************************************************************
 * RvSipTransportDNSListPushHostElement
 * ------------------------------------------------------------------------
 * General: adds host element to the head of the host elements list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPushHostElement(
                                              IN RvSipTransportMgrHandle          hTransportMgr,
                                              IN RvSipTransportDNSListHandle      hDnsList,
                                                IN RvSipTransportDNSHostNameElement *pHostElement)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    TransportMgr*          pTransportMgr = (TransportMgr*)hTransportMgr;
    TransportDNSList*      pDnsList      = (TransportDNSList*)hDnsList;
    RvUint32               Count4, Count6;

    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pHostElement)
    {
        return RV_ERROR_BADPARAM;
    }

    TransportMgrCounterLocalAddrsGet(pTransportMgr,pHostElement->protocol,&Count4,&Count6);
    if (0 == Count4 && 0 == Count6)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "RvSipTransportDNSListPushHostElement - cannot push host element. stack holds no %s addresses (rv=%d)",
                  SipCommonConvertEnumToStrTransportType(pHostElement->protocol),RV_ERROR_ILLEGAL_ACTION));
            return RV_ERROR_ILLEGAL_ACTION;
    }

    return TransportDNSListPushHostElement(pTransportMgr,pDnsList,pHostElement);
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pHostElement);

    return RV_ERROR_UNKNOWN;
#endif

}

/***************************************************************************
 * RvSipTransportDNSListGetIPElement
 * ------------------------------------------------------------------------
 * General: retrieves IP address element from the DNS list objects IP
 * addresses list according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *            pRelative        - relative IP element for get next/previous
 * Output:  pIPElement      - found element
 *            pRelative        - new relative IP element for consequent
 *            call to the RvSipTransportDNSListGetIPElement function.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListGetIPElement (
                                           IN     RvSipTransportMgrHandle       hTransportMgr,
                                           IN     RvSipTransportDNSListHandle    hDnsList,
                                           IN     RvSipListLocation                 location,
                                           INOUT  void                            **pRelative,
                                           OUT    RvSipTransportDNSIPElement    *pIPElement)
{
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pRelative || NULL == pIPElement ||
        NULL == ((TransportDNSList*)hDnsList)->hIPAddrList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListGetIPElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,location,pRelative,pIPElement);
}

/***************************************************************************
 * RvSipTransportDNSListRemoveTopmostIPElement
 * ------------------------------------------------------------------------
 * General: removes topmost element from the head of the DNS list
 * object IP addresses list.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListRemoveTopmostIPElement(
                                           IN RvSipTransportMgrHandle     hTransportMgr,
                                           IN RvSipTransportDNSListHandle hDnsList)
{
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == ((TransportDNSList*)hDnsList)->hIPAddrList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListRemoveTopmostIPElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList);
}


/***************************************************************************
 * RvSipTransportDNSListPopIPElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes from the IP addresses list of the DNS
 * list object the topmost IP address element.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - handle of the DNS list object
 * Output:  pIpElement      - element
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPopIPElement(
                                        IN  RvSipTransportMgrHandle     hTransportMgr,
                                        IN  RvSipTransportDNSListHandle    hDnsList,
                                        OUT RvSipTransportDNSIPElement    *pIPElement)
{
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pIPElement ||
        NULL == ((TransportDNSList*)hDnsList)->hIPAddrList)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListPopIPElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,pIPElement);
}

/***************************************************************************
 * RvSipTransportDNSListPushIPElement
 * ------------------------------------------------------------------------
 * General: adds single IP address element to the head of the IP addresses list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pIPElement        - IP address element structure to be added to the list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportDNSListPushIPElement(
                                           IN RvSipTransportMgrHandle       hTransportMgr,
                                           IN RvSipTransportDNSListHandle    hDnsList,
                                           IN RvSipTransportDNSIPElement    *pIPElement)
{
    TransportMgr*           pTransportMgr = (TransportMgr*)hTransportMgr;
    TransportDNSList*       pDnsList      = (TransportDNSList*)hDnsList;
    RvUint32               Count4, Count6;

    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pIPElement || pIPElement->protocol == RVSIP_TRANSPORT_UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportDNSListPushIPElement - pushing element to list 0x%p", pDnsList));

    TransportMgrCounterLocalAddrsGet(pTransportMgr,pIPElement->protocol,&Count4,&Count6);

    if ((0 == Count4 && pIPElement->bIsIpV6 == RV_FALSE) ||
        (0 == Count6 && pIPElement->bIsIpV6 == RV_TRUE)   )
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
            "RvSipTransportDNSListPushIPElement - Stack holds no addresses of type %s",
            SipCommonConvertEnumToStrTransportType(pIPElement->protocol)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return TransportDNSListPushIPElement(pTransportMgr,pDnsList,pIPElement);
}


/***************************************************************************
 * RvSipTransportGetNumberOfDNSListEntries
 * ------------------------------------------------------------------------
 * General: Retrieves number of elements in each of the DNS list object lists.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          hDnsList            - Handle of the DNS list object
 * Output:  pSrvElements        - number of SRV elements
 *            pHostNameElements    - number of host elements
 *            pIpAddrElements        - number of IP address elements
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetNumberOfDNSListEntries(
                                           IN  RvSipTransportMgrHandle      hTransportMgr,
                                           IN  RvSipTransportDNSListHandle  hDnsList,
                                           OUT RvUint32                    *pSrvElements,
                                           OUT RvUint32                    *pHostNameElements,
                                           OUT RvUint32                    *pIpAddrElements)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (NULL == hDnsList || NULL == hTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pSrvElements || NULL == pHostNameElements ||
        NULL == pIpAddrElements)
    {
        return RV_ERROR_BADPARAM;
    }

    return TransportDNSListGetNumberOfEntries((TransportMgr*)hTransportMgr,
                                              (TransportDNSList*)hDnsList,
                                              pSrvElements,
                                              pHostNameElements,
                                              pIpAddrElements);
#else /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(hDnsList);
    RV_UNUSED_ARG(pSrvElements);
    RV_UNUSED_ARG(pHostNameElements);
    RV_UNUSED_ARG(pIpAddrElements);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}


/***************************************************************************
 * RvSipTransportGetNumOfIPv4LocalAddresses
 * ------------------------------------------------------------------------
 * General: Returns the number of local addresses that the stack listens to.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          pNumberOfAddresses - The number of local addresses for which the
 *                              stack listens.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetNumOfIPv4LocalAddresses(
                                     IN   RvSipTransportMgrHandle    hTransportMgr,
                                     OUT  RvUint32                *pNumberOfAddresses)
{
    RvStatus       rv = RV_OK;
    TransportMgr*   pTransportMgr = (TransportMgr*)hTransportMgr;

    if (NULL == pTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pNumberOfAddresses)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetNumOfIPv4LocalAddresses((TransportMgr*)hTransportMgr,pNumberOfAddresses);

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportGetNumOfIPv4LocalAddresses - returning %d addresses", *pNumberOfAddresses));
    return rv;

}


/***************************************************************************
 * RvSipTransportGetIPv4LocalAddressByIndex
 * ------------------------------------------------------------------------
 * General: Retrieves the local address by index. Used when the stack was
 *          initialized with IPv4 local address of 0, and therefore listens
 *          on several distinct local addresses.
 *          To know how many local addresses are available by this function
 *          call RvSipTransportGetNumOfIPv4LocalAddresses. If for example this
 *          function returns 5 then you can call
 *          SipTransportGetIPv4LocalAddressByIndex with indexes going from
 *          0 to 4.
 *          Note: The IPv4 address requires 4-BYTEs of memory. This is the
 *          same as an unsigned int (RvUint32). This function requires
 *          pLocalAddr to be a pointer to a 4-BYTE allocated memory.
 *          It can also be a pointer to RvUint32 with an appropriate casting.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          index - The index for the local address to retrieve
 *          pLocalAddr  - A pointer to a 4-BYTE memory space to be filled
 *                        with the selected local address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetIPv4LocalAddressByIndex(
                                     IN   RvSipTransportMgrHandle    hTransportMgr,
                                     IN   RvUint                   index,
                                     OUT  RvUint8                  *pLocalAddr)
{
    if (NULL == hTransportMgr || NULL == pLocalAddr)
    {
        return RV_ERROR_NULLPTR;
    }
    return SipTransportGetIPv4LocalAddressByIndex((TransportMgr*)hTransportMgr,index,pLocalAddr);
}

/***************************************************************************
 * RvSipTransportGetIPv6LocalAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the local address that was actually open for listening
 *          when the stack was initiated with local address 0:0:0:0:0:0:0:0.
 *          Note: The IPv6 address requires 16-BYTEs of memory. This function
 *          requires pLocalAddr to be a pointer to a 16-BYTE allocated memory.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          pLocalAddr  - A pointer to a 16-BYTE memory space to be filled
 *                        with the selected local address.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetIPv6LocalAddress(
                              IN   RvSipTransportMgrHandle   hTransportMgr,
                              OUT  RvUint8*                  pLocalAddr)
{
#if(RV_NET_TYPE & RV_NET_IPV6)
    TransportMgr*   pTransportMgr = (TransportMgr*)hTransportMgr;
    const RvUint8*  pAddr;
    RvStatus        rv            = RV_OK;
    RvAddress       address;
    RvInt           i;
    rv = TransportMgrGetDefaultHost(pTransportMgr, RV_ADDRESS_TYPE_IPV6, &address);
    if (RV_OK != rv)
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
            "RvSipTransportGetIPv6LocalAddress - Failed to get ipv6 local node"));
    }

    pAddr = RvAddressIpv6GetIp(RvAddressGetIpv6(&address));

    if (NULL == pAddr)
    {
        for (i=0; i<16; i++)
        {
            pLocalAddr[i] = 0;
        }
        return RV_ERROR_UNKNOWN;
    }

    for (i=0; i<16; i++)
    {
        pLocalAddr[i] = pAddr[i];
    }
#else
    RV_UNUSED_ARG(hTransportMgr);
    RV_UNUSED_ARG(pLocalAddr);
#endif /* ifdef (RV_NET_TYPE & RV_NET_IPV6) */

    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transport
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClientMgr   - Handle to the register-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetStackInstance(
                                   IN  RvSipTransportConnectionHandle  hConn,
                                   OUT void                 **phStackInstance)
{
    RvStatus               rv;
    TransportConnection    *pTransportConnection;
    TransportMgr           *pTransportMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == hConn)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportConnection    = (TransportConnection *)hConn;
    pTransportMgr           = pTransportConnection->pTransportMgr;
    rv = RvSipTransportMgrGetStackInstance((RvSipTransportMgrHandle)pTransportMgr,
                                           phStackInstance);

    return rv;
}
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipTransportInjectMsg
 * ------------------------------------------------------------------------
 * General: enales application to 'inject' a meesage to the stack.
 *          the message may be given in a string or message structure handle formats.
 *          the given message will be handled as if it was received from network.
 *          in case of multihome ip - application should supply the local address,
 *          so stack will know from where to send the response. if not suppplied,
 *          stack will choose one by itself.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr  - Handle to the transport manager.
 *          pMsgBuffer     - The 'injected' message in a string format (must be ended with 2 CRLF).
 *          totalMsgLength - total length of the meesage given in pMsgBuffer.
 *          hMsg           - Handle to the 'injected' message, in a message structure format.
 *            hConn          - The connection handle that the message is 'injected' to.
 *          pAddressInfo   - Structure contains the local address and remote address.
 *                           (The remote address, is the one who will receive the response).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportInjectMsg(
                        IN  RvSipTransportMgrHandle        hTransportMgr,
                        IN  RvChar                        *pMsgBuffer,
                        IN  RvUint32                       totalMsgLength,
                        IN  RvSipMsgHandle                 hMsg,
                        IN  RvSipTransportConnectionHandle hConn,
                        IN  RvSipTransportMsgAddrCfg       *pAddressInfo)
{
    TransportMgr*        pTransportMgr  = (TransportMgr*)hTransportMgr;
    RvStatus            rv;

    if(pTransportMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(NULL == pMsgBuffer && NULL == hMsg)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportInjectMsg - inject message. str=0x%p, hMsg=0x%p, conn=0x%p",
              pMsgBuffer, hMsg,hConn));

    if(pAddressInfo != NULL)
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                   "RvSipTransportInjectMsg - addressing info for the injected msg: transport=%s,destAddr=%s:%d, localAddr=%s:%d",
                   SipCommonConvertEnumToStrTransportType(pAddressInfo->eTransportType),
                   (pAddressInfo->strDestIp == NULL)?"NULL":pAddressInfo->strDestIp,
                   pAddressInfo->destPort,
                   (pAddressInfo->strLocalIp == NULL)?"NULL":pAddressInfo->strLocalIp,
                   pAddressInfo->localPort));
    }

    rv = TransportInjectMsg(hTransportMgr, pMsgBuffer, totalMsgLength, hMsg,
                            hConn, pAddressInfo);

    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "RvSipTransportInjectMsg - Failed. status %d", rv));
        return rv;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "RvSipTransportInjectMsg - msg was injected successfuly"));
    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */

/******************************************************************************
 * RvSipTransportMgrLocalAddressAdd
 * ------------------------------------------------------------------------
 * General: add new local address,on which Stack will receive and send messages
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 *          eLocationInList - indication, where the new address should be placed
 *                            in the list of local addresses.
 *          hBaseLocalAddr  - An existing address in the list, before or after
 *                            which the new addresses can be added.
 *                            The parameter is meaningless, if eLocationInList
 *                            is not set to RVSIP_PREV_ELEMENT or
 *                            RVSIP_NEXT_ELEMENT.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr              *pAddressDetails,
                        IN  RvUint32                        addrStructSize,
                        IN  RvSipListLocation               eLocationInList,
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RvStatus rv;
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;
        
    if (NULL== pTransportMgr)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (NULL==pAddressDetails)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressAdd - NULL handle was supplied"));
        return RV_OK;
    }

	if (RV_FALSE == IsValidLocalAddress(pTransportMgr,pAddressDetails,addrStructSize)) 
	{
		return RV_ERROR_BADPARAM;
	}

    /* For IPv6 addresses only: if the application provided Scope ID in
    the IP string, set it into the designated field */
#if (RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAddressDetails->eAddrType)
    {
        RvChar *strScopeId;
        strScopeId = strchr(pAddressDetails->strIP, '%');
        if (NULL != strScopeId)
        {
            pAddressDetails->Ipv6Scope = atoi(strScopeId+1);
        }
    }
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/	

    rv = TransportMgrLocalAddressAdd(hTransportMgr,pAddressDetails,
            addrStructSize, eLocationInList, hBaseLocalAddr,
            RV_TRUE/*bConvertZeroPort*/, RV_FALSE/*pDummy*/, phLocalAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressAdd - Failed to add local %s address %s:%d (rv=%d)",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port,rv));
        return rv;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "RvSipTransportMgrLocalAddressAdd - Local %s address %s:%d was successfully added. Handle is 0x%p",
        SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
        pAddressDetails->strIP,pAddressDetails->port,*phLocalAddr));

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressRemove
 * ------------------------------------------------------------------------
 * General: remove the local address,on which Stack receives and sends messages
 *          The socket will be closed immediately.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the Transport Manager
 *          hLocalAddr      - handle to the address to be removed
 * Output : none
*****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressRemove(
                            IN  RvSipTransportMgrHandle         hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr)
{
    RvStatus rv;
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;

    if (NULL == pTransportMgr)
    {
        return RV_ERROR_UNKNOWN;
    }
    if (NULL == hLocalAddr)
    {
        
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressRemove - NULL handle was supplied"));
        return RV_OK;
    }

    rv = TransportMgrLocalAddressRemove(hTransportMgr,hLocalAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressRemove - Failed to remove local address 0x%p (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "RvSipTransportMgrLocalAddressRemove - Removing of Local Address 0x%p was successfully initiated",
        hLocalAddr));

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressFind
 * ------------------------------------------------------------------------
 * General: Finds the local address in the SIP  Stack that matches the details
 *          supplied by pAddressDetails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressFind(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr              *pAddressDetails,
                        IN  RvUint32                        addrStructSize,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RvStatus rv;
    RvSipTransportAddr      addressDetails;
    TransportMgrLocalAddr   localAddr;
    TransportMgr            *pTransportMgr;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if(pTransportMgr == NULL || phLocalAddr == NULL || pAddressDetails == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    *phLocalAddr = NULL;

    /*initialize a local address structure*/
    memset(&addressDetails,0,sizeof(RvSipTransportAddr));
    memcpy(&addressDetails,pAddressDetails,RvMin(sizeof(RvSipTransportAddr),addrStructSize));
    rv = TransportMgrInitLocalAddressStructure(pTransportMgr,
					       addressDetails.eTransportType, addressDetails.eAddrType,
					       addressDetails.strIP, addressDetails.port,
#if defined(UPDATED_BY_SPIRENT) 
					       addressDetails.if_name,
					       addressDetails.vdevblock,
#endif
					       &localAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressFind - Failed to initialize strucuture for local %s address %s:%d (rv=%d)",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port,rv));
        return rv;
    }
    /*find an equal structure in the local address list and get the handle*/
    TransportMgrFindLocalAddressHandle(pTransportMgr,&localAddr,phLocalAddr);
    if(*phLocalAddr == NULL)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressFind - local %s address %s:%d was not found",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port));
        return RV_ERROR_NOT_FOUND;
    }

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: Returns the details of the local address, the handle of which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     -  handle of the Local Address.
 *          addrStructSize  - size of the structure with details.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        IN  RvUint32                        addrStructSize,
                        OUT RvSipTransportAddr              *pAddressDetails)
{
    RvStatus rv;
    RvSipTransportAddr addressDetails;
    TransportMgr* pTransportMgr;

    if (NULL == hLocalAddr || NULL == pAddressDetails)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
    
#if (RV_NET_TYPE & RV_NET_IPV6)
    addressDetails.Ipv6Scope = UNDEFINED;
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    rv = TransportMgrLocalAddressGetDetails(hLocalAddr,&addressDetails);
    if(rv != RV_OK)
    {
        
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressGetDetails - Failed to get details of local address 0x%p (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    memcpy(pAddressDetails,&addressDetails,
           RvMin(sizeof(RvSipTransportAddr),addrStructSize));

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetFirst
 * ------------------------------------------------------------------------
 * General: gets handle of the Local Address, which is located at the head
 *          of the List of Local Addresses of the requested Transport Protocol
 *          type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          eTransportType  - type of the Transport Protocol.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetFirst(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransport                  eTransportType,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RvStatus rv;
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;

    if (NULL == pTransportMgr || NULL == phLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportMgrLocalAddressGetFirst(hTransportMgr,eTransportType,phLocalAddr);
    if(rv!=RV_OK && rv!=RV_ERROR_NOT_FOUND)
    {
        
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressGetFirst - Failed to get the first %s address (rv=%d)",
            SipCommonConvertEnumToStrTransportType(eTransportType),rv));
        return rv;
    }

    return rv;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetNext
 * ------------------------------------------------------------------------
 * General: Gets the handle of the local address that is located in the list of
 *          local addresses next to the address whose handle is supplied
 *          as a parameter to the function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBaseLocalAddr  - The handle to the local address that is located
 *                            before the requested address.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetNext(
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RvStatus rv;
    TransportMgr* pTransportMgr;

    if (NULL == hBaseLocalAddr || NULL == phLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = ((TransportMgrLocalAddr*)hBaseLocalAddr)->pMgr;

    rv = TransportMgrLocalAddressGetNext(hBaseLocalAddr,phLocalAddr);
    if(rv != RV_OK)
    {
        
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressGetNext - Failed to get the next after 0x%p address (rv=%d)",
            hBaseLocalAddr,rv));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressSetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Sets the IP_TOS socket option when the value is in decimal form.
 *          Note that the option does not provide QoS functionality
 *          in operation systems that support a more powerful DSCP mechanism
 *          in place of the previous TOS byte mechanism.
 *          The function can be called any time during the address life cycle.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - handle to the local address to be updated.
 *          typeOfService   - number, to be set as a TOS byte value.
 * Output : none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetIpTosSockOption(
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN RvInt32                         typeOfService)
{
    RvStatus rv;
    TransportMgr* pTransportMgr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
    rv = TransportMgrLocalAddressSetTypeOfService(pTransportMgr,
                            (TransportMgrLocalAddr*)hLocalAddr,typeOfService);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressSetIpTosSockOption - Failed to set TOS for 0x%p address (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Gets the value of the IP_TOS option that is set for the socket,
 *          which serves the specified local address.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - handle to the local address to be updated.
 * Output : pTypeOfService  - pointer to the memory, where the option value
 *                            will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetIpTosSockOption(
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN RvInt32                         *pTypeOfService)
{
    RvStatus rv;
    TransportMgr* pTransportMgr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
    rv = TransportMgrLocalAddressGetTypeOfService(pTransportMgr,
                            (TransportMgrLocalAddr*)hLocalAddr,pTypeOfService);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressGetIpTosSockOption - Failed to get TOS for 0x%p address (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    return RV_OK;
}

#ifdef RV_SIP_JSR32_SUPPORT

/******************************************************************************
 * RvSipTransportMgrLocalAddressSetSentBy
 * ------------------------------------------------------------------------
 * General: Sets a sent-by host and port to the local address object. These values
 *          will be set in the sent-by parameter of the top most via header
 *          of outgoing requests.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr   - handle of the local address.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          strSentByHost  - Null terminated sent-by host string.
 *          sentByPort - the sent by port, or UNDEFINED if the application does
 *                       not wish to set a port in the sent-by.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetSentByString(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        IN  RvChar*                         strSentByHost,
						IN  RvInt32                         sentByPort)
{
    RvStatus                rv           = RV_OK;
    TransportMgrLocalAddr*  pLocalAddr   = NULL;
    TransportMgr*           pTransportMgr= NULL;
	
    if (NULL == hLocalAddr || NULL == strSentByHost)
    {
        return RV_ERROR_BADPARAM;
    }
	pLocalAddr     = (TransportMgrLocalAddr*)hLocalAddr;
    pTransportMgr  = pLocalAddr->pMgr;
	rv = TransportMgrLocalAddressSetSentBy(pTransportMgr,pLocalAddr,strSentByHost,sentByPort);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressSetSentByString - localAddr 0x%p Failed to set %s as sent-by (rv=%d)",
            hLocalAddr,strSentByHost,rv));
        return rv;
    }
	RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
		"RvSipTransportMgrLocalAddressSetSentByString - localAddr 0x%p set %s as sent-by",
		hLocalAddr,strSentByHost));
	return RV_OK;

}
#endif
/******************************************************************************
 * RvSipTransportMgrLocalAddressGetConnection
 * ------------------------------------------------------------------------
 * General: Gets the handle of the listening Connection object,
 *          bound to the Local Address object of TCP/TLS/SCTP type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - handle to the local address to be updated
 * Output : phConn     - pointer to the memory, where the handle will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetConnection(
                                IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                                OUT RvSipTransportConnectionHandle  *phConn)
{
    RvStatus rv;
    TransportMgr* pTransportMgr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
    rv = TransportMgrLocalAddressGetConnection(pTransportMgr,
                                   (TransportMgrLocalAddr*)hLocalAddr, phConn);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressGetConnection - Failed to get hConn for 0x%p address (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportMgrLocalAddressCloseSocket
 * ------------------------------------------------------------------------
 * General: Closes the socket of the given Local Address object
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 * Output : none.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressCloseSocket(
                                IN RvSipTransportLocalAddrHandle   hLocalAddr)
{
    RvStatus rv;
    TransportMgr* pTransportMgr;
    TransportMgrLocalAddr* pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = pLocalAddr->pMgr;
    rv = TransportMgrLocalAddressCloseSocket(pLocalAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "RvSipTransportMgrLocalAddressCloseSocket - Address 0x%p - Failed to close the socket (rv=%d:%s)",
            hLocalAddr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "RvSipTransportMgrLocalAddressCloseSocket - Address 0x%p - socket was closed", hLocalAddr));
    return RV_OK;
}
/******************************************************************************
 * RvSipTransportMgrLocalAddressGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns an handle application to the local address object.
 *          You set this handle in the stack using the
 *          RvSipTransportMgrLocalAddressSetAppHandle() function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 * Output : phAppLocalAddr - The application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetAppHandle(
                                IN  RvSipTransportLocalAddrHandle     hLocalAddr,
                                OUT RvSipTransportLocalAddrAppHandle *phAppLocalAddr)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    *phAppLocalAddr = pLocalAddr->hAppLocalAddr; 

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets an handle application to the local address object.
 *          You get this handle in the stack using the
 *          RvSipTransportMgrLocalAddressGetAppHandle() function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 *			hAppLocalAddr  - The set application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetAppHandle(
                                IN  RvSipTransportLocalAddrHandle     hLocalAddr,
                                IN  RvSipTransportLocalAddrAppHandle  hAppLocalAddr)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    pLocalAddr->hAppLocalAddr = hAppLocalAddr;

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetSockAddrType
 * ------------------------------------------------------------------------
 * General: gets type of addresses (IPv4/IPv6), supported by the Local Address
 *          socket.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 *			hAppLocalAddr  - The set application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetSockAddrType(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvSipTransportAddressType*     peSockAddrType)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    
    if (NULL == hLocalAddr)
    {
        return RV_ERROR_BADPARAM;
    }
    
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    *peSockAddrType = pLocalAddr->eSockAddrType;

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetSecOwner
 * ----------------------------------------------------------------------------
 * General: Retrieves the security owner of the connection.
 *          The security owner is a security-object which is implemented in
 *          the RADVISION IMS SIP Security module.
 *          The IMS application should use this function when working with
 *          the Security module. The application should call this function from
 *          the RvSipTransportMsgReceivedEv() callback for a message received
 *          on a Local Address. This is to verify the identity of the message
 *          sender against the identity of the user with which the security
 *          owner established the protection. Calling this function from
 *          the RvSipTransportMsgReceivedEv() callback prevents further message
 *          processing in case the check fails.
 *          If the function returns NULL in the ppSecOwner parameter,this means
 *          that the message was not protected by the IMS security-object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - The connection handle
 * Output:  ppSecOwner - The pointer to the Security Object,
 *                       owning the Connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetSecOwner(
                        IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                        OUT void**                         ppSecOwner)
{
#ifdef RV_SIP_IMS_ON
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    
    if(hLocalAddr == NULL || ppSecOwner == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    *ppSecOwner = pLocalAddr->pSecOwner;
    
    RvLogDebug(pLocalAddr->pMgr->pLogSrc,(pLocalAddr->pMgr->pLogSrc,
        "RvSipTransportMgrLocalAddressGetSecOwner - Local Address 0x%p - returning %p",
        hLocalAddr, pLocalAddr->pSecOwner));

    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
#else
    RV_UNUSED_ARG(hLocalAddr);
    RV_UNUSED_ARG(ppSecOwner);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /*#ifdef RV_SIP_IMS_ON*/
}

/******************************************************************************
 * RvSipTransportConnectionSetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Sets the IP_TOS socket option for the socket serving the connection
 *          The option value is in decimal form.
 *          Note that the option does not provide QoS functionality
 *          in operation systems that support a more powerful DSCP mechanism
 *          in place of the previous TOS byte mechanism.
 *          The function can be called any time during the address life cycle.
 *
 * Return Value: RV_OK on success, error code otherwise (see RV_SIP_DEF.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hConn         - handle to the connection to be updated.
 *         typeOfService - number to be set as a TOS byte value.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetIpTosSockOption(
                            IN  RvSipTransportConnectionHandle  hConn,
                            IN  RvInt32                         typeOfService)
{
    RvStatus                rv = RV_OK;
    TransportConnection*    pConn         = (TransportConnection*)hConn;
    TransportMgr*           pTransportMgr = NULL;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(RV_FALSE == TRANSPORT_CONNECTION_TOS_VALID(typeOfService))
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    pConn->typeOfService = typeOfService;

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "RvSipTransportConnectionSetIpTosSockOption - Connection 0x%p - set typeOfService %d",
        pConn, typeOfService));

    if (RV_TRUE == TRANSPORT_CONNECTION_TOS_VALID(pConn->typeOfService) &&
        NULL != pConn->pSocket)
    {
        rv = RvSocketSetTypeOfService(pConn->pSocket,pConn->typeOfService,pConn->pTransportMgr->pLogMgr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "RvSipTransportConnectionSetIpTosSockOption - pConn 0x%p: failed to set TypeOfService=%d for socket %d, rv=%d",
                pConn,pConn->typeOfService,pConn->ccSocket,RvErrorGetCode(rv)));
        }
    }

    TransportConnectionUnLockAPI(pConn);

    return rv;
}
/******************************************************************************
 * RvSipTransportConnectionGetTypeOfService
 * ------------------------------------------------------------------------
 * General: Gets the value of the IP_TOS option that is set for the socket,
 *          which serves the specified connection.
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hConn           - handle to the connection to be updated.
 * Output: pTypeOfService  - pointer to the memory, where the option value
 *                           will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetIpTosSockOption(
                            IN  RvSipTransportConnectionHandle  hConn,
                            IN  RvInt32                         *pTypeOfService)
{
    RvStatus                rv = RV_OK;
    TransportConnection*    pConn         = (TransportConnection*)hConn;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *pTypeOfService = pConn->typeOfService;
    TransportConnectionUnLockAPI(pConn);

    return rv;
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransportConnectionSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message
 *          on the Connection, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hConn      - Handle to the Connection object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    TransportConnection  *pConn = (TransportConnection*)hConn;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(&params,pParams,minStructSize);

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "RvSipTransportConnectionSetSctpMsgSendingParams - connection 0x%p - failed to lock the TransportConnection",
            pConn));
        return RV_ERROR_INVALID_HANDLE;
    }

    SipTransportConnectionSetSctpMsgSendingParams(hConn, &params);

    RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "RvSipTransportConnectionSetSctpMsgSendingParams - connection 0x%p - SCTP params(stream=%d, flag=0x%x) were set",
        pConn, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    TransportConnectionUnLockAPI(pConn);
    return RV_OK;
}

/******************************************************************************
 * RvSipTransportConnectionGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message
 *          on the Connection, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hConn      - Handle to the Connection object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams  *pParams)
{
    RvStatus rv;
    TransportConnection  *pConn = (TransportConnection*)hConn;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "RvSipTransportConnectionGetSctpMsgSendingParams - connection 0x%p - failed to lock the TransportConnection",
            pConn));
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    SipTransportConnectionGetSctpMsgSendingParams(hConn, &params);
    memcpy(pParams,&params,minStructSize);

    RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "RvSipTransportConnectionGetSctpMsgSendingParams - connection 0x%p - SCTP params(stream=%d, flag=0x%x) were get",
        pConn, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    TransportConnectionUnLockAPI(pConn);
    return RV_OK;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/


/***************************************************************************
 * RvSipTransportConnectionEnableConnByAlias
 * ------------------------------------------------------------------------
 * General: This function enables a server connection for further reuse, by
 *          inserting it to the connections hash table. 
 *          When an incoming request has an alias parameter in it's top most
 *          via header, the callback RvSipTransportConnectionServerReuseEv()
 *          is called. in this callback application should authorize this
 *          connection, and if authorized, it should use this function to
 *          enable it for sending future requests.
 *          the connection is identified by an alias string, given in the 
 *          top most via header. 
 *          
 *          (The function is different from RvSipTransportConnectionEnable, 
 *          because it uses the alias name of the connection, and not the remote address)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionEnableConnByAlias(
                                        IN  RvSipTransportConnectionHandle hConn) 
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (RVSIP_TRANSPORT_CONN_TYPE_SERVER != pConn->eConnectionType)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionEnableConnByAlias - Conn: 0x%p - illegal action for non-server connection",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL == pConn->pAlias.hPage)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionEnableConnByAlias - Conn: 0x%p - Connection has no alias. Call this API from RvSipTransportConnectionServerReuseEv() only",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*insert connection to the hash*/
    rv = TransportConnectionHashInsert(hConn, RV_TRUE);
    if(rv != RV_OK)
    {
         RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnableConnByAlias - connection 0x%p - Failed to insert connection to hash (rv=%d)",
              pConn,rv));

            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
         return rv;
    }
    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionEnableConnByAlias - connection 0x%p - connection was inserted connection to hash",
              pConn));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransportConnectionGetAlias
 * ------------------------------------------------------------------------
 * General: This function retrieves the connection alias string.
 *          The function always retrieves the alias string length. 
 *          If an allocated buffer is also given, and allocatedBufferLen>0, 
 *          the alias string will be copy to the buffer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *            allocatedBufferLen - length of the given allocated buffer.
 * Output:    pAliasLength - length of the alias string. (0 if not exists)
 *            pBuffer      - The buffer filled with the alias string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetAlias(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        IN  RvInt32                         allocatedBufferLen,
                                        OUT RvInt32                        *pAliasLength,
                                        OUT RvChar                         *pBuffer)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus             rv;

    if(hConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pAliasLength == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    *pAliasLength = 0;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransportMgr = pConn->pTransportMgr;

    if (NULL == pConn->pAlias.hPage)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetAlias - Conn: 0x%p - no alias parameter",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_NOT_FOUND;
    }
    
    *pAliasLength = RPOOL_Strlen(pConn->pAlias.hPool, pConn->pAlias.hPage, pConn->pAlias.offset);

    if( allocatedBufferLen == 0)
    {
        TransportConnectionUnLockAPI(pConn);
        return RV_OK;
    }
    if(*pAliasLength+1 > allocatedBufferLen)
    {
        RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetAlias - Conn: 0x%p - insufficient buffer",
            pConn));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    
    if(pBuffer != NULL)
    {
        rv = RPOOL_CopyToExternal(pConn->pAlias.hPool, pConn->pAlias.hPage, pConn->pAlias.offset, pBuffer, *pAliasLength);
        if(rv != RV_OK)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
                "RvSipTransportConnectionGetAlias - Conn: 0x%p - Failed to copy alias string to the given buffer",
                pConn));
            pBuffer[0] = '\0';
            TransportConnectionUnLockAPI(pConn);
            return rv;
        }
        pBuffer[*pAliasLength] = '\0';

    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "RvSipTransportConnectionGetAlias - connection 0x%p - alias %s was retrived",
              pConn, pBuffer));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransportMgrLocalAddressGetContext
 * ----------------------------------------------------------------------------
 * General: The IMS application should use this function when working with
 *          the RADVISION IMS SIP Security-Agreement module. This function
 *          returns the context (sizeof(void*) bytes) that was set by
 *          the application into the security-agreement object which uses
 *          the local address for message protection. The context is considered
 *          to be an IMPI-identifier of the IMS client. Upon message reception,
 *          the application has to verify the identity of the message sender
 *          against the identity of the user, with which the security-agreement
 *          was performed. Therefore, the application should call
 *          the RvSipTransportMgrLocalAddressGetContext() function from
 *          the RvSipTransportMsgReceivedEv() callback to prevent message
 *          processing if the verification fails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - Handle to the local address.
 * Output : pContext   - The context.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetContext(
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                            OUT void**                          pContext)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if (NULL == hLocalAddr  ||  NULL == pContext)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    *pContext = pLocalAddr->impi; 

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransportConnectionGetContext
 * ------------------------------------------------------------------------
 * General: The IMS application should use this function when working with
 *          the RADVISION IMS SIP Security-Agreement module. This function
 *          returns the context (sizeof(void*) bytes) that was set by
 *          the application into the security-agreement object which uses
 *          the connection for message protection. The context is considered
 *          to be an IMPI-identifier of the IMS client. Upon message reception,
 *          the application has to verify the identity of the message sender
 *          against the identity of the user, with which the security-agreement
 *          was performed. Therefore, the application should call
 *          the RvSipTransportMgrLocalAddressGetContext() function from
 *          the RvSipTransportMsgReceivedEv() callback to prevent message
 *          processing if the verification fails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn    - Handle to the connection.
 * Output : pContext - The context.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetContext(
                                IN  RvSipTransportConnectionHandle hConn,
                                OUT void**                         pContext)
{
    TransportConnection* pConn = (TransportConnection*)hConn;
    RvStatus             rv;

    if(hConn == NULL || pContext == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return rv;
    }

    *pContext = pConn->impi;

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "RvSipTransportConnectionGetContext - connection 0x%p - returning 0x%p",
        pConn, pConn->impi));

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/******************************************************************************
 * RvSipTransportAddr2RvAddress
 * ----------------------------------------------------------------------------
 * General: Converts RvSipTransportAddr structure to RvAddress structure.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAddressDetails - RvSipTransportAddr structure.
 * Output:  pRvAddress      - RvAddress to be filled.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertTransportAddr2RvAddress(
                                    IN  RvSipTransportAddr  *pAddressDetails,
                                    OUT RvAddress           *pRvAddress)
{
    return TransportMgrConvertTransportAddr2RvAddress(NULL, pAddressDetails, pRvAddress);
}

/******************************************************************************
 * RvSipTransportConvertRvAddress2TransportAddr
 * ----------------------------------------------------------------------------
 * General: converts RvAddress object structure to RvSipTransportAddr structure.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pRvAddress      - RvSipTransportAddr structure.
 * Output:  pTransportAddr  - RvSipTransportAddr to be filled.
 *****************************************************************************/
RVAPI void RVCALLCONV RvSipTransportConvertRvAddress2TransportAddr(
                                      IN  RvAddress          *pRvAddress,
                                      OUT RvSipTransportAddr *pTransportAddr)
{
    TransportMgrConvertRvAddress2TransportAddr(pRvAddress, pTransportAddr);
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * RvSipTransportPQPrintOORlist
 * ----------------------------------------------------------------------------------
 * General: prints objects handles from the OOR list to the log.
 *          this function may be called by the application in order to have a better
 *          view of the out of resource status.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Transport manager handler
 *          
 * Output:  none
 ***********************************************************************************/
RVAPI void RVCALLCONV RvSipTransportPrintOORlist(IN  RvSipTransportMgrHandle   hTransportMgr)
{
    TransportPQPrintOORlist((TransportMgr*)hTransportMgr);
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * RvSipTransportConnectionGetClientLocalPort
 * ------------------------------------------------------------------------
 * General: The function returns the  port used by the local socket after
 *          establishing the given client-side connection object. 
 *
 * NOTE: This function might return different port number than the API 
 *       RvSipTransportConnectionGetLocalAddress(), since this function
 *       returns the actual port, bound to the client-side connection socket. 
 *       This port number might be an ephemeral port number.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to a client-side connection.
 *
 * Output : pPort - The local port of the given connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetClientLocalPort(
                                IN  RvSipTransportConnectionHandle  hConn,
                                OUT RvUint16                       *pPort)
{
    TransportConnection* pConn = (TransportConnection*)hConn;
    RvStatus             rv;

    if(hConn == NULL || pPort == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return rv;
    }

    if (pConn->eConnectionType != RVSIP_TRANSPORT_CONN_TYPE_CLIENT) 
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetClientLocalPort - connection 0x%p - cannot handle client side connection",
            pConn, pConn->localPort));
        
        rv = RV_ERROR_BADPARAM;
    }
    else if (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_UNDEFINED ||
             pConn->eState == RVSIP_TRANSPORT_CONN_STATE_IDLE      ||
             pConn->eState == RVSIP_TRANSPORT_CONN_STATE_READY)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetClientLocalPort - connection 0x%p - no connection was established yet",
            pConn, pConn->localPort));
        
        rv = RV_ERROR_BADPARAM;
    }
    else 
    {
        *pPort = (RvUint16)pConn->localPort;

        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "RvSipTransportConnectionGetClientLocalPort - connection 0x%p - returning port %d",
            pConn, pConn->localPort));

        rv = RV_OK;
    } 
          

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransportInjectMsg
 * ------------------------------------------------------------------------
 * General: enables application to 'inject' a message to the stack.
 *          the message may be given in a string or message structure handle formats.
 *          the given message will be handled as if it was received from network.
 *          in case of multihome ip - application should supply the local address,
 *          so stack will know from where to send the response. if not supplied,
 *          stack will choose one by itself.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr  - Handle to the transport manager.
 *          pMsgBuffer     - The 'injected' message in a string format.
 *          msgBufferLen   - Length of the message string.
 *          totalMsgLength - total length of the message given in pMsgBuffer.
 *          hMsg           - Handle to the 'injected' message, in a message structure format.
 *          hConn          - The connection handle that the message is 'injected' to.
 *          pAddressInfo   - Structure contains the local address and remote address.
 *                           (The remote address, is the one who will receive the response).
 ***************************************************************************/
static RvStatus RVCALLCONV TransportInjectMsg(
                        IN  RvSipTransportMgrHandle        hTransportMgr,
                        IN  RvChar                        *pMsgBuffer,
                        IN  RvUint32                       totalMsgLength,
                        IN  RvSipMsgHandle                 hMsg,
                        IN  RvSipTransportConnectionHandle hConn,
                        IN  RvSipTransportMsgAddrCfg      *pAddressInfo)
{
    TransportMgr*                     pTransportMgr  = (TransportMgr*)hTransportMgr;
    TransportProcessingQueueEvent*    ev             = NULL;
    RvStatus                          rv             = RV_OK;

    /* allocate event */
    rv = TransportProcessingQueueAllocateEvent(hTransportMgr, NULL, MESSAGE_RCVD_EVENT, RV_FALSE, &ev);
    if (rv != RV_OK)
    {
        /* if there are no elements for injecting a message, we will treat it as regular out of resources,
           and will not activate any OOR special treatment. the application can resend the message later*/
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportInjectMsg - Failed to allocate event structure for processing read event"));
        return rv;
    }

    /* set the injected message in the allocated event */
    rv = RvSipMsgConstruct(pTransportMgr->hMsgMgr,
                           pTransportMgr->hMsgPool,
                           &(ev->typeSpecific.msgReceived.hInjectedMsg));
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportInjectMsg - Failed to construct message in event struct"));
        TransportProcessingQueueFreeEvent(hTransportMgr, ev);
        return rv;
    }
    if(pMsgBuffer != NULL)
    {
        /* construct and parse the given message */
        rv = RvSipMsgParse(ev->typeSpecific.msgReceived.hInjectedMsg, pMsgBuffer, totalMsgLength);
    }
    else
    {
        rv = RvSipMsgCopy(ev->typeSpecific.msgReceived.hInjectedMsg, hMsg);
    }
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportInjectMsg - Failed to fill message in event struct"));
        RvSipMsgDestruct(ev->typeSpecific.msgReceived.hInjectedMsg);
        TransportProcessingQueueFreeEvent(hTransportMgr, ev);
        return rv;
    }

    /* set local and dest addresses in the event */
    ev->typeSpecific.msgReceived.hConn = hConn;

    if(hConn != NULL)
    {
        RvBool bCanSend = RV_TRUE;
        TransportConnection  *pConn = (TransportConnection*)(hConn);

        if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc ,
                "TransportInjectMsg - Conn: 0x%p - illegal action for multi-server connection",
                pConn));
            RvSipMsgDestruct(ev->typeSpecific.msgReceived.hInjectedMsg);
            TransportProcessingQueueFreeEvent(hTransportMgr, ev);
            return RV_ERROR_ILLEGAL_ACTION;
        }

        if(pConn->eState == RVSIP_TRANSPORT_CONN_STATE_IDLE ||
            pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CLOSING ||
            pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CLOSED ||
            pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TERMINATED)

        {
           bCanSend = RV_FALSE;
        }
#if (RV_TLS_TYPE != RV_TLS_NONE)
        if (RVSIP_TRANSPORT_TLS == pConn->eTransport &&
            (RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED == pConn->eTlsState||
            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED == pConn->eTlsState))
        {
           bCanSend = RV_FALSE;
        }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
        if (RV_FALSE == bCanSend)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportInjectMsg - Failed. connection 0x%p is in state %d", hConn, pConn->eState));
            RvSipMsgDestruct(ev->typeSpecific.msgReceived.hInjectedMsg);
            TransportProcessingQueueFreeEvent(hTransportMgr, ev);
            return rv;
        }
        ev->typeSpecific.msgReceived.hLocalAddr = pConn->hLocalAddr;
        ev->typeSpecific.msgReceived.eConnTransport = pConn->eTransport;
    }

    if(pAddressInfo == NULL)
    {
        RvSipTransportMsgAddrCfg tempAddressInfo;
        tempAddressInfo.destPort = 0;
        tempAddressInfo.localPort = 0;
        tempAddressInfo.strDestIp = NULL;
        tempAddressInfo.strLocalIp = NULL;
#if defined(UPDATED_BY_SPIRENT)
        tempAddressInfo.if_name = NULL;
	tempAddressInfo.vdevblock = 0;
#endif
        tempAddressInfo.eTransportType = RVSIP_TRANSPORT_UNDEFINED;

        rv = SetInjectAddresses(pTransportMgr, ev, hConn, &tempAddressInfo);
    }
    else
    {
        rv = SetInjectAddresses(pTransportMgr, ev, hConn, pAddressInfo);
    }
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportInjectMsg - SetInjectAddresses() Failed. rv = %d", rv));
            RvSipMsgDestruct(ev->typeSpecific.msgReceived.hInjectedMsg);
            TransportProcessingQueueFreeEvent(hTransportMgr, ev);
            return rv;
    }


    /* insert event to the queue */
    rv = TransportProcessingQueueTailEvent(hTransportMgr, ev);
    if (rv != RV_OK)
    {
        RvSipMsgDestruct(ev->typeSpecific.msgReceived.hInjectedMsg);
        TransportProcessingQueueFreeEvent(hTransportMgr, ev);
        return rv;
    }
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if(pMsgBuffer != NULL)
    {
        TransportMsgBuilderPrintMsg(pTransportMgr,
                                    pMsgBuffer,
                                    SIP_TRANSPORT_MSG_BUILDER_INCOMING,
                                    RV_FALSE);
    }
#endif/*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    return rv;
}

/***************************************************************************
 * SetInjectAddresses
 * ------------------------------------------------------------------------
 * General: The function sets the local and dest addresses from the given
 *          RvSipTransportMsgAddrCfg structure. The function sets default
 *          values for missing parameters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr  - Pointer to the transport manager.
 *          ev             - Pointer to the TransportProcessingQueueEvent that will
 *                           process this message.
 *            hConn          - The connection handle that the message is 'injected' to.
 *          pAddressInfo   - Structure contains the local address and remote address.
 *                           (The remote address, is the one who will receive the response).
 ***************************************************************************/
static RvStatus RVCALLCONV SetInjectAddresses(
                                    IN TransportMgr*                  pTransportMgr,
                                    IN TransportProcessingQueueEvent* ev,
                                    IN RvSipTransportConnectionHandle hConn,
                                    IN RvSipTransportMsgAddrCfg*      pAddressInfo)
{
    RvStatus                   rv = RV_OK;
    RvSipTransportAddressType   eAddrType;
    RvSipTransport              eDefaultTransportType;

    if(hConn == NULL)
    {
        eDefaultTransportType = (RVSIP_TRANSPORT_UNDEFINED == pAddressInfo->eTransportType)? RVSIP_TRANSPORT_UDP : pAddressInfo->eTransportType;

        if(pAddressInfo->strLocalIp != NULL && pAddressInfo->localPort > 0)
        {
            eAddrType = (pAddressInfo->strLocalIp[0] == '[')? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
            rv = SipTransportLocalAddressGetHandleByAddress(
                                (RvSipTransportMgrHandle)pTransportMgr,
                                eDefaultTransportType,
                                eAddrType,
                                pAddressInfo->strLocalIp,
                                pAddressInfo->localPort,
#if defined(UPDATED_BY_SPIRENT)
				pAddressInfo->if_name,
				pAddressInfo->vdevblock,
#endif
                                &(ev->typeSpecific.msgReceived.hLocalAddr));

            /* SCTP Local Address of IPv6 type can hold IPv4 addresses. 
               If the IPv4 Local Address was not found, try to find IPv6 addr*/
#if (RV_NET_TYPE & RV_NET_SCTP)
            if (RV_ERROR_NOT_FOUND == rv  &&
                RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eAddrType)
            {
                eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
                rv = SipTransportLocalAddressGetHandleByAddress(
                                (RvSipTransportMgrHandle)pTransportMgr,
                                eDefaultTransportType,
                                eAddrType,
                                pAddressInfo->strLocalIp,
                                pAddressInfo->localPort,
#if defined(UPDATED_BY_SPIRENT)
				pAddressInfo->if_name,
				pAddressInfo->vdevblock,
#endif
                                &(ev->typeSpecific.msgReceived.hLocalAddr));
            }
#endif
            if(rv != RV_OK)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "SetInjectAddresses - Failed to set default local address"));
                return rv;
            }
        }
        else /* take the addresses values according to the dest address */
        {

            RvSipTransportAddressType       eDefaultAddrType;


            /* take default values */
            if(pAddressInfo->strDestIp != NULL)
            {
                eDefaultAddrType = (pAddressInfo->strDestIp[0] == '[')? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
            }
            else
            {
                eDefaultAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
            }

            TransportMgrGetDefaultLocalAddress(
                                 pTransportMgr,
                                 eDefaultTransportType,
                                 eDefaultAddrType,
                                 &(ev->typeSpecific.msgReceived.hLocalAddr));
            if(ev->typeSpecific.msgReceived.hLocalAddr == NULL)
            {
                /* trying to get the ip6 address, if we took ip by default */
                if(RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eDefaultAddrType &&
                   NULL == pAddressInfo->strDestIp)
                {
                    TransportMgrGetDefaultLocalAddress(
                                 pTransportMgr,
                                 eDefaultTransportType,
                                 RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                                 &(ev->typeSpecific.msgReceived.hLocalAddr));
                }
                if(ev->typeSpecific.msgReceived.hLocalAddr == NULL)
                {
                    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "SetInjectAddresses - Failed to set default local address"));
                    return RV_ERROR_UNKNOWN;
                }
            }
        }
    } /* if(hConn == NULL) */

    if(pAddressInfo->strDestIp != NULL)
    {
        RvBool bIsIp4;
        bIsIp4 = (pAddressInfo->strDestIp[0] == '[')? RV_FALSE:RV_TRUE;
        rv = TransportMgrConvertString2Address(pAddressInfo->strDestIp,
                                               &(ev->typeSpecific.msgReceived.rcvFromAddr),
                                               bIsIp4,RV_TRUE);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SetInjectAddresses - Failed to set convert destIp to LI_Address (rv=%d)",rv));
            return RV_ERROR_UNKNOWN;
        }
        if(pAddressInfo->destPort > 0)
        {
            RvAddressSetIpPort(&ev->typeSpecific.msgReceived.rcvFromAddr.addr,pAddressInfo->destPort);
        }
    }
    return RV_OK;

}
#endif /*#ifndef RV_SIP_PRIMITIVES  */

/***************************************************************************
* IsValidLocalAddress
* ------------------------------------------------------------------------
* General: This function verfies that the given local address structure 
*	       is valid. For example, the IP address string might not match
*		   the local address address type. In this case the function will
*		   find a problem. 
* Return Value: RV_TRUE if the local address structure is valid otherwise
*				RV_FALSE.
* ------------------------------------------------------------------------
* Arguments:
* Input: pTransportMgr   - handle of the Transport Manager.
*        pAddressDetails - pointer to the memory, where the details of
*                            the address to be added are stored.
*        addrStructSize  - size of the structure with details.
***************************************************************************/
static RvBool RVCALLCONV IsValidLocalAddress(
						 IN  TransportMgr					*pTransportMgr,
						 IN  RvSipTransportAddr             *pAddressDetails,
						 IN  RvUint32                        addrStructSize)
{
	RvSipTransportAddr localAddrDetails;

	memset(&localAddrDetails,0,sizeof(RvSipTransportAddr));
    memcpy(&localAddrDetails,pAddressDetails,RvMin(sizeof(RvSipTransportAddr),addrStructSize));

	/* Check if the IP string match the address type */
	switch (localAddrDetails.eAddrType) 
	{
	case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:
		if (strcmp(localAddrDetails.strIP,"0") != 0 && /*zero IP V4*/
			RV_FALSE == SipTransportISIpV4(localAddrDetails.strIP))
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"IsValidLocalAddress - The given IP address %s is not valid IPV4 address",
				localAddrDetails.strIP));
			return RV_FALSE;	
		}
		break;
	case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:
		if (strcmp(localAddrDetails.strIP,"0") != 0 && /*zero IP V6*/
			RV_FALSE == SipTransportISIpV6(localAddrDetails.strIP))
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"IsValidLocalAddress - The given IP address %s is not valid IPV6 address",
				localAddrDetails.strIP));
			return RV_FALSE;
		}
		break;
	default:
		RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
			"IsValidLocalAddress - address type is UNDEFINED of local address %s",
			localAddrDetails.strIP));
		return RV_FALSE;
	}
    
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return RV_TRUE;
}
/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* TransportError handlers 
*/
#if defined(UPDATED_BY_SPIRENT)

RVAPI void RVCALLCONV RvSipTransportErrorSetEvHandlers(
                                    IN RvSipTransportErrorEvHandlers* hErrorEvHandlers)
{
   if ( NULL == hErrorEvHandlers ) {
      SIP_TransportErrorEvHandlers.pfnRejectMsgEvHandler = NULL;  
   } else {
      SIP_TransportErrorEvHandlers.pfnRejectMsgEvHandler = hErrorEvHandlers->pfnRejectMsgEvHandler;  
   }
}

RVAPI void RVCALLCONV RvSipTransportErrorRejectMsgEvHandler(
                                    IN  RvChar   *pEventTextMsg)
{
   if ( NULL != SIP_TransportErrorEvHandlers.pfnRejectMsgEvHandler )
   {
      SIP_TransportErrorEvHandlers.pfnRejectMsgEvHandler(pEventTextMsg);
   }
}

RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetSockSize(
								   IN RvSipTransportLocalAddrHandle   hLocalAddr,
								   IN RvUint32                        sockSize) {
  
  RvStatus rv;
  TransportMgr* pTransportMgr;
  
  if (NULL == hLocalAddr) {
    return RV_ERROR_BADPARAM;
  }
  
  pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
  rv = TransportMgrLocalAddressSetSockSize(pTransportMgr,
					   (TransportMgrLocalAddr*)hLocalAddr,sockSize);
  if(rv != RV_OK) {
    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				       "%s - Failed to set sock size for 0x%p address (rv=%d)",
				       __FUNCTION__,hLocalAddr,rv));
    return rv;
  }
  
  return RV_OK;  
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif

