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
 *                              _SipTransport.c
 *
 * This c file contains internal API of the transport module
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20 Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "_SipTransport.h"
#include "_SipTransportTypes.h"
#include "TransportMsgBuilder.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#include "AdsRpool.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransport.h"
#include "RvSipTransportDNS.h"
#include "RvSipTransportDNSTypes.h"
#include "AdsPqueue.h"
#include "TransportProcessingQueue.h"
#include "TransportMgrObject.h"
#include "TransportUDP.h"
#include "TransportDNS.h"
#include "TransportCallbacks.h"
#include "rvhost.h"
#include "_SipCommonCore.h"
#include "rvansi.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTP.h"
#endif

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

#define SPIRENT_READ_UNBLOCKING
#define SPIRENT_READ_TIME_LIMIT (10)

#include "rvexternal.h"

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

#define HANDLE_STR_SIZE 18

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV TransportSendObjectEvent(
                                IN RvSipTransportMgrHandle      hTransportMgr,
                                IN void*                        pObj,
                                IN RvInt32                      objUniqueIdentifier,
                                IN RvSipTransportObjEventInfo*  pEventInfo,
                                IN RvInt32                      param1,
                                IN RvInt32                      param2,
                                IN RvSipTransportObjectEventHandler    func,
                                IN SipTransportObjectStateEventHandler  funcExt,
                                IN RvBool                              bInternal,
                                IN const RvChar*                strLogInfo,
                                IN const RvChar*                strObjType);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/************************************************************************************
 * SipTransportConstruct
 * ----------------------------------------------------------------------------------
 * General: Construct and initialize the transport module.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportCfg        - Configuration needed for constructing the
 *                                 transport module.
 * Output:  hTransportMgr        - Handle to the transport manager.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConstruct(
                                   INOUT SipTransportCfg         *pTransportCfg,
                                   OUT   RvSipTransportMgrHandle *hTransportMgr)
{

    RvStatus         rv              = RV_OK;
    RvStatus          crv             = RV_OK;
    TransportMgr*     pTransportMgr   = NULL;

    *hTransportMgr = NULL;

    RvLogInfo(pTransportCfg->regId,(pTransportCfg->regId,
        "SipTransportConstruct - Constructing the Transport module"));

    /*allocate memory*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(TransportMgr), pTransportCfg->pLogMgr, (void*)&pTransportMgr))
    {
         return RV_ERROR_UNKNOWN;
    }
    memset(pTransportMgr, 0, sizeof(TransportMgr));

    /*construct all mutexs*/
    crv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->udpBufferLock);
    if(crv != RV_OK)
    {
        RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);
        return CRV2RV(crv);
    }
    crv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->hObjEventListLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
        RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);
        return CRV2RV(crv);

    }
    crv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->hRcvBufferAllocLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
        RvMutexDestruct(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
        RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);
        return CRV2RV(crv);

    }

    crv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->hOORListLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
        RvMutexDestruct(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
        RvMutexDestruct(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);
        RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);
        return CRV2RV(crv);

    }

    if (RV_FALSE != pTransportCfg->bDLAEnabled)
    {
        crv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->lockLocalAddresses);
        if(crv != RV_OK)
        {
            RvMutexDestruct(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
            RvMutexDestruct(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
            RvMutexDestruct(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);
            RvMutexDestruct(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
            RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);
            return CRV2RV(crv);
        }
    }

    RvSocketSetUDPMaxLength(pTransportCfg->maxBufSize);
   /*initialize the transport manager */
    rv = TransportMgrInitialize(pTransportMgr,pTransportCfg);
    if(rv != RV_OK)
    {
        RvLogError(pTransportCfg->regId,(pTransportCfg->regId,
                   "SipTransportConstruct - Failed - destructing the module"))
        TransportMgrDestruct(pTransportMgr);
        return rv;
    }
    *hTransportMgr = (RvSipTransportMgrHandle)pTransportMgr;
    return RV_OK;
}


/************************************************************************************
 * SipTransportStopProcessingThreads
 * ----------------------------------------------------------------------------------
 * General: Stops transport processing threads. Must be done before
 *          freeing allocated memory.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportStopProcessingThreads(IN RvSipTransportMgrHandle hTransportHandle)
{
    TransportMgr      *transportInfo;
    RvUint32           i;
    RvThread           nullthread;
    RvBool             bJoinToOther;

    memset(&nullthread,0,sizeof(nullthread));

    transportInfo = (TransportMgr*)hTransportHandle;
    /* Terminate Processing Threads (if any) */
    for(i=0; i < transportInfo->numberOfProcessingThreads; i++)
    {
        PQUEUE_KillThread(transportInfo->hProcessingQueue);
    }

    for(i=0; (transportInfo->processingThreads != NULL) && (i < transportInfo->numberOfProcessingThreads); i++)
    {
        if (0 == memcmp(&(transportInfo->processingThreads[i]),&nullthread,sizeof(nullthread)))
            continue;
        RvLogDebug(transportInfo->pLogSrc,(transportInfo->pLogSrc,
            "SipTransportStopProcessingThreads - waiting for thread 0x%p to be terminated",
                &((transportInfo->processingThreads)[i])));
        RvThreadJoin(&(transportInfo->processingThreads[i]),&bJoinToOther);
        if (RV_FALSE == bJoinToOther)
        {
            RvLogWarning(transportInfo->pLogSrc,(transportInfo->pLogSrc,
            "SipTransportStopProcessingThreads - thread 0x%p tries to join itself",
                &((transportInfo->processingThreads)[i])));
        }
    }

}


/************************************************************************************
 * SipTransportDestruct
 * ----------------------------------------------------------------------------------
 * General: Destruct the transport layer module and release the used memory.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to the transport manager
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportDestruct(IN RvSipTransportMgrHandle hTransportMgr)
{

    TransportMgr       *pTransportMgr;
    RvStatus           rv = RV_OK;
    RvLogSource        *pLogSrc;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if (pTransportMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    pLogSrc = pTransportMgr->pLogSrc;
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "SipTransportDestruct - Destructing the Transport module"));


    rv = TransportMgrDestruct(pTransportMgr);
    if(rv != RV_OK)
    {
        RvLogError(pLogSrc,(pLogSrc,
                   "SipTransportDestruct - Failed to destruct the Transport module"));
        return rv;
    }

    return RV_OK;
}

/************************************************************************************
 * SipTransportSetEventHandler
 * ----------------------------------------------------------------------------------
 * General: Set callback functions of the transport module.
 * Return Value: RvStatus - RV_OK, RV_ERROR_INVALID_HANDLE
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager.
 *          evHandlers      - struct with pointers to callback functions.
 *          evHandlersSize  - ev handler struct size
 *          hContext - will be used as a parameter for the callbacks.
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportSetEventHandler(
                                        IN RvSipTransportMgrHandle            hTransportMgr,
                                        IN SipTransportEvHandlers             *evHandlers,
                                        IN RvUint32                          evHandlersSize,
                                        IN SipTransportCallbackContextHandle  hContext)
{
    TransportMgr *pTransportMgr;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if (pTransportMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }



    /* setting the information for the call back */
    pTransportMgr->hCallbackContext = hContext;

    memset(&(pTransportMgr->eventHandlers), 0, evHandlersSize);
    memcpy(&(pTransportMgr->eventHandlers), evHandlers, evHandlersSize);

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "SipTransportSetEventHandler - the event handlers were set successfully"));

    return RV_OK;
}

/******************************************************************************
 * SipTransportSendObjectEvent
 * ----------------------------------------------------------------------------
 * General: Insert an object termination event into the processing queue.
 *        A termination event uses an out-of-resource recovery mechanism.
 *          Note - if you want to use an event for state, please use the 
 *          SipTransportSendStateObjectEvent function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr       - transport handler
 *          pObj                - Pointer to the object sending the event.
 *          pEventInfo          - empty allocated structure.
 *          eNum                - event enumeration, may be used for reason or state
 *          func                - callback function
 *          bInternal           - is the event stack or external to stack
 *          pLoggingInfo        - logging message limited to TRANSPORT_EVENT_LOG_LEN len
 *          pObjName            - the name of the object that sends the message
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSendTerminationObjectEvent(
                        IN RvSipTransportMgrHandle             hTransportMgr,
                        IN void*                               pObj,
                        IN RvSipTransportObjEventInfo*         pEventInfo,
                        IN RvInt32                             eNum,
                        IN RvSipTransportObjectEventHandler    func,
                        IN RvBool                              bInternal,
                        IN const RvChar*                       pLoggingInfo,
                        IN const RvChar*                       pObjName)

{
    return TransportSendObjectEvent(hTransportMgr, pObj, UNDEFINED, pEventInfo, 
                                  eNum, UNDEFINED, func, NULL, bInternal, pLoggingInfo, pObjName);
}

/******************************************************************************
 * SipTransportSendStateObjectEvent
 * ----------------------------------------------------------------------------
 * General: Insert a state-changed event into the processing queue.
 *        A state event does not use an out-of-resource recovery mechanism.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr       - transport handler
 *          pObj                - Pointer to the object sending the event.
 *          objUniqueIdentifier - the unique id of the object to terminate. when the event
 *                                pops, before handling the event we check that the object
 *                                was not reallocated, using this identifier.
 *          pEventInfo          - empty allocated structure.
 *          eNum                - event enumeration, may be used for reason or state
 *          func                - callback function
 *          bInternal           - is the event stack or external to stack
 *          pLoggingInfo        - logging message limited to TRANSPORT_EVENT_LOG_LEN len
 *          pObjName            - the name of the object that sends the message
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSendStateObjectEvent(
                        IN RvSipTransportMgrHandle             hTransportMgr,
                        IN void*                               pObj,
                        IN RvInt32                             objUniqueIdentifier,
                        IN RvInt32                             param1,
                        IN RvInt32                             param2,
                        IN SipTransportObjectStateEventHandler func,
                        IN RvBool                              bInternal,
                        IN const RvChar*                       pLoggingInfo,
                        IN const RvChar*                       pObjName)

{
    return TransportSendObjectEvent(hTransportMgr, pObj, objUniqueIdentifier, NULL, 
                                  param1, param2, NULL, func, bInternal, pLoggingInfo, pObjName);
}




#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransportMgrSetSigCompMgrHandle
 * ------------------------------------------------------------------------
 * General: Stores the SigComp manager handle in the transport manager
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr   - Handle to the stack  transport manager.
 *           hSigCompMgr     - Handle of the SigComp manager handle to be
 *                             stored.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportMgrSetSigCompMgrHandle(
                      IN RvSipTransportMgrHandle hTransportMgr,
                      IN RvSigCompMgrHandle      hSigCompMgr)
{
    TransportMgr *pTransportMgr;
    RvStatus      rv; 

    if(NULL == hTransportMgr || NULL == hSigCompMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = (TransportMgr *)hTransportMgr;

    RvMutexLock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr); /*lock the manager*/
    /* Verify that no SigComp activity takes places currently within the SIP Stack */ 
    if (RA_NumOfUsedElements(pTransportMgr->hMessageMemoryBuffPool) != 0)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportMgrSetSigCompMgrHandle - The SigComp manager couldn't be set in the middle of Transport layer activities"));
        rv = RV_ERROR_ILLEGAL_ACTION;
        /* NOTE: Don't add 'return rv' here since the Transport manager lock must be unlocked */ 
    }
    else
    {
        pTransportMgr->hSigCompMgr = hSigCompMgr;
        rv                         = RV_OK;
    }
    RvMutexUnlock(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr); /*unlock the manager*/

    return rv;
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------
                         LOCAL ADDRESS FUNCTIONS
  -----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransportLocalAddressGetDefaultAddrByType
 * ----------------------------------------------------------------------------------
 * General: Get a default local address according to transaport and address type.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          eTransportType   - required transport
 *          eAddressType     - required address type.
 *          pLocalAddresses   - handle to the matching local addresses.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetDefaultAddrByType(
                            IN RvSipTransportMgrHandle      hTransportMgr,
                            IN RvSipTransport               eTransportType,
                            IN RvSipTransportAddressType    eAddressType,
                            IN RvSipTransportLocalAddrHandle  *phLocalAddress)
{
    TransportMgr                *pTransportMgr;

#ifdef SIP_DEBUG
    if (hTransportMgr  == NULL || phLocalAddress==NULL)
    {
        return RV_ERROR_BADPARAM;
    }
#endif /*SIP_DEBUG*/
      pTransportMgr = (TransportMgr *)hTransportMgr;

    TransportMgrGetDefaultLocalAddress(pTransportMgr,eTransportType,eAddressType, phLocalAddress);
    if (*phLocalAddress == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }
    return RV_OK;
}

/************************************************************************************
 * SipTransportLocalAddressInitDefaults
 * ----------------------------------------------------------------------------------
 * General: Initialize the local addresses structure with a default local
 *          address of each address/transport type.The default is the first
 *          address found in the local address list.
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          pLocalAddresses   - A structure with all types of local addresses.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressInitDefaults(
                            IN RvSipTransportMgrHandle       hTransportMgr,
                            IN SipTransportObjLocalAddresses *pLocalAddresses)
{
    TransportMgr                    *pTransportMgr;
    SipTransportObjLocalAddresses   localAddressesNull;

#ifdef SIP_DEBUG
    if (hTransportMgr  == NULL || pLocalAddresses==NULL)
    {
        return RV_ERROR_BADPARAM;
    }
#endif /*SIP_DEBUG*/

    pTransportMgr = (TransportMgr *)hTransportMgr;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if( pTransportMgr->bLdaValid )
    {
        *pLocalAddresses = pTransportMgr->Lda;
        return RV_OK;
    }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


    memset(pLocalAddresses,0,sizeof(SipTransportObjLocalAddresses));

    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_UDP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                                       &(pLocalAddresses->hLocalUdpIpv4));

    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_UDP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                                       &(pLocalAddresses->hLocalUdpIpv6));

    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TCP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                                       &(pLocalAddresses->hLocalTcpIpv4));


    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TCP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                                       &(pLocalAddresses->hLocalTcpIpv6));
#if (RV_TLS_TYPE != RV_TLS_NONE)
    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TLS,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                                       &(pLocalAddresses->hLocalTlsIpv4));

    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TLS,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                                       &(pLocalAddresses->hLocalTlsIpv6));
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    TransportMgrGetDefaultLocalAddress(pTransportMgr, RVSIP_TRANSPORT_SCTP,
                                       RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                                       &(pLocalAddresses->hLocalSctpIpv4));
    TransportMgrGetDefaultLocalAddress(pTransportMgr, RVSIP_TRANSPORT_SCTP,
                                       RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                                       &(pLocalAddresses->hLocalSctpIpv6));
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    memset(&localAddressesNull,0,sizeof(SipTransportObjLocalAddresses));
    if(memcmp(pLocalAddresses,&localAddressesNull,sizeof(SipTransportObjLocalAddresses) == 0))
    {
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "SipTransportLocalAddressInitDefaults - No local address found in the SIP Stack"));
        return RV_ERROR_NOT_FOUND;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    pTransportMgr->Lda       = *pLocalAddresses;
    pTransportMgr->bLdaValid = RV_TRUE;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    return RV_OK;
}


/***************************************************************************
 * SipTransportLocalAddrSetHandleInStructure
 * ------------------------------------------------------------------------
 * General: Set a local address handle in a local address
 *          structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Input:   hTransportMgr    - Handle to transport manager
 *          pLocalAddresses  - The local address structure.
 *          hLocalAddr - the local address handle
 *          bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV SipTransportLocalAddrSetHandleInStructure(
                            IN RvSipTransportMgrHandle         hTransportMgr,
                            IN SipTransportObjLocalAddresses  *pLocalAddresses,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN RvBool                          bSetAddrInUse)
{

    TransportMgrLocalAddr          *pLocalAddr      = (TransportMgrLocalAddr*)hLocalAddr;
    RvSipTransportAddressType       eAddressType    = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
    RvSipTransport                  eTransportType;
    RvSipTransportLocalAddrHandle  *phUsedLocalAddr = NULL;
    
    SipTransportLocalAddressGetAddrType(hLocalAddr, &eAddressType);
    eTransportType = pLocalAddr->addr.eTransport;
    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            pLocalAddresses->hLocalUdpIpv4 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalUdpIpv4;
        }
        else
        {
            pLocalAddresses->hLocalUdpIpv6 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalUdpIpv6;
        }
        break;
    case RVSIP_TRANSPORT_TCP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            pLocalAddresses->hLocalTcpIpv4 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalTcpIpv4;
        }
        else
        {
            pLocalAddresses->hLocalTcpIpv6 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalTcpIpv6;
        }
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            pLocalAddresses->hLocalTlsIpv4 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalTlsIpv4;
        }
        else
        {
            pLocalAddresses->hLocalTlsIpv6 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalTlsIpv6;
        }
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            pLocalAddresses->hLocalSctpIpv4 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalSctpIpv4;
        }
        else
        {
            pLocalAddresses->hLocalSctpIpv6 = hLocalAddr;
            phUsedLocalAddr = &pLocalAddresses->hLocalSctpIpv6;
        }
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        break;
    } /*switch (eTransportType)*/
    if(bSetAddrInUse == RV_TRUE)
    {
        pLocalAddresses->hAddrInUse = phUsedLocalAddr;
    }

    RV_UNUSED_ARG(hTransportMgr);
}

/***************************************************************************
 * SipTransportSetLocalAddressForAllTransports
 * ------------------------------------------------------------------------
 * General: gets a string local ip and port and set it to all handles that have
 *          a matching address.
 * Return Value: (RvStatus)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to transport manager
 *            pLocalAddresses  - the local address structure to use.
 *            strLocalAddress - The local address string
 *            localPort - The local port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetLocalAddressForAllTransports(
                            IN  RvSipTransportMgrHandle        hTransportMgr,
                            IN  SipTransportObjLocalAddresses *pLocalAddresses,
                            IN  RvChar                        *strLocalAddress,
                            IN  RvUint16                       localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                      *if_name
			    , IN  RvUint16                     vdevblock
#endif
			    )
{

    RvSipTransportAddressType     eAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    RvSipTransportLocalAddrHandle hLocalAddr   = NULL;
    RvStatus                      rv           = RV_OK;
    RvBool                        bAddrFound   = RV_FALSE;

    if(strLocalAddress == NULL || localPort == 0)
    {
        return RV_ERROR_BADPARAM;
    }

    /*define the address type to be used*/
    if(strLocalAddress[0] == '[')
    {
        eAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }

    /*try to look for a udp address*/
    rv = SipTransportLocalAddressGetHandleByAddress(hTransportMgr,RVSIP_TRANSPORT_UDP,
                                                    eAddressType,strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name,
						    vdevblock,
#endif
                                                    &hLocalAddr);
    if(rv == RV_OK && hLocalAddr != NULL)
    {
        bAddrFound = RV_TRUE;
        if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            pLocalAddresses->hLocalUdpIpv6 = hLocalAddr;
        }
        else
        {
            pLocalAddresses->hLocalUdpIpv4 = hLocalAddr;
        }
    }


    /*try to look for a tcp address*/
    rv = SipTransportLocalAddressGetHandleByAddress(hTransportMgr,RVSIP_TRANSPORT_TCP,
                                                    eAddressType,strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name,
						    vdevblock,
#endif
                                                    &hLocalAddr);
    if(rv == RV_OK && hLocalAddr != NULL)
    {
        bAddrFound = RV_TRUE;
        if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            pLocalAddresses->hLocalTcpIpv6 = hLocalAddr;
        }
        else
        {
            pLocalAddresses->hLocalTcpIpv4 = hLocalAddr;
        }
    }
#if (RV_TLS_TYPE==RV_TLS_OPENSSL)
    /*try to look for a tls address*/
    rv = SipTransportLocalAddressGetHandleByAddress(hTransportMgr,RVSIP_TRANSPORT_TLS,
                                                    eAddressType,strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name,
						    vdevblock,
#endif
                                                    &hLocalAddr);
    if(rv == RV_OK && hLocalAddr != NULL)
    {
        bAddrFound = RV_TRUE;
        if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            pLocalAddresses->hLocalTlsIpv6 = hLocalAddr;
        }
        else
        {
            pLocalAddresses->hLocalTlsIpv4 = hLocalAddr;
        }
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    /*try to look for a SCTP address*/
    rv = SipTransportLocalAddressGetHandleByAddress(hTransportMgr,
						    RVSIP_TRANSPORT_SCTP, eAddressType, strLocalAddress, localPort,
#if defined(UPDATED_BY_SPIRENT) 
						    if_name,
						    vdevblock,
#endif
						    &hLocalAddr);
    if(rv == RV_OK && hLocalAddr != NULL)
    {
        bAddrFound = RV_TRUE;
        if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            pLocalAddresses->hLocalSctpIpv6 = hLocalAddr;
        }
        else
        {
            pLocalAddresses->hLocalSctpIpv4 = hLocalAddr;
        }
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    if(bAddrFound == RV_FALSE)
    {
        return RV_ERROR_NOT_FOUND;
    }
    return RV_OK;
}



/************************************************************************************
 * SipTransportLocalAddressGetAddressByHandle
 * ----------------------------------------------------------------------------------
 * General: returns the string Ip and port for a given local address handle.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          hLocalAddr       - Local address handle
 *          bConvertZeroToDefaultHost - Specifies whether to convert a zero IP to
 *                                      the default host.
 *          bAddScopeIp - Specifies whether to add the scope id to the end of the
 *                         string ip.
 * Output   localIp          - string IP of this local address
 *          localPort        - port of this local address
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetAddressByHandle(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN  RvBool                      bConvertZeroToDefaultHost,
                            IN  RvBool                      bAddScopeIp,
                            OUT RvChar                      *localIp,
                            OUT RvUint16                    *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT RvChar                    *if_name
			    , OUT RvUint16                  *vdevblock
#endif
			    )
{

    TransportMgrLocalAddr *pLocalAddr    = (TransportMgrLocalAddr*)hLocalAddr;
    TransportMgr          *pTransportMgr = (TransportMgr *)hTransportMgr;
    RvStatus               rv;
#ifdef SIP_DEBUG
    if(hLocalAddr == NULL || pTransportMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif /*SIP_DEBUG*/

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportLocalAddressGetAddressByHandle - Failed to lock Local Address 0x%p",pLocalAddr));
        return rv;
    }
    rv = TransportMgrLocalAddressGetIPandPortStrings(
                pTransportMgr,&pLocalAddr->addr.addr,bConvertZeroToDefaultHost,
                bAddScopeIp,localIp,localPort);
#if defined(UPDATED_BY_SPIRENT)
    if(if_name) strcpy(if_name,pLocalAddr->if_name);
    if(vdevblock) *vdevblock = pLocalAddr->vdevblock;
#endif
    TransportMgrLocalAddrUnlock(pLocalAddr);

    return rv;
}

/************************************************************************************
 * SipTransportLocalAddressGetBufferedAddressByHandle
 * ----------------------------------------------------------------------------------
 * General: returns the strings that represents the local address. 
 *          This includes the sentBy string if exist and the IP string and port.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr       - Local address handle
 *          sentByRpoolPtr   - RPool ptr of the sent-by host.
 *          sentByPort       - sent by port
 * Output   localIp          - string IP of this local address
 *          localPort        - port of this local address
 *          
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetBufferedAddressByHandle(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
							IN  RPOOL_Ptr*                     sentByRpoolPtr,
							OUT RvInt32*                       sentByPort,
                            OUT RvChar                         **localIp,
                            OUT RvUint16                       *localPort)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr   = (TransportMgrLocalAddr*)hLocalAddr;
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }
	if(sentByRpoolPtr != NULL)
	{

		if(pLocalAddr->hSentByPage != NULL_PAGE)
		{
			sentByRpoolPtr->hPool = pLocalAddr->pMgr->hElementPool;
			sentByRpoolPtr->hPage = pLocalAddr->hSentByPage;
			sentByRpoolPtr->offset = pLocalAddr->sentByHostOffset;
			*sentByPort = pLocalAddr->sentByPort;
		}
	}
    *localIp    = pLocalAddr->strLocalAddress;
    *localPort  = RvAddressGetIpPort(&pLocalAddr->addr.addr);
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}

/************************************************************************************
 * SipTransportLocalAddressGetHandleByAddress
 * ----------------------------------------------------------------------------------
 * General: Returns the handle of a local address according to given local address
 *          characteristics.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          eTransportType   - tcp/udp
 *          eAddressType     - ip4/ip6
 *          strLocalIp       - string IP of this local address
 *          localPort        - port of this local address
 *          if_name          - local interface name
 *          vdevblock           - local hardware port
 * Output   hLocalAddr      - local address handle
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetHandleByAddress(
                            IN  RvSipTransportMgrHandle         hTransportMgr,
                            IN RvSipTransport                   eTransportType,
                            IN RvSipTransportAddressType        eAddressType,
                            IN  RvChar                         *strLocalIp,
                            IN  RvUint16                        localPort,
#if defined(UPDATED_BY_SPIRENT)
                            IN  RvChar                         *if_name,
			    IN  RvUint16                        vdevblock,
#endif
                            OUT RvSipTransportLocalAddrHandle  *phLocalAddr)

{
    TransportMgrLocalAddr   localAddr;
       TransportMgr            *pTransportMgr=(TransportMgr *)hTransportMgr;
    RvStatus               rv;

    if(pTransportMgr == NULL || phLocalAddr == NULL || strLocalIp == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phLocalAddr = NULL;

    if(eTransportType == RVSIP_TRANSPORT_UNDEFINED ||
       eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportLocalAddressGetHandleByAddress - Failed - invalid parameter"));
        return RV_ERROR_BADPARAM;
    }

    /*initialize a local address structure*/
    rv = TransportMgrInitLocalAddressStructure(pTransportMgr,eTransportType,
                                               eAddressType,strLocalIp,localPort,
#if defined(UPDATED_BY_SPIRENT)
					       if_name,
					       vdevblock,
#endif
                                               &localAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportLocalAddressGetHandleByAddress - Failed to initialize local address structure"));
        return rv;
    }
    /*find an equal structure in the local address list and get the handle*/
    TransportMgrFindLocalAddressHandle(pTransportMgr,&localAddr,phLocalAddr);
    if(*phLocalAddr == NULL)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "SipTransportLocalAddressGetHandleByAddress - local address was not found (ip=%s,port=%d,transport=%s, nettype=%s)",
                 strLocalIp,localPort,
                 SipCommonConvertEnumToStrTransportType(eTransportType),
                 SipCommonConvertEnumToStrTransportAddressType(eAddressType)));
        return RV_ERROR_NOT_FOUND;
    }
    return RV_OK;
}

/************************************************************************************
 * SipTransportLocalAddressGetTransportType
 * ----------------------------------------------------------------------------------
 * General: Returns the transport type of a local address
 * Return Value: RvSipTransport
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - Local address handle
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetTransportType(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvSipTransport* pTransportType)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if(NULL == hLocalAddr || NULL == pTransportType)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }
    *pTransportType = pLocalAddr->addr.eTransport;
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}

#if(RV_NET_TYPE & RV_NET_IPV6)
/************************************************************************************
 * SipTransportLocalAddressGetScopeId
 * ----------------------------------------------------------------------------------
 * General: Returns the scope ID of an Ipv6 address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 *          hLocalAddr      - Local address handle
 *          pScodeId  -       scope id
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetScopeId(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvInt32                    *pScodeId)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
       TransportMgr          *pTransportMgr=(TransportMgr *)hTransportMgr;
#ifdef SIP_DEBUG
    if(hLocalAddr == NULL || pTransportMgr == NULL || pScodeId == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif /*SIP_DEBUG*/

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (RV_ADDRESS_TYPE_IPV6 != RvAddressGetType(&pLocalAddr->addr.addr))
    {
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return RV_ERROR_BADPARAM;
    }
    *pScodeId = RvAddressGetIpv6Scope(&pLocalAddr->addr.addr);
    TransportMgrLocalAddrUnlock(pLocalAddr);
	RV_UNUSED_ARG(pTransportMgr);

    return RV_OK;

}
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

/************************************************************************************
 * SipTransportLocalAddressGetPort
 * ----------------------------------------------------------------------------------
 * General: Returns the transport type of a local address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 *          hLocalAddr      - Local address handle
 *          pPort           - port
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetPort(
                            IN  RvSipTransportMgrHandle        hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvUint16                      *pPort)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
#ifdef SIP_DEBUG
    if(hLocalAddr == NULL || pPort == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif /*SIP_DEBUG*/
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    *pPort = RvAddressGetIpPort(&pLocalAddr->addr.addr);
    TransportMgrLocalAddrUnlock(pLocalAddr);

    RV_UNUSED_ARG(hTransportMgr);

    return RV_OK;
}


/************************************************************************************
 * SipTransportLocalAddressGetAddrType
 * ----------------------------------------------------------------------------------
 * General: Returns the address type of a local address
 * Return Value:RvSipTransportAddressType
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - local address handle
 * Output   (-)
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetAddrType(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvSipTransportAddressType    *pAddressType)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;

    if(NULL == hLocalAddr  ||  NULL == pAddressType)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Type of SCTP Local Address object is stored in eSockAddrType field.
       Local Addresses of other transports store their type in addr itself. */
    *pAddressType = pLocalAddr->eSockAddrType;

    if (RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == *pAddressType)
    {
        *pAddressType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pLocalAddr->addr.addr));
    }

    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}


/************************************************************************************
 * SipTransportLocalAddressGetFirstAvailAddress
 * ----------------------------------------------------------------------------------
 * General: returns the address of the first defined local address.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 * Output:  addr            - address structure
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetFirstAvailAddress(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  SipTransportAddress*         addr)
{

    TransportMgr*                pTransportMgr   = (TransportMgr*)hTransportMgr;
    TransportMgrLocalAddr*      pLocalAddr;
    RvSipTransportLocalAddrHandle hLocalAddr      = NULL;
    RvStatus rv;

#ifdef SIP_DEBUG
    if(addr == NULL || pTransportMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif /*SIP_DEBUG*/


    /*try to get udp/ip address*/
    TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_UDP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,&hLocalAddr);

#if(RV_NET_TYPE & RV_NET_IPV6)
    if(hLocalAddr == NULL)
    {
        /*try to get udp ipv6*/
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_UDP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,&hLocalAddr);
    }
#endif

    if(hLocalAddr == NULL)
    {
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TCP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,&hLocalAddr);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if(hLocalAddr == NULL)
    {
        /*try to get udp ipv6*/
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TCP,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,&hLocalAddr);
    }
#endif
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(hLocalAddr == NULL)
    {
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TLS,RVSIP_TRANSPORT_ADDRESS_TYPE_IP,&hLocalAddr);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if(hLocalAddr == NULL)
    {
        /*try to get udp ipv6*/
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_TLS,RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,&hLocalAddr);
    }
#endif
#endif /*(RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    if(hLocalAddr == NULL)
    {
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_SCTP,
            RVSIP_TRANSPORT_ADDRESS_TYPE_IP,&hLocalAddr);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if(hLocalAddr == NULL)
    {
        /*try to get SCTP ipv6*/
        TransportMgrGetDefaultLocalAddress(pTransportMgr,RVSIP_TRANSPORT_SCTP,
            RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,&hLocalAddr);
    }
#endif
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
    if(hLocalAddr == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }

    pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }
    *addr = pLocalAddr->addr;
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}

/*-----------------------------------------------------------------------
                         OUTBOUND ADDRESS FUNCTIONS
  -----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransportGetOutboundAddress
 * ----------------------------------------------------------------------------------
 * General: returns the outbound address information.
 * Return Value: host name
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 * Output:  strIp           - outbound address ip
 *          strHost         - outbound address host name
 *          pPort           - outbound address port
 *          peTransport     - outbound address transport
 *          peCompType      - outbound compression type
 *          strSigcompId    - outbound sigcomp-id
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportGetOutboundAddress(
                            IN  RvSipTransportMgrHandle   hTransportMgr,
                            OUT RvChar**                  strIp,
                            OUT RvChar**                  strHost,
                            OUT RvUint16*                 pPort,
                            OUT RvSipTransport*           peTransport,
                            OUT RvSipCompType*            peCompType,
							OUT RvChar**                  strSigcompId)
{
    TransportMgr          *pTransportMgr = (TransportMgr *)hTransportMgr;
#ifdef SIP_DEBUG
    if(pTransportMgr == NULL || strIp == NULL || strHost == NULL ||
       pPort == NULL || peTransport == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif /*SIP_DEBUG*/

    *strIp        = pTransportMgr->outboundAddr.strIp;
    *strHost      = pTransportMgr->outboundAddr.strHostName;
    *pPort        = pTransportMgr->outboundAddr.port;
    *peTransport  = pTransportMgr->outboundAddr.transport;
#ifdef RV_SIGCOMP_ON
    *peCompType   = pTransportMgr->outboundAddr.compType;
	*strSigcompId = pTransportMgr->outboundAddr.strSigcompId;
#else
    RV_UNUSED_ARG(peCompType);
	RV_UNUSED_ARG(strSigcompId);
#endif /*#ifdef RV_SIGCOMP_ON*/

    return RV_OK;
}
/***************************************************************************
 * SipTransportInitObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Initialize an outbound addr for a stack object such as call-leg
 *          transaction or Reg-Client.
 * Returned value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 * Input Output:  outboundAddr - An outbound addr structure
 ***************************************************************************/
void RVCALLCONV SipTransportInitObjectOutboundAddr (
                     IN    RvSipTransportMgrHandle     hTransportMgr,
                     INOUT SipTransportOutboundAddress *outboundAddr)
{
    TransportMgr          *pTransportMgr = (TransportMgr *)hTransportMgr;
    memset(outboundAddr,0, sizeof(SipTransportOutboundAddress));
    if(pTransportMgr->outboundAddr.strHostName != NULL)
    {
        outboundAddr->bUseHostName = RV_TRUE;
    }
#ifdef RV_SIGCOMP_ON
    outboundAddr->eCompType             = RVSIP_COMP_UNDEFINED;
	outboundAddr->rpoolSigcompId.offset = UNDEFINED;
#endif
    outboundAddr->ipAddress.eTransport = RVSIP_TRANSPORT_UNDEFINED;
    memset(&outboundAddr->ipAddress.addr,0,sizeof(outboundAddr->ipAddress.addr));
    RvAddressConstruct(RV_ADDRESS_TYPE_NONE,&outboundAddr->ipAddress.addr);
    outboundAddr->rpoolHostName.offset = UNDEFINED;

}

/***************************************************************************
 * SipTransportGetObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Copies the used outbound address ip and port to given parameters.
 *          First try to locate an outbound address in the object outbound
 *          address structure. If there is no outbound address in the object
 *          take the outbound address that was configured for the stack.
 *          If there is no outbound address for the stack reset the given
 *          parameters and return success.
 * Returned value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 * Output:    outboundAddrIp   -  A buffer with the outbound address IP used by the object.
 *          pOutboundProxyPort - The outbound address port
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundAddr (
                     IN    RvSipTransportMgrHandle     hTransportMgr,
                     IN    SipTransportOutboundAddress *pOutboundAddr,
                     OUT   RvChar                     *strOutboundAddrIp,
                     OUT   RvUint16                   *pOutboundAddrPort)
{

    TransportMgr         *pTransportMgr = (TransportMgr *)hTransportMgr;

    strOutboundAddrIp[0] = '\0';
    *pOutboundAddrPort = 0;

    /*no outbound address in the object. take it from the manager*/
    if(pOutboundAddr->bIpAddressExists == RV_FALSE)
    {
        if(pTransportMgr->outboundAddr.strIp != NULL)
        {
            strcpy(strOutboundAddrIp,pTransportMgr->outboundAddr.strIp);
            *pOutboundAddrPort = pTransportMgr->outboundAddr.port;
        }
        return RV_OK;
    }

    /*take address from the object*/
    if (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(&pOutboundAddr->ipAddress.addr))
    {
        RvAddressGetString(&pOutboundAddr->ipAddress.addr,46,strOutboundAddrIp);
        *pOutboundAddrPort = RvAddressGetIpPort(&pOutboundAddr->ipAddress.addr);
    }

#if(RV_NET_TYPE & RV_NET_IPV6)
    else if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&pOutboundAddr->ipAddress.addr))
    {
        if ( (RvAddressIsNull(&pOutboundAddr->ipAddress.addr)) &&
             (RvAddressGetIpPort(&pOutboundAddr->ipAddress.addr) == 0))
        {
            strcpy(strOutboundAddrIp,pTransportMgr->outboundAddr.strIp);
            *pOutboundAddrPort = pTransportMgr->outboundAddr.port;
        }
        else
        {
            RvAddressGetString(&pOutboundAddr->ipAddress.addr,46,strOutboundAddrIp);
            *pOutboundAddrPort = RvAddressGetIpPort(&pOutboundAddr->ipAddress.addr);
        }
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    else
    {
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "SipTransportGetObjectOutboundAddr - Object with ip outbound address with no transport type"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}
/***************************************************************************
 * SipTransportSetObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 *            strOutboundAddrIp   - The outbound ip to be set to this object.
 *          outboundPort  - The outbound port to be set to this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundAddr(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                      *strOutboundAddrIp,
                            IN  RvInt32                     outboundPort)
{
    TransportMgr         *pTransportMgr;
    SipTransportAddress   tempOutboundAddr; /*the temp address will be copied to the
                                            object address only on success*/
    RvStatus           rv = RV_OK;

    pTransportMgr = (TransportMgr *)hTransportMgr;
    if (outboundPort == UNDEFINED)
    {
		outboundPort = (pOutboundAddr->ipAddress.eTransport == RVSIP_TRANSPORT_TLS ? RVSIP_TRANSPORT_DEFAULT_TLS_PORT : RVSIP_TRANSPORT_DEFAULT_PORT);
    }
    

    rv = SipTransportString2Address(strOutboundAddrIp,
                                    &tempOutboundAddr,
                                    SipTransportISIpV4(strOutboundAddrIp));
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "SipTransportSetObjectOutboundAddr - Failed to convert string to ip address, rv=%d",rv));
        return rv;
    }

    RvAddressSetIpPort(&tempOutboundAddr.addr,(RvUint16)outboundPort);

    RvAddressCopy(&tempOutboundAddr.addr,&pOutboundAddr->ipAddress.addr);
    pOutboundAddr->bUseHostName     = RV_FALSE;
    pOutboundAddr->bIpAddressExists = RV_TRUE;
    return RV_OK;
}


/***************************************************************************
 * SipTransportSetObjectOutboundHost
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 *            strOutboundHost   - The outbound host to be set to this object.
 *          outboundPort  - The outbound port to be set to this object.
 *          hObjPool      - The memory pool the object uses.
 *          hObjPage      - The memory page the object uses.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundHost(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                     *strOutboundHost,
                            IN  RvUint16                   outboundPort,
                            IN  HRPOOL                      hObjPool,
                            IN  HPAGE                       hObjPage)
{
    RvStatus       rv             =RV_OK;
    TransportMgr    *pTransportMgr;
    RvInt32                offset;

    pTransportMgr = (TransportMgr *)hTransportMgr;
    if (outboundPort == 0)
    {
        outboundPort = (RvUint16)UNDEFINED;
    }

    rv = RPOOL_AppendFromExternal(hObjPool, hObjPage,
                                  strOutboundHost, (RvInt)strlen(strOutboundHost)+1, RV_FALSE,
                                  &offset, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "SipTransportSetObjectOutboundHost - Failed to allocate host on object page 0x%p (rv=%d)", hObjPage,rv));
        return rv;
    }


    pOutboundAddr->rpoolHostName.offset = offset;
    pOutboundAddr->rpoolHostName.hPage = hObjPage;
    pOutboundAddr->rpoolHostName.hPool = hObjPool;
    /* we don't know it the host is of ipv4 or of ipv6, we gamble on ipv4,
       since we do not check it on outbound host */
    RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&pOutboundAddr->ipAddress.addr);
    RvAddressSetIpPort(&pOutboundAddr->ipAddress.addr,outboundPort);
    pOutboundAddr->bUseHostName = RV_TRUE;
#ifdef SIP_DEBUG
    pOutboundAddr->strHostName = RPOOL_GetPtr(pOutboundAddr->rpoolHostName.hPool,
                                             pOutboundAddr->rpoolHostName.hPage,
                                             pOutboundAddr->rpoolHostName.offset);
#endif
    return RV_OK;
}

/***************************************************************************
 * SipTransportGetObjectOutboundHost
 * ------------------------------------------------------------------------
 * General: Copies the used outbound host and port to given parameters.
 *          First try to locate an outbound host in the object outbound
 *          address structure. If there is no outbound host in the object
 *          take the outbound host that was configured for the stack.
 *          If there is no outbound host for the stack reset the given
 *          parameters and return success.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 *            strOutboundHostName   - The outbound host used by this object.
 *          pOutboundPort  - The outbound port used by this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundHost(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvChar                     *strOutboundHostName,
                            OUT RvUint16                   *pOutboundPort)
{
    TransportMgr    *pTransportMgr = (TransportMgr *)hTransportMgr;

    strOutboundHostName[0] = '\0';
    *pOutboundPort = 0;

    if(pOutboundAddr->bUseHostName == RV_TRUE)
    {
        if(pOutboundAddr->rpoolHostName.hPool != NULL &&
           pOutboundAddr->rpoolHostName.hPage != NULL_PAGE &&
           pOutboundAddr->rpoolHostName.offset != UNDEFINED)
        {

            RvInt32  actualSize = 0;
            RvStatus rv = RV_OK;
            actualSize = RPOOL_Strlen(pOutboundAddr->rpoolHostName.hPool,
                                      pOutboundAddr->rpoolHostName.hPage,
                                      pOutboundAddr->rpoolHostName.offset);
            rv = RPOOL_CopyToExternal(pOutboundAddr->rpoolHostName.hPool,
                                      pOutboundAddr->rpoolHostName.hPage,
                                      pOutboundAddr->rpoolHostName.offset,
                                       strOutboundHostName,
                                      actualSize+1);
            if(rv != RV_OK)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                          "SipTransportGetObjectOutboundHost - Failed to copy outbound host to buff (rv=%d)", rv));
                return rv;
            }
            *pOutboundPort = RvAddressGetIpPort(&pOutboundAddr->ipAddress.addr);
        }
        else
        {
            if(pTransportMgr->outboundAddr.strHostName != NULL)
            {
                strcpy(strOutboundHostName,pTransportMgr->outboundAddr.strHostName);
                *pOutboundPort = pTransportMgr->outboundAddr.port;
            }
        }
    }
    return RV_OK;
}

/***************************************************************************
 * SipTransportGetObjectOutboundTransportType
 * ------------------------------------------------------------------------
 * General: returns the transport of the outbound address the object is using.
 *          If the object does not have and outbound address, the transport is
 *          taken from the configured outbound proxy. If and outbound proxy was
 *          not configured undefined is returns.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 * Output:    eOutboundProxyTransport   - The outbound transport type
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundTransportType(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvSipTransport       *eOutboundProxyTransport)
{

    TransportMgr    *pTransportMgr = (TransportMgr *)hTransportMgr;

    *eOutboundProxyTransport = RVSIP_TRANSPORT_UNDEFINED;
    if(pOutboundAddr->bUseHostName == RV_TRUE &&
       RPOOL_IsValidPtr(&pOutboundAddr->rpoolHostName))
    {
        *eOutboundProxyTransport = pOutboundAddr->ipAddress.eTransport;
    }
    else if(pOutboundAddr->bUseHostName == RV_FALSE &&
        pOutboundAddr->bIpAddressExists == RV_TRUE)
    {
        *eOutboundProxyTransport = pOutboundAddr->ipAddress.eTransport;
    }
    else /*take the transport from the manager*/
    {
        *eOutboundProxyTransport = pTransportMgr->outboundAddr.transport;
    }
    return RV_OK;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransportGetObjectOutboundCompressionType
 * ------------------------------------------------------------------------
 * General: returns the compression type of the outbound address the object
 *          is using. If the object does not have and outbound address, the
 *          compression type is taken from the configured outbound proxy.
 *          If and outbound proxy was not configured undefined is returned.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structure of a stack object
 * Output:    peOutboundProxyComp - The outbound compression type
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundCompressionType(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvSipCompType              *peOutboundProxyComp)
{
    TransportMgr *pTransportMgr = (TransportMgr *)hTransportMgr;

    *peOutboundProxyComp = RVSIP_COMP_UNDEFINED;

    if(pOutboundAddr->bUseHostName == RV_TRUE &&
       RPOOL_IsValidPtr(&pOutboundAddr->rpoolHostName))
    {
        *peOutboundProxyComp = pOutboundAddr->eCompType;
    }
    else if(pOutboundAddr->bUseHostName == RV_FALSE &&
        pOutboundAddr->bIpAddressExists == RV_TRUE)
    {
        *peOutboundProxyComp = pOutboundAddr->eCompType;
    }
    else /*take the transport from the manager*/
    {
        *peOutboundProxyComp = pTransportMgr->outboundAddr.compType;
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransportSetObjectOutboundSigcompId
 * ------------------------------------------------------------------------
 * General: Sets the outbound sigcomp-id string for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr			- The transport manager.
 *          pOutboundAddr			- An outbound addr structure of a stack object
 *          strOutboundSigcompId	- The outbound host to be set to this object.
 *          hObjPool				- The memory pool the object uses.
 *          hObjPage				- The memory page the object uses.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundSigcompId(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                      *strOutboundSigcompId,
                            IN  HRPOOL                      hObjPool,
                            IN  HPAGE                       hObjPage)
{
    RvStatus       rv             =RV_OK;
    TransportMgr  *pTransportMgr;
    RvInt32        offset;

    pTransportMgr = (TransportMgr *)hTransportMgr;

    rv = RPOOL_AppendFromExternal(hObjPool, hObjPage,
                                  strOutboundSigcompId, (RvInt)strlen(strOutboundSigcompId)+1, RV_FALSE,
                                  &offset, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "SipTransportSetObjectOutboundSigcompId - Failed to allocate sigcomp-id on object page 0x%p (rv=%d)", hObjPage,rv));
        return rv;
    }


    pOutboundAddr->rpoolSigcompId.offset = offset;
    pOutboundAddr->rpoolSigcompId.hPage = hObjPage;
    pOutboundAddr->rpoolSigcompId.hPool = hObjPool;
    /* we don't know it the host is of ipv4 or of ipv6, we gamble on ipv4,
       since we do not check it on outbound host */
#ifdef SIP_DEBUG
    pOutboundAddr->strSigcompId = RPOOL_GetPtr(pOutboundAddr->rpoolSigcompId.hPool,
                                               pOutboundAddr->rpoolSigcompId.hPage,
                                               pOutboundAddr->rpoolSigcompId.offset);
#endif
    return RV_OK;
}

/***************************************************************************
 * SipTransportGetObjectOutboundSigcompId
 * ------------------------------------------------------------------------
 * General: Copies the used outbound sigcomp-id to given parameters.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *           pOutboundAddr - An outbound addr structure of a stack object
 *           strOutboundSigcompId - The outbound sigcompId used by this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundSigcompId(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvChar                      *strOutboundSigcompId)
{
    TransportMgr    *pTransportMgr = (TransportMgr *)hTransportMgr;

    strOutboundSigcompId[0] = '\0';

	if(pOutboundAddr->rpoolSigcompId.hPool != NULL &&
		pOutboundAddr->rpoolSigcompId.hPage != NULL_PAGE &&
		pOutboundAddr->rpoolSigcompId.offset != UNDEFINED)
	{
		
		RvInt32  actualSize = 0;
		RvStatus rv         = RV_OK;

		actualSize = RPOOL_Strlen(pOutboundAddr->rpoolSigcompId.hPool,
			pOutboundAddr->rpoolSigcompId.hPage,
			pOutboundAddr->rpoolSigcompId.offset);

		rv = RPOOL_CopyToExternal(pOutboundAddr->rpoolSigcompId.hPool,
			pOutboundAddr->rpoolSigcompId.hPage,
			pOutboundAddr->rpoolSigcompId.offset,
			strOutboundSigcompId,
			actualSize+1);
		if(rv != RV_OK)
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"SipTransportGetObjectOutboundSigcompId - Failed to copy outbound sigcomp-id to buff (rv=%d)", rv));
			return rv;
		}
	}
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}

#endif /* RV_SIGCOMP_ON */

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SipTransportGetMsgTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given msgType
 * Return Value: The string with the msg type name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eMsgType -  message type
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransportGetMsgTypeName(
                              IN SipTransportMsgType    eMsgType)

{
    switch(eMsgType)
    {
    case SIP_TRANSPORT_MSG_REQUEST_SENT:
        return "Request Sent";
    case SIP_TRANSPORT_MSG_ACK_SENT:
        return "Ack Sent";
    case SIP_TRANSPORT_MSG_FINAL_RESP_SENT:
        return "Final Response Sent";
    case SIP_TRANSPORT_MSG_PROV_RESP_REL_SENT:
        return "Provisional Response Reliable Sent";
    case SIP_TRANSPORT_MSG_OUT_OF_CONTEXT:
        return "Out Of Context";
    case SIP_TRANSPORT_MSG_PROXY_2XX_SENT:
        return "Proxy 2xx Sent";
    case SIP_TRANSPORT_MSG_PROV_RESP_SENT:
        return "Provisional Response Sent";
    case SIP_TRANSPORT_MSG_APP:
        return "App";
    case SIP_TRANSPORT_MSG_UNDEFINED:
        return "UNDEFINED";
    default:
        return "Unknown msg type";
    }
}

/***************************************************************************
 * SipTransportGetCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given compression type
 * Return Value: The string with the address compression type name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eCompType  - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransportGetCompTypeName(
                                  IN RvSipCompType    eCompType)
{
    switch (eCompType)
    {
    case RVSIP_COMP_SIGCOMP:
        return "sigcomp";
    case RVSIP_COMP_OTHER:
        return "other comp";
    default:
        return "Unknown compression type";
    }
}

/************************************************************************************
 * SipTransportMsgBuilderPrintMsg
 * ----------------------------------------------------------------------------------
 * General: log print function. used only for printing the sent buffer to the log.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to the transport manager.
 *          pBuf             - the buffer that was received from the network.
 *          msgDirection     - The direction of the printed message.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportMsgBuilderPrintMsg(
                    IN RvSipTransportMgrHandle           hTransportMgr,
                    IN RvChar                           *pBuf,
                    IN SipTransportMgrBuilderPrintMsgDir msgDirection)
{
    TransportMgr *pTransportMgr = (TransportMgr *)hTransportMgr;

    TransportMsgBuilderPrintMsg(pTransportMgr,pBuf,msgDirection,RV_TRUE);
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/





/************************************************************************************
 * SipTransportUdpSendMessage
 * ----------------------------------------------------------------------------------
 * General: Send Message on udp socket after copying it to a local buffer
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          hPool               - handle to the message rpool
 *          element             - rpool element that is going to be sent
 *          hLocalAddr          - The local address to send the message from.
 *          pAddress            - The address to send the message to.
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportUdpSendMessage(
                          IN RvSipTransportMgrHandle     hTransportHandle,
                          IN HRPOOL                      hPool,
                          IN HPAGE                       element,
                          IN RvSipTransportLocalAddrHandle hLocalAddr,
                          IN RvAddress*                  pAddress)
{
    TransportMgr    *transportInfo = (TransportMgr*)hTransportHandle;
    RvSize_t        bytesSend = 0;
    RvStatus        status;
    RvChar          *buffer;
    RvUint32        size;
    RvBool          bSendMsg                        = RV_TRUE;
    RvSipTransportLocalAddrHandle hUsedLocalAddr    = hLocalAddr;
    TransportMgrLocalAddr *pLocalAddr;
    RvBool          bDiscardMsg = RV_FALSE;
    SipTransportAddress sipDestAddress;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvStatus          rv = RV_OK;
    RvChar            localIp[RVSIP_TRANSPORT_LEN_STRING_IP]  = {'\0'};
    RvChar            remoteIp[RVSIP_TRANSPORT_LEN_STRING_IP] = {'\0'};
#if defined(UPDATED_BY_SPIRENT)
    RvChar            if_name[RVSIP_TRANSPORT_IFNAME_SIZE] = {'\0'};
    RvUint16          vdevblock=0;
#endif
    RvUint16          localPort    = 0;
    RvUint16          remotePort   = 0;
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


    if(hUsedLocalAddr == NULL)
    {
        RvLogExcep(transportInfo->pLogSrc,(transportInfo->pLogSrc,
               "SipTransportUdpSendMessage - Failed - no local address"));
        return RV_ERROR_UNKNOWN;
    }


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    rv = SipTransportLocalAddressGetAddressByHandle(hTransportHandle,hUsedLocalAddr,
						    RV_FALSE,RV_TRUE, localIp, &localPort
#if defined(UPDATED_BY_SPIRENT)
						    ,if_name
						    ,&vdevblock
#endif
						    );
    if (RV_OK != rv)
    {
        RvLogError(transportInfo->pLogSrc,(transportInfo->pLogSrc,
            "SipTransportUdpSendMessage - Failed to get info for local address 0x%p (rv=%d)",
            hUsedLocalAddr,rv));
        return rv;
    }
    RvAddressGetString(pAddress,RVSIP_TRANSPORT_LEN_STRING_IP,remoteIp);
    remotePort  = RvAddressGetIpPort(pAddress);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    pLocalAddr = (TransportMgrLocalAddr*)hUsedLocalAddr;
    RvLogInfo(transportInfo->pLogSrc,(transportInfo->pLogSrc,
               "SipTransportUdpSendMessage - Sending message. local-address=%s:%d. remote-address=%s:%d",
               localIp, localPort, remoteIp, remotePort));

    RvMutexLock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
    size = (RvUint32)RPOOL_GetSize(hPool, element);
    if (size == 0)
    {
        RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
        return RV_ERROR_BADPARAM;
    }
    if (size > transportInfo->maxBufSize)
    {
        RvLogError(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                  "SipTransportUdpSendMessage - The transport buffer (%d) is shorter than the message to send (%d)",transportInfo->maxBufSize,size));
        RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    /* copying the send buffer */
    status = RPOOL_CopyToExternal(hPool,
                                  element,
                                  0,
                                  (void*)(transportInfo->sendBuffer),
                                  size);


    if (status != RV_OK)
    {
        RvLogError(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                   "SipTransportUdpSendMessage - Can not copy buffer"));
        RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
        return status;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#if(RV_NET_TYPE & RV_NET_IPV6)
    // set traffic class for IPv6 according to TOS byte from configuration
    if (pAddress->addrtype == RV_ADDRESS_TYPE_IPV6) {
        pAddress->data.ipv6.flowinfo = ((unsigned long) pLocalAddr->tos) << 4;
    }
#endif /*RV_IPV6*/
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    buffer = (RvChar*)transportInfo->sendBuffer;
    /* printing the buffer */
    buffer[size] = '\0';

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        {
#if (RV_LOGMASK & RV_LOGLEVEL_DEBUG)
            RvChar localip[RVSIP_TRANSPORT_LEN_STRING_IP];
            RvChar remoteip[RVSIP_TRANSPORT_LEN_STRING_IP];
#endif /*RV_LOGLEVEL_DEBUG*/      
            /*lint -e578*/
            RvStatus rv;
            rv = TransportMgrLocalAddrLock(pLocalAddr);
            if (RV_OK != rv)
            {
                RvLogWarning(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                    "SipTransportUdpSendMessage - 1-Can not lock local address 0x%p, Probably it was closed (rv=%d)",pLocalAddr,rv));
                RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
                return rv;
            }
            RvLogDebug(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                       "SipTransportUdpSendMessage - sock %d: Sending UDP message. %s:%d->%s:%d, size=%d",
                       (RvInt)(pLocalAddr->ccSocket),
                       RvAddressGetString(&pLocalAddr->addr.addr, RVSIP_TRANSPORT_LEN_STRING_IP, localip),
                       RvAddressGetIpPort(&pLocalAddr->addr.addr),
                       RvAddressGetString(pAddress, RVSIP_TRANSPORT_LEN_STRING_IP, remoteip),
                       RvAddressGetIpPort(pAddress),
                       size));
            TransportMgrLocalAddrUnlock(pLocalAddr);
            /* printing the buffer */
            TransportMsgBuilderPrintMsg(transportInfo,
                                        buffer,
                                        SIP_TRANSPORT_MSG_BUILDER_OUTGOING,
                                        RV_FALSE);
        }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


    /* Get permition from the application to send the message */
    bSendMsg = TransportCallbackMsgToSendEv(transportInfo,buffer,size);
    sipDestAddress.addr = *pAddress;
    sipDestAddress.eTransport = RVSIP_TRANSPORT_UDP;
    TransportCallbackBufferToSend(transportInfo, pLocalAddr, &sipDestAddress,
                                  NULL, buffer, size, &bDiscardMsg);
    if (RV_TRUE == bSendMsg  &&  RV_FALSE == bDiscardMsg)
    {
        /*lint -e578*/
        RvStatus rv;
        RvStatus crv = RV_OK;
        rv = TransportMgrLocalAddrLock(pLocalAddr);
        if (RV_OK != rv)
        {
            RvLogWarning(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                "SipTransportUdpSendMessage - 2-Can not lock local address 0x%p, Probably it was closed (rv=%d)",pLocalAddr,rv));
            RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
            return rv;
        }
        /* the message is sent in one buffer */
        if (pLocalAddr->pSocket != NULL)
        {
            crv = RvSocketSendBuffer(pLocalAddr->pSocket,
                                    (RvUint8*)buffer,
                                    size,
                                    pAddress,
                                    transportInfo->pLogMgr,
                                    &bytesSend);
        }
        else
        {
            crv = RV_ERROR_UNKNOWN;
        }
        /* checking if the message was sent properly */
        if (RV_OK != crv)
        {
            RvLogError(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                     "SipTransportUdpSendMessage - Message sending failed (crv=%d)",RvErrorGetCode(crv)));

#if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES)
            TransportUDPReopen(transportInfo, pLocalAddr);
#endif /* #if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES) */

            TransportMgrLocalAddrUnlock(pLocalAddr);
            RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
#ifdef RV_SIP_IMS_ON
            if (NULL != pLocalAddr->pSecOwner)
            {
                TransportCallbackMsgSendFailureEv(transportInfo,
                    element, pLocalAddr, NULL/*pConn*/, pLocalAddr->pSecOwner);
            }
#endif /*#ifdef RV_SIP_IMS_ON*/
            return RV_ERROR_NETWORK_PROBLEM;
        }
        TransportMgrLocalAddrUnlock(pLocalAddr);

        if(bytesSend == 0)
        {
            RvLogDebug(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                     "SipTransportUdpSendMessage - Message was not sent probably because of a WouldBlock error, returning RV_OK"));
            RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
            return RV_OK;
        }
/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting outgoing UDP messages 
*/
#if defined(UPDATED_BY_SPIRENT)
        RvSipRawMessageCounterHandlersRun(SPIRENT_SIP_RAW_MESSAGE_OUTGOING);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
    }
    else
    {
        RvLogInfo(transportInfo->pLogSrc,(transportInfo->pLogSrc,
                  "SipTransportUdpSendMessage - message was not sent due to application request."));
    }

    RvMutexUnlock(&transportInfo->udpBufferLock,transportInfo->pLogMgr);
    return RV_OK;
}

/************************************************************************************
 * SipTransportString2Address
 * ----------------------------------------------------------------------------------
 * General: make RV_LI_AddressType from string
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   strIp - the source string
 *          bIsIpV4   - Defines if the address is ip4 or ip6
 * Output:  address - the address in RV_LI_AddressType format
 ***********************************************************************************/
RvStatus SipTransportString2Address(RvChar* strIp, SipTransportAddress* address, RvBool bIsIpV4)
{
    return TransportMgrConvertString2Address(strIp,address,bIsIpV4,RV_TRUE);
}




#if (RV_TLS_TYPE != RV_TLS_NONE)
/************************************************************************************
 * SipTransportConnectionSetHostStringName
 * ----------------------------------------------------------------------------------
 * General: sets the connection's host name string, so it can compare it to the certificate.
 *          this procedure is only done until the connection has been approved by the
 *          application. once the application approved the connection, it can no longer
 *          change the host name
 *
 * Return Value: RvStatus - RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection    - the connection's handle
 *          strAddress     - the address that the connection will be connected to
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetHostStringName(
               IN  RvSipTransportConnectionHandle         hConnection,
               IN  RvChar*                               strAddress)
{
    RvStatus            rv     = RV_OK;
    TransportConnection *pConn  = (TransportConnection*)hConnection;

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (RV_TRUE == pConn->bConnectionAsserted)
    {
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_OK;
    }
    if(strAddress == NULL || strcmp(strAddress,"")==0)
    {
          RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                   "SipTransportConnectionSetHostStringName - Connection 0x%p: No host was set on TLS connection for connection assertion", pConn));
          pConn->strSipAddrOffset = UNDEFINED;
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
          return RV_OK;
    }
    if(pConn->strSipAddrOffset != UNDEFINED)
    {
        RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "SipTransportConnectionSetHostStringName - Connection 0x%p: already has a host name.", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_OK;
    }
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
               "SipTransportConnectionSetHostStringName - Connection 0x%p: setting sip address to %s", pConn, strAddress));

    if(pConn->hConnPage == NULL)
    {
        rv = RPOOL_GetPage(pConn->pTransportMgr->hElementPool,0,&pConn->hConnPage);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "SipTransportConnectionSetHostStringName - Connection 0x%p: can not allocate page",
                pConn));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return rv;
        }
    }
    rv = RPOOL_AppendFromExternalToPage(pConn->pTransportMgr->hElementPool,
                                        pConn->hConnPage,
                                        (void*)strAddress,
                                        (int)strlen(strAddress)+1,
                                        &pConn->strSipAddrOffset);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                   "SipTransportConnectionSetHostStringName - Connection 0x%p: can not set strAddress %s to page",
                   pConn, strAddress));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
}


#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/***************************************************************************
 * TransportConnectionHashFindByAlias
 * ------------------------------------------------------------------------
 * General: find a connection in the hash table according to given information.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr -    Handle of the transport manager
 *          eTransport    -    The requested transport (Tcp/Tls/Sctp)
 *          hLocalAddr    -    Handle to the requested local address
 *          pDestAddr     -    pointer to the requested destination address
 * Output:  phConn -           Handle to the connection found in the hash or NULL
 *                             if the connection was not found.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionHashFindByAlias(
                         IN RvSipTransportMgrHandle        hTransportMgr,
                         IN RvSipTransport                 eTransport,
                         IN RvSipTransportLocalAddrHandle  hLocalAddress,
                         IN HRPOOL                         hAliasPool,
                         IN HPAGE                          hAliasPage,
                         IN RvInt32                        aliasOffset,
                         OUT RvSipTransportConnectionHandle *phConn)
{
    RPOOL_Ptr pAlias;
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;

    if(aliasOffset == UNDEFINED || hAliasPage == NULL)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConnectionHashFindByAlias - No alias is given"));
        return RV_OK;
    }

    pAlias.hPool = hAliasPool;
    pAlias.hPage = hAliasPage;
    pAlias.offset = aliasOffset;

    TransportConnectionHashFind(pTransportMgr,eTransport,hLocalAddress, NULL, &pAlias, phConn);

    if(*phConn != NULL)
    {
        return RV_OK;
    }
    else
    {
        return RV_ERROR_NOT_FOUND;
    }
}
/************************************************************************************
 * SipTransportConnectionConstruct
 * ----------------------------------------------------------------------------------
 * General: Construct a new connection.
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          bForceCreation      - indicates whether to look for such a connection in
 *                                the hash or to force the creation of a new connection.
 *          eConnType           - client or server
 *          pDestAddress        - The destination address
 *          hOwner              - The connection owner.
 *          pfnStateChangedEvHandler - function pointer to notify of connection state.
 *          pfnConnStatusEvHandle - function pointer to notify of connection status.
 *          pbNewConnCreated    - Indicates whether the connection that was returned is
 *                                a new connection that was just created.
 * Output:  hConnection         - the newly created connection handle
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionConstruct(
               IN  RvSipTransportMgrHandle                hTransportMgr,
               IN  RvBool                                 bForceCreation,
               IN  RvSipTransportConnectionType           eConnType,
               IN  RvSipTransport                         eTransport,
               IN  RvSipTransportLocalAddrHandle          hLocalAddr,
               IN  SipTransportAddress                    *pDestAddress,
               IN  RvSipTransportConnectionOwnerHandle    hOwner,
               IN  RvSipTransportConnectionStateChangedEv pfnStateChangedEvHandler,
               IN  RvSipTransportConnectionStatusEv       pfnConnStatusEvHandle,
               OUT RvBool                                 *pbNewConnCreated,
               OUT RvSipTransportConnectionHandle         *hConnection)

{
    TransportMgr      *pTransportMgr;
    RvStatus         rv;

    pTransportMgr = (TransportMgr*)hTransportMgr;

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "SipTransportConnectionConstruct - constructing new connection for owner 0x%p", hOwner));

    rv = TransportConnectionConstruct(pTransportMgr,bForceCreation,
                                      eConnType,eTransport,
                                      hLocalAddr,pDestAddress,
                                      hOwner,pfnStateChangedEvHandler,
                                      pfnConnStatusEvHandle,
                                      pbNewConnCreated,hConnection);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "SipTransportConnectionConstruct - Failed (rv=%d)",rv));
        return rv;
    }
    return RV_OK;
}

/************************************************************************************
 * SipTransportConnectionIsUsable
 * ----------------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to a connection to check
 *          hTransport - the transport to check
 *          hLocalAddr - the local address to check
 *          pDestAddress    - The destination address to check
 *          bCheckStateOnly - RV_TRUE - check only the connection state.
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportConnectionIsUsable(
               IN  RvSipTransportConnectionHandle         hConn,
               IN  RvSipTransport                         eTransport,
               IN  RvSipTransportLocalAddrHandle            hLocalAddr,
               IN  SipTransportAddress                      *pDestAddress,
               IN  RvBool                                bCheckStateOnly)
{
    RvStatus rv;
    TransportConnection *pConn = (TransportConnection*)hConn;
    if(pConn == NULL)
    {
        return RV_FALSE;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_FALSE;
    }

    if(bCheckStateOnly == RV_FALSE)
    {
        if(pConn->hLocalAddr != hLocalAddr)
        {
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - local addresses are different", pConn));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_FALSE;

        }

        if(pConn->eTransport != eTransport)
        {
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - transport is different", pConn));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_FALSE;

        }

        if(TransportMgrSipAddressIsEqual(&pConn->destAddress,pDestAddress) != RV_TRUE)
        {
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - remote address is different", pConn));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_FALSE;
        }
    }
    if((pConn->eState != RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED &&
       pConn->eState != RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED &&
       pConn->eState != RVSIP_TRANSPORT_CONN_STATE_CONNECTING &&
       pConn->eState != RVSIP_TRANSPORT_CONN_STATE_READY) ||
       pConn->bCloseEventInQueue != RV_FALSE)
    {
        RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - invalid state %s / bCloseEventInQueue = %d",
                    pConn,
                    SipCommonConvertEnumToStrConnectionState(pConn->eState),
                    pConn->bCloseEventInQueue));

        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_FALSE;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(pConn->eTransport == RVSIP_TRANSPORT_TLS)
    {
        switch (pConn->eTlsState)
        {
        case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED:
        case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY:
        case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED:
        case RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED:
            break;
        case RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED:
            if  (   (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED  ||
                    pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CONNECTING      ||
                    pConn->eState == RVSIP_TRANSPORT_CONN_STATE_READY            )   &&
                    (pConn->bCloseEventInQueue == RV_FALSE                       )   )
            {
                break;
            }
        default:
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                   "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - invalid TLS state %s",
                    pConn, TransportConnectionGetTlsStateName(pConn->eTlsState)));

            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_FALSE;

        } /*switch (pConn->eTlsState)*/
    }
#endif /*#ifdef  (RV_TLS_TYPE != RV_TLS_NONE)*/
    if (RV_TRUE == RvFdIsClosedByPeer(&pConn->ccSelectFd))
    {
        RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "SipTransportConnectionIsUsable - Connection 0x%p: not reusable - remote peer has closed connection", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_FALSE;
    }
    RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
           "SipTransportConnectionIsUsable - Connection 0x%p: reusable", pConn));
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_TRUE;
}


/************************************************************************************
 * SipTransportConnectionSend
 * ----------------------------------------------------------------------------------
 * General: Send Message on a tcp connection after copying it to a local buffer
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          hConnection         -
 *          hPool               - handle to the message rpool
 *          element             - rpool element that is going to be sent
 *          hLocalAddress       - The local address to send the message from.
 *          bFreeMsg            - The connection is responsible for deleteing the message page
 *          msgType             - The message type - will be notified in the conncetion callback
 *          hOwner                - The owner of the conncetion.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSend(
                    IN RvSipTransportMgrHandle             hTransportHandle,
                    IN RvSipTransportConnectionHandle      hConnHandle,
                    IN HRPOOL                              hPool,
                    IN HPAGE                               element,
                    IN RvBool                              bFreeMsg,
                    IN SipTransportMsgType                 msgType,
                    IN RvSipTransportConnectionOwnerHandle hOwner
#if (RV_NET_TYPE & RV_NET_SCTP)
                    ,
                    IN RvSipTransportSctpMsgSendingParams  *pSctpParams
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
                                        )
{
    TransportMgr          *pTransportMgr = (TransportMgr*)hTransportHandle;
    TransportConnection   *pConn         = (TransportConnection*)hConnHandle;
    RvStatus              rv             = RV_OK;
    RvInt32               size           = 0;
    RLIST_ITEM_HANDLE     hListItem      = NULL;
    SipTransportSendInfo* newMsg         = NULL;

    if(pTransportMgr == NULL || pConn == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "SipTransportConnectionSend - Connection 0x%p: About to send %s msg page 0x%p",
               pConn, SipCommonConvertEnumToStrTransportType(pConn->eTransport), element));

    if (pConn->hMsgToSendList == NULL)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConnectionSend - Connection 0x%p: no message to send list", pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_UNKNOWN;
    }
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    rv = RLIST_InsertTail(pConn->pTransportMgr->hMsgToSendPoolList,pConn->hMsgToSendList ,&hListItem);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConnectionSend - Connection 0x%p: Couldn't allocate outgoing msg to list.",pConn));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_UNKNOWN;
    }
    /* open the connection */
    newMsg            = (SipTransportSendInfo*)hListItem;
    newMsg->msgToSend = element;
    newMsg->msgType   = msgType;
    newMsg->bFreeMsg  = bFreeMsg;
    newMsg->hOwner    = hOwner;
    size              = (RvUint32)RPOOL_GetSize(hPool,element);
    newMsg->msgSize   = size;
    newMsg->currPos   = 0;
#if (RV_NET_TYPE & RV_NET_SCTP)
    newMsg->sctpData  = *pSctpParams;
    if (newMsg->sctpData.sctpMsgSendingFlags == UNDEFINED)
    {
        newMsg->sctpData.sctpMsgSendingFlags = pConn->sctpData.sctpParams.sctpMsgSendingFlags;
    }
    if (newMsg->sctpData.sctpStream == UNDEFINED)
    {
        newMsg->sctpData.sctpStream = pConn->sctpData.sctpParams.sctpStream;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    newMsg->bWasNotifiedMsgNotSent = RV_FALSE;
    newMsg->bDontRemove = RV_FALSE;

    if ((RvUint32)size > pConn->pTransportMgr->maxBufSize)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "SipTransportConnectionSend - pConn 0x%p: The transport's buffer is shorter than the message to send (%d<%d)",pConn,pConn->pTransportMgr->maxBufSize,size));
        TransportConnectionUnLockAPI(pConn);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    /*handle the open and connect*/
    if((pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CONNECTING) ||
        (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_CLOSING ) )
    {
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_OK;
    }
    /* open the TCP/SCTP session if not opened yet */
    if(pConn->eState == RVSIP_TRANSPORT_CONN_STATE_READY)
    {
        rv = TransportConnectionOpen(pConn,
                RV_TRUE /*bConnect=RV_TRUE -> connect immediatelly*/,
                0 /*clientLocalPort=0 -> the port will be choosen by OS*/);
        if (rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportConnectionSend - Connection 0x%p: Can't open TCP/SCTP connection",pConn))
            /* close the faulty connection */
            TransportConnectionClose(pConn,RV_TRUE/*bCloseTlsAsTcp*/);
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            /* In order to reach to MSG_SEND_FAILURE state if RV_DNS_ENHANCED_FEATURES_SUPPORT
               is defined, the return value should be RV_ERROR_NETWORK_PROBLEM
               (see function TransactionTransportSendMsg()) */
            return RV_ERROR_NETWORK_PROBLEM;
        }

        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "SipTransportConnectionSend - Connection 0x%p - Waiting for connection establishment to send data (transport=%s).",
                   pConn,SipCommonConvertEnumToStrTransportType(pConn->eTransport)));
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else if ((pConn->eTransport == RVSIP_TRANSPORT_TLS && pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY) ||
             (pConn->eTransport == RVSIP_TRANSPORT_TLS && pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED)       ||
             (pConn->eTransport == RVSIP_TRANSPORT_TLS && pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED))
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "SipTransportConnectionSend - Connection 0x%p: waiting for handshake completion (TLS state=%s, Tcp State=%s)",
                   pConn,TransportConnectionGetTlsStateName(pConn->eTlsState),
                   SipCommonConvertEnumToStrConnectionState(pConn->eState)));
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    else
    {
        switch(pConn->eTransport)
        {
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
                pConn->eTlsEvents = RV_TLS_WRITE_EV| RV_TLS_READ_EV;
                TransportTlsCallOnThread(pConn, RV_TRUE);
            break;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            pConn->ccEvent = RvSelectRead| RvSelectWrite;
            /* Never register to CLOSE event, since Common Core is not able
               to deduce it from READ event, when runs above LK SCTP Stack */
            TransportConnectionRegisterSelectEvents(pConn, RV_FALSE/*bAddClose*/,
                RV_FALSE/*bFirstCall*/);
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        case RVSIP_TRANSPORT_TCP:
            pConn->ccEvent = RvSelectRead| RvSelectWrite;
            TransportConnectionRegisterSelectEvents(pConn, RV_TRUE/*bAddClose*/,
                RV_FALSE/*bFirstCall*/);
            break;
        default:
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportConnectionSend - Connection 0x%p: 2 unsupported type %d",
                pConn, pConn->eTransport));
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return RV_ERROR_UNKNOWN;
        }
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_OK;
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}

/***************************************************************************
* SipTransportTcpEnabled
* ------------------------------------------------------------------------
* General: return status of tcp enabled or not
* Return Value: RvBool
* ------------------------------------------------------------------------
* Arguments:
* Output:     hTransport - handle to the transport
*
***************************************************************************/
RvBool RVCALLCONV SipTransportTcpEnabled(IN  RvSipTransportMgrHandle    hTransport)
{
    TransportMgr        *pTransportMgr;
    pTransportMgr = (TransportMgr*)hTransport;

    return pTransportMgr->tcpEnabled;
}

/***************************************************************************
* SipTransportGetMaxDnsElements
* ------------------------------------------------------------------------
* General: return status of tcp enabled or not
* Return Value: RvUint32 - maximum number of DNS list entries
* ------------------------------------------------------------------------
* Arguments:
* Input:     hTransport - handle to the transport
*
***************************************************************************/
RvUint32 RVCALLCONV SipTransportGetMaxDnsElements(
                                    IN  RvSipTransportMgrHandle    hTransport)
{
    return TransportDNSGetMaxElements((TransportMgr*)hTransport);
}

/***************************************************************************
 * SipTransportDNSListConstruct
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListConstruct function allocates and fills
 *          the TransportDNSList structure and returns handler to the
 *            TransportDNSList and appropriate error code or RV_OK.
 *            Receives as input memory pool, page and list pool where lists
 *            element and the TransportDNSList object itself will be allocated.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMemPool    - Handle of the memory pool.
 *            hMemPage    - Handle of the memory page
 *            hListsPool    - Handle of the list pool
 *            maxElementsInSingleDnsList - maximum num. of elements
 *                        in single list
 * Output:     phDnsList    - Pointer to a DNS list object handler.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListConstruct(IN  HRPOOL                           hMemPool,
                                                  IN  HPAGE                            hMemPage,
                                                  IN  RvUint32                        maxElementsInSingleDnsList,
                                                  OUT RvSipTransportDNSListHandle    *phDnsList)
{
    return TransportDNSListConstruct(hMemPool, hMemPage, maxElementsInSingleDnsList,
                                    RV_FALSE, (TransportDNSList**)phDnsList);
}

/***************************************************************************
 * SipTransportDNSListDestruct
 * ------------------------------------------------------------------------
 * General: The RvSipTransportDNSListDestruct function frees all memory allocated
 *          by the TransportDNSList object, including the TransportDNSList itself.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phDnsList    - Pointer to a DNS list object handler.
 * Output:     N/A
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListDestruct(IN RvSipTransportDNSListHandle    pDnsList)
{
    return TransportDNSListDestruct((TransportDNSList*)pDnsList);
}


/***************************************************************************
 * SipTransportDNSListClone
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListClone function copies entire original
 *          TransportDNSList object to it's clone object.
 *          Note that clone object should be constructed before by call to the
 *          SipTransportDNSListConstruct function.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hMgr               - Transport manager  
 *             hDnsListOriginal   - Original DNS list object handler.
 * Output:     hDnsListClone      - Clone DNS list object handler
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListClone(IN RvSipTransportMgrHandle        hMgr, 
                                             IN  RvSipTransportDNSListHandle   hDnsListOriginal,
                                              OUT RvSipTransportDNSListHandle  hDnsListClone)
{
    return TransportDNSListClone((TransportMgr*)hMgr,
                                 (TransportDNSList*)hDnsListOriginal,
                                 (TransportDNSList*)hDnsListClone);
}



/************************************************************************************
 * SipTransportISIpV4
 * ----------------------------------------------------------------------------------
 * General: check if the string is an ip4 address
 *
 * Return Value: RV_TRUE - ip4 address, RV_FALSE - not ip4 address
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 * Output:  (-)
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportISIpV4(RvChar* StringIp)
{
    RvUint32 ip1;
    RvUint32 ip2;
    RvUint32 ip3;
    RvUint32 ip4;

    if (sscanf(StringIp, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4)
    {
        return RV_FALSE;
    }

    return RV_TRUE;
}

/************************************************************************************
 * SipTransportISIpV6
 * ----------------------------------------------------------------------------------
 * General: check if the string is an ip6 address
 *
 * Return Value: RV_TRUE - ip4 address, RV_FALSE - not ip4 address
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 * Output:  (-)
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportISIpV6(RvChar* StringIp)
{
    SipTransportAddress addr;
    RvBool              bIp = RV_FALSE;
        
    RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&addr.addr);
    if (RV_OK == TransportMgrConvertString2Address(StringIp,&addr, RV_FALSE,RV_FALSE))
    {
        /* this is an ipv6 address */
        bIp = RV_TRUE;
    }
    RvAddressDestruct(&addr.addr);

    return bIp;
}

/***************************************************************************
 * SipTransportGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: returns the status of the message module's resources.
 *          (this is a debug function!)
 * Return Value: RV_OK, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 * Output:     pResources - struct that contain the resources.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetResourcesStatus (
                                      IN  RvSipTransportMgrHandle    hTransport,
                                      OUT RvSipTransportResources  *pResources)
{

    TransportMgr        *pTransportMgr;
    RV_Resource   pQueueElements;
    RV_Resource   pQueueEvents;
    RV_Resource   readBuffers;
    RV_HASH_Resource connHash;
    RV_HASH_Resource ownersHash;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RV_Resource   tlsEngines;
    RV_Resource   tlsSessions;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

    RvUint32 allocNumOfLists=0;
    RvUint32 allocMaxNumOfUserElement=0;
    RvUint32 currNumOfUsedLists=0;
    RvUint32 currNumOfUsedUserElement=0;
    RvUint32 maxUsageInLists=0;
    RvUint32 maxUsageInUserElement=0;
    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pTransportMgr = (TransportMgr*)hTransport;

    memset(pResources,0,sizeof(RvSipTransportResources));

    /* connections list*/
    if (pTransportMgr->tcpEnabled==RV_TRUE)
    {
        RLIST_GetResourcesStatus(pTransportMgr->hConnPool,
                                &allocNumOfLists,
                                &allocMaxNumOfUserElement,
                                &currNumOfUsedLists,
                                &currNumOfUsedUserElement,
                                &maxUsageInLists,
                                &maxUsageInUserElement);

        /* elements resources */
        pResources->connections.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->connections.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->connections.maxUsageOfElements     = maxUsageInUserElement;


        HASH_GetResourcesStatus(pTransportMgr->hConnHash,&connHash);
        pResources->connHash.currNumOfUsedElements = connHash.elements.numOfUsed;
        pResources->connHash.numOfAllocatedElements = connHash.elements.maxNumOfElements;
        pResources->connHash.maxUsageOfElements = connHash.elements.maxUsage;

        HASH_GetResourcesStatus(pTransportMgr->hConnOwnersHash,&ownersHash);
        pResources->ownersHash.currNumOfUsedElements = ownersHash.elements.numOfUsed;
        pResources->ownersHash.numOfAllocatedElements = ownersHash.elements.maxNumOfElements;
        pResources->ownersHash.maxUsageOfElements = ownersHash.elements.maxUsage;
    }

    /* connections-no-owner list*/
    if (pTransportMgr->tcpEnabled==RV_TRUE)
    {
        RLIST_GetResourcesStatus(pTransportMgr->hConnectedNoOwnerConnPool,
                                &allocNumOfLists,
                                &allocMaxNumOfUserElement,
                                &currNumOfUsedLists,
                                &currNumOfUsedUserElement,
                                &maxUsageInLists,
                                &maxUsageInUserElement);
        
        /* elements resources */
        pResources->connectionsNoOwners.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->connectionsNoOwners.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->connectionsNoOwners.maxUsageOfElements     = maxUsageInUserElement;
    }

    /* oor resources */
    RLIST_GetResourcesStatus(pTransportMgr->oorListPool,
                            &allocNumOfLists,
                            &allocMaxNumOfUserElement,
                            &currNumOfUsedLists,
                            &currNumOfUsedUserElement,
                            &maxUsageInLists,
                            &maxUsageInUserElement);

    pResources->oorEvents.currNumOfUsedElements  = currNumOfUsedUserElement;
    pResources->oorEvents.numOfAllocatedElements = allocMaxNumOfUserElement;
    pResources->oorEvents.maxUsageOfElements     = maxUsageInUserElement;


    PQUEUE_GetResourcesStatus(pTransportMgr->hProcessingQueue,
                              &pQueueElements,
                              &pQueueEvents);

    pResources->pQueueElements.currNumOfUsedElements  = pQueueElements.numOfUsed;
    pResources->pQueueElements.numOfAllocatedElements = pQueueElements.maxNumOfElements;
    pResources->pQueueElements.maxUsageOfElements     = pQueueElements.maxUsage;
    pResources->pQueueEvents.currNumOfUsedElements    = pQueueEvents.numOfUsed;
    pResources->pQueueEvents.numOfAllocatedElements   = pQueueEvents.maxNumOfElements;
    pResources->pQueueEvents.maxUsageOfElements       = pQueueEvents.maxUsage;

    RA_GetResourcesStatus(pTransportMgr->hMessageMemoryBuffPool,
                         &readBuffers);

    pResources->readBuffers.currNumOfUsedElements  = readBuffers.numOfUsed;
    pResources->readBuffers.numOfAllocatedElements = readBuffers.maxNumOfElements;
    pResources->readBuffers.maxUsageOfElements     = readBuffers.maxUsage;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (NULL != pTransportMgr->tlsMgr.hraSessions)
    {
        RA_GetResourcesStatus(pTransportMgr->tlsMgr.hraSessions,
                             &tlsSessions);

        pResources->tlsSessions.currNumOfUsedElements  = tlsSessions.numOfUsed;
        pResources->tlsSessions.numOfAllocatedElements = tlsSessions.maxNumOfElements;
        pResources->tlsSessions.maxUsageOfElements     = tlsSessions.maxUsage;
    }
    else
    {
        pResources->tlsSessions.currNumOfUsedElements  = 0;
        pResources->tlsSessions.numOfAllocatedElements = 0;
        pResources->tlsSessions.maxUsageOfElements     = 0;
    }

    if (NULL != pTransportMgr->tlsMgr.hraEngines)
    {
        RA_GetResourcesStatus(pTransportMgr->tlsMgr.hraEngines,
                             &tlsEngines);

        pResources->tlsEngines.currNumOfUsedElements  = tlsEngines.numOfUsed;
        pResources->tlsEngines.numOfAllocatedElements = tlsEngines.maxNumOfElements;
        pResources->tlsEngines.maxUsageOfElements     = tlsEngines.maxUsage;
    }
    else
    {
        pResources->tlsEngines.currNumOfUsedElements  = 0;
        pResources->tlsEngines.numOfAllocatedElements = 0;
        pResources->tlsEngines.maxUsageOfElements     = 0;
    }
#else /*  (RV_TLS_TYPE != RV_TLS_NONE) */
        pResources->tlsSessions.currNumOfUsedElements  = 0;
        pResources->tlsSessions.numOfAllocatedElements = 0;
        pResources->tlsSessions.maxUsageOfElements     = 0;
        pResources->tlsEngines.currNumOfUsedElements   = 0;
        pResources->tlsEngines.numOfAllocatedElements  = 0;
        pResources->tlsEngines.maxUsageOfElements      = 0;
#endif/*  (RV_TLS_TYPE != RV_TLS_NONE) */
    return RV_OK;
}

/***************************************************************************
 * SipTransportMgrIsMultiThread
 * ------------------------------------------------------------------------
 * General: Return true if SIP stack is configured to be multi-thread.
 * Return Value: RV_TRUE  - multi thread stack.
 *               RV_FALSE - single thread stack
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the transport manager.
 ***************************************************************************/
RvBool RVCALLCONV SipTransportMgrIsMultiThread(IN  RvSipTransportMgrHandle    hTransportMgr)
{
    TransportMgr *transportInfo;

    transportInfo = (TransportMgr*)hTransportMgr;
    if ((transportInfo != NULL) && (transportInfo->numberOfProcessingThreads > 0))
        return RV_TRUE;

    return RV_FALSE;
}

/***************************************************************************
 * SipTransportMgrIsSupportServerConnReuse
 * ------------------------------------------------------------------------
 * General: Return true if application is registered on pfnEvConnServerReuse.
 * Return Value: RV_TRUE  - supports connection reuse.
 *               RV_FALSE - does not support connection reuse.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the transport manager.
 ***************************************************************************/
RvBool RVCALLCONV SipTransportMgrIsSupportServerConnReuse(IN  RvSipTransportMgrHandle    hTransportMgr)
{
    TransportMgr *transportInfo;

    transportInfo = (TransportMgr*)hTransportMgr;

    if(transportInfo->appEvHandlers.pfnEvConnServerReuse != NULL)
    {
        return RV_TRUE;
    }

    return RV_FALSE;
}

/************************************************************************************
 * SipTransportConvertCoreAddrTypeToSipAddrType
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
RvSipTransportAddressType SipTransportConvertCoreAddrTypeToSipAddrType(
                        IN   RvInt      eCoreAddressType)
{
    return TransportMgrConvertCoreAddrTypeToSipAddrType(eCoreAddressType);
}

/***************************************************************************
 * SipTransportGetNumOfIPv4LocalAddresses
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
RvStatus RVCALLCONV SipTransportGetNumOfIPv4LocalAddresses(
                                     IN   TransportMgr* pTransportMgr,
                                     OUT  RvUint32*    pNumberOfAddresses)
{
    RvStatus      crv            = RV_OK;
    RvUint32      numofAddresses = RV_HOST_MAX_ADDRS;
    RvAddress     addresses[RV_HOST_MAX_ADDRS];
    RvUint32      i              = 0;
    crv = RvHostLocalGetAddress(pTransportMgr->pLogMgr,&numofAddresses,(RvAddress*)addresses);
    if (RV_OK != crv)
    {
        *pNumberOfAddresses = 0;
        return CRV2RV(crv);
    }

    while (i < numofAddresses && RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(&addresses[i]))
        i++;

    *pNumberOfAddresses = i;
    return RV_OK;
}

/***************************************************************************
 * SipTransportGetIPv4LocalAddressByIndex
 * ------------------------------------------------------------------------
 * General: Retrieves the 'index'th local address. Used when the stack was
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
RvStatus RVCALLCONV SipTransportGetIPv4LocalAddressByIndex(
                                     IN   TransportMgr *pTransportMgr,
                                     IN   RvUint                   index,
                                     OUT  RvUint8                  *pLocalAddr)
{
    RvUint        i;
    RvStatus      crv            = RV_OK;
    RvUint32      numofAddresses = RV_HOST_MAX_ADDRS;
    RvAddress     addresses[RV_HOST_MAX_ADDRS];
    RvUint        ipAddr         = 0;

    crv = RvHostLocalGetAddress(pTransportMgr->pLogMgr,&numofAddresses,(RvAddress*)addresses);
    if (RV_OK != crv)
    {
        return CRV2RV(crv);
    }
    if (index >= numofAddresses)
    {
        return RV_ERROR_BADPARAM;
    }
    if (RV_ADDRESS_TYPE_IPV4 != RvAddressGetType(&addresses[index]))
    {
        return RV_ERROR_BADPARAM;
    }

#define IPV4_NUMOF_BYTES 4
    ipAddr = RvAddressIpv4GetIp(RvAddressGetIpv4(&addresses[index]));
    for (i=0; i<IPV4_NUMOF_BYTES; i++)
    {
        pLocalAddr[i] = ((RvUint8 *)(&ipAddr))[i];
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransportGetLocalAddressByType
 * ------------------------------------------------------------------------
 * General: Get the local address from a local address struct
 *        . This is the address the user set using the
 *          RvSipXXXXSetLocalAddress function. If no adress was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to local address struct
 *          eTransportType - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type (ip4 or ip6).
 *          hTransportMgr       - transport Mgr
 * Output:    localAddress - The local address this call-leg is using(must be longer than 48).
 *          localPort - The local port this call-leg is using.
 * if_name - the local interface name.
 * vdevblock  - the local hardware port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetLocalAddressByType(
                            IN  RvSipTransportMgrHandle   hTransportMgr,
                            IN  SipTransportObjLocalAddresses     *pLocalAddr,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *strLocalAddress,
                            OUT  RvUint16                *pLocalPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    )
{
    RvStatus                   rv         = RV_OK;
    RvSipTransportLocalAddrHandle hLocalAddr = NULL;
    TransportMgr                *pTransportInfo;

    pTransportInfo = (TransportMgr*)hTransportMgr;

    if (NULL == pLocalAddr || strLocalAddress==NULL || pLocalPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    strLocalAddress[0] = '\0'; /*reset the local address*/
    *pLocalPort = 0;

    if (RVSIP_TRANSPORT_UNDEFINED == eTransportType ||
        RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == eAddressType)
    {
        return RV_ERROR_BADPARAM;
    }


    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            hLocalAddr = pLocalAddr->hLocalUdpIpv4;
        }
        else
        {
            hLocalAddr = pLocalAddr->hLocalUdpIpv6;
        }
        break;
    case RVSIP_TRANSPORT_TCP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            hLocalAddr = pLocalAddr->hLocalTcpIpv4;
        }
        else
        {
            hLocalAddr = pLocalAddr->hLocalTcpIpv6;
        }
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            hLocalAddr = pLocalAddr->hLocalTlsIpv4;
        }
        else
        {
            hLocalAddr = pLocalAddr->hLocalTlsIpv6;
        }
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
        {
            hLocalAddr = pLocalAddr->hLocalSctpIpv4;
        }
        else
        {
            hLocalAddr = pLocalAddr->hLocalSctpIpv6;
        }
        break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        break;
    } /*switch (eTransportType)*/

    /*choose the correct local address handle according to transport and address types*/
    if(hLocalAddr == NULL)
    {
       RvLogDebug(pTransportInfo->pLogSrc ,(pTransportInfo->pLogSrc ,
                  "SipTransportGetLocalAddressByType - Local %s %s address not found",
                   SipCommonConvertEnumToStrTransportType(eTransportType),
                   SipCommonConvertEnumToStrTransportAddressType(eAddressType)));
       return RV_ERROR_NOT_FOUND;

    }

    rv = SipTransportLocalAddressGetAddressByHandle(hTransportMgr,
                                                    hLocalAddr,RV_FALSE,RV_TRUE,
                                                    strLocalAddress,pLocalPort
#if defined(UPDATED_BY_SPIRENT)
						    ,if_name
						    ,vdevblock
#endif
						    );
    if(rv != RV_OK)
    {
        RvLogError(pTransportInfo->pLogSrc,(pTransportInfo->pLogSrc,
                  "SipTransportGetLocalAddressByType - Failed to get info for local %s %s address 0x%p (rv=%d:%s)",
                   SipCommonConvertEnumToStrTransportType(eTransportType),
                   SipCommonConvertEnumToStrTransportAddressType(eAddressType),
                   hLocalAddr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/************************************************************************************
 * SipTransportMgrAllocateRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Allocates receive buffer. This function gives other layers, the option
 *          to use a consecutive buffer.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 * Output:  buff            - allocated buffer
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportMgrAllocateRcvBuffer(
                               IN  RvSipTransportMgrHandle   hTransportMgr,
                               OUT RvChar                     **buff,
                               OUT RvUint32                 *bufLen)
{
    RvStatus rv;
    TransportMgr                *pMgr = (TransportMgr*)hTransportMgr;

    rv = TransportMgrAllocateRcvBuffer(pMgr, RV_FALSE, buff);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipTransportMgrAllocateRcvBuffer - failed to allocate Rcv buffer. (rv=%d)",rv));
        return rv;
    }
    *bufLen = pMgr->maxBufSize;
    return RV_OK;
}

/************************************************************************************
 * SipTransportMgrFreeRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Frees allocated receive buffer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 *            buff            - allocated buffer
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportMgrFreeRcvBuffer(
                              IN  RvSipTransportMgrHandle   hTransportMgr,
                              IN  RvChar                    *buff)
{
    return TransportMgrFreeRcvBuffer((TransportMgr*)hTransportMgr, buff);
}

/************************************************************************************
 * SipTransportConvertApiTransportAddrToCoreAddress
 * ----------------------------------------------------------------------------------
 * General: converts API address (RvSipTransportAddr) to Transport
 *          address (SipTransportAddress).
 *          the Transport parameter is ignored
 *          *** it is the responsibility of the caller to destruct the core
 *              address inside the transport address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to the Transport manager
 *          pAPIAddr        - source address
 * Output:  pCoreAddr       - target address
*****************************************************************************/
RvStatus RVCALLCONV SipTransportConvertApiTransportAddrToCoreAddress(
                            IN  RvSipTransportMgrHandle hTransportMgr,
                            IN  RvSipTransportAddr*     pAPIAddr,
                            OUT RvAddress*              pCoreAddr)
{
    return TransportMgrConvertTransportAddr2RvAddress(
                (TransportMgr*)hTransportMgr, pAPIAddr, pCoreAddr);
}

/************************************************************************************
 * SipTransportConvertCoreAddressToApiTransportAddr
 * ----------------------------------------------------------------------------------
 * General: converts Transport address (SipTransportAddress) to API address (RvSipTransportAddr) .
 *          the Transport parameter is ignored
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pCoreAddr      - source address
 *          pAPIAddr       - target address
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportConvertCoreAddressToApiTransportAddr(
                              IN  RvAddress*                pCoreAddr,
                              OUT RvSipTransportAddr*       pAPIAddr)
{
    pAPIAddr->eAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(pCoreAddr));
    pAPIAddr->port           = RvAddressGetIpPort(pCoreAddr);
#if(RV_NET_TYPE & RV_NET_IPV6)
    pAPIAddr->Ipv6Scope      = RvAddressGetIpv6Scope(pCoreAddr);
#else
    pAPIAddr->Ipv6Scope      = UNDEFINED;
#endif /*RV_NET_IPV6*/
    pAPIAddr->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    /* For IPv6 addresses add brackets */
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAPIAddr->eAddrType)
    {
        RvSize_t lenOfAddr;
        pAPIAddr->strIP[0] = '[';
        RvAddressGetString(pCoreAddr,RVSIP_TRANSPORT_LEN_STRING_IP,(RvChar*)pAPIAddr->strIP+1);
        lenOfAddr = strlen(pAPIAddr->strIP);
        pAPIAddr->strIP[lenOfAddr] = ']';
        pAPIAddr->strIP[lenOfAddr+1] = '\0';
    }
    else
    {
        RvAddressGetString(pCoreAddr,RVSIP_TRANSPORT_LEN_STRING_IP,(RvChar*)pAPIAddr->strIP);
    }
}

/************************************************************************************
 * SipTransportConvertSipTransportAddressToApiTransportAddr
 * ----------------------------------------------------------------------------------
 * General: converts Transport address (SipTransportAddress) to API address (RvSipTransportAddr) .
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pCoreAddr      - source address
 *          pAPIAddr       - target address
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportConvertSipTransportAddressToApiTransportAddr(
                              IN  SipTransportAddress*      pInternalAddr,
                              OUT RvSipTransportAddr*       pAPIAddr)
{
    SipTransportConvertCoreAddressToApiTransportAddr(&pInternalAddr->addr,pAPIAddr);
    pAPIAddr->eTransportType = pInternalAddr->eTransport;
}

/******************************************************************************
 * SipTransportConvertStrIp2TransportAddress
 * ----------------------------------------------------------------------------
 * General: parse string, representing IP address,
 *          and fills the RvSipTransportAddr structure with the IP details
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr  - Handle to the Transport Manager object.
 *          strIP          - String, conatining IP.
 * Output:  pTransportAddr - Structure, to be filled with the IP details.
 *****************************************************************************/
RvStatus SipTransportConvertStrIp2TransportAddress(
                                   IN  RvSipTransportMgrHandle hTransportMgr,
                                   IN  RvChar*                 strIP,
                                   OUT RvSipTransportAddr*     pTransportAddr)
{
    TransportMgr* pTransportMgr = (TransportMgr*)hTransportMgr;

    /*To prevent compilation warning on Nucleus, while compiling without logs*/
    RV_UNUSED_ARG(pTransportMgr);

    memset(pTransportAddr, 0, sizeof(RvSipTransportAddr));
    
    if (NULL == strstr(strIP,":"))
    {
        pTransportAddr->eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    }
    else
    {
        pTransportAddr->eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }
    
    pTransportAddr->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    pTransportAddr->port = 0;
    
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP == pTransportAddr->eAddrType)
    {
        strcpy(pTransportAddr->strIP, strIP);
    }
    else
    {
#if !(RV_NET_TYPE & RV_NET_IPV6)
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConvertStrIp2TransportAddress - IPv6 is not supported"));
        return RV_ERROR_BADPARAM;
#else
        RvChar*  strScopeId;
        RvChar*  strBracket;
        RvChar   strTempBuff[RVSIP_TRANSPORT_LEN_STRING_IP];
        
        strBracket = strchr(strIP,']');
        strScopeId = strchr(strIP,'%');
        
        /* Extract IP itself - without brackets and scope Id */
        if (NULL != strBracket)
        {
            memcpy(strTempBuff, &strIP[1], (strBracket-&strIP[1]));
            strTempBuff[strBracket-&strIP[1]] = '\0';
        }
        else if (NULL != strScopeId)
        {
            memcpy(strTempBuff, strIP, (strScopeId-strIP));
            strTempBuff[strScopeId-strIP] = '\0';
        }
        else
        {
            strcpy(pTransportAddr->strIP, strIP);
            return RV_OK;
        }
        strcpy(pTransportAddr->strIP, strTempBuff);
        
        if (NULL != strScopeId)
        {
            pTransportAddr->Ipv6Scope = atoi(strScopeId+1);
        }
        else
        {
            pTransportAddr->Ipv6Scope = UNDEFINED;
        }
        
        RV_UNUSED_ARG(pTransportMgr);
#endif /*#if !(RV_NET_TYPE & RV_NET_TYPE) #else*/
    }
    return RV_OK;
}

/************************************************************************************
 * SipTransportIPElementToSipTransportAddress
 * ----------------------------------------------------------------------------------
 * General: converts API address (RvSipTransportAddr) to Transport
 *          address (SipTransportAddress).
 *          the Transport parameter is ignored
 *          *** it is the responsibility of the caller to destruct the core
 *              address inside the transport address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pAPIAddr        - source address
 *          pCoreAddr       - target address
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportIPElementToSipTransportAddress(
                              INOUT SipTransportAddress*       pDestAddr,
                              IN RvSipTransportDNSIPElement*   pIPElement)
{
    pDestAddr->eTransport = pIPElement->protocol;
    if (pIPElement->bIsIpV6 == RV_FALSE)
    {
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&pDestAddr->addr);
        memcpy(&pDestAddr->addr.data.ipv4.ip,pIPElement->ip,4);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (pIPElement->bIsIpV6 == RV_TRUE)
    {
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&pDestAddr->addr);
        memcpy(pDestAddr->addr.data.ipv6.ip,pIPElement->ip,RVSIP_TRANSPORT_LEN_BYTES_IPV6);
    }
#endif
    RvAddressSetIpPort(&pDestAddr->addr,pIPElement->port);
}

 /***********************************************************************************************************************
 * SipTransportDnsSetEnumResult
 * purpose : sets the result on an ENUM NAPTR query
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 * output  : hDnsList       - dns list to set record to
 * return  : RvStatus       - RV_OK or error
 *************************************************************************************************************************/
RvStatus SipTransportDnsSetEnumResult(
    IN RvSipTransportMgrHandle          hTransportMgr,
    IN RvDnsData*                       pDnsData,
    INOUT RvSipTransportDNSListHandle   hDnsList)
{
    return TransportDnsSetEnumResult((TransportMgr*)hTransportMgr,pDnsData,(TransportDNSList*)hDnsList);
}

 /***********************************************************************************************************************
 * SipTransportDnsGetEnumResult
 * purpose : get the result on an ENUM NAPTR query
 * input   : hTransportMgr  - transport mgr
 *           hDnsList       - DNS list to set record to
 * output  : pEnumRes       - pointer to ENUM string
 * return  : RvStatus       - RV_OK or error
 *************************************************************************************************************************/
RvStatus SipTransportDnsGetEnumResult(
    IN  RvSipTransportMgrHandle       hTransportMgr,
    IN  RvSipTransportDNSListHandle   hDnsList,
    OUT RvChar**                      pEnumRes)
{
    return TransportDnsGetEnumResult((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,pEnumRes);
}

#if defined(RV_DNS_ENHANCED_FEATURES_SUPPORT)
 /***********************************************************************************************************************
 * SipTransportDnsAddNAPTRRecord
 * purpose : Adds NAPTR record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           bIsSecure      - Allow ONLY Secure resilts (i.e. TLS)
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus       - RV_OK or error
 *************************************************************************************************************************/
RvStatus SipTransportDnsAddNAPTRRecord(
    IN RvSipTransportMgrHandle          hTransportMgr,
    IN RvDnsData*                       pDnsData,
    IN RvBool                           bIsSecure,
    INOUT RvSipTransportDNSListHandle   hDnsList)
{
    return TransportDnsAddNAPTRRecord((TransportMgr*)hTransportMgr,pDnsData,bIsSecure,(TransportDNSList*)hDnsList);
}

/***********************************************************************************************************************
 * SipTransportDnsAddSRVRecord
 * purpose : Adds SRV record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           eTransport     - transportof the added record
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus       - RV_OK or error
 *************************************************************************************************************************/
RvStatus SipTransportDnsAddSRVRecord(
    IN RvSipTransportMgrHandle   hTransportMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    INOUT RvSipTransportDNSListHandle hDnsList)

{
    return TransportDnsAddSRVRecord((TransportMgr*)hTransportMgr,pDnsData,eTransport,(TransportDNSList*)hDnsList);
}

/***************************************************************************
 * SipTransportDNSListGetHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host element from the host elements list of the DNS
 * list object according to specified by input location.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - pointer of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative host name element for 'next' or 'previous'
 *                            locations
 * Output:  pHostElement    - found element
 *          pRelative       - new relative host name element for consequent
 *          call to the RvSipTransportDNSListGetHostElement function.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListGetHostElement (
    IN    RvSipTransportMgrHandle           hTransportMgr,
    IN    RvSipTransportDNSListHandle       hDnsList,
    IN    RvSipListLocation                 location,
    INOUT void**                            pRelative,
    OUT   RvSipTransportDNSHostNameElement* pHostElement)
{
    return TransportDNSListGetHostElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,location,pRelative,pHostElement);
}

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/***************************************************************************
 * SipTransportDNSListGetIPElement
 * ------------------------------------------------------------------------
 * General: retrieves IP address element from the DNS list objects IP
 * addresses list according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative IP element for get next/previous
 * Output:  pIPElement      - found element
 *          pRelative       - new relative IP element for consequent
 *          call to the RvSipTransportDNSListGetIPElement function.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListGetIPElement (
    IN    RvSipTransportMgrHandle        hTransportMgr,
    IN    RvSipTransportDNSListHandle    hDnsList,
    IN    RvSipListLocation              location,
    INOUT void**                         pRelative,
    OUT   RvSipTransportDNSIPElement*    pIPElement)
{
    return TransportDNSListGetIPElement((TransportMgr*)hTransportMgr,(TransportDNSList*)hDnsList,location,pRelative,pIPElement);
}

/***********************************************************************************************************************
 * SipTransportDnsAddIPRecord
 * purpose : Adds IP record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           eTransport     - transport of record
 *           port           - port of record
 *
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
RvStatus SipTransportDnsAddIPRecord(
    IN RvSipTransportMgrHandle   hTransportMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    IN RvUint16              port,
    INOUT RvSipTransportDNSListHandle hDnsList)
{
    return TransportDnsAddIPRecord((TransportMgr*)hTransportMgr,pDnsData,eTransport,port,(TransportDNSList*)hDnsList);
}

/******************************************************************************
 * SipTransportMgrCounterLocalAddrsGet
 * ----------------------------------------------------------------------------
 * General: Returns counter of the opened addresses of the required type.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - the Transport Manager.
 *            eTransportType- type of Transport Protocol.
 * Output:    pV4Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *            pV6Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *****************************************************************************/
void RVCALLCONV SipTransportMgrCounterLocalAddrsGet(
                                    IN  RvSipTransportMgrHandle   hTransportMgr,
                                    IN  RvSipTransport          eTransportType,
                                    OUT RvUint32*               pV4Counter,
                                    OUT RvUint32*               pV6Counter)
{
    TransportMgrCounterLocalAddrsGet((TransportMgr*)hTransportMgr,eTransportType,pV4Counter,pV6Counter);
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/*****************************************************************************
* SipTransportDnsListPrepare
* ---------------------------------------------------------------------------
* General: 1. Removes TLS if client does not support TLS
*          2. Removes non-TLS if transport supposed to be TSL (sips URI)
*          3. Does nothing if client supports TLS and uri was "sip"
*
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr        - pointer to transport manager
*          strHostName          - the host name we handle
*          bIsSecure            - was the URI of the message secure (eg. sips)
*          hSrvNamesList        - the list that holds the srv records
*****************************************************************************/
void RVCALLCONV SipTransportDnsListPrepare(
      IN  RvSipTransportMgrHandle   hTransportMgr,
      IN  RvChar*             strHostName,
      IN  RvSipTransportDNSListHandle hDnsList)
{
    TransportDNSList* pList = (TransportDNSList*)hDnsList;

    if (NULL == pList || NULL == pList->hSrvNamesList)
        return;

    TransportDnsListPrepare((TransportMgr*)hTransportMgr,strHostName,pList->hSrvNamesList);
}

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/******************************************************************************
 * SipTransportConnectionAttachOwner
 * ----------------------------------------------------------------------------
 * General: Attach a new owner to the supplied connection together with a set
 *          of callback functions that will be used to notify this owner about
 *          connection states and statuses. You can use this function only if
 *          the connection is connected or in the process to be connected.
 *          You cannot attach an owner to a connection that started its
 *          disconnection process.
 *          Note: The connection will not disconnect as long as it has owners
 *          attached to it.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to the connection.
 *          hOwner - The owner handle
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the event handler tructure.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionAttachOwner(
                            IN  RvSipTransportConnectionHandle        hConn,
                            IN  RvSipTransportConnectionOwnerHandle   hOwner,
                            IN  RvSipTransportConnectionEvHandlers    *pEvHandlers,
                            IN  RvInt32                               sizeofEvHandlers)
{
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    RvStatus              rv;
    RvSipTransportConnectionEvHandlers evHandlers;
    RvUint32              minStructSize;

    minStructSize = RvMin((RvUint32)sizeofEvHandlers,
                          sizeof(RvSipTransportConnectionEvHandlers));

    memset(&evHandlers, 0 ,sizeof(RvSipTransportConnectionEvHandlers));
    if(pEvHandlers != NULL)
    {
        memcpy(&evHandlers,pEvHandlers,minStructSize);
    }

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;
    switch (pConn->eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_IDLE:
    case RVSIP_TRANSPORT_CONN_STATE_READY:
    case RVSIP_TRANSPORT_CONN_STATE_CONNECTING:
    case RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED:
    case RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED:
        break;
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
    case RVSIP_TRANSPORT_CONN_STATE_CLOSED:
    case RVSIP_TRANSPORT_CONN_STATE_CLOSING:
        RvLogWarning(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
            "SipTransportConnectionAttachOwner - conn: 0x%p -  Failed, Cannot attach owner at state %s",pConn,
            SipCommonConvertEnumToStrConnectionState(pConn->eState)));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_ILLEGAL_ACTION;
    default:
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
            "SipTransportConnectionAttachOwner - conn: 0x%p -  Failed, Cannot attach owner at state %s",pConn,
            SipCommonConvertEnumToStrConnectionState(pConn->eState)));
        TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return RV_ERROR_UNKNOWN;
    }

    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "SipTransportConnectionAttachOwner - Attaching owner 0x%p to connection 0x%p",hOwner,pConn));

    rv = TransportConnectionAttachOwner(pConn,evHandlers.pfnConnStateChangedEvHandler,
        evHandlers.pfnConnStausEvHandler,hOwner);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConnectionAttachOwner - Failed Attach owner 0x%p to connection 0x%p",hOwner,pConn))
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
        return rv;
    }

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return RV_OK;
}

/******************************************************************************
 * SipTransportFixIpv6DestinationAddr
 * ----------------------------------------------------------------------------
 * General: Destination IPv6 address of local-link level should contain
 *          Scope ID, which tells to the Operation System, through which
 *          interface the SIP message for UDP, or SIN packet for TCP should be
 *          sent. If it the destination address has no Scope Id,
 *          this function sets it Scope ID to the value, set for Local Address.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - handle of Local Address object - source of Scope ID
 *          pDestAddr  - pointer to the address to be fixed
 * Output:  none.
 *****************************************************************************/
void SipTransportFixIpv6DestinationAddr(
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            OUT RvAddress*                    pDestAddr)
{
#if(RV_NET_TYPE & RV_NET_IPV6)
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return;
    }
    TransportMgrAddrCopyScopeId(&pLocalAddr->addr.addr, pDestAddr);
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return;
#else
        RV_UNUSED_ARG(hLocalAddr)
        RV_UNUSED_ARG(pDestAddr)
#endif
}

/***************************************************************************
 * SipTransportConnectionGetRemoteAddress
 * ------------------------------------------------------------------------
 * General: Copy the remote address struct from conection to the given address object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - the connection that its address we seek
 * Output:  pRemoteAddress  - A previously allocated space for the address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionGetRemoteAddress(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT SipTransportAddress    *pRemoteAddress)
{

    RvStatus rv;
    TransportConnection    *pConn = (TransportConnection*)hConn;

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memcpy((void*)pRemoteAddress, (void*)&pConn->destAddress, sizeof(SipTransportAddress));

    TransportConnectionUnLockAPI(pConn);

    return RV_OK;
}

/******************************************************************************
 * SipTransportLocalAddressGetNumIpv4Ipv6Ips
 * ----------------------------------------------------------------------------
 * General: For Local Addresses, socket of that is bound to few IPs
 *          (such as SCTP address) - get number of IPv4 and IPv6 IPs.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr    - Handle to Local Address object
 * Output:  pNumOfIpv4    - Number of IPv4 IPs, to which the socket is bound
 *          pNumOfIpv6    - Number of IPv6 IPs, to which the socket is bound
 *****************************************************************************/
RvStatus SipTransportLocalAddressGetNumIpv4Ipv6Ips(
                            IN  RvSipTransportLocalAddrHandle hLocalAddress,
                            OUT RvUint32*                     pNumOfIpv4,
                            OUT RvUint32*                     pNumOfIpv6)
{
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvStatus rv;
    TransportMgrLocalAddr* pLocalAddr = (TransportMgrLocalAddr*)hLocalAddress;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }
    rv = TransportSctpLocalAddressGetNumIpv4Ipv6Ips(pLocalAddr,
                                                    pNumOfIpv4, pNumOfIpv6);
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return rv;
#else
    RV_UNUSED_ARG(hLocalAddress);
    RV_UNUSED_ARG(pNumOfIpv4);
    RV_UNUSED_ARG(pNumOfIpv6);
    return RV_OK;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionGetTransport
 * ------------------------------------------------------------------------
 * General: Returns the transport type of this connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - the connection that its address we seek
 ***************************************************************************/
RvSipTransport RVCALLCONV SipTransportConnectionGetTransport(
                                        IN  RvSipTransportConnectionHandle hConn)
{
	TransportConnection    *pConn = (TransportConnection*)hConn;

    return pConn->eTransport;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransportConnectionSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Connection object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient - Handle to the Reg-Client object.
 *          pParams    - Parameters to be set.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransportConnectionSetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    TransportConnection *pConn = (TransportConnection*)hConn;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return;
    }
    memcpy(&pConn->sctpData.sctpParams, pParams, sizeof(RvSipTransportSctpMsgSendingParams));
    TransportConnectionUnLockAPI(pConn);
}

/***************************************************************************
 * SipTransportConnectionGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Connection object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient - Handle to the Reg-Client object.
 * Output:  pParams    - Parameters to be set.
 ***************************************************************************/
void RVCALLCONV SipTransportConnectionGetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    TransportConnection *pConn = (TransportConnection*)hConn;
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return;
    }
    memcpy(pParams, &pConn->sctpData.sctpParams, sizeof(RvSipTransportSctpMsgSendingParams));
    TransportConnectionUnLockAPI(pConn);
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionSetSecOwner
 * ----------------------------------------------------------------------------
 * General: set object, implemented in Security Module, as Security Owner of 
 *          the connection. The function is used by the Security module.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn       - Details of the address to be opened
 *          pSecOwner   - Pointer to the security owner
 *          bDisconnect - If RV_TRUE, and pSecOwner is NULL, disconnection of
 *                        the connection will be initiated.
*****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetSecOwner(
                                IN RvSipTransportConnectionHandle hConn,
                                IN void*                          pSecOwner,
                                IN RvBool                         bDisconnect)
{
    RvStatus              rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    TransportMgr*         pTransportMgr;
    
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pConn->pTransportMgr;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "SipTransportConnectionSetSecOwner - Set Security Owner %p into connection %p",
        pSecOwner, pConn));

    if (NULL != pConn->pSecOwner  &&  NULL != pSecOwner)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportConnectionSetSecOwner - Current Owner %p will be overridden",
            pConn->pSecOwner))
    }

    pConn->pSecOwner = pSecOwner;

    /* If Security Owner is removed, disconnect the connection, if there are no
    other owners */
    if (NULL == pConn->pSecOwner  &&  RV_TRUE == bDisconnect)
    {
        rv = TransportConnectionDisconnectOnNoOwners(pConn, RV_TRUE/*bForce*/);
        if (RV_OK != rv)
        {
            TransportConnectionUnLockAPI(pConn);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportConnectionSetSecOwner - pConn %p: failure to disconnect the connection(rv=%d:%s)",
                pConn, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportLocalAddressSetSecOwner
 * ----------------------------------------------------------------------------
 * General: set object, implemented in Security Module, as Security Owner of 
 *          the Local Address. The function is used by the Security module.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr    - Handle of the Local Address to be closed.
 *          pSecOwner     - Pointer to the Security Owner.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressSetSecOwner(
                        IN RvSipTransportLocalAddrHandle   hLocalAddr,
                        IN void*                           pSecOwner)
{
    RvStatus               rv;
    TransportMgrLocalAddr* pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    TransportMgr*          pTransportMgr;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    pTransportMgr = pLocalAddr->pMgr;
    
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "SipTransportLocalAddressSetSecOwner - Set Security Owner %p into Local Address %p",
        pSecOwner, pLocalAddr));
    
    if (NULL != pLocalAddr->pSecOwner  &&  NULL != pSecOwner)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportLocalAddressSetSecOwner - Current Owner %p will be overridden",
            pLocalAddr->pSecOwner));
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(pLocalAddr->pSecOwner != pSecOwner) 
    {
       if (NULL != pSecOwner)
       {
          pLocalAddr->secOwnerCounter++;
          pLocalAddr->pSecOwner = pSecOwner;
       }
       else
       {
          pLocalAddr->secOwnerCounter--;
          if(!pLocalAddr->secOwnerCounter)
             pLocalAddr->pSecOwner = NULL;
       }
    }
#else      
      pLocalAddr->pSecOwner = pSecOwner;
      
      if (NULL != pSecOwner)
      {
        pLocalAddr->secOwnerCounter++;
      }
      else
      {
        pLocalAddr->secOwnerCounter--;
        
      }
#endif
/* SPIRENT_END */

    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportSetSecEventHandlers
 * ----------------------------------------------------------------------------
 * General: Set callback functions, implemented in the Security Module,
 *          into the Transport Manager.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager.
 *          evHandlers      - struct with pointers to callback functions.
 *          evHandlersSize  - ev handler struct size
 *          hContext - Will be used as a parameter for the callbacks.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSetSecEventHandlers(
                                IN RvSipTransportMgrHandle    hTransportMgr,
                                IN SipTransportSecEvHandlers* pEvHandlers)
{
    TransportMgr *pTransportMgr;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if (pTransportMgr == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    
    memcpy(&pTransportMgr->secEvHandlers, pEvHandlers, 
           sizeof(SipTransportSecEvHandlers));
    
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "SipTransportSetSecEventHandlers - event handlers for Security Module were set"));
    
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportMgrLocalAddressAdd
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
 *                            the address to be added are stored
 *          bDontOpen       - if RV_TRUE, no socket will be opened,
 *                            for TCP/TLS/SCTP address - no listening
 *                            connection will be constructed.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
*****************************************************************************/
RvStatus RVCALLCONV SipTransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr*             pAddressDetails,
                        IN  RvBool                          bDontOpen,
                        OUT RvSipTransportLocalAddrHandle*  phLocalAddr)
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
            "SipTransportMgrLocalAddressAdd - NULL handle was supplied"));
        return RV_OK;
    }

    
    rv = TransportMgrLocalAddressAdd(hTransportMgr,pAddressDetails,
            sizeof(RvSipTransportAddr), RVSIP_LAST_ELEMENT/*eLocationInList*/,
            NULL/*hBaseLocalAddr*/, RV_FALSE/*bConvertZeroPort*/, bDontOpen,
            phLocalAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportMgrLocalAddressAdd - Failed to add local %s address %s:%d. Dont open=%d(rv=%d)",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP, pAddressDetails->port, bDontOpen, rv));
        return rv;
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "SipTransportMgrLocalAddressAdd - Local %s address %s:%d was successfully added. Handle is 0x%p",
        SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
        pAddressDetails->strIP,pAddressDetails->port,*phLocalAddr));

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: Returns the details of the local address, the handle of which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      -  handle of the Local Address.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        OUT RvSipTransportAddr              *pAddressDetails)
{
    RvStatus      rv;
    TransportMgr* pTransportMgr;
    
    pTransportMgr = ((TransportMgrLocalAddr*)hLocalAddr)->pMgr;
    
    rv = TransportMgrLocalAddressGetDetails(hLocalAddr, pAddressDetails);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "SipTransportMgrLocalAddressGetDetails - Failed to get details of local address 0x%p (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: retrieves handle to the local address, used by the Connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn          - The handle to the Connection
 * Output:  phLocalAddress - Handle to the Local Address
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionGetLocalAddress(
                        IN  RvSipTransportConnectionHandle hConn,
                        OUT RvSipTransportLocalAddrHandle* phLocalAddress)
{
    TransportConnection* pConn = (TransportConnection*)hConn;
    RvStatus             rv             = RV_OK;

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phLocalAddress = pConn->hLocalAddr;

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return rv;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportLocalAddressSetImpi
 * ------------------------------------------------------------------------
 * General: set IMPI into the Local Address. The IMPI is provided by
 *          the Application through Security Agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - The handle to the Connection
 *          impi  - IMPI context
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressSetImpi(
                        IN RvSipTransportLocalAddrHandle hLocalAddress,
                        IN void*                         impi)
{
    RvStatus               rv;
    TransportMgrLocalAddr* pLocalAddr = (TransportMgrLocalAddr*)hLocalAddress;
    TransportMgr*          pTransportMgr;
    RvSipTransportConnectionHandle hConnListening;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransportMgr = pLocalAddr->pMgr;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
        "SipTransportLocalAddressSetImpi - Set IMPI %p into Local Address %p",
        impi, pLocalAddr));

    pLocalAddr->impi = impi;

    hConnListening = pLocalAddr->hConnListening;
    TransportMgrLocalAddrUnlock(pLocalAddr);
    
    /* Set the IMPI into the listening connection also */
    if (NULL != hConnListening)
    {
        rv = SipTransportConnectionSetImpi(hConnListening, impi);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "SipTransportLocalAddressSetImpi - Failed to set IMPI %p into listening connection %p(rv=%d:%s)",
                impi, hConnListening, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionSetImpi
 * ------------------------------------------------------------------------
 * General: set IMPI into the Connection. The IMPI is provided by
 *          the Application through Security Agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - The handle to the Connection
 *          impi  - IMPI context
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetImpi(
                        IN RvSipTransportConnectionHandle hConn,
                        IN void*                          impi)
{
    RvStatus              rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc, (pConn->pTransportMgr->pLogSrc,
        "SipTransportConnectionSetImpi - Set IMPI %p into connection %p",
        impi, pConn));

    pConn->impi = impi;

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionOpenDontConnect
 * ----------------------------------------------------------------------------
 * General: opens client connection, bound to the specified local port.
 *          Doesn't connect it.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn     - The handle to the Connection
 *          localPort - Port, to which the connection socket should be bound
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionOpenDontConnect(
                        IN RvSipTransportConnectionHandle hConn,
//SPIRENT_BEGIN
#ifdef UPDATED_BY_SPIRENT
                        IN RvUint16                       *localPort)
#else
                        IN RvUint16                       localPort)
#endif
//SPIRENT_END
{

#if !defined(_MSC_VER) //!!!SPIRENT

    RvStatus              rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pConn->pTransportMgr->pLogSrc, (pConn->pTransportMgr->pLogSrc,
        "SipTransportConnectionOpenDontConnect - hConn %p, local port = %d",
/*SPIRENT_BEGIN*/
#ifdef UPDATED_BY_SPIRENT
        hConn, *localPort));
#else
        hConn, localPort));
#endif
/*SPIRENT_END*/

/*SPIRENT_BEGIN*/
#ifdef UPDATED_BY_SPIRENT
    rv = TransportConnectionOpen(pConn, RV_FALSE/*bConnect*/, *localPort);
#else
    rv = TransportConnectionOpen(pConn, RV_FALSE/*bConnect*/, localPort);
#endif
/*SPIRENT_END*/

    if (RV_OK != rv)
    {
        TransportConnectionUnLockAPI(pConn);
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "SipTransportConnectionOpenDontConnect - failed to open connection %p(rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

/*SPIRENT_BEGIN*/
#ifdef UPDATED_BY_SPIRENT
    *localPort = pConn->localPort;
#endif
/*SPIRENT_END*/

    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

#endif //!!!SPIRENT

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionSetDestAddress
 * ----------------------------------------------------------------------------
 * General: sets destination address into connection.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn    - The handle to the Connection
 *          pAddress - Destination common core address
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetDestAddress(
                        IN RvSipTransportConnectionHandle hConn,
                        IN RvAddress*                     pAddress)
{
    RvStatus              rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    RvChar                strIP[RVSIP_TRANSPORT_LEN_STRING_IP];

    /*To prevent compilation warning on Nucleus, while compiling without logs*/
    RV_UNUSED_ARGS(strIP);

    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc, (pConn->pTransportMgr->pLogSrc,
        "SipTransportConnectionSetDestAddress - hConn %p: %s:%d",
        hConn,
        RvAddressGetString(pAddress,RVSIP_TRANSPORT_LEN_STRING_IP,strIP),
        RvAddressGetIpPort(pAddress)));

    memcpy(&pConn->destAddress.addr, pAddress, sizeof(RvAddress));
    pConn->destAddress.eTransport = pConn->eTransport;

    TransportConnectionUnLockAPI(pConn);
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

/******************************************************************************
 * SipTransportConnectionRemoveOwnerMsgs
 * ----------------------------------------------------------------------------
 * General: remove all messages of a specific owner from connection send list.
 * Return Value: void
 * ----------------------------------------------------------------------------
 * Arguments:
 * Output:     pConn - handle to the connection to be set.
 *            hOwner - handle to the connection owner.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionRemoveOwnerMsgs(
                      IN RvSipTransportConnectionHandle      hConn,
                      IN RvSipTransportConnectionOwnerHandle hOwner)
{
    RvStatus              rv;
    TransportConnection*  pConn = (TransportConnection*)hConn;
    
    rv = TransportConnectionLockAPI(pConn);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogInfo(pConn->pTransportMgr->pLogSrc, (pConn->pTransportMgr->pLogSrc,
        "SipTransportConnectionRemoveOwnerMsgs - hConn %p, Owner %p",
        hConn, hOwner));

    TransportConnectionRemoveOwnerMsgs(pConn, hOwner);

    TransportConnectionUnLockAPI(pConn);
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/******************************************************************************
 * TransportSendObjectEvent
 * ----------------------------------------------------------------------------
 * General: Insert an INTERNAL_OBJECT_EVENT event into the Transport Processing
 *          Queue.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr - transport handler
 *          pObj          - Pointer to the object sending the event.
 *          objUniqueIdentifier - the unique id of the object to terminate. when the event
 *                                pops, before handling the event we check that the object
 *                                was not reallocated, using this identifier.
 *          pEventInfo    - empty allocated structure.
 *          param1        - param, provided to the sending Object on pop event 
 *          param2        - param, provided to the sending Object on pop event 
 *          func          - callback function
 *          strLogInfo    - message limited to TRANSPORT_EVENT_LOG_LEN len
 *          strObjType    - type of the sending object
 *****************************************************************************/
static RvStatus RVCALLCONV TransportSendObjectEvent(
                                IN RvSipTransportMgrHandle      hTransportMgr,
                                IN void*                        pObj,
                                IN RvInt32                      objUniqueIdentifier,
                                IN RvSipTransportObjEventInfo*  pEventInfo,
                                IN RvInt32                      param1,
                                IN RvInt32                      param2,
                                IN RvSipTransportObjectEventHandler     terminateFunc,
                                IN SipTransportObjectStateEventHandler  stateFunc,
                                IN RvBool                              bInternal,
                                IN const RvChar*                strLogInfo,
                                IN const RvChar*                strObjType)
{
    RvStatus       rv;
    TransportMgr*  pTransportMgr;
    TransportProcessingQueueEvent* ev;

    if ((hTransportMgr == NULL) || (pObj == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = (TransportMgr*)hTransportMgr;

    /* If the object was already inserted to the termination queue, or to the OOR list
       - do not insert again */
    if(pEventInfo != NULL && RV_TRUE == pEventInfo->bInTermination)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportSendObjectEvent - pObj=0x%p - the object is already in the termination queue/OOR list",
            pObj));
        return RV_OK;
    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportSendObjectEvent - pObj=0x%p,pEventInfo=0x%p,param1=%d,param2=%d",
        pObj,pEventInfo,param1,param2));

    rv = TransportProcessingQueueAllocateEvent(hTransportMgr,
                                NULL/*hConn*/,
                                ((RV_TRUE == bInternal) ? INTERNAL_OBJECT_EVENT : EXTERNAL_OBJECT_EVENT),
                                RV_FALSE/*bAllocateRcvdBuff*/, &ev);
    if (RV_OK != rv)
    {
		if (pEventInfo == NULL)
		{
		  /* This is not a termination event */
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportSendObjectEvent - Failed to allocate event for %s 0x%p: %s (not for termination), returning rv=%d",
              strObjType, pObj, strLogInfo, rv));
			return rv;
		}
		RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportSendObjectEvent - Failed to allocate event for %s 0x%p: %s, Save event for oor recovery.",
              strObjType, pObj, strLogInfo));
        
		
        /*If event is already in list, don't save it again. Just return error*/
        if (pEventInfo->bInTermination == RV_TRUE)
        {
            RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportSendObjectEvent - ignore %s event for 0x%p at this time",
                  strObjType, pObj));
            return RV_OK;
        }

        RvMutexLock(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
        if ((pEventInfo->next == NULL) &&
            (pEventInfo != pTransportMgr->pEventToBeNotified))
        { 
            /* Element that was not already in the list - this check is for reSend option */
            pEventInfo->next   = pTransportMgr->pEventToBeNotified;
            pEventInfo->reason = param1;
            pEventInfo->func   = terminateFunc;
            pEventInfo->objHandle   = pObj;
            pTransportMgr->pEventToBeNotified = pEventInfo;
			pEventInfo->bInTermination = RV_TRUE;
            
        }
        RvMutexUnlock(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
        return RV_OK;
    }

    ev->typeSpecific.objEvent.obj         = pObj;
    ev->typeSpecific.objEvent.param1      = param1;
    ev->typeSpecific.objEvent.param2      = param2;
    ev->typeSpecific.objEvent.objUniqueIdentifier = objUniqueIdentifier;
    ev->typeSpecific.objEvent.eventFunc   = terminateFunc;
    ev->typeSpecific.objEvent.eventStateFunc = stateFunc;
#if (RV_LOGMASK & RV_LOGLEVEL_DEBUG)
    {

        if (strlen(strObjType)+strlen(strLogInfo)+HANDLE_STR_SIZE < TRANSPORT_EVENT_LOG_LEN-1)
        {
            RvSprintf(ev->typeSpecific.objEvent.strLoginfo,"%s 0x%p: %s",strObjType,pObj,strLogInfo);
        }
        else
        {
            RvSprintf(ev->typeSpecific.objEvent.strLoginfo,"obj 0x%p:",pObj);
        }
    }
#endif /*RV_LOGLEVEL_DEBUG*/


    rv = TransportProcessingQueueTailEvent(hTransportMgr,ev);
    if(RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportSendObjectEvent - Failed to add object event %s to the queue",
              ev->typeSpecific.objEvent.strLoginfo));
        TransportProcessingQueueFreeEvent(hTransportMgr,ev);
        return rv;
    }
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportSendObjectEvent - ev 0x%p was sent. (%s)",
              ev,ev->typeSpecific.objEvent.strLoginfo));


#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((strObjType,strLogInfo));
#endif

    return RV_OK;
}



#ifdef __cplusplus
}
#endif
