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
 *                              TransportMgrObject.c
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                 08/12/02
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"

#include "TransportMgrObject.h"
#include "rvmemory.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#include "rvtls.h"
#include "TransportUDP.h"
#include "TransportConnection.h"
#include "_SipTransport.h"
#include "TransportProcessingQueue.h"
#include "RvSipTransportTypes.h"
#include "TransportPreemption.h"
#include "rvansi.h"
#include "_SipCommonCore.h"
#include "rvhost.h"
#include "_SipCommonUtils.h"
#include "TransportDNS.h"

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Socket HW binding to Ehernet Interface
 */
#if defined(UPDATED_BY_SPIRENT)

#include <net/if.h>
#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTPTypes.h"
#include "TransportSCTP.h"
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */
#include "rvaddress.h"
    
    
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#define  HASH_SIZE          (4096*2+1)   /* Local Addresses Hash Table size - for holding max of 4096 unique addresses   */
#define  HASH_ON_TRESHOLD   32           /* Number of IPs to activate hashing */

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */
#define SEND_RCV_BUFFER_SIZE 0x8000
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV ConstructConnectionResources(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg);

static RvStatus RVCALLCONV ConstructLocalAddressLists(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg);

static RvStatus RVCALLCONV InitializeLocalAddressLists(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg);

static RvStatus RVCALLCONV LocalAddressRemoveFromList(
                                    IN  TransportMgr*         pTransportMgr,
                                    IN  TransportMgrLocalAddr *pAddress);

static RvStatus RVCALLCONV LocalAddressOpen(
                                    IN  TransportMgr*          pTransportMgr,
                                    IN  TransportMgrLocalAddr* pAddress,
                                    OUT RvUint16*              pBoundPort);

static RvStatus RVCALLCONV LocalAddressClose(
                                    IN  TransportMgr*         pTransportMgr,
                                    IN  TransportMgrLocalAddr *pAddress);

static RvStatus RVCALLCONV OpenLocalAddresses(
                                    IN    TransportMgr     *pTransportMgr,
                                    IN SipTransportCfg     *pTransportCfg);

static RvStatus RVCALLCONV InitializeOutboundAddress(
                                    IN TransportMgr        *pTransportMgr,
                                    IN SipTransportCfg     *pTransportCfg);

static RvStatus MultiThreadedEnvironmentConstruct(
                                        IN TransportMgr      *pTransportMgr,
                                        IN SipTransportCfg    *pTransportCfg);

static void MultiThreadedEnvironmentDestruct(
                                        IN TransportMgr    *pTransportMgr);


static RvBool MultiThreadedTimerEventHandler(
                                        IN void               *context);

static RvStatus ProcessingThreadConstruct(IN  TransportMgr *pTransportMgr,
                                          IN  RvInt32       threadIndex,
                                          OUT RvThread     *pProcThread);

static void ProcessingThreadArrayDestruct(IN TransportMgr *pTransportMgr,
                                          IN RvUint32      numOfProcThreads);

static RvStatus RVCALLCONV OpenLocalAddressesFromList(
                            IN    TransportMgr          *pTransportMgr,
                            IN    RLIST_HANDLE           hAddrList);
#if (RV_TLS_TYPE != RV_TLS_NONE)
static RvStatus RVCALLCONV ConstructTLSResources(
                                        IN  TransportMgr    *pTransportMgr);

#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

static void LocalAddressCounterUpdate(
                                    IN  TransportMgr            *pTransportMgr,
                                    IN  RvSipTransportAddressType eIPAddrType,
                                    IN  RvSipTransport          eTransportType,
                                    IN  RvInt32                 delta);

static void InitStructTransportAddr (
                                    IN RvSipTransport       eTransportType,
                                    IN RvSipTransportAddressType eAddrType,
                                    IN RvChar               *strIP,
                                    IN RvUint16             port,
#if defined(UPDATED_BY_SPIRENT)
				    IN RvChar               *if_name,
				    IN RvUint16             vdevblock,
#endif
                                    OUT RvSipTransportAddr  *pTransportAddrStruct);

#if (RV_NET_TYPE & RV_NET_IPV6)
static void RVCALLCONV ConvertIpStringTransport2CommonCore(
                                    IN  RvChar*   strTransportIp,
                                    OUT RvChar*   strCcIp,
                                    OUT RvInt32*  pScopeId);
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

static RvUint HashFunc_LocalAddr    ( IN void *Param );
static RvBool HashCompare_LocalAddr ( IN void *Param1 , IN void *Param2 );

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                                 GLOBALS                               */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * TransportMgrInitialize
 * ----------------------------------------------------------------------------------
 * General: initialize the transport manager object.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   TransportMgr        - manager object
 *          SipTransportCfg     - manager configuration.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrInitialize(
                                   IN    TransportMgr*     pTransportMgr,
                                   INOUT SipTransportCfg*  pTransportCfg)
{
    RvStatus                       rv              = RV_OK;
    RvInt                          numofSockets    = 5; /* 5 "extra" sockets*/
    RvStatus                       crv             = RV_OK;
    RvAddress                      dummyAddr[RV_HOST_MAX_ADDRS];
    RvUint32                       dummyNumOfAddr = RV_HOST_MAX_ADDRS;
    /*initialize manager structure*/
    pTransportMgr->pLogMgr            = pTransportCfg->pLogMgr;
    pTransportMgr->pLogSrc            = pTransportCfg->regId;
    pTransportMgr->pMBLogSrc          = pTransportCfg->pMBLogSrc;
    pTransportMgr->hMsgMgr            = pTransportCfg->hMsgMgr;
    pTransportMgr->hMsgPool           = pTransportCfg->hMsgPool;
    pTransportMgr->hGeneralPool       = pTransportCfg->hGeneralPool;
    pTransportMgr->hElementPool       = pTransportCfg->hElementPool;
    pTransportMgr->maxBufSize         = pTransportCfg->maxBufSize;
    pTransportMgr->seed               = pTransportCfg->seed;
    pTransportMgr->hStack             = pTransportCfg->hStack;
    pTransportMgr->pEventToBeNotified = NULL;
    pTransportMgr->maxPageNuber       = pTransportCfg->maxPageNuber;
    pTransportMgr->maxConnections     = pTransportCfg->maxConnections;
    pTransportCfg->maxDNSQueries      = 100;
    pTransportMgr->maxDNSQueries      = pTransportCfg->maxDNSQueries;
    pTransportMgr->maxConnOwners      = pTransportCfg->maxConnOwners;
    pTransportMgr->tcpEnabled         = pTransportCfg->tcpEnabled;
    pTransportMgr->bIsShuttingDown    = RV_FALSE;
    pTransportMgr->ePersistencyLevel  = pTransportCfg->ePersistencyLevel;
    pTransportMgr->serverConnectionTimeout = pTransportCfg->serverConnectionTimeout;
    pTransportMgr->bDLAEnabled        = pTransportCfg->bDLAEnabled;

    pTransportMgr->connectionCapacityPercent = pTransportCfg->connectionCapacityPercent;
    pTransportMgr->bMultiThreadEnvironmentConstructed = RV_FALSE;
	pTransportMgr->preemptionCellAvailable    = 0;
	pTransportMgr->preemptionDispatchEvents   = 0;
	pTransportMgr->preemptionReadBuffersReady = 0;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

    pTransportMgr->localPriority     = pTransportCfg->localPriority;

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

#if (RV_TLS_TYPE != RV_TLS_NONE)
    pTransportMgr->tlsMgr.numOfEngines      = pTransportCfg->numOfTlsEngines;
    pTransportMgr->tlsMgr.maxSessions       = pTransportCfg->maxTlsSessions;
    pTransportMgr->tlsMgr.numOfTlsAddresses    = pTransportCfg->numOfTlsAddresses;
    pTransportMgr->tlsMgr.TlsHandshakeTimeout  = pTransportCfg->TlsHandshakeTimeout;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    pTransportMgr->noEnoughBuffersInPool        = RV_FALSE;
    pTransportMgr->notEnoughBuffersInPoolForUdp = RV_FALSE;
    pTransportMgr->timerOOR                     = RV_FALSE;

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    pTransportMgr->maxElementsInSingleDnsList = pTransportCfg->maxElementsInSingleDnsList;
#endif
    pTransportMgr->maxTimers    = pTransportCfg->maxTimers;

#if (RV_NET_TYPE & RV_NET_SCTP)
    TransportSctpMgrSetConfiguration(pTransportMgr, pTransportCfg);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    /* num of sockets allocates is:
       1 (first udp) + maxConnections + numOfExtraUdpAddresses */
    numofSockets += pTransportCfg->maxNumOfLocalAddresses;
    numofSockets += (pTransportMgr->maxConnections < 0 ) ? 0 : pTransportMgr->maxConnections;
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportMgrInitialize - max num of sockets for the stack is: %d", numofSockets))

    crv = RvSelectConstruct(numofSockets,
                           pTransportMgr->maxTimers,
                           pTransportMgr->pLogMgr,
                           &pTransportMgr->pSelect);
    if (RV_OK != crv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct selecet Engine"))
        return CRV2RV(crv);
    }

#define SOCKET_OOR_RATIO 10
    pTransportMgr->oorListPool = RLIST_PoolListConstruct(SOCKET_OOR_RATIO*numofSockets+5,
                                                         1,
                                                         sizeof(TransportConnection *),
                                                         pTransportMgr->pLogMgr,
                                                         "OORPool");
    if (NULL == pTransportMgr->oorListPool)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct OOR pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pTransportMgr->oorList =  RLIST_ListConstruct(pTransportMgr->oorListPool);
    if (NULL == pTransportMgr->oorList)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct OOR list"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    rv = TransportPreemptionConstruct(pTransportMgr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrInitialize - could not construct peemption (rv=%d)",rv));
        return rv;
    }
    
    /* construct send and receive buffers */
    pTransportMgr->hraSendBuffer = RA_Construct (pTransportMgr->maxBufSize,
                                                 1,
                                                 NULL,
                                                 pTransportMgr->pLogMgr,
                                                 "SendBuffer");
    if(pTransportMgr->hraSendBuffer == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct send buffers"))
        return RV_ERROR_UNKNOWN;

    }

    rv = RA_Alloc(pTransportMgr->hraSendBuffer, &(pTransportMgr->sendBuffer));
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed allocating buffers"))
        return rv;
    }

    pTransportMgr->hraRecvBuffer = RA_Construct (pTransportMgr->maxBufSize,
                                                 1,
                                                 NULL,
                                                 pTransportMgr->pLogMgr,
                                                 "RecvBuffer");
    if(pTransportMgr->hraRecvBuffer == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct rcv buffers"))
        return RV_ERROR_UNKNOWN;

    }
    rv = RA_Alloc(pTransportMgr->hraRecvBuffer, &(pTransportMgr->recvBuffer));
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed allocating buffers"))
        return rv;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (pTransportCfg->maxTlsSessions >0)
    {
        rv = ConstructTLSResources(pTransportMgr);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrInitialize - Failed to construct TLS resources"));
            return rv;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    /* Call RvHostLocalGetAddress which takes significant time the first time
       it is called. This is a caching call to fill the core local addresses */
    RvHostLocalGetAddress(pTransportMgr->pLogMgr,&dummyNumOfAddr,(RvAddress*)dummyAddr);


    /*construct all resources needed for tcp*/
    rv = ConstructConnectionResources(pTransportMgr,pTransportCfg);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrInitialize - Failed to construct local addresses lists"));
        return rv;
    }

    /*construct local address lists*/
    rv = ConstructLocalAddressLists(pTransportMgr,pTransportCfg);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct local addresses lists"));
        return rv;
    }


    if(0 < pTransportMgr->connectionCapacityPercent)
    {
        /* construct the 'connected-no-owner' connection list */
        pTransportMgr->hConnectedNoOwnerConnPool = RLIST_PoolListConstruct(
                        pTransportMgr->maxConnections, 1, sizeof(RvSipTransportConnectionHandle), 
                        pTransportMgr->pLogMgr, "Connected No Owners Conn List");
        if(pTransportMgr->hConnectedNoOwnerConnPool == NULL)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrInitialize - Failed to construct connected-no-owner-conn pool"));
            return RV_ERROR_OUTOFRESOURCES;
        }
        pTransportMgr->hConnectedNoOwnerConnList = RLIST_ListConstruct(pTransportMgr->hConnectedNoOwnerConnPool);
        if(pTransportMgr->hConnectedNoOwnerConnList == NULL)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrInitialize - Failed to construct connected-no-owner-conn list"));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        pTransportMgr->hConnectedNoOwnerConnList = NULL;
        pTransportMgr->hConnectedNoOwnerConnPool = NULL;
    }
#if (RV_NET_TYPE & RV_NET_SCTP)
    /* SCTP initialization should be done before local address opening */
    if (RV_TRUE == pTransportMgr->tcpEnabled)
    {
        rv = TransportSctpMgrConstruct(pTransportMgr);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrInitialize - Failed to construct SCTP Manager"));
            return rv;
        }
    }
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

	
	/*initialize the local addresses only if needed*/
	if(pTransportCfg->bIgnoreLocalAddresses == RV_TRUE)
	{
        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrInitialize - bIgnoreLocalAddresses = 1, Ignoring all configured local addresses"));
	}
	else
	{
		rv = InitializeLocalAddressLists(pTransportMgr,pTransportCfg);
		if(rv != RV_OK)
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportMgrInitialize - Failed to initialize local addresses lists"));
			return rv;
			
		}
		
		rv = OpenLocalAddresses(pTransportMgr, pTransportCfg);
		if(rv != RV_OK)
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportMgrInitialize - Failed to open local addresses"));
			return rv;
		}
	}

    rv = InitializeOutboundAddress(pTransportMgr,pTransportCfg);
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to initialize outbound address"));
        return rv;
    }


    /* Constructing processing queue, memory buffers pool and initiating
       processing threads */
    rv = MultiThreadedEnvironmentConstruct(pTransportMgr,pTransportCfg);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "TransportMgrInitialize - Failed to construct multithreaded environment"));
        return rv;
    }

    TransportMgrPrintLocalAddresses(pTransportMgr);

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportMgrInitialize - Transport was constructed successfully. The handle is 0x%p",
               (RvSipTransportMgrHandle)pTransportMgr));

    return RV_OK;
}

/************************************************************************************
 * TransportMgrDestruct
 * ----------------------------------------------------------------------------------
 * General: Destruct the transport manager.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - Pointer to the transport manager
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrDestruct(IN TransportMgr*  pTransportMgr)
{
    RvStatus            rv          = RV_OK;
    RLIST_ITEM_HANDLE   hListElem   = NULL;

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportMgrDestruct - Destructing the transport manager"));

    pTransportMgr->bIsShuttingDown   = RV_TRUE;
    MultiThreadedEnvironmentDestruct(pTransportMgr);
    TransportPreemptionDestruct(pTransportMgr);

    if (pTransportMgr->tcpEnabled==RV_TRUE)
    {
        if(pTransportMgr->hConnList != NULL)
        {
            rv = TransportConnectionListDestruct(pTransportMgr);
            if (RV_OK != rv)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                           "TransportMgrDestruct - Failed to destruct connection list."));
            }
        }
        if(pTransportMgr->hConnHash != NULL)
        {
            HASH_Destruct(pTransportMgr->hConnHash);
            pTransportMgr->hConnHash= NULL;
        }
        if(pTransportMgr->hConnOwnersHash != NULL)
        {
            HASH_Destruct(pTransportMgr->hConnOwnersHash);
            pTransportMgr->hConnOwnersHash= NULL;
        }

        if(pTransportMgr->hMsgToSendPoolList != NULL)
        {
            RLIST_PoolListDestruct(pTransportMgr->hMsgToSendPoolList);
            pTransportMgr->hMsgToSendPoolList= NULL;
            RvMutexDestruct(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        }
        if(pTransportMgr->hConnectionOwnersPoolList)
        {
            RLIST_PoolListDestruct(pTransportMgr->hConnectionOwnersPoolList);
            pTransportMgr->hConnectionOwnersPoolList= NULL;
        }

#if (RV_NET_TYPE & RV_NET_SCTP)
        TransportSctpMgrDestruct(pTransportMgr);
#endif /*ENDOF: #if (RV_NET_TYPE & RV_NET_SCTP)*/
    }
    if (NULL != pTransportMgr->hLocalUdpAddrList)
    {
        /* Close the UDP addresses */
        TransportMgrLocalAddrListLock(pTransportMgr);

        RLIST_GetHead(  pTransportMgr->hLocalAddrPool,
            pTransportMgr->hLocalUdpAddrList,&hListElem);
        while (NULL != hListElem)
        {
            TransportMgrLocalAddr  *pLocalAddr = (TransportMgrLocalAddr *)hListElem;
            rv = TransportUDPClose(pTransportMgr,pLocalAddr);
            if (RV_OK != rv)
            {
                RvUint16 port;
                RvChar  strIP[RVSIP_TRANSPORT_LEN_STRING_IP];
                RvAddressGetString(&pLocalAddr->addr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strIP);
                port = RvAddressGetIpPort(&pLocalAddr->addr.addr);
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportMgrDestruct: TransportUDPClose(UDP address %s:%d) failed",
                    strIP,port));
                /*continue destruction on failure*/
            }

            RLIST_GetNext(  pTransportMgr->hLocalAddrPool,
                pTransportMgr->hLocalUdpAddrList,hListElem, &hListElem);
        }
        TransportMgrLocalAddrListUnlock(pTransportMgr);
    }

    /* Free pool of Local Addresses */
    if (NULL != pTransportMgr->hLocalAddrPool)
    {
        TransportMgrLocalAddr* pLocalAddr;
        /* Run through pool of local addresses and free locks */
        if (RV_FALSE != pTransportMgr->bDLAEnabled)
        {
            RLIST_PoolGetFirstElem(pTransportMgr->hLocalAddrPool,
                (RLIST_ITEM_HANDLE*)&pLocalAddr);
            while (NULL != pLocalAddr)
            {
                RvMutexDestruct(&pLocalAddr->hLock,pTransportMgr->pLogMgr);
                RLIST_PoolGetNextElem(pTransportMgr->hLocalAddrPool,
                    (RLIST_ITEM_HANDLE)pLocalAddr,
                    (RLIST_ITEM_HANDLE*)&pLocalAddr);
            }
        }
        RLIST_PoolListDestruct(pTransportMgr->hLocalAddrPool);
        pTransportMgr->hLocalAddrPool= NULL;
    }

#if defined(UPDATED_BY_SPIRENT) && (RV_TLS_TYPE != RV_TLS_NONE)

    /* Free Local UDP Addresses Hash Tables */
    if( pTransportMgr->hLocalUdpAddrHash != NULL )
    {
        HASH_Destruct(pTransportMgr->hLocalUdpAddrHash);
        pTransportMgr->hLocalUdpAddrHash = NULL;
    }

    /* Free Local TCP Addresses Hash Tables */
    if( pTransportMgr->hLocalTcpAddrHash != NULL )
    {
        HASH_Destruct(pTransportMgr->hLocalTcpAddrHash);
        pTransportMgr->hLocalTcpAddrHash = NULL;
    }

    /* Free Local TLS Addresses Hash Tables */
    if( pTransportMgr->tlsMgr.hLocalTlsAddrHash != NULL )
    {
        HASH_Destruct(pTransportMgr->tlsMgr.hLocalTlsAddrHash);
        pTransportMgr->tlsMgr.hLocalTlsAddrHash = NULL;
    }

#endif /* defined(UPDATED_BY_SPIRENT) */

    if (NULL != pTransportMgr->pSelect)
    {
        /*if the select is using the stack log - remove it*/
        RvSelectRemoveLogMgr(pTransportMgr->pSelect,pTransportMgr->pLogMgr);
        RvSelectDestruct(pTransportMgr->pSelect,pTransportMgr->maxTimers);
        pTransportMgr->pSelect = NULL;
    }


    if(pTransportMgr->hraSendBuffer != NULL)
    {
        RA_Destruct(pTransportMgr->hraSendBuffer);
        pTransportMgr->hraSendBuffer= NULL;
    }
    if(pTransportMgr->hraRecvBuffer != NULL)
    {
        RA_Destruct(pTransportMgr->hraRecvBuffer);
        pTransportMgr->hraRecvBuffer= NULL;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(NULL != pTransportMgr->tlsMgr.hraEngines)
    {
        TransportTlsShutdown(pTransportMgr);
        RA_Destruct(pTransportMgr->tlsMgr.hraEngines);
        pTransportMgr->tlsMgr.hraEngines=NULL;
    }
    if(NULL != pTransportMgr->tlsMgr.hraSessions)
    {
        RA_Destruct(pTransportMgr->tlsMgr.hraSessions);
        pTransportMgr->tlsMgr.hraSessions= NULL;
    }

#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    /*free outbound host name*/
    if(pTransportMgr->outboundAddr.strHostName != NULL)
    {
        RvMemoryFree(pTransportMgr->outboundAddr.strHostName, pTransportMgr->pLogMgr);
        pTransportMgr->outboundAddr.strHostName= NULL;
    }
    /*free outbound ip*/
    if(pTransportMgr->outboundAddr.strIp != NULL)
    {
        RvMemoryFree(pTransportMgr->outboundAddr.strIp, pTransportMgr->pLogMgr);
        pTransportMgr->outboundAddr.strIp= NULL;
    }
    if (NULL != pTransportMgr->oorListPool)
    {
        RLIST_PoolListDestruct(pTransportMgr->oorListPool);
        pTransportMgr->oorListPool = NULL;
    }
    RvMutexDestruct(&pTransportMgr->udpBufferLock,pTransportMgr->pLogMgr);
    RvMutexDestruct(&pTransportMgr->hObjEventListLock,pTransportMgr->pLogMgr);
    RvMutexDestruct(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);
    RvMutexDestruct(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
    if (RV_FALSE != pTransportMgr->bDLAEnabled)
    {
        RvMutexDestruct(&pTransportMgr->lockLocalAddresses,pTransportMgr->pLogMgr);
    }

    RvMemoryFree(pTransportMgr, pTransportMgr->pLogMgr);

    return rv;
}


/************************************************************************************
 * TransportMgrAllocateRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Allocates receive buffer for Message Received event
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 *            forRead            - flag that notifies that allocation is
 *                              for reading from socket
 * Output:  buff            - allocated buffer
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrAllocateRcvBuffer(IN  TransportMgr    *pTransportMgr,
                                                   IN  RvBool           forRead,
                                                   OUT RvChar          **buff)
{
  RvStatus rv = RV_OK;
  RvInt32  numOfFreeElements;

  RvMutexLock(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);
  rv = RA_Alloc(pTransportMgr->hMessageMemoryBuffPool,(RA_Element *)buff);
  if (rv != RV_OK)
  {
      rv = RV_ERROR_OUTOFRESOURCES;
      RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
          "TransportMgrAllocateRcvBuffer - out of resources when allocating read memory buffer"));
  }

  if (RV_OK == rv)
  {
      numOfFreeElements = RA_NumOfFreeElements(pTransportMgr->hMessageMemoryBuffPool);
      if ((forRead == RV_TRUE) && (numOfFreeElements <= 0))
      {
          RA_DeAlloc(pTransportMgr->hMessageMemoryBuffPool,(RA_Element)*buff);
          rv = RV_ERROR_OUTOFRESOURCES;
          RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportMgrAllocateRcvBuffer - out of resources (for read) when allocating read memory buffer"));
      }

  }
  if (RV_OK == rv)
  {
      RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
          "TransportMgrAllocateRcvBuffer - Successfully allocated buffer 0x%p", *buff));
  }
  else
  {
      pTransportMgr->noEnoughBuffersInPool = RV_TRUE;
      *buff = NULL;
  }
  RvMutexUnlock(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);

  return rv;
}

/************************************************************************************
 * TransportMgrFreeRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Frees allocated receive buffer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 *            buff            - allocated buffer
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrFreeRcvBuffer(IN  TransportMgr    *pTransportMgr,
                                               IN  RvChar            *buff)
{
  RvStatus rv = RV_OK;

  RvMutexLock(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);
  RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrFreeRcvBuffer - about to free buffer 0x%p",buff));

  rv = RA_DeAlloc(pTransportMgr->hMessageMemoryBuffPool,(RA_Element)buff);

  if ((rv == RV_OK) && (pTransportMgr->noEnoughBuffersInPool))
  {
      pTransportMgr->noEnoughBuffersInPool = RV_FALSE;
      RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "TransportMgrFreeRcvBuffer - notifying resource availability of receive memory buffer"));
      RvSelectStopWaiting(pTransportMgr->pSelect,
						  pTransportMgr->preemptionReadBuffersReady,
                          pTransportMgr->pLogMgr);
  }
  RvMutexUnlock(&pTransportMgr->hRcvBufferAllocLock,pTransportMgr->pLogMgr);

  return rv;
}

/************************************************************************************
 * TransportMgrConvertString2Address
 * ----------------------------------------------------------------------------------
 * General: make RV_LI_AddressType from string
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 *          bIsIpV4   - Defines if the address is ip4 or ip6
 *          bConstructAddr - the received address needs to be constructed.
 * Output:  address - the address in RV_LI_AddressType format
 ***********************************************************************************/
RvStatus TransportMgrConvertString2Address(IN  RvChar*                StringIp,
                                           OUT SipTransportAddress*    address,
                                           IN  RvBool                 bIsIpV4,
                                           IN  RvBool                 bConstructAddr)
{
    RvStatus rv = RV_OK;
    if (RV_TRUE == bIsIpV4)
    {
        if(bConstructAddr == RV_TRUE)
        {
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&address->addr);
        }
        if (NULL == RvAddressSetString(StringIp,&address->addr))
        {
            rv = RV_ERROR_UNKNOWN;
        }
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    else if (bIsIpV4 == RV_FALSE)
    {
        RvChar   tempChar          = '\0';
        RvChar*  pScopeIdLocation  = NULL;
        RvChar   tmpstrBuff[RVSIP_TRANSPORT_LEN_STRING_IP];

        if(bConstructAddr == RV_TRUE)
        {
           RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&address->addr);
        }

/* SPIRENT_BEGIN */        
#if defined(UPDATED_BY_SPIRENT)
        /* sanity check */
        if(strlen(StringIp) > RVSIP_TRANSPORT_LEN_STRING_IP)
        {
           /* Oop! the given 'StringIp' is longer than we expect for a string of digits representing a Ip addresss.
              So, we don't bother to attempt to convert it. To properly convert this string, users would need to resolve it
              through a DNS request.
            */
           return RV_ERROR_BADPARAM;  /* return immediately in this case to avoid corrupting memories by the following copy */
        }
#endif
/* SPIRENT_END */

        
        strncpy(tmpstrBuff,StringIp,RVSIP_TRANSPORT_LEN_STRING_IP-1);
        tmpstrBuff[RVSIP_TRANSPORT_LEN_STRING_IP-1]='\0';

        /* checking for scope id in the address */
        pScopeIdLocation = strchr(tmpstrBuff,'%');
        if (NULL != pScopeIdLocation)
        {
            RvAddressSetIpv6Scope(&address->addr,atoi(pScopeIdLocation+1));
            *pScopeIdLocation = '\0';
        }
        else
        {
            RvAddressSetIpv6Scope(&address->addr,0);
        }
        if (tmpstrBuff[0] == '[')
        {
            /* avoiding the square parenthesis */
            tempChar = tmpstrBuff[strlen(tmpstrBuff) - 1];
            tmpstrBuff[strlen(tmpstrBuff) - 1] = '\0';
            if (NULL == RvAddressSetString(tmpstrBuff + 1,&address->addr))
                rv = RV_ERROR_BADPARAM;
            tmpstrBuff[strlen(tmpstrBuff)] = tempChar;
        }
        else
        {
            if (NULL == RvAddressSetString(tmpstrBuff,&address->addr))
                rv = RV_ERROR_BADPARAM;
        }
        if (NULL != pScopeIdLocation)
        {
            *pScopeIdLocation = '%';
        }
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    else
    {
        rv = RV_ERROR_BADPARAM;
    }
    return rv;
}

/******************************************************************************
 * TransportMgrConvertTransportAddr2RvAddress
 * ----------------------------------------------------------------------------
 * General: construct the RvAddress object and initialize it with the address
 *          details, contained in the RvSipTransportAddr structure.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager object, for log prints.
 *                            might be NULL.
 *          pAddressDetails - details of the address to be constructed.
 * Output:  pRvAddress      - address to be constructed.
 *****************************************************************************/
RvStatus TransportMgrConvertTransportAddr2RvAddress(
                                    IN  TransportMgr*       pTransportMgr,
                                    IN  RvSipTransportAddr  *pAddressDetails,
                                    OUT RvAddress           *pRvAddress)
{
    RvChar* strIp;
#if (RV_NET_TYPE & RV_NET_IPV6)
    RvChar  strCcIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvInt32 scopeId = UNDEFINED;
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    
    /* Construct RvAddress */
    memset(pRvAddress,0,sizeof(RvAddress));
    pRvAddress = RvAddressConstruct(
        SipCommonConvertSipToCoreAddrType(pAddressDetails->eAddrType),
        pRvAddress);
    if (NULL == pRvAddress)
    {
        if(pTransportMgr != NULL)
        {
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrConvertTransportAddr2RvAddress - failed to construct RvAddress"));
        }
        
        return RV_ERROR_UNKNOWN;
    }

    /* Fill the RvAddress with the IP */
    strIp = pAddressDetails->strIP;
    /* If IP string contains square brackets or scope id, remove them */
#if (RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAddressDetails->eAddrType)
    {
        ConvertIpStringTransport2CommonCore(pAddressDetails->strIP, strCcIp, &scopeId);
        strIp = strCcIp;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    pRvAddress = RvAddressSetString(strIp, pRvAddress);
    if (NULL == pRvAddress)
    {
        if(pTransportMgr != NULL)
        {
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrConvertTransportAddr2RvAddress - failed to set IP(%s)",
                strIp));
        }
        return RV_ERROR_UNKNOWN;
    }

    /* Fill the RvAddress with the port */
    pRvAddress = RvAddressSetIpPort(pRvAddress, pAddressDetails->port);
    if (NULL == pRvAddress)
    {
        if(pTransportMgr != NULL)
        {
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrConvertTransportAddr2RvAddress - failed to set Port(%d)",
                pAddressDetails->port));
        }
        return RV_ERROR_UNKNOWN;
    }
    /* Set IPv6 scope, if need */
#if (RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAddressDetails->eAddrType)
    {
        /* "0" is legal value for Scope Id (RFC2851). Use UNDEFINED instead. */
        /* Scope Id, provided with "Ipv6Scope" field has precedence over scope
        id, found in "strIP" field */
        if (pAddressDetails->Ipv6Scope != UNDEFINED)
        {
            scopeId = pAddressDetails->Ipv6Scope;
        }
        if (scopeId != UNDEFINED)
        {
            pRvAddress = RvAddressSetIpv6Scope(pRvAddress, (RvUint32)scopeId);
            if (NULL == pRvAddress)
            {
                if(pTransportMgr != NULL)
                {
                    RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "TransportMgrConvertTransportAddr2RvAddress - failed to set IPv6 scope(%d)",
                        scopeId));
                }
                return RV_ERROR_UNKNOWN;
            }
        }
    }
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    return RV_OK;
}

/******************************************************************************
 * TransportMgrConvertRvAddress2TransportAddr
 * ----------------------------------------------------------------------------
 * General: extracts data from RvAddress object and
 *          put it into the RvSipTransportAddr structure.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager object.
 *          pRvAddress      - common core address.
 * Output:  pAddressDetails - details of the address.
 *****************************************************************************/
void TransportMgrConvertRvAddress2TransportAddr(
                                      IN  RvAddress          *pRvAddress,
                                      OUT RvSipTransportAddr *pAddressDetails)
{
    pAddressDetails->port = RvAddressGetIpPort(pRvAddress);
    pAddressDetails->eAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(pRvAddress));
    RvAddressGetString(pRvAddress, RVSIP_TRANSPORT_LEN_STRING_IP, pAddressDetails->strIP);
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAddressDetails->eAddrType)
    {
        pAddressDetails->Ipv6Scope = RvAddressGetIpv6Scope(pRvAddress);
    }
#endif
}

/************************************************************************************
 * TransportMgrGetDefaultLocalAddress
 * ----------------------------------------------------------------------------------
 * General: returns the handle to a local address according to address
 *          type and transport.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to transport manager
 *          eTransportType      - Transport type (udp/tcp)
 *          eAddressType        - address type (Ipv6/Ipv4)
 * Output:phLocalAddress - The local address handle
 ***********************************************************************************/
void RVCALLCONV TransportMgrGetDefaultLocalAddress(
                            IN  TransportMgr                *pTransportMgr,
                            IN  RvSipTransport              eTransportType,
                            IN  RvSipTransportAddressType   eAddressType,
                            OUT RvSipTransportLocalAddrHandle *phLocalAddress)
{
    RvSipTransportLocalAddrHandle localAddr     = NULL;
    RLIST_ITEM_HANDLE           hListElem;
    RLIST_HANDLE                hlocalAddrList = NULL;
    RvInt32  safeCounter;

    *phLocalAddress = NULL;
    safeCounter = pTransportMgr->maxNumOfLocalAddresses + 1;
    /* safeCounter should be greater than maxNumOfLocalAddresses
    in order to prevent "infinite loop detected" error print,
    when there are exact maxNumOfLocalAddresses addresses in stack,
    but they doesn't match Transport/Address Type pattern. */

    if(eTransportType == RVSIP_TRANSPORT_UDP)
    {
        hlocalAddrList = pTransportMgr->hLocalUdpAddrList;
    }
    else if(eTransportType == RVSIP_TRANSPORT_TCP &&
            pTransportMgr->tcpEnabled  == RV_TRUE &&
            pTransportMgr->hLocalTcpAddrList != NULL)
    {
        hlocalAddrList = pTransportMgr->hLocalTcpAddrList;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else if (eTransportType == RVSIP_TRANSPORT_TLS &&
             NULL != pTransportMgr->tlsMgr.hLocalTlsAddrList)
    {
        hlocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
#if (RV_NET_TYPE & RV_NET_SCTP)
    else if (eTransportType == RVSIP_TRANSPORT_SCTP &&
             NULL != pTransportMgr->sctpData.hSctpLocalAddrList)
    {
        hlocalAddrList = pTransportMgr->sctpData.hSctpLocalAddrList;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
    else
    {
        return;
    }

    TransportMgrLocalAddrListLock(pTransportMgr);

    RLIST_GetHead(pTransportMgr->hLocalAddrPool,
                  hlocalAddrList,
                  &hListElem);

    while (NULL != hListElem && safeCounter > 0)
    {
        RvStatus rv;
        RvSipTransportAddressType eAddressTypeListElem;
        localAddr = (RvSipTransportLocalAddrHandle)hListElem;

        rv = SipTransportLocalAddressGetAddrType(localAddr,&eAddressTypeListElem);
        if(RV_OK != rv  ||  RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == eAddressTypeListElem)
        {
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrGetDefaultLocalAddress - Failed to get address type for local address 0x%p",
                localAddr));
            return;
        }
        if(eAddressTypeListElem == eAddressType)
        {
            *phLocalAddress = localAddr;
            break;
        }
        RLIST_GetNext(pTransportMgr->hLocalAddrPool,
                      hlocalAddrList,
                      hListElem, &hListElem);
                      safeCounter--;

    }

    TransportMgrLocalAddrListUnlock(pTransportMgr);
    if(safeCounter == 0)
    {
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrGetDefaultLocalAddress - infinte loop detected, safe counter reached %d",
             safeCounter));
    }
    return;
}

/************************************************************************************
 * TransportMgrFindLocalAddressHandle
 * ----------------------------------------------------------------------------------
 * General: returns the handle of a local address from the local address list
 *          that is equal to the supplied local address.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to transport manager.
 *          pLocalAddr      - local address structure pointer.
 * Output:  phLocalAddress  - Handle to an equal local address.
 ***********************************************************************************/
void RVCALLCONV TransportMgrFindLocalAddressHandle(
                            IN  TransportMgr                  *pTransportMgr,
                            IN  TransportMgrLocalAddr         *pLocalAddr,
                            OUT RvSipTransportLocalAddrHandle *phLocalAddress)
{

    RLIST_ITEM_HANDLE          hListElem;
    RLIST_HANDLE               hLocalAddrList = NULL;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE                hLocalAddrHash = 0;
#endif /*defined(UPDATED_BY_SPIRENT) */
    TransportMgrLocalAddr     *pLocalAddrFromList;
    RvInt32  safeCounter = 1000;

    *phLocalAddress = NULL;

    switch (pLocalAddr->addr.eTransport)
    {
    case RVSIP_TRANSPORT_UDP:
        hLocalAddrList = pTransportMgr->hLocalUdpAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->hLocalUdpAddrHash;
#endif /*defined(UPDATED_BY_SPIRENT) */
        safeCounter = pTransportMgr->numOfUdpAddresses*2;
        break;
    case RVSIP_TRANSPORT_TCP:
        hLocalAddrList = pTransportMgr->hLocalTcpAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->hLocalTcpAddrHash;
#endif /*defined(UPDATED_BY_SPIRENT) */
        safeCounter = pTransportMgr->numOfTcpAddresses*2;
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        hLocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->tlsMgr.hLocalTlsAddrHash;
#endif /*defined(UPDATED_BY_SPIRENT) */
        safeCounter = pTransportMgr->tlsMgr.numOfTlsAddresses*2;
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        TransportSctpMgrFindLocalAddressHandle(pTransportMgr,pLocalAddr,phLocalAddress);
        return;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        break;
    } /*switch (pLocalAddr->address.transport)*/

    if(hLocalAddrList == NULL || safeCounter == 0)
    {
        return;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(safeCounter == 0)
    {
        /* this means there is no address which has the same type as the one we're requested to look for. */
        /* for instance, we often see that safeCounter=0 if pTransportMgr->numOfTlsAddresses=0.           */
        /* in this case, we don't have to continue the search and abort right here to 
           prevent raising the annoying LOG EXCEPTION at the end of this function which misleads users */
        /*
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportMgrFindLocalAddressHandle - no address of type=%d", pLocalAddr->addr.eTransport));
         */
        return;
    }
#endif /*defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

    TransportMgrLocalAddrListLock(pTransportMgr);

    RLIST_GetHead(pTransportMgr->hLocalAddrPool,
                  hLocalAddrList,
                  &hListElem);

#if defined(UPDATED_BY_SPIRENT)
    if( hLocalAddrHash == 0 )
    {
#endif /* defined(UPDATED_BY_SPIRENT) */
    while (NULL != hListElem && safeCounter > 0)
    {
        pLocalAddrFromList = (TransportMgrLocalAddr *)hListElem;
        if (RV_OK != TransportMgrLocalAddrLock(pLocalAddrFromList))
        {
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            return;
        }
#if defined(UPDATED_BY_SPIRENT)
	if(strcmp(pLocalAddrFromList->if_name,pLocalAddr->if_name)==0)
#endif
        if(TransportMgrSipAddressIsEqual(&(pLocalAddrFromList->addr),&(pLocalAddr->addr))== RV_TRUE)
        {
            *phLocalAddress = (RvSipTransportLocalAddrHandle)pLocalAddrFromList;
            TransportMgrLocalAddrUnlock(pLocalAddrFromList);
            break;
        }

        RLIST_GetNext(pTransportMgr->hLocalAddrPool, hLocalAddrList,
                      hListElem, &hListElem);
        TransportMgrLocalAddrUnlock(pLocalAddrFromList);

        safeCounter--;
   }
#if defined(UPDATED_BY_SPIRENT)
   }
   else
   {
        void    *  pElem = NULL;
        RvStatus   rv;
        TransportMgrLocalAddr * pLA;


        rv = HASH_FindElement( hLocalAddrHash, pLocalAddr, HashCompare_LocalAddr, &pElem );
        if( rv == RV_TRUE && pElem != NULL )
        {
            HASH_GetUserElement( hLocalAddrHash, pElem, (void *)(&pLA) );
            *phLocalAddress = (RvSipTransportLocalAddrHandle)(pLA);
        }
   }
#endif /* defined(UPDATED_BY_SPIRENT) */
   TransportMgrLocalAddrListUnlock(pTransportMgr);
   if(safeCounter == 0)
   {
	   /* We might reach here in a valid state, when a call-leg/subscription */
	   /* is being restored in different host than the host it was stored in */
       RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
		"TransportMgrFindLocalAddressHandle - infinte loop detected, safe counter reached %d", safeCounter));
   }
}
/************************************************************************************
 * TransportMgrInitLocalAddressStructure
 * ----------------------------------------------------------------------------------
 * General: Initialize a local address structure based on the given parameters.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr           - pointer to transport manager
 *          eCoreTransportType      - Transport type (udp/tcp)
 *          eCoreAddressType        - address type (Ipv6/Ipv4)
 *          strLocalIp              - local IP as a string
 *          localPort               - local port
 *          if_name                 - local interface name
 *          vdevblock                  - local hardware port
 * Output:phLocalAddress - The local address handle
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrInitLocalAddressStructure(
                            IN TransportMgr*              pTransportMgr,
                            IN  RvSipTransport            eTransport,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar*                   strLocalIp,
                            IN  RvUint16                  localPort,
#if defined(UPDATED_BY_SPIRENT)
			    IN  RvChar*                   if_name,
			    IN  RvUint16                  vdevblock,
#endif
                            OUT TransportMgrLocalAddr*    pLocalAddr)
{
    memset(pLocalAddr,0 ,sizeof(TransportMgrLocalAddr));

    if (strLocalIp==NULL || strLocalIp[0]=='\0')
    {
        return RV_ERROR_BADPARAM;
    }

    pLocalAddr->pSocket         = NULL;
    pLocalAddr->addr.eTransport = eTransport;
    strcpy(pLocalAddr->strLocalAddress, strLocalIp);
#if defined(UPDATED_BY_SPIRENT)
    if(if_name) strncpy(pLocalAddr->if_name,if_name,sizeof(pLocalAddr->if_name)-1);
    pLocalAddr->vdevblock = vdevblock;
#endif

    /* For SCTP addresses type of address stored in eSockAddrType can
    differ from the type of pLocalAddr->addr.addr address.
    SCTP Local Address can be of IPv6 type, since it has IPv6 multihomong
    addresses, and simultaneously can keep IPv4 address in
    pLocalAddr->addr.addr.
       Therefore store asked Address Type in the eSockAddrType field,
    and set the Address Type of pLocalAddr->addr.addr to match IP, that will
    be stored in pLocalAddr->addr.addr */
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_SCTP == eTransport)
    {
        pLocalAddr->eSockAddrType = eAddressType;
        eAddressType = (NULL == strchr(strLocalIp, ':'))?
            RVSIP_TRANSPORT_ADDRESS_TYPE_IP :
            RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }
#endif /*#if(RV_NET_TYPE & RV_NET_IPV6)*/


    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eAddressType)
    {
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&pLocalAddr->addr.addr);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    else
    {
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&pLocalAddr->addr.addr);
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* Set the given port. Set default port if the given port is 0 */
    if (localPort == 0)
    {
        localPort = RVSIP_TRANSPORT_DEFAULT_PORT;
    }

    RvAddressSetIpPort(&pLocalAddr->addr.addr,localPort);


    if(RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eAddressType)
    {
        if (strcmp(strLocalIp,IPV4_LOCAL_LOOP) == 0 ||
            strcmp(strLocalIp,IPV4_LOCAL_ADDRESS) == 0   ||
            strcmp(strLocalIp,"0") == 0)
        {
            RvAddressSetString(IPV4_LOCAL_ADDRESS,&pLocalAddr->addr.addr);
        }
        else
        {
            if (NULL ==  RvAddressSetString(strLocalIp,&pLocalAddr->addr.addr))
            {
                return RV_ERROR_BADPARAM;
            }
        }
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    else if(RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == eAddressType)
    {
        RvStatus rv = RV_OK;

        if ((memcmp(strLocalIp,IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) == 0) )
        {
            /*we call this function to init the scope id*/
            rv = TransportMgrConvertString2Address(strLocalIp, &(pLocalAddr->addr), RV_FALSE,RV_FALSE);
            if (RV_OK != rv)
            {
                return rv;
            }
            /*zero IP V6*/
            memset(pLocalAddr->addr.addr.data.ipv6.ip, 0,RVSIP_TRANSPORT_LEN_BYTES_IPV6);
        }
        else
        {
            /*other IP V6*/
            rv = TransportMgrConvertString2Address(strLocalIp, &(pLocalAddr->addr), RV_FALSE,RV_FALSE);
            if (RV_OK != rv)
            {
                return rv;
            }
        }
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    else
    {
        return RV_ERROR_BADPARAM;
    }
    RV_UNUSED_ARG(pTransportMgr);

    return RV_OK;
}

/************************************************************************************
 * TransportMgrLocalAddressGetIPandPortStrings
 * ----------------------------------------------------------------------------------
 * General: returns the IP and port in string format for a given address.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - Pointer to transport manager
 *          pAddress        - Common Core address, stored in the Local Address
 *          bConvertZeroToDefaultHost - Specifies whether to convert a zero IP
 *                                      to the default host.
 *          bAddScopeIp     - Specifies whether to add the scope id to the end
 *                            of the string ip.
 * Output   localIp         - string IP of this local address
 *          localPort       - port of this local address
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetIPandPortStrings(
                            IN  TransportMgr*           pTransportMgr,
                            IN  RvAddress*              pAddress,
                            IN  RvBool                  bConvertZeroToDefaultHost,
                            IN  RvBool                  bAddScopeIp,
                            OUT RvChar                 *localIp,
                            OUT RvUint16               *localPort)
{
    RvStatus   rv;
    RvBool     bZeroIP;

    /* Load original values */
    if (NULL != localPort)
        *localPort = RvAddressGetIpPort(pAddress);
    /* Determine, if the local address contains zero IP */
    bZeroIP = RV_FALSE;
    if (RvAddressIsNull(pAddress))
    {
        bZeroIP = RV_TRUE;
    }

    if (RV_ADDRESS_TYPE_IPV4==RvAddressGetType(pAddress))
    {
        /* Overload "0.0.0.0" with default IP, returned by OS call */
        if(RV_TRUE == bZeroIP  &&  RV_TRUE == bConvertZeroToDefaultHost)
        {
            RvAddress ccDefaultAddr; /* Will store Common Core default address */
            rv = TransportMgrGetDefaultHost(pTransportMgr, RvAddressGetType(pAddress), &ccDefaultAddr);
            if (RV_OK != rv)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportMgrLocalAddressGetIPandPortStrings - Failed to get default host"));
                return rv;
            }
            RvAddressGetString(&ccDefaultAddr,RVSIP_TRANSPORT_LEN_STRING_IP,localIp);
        }
        else
        {
            RvAddressGetString(pAddress,RVSIP_TRANSPORT_LEN_STRING_IP,localIp);
        }
    }

    /* Format IPv6 address */
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6==RvAddressGetType(pAddress))
    {
        RvChar tmpbuff[RVSIP_TRANSPORT_LEN_STRING_IP];

        if(RV_TRUE == bZeroIP  &&  RV_TRUE == bConvertZeroToDefaultHost)
        {
            RvAddress ccDefaultAddr; /* Will store Common Core default address */
            rv = TransportMgrGetDefaultHost(pTransportMgr, RvAddressGetType(pAddress), &ccDefaultAddr);
            if (RV_OK != rv)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportMgrLocalAddressGetIPandPortStrings - Failed to get default host"));
                return rv;
            }

            RvSprintf(localIp,"[%s]", RvAddressGetString(&ccDefaultAddr,RVSIP_TRANSPORT_LEN_STRING_IP,tmpbuff));

        }
        else
        {
            RvSprintf(localIp, "[%s]", RvAddressGetString(pAddress,RVSIP_TRANSPORT_LEN_STRING_IP,tmpbuff));
        }
        /* Add Scope Id */
        if(RV_TRUE == bAddScopeIp)
        {
            RvSprintf(localIp, "%s%s%d",localIp,"%",RvAddressGetIpv6Scope(pAddress));
        }
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    RV_UNUSED_ARG(bAddScopeIp);

    return RV_OK;
}


/************************************************************************************
 * TransportMgrConvertCoreAddrTypeToSipAddrType
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
RvSipTransportAddressType TransportMgrConvertCoreAddrTypeToSipAddrType(
                        IN  RvInt       eCoreAddressType)

{
    switch(eCoreAddressType)
    {
    case RV_ADDRESS_TYPE_IPV4:
        return RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    case RV_ADDRESS_TYPE_IPV6:
        return RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    default:
        return RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
    }
}

/******************************************************************************
 * TransportMgrSipAddrGetDetails
 * ----------------------------------------------------------------------------
 * General: auxiliary function, that fills RvSipTransportAddr structure with
 *          data, extracted from SipTransportAddress structure.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSipAddr - pointer to the SipTransportAddress structure.
 * Output:  pAddrDetails - pointer to the RvSipTransportAddr structure.
 *****************************************************************************/
void TransportMgrSipAddrGetDetails( IN  SipTransportAddress *pSipAddr,
                                    OUT RvSipTransportAddr  *pAddrDetails)
{
    pAddrDetails->eTransportType = pSipAddr->eTransport;
    pAddrDetails->eAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(pSipAddr->addr.addrtype);
    pAddrDetails->port = RvAddressGetIpPort(&pSipAddr->addr);
    RvAddressGetString(&pSipAddr->addr,RVSIP_TRANSPORT_LEN_STRING_IP,pAddrDetails->strIP);
    return;
}

/************************************************************************************
 * TransportMgrSetSocketDefaultAttrb
 * ----------------------------------------------------------------------------------
 * General: Set Default attributes to a TCP socket:
 *          TCP:
 *               Non blocking
 *               NO Linger
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pSocket - Pointer to the relevant socket.
 *          pMgr     - transport mgr
 *          eProt    - core protocol
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrSetSocketDefaultAttrb(IN RvSocket*       pSocket,
                                                     IN TransportMgr*   pMgr,
                                                     IN RvSocketProtocol eProt)
{
    RvStatus  crv    = RV_OK;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Do not change the read/write socket size. It will be taken from 
 *           rmem_default/wmem_default. SIP Glue takes care about them. 
 */
#if !defined(UPDATED_BY_SPIRENT)
    crv = RvSocketSetBuffers(pSocket,SEND_RCV_BUFFER_SIZE,SEND_RCV_BUFFER_SIZE,pMgr->pLogMgr);
	if (RV_OK != crv)
	{
		return CRV2RV(crv);
	}
#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

    if (RvSocketProtocolTcp == eProt)
    {
        crv = RvSocketSetBlocking(pSocket,RV_FALSE,pMgr->pLogMgr);
        if (RV_OK != crv)
        {
            return CRV2RV(crv);
        }
        crv = RvSocketSetLinger(pSocket,0,pMgr->pLogMgr);
        if (RV_ERROR_NOTSUPPORTED == RvErrorGetCode(crv))
        {
            RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                "TransportMgrSetSocketDefaultAttrb - Sock %d: failed to set linger param",*pSocket));
            crv = RV_OK;
        }
    }
/* SPIRENT_BEGIN */
/*
 * COMMENTS: make it non-blocking for any socket type
 */
#if defined(UPDATED_BY_SPIRENT)
    else 
    {
        crv = RvSocketSetBlocking(pSocket,RV_FALSE,pMgr->pLogMgr);
        if (RV_OK != crv)
        {
            return CRV2RV(crv);
        }
    }
#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

    return CRV2RV(crv);
}

/************************************************************************************
 * TransportMgrProccesEvents
 * ----------------------------------------------------------------------------------
 * General: calls select
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr        - pointer to transport manager
 *          msMaxTimeout         - timeout for select to exit. if 0, will wait indefinitely
 * Output:  -
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrProccesEvents(IN RvSipTransportMgrHandle  hTransportMgr,
                                               IN RvUint32                msMaxTimeout)
{
    RvStatus        crv             = RV_OK;
    TransportMgr*   pTransportMgr   = (TransportMgr*)hTransportMgr;


    while (crv == RV_OK)
    {
        RvUint64 timeout = RvUint64Mul(RvUint64Const(0,RV_TIME_NSECPERMSEC),
                                       RvUint64FromRvUint32(msMaxTimeout));
        crv = RvSelectWaitAndBlock(pTransportMgr->pSelect, timeout);
    }
    return CRV2RV(crv);
}

/***************************************************************************
* TransportMgrSipAddressIsEqual
* ------------------------------------------------------------------------
* General: Compares to LI addresses
* Return Value: TRUE if the addresses are equal, FALSE otherwise.
*               This function does not check the scope id.
* ------------------------------------------------------------------------
* Arguments:
* Input:     firstAddr  - first address
*           secondAddr  - second address
***************************************************************************/
RvBool TransportMgrSipAddressIsEqual(IN SipTransportAddress*  firstAddr,
                                     IN SipTransportAddress*  secondAddr)
{
    if (firstAddr->eTransport != secondAddr->eTransport)
    {
        return RV_FALSE;
    }
    return TransportMgrRvAddressIsEqual(&firstAddr->addr,&secondAddr->addr);
}

/***************************************************************************
* TransportMgrRvAddressIsEqual
* ------------------------------------------------------------------------
* General:      Compares Common Core addresses in context of SIP:
*               their IPs and ports.
* Return Value: TRUE if the addresses are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input: firstAddr   - the first address.
*        secondAddr  - an another address.
***************************************************************************/
RvBool TransportMgrRvAddressIsEqual(IN RvAddress*  firstAddr,
                        IN RvAddress*  secondAddr)
{
    /* Check IPs */
    if (RV_TRUE != RvAddressCompare(firstAddr,secondAddr,RV_ADDRESS_BASEADDRESS))
    {
        return RV_FALSE;
    }
    /* Check Ports */
    switch(firstAddr->addrtype)
    {
    case RV_ADDRESS_TYPE_IPV4: 
        return (firstAddr->data.ipv4.port == secondAddr->data.ipv4.port);
#if (RV_NET_TYPE & RV_NET_IPV6)
    case RV_ADDRESS_TYPE_IPV6: 
        return (firstAddr->data.ipv6.port == secondAddr->data.ipv6.port);
#endif
    default:
        return RV_FALSE;
    }
}

/***************************************************************************
* TransportMgrRvAddressPortAreEqual
* ------------------------------------------------------------------------
* General:      Compares ports of Common Core addresses.
*               The addresses can be of different IPv4/IPv6 types.
* Return Value: TRUE if the ports are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input: firstAddr   - the first address.
*        secondAddr  - an another address.
***************************************************************************/
RvBool TransportMgrRvAddressPortAreEqual(IN RvAddress*  firstAddr,
                                         IN RvAddress*  secondAddr)
{
    RvUint16 firstPort  = RvAddressGetIpPort(firstAddr);
    RvUint16 secondPort = RvAddressGetIpPort(secondAddr);
    return (firstPort==secondPort) ? RV_TRUE : RV_FALSE;
}

/******************************************************************************
 * TransportMgrLocalAddressAdd
 * ----------------------------------------------------------------------------
 * General: add local address the manager, open it for listening and sending
 *
 * Return Value: RvStatus - RV_OK, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 *          eLocationInList - indication, where the new address should be placed
 *                            in the list of local addresses.
 *          hBaseLocalAddr  - existing address in the list, relative to which
 *                            the new addresses can be added.
 *                            The parameter is meaningless, if 'eLocationInList'
 *                            is not set to RVSIP_PREV_ELEMENT or RVSIP_NEXT_ELEMENT
 *          bConvertZeroPort- If RV_TRUE, and pAddressDetails contains zero
 *                            port, the port will be set to well known SIP port
 *                            according the Transport contained in
 *                            pAddressDetails.
 *          bDontOpen       - if RV_TRUE, no socket will be opened,
 *                            for TCP/TLS/SCTP address - no listening
 *                            connection will be constructed.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr              *pAddressDetails,
                        IN  RvUint32                        addrStructSize,
                        IN  RvSipListLocation               eLocationInList,
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        IN  RvBool                          bConvertZeroPort,
                        IN  RvBool                          bDontOpen,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr, *pBaseLocalAddress;
    TransportMgr* pTransportMgr;
    RvSipTransportAddr addrDetails;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if (RV_FALSE == pTransportMgr->bDLAEnabled)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressAdd - DLA is disabled. %s Address %s:%d will not be added",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (RV_FALSE == pTransportMgr->tcpEnabled)
    {
        if (RVSIP_TRANSPORT_TCP  == pAddressDetails->eTransportType ||
            RVSIP_TRANSPORT_TLS  == pAddressDetails->eTransportType ||
            RVSIP_TRANSPORT_SCTP == pAddressDetails->eTransportType)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrLocalAddressAdd - TCP is disabled. %s Address %s:%d will not be added",
                SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
                pAddressDetails->strIP,pAddressDetails->port));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_TRANSPORT_TLS == pAddressDetails->eTransportType &&
        (0>=pTransportMgr->tlsMgr.numOfEngines || 0>=pTransportMgr->tlsMgr.maxSessions))
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressAdd - Number of %s <= 0. %s Address %s:%d will not be added",
            (0>=pTransportMgr->tlsMgr.numOfEngines)?"TLS Engines":"TLS sessions",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port));
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

    memset(&addrDetails,0,sizeof(RvSipTransportAddr));
    memcpy(&addrDetails,pAddressDetails,RvMin(sizeof(RvSipTransportAddr),addrStructSize));
    pBaseLocalAddress   = (TransportMgrLocalAddr*)hBaseLocalAddr;
    pTransportMgr       = (TransportMgr*)hTransportMgr;
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    rv = TransportMgrAddLocalAddressToList( pTransportMgr, &addrDetails,
               eLocationInList, pBaseLocalAddress, bConvertZeroPort,
               &pLocalAddr, NULL/*pPort*/, pTransportMgr->localPriority);
#else
    rv = TransportMgrAddLocalAddressToList( pTransportMgr, &addrDetails,
               eLocationInList, pBaseLocalAddress, bConvertZeroPort,
               &pLocalAddr, NULL/*pPort*/);
#endif
/* SPIRENT_END */

    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressAdd - Failed to add %s Address %s:%d to list of local addresses (rv=%d)",
            SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
            pAddressDetails->strIP,pAddressDetails->port,rv));
        return rv;
    }

    /* Open socket for listening, if it should be opened */
    if (RV_FALSE == bDontOpen)
    {
        rv = LocalAddressOpen(pTransportMgr, pLocalAddr, &pAddressDetails->port);
        if (RV_OK != rv)
        {
            /* Remove not opened address from the List of Local Addresses */
            if (RV_OK != TransportMgrLocalAddressRemove(hTransportMgr, (RvSipTransportLocalAddrHandle)pLocalAddr))
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportMgrLocalAddressAdd - TransportMgrLocalAddressRemove(Address 0x%p) failed (rv=%d:%s)",
                    pLocalAddr, rv, SipCommonStatus2Str(rv)));
            }
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrLocalAddressAdd - Failed to open %s Address %s:%d (rv=%d)",
                SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
                pAddressDetails->strIP,pAddressDetails->port,rv));
            return rv;
        }
    }
#if defined(UPDATED_BY_SPIRENT)
  {
    HASH_HANDLE hLocalAddrHash = NULL;
    /* set hLocalAddrHash */
    switch (pAddressDetails->eTransportType)
    {
        case RVSIP_TRANSPORT_UDP:
            hLocalAddrHash = pTransportMgr->hLocalUdpAddrHash;

        case RVSIP_TRANSPORT_TCP:
            hLocalAddrHash = pTransportMgr->hLocalTcpAddrHash;
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            hLocalAddrHash = pTransportMgr->tlsMgr.hLocalTlsAddrHash;
            break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        default:
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrAddLocalAddressToList - Unsupported type of transport - %d (rv=%d)",
                pAddressDetails->eTransportType,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
    }

    /* Insert Local Address element into hash table */
    if( hLocalAddrHash != NULL )
    {
        RvStatus rv;
        void  *  pElem = NULL;
        RvBool   bInserted;

        rv = HASH_InsertElement(
                hLocalAddrHash, pLocalAddr, (void *)&pLocalAddr, 
                RV_FALSE, &bInserted, &pElem, HashCompare_LocalAddr);

        if( rv != RV_OK || bInserted == RV_FALSE )
        {
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
  }
#endif /* defined(UPDATED_BY_SPIRENT) */

    if (NULL != phLocalAddr)
    {
        *phLocalAddr   = (RvSipTransportLocalAddrHandle)pLocalAddr;
    }

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddressRemove
 * ----------------------------------------------------------------------------
 * General: remove local address from manager database, close it's socket
 *
 * Return Value: RvStatus - RV_OK, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the Transport Manager
 *          hLocalAddr      - handle to the address to be removed
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressRemove(
                            IN  RvSipTransportMgrHandle         hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr)
{
    RvStatus rv;
    TransportMgrLocalAddr   *pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    TransportMgr            *pTransportMgr;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    if (RV_FALSE == pTransportMgr->bDLAEnabled)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressRemove - DLA is disabled. Address 0x%p will not be removed",
            hLocalAddr));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = LocalAddressClose(pTransportMgr,pLocalAddr);
    if (RV_OK != rv)
    {
        if (RV_ERROR_TRY_AGAIN != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrLocalAddressRemove - Failed to close Local Address 0x%p (rv=%d:%s)",
                hLocalAddr, rv, SipCommonStatus2Str(rv)));
        }
        else
        {
            rv = RV_OK;
        }
        return rv;
    }

    rv = LocalAddressRemoveFromList(pTransportMgr,pLocalAddr);
    if (RV_OK != rv)
    {
        TransportMgrLogLocalAddressError(pTransportMgr, pLocalAddr, rv,
            "TransportMgrLocalAddressRemove - Failed to remove %s Address %s:%d (0x%p) from list of local addresses. rv=%d");
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: returns details of the Local Address, the handle to which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     -  handle of the Local Address.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        OUT RvSipTransportAddr              *pAddressDetails)
{
    RvStatus rv;
    TransportMgrLocalAddr *pLocalAddr;
    TransportMgr          *pTransportMgr;

    pLocalAddr = (TransportMgrLocalAddr*)hLocalAddr;
    pTransportMgr = pLocalAddr->pMgr;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetDetails - Failed to lock Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        return rv;
    }

    rv = TransportMgrLocalAddressGetIPandPortStrings(pTransportMgr,
        &pLocalAddr->addr.addr, RV_FALSE,
        RV_TRUE, pAddressDetails->strIP, &pAddressDetails->port);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetDetails - Failed to get IP and Port strings for Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return rv;
    }
    pAddressDetails->eTransportType = pLocalAddr->addr.eTransport;

    rv = SipTransportLocalAddressGetAddrType(hLocalAddr, &pAddressDetails->eAddrType);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetDetails - Failed to get Address Type of Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return rv;
    }

#if (RV_NET_TYPE & RV_NET_IPV6)
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == pAddressDetails->eAddrType)
    {
        pAddressDetails->Ipv6Scope = 
                            RvAddressGetIpv6Scope(&pLocalAddr->addr.addr);
    }
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
#if defined(UPDATED_BY_SPIRENT)
    strncpy(pAddressDetails->if_name,pLocalAddr->if_name,sizeof(pAddressDetails->if_name)-1);
    pAddressDetails->vdevblock = pLocalAddr->vdevblock;
#endif

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddressGetFirst
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
RvStatus RVCALLCONV TransportMgrLocalAddressGetFirst(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransport                  eTransportType,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RLIST_HANDLE hLocalAddrList;
    TransportMgr *pTransportMgr;

    pTransportMgr = (TransportMgr*)hTransportMgr;
    /* Get handle to List of requested Transport Protocol type */
    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        hLocalAddrList = pTransportMgr->hLocalUdpAddrList;
        break;
    case RVSIP_TRANSPORT_TCP:
        hLocalAddrList = pTransportMgr->hLocalTcpAddrList;
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        hLocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        hLocalAddrList = pTransportMgr->sctpData.hSctpLocalAddrList;
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetFirst - unsupported Transport Type %d",
            eTransportType));
        return RV_ERROR_BADPARAM;
    }

    if (NULL == hLocalAddrList)
    {
        return RV_ERROR_NOT_FOUND;
    }

    TransportMgrLocalAddrListLock(pTransportMgr);

    RLIST_GetHead(pTransportMgr->hLocalAddrPool, hLocalAddrList, (RLIST_ITEM_HANDLE*)phLocalAddr);

    TransportMgrLocalAddrListUnlock(pTransportMgr);

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddressGetNext
 * ------------------------------------------------------------------------
 * General: gets handle of the Local Address, located in the List of Local
 *          addresses next to the address, handle of which is supplied
 *          as a parameter to the function.
 *          type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBaseLocalAddr  - handle to the Local Address, next of which is
 *                            requested.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetNext(
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr)
{
    RLIST_HANDLE hLocalAddrList;
    TransportMgrLocalAddr *pBaseLocalAddr;
    TransportMgr            *pTransportMgr;

    pBaseLocalAddr  = (TransportMgrLocalAddr*)hBaseLocalAddr;
    pTransportMgr   = pBaseLocalAddr->pMgr;

    /* Get handle to List of requested Transport Protocol type */
    switch (pBaseLocalAddr->addr.eTransport)
    {
    case RVSIP_TRANSPORT_UDP:
        hLocalAddrList = pTransportMgr->hLocalUdpAddrList;
        break;
    case RVSIP_TRANSPORT_TCP:
        hLocalAddrList = pTransportMgr->hLocalTcpAddrList;
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        hLocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        hLocalAddrList = pTransportMgr->sctpData.hSctpLocalAddrList;
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetNext - unsupported Transport Type %d",
            pBaseLocalAddr->addr.eTransport));
        return RV_ERROR_BADPARAM;
    }

    TransportMgrLocalAddrListLock(pTransportMgr);

    RLIST_GetNext(pTransportMgr->hLocalAddrPool, hLocalAddrList,
        (RLIST_ITEM_HANDLE)pBaseLocalAddr, (RLIST_ITEM_HANDLE*)phLocalAddr);

    TransportMgrLocalAddrListUnlock(pTransportMgr);

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddressCloseSocket
 * ----------------------------------------------------------------------------
 * General: closes underlying socket of the Local Address.
 * Return Value: RV_OK on success, error code otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressCloseSocket(
                                        IN  TransportMgrLocalAddr *pLocalAddr)
{
    RvStatus rv;
    TransportMgr *pTransportMgr;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        return rv;
    }
    /* Check, if the socket was already closed once */
    if (NULL == pLocalAddr->pSocket)
    {
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return RV_OK;
    }

    pTransportMgr = pLocalAddr->pMgr;

    switch (pLocalAddr->addr.eTransport)
    {
        case RVSIP_TRANSPORT_UDP:
            {
                rv = TransportUDPClose(pTransportMgr, pLocalAddr);
                if (rv != RV_OK)
                {
                    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "TransportMgrLocalAddressCloseSocket: failed to close socket=%d of the address 0x%p",
                        pLocalAddr->ccSocket, pLocalAddr));
                    TransportMgrLocalAddrUnlock(pLocalAddr);
                    return rv;
                }
            }
            break;
        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_TLS:
        case RVSIP_TRANSPORT_SCTP:
            {
                rv = TransportConnectionDisconnect((TransportConnection*)pLocalAddr->hConnListening);
                if (RV_OK != rv)
                {
                    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "TransportMgrLocalAddressCloseSocket: failed to disconnect connection %x socket=%d Local Address=0x%p",
                        pLocalAddr->hConnListening, pLocalAddr->ccSocket, pLocalAddr));
                    TransportMgrLocalAddrUnlock(pLocalAddr);
                    return rv;
                }
                pLocalAddr->ccSocket        = RV_INVALID_SOCKET;
                pLocalAddr->pSocket         = NULL;
                pLocalAddr->hConnListening  = NULL;
            }
            break;
        default:
            TransportMgrLocalAddrUnlock(pLocalAddr);
            return RV_ERROR_NOTSUPPORTED;
    } /*switch (pLocalAddr->address.eTransport)*/

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportMgrLocalAddressCloseSocket: Local Address 0x%p socket is being closed", pLocalAddr));

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}
 /*****************************************************************************
 * TransportMgrLocalAddrLock
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and locks it.
 * Return Value: RV_OK on success, error code if check was failed
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
******************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddrLock(IN  TransportMgrLocalAddr *pLocalAddr)
{
    if (RV_FALSE == pLocalAddr->pMgr->bDLAEnabled)
    {
        return RV_OK;
    }

    RvMutexLock(&pLocalAddr->hLock,pLocalAddr->pMgr->pLogMgr);
    if (RV_TRUE == pLocalAddr->bVacant)
    {
        RvMutexUnlock(&pLocalAddr->hLock,pLocalAddr->pMgr->pLogMgr);
        return RV_ERROR_INVALID_HANDLE;
    }
    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddrUnlock
 * ----------------------------------------------------------------------------
 * General: Unlocks Local Address.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
******************************************************************************/
void RVCALLCONV TransportMgrLocalAddrUnlock(IN  TransportMgrLocalAddr *pLocalAddr)
{
    if (RV_FALSE == pLocalAddr->pMgr->bDLAEnabled)
    {
        return;
    }

    RvMutexUnlock(&pLocalAddr->hLock,pLocalAddr->pMgr->pLogMgr);
}

/******************************************************************************
 * TransportMgrLocalAddrLockRelease
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and release it lock
 *          (removes all nested locks).
 * Return Value: RV_OK on success, error code if check was failed
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLocalAddr        - pointer to the Local Address object.
 * Output:  numOfnestedLocks  - number of nested locks.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddrLockRelease(
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                OUT RvInt32                 *pNumOfNestedLocks)
{
    if (RV_FALSE == pLocalAddr->pMgr->bDLAEnabled)
    {
        return RV_OK;
    }

    RvMutexRelease(&pLocalAddr->hLock,pLocalAddr->pMgr->pLogMgr,pNumOfNestedLocks);
#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
	RV_UNUSED_ARG(pNumOfNestedLocks)
#endif
    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddrLockRestore
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and locks it lock
 *          numOfNestedLock times.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLocalAddr        - pointer to the Local Address object.
 *          numOfnestedLock   - number of nested locks.
 *****************************************************************************/
void RVCALLCONV TransportMgrLocalAddrLockRestore(
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  RvInt32                 numOfNestedLocks)
{
    if (RV_FALSE == pLocalAddr->pMgr->bDLAEnabled)
    {
        return;
    }

    RvMutexRestore(&pLocalAddr->hLock,pLocalAddr->pMgr->pLogMgr,numOfNestedLocks);
#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
	RV_UNUSED_ARG(numOfNestedLocks)
#endif

}

/******************************************************************************
 * TransportMgrCounterLocalAddrsGet
 * ----------------------------------------------------------------------------
 * General: Returns counter of the opened addresses of the required type.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - the Transport Manager.
 *            eTransportType- type of Transport Protocol.
 * Output:    pV4Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *            pV6Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *****************************************************************************/
void RVCALLCONV TransportMgrCounterLocalAddrsGet(
                                    IN  TransportMgr           *pTransportMgr,
                                    IN  RvSipTransport          eTransportType,
                                    OUT RvUint32               *pV4Counter,
                                    OUT RvUint32               *pV6Counter)
{
    if (RVSIP_TRANSPORT_UNDEFINED == eTransportType)
    {
        return;
    }
    TransportMgrLocalAddrListLock(pTransportMgr);
    *pV4Counter = pTransportMgr->localAddrCounters[RVSIP_TRANSPORT_ADDRESS_TYPE_IP][eTransportType];
    *pV6Counter = pTransportMgr->localAddrCounters[RVSIP_TRANSPORT_ADDRESS_TYPE_IP6][eTransportType];
    TransportMgrLocalAddrListUnlock(pTransportMgr);
}

/******************************************************************************
 * TransportMgrLocalAddressSetTypeOfService
 * ------------------------------------------------------------------------
 * General: Set the type of service (called also DiffServ Code Point)
 *          in decimal form for all IP packets, sent out from the local address
 *          (For 02/2004 the first 6 bits of the TOS byte in IP header
 *          can be used for TOS)
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address to be updated.
 *          typeOfService   - number, to be set as a TypeOfService.
 * Output : none.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressSetTypeOfService(
                                IN TransportMgr             *pTransportMgr,
                                IN TransportMgrLocalAddr    *pLocalAddr,
                                IN RvInt32                  typeOfService)
{
    RvStatus rv;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressSetTypeOfService - Failed to lock Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        return rv;
    }
    
    if (NULL == pLocalAddr->pSocket)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressSetTypeOfService - There is no socket under Local Address 0x%p",
            pLocalAddr));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RvSocketSetTypeOfService(&pLocalAddr->ccSocket,typeOfService,pTransportMgr->pLogMgr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressSetTypeOfService - RvSocketSetTypeOfService(addr=0x%p, sock=%d, TOS=%d) failed (rv=%d)",
            pLocalAddr,pLocalAddr->ccSocket,typeOfService,rv));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return CRV2RV(rv);
    }

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV TransportMgrLocalAddressSetSockSize(
							IN TransportMgr             *pTransportMgr,
							IN TransportMgrLocalAddr    *pLocalAddr,
							IN RvUint32                 sockSize) {
  RvStatus rv;
  
  rv = TransportMgrLocalAddrLock(pLocalAddr);
  if (RV_OK != rv) {
    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				       "%s - Failed to lock Local Address 0x%p (rv=%d)",
				       __FUNCTION__,pLocalAddr,rv));
    return rv;
  }
    
  if (NULL == pLocalAddr->pSocket) {
    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				       "%s - There is no socket under Local Address 0x%p",
				       __FUNCTION__,pLocalAddr));
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_ERROR_ILLEGAL_ACTION;
  }

  rv = RvSocketSetBuffers(&pLocalAddr->ccSocket,(RvInt32)sockSize,(RvInt32)sockSize,pTransportMgr->pLogMgr);
  if (RV_OK != rv) {
    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				       "%s(addr=0x%p, sock=%d, size=%d) failed (rv=%d)",
				       __FUNCTION__,pLocalAddr,pLocalAddr->ccSocket,sockSize,rv));
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return CRV2RV(rv);
  }

  TransportMgrLocalAddrUnlock(pLocalAddr);
  
  return RV_OK;
}
#endif
  //SPIRENT_END

/******************************************************************************
 * TransportMgrLocalAddressGetTypeOfService
 * ------------------------------------------------------------------------
 * General: Get type of service (called also DiffServ Code Point), which
 *          was set for the specified Local Address.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address to be updated.
 * Output : pTypeOfService  - pointer to the memory, where the TypeOfService
 *                            will be stored by the function.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetTypeOfService(
                                IN TransportMgr             *pTransportMgr,
                                IN TransportMgrLocalAddr    *pLocalAddr,
                                IN RvInt32                  *pTypeOfService)
{
    RvStatus rv;

    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetTypeOfService - Failed to lock Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        return rv;
    }

    if (NULL == pLocalAddr->pSocket)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetTypeOfService - There is no socket under Local Address 0x%p",
            pLocalAddr));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RvSocketGetTypeOfService(&pLocalAddr->ccSocket,pTransportMgr->pLogMgr,pTypeOfService);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetTypeOfService - RvSocketGetTypeOfService(addr=0x%p, sock=%d) failed (rv=%d)",
            pLocalAddr,pLocalAddr->ccSocket,rv));
        TransportMgrLocalAddrUnlock(pLocalAddr);
        return CRV2RV(rv);
    }

    TransportMgrLocalAddrUnlock(pLocalAddr);

    return RV_OK;
}

#ifdef RV_SIP_JSR32_SUPPORT

/******************************************************************************
 * TransportMgrLocalAddressSetSentBy
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
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address
 *          strSentByHost   - Null terminated sent-by string.
 *          sentByPort - the sent by port, or UNDEFINED if the application does
 *                       not wish to set a port in the sent-by.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressSetSentBy(
										IN  TransportMgr*          pTransportMgr,
										IN  TransportMgrLocalAddr* pLocalAddr,
										IN  RvChar*                strSentByHost,
										IN  RvInt32                sentByPort)

{
    RvStatus rv         = RV_OK;
	RvInt32  sentBySize = strlen(strSentByHost);
    RvInt32  offset     = UNDEFINED;
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressSetSentBy - Failed to lock Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        return rv;
    }

	if(pLocalAddr->hSentByPage != NULL_PAGE)
	{
		RPOOL_FreePage(pTransportMgr->hElementPool, pLocalAddr->hSentByPage);
	    pLocalAddr->hSentByPage = NULL_PAGE;
	}
	rv = RPOOL_GetPage(pTransportMgr->hElementPool,0,&pLocalAddr->hSentByPage);
	if(rv != RV_OK)
	{
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressSetSentBy - Local Address 0x%p failed to get a new page(rv=%d)",
            pLocalAddr,rv));
		TransportMgrLocalAddrUnlock(pLocalAddr);
		return rv;
	}


	rv = RPOOL_AppendFromExternal(pTransportMgr->hElementPool, pLocalAddr->hSentByPage, 
		                          strSentByHost,sentBySize+1, RV_FALSE, &offset,NULL);
    if(RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
			"TransportMgrLocalAddressSetSentBy - Local Address 0x%p failed in RPOOL_AppendFromExternal (rv=%d)",
			pLocalAddr,rv));
		TransportMgrLocalAddrUnlock(pLocalAddr);
        return rv;
    }
    /* Get the offset of the new allocated memory */
    pLocalAddr->sentByHostOffset = offset;
#ifdef SIP_DEBUG
    pLocalAddr->strSentByHost = RPOOL_GetPtr(pTransportMgr->hElementPool, pLocalAddr->hSentByPage, 
									      pLocalAddr->sentByHostOffset);
#endif

	pLocalAddr->sentByPort = sentByPort;
    TransportMgrLocalAddrUnlock(pLocalAddr);
    return RV_OK;
}

#endif /*#ifdef RV_SIP_JSR32_SUPPORT*/

/******************************************************************************
 *TransportMgrLocalAddressGetConnection
 * ------------------------------------------------------------------------
 * General: Gets the handle of the listening Connection object,
 *          bound to the Local Address object of TCP/TLS/SCTP type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr - handle to the local address to be updated
 * Output : phConn     - pointer to the memory, where the handle will be stored
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetConnection(
														  IN  TransportMgr                   *pTransportMgr,
														  IN  TransportMgrLocalAddr          *pLocalAddr,
														  OUT RvSipTransportConnectionHandle *phConn)
{
    RvStatus rv;
    
    rv = TransportMgrLocalAddrLock(pLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrLocalAddressGetConnection - Failed to lock Local Address 0x%p (rv=%d)",
            pLocalAddr,rv));
        return rv;
    }
	
    if (NULL != phConn)
    {
        *phConn = pLocalAddr->hConnListening;
    }
    
    TransportMgrLocalAddrUnlock(pLocalAddr);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}
/************************************************************************************
 * TransportMgrGetDefaultHost
 * ----------------------------------------------------------------------------------
 * General: Get ip address of the default local host as received from the operating
 *          system. Can be used to get one of the local addresses of the system,
 *          when you opened a socket with IP 0. Openning a socket with ip 0 will
 *          cause listenning to all local IP addresses, including the one returned
 *          here.
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to transport instance
 *          eAddressType - indicates the class of address we want to get ipv4/ipv6
 *
 * Output:  pAddr - The default local address
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrGetDefaultHost(
                                        IN    TransportMgr*     pTransportMgr,
                                        IN    RvInt            eAddressType,
                                        INOUT RvAddress*        pAddr)
{

    RvStatus      crv         = RV_OK;
    RvUint32      numofAddresses = 1;
    RvUint        i              = 0;
    RvAddress     addresses[RV_HOST_MAX_ADDRS];

#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == eAddressType)
        numofAddresses = RV_HOST_MAX_ADDRS;
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    crv = RvHostLocalGetAddress(pTransportMgr->pLogMgr,&numofAddresses,addresses);
    if (RV_OK != crv)
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "TransportMgrGetDefaultHost - Failed to get default address"));
        return CRV2RV(crv);
    }

    /* Get the local hosts array from the core layer*/
    if (RV_ADDRESS_TYPE_IPV4 == eAddressType)
    {
        if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&addresses[0]))
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                      "TransportMgrGetDefaultHost - Failed to get default IPv4 address, computer supports only ipv6 addresses"));
            return RV_ERROR_BADPARAM;
        }
        RvAddressCopy(&addresses[i],pAddr);
        return RV_OK;
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == eAddressType)
    {
        static RvUint8 localLinkAddr[16] = {0xfe,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01};
        for (i=0; i<numofAddresses; i++)
        {
            if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&addresses[i]) &&
                0 != memcmp(addresses[i].data.ipv6.ip,localLinkAddr,16) &&
                0 == memcmp(addresses[i].data.ipv6.ip,localLinkAddr,2))
            {
                RvAddressCopy(&addresses[i],pAddr);
                return RV_OK;
            }
        }
        return RV_ERROR_NOT_FOUND;
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    return RV_OK;
}

/************************************************************************************
 * TransportMgrSelectUpdateForAllUdpAddrs
 * ----------------------------------------------------------------------------------
 * General: performs a select update for all UDP listening sockets.
 *          This function is used to go in to OOR state (with event=0) and to get out 
 *          of OOR (with event=read).
 * Return Value: RvStatus - RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr - pointer to transport manager
 *          selectEvent - seelct event to update for all UDP addresses
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrSelectUpdateForAllUdpAddrs(
                                        IN    TransportMgr*   pMgr,
                                        IN    RvSelectEvents  selectEvent)
{
    RvStatus               rv          = RV_OK;
    RLIST_ITEM_HANDLE       hListElem   = NULL;
    TransportMgrLocalAddr   *pLocalAddr =NULL;

    /* locking the UDP addresses */
    TransportMgrLocalAddrListLock(pMgr);
    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportMgrSelectUpdateForAllUdpAddrs - Setting all UDP select callons to 0x%x",selectEvent));

    RLIST_GetHead(pMgr->hLocalAddrPool, pMgr->hLocalUdpAddrList, &hListElem);
    while (NULL != hListElem)
    {
        pLocalAddr = (TransportMgrLocalAddr *)hListElem;
        RLIST_GetNext(pMgr->hLocalAddrPool, pMgr->hLocalUdpAddrList,
                      hListElem, &hListElem);
        rv = TransportMgrLocalAddrLock(pLocalAddr);
        if (RV_OK != rv)
        {
            continue;
        }

        /* If Address was closed meanwhile, skip it */
        if (NULL == pLocalAddr->pSocket)
        {
            TransportMgrLocalAddrUnlock(pLocalAddr);
            continue;
        }

        rv = RvSelectUpdate(pMgr->pSelect,&pLocalAddr->ccSelectFd,selectEvent,TransportUdpEventCallback);
        if (RV_OK != rv)
        {
            TransportMgrLocalAddrUnlock(pLocalAddr);
            RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                "TransportMgrSelectUpdateForAllUdpAddrs - failed to update socket %d",pLocalAddr->ccSocket));
        }
        TransportMgrLocalAddrUnlock(pLocalAddr);
    }
    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportMgrSelectUpdateForAllUdpAddrs - All UDP select callons were set to 0x%x",selectEvent));
    
    TransportMgrLocalAddrListUnlock(pMgr);
    return RV_OK;
}

/******************************************************************************
 * TransportMgrAddLocalAddressToList
 * ----------------------------------------------------------------------------
 * General:      Insert the new Local Address into the List of Local Addresses 
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager.
 *          pAddressDetails - pointer to the memory, where the details of the
 *                            address to be add are stored.
 *          eLocationInList - location of the new address in the list.
 *          pBaseLocalAddr  - pointer to the local address, relatively to which
 *                            the new address should be inserted
 *          bConvertZeroPort- If RV_TRUE, and pAddressDetails contains zero
 *                            port, the port will be set to well known SIP port
 *                            according the Transport contained in
 *                            pAddressDetails.
 * Output:  ppLocalAddr     - pointer to the memory, where the pointer to
 *                            the local address (handle) will be returned.
 *          pPort           - pointer to the memory, where the port, actually
 *                            used while local address opening,will be returned
 *                            The actually set port may differ from the one,
 *                            supplied with pAddressDetails structure.
 *                            If the supplied port was 0, the default one will
 *                            be set.
 *****************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV TransportMgrAddLocalAddressToList(
                                IN  TransportMgr            *pTransportMgr,
                                IN  RvSipTransportAddr      *pAddressDetails,
                                IN  RvSipListLocation       eLocationInList,
                                IN  TransportMgrLocalAddr   *pBaseLocalAddr,
                                IN  RvBool                  bConvertZeroPort,
                                OUT TransportMgrLocalAddr   **ppLocalAddr,
                                OUT RvUint16                *pPort,
                                RV_TOSDiffserv              priority)
#else
RvStatus RVCALLCONV TransportMgrAddLocalAddressToList(
                                IN  TransportMgr            *pTransportMgr,
                                IN  RvSipTransportAddr      *pAddressDetails,
                                IN  RvSipListLocation       eLocationInList,
                                IN  TransportMgrLocalAddr   *pBaseLocalAddr,
                                IN  RvBool                  bConvertZeroPort,
                                OUT TransportMgrLocalAddr   **ppLocalAddr,
                                OUT RvUint16                *pPort)
#endif
/* SPIRENT_END */
{

    RvStatus                   rv              = RV_OK;
    RLIST_HANDLE               hLocalAddrList  = NULL;
    RLIST_ITEM_HANDLE          listElem        = NULL;
    TransportMgrLocalAddr      localAddr;
    TransportMgrLocalAddr      *pLocalAddr     = NULL;
    RvUint16                   localPortDummy  = 0;
    RvMutex                    tempLock;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE                 hLocalAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */

    if (NULL == pAddressDetails || '\0' == pAddressDetails->strIP[0])
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrAddLocalAddressToList - trial to add NULL address (rv=%d)",
            RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pBaseLocalAddr  &&
        (RVSIP_NEXT_ELEMENT==eLocationInList || RVSIP_PREV_ELEMENT==eLocationInList))
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrAddLocalAddressToList - Base Address is not supplied, while NEXT or PREV location is requested (rv=%d)",
            RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
    /* Check, if the BaseLocalAddress and the new address are of same Transport Type */
    if (NULL != pBaseLocalAddr  &&
       (RVSIP_NEXT_ELEMENT==eLocationInList || RVSIP_PREV_ELEMENT==eLocationInList))
    {
        if (pBaseLocalAddr->addr.eTransport != pAddressDetails->eTransportType)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrAddLocalAddressToList - Base Address and new one have different Transport Types (%s!=%s) (rv=%d)",
                SipCommonConvertEnumToStrTransportType(pBaseLocalAddr->addr.eTransport),
                SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
                RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
        }
    }

    /* Initialize local address structure */
    memset(&localAddr,0 ,sizeof(TransportMgrLocalAddr));
    localAddr.pSocket = NULL;
#if defined(UPDATED_BY_SPIRENT)
    strncpy(localAddr.if_name,pAddressDetails->if_name,sizeof(localAddr.if_name)-1);
    localAddr.vdevblock = pAddressDetails->vdevblock;
#endif
    localAddr.addr.eTransport  = pAddressDetails->eTransportType;
    localAddr.pMgr             = pTransportMgr;
    localAddr.bVacant          = RV_FALSE;
	localAddr.hSentByPage      = NULL_PAGE;
	localAddr.sentByPort       = UNDEFINED;
	localAddr.sentByHostOffset = UNDEFINED;
    localAddr.eSockAddrType    = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
#ifdef RV_SIP_IMS_ON
    localAddr.pSecOwner        = NULL;
    localAddr.secOwnerCounter  = 0;
#endif /*#ifdef RV_SIP_IMS_ON*/

    /*memcpy(localAddr.pstrLocalAddress,pAddressDetails->szIP,RVSIP_TRANSPORT_LEN_STRING_IP);*/

    /*set the transport*/
    switch (pAddressDetails->eTransportType)
    {
        case RVSIP_TRANSPORT_UDP:
           hLocalAddrList = pTransportMgr->hLocalUdpAddrList;
#if defined(UPDATED_BY_SPIRENT)
            hLocalAddrHash = pTransportMgr->hLocalUdpAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
        break;
        case RVSIP_TRANSPORT_TCP:
            hLocalAddrList = pTransportMgr->hLocalTcpAddrList;
#if defined(UPDATED_BY_SPIRENT)
            hLocalAddrHash = pTransportMgr->hLocalTcpAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            hLocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
#if defined(UPDATED_BY_SPIRENT)
            hLocalAddrHash = pTransportMgr->tlsMgr.hLocalTlsAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
            break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            hLocalAddrList = pTransportMgr->sctpData.hSctpLocalAddrList;
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        default:
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrAddLocalAddressToList - Unsupported type of transport - %d (rv=%d)",
                pAddressDetails->eTransportType,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
    }

    if(hLocalAddrList == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrAddLocalAddressToList - Unsupported type of transport - %d ",
                 pAddressDetails->eTransportType));
        return RV_ERROR_BADPARAM;
    }

    /* Set the given port. Set default port if the given port is 0 */
    if (pAddressDetails->port == 0  &&  RV_TRUE==bConvertZeroPort)
    {
        if (RVSIP_TRANSPORT_TLS != pAddressDetails->eTransportType)
        {
            pAddressDetails->port = RVSIP_TRANSPORT_DEFAULT_PORT;
        }
        else
        {
            pAddressDetails->port = RVSIP_TRANSPORT_DEFAULT_TLS_PORT;
        }
    }

#if !(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_FALSE == SipTransportISIpV4(pAddressDetails->strIP))
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportMgrAddLocalAddressToList - The ip is not of IPv4 type, can not add it to the list (rv=%d)",RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* Get the local host and then open the socket. opening the socket with ip 0
       opens a socket for all local Ips according to the given port. Its also
       allows you to send and receive using 127.0.0.1 IP */
    if (strcmp(pAddressDetails->strIP,IPV4_LOCAL_LOOP) == 0 || /*zero IP V4*/
        strcmp(pAddressDetails->strIP,IPV4_LOCAL_ADDRESS) == 0   ||
        strcmp(pAddressDetails->strIP,"0") == 0)
    {
        RvAddressConstructIpv4(&localAddr.addr.addr,0,pAddressDetails->port);
        pAddressDetails->eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    }
    else if (SipTransportISIpV4(pAddressDetails->strIP) == RV_TRUE)/*other IP V4*/
    {
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&localAddr.addr.addr);
        RvAddressSetString(pAddressDetails->strIP,&localAddr.addr.addr);
        RvAddressSetIpPort(&localAddr.addr.addr,pAddressDetails->port);
        pAddressDetails->eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
    }

#if(RV_NET_TYPE & RV_NET_IPV6)
    else
    {
        /*other IP V6*/
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&localAddr.addr.addr);
        rv = TransportMgrConvertString2Address(pAddressDetails->strIP,&(localAddr.addr), RV_FALSE,RV_FALSE);
        if (RV_OK != rv)
        {
            return rv;
        }
        RvAddressSetIpPort(&localAddr.addr.addr,pAddressDetails->port);
        if (UNDEFINED != pAddressDetails->Ipv6Scope)
        {
            RvAddressSetIpv6Scope(&localAddr.addr.addr,pAddressDetails->Ipv6Scope);
        }
        pAddressDetails->eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
}
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */

    rv = TransportMgrLocalAddressGetIPandPortStrings(
                            pTransportMgr,&localAddr.addr.addr,RV_TRUE,RV_FALSE,
                            localAddr.strLocalAddress,&localPortDummy);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrAddLocalAddressToList - Failed to get IP and Port strings (rv=%d)",
            rv));
        return rv;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

    localAddr.tos  = priority.TOSByte;

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

    /*---------------------------------------------------------
            Add local address to list
      --------------------------------------------------------*/
    TransportMgrLocalAddrListLock(pTransportMgr);

    switch (eLocationInList)
    {
        case RVSIP_FIRST_ELEMENT:
            rv = RLIST_InsertHead(  pTransportMgr->hLocalAddrPool,
                                    hLocalAddrList, &listElem);
            break;
        case RVSIP_LAST_ELEMENT:
            rv = RLIST_InsertTail(  pTransportMgr->hLocalAddrPool,
                                    hLocalAddrList, &listElem);
            break;
        case RVSIP_NEXT_ELEMENT:
            rv = RLIST_InsertAfter(  pTransportMgr->hLocalAddrPool,
                 hLocalAddrList,(RLIST_ITEM_HANDLE)pBaseLocalAddr,&listElem);
            break;
        case RVSIP_PREV_ELEMENT:
            rv = RLIST_InsertBefore(  pTransportMgr->hLocalAddrPool,
                 hLocalAddrList,(RLIST_ITEM_HANDLE)pBaseLocalAddr,&listElem);
            break;
        default:
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportMgrAddLocalAddressToList - Illegal location parameter - %d (rv=%d)",
                eLocationInList,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
    }
    if (rv != RV_OK)
    {
        TransportMgrLocalAddrListUnlock(pTransportMgr);
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportMgrAddLocalAddressToList - Failed to add local address to list (rv=%d)",rv));
        return rv;
    }

    pLocalAddr  = (TransportMgrLocalAddr *)listElem;
    /* Copy local structure into the new address.
    Before address overwriting store lock, existing in the address.
    Restore it immediately after copying. */
    memcpy(&tempLock,&pLocalAddr->hLock,sizeof(tempLock));
    memcpy(pLocalAddr,&localAddr,sizeof(localAddr));
    memcpy(&pLocalAddr->hLock,&tempLock,sizeof(tempLock));
    pLocalAddr->bVacant = RV_FALSE;

#if defined(UPDATED_BY_SPIRENT)

    /* Insert Local Address element into hash table */
    if( hLocalAddrHash != NULL && NULL != pPort )
    {
        RvStatus rv;
        void  *  pElem = NULL;
        RvBool   bInserted;

        rv = HASH_InsertElement(
                hLocalAddrHash, pLocalAddr, (void *)&pLocalAddr, 
                RV_FALSE, &bInserted, &pElem, HashCompare_LocalAddr);

        if( rv != RV_OK || bInserted == RV_FALSE )
        {
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

#endif /* defined(UPDATED_BY_SPIRENT) */

    TransportMgrLogLocalAddressInfo(pTransportMgr, pLocalAddr,
        "TransportMgrAddLocalAddressToList - %s Local Address %s:%d (0x%p) was added to list");

    /* SCTP address counting should take in account mutltihoming IPs,
    because SCTP of mixed (both IPv4 and IPv6) type can send messages
    or establish connections to both IPv4 and IPv6 addresses.
        Counters for SCTP will be updated with call to 
    TransportSctpConnectionLocalAddressAdd(),
    TransportSctpConnectionLocalAddressRemove() and LocalAddressClose().
    */
    if (RVSIP_TRANSPORT_SCTP != pAddressDetails->eTransportType)
    {
        LocalAddressCounterUpdate(pTransportMgr, pAddressDetails->eAddrType,
                                  pAddressDetails->eTransportType, 1);
    }

    TransportMgrLocalAddrListUnlock(pTransportMgr);

    if (NULL != ppLocalAddr)
    {
        *ppLocalAddr = pLocalAddr;
    }
    if (NULL != pPort)
    {
        *pPort = pAddressDetails->port;
    }

    return RV_OK;
}

/***************************************************************************
 * TransportMgrCheckConnCapacity
 * ------------------------------------------------------------------------
 * General: Check if the connections capacity reached the maximum we want.
 *          If so, we close the least-recently-used connection.
 *          (The first connection in the connected-no-owner-conn list).
 *          If there is no connection in the list, so we are in a 
 *          out-of-resource condition. nothing to do here.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Output:     pMgr - pointer to the transport manager.
 ***************************************************************************/
void RVCALLCONV TransportMgrCheckConnCapacity(TransportMgr   *pMgr)
{
    RLIST_ITEM_HANDLE pItem = NULL;
    RvSipTransportConnectionHandle* phConn = NULL;
    RvUint32 allocNumOfLists=0;
    RvUint32 allocMaxNumOfUserElement=0;
    RvUint32 currNumOfUsedLists=0;
    RvUint32 currNumOfUsedUserElement=0;
    RvUint32 maxUsageInLists=0;
    RvUint32 maxUsageInUserElement=0;
    RvUint32 capacityPercent = 0;

    if(0 == pMgr->connectionCapacityPercent)
    {
        RvLogExcep(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransportMgrCheckConnCapacity - capacity is 0. shouldn't be here"));
        return;
    }
    RLIST_GetResourcesStatus(pMgr->hConnPool,
                            &allocNumOfLists,
                            &allocMaxNumOfUserElement,
                            &currNumOfUsedLists,
                            &currNumOfUsedUserElement,
                            &maxUsageInLists,
                            &maxUsageInUserElement);

    capacityPercent = (currNumOfUsedUserElement*100/allocMaxNumOfUserElement);
    if(capacityPercent <= pMgr->connectionCapacityPercent &&
       capacityPercent < 100 )
    {
        /* capacity did not reach the maximum capacity. do nothing */
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransportMgrCheckConnCapacity - capacity = %d percent. do nothing (used connections=%d, alloc connections=%d) ",
              capacityPercent,currNumOfUsedUserElement,allocMaxNumOfUserElement));
        return;
    }
    
    /* capacity of connections it too high - 
       close connection that are not in use, if exist */
    RvMutexLock(&pMgr->connectionPoolLock,pMgr->pLogMgr);

    RLIST_GetHead(pMgr->hConnectedNoOwnerConnPool, pMgr->hConnectedNoOwnerConnList, &pItem);
    if(NULL == pItem)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
              "TransportMgrCheckConnCapacity - capacity = %d percent, but no connection in list",
              capacityPercent));
        RvMutexUnlock(&pMgr->connectionPoolLock,pMgr->pLogMgr);
        return;
    }
    phConn = (RvSipTransportConnectionHandle*)pItem;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "TransportMgrCheckConnCapacity - disconnect Connection 0x%p",*phConn));
    
    TransportConnectionRemoveFromConnectedNoOwnerList((TransportConnection*)*phConn);
    TransportConnectionDisconnect((TransportConnection*)*phConn);
        
    RvMutexUnlock(&pMgr->connectionPoolLock,pMgr->pLogMgr);

    
}

#if(RV_NET_TYPE & RV_NET_IPV6)
/******************************************************************************
 * TransportMgrAddrCopyScopeId
 * ----------------------------------------------------------------------------
 * General: Sets Scope ID of the Source address into the Destination Address.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSrcAddr - pointer to the address - source of Scope ID
 *          pDstAddr - pointer to the address to be updated
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV TransportMgrAddrCopyScopeId(
                                                IN  RvAddress* pSrcAddr,
                                                OUT RvAddress* pDstAddr)
{
    RvChar strDestIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    if (RV_ADDRESS_TYPE_IPV6 != RvAddressGetType(pDstAddr))
    {
        return;
    }
    strDestIp[0] = '\0';
    RvAddressGetString(pSrcAddr, RVSIP_TRANSPORT_LEN_STRING_IP, strDestIp);
    if (NULL != strstr(strDestIp, "fe80::"))
    {
        RvAddressSetIpv6Scope(pDstAddr, 
            RvAddressGetIpv6Scope(pSrcAddr));
    }
}
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6)*/

#if(RV_NET_TYPE & RV_NET_IPV6)
/******************************************************************************
* TransportMgrConvertIPv4RvAddressToIPv6
* -----------------------------------------------------------------------------
* General:      Converts IPv4 address to IPv6 form.
* Return Value: RV_OK, if the operation was performed, error code - otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input: pAddr   - the first address.
******************************************************************************/
RvStatus TransportMgrConvertIPv4RvAddressToIPv6(IN RvAddress*  pAddr)
{
    RvAddress addrIPv6;
    RvChar    strIPv6[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvSize_t  ipv6PrefixLen;

    if (NULL == RvAddressConstruct(RV_ADDRESS_TYPE_IPV6, &addrIPv6))
    {
        return RV_ERROR_UNKNOWN;
    }
    if (NULL == RvAddressSetIpPort(&addrIPv6, RvAddressGetIpPort(pAddr)))
    {
        return RV_ERROR_UNKNOWN;
    }
    /* Prepare IPv6 string */
    strcpy(strIPv6,"::ffff:");
    ipv6PrefixLen = strlen(strIPv6);
    if (NULL == RvAddressGetString(pAddr,
                                   RVSIP_TRANSPORT_LEN_STRING_IP-ipv6PrefixLen,
                                   strIPv6+ipv6PrefixLen))
    {
        return RV_ERROR_UNKNOWN;
    }
    if (NULL == RvAddressSetString(strIPv6, &addrIPv6))
    {
        return RV_ERROR_UNKNOWN;
    }
    if (NULL == RvAddressCopy(&addrIPv6, pAddr))
    {
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6)*/

/******************************************************************************
* TransportMgrSocketDestruct
* -----------------------------------------------------------------------------
* General:      destructs Common Core object RvSocket and correspondent
*               File Descriptor object, set into the Select Engine.
*.
* Return Value: RV_OK, if the operation was performed, error code - otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input: pSocket         - socket object to be destrcuted.
*        bCleanSocket    - if RV_TRUE, the underlying socket will be cleaned
*                          from received data.
*        pSelectFd       - file descriptor object to be destrcuted.Can be NULL.
******************************************************************************/
RvStatus TransportMgrSocketDestruct(
                                IN TransportMgr* pTransportMgr,
                                IN RvSocket*     pSocket,
                                IN RvBool        bCleanSocket,
                                IN RvSelectFd*   pSelectFd)
{
    RvStatus crv;

    if (NULL != pSelectFd  &&  RV_INVALID_SOCKET != pSelectFd->fd)
    {
        RvSelectRemove(pTransportMgr->pSelect, pSelectFd);
        RvFdDestruct(pSelectFd);
    }

    crv = RvSocketDestruct(pSocket, bCleanSocket, NULL/*port range*/,
                           pTransportMgr->pLogMgr);
    if (crv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportMgrSocketDestruct: Failed to destruct socket=%d(crv=%d)",
            *pSocket, RvErrorGetCode(crv)));
        return CRV2RV(crv);
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


/************************************************************************************
 * MultiThreadedEnvironmentConstruct
 * ----------------------------------------------------------------------------------
 * General: Initiates multi-threaded SIP stack modules/parameters and initiate
 *          processing threads.
 * Return Value: RvStatus - RV_OK
 *                           RV_ERROR_OUTOFRESOURCES
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   transportCfg    - transport configuration structure
 *            pTransportMgr    - transport instance
 * Output:  none
 ***********************************************************************************/
static RvStatus MultiThreadedEnvironmentConstruct(IN TransportMgr      *pTransportMgr,
                                                   IN SipTransportCfg    *pTransportCfg)
{
    RvStatus    status = RV_OK;
    RvInt32    i;

    pTransportMgr->hProcessingQueue = NULL;
    pTransportMgr->processingThreads = NULL;
    pTransportMgr->hMessageMemoryBuffPool = NULL;
    
    pTransportMgr->numberOfProcessingThreads = pTransportCfg->numberOfProcessingThreads;

    pTransportMgr->processingTaskStackSize   = pTransportCfg->processingTaskStackSize;

    /* Note: we do not check correctness of processingTaskPriority because
    possible values are OS dependent */
    pTransportMgr->processingTaskPriority = pTransportCfg->processingTaskPriority;

    status = RvMutexConstruct(NULL, &pTransportMgr->processingQueueLock);
    if(status != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "MultiThreadedEnvironmentConstruct - Failed initiating processing queue mutex"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* this boolean purpose is to protect processingQueueLock.
       in case we failed in transport manager construction, we will get to MultiThreadedEnvironmentDestruct()
       and then we will try to unlock it.
       for this reason, we update this boolean only here, after allocating the mutex well */
    pTransportMgr->bMultiThreadEnvironmentConstructed = RV_TRUE;
    
    status = PQUEUE_Construct(pTransportCfg->processingQueueSize,
                              pTransportMgr->pLogMgr,
                              &pTransportMgr->processingQueueLock,
                              sizeof(TransportProcessingQueueEvent),
                              (pTransportMgr->numberOfProcessingThreads > 0)?RV_TRUE:RV_FALSE,
                              pTransportMgr->pSelect,
                              "StackProcessingQueue",
                              &(pTransportMgr->hProcessingQueue));
    if (status != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "MultiThreadedEnvironmentConstruct - Failed initiating processing queue"));
        return status;
    }

    PQUEUE_SetPreemptionLocation(pTransportMgr->hProcessingQueue,
        pTransportMgr->preemptionDispatchEvents,
        pTransportMgr->preemptionReadBuffersReady,
        pTransportMgr->preemptionCellAvailable);

    status = RvSelectSetTimeoutInfo(pTransportMgr->pSelect,
                                    MultiThreadedTimerEventHandler,
                                    (void *)pTransportMgr);
    if (status != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "MultiThreadedEnvironmentConstruct - Failed to set timer callback"));
        return status;
    }
    
    pTransportMgr->hMessageMemoryBuffPool = RA_Construct(pTransportCfg->maxBufSize,
                                                         pTransportCfg->numOfReadBuffers,
                                                         NULL,pTransportCfg->pLogMgr,
                                                         "Read buffers pool");
    if (pTransportMgr->hMessageMemoryBuffPool == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "MultiThreadedEnvironmentConstruct - Failed allocating read buffers (maxBufSize=%d, numOfReadBuffers=%d)",
                   pTransportCfg->maxBufSize, pTransportCfg->numOfReadBuffers));

        return RV_ERROR_UNKNOWN;
    }

    pTransportMgr->noEnoughBuffersInPool = RV_FALSE;

    /* Initiating Processing Threads (if any) */
    if (pTransportCfg->numberOfProcessingThreads > 0)
    {
        RvUint threadArraySize = sizeof(RvThread)*pTransportCfg->numberOfProcessingThreads;
        /* Construct processing threads handlers array of the SIP stack */
        status = RvMemoryAlloc(NULL, threadArraySize, pTransportCfg->pLogMgr, (void*)&pTransportMgr->processingThreads);
        if (RV_OK != status)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "MultiThreadedEnvironmentConstruct - Failed initiating processing threads IDs storages"));
            return RV_ERROR_UNKNOWN;
        }
        memset(pTransportMgr->processingThreads,0,threadArraySize);
    }
    
    for(i=0; i < (RvInt32)pTransportCfg->numberOfProcessingThreads; i++)
    {
        status = ProcessingThreadConstruct(pTransportMgr,i,
                                           &(pTransportMgr->processingThreads[i]));
        if (status != RV_OK)
        {
            ProcessingThreadArrayDestruct(pTransportMgr,i);
            RvMemoryFree(pTransportMgr->processingThreads,pTransportCfg->pLogMgr);
            pTransportMgr->processingThreads = NULL;
            
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "MultiThreadedEnvironmentConstruct - Failed to construct the processing thread %d",i));

            return status;
        }
    }

    return status;
}

/************************************************************************************
 * MultiThreadedEnvironmentDestruct
 * ----------------------------------------------------------------------------------
 * General: Destructs multi-threaded SIP stack modules/parameters.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - Handle to the transport manager.
 ***********************************************************************************/
static void MultiThreadedEnvironmentDestruct(IN TransportMgr    *pTransportMgr)
{
    if(pTransportMgr->bMultiThreadEnvironmentConstructed == RV_FALSE)
    {
        /* no need to destruct the MT environment */
        return;
    }
    RvSelectSetTimeoutInfo(pTransportMgr->pSelect, NULL, NULL);
    if (pTransportMgr->hMessageMemoryBuffPool != NULL)
    {
        RA_Destruct(pTransportMgr->hMessageMemoryBuffPool);
    }
    ProcessingThreadArrayDestruct(pTransportMgr,pTransportMgr->numberOfProcessingThreads);
    if ((pTransportMgr->numberOfProcessingThreads > 0) && (pTransportMgr->processingThreads != NULL))
    {   /* In case of multi-threaded configuration */
        RvMemoryFree(pTransportMgr->processingThreads, pTransportMgr->pLogMgr);
        pTransportMgr->processingThreads = NULL;
    }
    
    if (pTransportMgr->hProcessingQueue != NULL)
    {
        PQUEUE_Destruct(pTransportMgr->hProcessingQueue);  
    }
    
    RvMutexDestruct(&pTransportMgr->processingQueueLock, NULL);
}


/************************************************************************************
 * MultiThreadedTimerEventHandler
 * ----------------------------------------------------------------------------------
 * General:  Creates TIMER_EXPIRED_EVENT processing queue event and sends it via the queue
 * for future processes by one of processing threads.
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   context        - timer context
 * Output:  none
 ***********************************************************************************/
static RvBool MultiThreadedTimerEventHandler(IN void    *context)
{
    TransportProcessingQueueEvent    *ev     = NULL;
    RvStatus                        rv = RV_OK;
    TransportMgr                    *pTransportMgr = NULL;
    pTransportMgr                           = (TransportMgr*)context;


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "MultiThreadedTimerEventHandler - Allocating timer expired event"));
#endif
/* SPIRENT_END */

    RvTimerQueueControl(&pTransportMgr->pSelect->tqueue, RV_TIMERQUEUE_DISABLED);

    rv = TransportProcessingQueueAllocateEvent(
            (RvSipTransportMgrHandle)pTransportMgr, NULL, TIMER_EXPIRED_EVENT,
            RV_FALSE/*bAllocateRcvdBuff*/, &ev);

    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "MultiThreadedTimerEventHandler - Failed to allocate timer expired event for timer (rv=%d)",rv));
        
        /* if we ran out of events, we can not process any more timers */
        if (RV_ERROR_OUTOFRESOURCES == rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "MultiThreadedTimerEventHandler - Error is due to OOR, timer remains disabled"));
            pTransportMgr->timerOOR = RV_TRUE;
            return rv;
        }
        RvTimerQueueControl(&pTransportMgr->pSelect->tqueue, RV_TIMERQUEUE_ENABLED);
        return rv;
    }
    rv = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pTransportMgr,ev);

    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "MultiThreadedTimerEventHandler - Failed to tail timer expired event for timer"));
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pTransportMgr,ev);
        RvTimerQueueControl(&pTransportMgr->pSelect->tqueue, RV_TIMERQUEUE_ENABLED);
    }
    return rv;
}

/************************************************************************************
 * ProcessingThreadConstruct
 * ----------------------------------------------------------------------------------
 * General: Initiates a single processing threads.
 * Return Value: RvStatus - RV_OK
 *                          RV_ERROR_OUTOFRESOURCES
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - Transport instance
 *          threadIndex     - the index of the thread in the mgr thread array
 * Output:  pProcThread     - Pointer to the constructed processing thread.
 ***********************************************************************************/
static RvStatus ProcessingThreadConstruct(IN  TransportMgr *pTransportMgr,
                                          IN  RvInt32      threadIndex,
                                          OUT RvThread     *pProcThread)
{
    RvStatus rv;
    RvChar   tName[30];

    rv = RvThreadConstruct(TransportProcessingQueueThreadDispatchEvents,
                           (void *)pTransportMgr,
                           pTransportMgr->pLogMgr,
                           pProcThread);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ProcessingThreadConstruct - Failed to new construct processing thread obj"));
        return CRV2RV(rv);
    }
#define THREAD_NAME_PREFIX  "RvSipProc"   
    sprintf(tName,"%s%d",THREAD_NAME_PREFIX,threadIndex);
    RvThreadSetName(pProcThread,tName);

    rv = RvThreadSetStack(pProcThread,NULL,pTransportMgr->processingTaskStackSize);
    if (rv != RV_OK)
    {
        RvThreadDestruct(pProcThread);
        pProcThread = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ProcessingThreadConstruct - Failed to set stack size of processing thread object 0x%p",
                   pProcThread));
        return CRV2RV(rv);
    }

    rv = RvThreadSetPriority(pProcThread,pTransportMgr->processingTaskPriority);
    if (rv != RV_OK)
    {
        RvThreadDestruct(pProcThread);
        pProcThread = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ProcessingThreadConstruct - Failed to set the priority of processing thread object 0x%p",
                   pProcThread));
        return CRV2RV(rv);
    }

    rv = RvThreadCreate(pProcThread) ;
    if (rv != RV_OK)
    {
        RvThreadDestruct(pProcThread);
        pProcThread = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ProcessingThreadConstruct - Failed to create thread from processing object 0x%p",
                   pProcThread));
        return CRV2RV(rv);
    }

    rv = RvThreadStart(pProcThread);
    if (rv != RV_OK)
    {
        RvThreadDestruct(pProcThread);
        pProcThread = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ProcessingThreadConstruct - Failed to start thread from processing object 0x%p",
                   pProcThread));
        return CRV2RV(rv);
    }

    return RV_OK;
}

/************************************************************************************
 * ProcessingThreadArrayDestruct
 * ----------------------------------------------------------------------------------
 * General: Destructs the transport mgr processing threads array.
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - Transport instance
 *          numOfProcThreads - The number of processing threads to desturct.
 * Output:  -
 ***********************************************************************************/
static void ProcessingThreadArrayDestruct(IN TransportMgr *pTransportMgr,
                                          IN RvUint32      numOfProcThreads)
{
    RvUint   currThread;

    if(pTransportMgr->processingThreads == NULL)
    {
        return;
    }

    for (currThread = 0; currThread < numOfProcThreads; currThread++)
    {
        RvThreadDestruct(&(pTransportMgr->processingThreads[currThread]));
    }
}

/************************************************************************************
 * ConstructLocalAddressLists
 * ----------------------------------------------------------------------------------
 * General:  Construct the local addresses lists. Both TCP and UDP lists are allocated.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 *          pTransportCfg - pointer to the transport configuration stucture
 ***********************************************************************************/
static RvStatus RVCALLCONV ConstructLocalAddressLists(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg)

{

    RvInt32  numOfLocalAddrLists = 1;
    TransportMgrLocalAddr *pLocalAddr;
#if defined(UPDATED_BY_SPIRENT)
    TransportMgrLocalAddr **ppLocalAddr;
#endif /* defined(UPDATED_BY_SPIRENT) */

    if(pTransportCfg->tcpEnabled == RV_FALSE)
    {
        pTransportMgr->numOfUdpAddresses = 1 + pTransportCfg->numOfExtraUdpAddresses;
        pTransportMgr->numOfTcpAddresses = 0;
    }
    else
    {
        pTransportMgr->numOfUdpAddresses = 1 + pTransportCfg->numOfExtraUdpAddresses;
        pTransportMgr->numOfTcpAddresses = 1 + pTransportCfg->numOfExtraTcpAddresses;
        numOfLocalAddrLists = 2;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    numOfLocalAddrLists++;
    pTransportMgr->tlsMgr.numOfTlsAddresses = pTransportCfg->numOfTlsAddresses;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    if(RV_TRUE == pTransportCfg->tcpEnabled)
    {
        numOfLocalAddrLists++;
    }
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */


    pTransportMgr->maxNumOfLocalAddresses = pTransportCfg->maxNumOfLocalAddresses;

    /*construct local address lists lists*/
    /* Create the local addresses pool */
    pTransportMgr->hLocalAddrPool = RLIST_PoolListConstruct(
                                    pTransportMgr->maxNumOfLocalAddresses,
                                    numOfLocalAddrLists,
                                    sizeof(TransportMgrLocalAddr),
                                    pTransportMgr->pLogMgr,
                                    "LocalAddressesPool");
    if (pTransportMgr->hLocalAddrPool == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "ConstructLocalAddressLists - Failed allocating local addresses pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Run through pool of local addresses and initialize them */
    RLIST_PoolGetFirstElem(pTransportMgr->hLocalAddrPool,
                           (RLIST_ITEM_HANDLE*)&pLocalAddr);
    while (NULL != pLocalAddr)
    {
        RvStatus rv;
        /* Create lock.
           Lock has a life cycle same as an one of the TransportManager */
        if (RV_FALSE != pTransportMgr->bDLAEnabled)
        {
            rv = RvMutexConstruct(pTransportMgr->pLogMgr,&pLocalAddr->hLock);
            if(rv != RV_OK)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "ConstructLocalAddressLists - Failed initiating local address mutex"));
                return RV_ERROR_OUTOFRESOURCES;
            }
        }
        /* Set pointer to the Transport Manager.
           The pointer has a life cycle same as an one of the Transport Manager */
        pLocalAddr->pMgr = pTransportMgr;
        /* Set bVacant field */
        pLocalAddr->bVacant = RV_TRUE;
        /* Initialize strLocalAddress string */
        pLocalAddr->strLocalAddress[0] = '\0';
        pLocalAddr->hConnListening = (RvSipTransportConnectionHandle)0L;

        RLIST_PoolGetNextElem(pTransportMgr->hLocalAddrPool,
                              (RLIST_ITEM_HANDLE)pLocalAddr,
                              (RLIST_ITEM_HANDLE*)&pLocalAddr);
    }

    /* Create the local UDP addresses list */
    pTransportMgr->hLocalUdpAddrList = RLIST_ListConstruct(pTransportMgr->hLocalAddrPool);
    if (NULL == pTransportMgr->hLocalUdpAddrList)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "ConstructLocalAddressLists - Failed allocating local udp addresses list"))
        return RV_ERROR_OUTOFRESOURCES;
    }

#if defined(UPDATED_BY_SPIRENT)

    /* construct local UDP addresses hash table */
    pTransportMgr->hLocalUdpAddrHash = NULL;
    if(pTransportMgr->numOfUdpAddresses > HASH_ON_TRESHOLD )
    {
        int numOfAddresses = pTransportMgr->numOfUdpAddresses;
        if (pTransportCfg->isImsClient)
        {
            // increase size of Local Addresses Hash table
            numOfAddresses *= 10;
        }

        pTransportMgr->hLocalUdpAddrHash = HASH_Construct( 
                                            //HASH_DEFAULT_TABLE_SIZE(pTransportMgr->numOfUdpAddresses),
                                            HASH_SIZE,
                                            numOfAddresses,
                                            HashFunc_LocalAddr,
                                            sizeof(TransportMgrLocalAddr*),
                                            sizeof(TransportMgrLocalAddr),
                                            pTransportMgr->pLogMgr,
                                            "Local UDP Addresses Hash table");
        // If HASH_Construct() failed then linear search will be used
    }

#endif /* defined(UPDATED_BY_SPIRENT) */ 

    /* Create the local TCP addresses list */
    if(pTransportCfg->tcpEnabled == RV_TRUE)
    {

        pTransportMgr->hLocalTcpAddrList = RLIST_ListConstruct(
            pTransportMgr->hLocalAddrPool);
        if (NULL == pTransportMgr->hLocalTcpAddrList)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ConstructLocalAddressLists - Failed allocating tcp local addresses list"))
                return RV_ERROR_OUTOFRESOURCES;
        }
#if (RV_NET_TYPE & RV_NET_SCTP)
        pTransportMgr->sctpData.hSctpLocalAddrList =
            RLIST_ListConstruct(pTransportMgr->hLocalAddrPool);
        if (NULL == pTransportMgr->sctpData.hSctpLocalAddrList)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ConstructLocalAddressLists - Failed to construct list of local addresses"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    }

#if defined(UPDATED_BY_SPIRENT)

    /* construct local TCP addresses hash table */
    pTransportMgr->hLocalTcpAddrHash = NULL;
    if(pTransportMgr->numOfTcpAddresses > HASH_ON_TRESHOLD )
    {
        int numOfAddresses = pTransportMgr->numOfTcpAddresses;
        if (pTransportCfg->isImsClient)
        {
            // increase size of Local Addresses Hash table
            numOfAddresses *= 10;
        }

        pTransportMgr->hLocalTcpAddrHash = HASH_Construct( 
                                            //HASH_DEFAULT_TABLE_SIZE(pTransportMgr->numOfTcpAddresses),
                                            HASH_SIZE,
                                            numOfAddresses,
                                            HashFunc_LocalAddr,
                                            sizeof(TransportMgrLocalAddr*),
                                            sizeof(TransportMgrLocalAddr),
                                            pTransportMgr->pLogMgr,
                                            "Local TCP Addresses Hash table");
        // If HASH_Construct() failed then linear search will be used
    }

#endif /* defined(UPDATED_BY_SPIRENT) */


#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Create the local TLS addresses list */
    pTransportMgr->tlsMgr.hLocalTlsAddrList = RLIST_ListConstruct(
                                                pTransportMgr->hLocalAddrPool);
    if (NULL == pTransportMgr->tlsMgr.hLocalTlsAddrList)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                   "ConstructLocalAddressLists - Failed allocating TLS local addresses list"))
        return RV_ERROR_OUTOFRESOURCES;
    }

#if defined(UPDATED_BY_SPIRENT)

    /* construct local TLS addresses hash table */
    pTransportMgr->tlsMgr.hLocalTlsAddrHash = NULL;
    if(pTransportMgr->tlsMgr.numOfTlsAddresses > HASH_ON_TRESHOLD )
    {
        pTransportMgr->tlsMgr.hLocalTlsAddrHash = HASH_Construct( 
                                            //HASH_DEFAULT_TABLE_SIZE(pTransportMgr->tlsMgr.numOfTlsAddresses),
                                            HASH_SIZE,
                                            pTransportMgr->tlsMgr.numOfTlsAddresses,
                                            HashFunc_LocalAddr,
                                            sizeof(TransportMgrLocalAddr*),
                                            sizeof(TransportMgrLocalAddr),
                                            pTransportMgr->pLogMgr,
                                            "Local TLS Addresses Hash table");
        // If HASH_Construct() failed then linear search will be used
    }

#endif /* defined(UPDATED_BY_SPIRENT) */

#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    return RV_OK;
}

#if (RV_TLS_TYPE != RV_TLS_NONE)
/************************************************************************************
 * ConstructTLSResources
 * ----------------------------------------------------------------------------------
 * General:  Construct the TLS resources: context (engines) and sessions.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 ***********************************************************************************/
static RvStatus RVCALLCONV ConstructTLSResources(
                                        IN  TransportMgr    *pTransportMgr)
{
    /* construct RA for TLS engine */
    if (0 != pTransportMgr->tlsMgr.numOfEngines)
    {
        pTransportMgr->tlsMgr.hraEngines = RA_Construct (sizeof(SipTlsEngine),
                                                     pTransportMgr->tlsMgr.numOfEngines,
                                                     NULL,
                                                     pTransportMgr->pLogMgr,
                                                     "TLSEngines");
        if(pTransportMgr->tlsMgr.hraEngines == NULL)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ConstructTLSResources - Failed to construct TLS Engines array"))
            return RV_ERROR_UNKNOWN;

        }
    }
    else
    {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ConstructTLSResources - For TLS to operate, at least 1 TLS engine must be allocated"))
            return RV_ERROR_UNKNOWN;
    }

    if (0 < pTransportMgr->tlsMgr.maxSessions)
    {
        pTransportMgr->tlsMgr.hraSessions = RA_Construct (sizeof(RvTLSSession),
                                                     pTransportMgr->tlsMgr.maxSessions,
                                                     NULL,
                                                     pTransportMgr->pLogMgr,
                                                     "TLSSession");
        if(pTransportMgr->tlsMgr.hraSessions == NULL)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ConstructTLSResources - Failed to construct TLS Sessions array"))
            return RV_ERROR_UNKNOWN;

        }
    }
    else
    {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ConstructTLSResources - For TLS to operate, at least 1 TLS session must be allocated"))
            return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/************************************************************************************
 * ConstructConnectionResources
 * ----------------------------------------------------------------------------------
 * General:  Construct the local addresses lists. Both TCP and UDP lists are allocated.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 *          pTransportCfg - pointer to the transport configuration stucture
 ***********************************************************************************/
static RvStatus RVCALLCONV ConstructConnectionResources(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg)

{
    RvStatus rv = RV_OK;

    if(pTransportMgr->tcpEnabled == RV_TRUE)
    {
        /*lock for the pool of connections*/
        rv = RvMutexConstruct(pTransportMgr->pLogMgr, &pTransportMgr->connectionPoolLock);
        if (RV_OK != rv)
        {
            return rv;
        }
        /*  Allocate the pool lists of messages.
            for each connection there is a list that holds all the outgoing messages.
            when message is sent it is removed from the list. The
            owner of the message is being notified that the message was sent successfuly*/
        pTransportMgr->hMsgToSendPoolList = RLIST_PoolListConstruct(
                                                pTransportMgr->maxPageNuber,
                                                pTransportMgr->maxConnections ,
                                                sizeof(SipTransportSendInfo),
                                                pTransportMgr->pLogMgr,
                                                "Connection MsgToSend Pool");

        if(pTransportMgr->hMsgToSendPoolList== NULL)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ConstructConnectionResources - Failed to construct the Connection MsgToSend Pool"));
            RvMutexDestruct(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /*Allocated the pool lists of owners.
        for each connection there is a list that holds all the owners of the
        connection. when a connection is removed it notifies all owners*/
        pTransportMgr->hConnectionOwnersPoolList = RLIST_PoolListConstruct(
                                                pTransportCfg->maxConnOwners,
                                                pTransportMgr->maxConnections ,
                                                sizeof(TransportConnectionOwnerInfo),
                                                pTransportMgr->pLogMgr,
                                                "Connection Owners Pool");

        if(pTransportMgr->hConnectionOwnersPoolList== NULL)
        {
               RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                       "ConstructConnectionResources - Failed to construct the Connection Owners Pool"));
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* initialize Connection list */
        rv = TransportConnectionListConstruct (pTransportMgr, pTransportMgr->maxConnections);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                      "ConstructConnectionResources - Failed to initialize Conn list (rv=%d)",rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else
    {
        if (pTransportMgr->tlsMgr.numOfTlsAddresses > 0)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                      "ConstructConnectionResources - TCP is disabled, but there are %d TLS addresses (rv=%d)",
                      pTransportMgr->tlsMgr.numOfTlsAddresses ,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    return RV_OK;
}

/************************************************************************************
 * ConstructLocalAddressLists
 * ----------------------------------------------------------------------------------
 * General:  Construct the local addresses lists. Both TCP and UDP lists are allocated.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 *          pTransportCfg - pointer to the transport configuration structure
 ***********************************************************************************/
static RvStatus RVCALLCONV InitializeLocalAddressLists(
                                        IN  TransportMgr    *pTransportMgr,
                                        IN  SipTransportCfg *pTransportCfg)
{
    RvStatus           rv;
    RvSipTransportAddr  addrDetails;
    int                 i;

    /*first UDP address*/
    InitStructTransportAddr(RVSIP_TRANSPORT_UDP,
			    RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED,pTransportCfg->udpAddress,
			    pTransportCfg->udpPort,
#if defined(UPDATED_BY_SPIRENT)
			    pTransportCfg->if_name,
			    pTransportCfg->vdevblock,
#endif
			    &addrDetails);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
            RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
            &pTransportCfg->udpPort, pTransportCfg->localPriority);
#else
    rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
            RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
            &pTransportCfg->udpPort);
#endif
/* SPIRENT_END */
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "InitializeLocalAddressLists - Failed to initialize first local udp address (rv=%d)",rv));
        return rv;
    }

    /*set the rest of the UDP addresses*/
    for(i=0; i<pTransportCfg->numOfExtraUdpAddresses; i++)
    {
        if (NULL == pTransportCfg->localUdpAddresses[i])
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - numOfExtraUdpAddresses=%d, but no addresses supplied (rv=%d)",
                pTransportCfg->numOfExtraUdpAddresses,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
        }

        InitStructTransportAddr(RVSIP_TRANSPORT_UDP,
				RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED,
				pTransportCfg->localUdpAddresses[i],pTransportCfg->localUdpPorts[i],
#if defined(UPDATED_BY_SPIRENT)
				pTransportCfg->localIfNames[i],
				pTransportCfg->localVDevBlocks[i],
#endif
				&addrDetails);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails, 
                 RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                 &pTransportCfg->localUdpPorts[i], pTransportCfg->localPriorities[i]);
#else
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails, 
                 RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                 &pTransportCfg->localUdpPorts[i]);
#endif
/* SPIRENT_END */
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - Failed to initialize local udp address number %d (rv=%d)",i,rv));
            return rv;
        }
    }

    if(pTransportMgr->tcpEnabled == RV_FALSE)
    {
        return RV_OK;
    }
    /*set first TCP address */
/* SPIRENT_BEGIN */
/*
 * COMMENTS: If it is TLS, no TCP should be set, and vice versa
 */
#if defined(UPDATED_BY_SPIRENT) && (RV_TLS_TYPE != RV_TLS_NONE)
    if (pTransportCfg->numOfTlsAddresses == 0) 
    {
#endif
/* SPIRENT_END */
    InitStructTransportAddr(RVSIP_TRANSPORT_TCP,
			    RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED,pTransportCfg->tcpAddress,
			    pTransportCfg->tcpPort,
#if defined(UPDATED_BY_SPIRENT)
			    pTransportCfg->if_name,
			    pTransportCfg->vdevblock,
#endif
			    &addrDetails);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
            RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
            &pTransportCfg->tcpPort, pTransportCfg->localPriority);
#else
    rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
            RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
            &pTransportCfg->tcpPort);
#endif
/* SPIRENT_END */
    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                      "InitializeLocalAddressLists - Failed to initialize first local tcp address (rv=%d)",rv));
        return rv;
    }

    /*set the rest of the TCP addresses*/
    for(i=0; i<pTransportCfg->numOfExtraTcpAddresses; i++)
    {
        if (NULL == pTransportCfg->localTcpAddresses[i])
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - numOfExtraTcpAddresses=%d, but no addresses supplied (rv=%d)",
                pTransportCfg->numOfExtraTcpAddresses,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
        }

        InitStructTransportAddr(RVSIP_TRANSPORT_TCP,
				RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED,
				pTransportCfg->localTcpAddresses[i],pTransportCfg->localTcpPorts[i],
#if defined(UPDATED_BY_SPIRENT)
				pTransportCfg->localIfNames[i],
				pTransportCfg->localVDevBlocks[i],
#endif
				&addrDetails);
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
                RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                &pTransportCfg->localTcpPorts[i], pTransportCfg->localPriorities[i]);
#else
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
                RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                &pTransportCfg->localTcpPorts[i]);
#endif
/* SPIRENT_END */
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - Failed to initialize local tcp address number %d (rv=%d)",i,rv));
            return rv;
        }
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    rv = TransportSctpMgrInitializeLocalAddressList(pTransportMgr,pTransportCfg);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "InitializeLocalAddressLists - TransportSctpMgrInitializeLocalAddressList failed (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
#endif /*(RV_NET_TYPE & RV_NET_SCTP)*/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT) && (RV_TLS_TYPE != RV_TLS_NONE)
    }
#endif
/* SPIRENT_END */

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /*set the TLS addresses*/
    for(i=0; i<pTransportCfg->numOfTlsAddresses; i++)
    {
        if (NULL == pTransportCfg->localTlsAddresses[i])
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - localTlsAddresses=%d, but no addresses supplied (rv=%d)",
                pTransportCfg->numOfTlsAddresses,RV_ERROR_BADPARAM));
            return RV_ERROR_BADPARAM;
        }

        InitStructTransportAddr(RVSIP_TRANSPORT_TLS,
				RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED,
				pTransportCfg->localTlsAddresses[i],pTransportCfg->localTlsPorts[i],
#if defined(UPDATED_BY_SPIRENT)
				pTransportCfg->localIfNames[i],
				pTransportCfg->localVDevBlocks[i],
#endif
				&addrDetails);
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        if (i)
        {
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
                RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                &pTransportCfg->localTlsPorts[i], pTransportCfg->localPriorities[i-1]);
        }
        else
        {
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
                RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                &pTransportCfg->localTlsPorts[i], pTransportCfg->localPriority);
        }
#else
        rv = TransportMgrAddLocalAddressToList(pTransportMgr,&addrDetails,
                RVSIP_LAST_ELEMENT, NULL, RV_TRUE/*bConvertZeroPort*/, NULL,
                &pTransportCfg->localTlsPorts[i]);
#endif
/* SPIRENT_END */
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "InitializeLocalAddressLists - Failed to initialize local tls address number %d (rv=%d)",i,rv));
            return rv;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    return RV_OK;
}

/************************************************************************************
 * OpenLocalAddresses
 * ----------------------------------------------------------------------------------
 * General:  Construct the local addresses lists. Both TCP and UDP lists are allocated.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV OpenLocalAddresses(
                                    IN TransportMgr     *pTransportMgr,
                                    IN SipTransportCfg  *pTransportCfg)
{

    RvStatus                  rv = RV_OK;
#ifdef RV_SIP_DISABLE_LISTENING_SOCKETS
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "OpenLocalAddresses - Sockets will not be opened. Stack was compiled with the RV_SIP_DISABLE_LISTENING_SOCKETS flag"));
    return rv;
#else
    /* Open UDP Addresses */
    rv = OpenLocalAddressesFromList(pTransportMgr,
                                    pTransportMgr->hLocalUdpAddrList);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "OpenLocalAddresses - Failed to open UDP addresses (rv=%d)",rv));
        return rv;
    }

    /* Open TCP Addresses */
    if(pTransportMgr->tcpEnabled == RV_FALSE ||
       pTransportMgr->hLocalTcpAddrList == NULL)
    {
        return RV_OK;
    }
    rv = OpenLocalAddressesFromList(pTransportMgr,
                                    pTransportMgr->hLocalTcpAddrList);
    if (rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "OpenLocalAddresses - Failed to open TCP addresses (rv=%d)",rv));
        return rv;
    }

    /* Open SCTP Addresses */
#if (RV_NET_TYPE & RV_NET_SCTP)
    rv = TransportSctpMgrOpenLocalAddresses(pTransportMgr,pTransportCfg);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "OpenLocalAddresses - Failed to open SCTP addresses (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
#else
    RV_UNUSED_ARG(pTransportCfg)
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    /* Open TLS Addresses */
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (0 < pTransportMgr->tlsMgr.numOfTlsAddresses)
    {
        rv = OpenLocalAddressesFromList(pTransportMgr,
                                        pTransportMgr->tlsMgr.hLocalTlsAddrList);
        if (rv != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "OpenLocalAddresses - Failed to open TLS addresses (rv=%d)",rv));
            return rv;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    return RV_OK;
#endif /*#ifdef RV_SIP_DISABLE_LISTENING_SOCKETS*/
}
/************************************************************************************
 * OpenLocalAddressesFromList
 * ----------------------------------------------------------------------------------
 * General:  open local addresses from a given list.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 *          hLocalAddrList     - the list of addresses to be opened.
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV OpenLocalAddressesFromList(
                            IN    TransportMgr          *pTransportMgr,
                            IN    RLIST_HANDLE           hLocalAddrList)
{
    RvStatus               rv          = RV_OK;
    RLIST_ITEM_HANDLE       hListElem   = NULL;
    TransportMgrLocalAddr   *pLocalAddr =NULL;

    TransportMgrLocalAddrListLock(pTransportMgr);

    RLIST_GetHead(pTransportMgr->hLocalAddrPool, hLocalAddrList, &hListElem);
    while (NULL != hListElem)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)							
        unsigned char tos = 0;
        int           value = 0;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        pLocalAddr = (TransportMgrLocalAddr *)hListElem;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)							
        tos = pLocalAddr->tos;
#if(RV_NET_TYPE & RV_NET_IPV6)
        // set traffic class for IPv6 according to TOS byte from configuration
        if (pLocalAddr->addr.addr.addrtype == RV_ADDRESS_TYPE_IPV6) {
            pLocalAddr->addr.addr.data.ipv6.flowinfo = ((unsigned long) tos) << 4;
        }
#endif /*RV_IPV6*/
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


        rv = LocalAddressOpen(pTransportMgr, pLocalAddr, NULL/*pBoundPort*/);
        if (rv != RV_OK)
        {
            TransportMgrLogLocalAddressError(pTransportMgr, pLocalAddr, rv,
                "OpenLocalAddressesFromList - Failed to open local %s address %s:%d (0x%p). rv=%d");
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            return rv;
        }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)							
#if(RV_NET_TYPE & RV_NET_IPV6)
        if (pLocalAddr->addr.addr.addrtype == RV_ADDRESS_TYPE_IPV6) {
            value = 1; // send flow information
            rv = RvSocketSetSockOpt(pLocalAddr->pSocket, IPPROTO_IPV6, IPV6_FLOWINFO_SEND, (void *)&value, sizeof(value));
            if (RV_OK != rv)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "OpenLocalAddressesFromList - Failed to set IPV6_FLOWINFO_SEND=%d for socket %d, rv=%d",
                    value, *(int*)(pLocalAddr->pSocket),RvErrorGetCode(rv)));                
                rv = RV_OK; // set the status back to RV_OK to ignore this setting and continue!
            }

        } else
#endif // RV_IPV6
        {
            rv = RvSocketSetTypeOfService(pLocalAddr->pSocket, tos, pTransportMgr->pLogMgr);
            if (RV_OK != rv)
            {
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "OpenLocalAddressesFromList - Failed to set TypeOfService=%d for socket %d, rv=%d",
                    tos, *(int*)(pLocalAddr->pSocket),RvErrorGetCode(rv)));                
                rv = RV_OK; // set the status back to RV_OK to ignore this setting and continue!
            }
        }

#endif
/* SPIRENT_END */

        RLIST_GetNext(pTransportMgr->hLocalAddrPool, hLocalAddrList,
                      hListElem, &hListElem);
    }

    TransportMgrLocalAddrListUnlock(pTransportMgr);

    return RV_OK;
}


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
#if(RV_NET_TYPE & RV_NET_IPV6)
/************************************************************************************
 * PrintAddressFromZero6Annotation
 * ----------------------------------------------------------------------------------
 * General:  Expands the local address annotations for IPv4
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV PrintAddressFromZero6Annotation(
                                        IN    TransportMgr*    pTransportMgr,
                                        IN    RvChar*          strTransport,
                                        IN    RvUint16         port)
{
    RvChar     buff[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvAddress  addr;
    RvStatus   rv;

    RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&addr);
    rv = RvSipTransportGetIPv6LocalAddress((RvSipTransportMgrHandle)pTransportMgr,(RvUint8*)addr.data.ipv6.ip);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "PrintAddressFromZero6Annotation - RvSipTransportGetIPv6LocalAddress(transport=%s, port=%d) failed",
            strTransport, port));
    }

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "PrintAddressFromZero6Annotation - %s translated: %s:%d",
               strTransport, RvAddressGetString(&addr,RVSIP_TRANSPORT_LEN_STRING_IP,buff),port));
}
#else /*(RV_NET_TYPE & RV_NET_IPV6)*/
#define PrintAddressFromZero6Annotation(_a,_b,_c)
#endif

/************************************************************************************
 * PrintAddressFromZero4Annotation
 * ----------------------------------------------------------------------------------
 * General:  Expands the local address annotations for IPv4
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV PrintAddressFromZero4Annotation(
                                            IN    TransportMgr*  pTransportMgr,
                                            IN    RvChar*        strTransport,
                                            IN    RvUint16       port)
{
    RvUint32   numberOfAddresses   = 0;
    RvUint32   i                   = 0;
    RvChar     buff[RVSIP_TRANSPORT_LEN_BYTES_IPV6 + 1];
    RvUint32   ip;
    RvStatus   rv;

    rv = SipTransportGetNumOfIPv4LocalAddresses(pTransportMgr,&numberOfAddresses);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "PrintAddressFromZero4Annotation - SipTransportGetNumOfIPv4LocalAddresses() failed (vr=%d)",rv));
        return;
    }
    for (i=0; i<numberOfAddresses;i++)
    {
        rv = SipTransportGetIPv4LocalAddressByIndex(pTransportMgr,(RvUint)i,(RvUint8 *)&ip);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "PrintAddressFromZero4Annotation - Failed to Get IPv4 local address (vr=%d)",rv));
            continue;
        }
        RvSipTransportConvertIpToString((RvSipTransportMgrHandle)pTransportMgr,
                                        (RvUint8*)&ip,
                                        RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                                        (RVSIP_TRANSPORT_LEN_BYTES_IPV6+1),
                                        buff);

        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "                                   %s expanded (%d/%d): %s:%u",
                   strTransport,i+1,numberOfAddresses, buff, port));
    }
}

/************************************************************************************
 * PrintOneAddressList
 * ----------------------------------------------------------------------------------
 * General:  Prints all the local addresses the stack listens to
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV PrintOneAddressList(
                                        IN    TransportMgr*          pTransportMgr,
                                        IN    RvChar*               strTransport,
                                        IN    RLIST_HANDLE           hLocalAddrList)
{
    TransportMgrLocalAddr   *pLocalAddr;
    RLIST_ITEM_HANDLE       hListElem;
    RvChar                  localIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvChar                  localIpWithoutScopeId[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvUint16                localPort;

    TransportMgrLocalAddrListLock(pTransportMgr);
    RLIST_GetHead(pTransportMgr->hLocalAddrPool, hLocalAddrList, &hListElem);
    while (NULL != hListElem)
    {
        RvStatus rv;
        pLocalAddr = (TransportMgrLocalAddr *)hListElem;
        rv = TransportMgrLocalAddrLock(pLocalAddr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "PrintOneAddressList - Failed to lock Local Address 0x%p (rv=%d)",pLocalAddr,rv));
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            return;
        }
        localIp[0] = '\0';
        rv = TransportMgrLocalAddressGetIPandPortStrings(pTransportMgr,
                   &pLocalAddr->addr.addr,RV_FALSE,RV_TRUE,localIp,&localPort);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "PrintOneAddressList - Failed to get IP and Port strings for Local Address 0x%p (rv=%d)",
                pLocalAddr,rv));
            TransportMgrLocalAddrUnlock(pLocalAddr);
            TransportMgrLocalAddrListUnlock(pTransportMgr);
            return;
        }
        rv = TransportMgrLocalAddressGetIPandPortStrings(pTransportMgr,
            &pLocalAddr->addr.addr,RV_FALSE,RV_FALSE,localIpWithoutScopeId,&localPort);
        TransportMgrLocalAddrUnlock(pLocalAddr);

        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "PrintOneAddressList - %s Address: %s:%d",strTransport,localIp,localPort));

        if (0 == strcmp(IPV4_LOCAL_ADDRESS,localIp))
        {
            PrintAddressFromZero4Annotation(pTransportMgr,strTransport,localPort);
        }
        if (0 == strcmp(IPV6_LOCAL_ADDRESS,localIpWithoutScopeId))
        {
            PrintAddressFromZero6Annotation(pTransportMgr,strTransport,localPort);
        }

        RLIST_GetNext(pTransportMgr->hLocalAddrPool, hLocalAddrList,
                      hListElem, &hListElem);
    }

    TransportMgrLocalAddrListUnlock(pTransportMgr);
}

/************************************************************************************
 * TransportMgrPrintLocalAddresses
 * ----------------------------------------------------------------------------------
 * General:  Prints all the local addresses the stack listens to
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager.
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrPrintLocalAddresses(
                                        IN    TransportMgr          *pTransportMgr )
{


#ifndef RV_SIP_DISABLE_LISTENING_SOCKETS
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
             "PrintLocalAddresses ================ Local Addresses ================"));
#else
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
             "PrintLocalAddresses ================ Local Addresses (with no listening sockets==="));
#endif /*#ifndef RV_SIP_DISABLE_LISTENING_SOCKETS*/


    /* Udp Addresses */
    PrintOneAddressList(pTransportMgr,"UDP",pTransportMgr->hLocalUdpAddrList);

    /* Tcp Addresses */
    if(pTransportMgr->hLocalTcpAddrList != NULL)
    {
        PrintOneAddressList(pTransportMgr,"TCP",pTransportMgr->hLocalTcpAddrList);
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Tls Addresses */
    if(pTransportMgr->tlsMgr.hLocalTlsAddrList != NULL)
    {
        PrintOneAddressList(pTransportMgr,"TLS",pTransportMgr->tlsMgr.hLocalTlsAddrList);
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    /* Sctp Addresses */
    if(pTransportMgr->sctpData.hSctpLocalAddrList != NULL)
    {
        PrintOneAddressList(pTransportMgr,"SCTP",
                            pTransportMgr->sctpData.hSctpLocalAddrList);
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
             "PrintLocalAddresses ===================================================="));

    return RV_OK;
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
/************************************************************************************
 * InitializeOutboundAddress
 * ----------------------------------------------------------------------------------
 * General: Initialize the outbound address used by the stack.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStrIp6 - The address string.
 * Output:  (-)
 ***********************************************************************************/
static RvStatus RVCALLCONV InitializeOutboundAddress(
                                    IN TransportMgr        *pTransportMgr,
                                    IN SipTransportCfg     *pTransportCfg)
{
    RvInt32  length;
    memset(&pTransportMgr->outboundAddr,0,sizeof(TransportOutboundAddr));
    if(pTransportCfg->outboundProxyIp != NULL &&
       pTransportCfg->outboundProxyIp[0] != '\0')
    {
        length = (RvInt32)strlen(pTransportCfg->outboundProxyIp)+1;

        if (RV_OK != RvMemoryAlloc(NULL, length, pTransportCfg->pLogMgr, (void*)&pTransportMgr->outboundAddr.strIp))
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                       "InitializeOutboundAddress - Failed to allocate outbound proxy ip"));
            return RV_ERROR_OUTOFRESOURCES;
        }
        strcpy(pTransportMgr->outboundAddr.strIp,pTransportCfg->outboundProxyIp);
    }

    pTransportMgr->outboundAddr.port      = (RvUint16)((pTransportCfg->outboundProxyPort == UNDEFINED || pTransportCfg->outboundProxyPort > 65535) ? RVSIP_TRANSPORT_DEFAULT_PORT : pTransportCfg->outboundProxyPort);
    pTransportMgr->outboundAddr.transport = pTransportCfg->eOutboundProxyTransport;
#ifdef RV_SIGCOMP_ON
    pTransportMgr->outboundAddr.compType  = pTransportCfg->eOutboundProxyCompression;
#endif

    /*if there is no outbound ip see if there is a domain name*/
    if(pTransportMgr->outboundAddr.strIp == NULL)
    {
        if(pTransportCfg->outboundProxyHostName != NULL &&
           strcmp(pTransportCfg->outboundProxyHostName,"") != 0)
        {
            length = (RvInt32)strlen(pTransportCfg->outboundProxyHostName)+1;

            if (RV_OK != RvMemoryAlloc(NULL, length, pTransportCfg->pLogMgr, (void*)&pTransportMgr->outboundAddr.strHostName))
            {
                RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                          "InitializeOutboundAddress - Failed to allocate outbound proxy name"));
                return RV_ERROR_OUTOFRESOURCES;
            }
            strcpy(pTransportMgr->outboundAddr.strHostName,pTransportCfg->outboundProxyHostName);
        }
    }
    return RV_OK;
}

/******************************************************************************
 * LocalAddressOpen
 * ----------------------------------------------------------------------------
 * General: Open local address
 * Return Value: RV_OK on success, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - transport manager
 *          pLocalAddr      - address to be opened
 * Output:  pBoundPort      - port, to which the Local Address socket was bound
 *****************************************************************************/
static RvStatus RVCALLCONV LocalAddressOpen(
                                IN  TransportMgr*          pTransportMgr,
                                IN  TransportMgrLocalAddr* pAddress,
                                OUT RvUint16*              pBoundPort)
{
    RvStatus rv;
    RvBool   bNewConnCreated = RV_FALSE;
    RvUint16 boundPort;

    rv = TransportMgrLocalAddrLock(pAddress);
    if (RV_OK != rv)
    {
        return RV_ERROR_NULLPTR;
    }

    pAddress->eSockAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pAddress->addr.addr));

    switch (pAddress->addr.eTransport)
    {
        case RVSIP_TRANSPORT_UDP:
            {
                RvStatus rv;
                rv = TransportUDPOpen(pTransportMgr, pAddress);
                if(rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to open local %s address %s:%d (0x%p). rv=%d");
                    TransportMgrLocalAddrUnlock(pAddress);
                    return RV_ERROR_UNKNOWN;
                }
            }
            break;  /* END OF  case RVSIP_TRANSPORT_UDP:*/

        case RVSIP_TRANSPORT_TCP:
            {
                /* Construct TCP connection */
                RvSipTransportConnectionHandle   hConn = NULL;
                rv = TransportConnectionConstruct(pTransportMgr,
                    RV_TRUE, RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER,
                    RVSIP_TRANSPORT_TCP, (RvSipTransportLocalAddrHandle)pAddress,
                    NULL, NULL, NULL, NULL, &bNewConnCreated,&hConn);
                if (rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to construct connection for %s Address %s:%d (0x%p). rv=%d");
                    TransportMgrLocalAddrUnlock(pAddress);
                    return rv;
                }
                rv = TransportConnectionLockAPI((TransportConnection*)hConn);
                if(rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to construct lock connection for %s Address %s:%d (0x%p). rv=%d");
                    TransportConnectionFree((TransportConnection*)hConn);
                    TransportMgrLocalAddrUnlock(pAddress);
                    return RV_ERROR_INVALID_HANDLE;
                }
                /* open the TCP Server session */
                rv = TransportTCPOpen(pTransportMgr,(TransportConnection*)hConn,
                        RV_FALSE/*bConnect*/, 0/*clientLocalPort -> local port will be choosen by OS*/);
                if (rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to open TCP Server session from %s Address %s:%d (0x%p). rv=%d");
                    TransportConnectionFree((TransportConnection*)hConn);
                    TransportConnectionUnLockAPI((TransportConnection*)hConn); /*unlock the connection*/
                    TransportMgrLocalAddrUnlock(pAddress);
                    return rv;
                }
                pAddress->ccSocket = ((TransportConnection*)hConn)->ccSocket;
                pAddress->pSocket = &pAddress->ccSocket;
                pAddress->hConnListening = hConn;
                TransportConnectionUnLockAPI((TransportConnection*)hConn); /*unlock the connection*/
            }
            break;  /* END OF  case RVSIP_TRANSPORT_TCP:*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            {
                /* Construct TCP connection */
                RvSipTransportConnectionHandle   hConn = NULL;
                rv = TransportConnectionConstruct(pTransportMgr,
                        RV_TRUE, RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER,
                        RVSIP_TRANSPORT_TLS,(RvSipTransportLocalAddrHandle)pAddress,
                        NULL, NULL, NULL, NULL, &bNewConnCreated,&hConn);
                if (rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to construct connection for %s Address %s:%d (0x%p). rv=%d");
                    TransportMgrLocalAddrUnlock(pAddress);
                    return rv;
                }
                rv = TransportConnectionLockAPI((TransportConnection*)hConn);
                if(rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to construct lock connection for %s Address %s:%d (0x%p). rv=%d");
                    TransportConnectionFree((TransportConnection*)hConn);
                    TransportMgrLocalAddrUnlock(pAddress);
                    return RV_ERROR_INVALID_HANDLE;
                }
                /* open the TCP Server session */
                rv = TransportTCPOpen(pTransportMgr,(TransportConnection*)hConn,
                        RV_FALSE/*bConnect*/, 0/*clientLocalPort -> local port will be choosen by OS*/);
                if (rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to open TCP Server session from %s Address %s:%d (0x%p). rv=%d");
                    TransportConnectionFree((TransportConnection*)hConn);
                    TransportConnectionUnLockAPI((TransportConnection*)hConn); /*unlock the connection*/
                    TransportMgrLocalAddrUnlock(pAddress);
                    return rv;
                }
                pAddress->ccSocket = ((TransportConnection*)hConn)->ccSocket;
                pAddress->hConnListening = hConn;
                pAddress->pSocket = &pAddress->ccSocket;
                TransportConnectionUnLockAPI((TransportConnection*)hConn); /*unlock the connection*/
            }
            break;  /* END OF  case RVSIP_TRANSPORT_TLS:*/
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            {
                rv = TransportSctpLocalAddressOpen(pTransportMgr, pAddress->strLocalAddress, pAddress);
                if (rv != RV_OK)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressOpen - Failed to open SCTP address from %s Address %s:%d (0x%p). rv=%d");
                    TransportMgrLocalAddrUnlock(pAddress);
                    return rv;
                }
            }
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        default:
            TransportMgrLocalAddrUnlock(pAddress);
            return RV_ERROR_BADPARAM;
    }

    /* If zero port was provided, update the Local Address object
    with the actual port, to which the address was bound */
    boundPort = RvAddressGetIpPort(&pAddress->addr.addr);
    if (0 == boundPort)
    {
        RvStatus  crv;
        RvAddress ccLocalAddress;
        crv = RvSocketGetLocalAddress(&pAddress->ccSocket,
                                      pTransportMgr->pLogMgr, &ccLocalAddress);
        if (RV_OK != crv)
        {
            rv = CRV2RV(crv);
            TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                "LocalAddressOpen - Failed to get socket's local address for %s Local Address %s:%d (0x%p). rv=%d");
            if (NULL != pAddress->hConnListening)
            {
                rv = TransportConnectionLockAPI(
                        (TransportConnection*)pAddress->hConnListening);
                if (RV_OK == rv)
                {
                    TransportConnectionFree(
                        (TransportConnection*)pAddress->hConnListening);
                    TransportConnectionUnLockAPI(
                        (TransportConnection*)pAddress->hConnListening);
                }
                pAddress->hConnListening = NULL;
            }
            pAddress->pSocket = NULL;
            TransportMgrLocalAddrUnlock(pAddress);
            return rv;
        }
        boundPort = RvAddressGetIpPort(&ccLocalAddress);
        RvAddressSetIpPort(&pAddress->addr.addr, boundPort);
    }
    if (NULL != pBoundPort)
    {
        *pBoundPort = boundPort;
    }

    TransportMgrLocalAddrUnlock(pAddress);
    return RV_OK;
}

/******************************************************************************
 * LocalAddressClose
 * ----------------------------------------------------------------------------
 * General: Close local address
 * Return Value: RV_OK on success, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - transport manager
 *          pLocalAddr      - address to be closed
 * Output:
 *****************************************************************************/
static RvStatus RVCALLCONV LocalAddressClose(
                                IN  TransportMgr*         pTransportMgr,
                                IN  TransportMgrLocalAddr *pAddress)
{
    RvStatus   rv;

    rv = TransportMgrLocalAddrLock(pAddress);
    if (RV_OK != rv)
    {
        return RV_ERROR_NULLPTR;
    }

#ifdef RV_SIP_IMS_ON
    if (pAddress->secOwnerCounter > 0)
    {
        TransportMgrLogLocalAddressWarning(pTransportMgr, pAddress, pAddress->secOwnerCounter,
            "LocalAddressClose - Can 't remove Local %s address %s:%d (0x%p): there are %d security owneres");
        TransportMgrLocalAddrUnlock(pAddress);
        return RV_ERROR_TRY_AGAIN;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    switch (pAddress->addr.eTransport)
    {
        case RVSIP_TRANSPORT_UDP:
            {
                rv = TransportUDPClose(pTransportMgr,pAddress);
                if(RV_OK != rv)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressClose - Failed to close local %s address %s:%d (0x%p). rv=%d");
                }
            }
            break;  /* END OF  case RVSIP_TRANSPORT_UDP:*/

        case RVSIP_TRANSPORT_TCP:
            if (NULL != pAddress->hConnListening)
            {
                rv = TransportTCPClose((TransportConnection*)pAddress->hConnListening);
                if(RV_OK != rv)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressClose - Failed to close local %s address %s:%d (0x%p). rv=%d");
                }
                pAddress->hConnListening = NULL;
            }
            else
            {
                TransportMgrLogLocalAddressWarning(pTransportMgr, pAddress, rv,
                    "LocalAddressClose - No connection for local %s address %s:%d (0x%p) was found. rv=%d");
            }
            break;  /* END OF  case RVSIP_TRANSPORT_TCP:*/

#if (RV_TLS_TYPE==RV_TLS_OPENSSL)
        case RVSIP_TRANSPORT_TLS:
            if (NULL != pAddress->hConnListening)
            {
                rv = TransportTlsClose((TransportConnection*)pAddress->hConnListening);
                if(RV_OK != rv)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressClose - Failed to close local %s address %s:%d (0x%p). rv=%d");
                }
                pAddress->hConnListening = NULL;
            }
            else
            {
                TransportMgrLogLocalAddressWarning(pTransportMgr, pAddress, rv,
                    "LocalAddressClose - No connection for local %s address %s:%d (0x%p) was found. rv=%d");
            }
            break;  /* END OF  case RVSIP_TRANSPORT_TLS:*/
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            if (NULL != pAddress->hConnListening)
            {
                RvUint32 numIPv4addrs =
                    ((TransportConnection*)pAddress->hConnListening)->sctpData.numIPv4addrs;
                RvUint32 numIPv6addrs =
                    ((TransportConnection*)pAddress->hConnListening)->sctpData.numIPv6addrs;
                rv = TransportSctpClose((TransportConnection*)pAddress->hConnListening);
                if(RV_OK != rv)
                {
                    TransportMgrLogLocalAddressError(pTransportMgr, pAddress, rv,
                        "LocalAddressClose - Failed to close local %s address %s:%d (0x%p). rv=%d");
                }
                pAddress->hConnListening = NULL;
                pTransportMgr->localAddrCounters[RVSIP_TRANSPORT_ADDRESS_TYPE_IP][RVSIP_TRANSPORT_SCTP] -= numIPv4addrs;
                pTransportMgr->localAddrCounters[RVSIP_TRANSPORT_ADDRESS_TYPE_IP6][RVSIP_TRANSPORT_SCTP] -= numIPv6addrs;
            }
            else
            {
                TransportMgrLogLocalAddressWarning(pTransportMgr, pAddress, rv,
                    "LocalAddressClose - No connection for local %s address %s:%d (0x%p) was found. rv=%d");
            }
            pTransportMgr->sctpData.sctpNumOfSctpAddresses--;
            break;  /* END OF  case RVSIP_TRANSPORT_SCTP:*/
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
            
        default:
            TransportMgrLocalAddrUnlock(pAddress);
            return RV_ERROR_BADPARAM;
    }

    TransportMgrLocalAddrUnlock(pAddress);
    return rv;
}

/******************************************************************************
 * LocalAddressRemoveFromList
 * ----------------------------------------------------------------------------
 * General: Remove local address element from the list of local address
 * Return Value: RV_OK on success, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - transport manager
 *          pLocalAddr      - address to be removed
 * Output:
 *****************************************************************************/
static RvStatus RVCALLCONV LocalAddressRemoveFromList(
                                            IN  TransportMgr*         pTransportMgr,
                                            IN  TransportMgrLocalAddr *pAddress)
{
    RLIST_HANDLE    hLocalAddrList = NULL;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE     hLocalAddrHash = NULL;
#endif /* defined(UPDATED_BY_SPIRENT) */
    RvSipTransportAddressType eAddrType;
    RvSipTransport  eTransportType;
    RvStatus rv;

    eTransportType = pAddress->addr.eTransport;
    eAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pAddress->addr.addr));

    /*set the transport*/
    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        hLocalAddrList = pTransportMgr->hLocalUdpAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->hLocalUdpAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
        break;
    case RVSIP_TRANSPORT_TCP:
        hLocalAddrList = pTransportMgr->hLocalTcpAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->hLocalTcpAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        hLocalAddrList = pTransportMgr->tlsMgr.hLocalTlsAddrList;
#if defined(UPDATED_BY_SPIRENT)
        hLocalAddrHash = pTransportMgr->tlsMgr.hLocalTlsAddrHash;
#endif /* defined(UPDATED_BY_SPIRENT) */
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        hLocalAddrList = pTransportMgr->sctpData.hSctpLocalAddrList;
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "LocalAddressRemoveFromList - Failed to remove local address of unsupported type %d",
            pAddress->addr.eTransport));
        return RV_ERROR_BADPARAM;
    }

    TransportMgrLocalAddrListLock(pTransportMgr);

    /* Lock Address after the List had been locked in order to prevent
    a possible Removal from the List by another thread after the current
    removing will be finished. */
    rv = TransportMgrLocalAddrLock(pAddress);
    if (RV_OK != rv)
    {
        TransportMgrLocalAddrListUnlock(pTransportMgr);
        return rv;
    }

    if (RV_TRUE == pAddress->bVacant)
    {
        TransportMgrLocalAddrUnlock(pAddress);
        TransportMgrLocalAddrListUnlock(pTransportMgr);
        return RV_OK;
    }

#if defined(UPDATED_BY_SPIRENT)

    /* Remove from the appropriate hash table */
    if( hLocalAddrHash != 0 )
    {
        void   * pElem = NULL;
        RvBool   rv;

        rv = HASH_FindElement( hLocalAddrHash, pAddress, HashCompare_LocalAddr, &pElem );

        if( rv == RV_TRUE && pElem )
            HASH_RemoveElement( hLocalAddrHash, pElem );
        else
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "LocalAddressRemoveFromList - Failed to remove local address (%s) from hash table",
                pAddress->strLocalAddress));
        }
    }

#endif /* defined(UPDATED_BY_SPIRENT) */


    RLIST_Remove(pTransportMgr->hLocalAddrPool, hLocalAddrList, (RLIST_ITEM_HANDLE)pAddress);

    TransportMgrLogLocalAddressInfo(pTransportMgr, pAddress,
        "LocalAddressRemoveFromList - %s Local Address %s:%d (0x%p) was removed from list");

    memset(&pAddress->addr,0,sizeof(SipTransportAddress));
    memset(&pAddress->ccSelectFd,0,sizeof(RvSelectFd));
    pAddress->pSocket            = NULL;
    pAddress->hConnListening     = NULL;
    pAddress->strLocalAddress[0] = '\0';
    pAddress->bVacant            = RV_TRUE;

	if(pAddress->hSentByPage != NULL_PAGE)
	{
		RPOOL_FreePage(pTransportMgr->hElementPool, pAddress->hSentByPage);
		pAddress->hSentByPage = NULL;
	}
	pAddress->sentByHostOffset = UNDEFINED;
	pAddress->sentByPort = UNDEFINED;

    TransportMgrLocalAddrUnlock(pAddress);

    /* Update Counter of Addresses */
    /* SCTP address counting should take in account mutltihoming IPs,
    because SCTP of mixed (both IPv4 and IPv6) type can send messages
    or establish connections to both IPv4 and IPv6 addresses.
        Counters for SCTP will be updated with call to 
    TransportSctpConnectionLocalAddressAdd(),
    TransportSctpConnectionLocalAddressRemove() and LocalAddressClose().
    */
    if (RVSIP_TRANSPORT_SCTP != eTransportType)
    {
        LocalAddressCounterUpdate(pTransportMgr,eAddrType,eTransportType,-1);
    }

    TransportMgrLocalAddrListUnlock(pTransportMgr);

    return RV_OK;
}

/******************************************************************************
 * TransportMgrLocalAddrListLock
 * ----------------------------------------------------------------------------
 * General: locks supplied List of Local Addresses.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the Transport Manager.
 *            hListOfLocalAddr - handle of the List of Local Addresses
 *****************************************************************************/
void TransportMgrLocalAddrListLock(IN  TransportMgr    *pTransportMgr)
{
    if (RV_FALSE == pTransportMgr->bDLAEnabled)
    {
        return;
    }

    RvMutexLock(&pTransportMgr->lockLocalAddresses,pTransportMgr->pLogMgr);
}

/******************************************************************************
 * TransportMgrLocalAddrListUnlock
 * ----------------------------------------------------------------------------
 * General: unlocks supplied List of Local Addresses.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the Transport Manager.
 *            hListOfLocalAddr - handle of the List of Local Addresses
 *****************************************************************************/
void TransportMgrLocalAddrListUnlock(IN  TransportMgr    *pTransportMgr)
{
    if (RV_FALSE == pTransportMgr->bDLAEnabled)
    {
        return;
    }

    RvMutexUnlock(&pTransportMgr->lockLocalAddresses,pTransportMgr->pLogMgr);
}

/******************************************************************************
 * LocalAddressCounterUpdate
 * ----------------------------------------------------------------------------
 * General: Increase or decrease value of the counter of the requested type.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr     - pointer to the Transport Manager.
 *            eIPAddrType       - type of the IP address.
 *            eTransportType    - type of Transport Protocol.
 *            delta             - value, that will be added to the counter.
 *****************************************************************************/
static void LocalAddressCounterUpdate(
                                    IN  TransportMgr            *pTransportMgr,
                                    IN  RvSipTransportAddressType eIPAddrType,
                                    IN  RvSipTransport          eTransportType,
                                    IN  RvInt32                 delta)
{
    /* Calling function should lock the Transport Manager in order to ensure
    correct update in multithreading applications. */
    if (RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == eIPAddrType ||
        RVSIP_TRANSPORT_UNDEFINED == eTransportType)
    {
        return;
    }
    /*lint -e676*/
    pTransportMgr->localAddrCounters[eIPAddrType][eTransportType] += delta;

    switch (eTransportType)
    {
        case RVSIP_TRANSPORT_UDP: pTransportMgr->numOfUdpAddresses+=delta; break;
        case RVSIP_TRANSPORT_TCP: pTransportMgr->numOfTcpAddresses+=delta; break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS: pTransportMgr->tlsMgr.numOfTlsAddresses+=delta; break;
#endif /* #ifdef  (RV_TLS_TYPE != RV_TLS_NONE) */
        default: break;
    }
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * TransportMgrLogLocalAddressError
 * ----------------------------------------------------------------------------
 * General: wrapper for RvLogError(), used for output of Local Address related
 *          errors. Encapsulates formatting of Local Address details to be
 *          printed.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr     - pointer to the Transport Manager.
 *            pLocalAddr        - pointer to the Local Address.
 *            strLogLine        - formatted log line, to be printed.
 *****************************************************************************/
void TransportMgrLogLocalAddressError(IN TransportMgr *pTransportMgr,
                                 IN TransportMgrLocalAddr *pLocalAddr,
                                 IN RvInt32 errCode,
                                 IN RvChar  *strLogLine)
{
    RvUint16    port;
    RvChar      strIP[RVSIP_TRANSPORT_LEN_STRING_IP];

    RvAddressGetString(&pLocalAddr->addr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strIP);
    port = RvAddressGetIpPort(&pLocalAddr->addr.addr);

    RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc, strLogLine,
               SipCommonConvertEnumToStrTransportType(pLocalAddr->addr.eTransport),
               strIP, port, pLocalAddr, errCode));
    return;
}

/******************************************************************************
 * TransportMgrLogLocalAddressWarning
 * ----------------------------------------------------------------------------
 * General: wrapper for RvLogWarning(), used for output of Local Address related
 *          errors. Encapsulates formatting of Local Address details to be
 *          printed.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr     - pointer to the Transport Manager.
 *            pLocalAddr        - pointer to the Local Address.
 *            strLogLine        - formatted log line, to be printed.
 *****************************************************************************/
void TransportMgrLogLocalAddressWarning(IN TransportMgr *pTransportMgr,
                                 IN TransportMgrLocalAddr *pLocalAddr,
                                 IN RvInt32 errCode,
                                 IN RvChar  *strLogLine)
{
    RvUint16    port;
    RvChar      strIP[RVSIP_TRANSPORT_LEN_STRING_IP];

    RvAddressGetString(&pLocalAddr->addr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strIP);
    port = RvAddressGetIpPort(&pLocalAddr->addr.addr);

    RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc, strLogLine,
               SipCommonConvertEnumToStrTransportType(pLocalAddr->addr.eTransport),
               strIP, port, pLocalAddr, errCode));
    return;
}

/******************************************************************************
 * TransportMgrLogLocalAddressInfo
 * ----------------------------------------------------------------------------
 * General: wrapper for RvLogInfo(), used for output of Local Address related
 *          info. Encapsulates formatting of Local Address details to be
 *          printed.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr     - pointer to the Transport Manager.
 *            pLocalAddr        - pointer to the Local Address.
 *            strLogLine        - formatted log line, to be printed.
 *****************************************************************************/
void TransportMgrLogLocalAddressInfo(    IN TransportMgr *pTransportMgr,
                                    IN TransportMgrLocalAddr *pLocalAddr,
                                    IN RvChar *strLogLine)
{
    RvUint16    port;
    RvChar      strIP[RVSIP_TRANSPORT_LEN_STRING_IP];

    RvAddressGetString(&pLocalAddr->addr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strIP);
    port = RvAddressGetIpPort(&pLocalAddr->addr.addr);

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc, strLogLine,
        SipCommonConvertEnumToStrTransportType(pLocalAddr->addr.eTransport),
        strIP, port, pLocalAddr));
    return;
}
#endif /* END OF: #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

/******************************************************************************
 * InitStructTransportAddr
 * ----------------------------------------------------------------------------
 * General: utility function. Set data into the pointed struct
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eTransportType    - Transport Type to be set in the struct.
 *            eAddrType         - Address Type to be set in the struct.
 *            strIP             - IP in the string form to be set in the struct.
 *            port              - port to be set in the struct.
 * Output:    pTransportAddrStruct - pointer to the struct to be initiated.
 *****************************************************************************/
static void InitStructTransportAddr (
                                IN RvSipTransport       eTransportType,
                                IN RvSipTransportAddressType eAddrType,
                                IN RvChar               *strIP,
                                IN RvUint16             port,
#if defined(UPDATED_BY_SPIRENT)
				IN RvChar              *if_name,
				IN RvUint16             vdevblock,
#endif
                                OUT RvSipTransportAddr  *pTransportAddrStruct)
{
    RvUint32 lenOfStrIP = (RvUint32)strlen(strIP);
#if defined(UPDATED_BY_SPIRENT)
    RvUint32 lenOfIfName = if_name ? (RvUint32)strlen(if_name) : 0;
#endif
    memset(pTransportAddrStruct,0,sizeof(RvSipTransportAddr));
    pTransportAddrStruct->eTransportType = eTransportType;
    pTransportAddrStruct->eAddrType      = eAddrType;
    pTransportAddrStruct->port           = port;
    pTransportAddrStruct->Ipv6Scope      = UNDEFINED;
    memcpy(pTransportAddrStruct->strIP,strIP,RvMin(lenOfStrIP,RVSIP_TRANSPORT_LEN_STRING_IP));
#if defined(UPDATED_BY_SPIRENT)
    if(if_name) memcpy(pTransportAddrStruct->if_name,if_name,RvMin(lenOfIfName,RVSIP_TRANSPORT_IFNAME_SIZE));
    pTransportAddrStruct->vdevblock=vdevblock;
#endif

    if (RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED==pTransportAddrStruct->eAddrType)
    {
        pTransportAddrStruct->eAddrType = (NULL == strstr(strIP,":")) ?
              RVSIP_TRANSPORT_ADDRESS_TYPE_IP:RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
    }

    return;
}

#if (RV_NET_TYPE & RV_NET_IPV6)
/******************************************************************************
* ConvertIpStringTransport2CommonCore
* ----------------------------------------------------------------------------
* General: SIP Stack enables the application to use strings for IP
*          presentations, that contains square brackets and / or scope
*          identification. Common Core uses "pure" IP strings only.
*          This function removes the square brackets and scope id from string,
*          which represents IP, and stores the result in the output buffer.
*
* Return Value: none.
* ----------------------------------------------------------------------------
* Arguments:
* Input:     strTransportIp - IP string in SIP Stack Transport module format
* Output:    strCcIp        - IP string in Common Core format
*            pScopeId       - Scope Id, if was found in the Transport string
*****************************************************************************/
static void RVCALLCONV ConvertIpStringTransport2CommonCore(
                                                           IN  RvChar*   strTransportIp,
                                                           OUT RvChar*   strCcIp,
                                                           OUT RvInt32*  pScopeId)
{
#if !(RV_NET_TYPE & RV_NET_IPV6)
    /* If IPv6 is not supported, don't seek for square brackets and Scope Id */
    strcpy(strCcIp, strTransportIp);
#else
    RvChar*  pClosingBracketLocation;
    RvChar*  pScopeIdLocation;
    RvUint32 lenStrCcIp;
    
    pClosingBracketLocation = strchr(strTransportIp, ']');
    pScopeIdLocation = strchr(strTransportIp, '%');
    
    /* If neither bracket nor scope id exist in string - copy whole string and return */
    if (NULL==pClosingBracketLocation  && NULL==pScopeIdLocation)
    {
        strcpy(strCcIp, strTransportIp);
        return;
    }
    
    /* If brackets exist, copy their content */
    if (NULL != pClosingBracketLocation)
    {
        lenStrCcIp = (RvUint32)(pClosingBracketLocation-(strTransportIp+1));
        memcpy(strCcIp, strTransportIp+1, lenStrCcIp);
    }
    else
        /* If brackets don't exist, but Scope Id exist copy string till scope id*/
    {
        lenStrCcIp = (RvUint32)(pScopeIdLocation-strTransportIp);
        memcpy(strCcIp, strTransportIp, lenStrCcIp);
    }
    strCcIp[lenStrCcIp] = '\0';
    
    /* Convert Scope Id into the integer, if exists */
    if (NULL != pScopeIdLocation)
    {
        *pScopeId = atoi(pScopeIdLocation+1);
    }
#endif
    RV_UNUSED_ARG(pScopeId)
}
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/

#if defined(UPDATED_BY_SPIRENT)

/******************************************************************************
 * HashFunc_LocalAddr
 * ----------------------------------------------------------------------------
 * General: IP Info specific hash function calculation. 
 *          Pretty fast and good randomization.
 *
 * Return Value: Hash value.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pParam            - ptr to Transport Mgr Local Address  
 * Output:    none
 *****************************************************************************/
static RvUint HashFunc_LocalAddr( IN void *pParam )
{
    RvInt    i;
    RvUint   hash;
    RvInt    szHash;
    TransportMgrLocalAddr * pLA   = (TransportMgrLocalAddr *)pParam;
    SipTransportAddress   * pAddr = &pLA->addr;
    RvChar                * sAddr;

    if( pAddr->addr.addrtype ==  RV_ADDRESS_TYPE_IPV4 )
    {
        szHash = sizeof(pAddr->addr.data.ipv4);
        sAddr  = (RvChar *)&pAddr->addr.data.ipv4;
    }
    else
    {
        szHash = sizeof(pAddr->addr.data.ipv6);
        sAddr  = (RvChar *)&pAddr->addr.data.ipv6;
    }

    for( hash = 0, i = 0; i < szHash; i++ )
    {
       hash += sAddr[i];
       hash += (hash << 10);
       hash ^= (hash >> 6);
    }

    sAddr = pLA->if_name;

    for( i = 0; sAddr[i]; i++ )
    {
       hash += sAddr[i];
       hash += (hash << 10);
       hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash%HASH_SIZE;
}

/******************************************************************************
 * HashCompare_LocalAddr
 * ----------------------------------------------------------------------------
 * General: IP info comparator
 *
 * Return Value: RV_TRUE  - IP infos are equal
 *               RV_FALSE - IP infos are not equal
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pParam1       - ptr to Transport Mgr Local Address #1
 *            pParam1       - ptr to Transport Mgr Local Address #2
 * Output:    none
 *****************************************************************************/
static RvBool HashCompare_LocalAddr (IN void *pParam1 , IN void *pParam2 )
{
    TransportMgrLocalAddr * pLA1 = (TransportMgrLocalAddr *)pParam1;
    TransportMgrLocalAddr * pLA2 = (TransportMgrLocalAddr *)pParam2;

    if(strcmp(pLA1->if_name,pLA2->if_name)!=0) return RV_FALSE;

    return (pLA1 && pLA2 && TransportMgrSipAddressIsEqual(&pLA1->addr, &pLA2->addr)) ?
        RV_TRUE : RV_FALSE;
}

#endif /* #if defined(UPDATED_BY_SPIRENT) */

#ifdef __cplusplus
}
#endif

