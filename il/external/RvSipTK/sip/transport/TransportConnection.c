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
 *                              TransportConnection.c
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
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"

#include "TransportConnection.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTPTypes.h"
#include "TransportSCTP.h"
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */
#include "_SipTransport.h"
#include "TransportCallbacks.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"
#include "TransportMgrObject.h"
#include "RvSipViaHeader.h"
#include "TransportMsgBuilder.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus RVCALLCONV RemoveAllConnectionOwners(
                                     IN RvSipTransportConnectionHandle hConn);

static RvStatus RVCALLCONV RemoveAllPendingMessagesOnConnection(
                                     IN RvSipTransportConnectionHandle hConn);

static RvStatus RVCALLCONV NotifyConnUnsentMessagesStatus(
                                    IN TransportConnection *pConn);

static RvStatus  RVCALLCONV TerminateConnectionEventHandler(IN void    *pConnObj,
                                                            IN RvInt32  reason);

static RvStatus  RVCALLCONV ChangeConnStateEventHandler(IN void    *pConnObj,
                                                        IN RvInt32 state,
                                                        IN RvInt32 notUsed,
                                                        IN RvInt32 objUniqueIdentifier);

static RvStatus  RVCALLCONV NotifyConnStatusEventHandler(IN void    *pConnObj,
                                                         IN RvInt32 status,
                                                         IN RvInt32 notUsed,
                                                         IN RvInt32 objUniqueIdentifier);
static void RVCALLCONV MarkOwnersAsNotNotified(
                                           IN TransportConnection* pConn);

static RvBool ServerConnectionTimerExpiredEvHandler(
                                        IN void    *pContext,
                                        IN RvTimer *timerInfo);

static void NotifyConnectionStateChange(
                    IN TransportConnection*                       pConn,
                    IN RvSipTransportConnectionState              eConnState,
                    IN RvSipTransportConnectionStateChangedReason eConnReason);

static void NotifyConnectionStateChangeToOwner(
        IN TransportConnection*                       pConn,
        IN RvSipTransportConnectionStateChangedEv     pfnStateChangeEvHandler,
        IN RvSipTransportConnectionOwnerHandle        hConnOwner,
        IN RvSipTransportConnectionState              eConnState,
        IN RvSipTransportConnectionStateChangedReason eConnReason,
        IN RvChar*                                    strCallbackName);

static void NotifyConnectionStatus(
                    IN TransportConnection*           pConn,
                    IN RvSipTransportConnectionStatus eConnStatus,
                    IN void*                          pInfo);

static void NotifyConnectionStatusToOwner(
                    IN TransportConnection*                 pConn,
                    IN RvSipTransportConnectionStatusEv     pfnStausEvHandler,
                    IN RvSipTransportConnectionOwnerHandle  hConnOwner,
                    IN RvSipTransportConnectionStatus       eConnStatus,
                    IN void*                                pInfo,
                    IN RvChar*                              strCallbackName);

static RvStatus GetConnectionAliasFromVia(TransportConnection* pConn,
                                          RvSipViaHeaderHandle hVia);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus RVCALLCONV ConnectionLock(IN TransportConnection *pConn,
                                              IN TransportMgr  *pMgr,
                                              IN SipTripleLock *pTripleLock,
                                              IN RvInt32        identifier);

static void RVCALLCONV ConnectionUnLock(IN  RvLogMgr   *pLogMgr,
                                        IN  RvMutex       *hLock);
#else
#define ConnectionLock(a,b,c,d) (RV_OK)
#define ConnectionUnLock(a,b)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/*--------------------OWNERS HASH FUNCTIONS----------------------*/


static RvUint  OwnersHashFunction(IN void *key);
static RvBool OwnersHashCompare(IN void *newHashElement,
                                 IN void *oldHashElement);
static RvStatus RVCALLCONV OwnersHashInsert(
                                 IN TransportConnection*                 pConn,
                                 IN TransportConnectionOwnerInfo*        pOwner);
static void RVCALLCONV OwnersHashFind(
                                 IN TransportConnection                 *pConn,
                                 IN RvSipTransportConnectionOwnerHandle hOwner,
                                 OUT TransportConnectionOwnerInfo       **ppOwner);

/*--------------------CONNECTION HASH FUNCTIONS----------------------*/

static RvUint  ConnectionHashFunction(
                                     IN void *key);

static void RVCALLCONV ConnectionHashFind(
                                     IN TransportMgr                *pTransportMgr,
                                     IN RvSipTransport               eTransport,
                                     IN RvSipTransportLocalAddrHandle  hLocalAddress,
                                     IN SipTransportAddress          *pDestAddress,
                                     IN RPOOL_Ptr                      *pAlias,
                                    OUT RvSipTransportConnectionHandle *phConn);

static RvStatus ConnectionSelectEventAdd(IN TransportConnection* pConn);

static RvStatus ConnectionSelectEventUpdate(IN TransportConnection* pConn);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransportConnectionListConstruct
 * ------------------------------------------------------------------------
 * General: Allocated the list of Connections managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr   - Handle to the transport manager
 *            maxNumOfConn - Max number of Connections to allocate
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionListConstruct(
                                        IN TransportMgr      *pTransportMgr,
                                        IN RvInt32             maxNumOfConn)
{
    RvInt32  i;
    TransportConnection* pConn;
    RvInt32  numOfUserElements;
    RvStatus rv;
    /*allocate a pool with 1 list*/
    pTransportMgr->hConnPool = RLIST_PoolListConstruct(
                                        maxNumOfConn,
                                        1,
                                        sizeof(TransportConnection),
                                        pTransportMgr->pLogMgr ,
                                        "Connections List");

    if(pTransportMgr->hConnPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pTransportMgr->hConnList = RLIST_ListConstruct(pTransportMgr->hConnPool);
    if(pTransportMgr->hConnList == NULL)
    {
        RLIST_PoolListDestruct(pTransportMgr->hConnPool);
        pTransportMgr->hConnPool = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

    for (i=0; i < maxNumOfConn; i++)
    {
        RLIST_InsertTail(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE *)&pConn);
        pConn->pTransportMgr = pTransportMgr;
        rv = SipCommonConstructTripleLock(
                            &pConn->connTripleLock, pTransportMgr->pLogMgr);
        if (RV_OK != rv)
        {
            RLIST_PoolListDestruct(pTransportMgr->hConnPool);
            pTransportMgr->hConnPool = NULL;
            pTransportMgr->hConnList = NULL;
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportConnectionListConstruct - Failed to construct Connection Triple Lock for the %d-th connection(rv=%d:%s)",
                i, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        pConn->pTripleLock = NULL;
    }

    /* initiate locks of all connections */
    for (i=0; i < maxNumOfConn; i++)
    {
        RLIST_GetHead(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE *)&pConn);
        RLIST_Remove(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE)pConn);
    }
    RLIST_ResetMaxUsageResourcesStatus(pTransportMgr->hConnPool);

#if (RV_NET_TYPE & RV_NET_SCTP)
    numOfUserElements = pTransportMgr->maxConnections * RVSIP_TRANSPORT_SCTP_MULTIHOMED_ADDR_NUM_AVG_PER_CONN;
#else
    numOfUserElements = pTransportMgr->maxConnections;
#endif
    pTransportMgr->connHashSize = HASH_DEFAULT_TABLE_SIZE(numOfUserElements);
    pTransportMgr->hConnHash = HASH_Construct(
                                    HASH_DEFAULT_TABLE_SIZE(numOfUserElements),
                                    numOfUserElements,
                                    ConnectionHashFunction,
                                    sizeof(TransportConnection*),
                                    sizeof(TransportConnectionHashKey),
                                    pTransportMgr->pLogMgr,
                                    "Transport Connection Hash");
    if(pTransportMgr->hConnHash == NULL)
    {
        RLIST_PoolListDestruct(pTransportMgr->hConnPool);
        pTransportMgr->hConnPool = NULL;
        pTransportMgr->hConnList = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionListConstruct - Failed to construct Connection hash for %d entries",
            (pTransportMgr->connHashSize-1)/2));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pTransportMgr->connOwnersHashSize = HASH_DEFAULT_TABLE_SIZE((pTransportMgr->maxConnOwners*2));
    pTransportMgr->hConnOwnersHash = HASH_Construct(
                       HASH_DEFAULT_TABLE_SIZE((pTransportMgr->maxConnOwners*2)),
                       pTransportMgr->maxConnOwners*2,
                       OwnersHashFunction,
                       sizeof(TransportConnectionOwnerInfo*),
                       sizeof(ConnectionOwnerHashKey),
                       pTransportMgr->pLogMgr,
                       "Connection Owners Hash");
    if(pTransportMgr->hConnOwnersHash == NULL)
    {
        RLIST_PoolListDestruct(pTransportMgr->hConnPool);
        pTransportMgr->hConnPool = NULL;
        pTransportMgr->hConnList = NULL;
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionListConstruct - Failed to construct Connection hash"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}


/***************************************************************************
 * TransportConnectionListDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the list of connections. All connections that are not
 *          closed are closed.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the transport manager
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionListDestruct(
                                        IN TransportMgr      *pTransportMgr)
{
    RLIST_ITEM_HANDLE listElement;
    RLIST_ITEM_HANDLE listPrevElement;
    TransportConnection* pConn;
    RvInt32    i;
    RvStatus            status;

    RLIST_GetTail(pTransportMgr->hConnPool,
                pTransportMgr->hConnList,
                &listElement);

    while (listElement !=NULL) 
    {
        pConn = (TransportConnection*)listElement;

        RLIST_GetPrev(pTransportMgr->hConnPool,
                      pTransportMgr->hConnList,
                      listElement,
                      &listPrevElement);

        listElement = listPrevElement;


        /*need to remove all owners*/
        status = RemoveAllConnectionOwners((RvSipTransportConnectionHandle)pConn);
        if (status!=RV_OK)
        {
            RvLogError( pTransportMgr->pLogSrc,( pTransportMgr->pLogSrc,
                       "TransportConnectionListDestruct- Failed to clear connection owners list"));

        }
        /*if the connection is in state closing it means that we destructed it
         already and its socket has been shutdown but not get the close event ,
         so now we just have to close it*/
        pConn->eState = RVSIP_TRANSPORT_CONN_STATE_CLOSING;
#if defined(UPDATED_BY_SPIRENT)        
        if(pTransportMgr->ePersistencyLevel == RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER && pConn->eTransport == RVSIP_TRANSPORT_TLS)
           TransportConnectionClose(pConn,RV_FALSE);
        else
           TransportConnectionClose(pConn,RV_TRUE);
#else
        TransportConnectionClose(pConn,RV_TRUE);
#endif        
        
    }

    for (i=0; i < pTransportMgr->maxConnections; i++)
    {
        RLIST_GetHead(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE *)&pConn);
        if (pConn == NULL)
            break;
        RLIST_Remove(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE)pConn);
    }

    for (i=0; i < pTransportMgr->maxConnections; i++)
    {
        RLIST_InsertTail(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE *)&pConn);
    }

    for (i=0; i < pTransportMgr->maxConnections; i++)
    {
        RLIST_GetHead(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE *)&pConn);
        SipCommonDestructTripleLock(
            &pConn->connTripleLock, pTransportMgr->pLogMgr);
        RLIST_Remove(pTransportMgr->hConnPool,pTransportMgr->hConnList,(RLIST_ITEM_HANDLE)pConn);
    }

    RLIST_ListDestruct(pTransportMgr->hConnPool,
                      pTransportMgr->hConnList );


    RLIST_PoolListDestruct(pTransportMgr->hConnPool);
    return RV_OK;
}


/************************************************************************************
 * TransportConnectionAllocate
 * ----------------------------------------------------------------------------------
 * General: Allocates a empty new connection object.
 *          The connection parameters are set to undefined.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr       - Pointer to transport instance
 * Output:  hConnection            - free connection to use
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionAllocate(
                     IN  TransportMgr                     *pTransportMgr,
                     OUT RvSipTransportConnectionHandle   *phConn)
{
    RvStatus             rv;
    TransportConnection      *pConn;
    RLIST_ITEM_HANDLE     hListItem;

#ifdef SIP_DEBUG
    if(pTransportMgr == NULL || phConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif
    if(pTransportMgr->hConnPool == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportConnectionAllocate - Failed to create a new connection-connection list is not defined."));
        return RV_ERROR_UNKNOWN;
    }

    if(0 < pTransportMgr->connectionCapacityPercent)
    {
        TransportMgrCheckConnCapacity(pTransportMgr);
    }

    RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);

    rv = RLIST_InsertTail(pTransportMgr->hConnPool,
                           pTransportMgr->hConnList,&hListItem);

    if(rv != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportConnectionAllocate - Failed to create a new connection (rv=%d)",rv));
        RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        return rv;
    }

    pConn = (TransportConnection*)hListItem;
    memset(&(pConn->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));

    pConn->pTransportMgr   = pTransportMgr;
    pConn->isDeleted          = RV_FALSE;
    pConn->usageCounter          = 0;
    pConn->eConnectionType    = RVSIP_TRANSPORT_CONN_TYPE_UNDEFINED;
    pConn->bufSize          = 0;
    pConn->pBuf             = NULL;
    pConn->originalBuffer   = NULL;
    pConn->msgStart         = 0;
    pConn->msgSize          = 0;
    pConn->bytesReadFromBuf = 0;
    pConn->msgComplete      = 0;
    pConn->eTransport       = RVSIP_TRANSPORT_UNDEFINED;
    pConn->numOfOwners      = 0;
    pConn->pTripleLock        = NULL;
    pConn->eState           = RVSIP_TRANSPORT_CONN_STATE_IDLE;
    pConn->bConnErr         = RV_FALSE;
    SipCommonCoreRvTimerInit(&pConn->hServerTimer);
    RvRandomGeneratorGetInRange(pTransportMgr->seed,RV_INT32_MAX,(RvRandom*)&pConn->connUniqueIdentifier);
    pConn->hAppHandle              = NULL;
    pConn->eTcpReason              = RVSIP_TRANSPORT_CONN_REASON_UNDEFINED;
    pConn->bWriteEventInQueue      = RV_FALSE;
    pConn->bCloseEventInQueue      = RV_FALSE;
    pConn->bClosedByLocal          = RV_FALSE;
	pConn->bDisconnectionTimer     = RV_FALSE;

    /* cc */
    pConn->pSocket              = NULL;
    pConn->ccEvent              = 0;
    pConn->oorEvents            = 0;
    pConn->oorListLocation      = NULL;
    memset(&pConn->ccSelectFd,0,sizeof(pConn->ccSelectFd));
    pConn->bFdAllocated         = RV_FALSE;
    pConn->oorConnectIsError    = RV_FALSE;
    /* end cc */
    SipCommonCoreRvTimerInit(&pConn->saftyTimer);

#if (RV_TLS_TYPE != RV_TLS_NONE)
    pConn->pTlsEngine               = NULL;
    pConn->pTlsSession              = NULL;
    pConn->eTlsHandShakeState       = TRANSPORT_TLS_HANDSHAKE_IDLE;
    pConn->bConnectionAsserted      = RV_FALSE;
    pConn->strSipAddrOffset         = UNDEFINED;
    pConn->pfnVerifyCertEvHandler   = NULL;
    pConn->eTlsState                = RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED;
    pConn->tlsShutdownSafeCounter   = 0;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    pConn->hConnPage                = NULL;
    pConn->hLocalAddr       = NULL;

    /*initiate recv info*/
    pConn->recvInfo.bReadingBody     = RV_FALSE;
    pConn->recvInfo.contentLength    = UNDEFINED;
    pConn->recvInfo.lastHeaderOffset = 0;
    pConn->recvInfo.hMsgReceived     = NULL_PAGE;
    pConn->recvInfo.msgSize          = 0;
    pConn->recvInfo.offsetInBody     = 0;
    pConn->recvInfo.bodyStartIndex   = 0;
#ifdef RV_SIGCOMP_ON
    pConn->recvInfo.pDecompressedBuf       = NULL;
    pConn->recvInfo.decompressedBufSize    = 0;
    pConn->recvInfo.eMsgCompType           = RVSIP_COMP_UNDEFINED;
    pConn->recvInfo.sigCompQuotedBytesLeft = 0;
    pConn->recvInfo.bSigCompDelimiterRcvd  = RV_FALSE;
#endif
    pConn->pHashElement              = NULL;
    pConn->pTripleLock               = &pConn->connTripleLock;

    pConn->pHashAliasElement = NULL;
    pConn->pAlias.hPage  = NULL;
    pConn->pAlias.hPool  = NULL;
    pConn->pAlias.offset = UNDEFINED;
    pConn->hConnNoOwnerListItem = NULL;
	
#ifdef RV_SIP_IMS_ON    
    pConn->pSecOwner = NULL;
    pConn->impi      = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/


    /*construct the list of the specific connection, to hold outgoing messages.*/
    pConn->hMsgToSendList = RLIST_ListConstruct(pConn->pTransportMgr->hMsgToSendPoolList);
    if(pConn->hMsgToSendList == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportConnectionAllocate- hConnList Failed to construct outgoing msg list"));

        TransportConnectionFree(pConn);
        RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*construct the list holds the owner of this connection*/
    pConn->hConnectionOwnersList = RLIST_ListConstruct(pConn->pTransportMgr->hConnectionOwnersPoolList);

    if(pConn->hConnectionOwnersList == NULL)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "TransportConnectionAllocate- Failed to construct connection owners list, removing connection"));
        TransportConnectionFree(pConn);
        RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    rv = TransportSctpConnectionConstruct(pConn);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionAllocate - Failed to construct SCTP data in connection (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        TransportConnectionFree(pConn);
        RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        return rv;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    *phConn = (RvSipTransportConnectionHandle)pConn;

    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportConnectionAllocate - Connection 0x%p was constructed successfully",pConn));

    RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
    return RV_OK;

}

/************************************************************************************
 * TransportConnectionInit
 * ----------------------------------------------------------------------------------
 * General: Initialized a connection and insert it to the hash if its type is client
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - connection pointer
 *          type        - connection type - client/server
 *          eTransport  - tcp/udp/sctp
 *          hLocalAddress - local address to be used by the connection.
 *          pDestAddress - connection dest address.
 *          bInsertToHash - indicates whether to insert the connection to the hash
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionInit(
                     IN TransportConnection*              pConn,
                     IN RvSipTransportConnectionType      type,
                     IN RvSipTransport                    eTransport,
                     IN RvSipTransportLocalAddrHandle     hLocalAddress,
                     IN SipTransportAddress*              pDestAddress,
                     IN RvBool                            bInsertToHash)

{
	RvStatus retStatus = RV_OK;
    /*initialize the connection*/
    pConn->eConnectionType = type;
    pConn->eTransport      = eTransport;
    pConn->localPort       = 0;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_TRANSPORT_TLS ==pConn->eTransport)
    {
        if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT == pConn->eConnectionType)
        {
            pConn->eTlsHandshakeSide = RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_CLIENT;
        }
        else
        {
            pConn->eTlsHandshakeSide = RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_SERVER;
        }
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

    /*for a received client connection there is no destination address at this stage*/
    if(pDestAddress != NULL)
    {
        pConn->destAddress = *pDestAddress;
    }
    pConn->hLocalAddr = hLocalAddress;
    pConn->typeOfService = TRANSPORT_CONNECTION_TOS_UNDEFINED;

#if (RV_NET_TYPE & RV_NET_SCTP)
    if (RVSIP_TRANSPORT_SCTP == pConn->eTransport)
    {
        RvStatus rv;
        rv = TransportSctpConnectionInit(pConn);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionInit - Failed to init SCTP data (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    /*only client connections are inserted to the hash*/
    if (bInsertToHash==RV_TRUE && type == RVSIP_TRANSPORT_CONN_TYPE_CLIENT &&
        pDestAddress != NULL)
    {
        TransportConnectionHashInsert((RvSipTransportConnectionHandle)pConn, RV_FALSE);
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)    /*print function input to log*/
    {
        RvStatus  rv;
        RvChar    destIp[RVSIP_TRANSPORT_LEN_STRING_IP];
        RvChar    localIp[RVSIP_TRANSPORT_LEN_STRING_IP];
#if defined(UPDATED_BY_SPIRENT)
        RvChar    if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
	RvUint16  vdevblock = 0;
#endif  
	RvUint    destPort = 0;
        RvUint16  port = 0;

        rv = SipTransportLocalAddressGetAddressByHandle(
							(RvSipTransportMgrHandle)pConn->pTransportMgr,
							hLocalAddress,RV_TRUE,RV_TRUE,localIp,&port
#if defined(UPDATED_BY_SPIRENT)
							,if_name
							,&vdevblock
#endif
							);
        if(RV_OK != rv)
        {
            RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionInit - Failed to get address info for local address 0x%p (rv=%d)",
                hLocalAddress,rv));
        }
        if (RV_OK == rv)
        {
            if(pDestAddress != NULL)
            {
                RvAddressGetString(&pDestAddress->addr,RVSIP_TRANSPORT_LEN_STRING_IP,destIp);
                destPort = RvAddressGetIpPort(&pDestAddress->addr);
            }
            else
            {
                strcpy(destIp,"Unspecified");
            }
            if(type == RVSIP_TRANSPORT_CONN_TYPE_CLIENT)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                           "TransportConnectionInit - initialized a new CLIENT Connection 0x%p - local address=%s:%d,Remote Address=%s:%d",
                           pConn,localIp,port,destIp,destPort));
            }
            else
            if(type == RVSIP_TRANSPORT_CONN_TYPE_SERVER)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                          "TransportConnectionInit - initialized a new SERVER Connection 0x%p - local address=%s:%d, remote address=%s:%d",
                           pConn,localIp,port,destIp,destPort));
            }
            else
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionInit - initialized a new MULTI-SERVER Connection 0x%p - local address=%s:%d, remote address=%s:%d",
                    pConn,localIp,port,destIp,destPort));
            }
        }
    }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    if(type == RVSIP_TRANSPORT_CONN_TYPE_CLIENT)
    {
        retStatus = TransportConnectionChangeState(pConn,RVSIP_TRANSPORT_CONN_STATE_READY,
                                       RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                                       RV_TRUE,RV_TRUE);
    }
    /* Don't report READY state for incoming connection object, because
    the connection is constructed before call to ConnectionCreated callback
    to Application (see TcpHandleAcceptEvent()). */
    else if (RVSIP_TRANSPORT_CONN_TYPE_SERVER != type &&
             RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER != type)
    {   /*server is always constructed from the queue*/
        retStatus = TransportConnectionChangeState(pConn,RVSIP_TRANSPORT_CONN_STATE_READY,
                                   RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,RV_FALSE,RV_TRUE);
    }

    return retStatus;
}

/******************************************************************************
 * TransportConnectionConstruct
 * ----------------------------------------------------------------------------
 * General: Allocate and initialize a connection object. This function is never
 *          called from the application.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr       - Pointer to transport instance
 *          bForceCreation      - indicates whether to look for such a connection in
 *                                the hash or to force the creation of a new connection.
 *          type                - client or server
 *          hLocalAddress       - connection local address
 *          pDestAddress        - The destination address
 *          hOwner              - The connection owner.
 *          pfnConnStateChangeHandler - Function pointer to notify connection state changed
 *          pfnConnStatusEvHandler - function pointer to notify of connection status.
 *          pbNewConnCreated    - Indicates whether the connection that was returned is
 *                                a new connection that was just created.
 * Output:  hConnection         - free connection to use
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionConstruct(
          IN  TransportMgr                           *pTransportMgr,
          IN  RvBool                                 bForceCreation,
          IN  RvSipTransportConnectionType           type,
          IN  RvSipTransport                         eTransport,
          IN  RvSipTransportLocalAddrHandle          hLocalAddress,
          IN  SipTransportAddress                    *pDestAddress,
          IN  RvSipTransportConnectionOwnerHandle    hOwner,
          IN  RvSipTransportConnectionStateChangedEv pfnConnStateChangeHandler,
          IN  RvSipTransportConnectionStatusEv       pfnConnStatusEvHandler,
          OUT RvBool                                 *pbNewConnCreated,
          OUT RvSipTransportConnectionHandle         *phConn)
{
    RvStatus                        status        = RV_OK;
    TransportConnection            *pConn         = NULL;
    RvSipTransportConnectionHandle  hConnFromHash = NULL;
    RvInt32                         curIdentifier = 0;

    if (NULL != pbNewConnCreated)
    {
        *pbNewConnCreated = RV_FALSE;
    }

    /* In case of SCTP force persistency */
#if (RV_NET_TYPE & RV_NET_SCTP)
    if (RVSIP_TRANSPORT_SCTP == eTransport)
    {
        bForceCreation = RV_FALSE;
    }
#endif

    if (phConn == NULL || hLocalAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phConn = NULL;

    /*look for client connections in the hash*/
    if(type == RVSIP_TRANSPORT_CONN_TYPE_CLIENT && bForceCreation == RV_FALSE &&
       NULL != pDestAddress)
    {
        /* Locking the connection pool in order to look for the connection and
           get its identifier */
        RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        ConnectionHashFind(pTransportMgr,eTransport,hLocalAddress,pDestAddress, NULL, &hConnFromHash);
        /*first lock the connection and then check the connection state again*/
        if(hConnFromHash != NULL)
        {
            pConn         = (TransportConnection*)hConnFromHash;
            curIdentifier = pConn->connUniqueIdentifier;

            RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);

            status = TransportConnectionLockAPI(pConn);
            if(status != RV_OK)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                           "TransportConnectionConstruct - Connection 0x%p found in hash but failed to be locked. setting pConn to NULL",pConn));
                pConn = NULL;
            }

            /*the connection was destructed and is out of the hash and out of
            the queue but it was constructed again so the locking succeed
            on the wrong object*/
            if (pConn != NULL && pConn->connUniqueIdentifier != curIdentifier)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionConstruct - Connection 0x%p found in hash but its current identifier is mismatched",
                    pConn));

                TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
                pConn = NULL;
            }
            /*the connection is locked - check if the state was not changed*/
            if(pConn         != NULL &&
               pConn->eState != RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED &&
               pConn->eState != RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED &&
               pConn->eState != RVSIP_TRANSPORT_CONN_STATE_CONNECTING &&
#if (RV_TLS_TYPE != RV_TLS_NONE)
               pConn->eTlsState != RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED &&
               pConn->eTlsState != RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED &&
               pConn->eTlsState != RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY &&
               pConn->eTlsState != RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED &&
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
               pConn->eState != RVSIP_TRANSPORT_CONN_STATE_READY)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionConstruct - Connection 0x%p found in hash but has an invalid state %s",
                    pConn,SipCommonConvertEnumToStrConnectionState(pConn->eState)));

                TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
                pConn = NULL;
            }
            /* If NETWORK ERROR occur on connection,don't use the connection */
            if (NULL != pConn && RV_TRUE == pConn->bConnErr)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionConstruct - Connection 0x%p found in hash but can't be used since it got NETWORK ERROR. Connection closure was initiated",
                    pConn));
                TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
                pConn = NULL;
            }
			if ((NULL != pConn) && (RV_TRUE == pConn->bDisconnectionTimer))
			{
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionConstruct - Connection 0x%p already started disconnection timer",
                    pConn));

                TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
                pConn = NULL;
			}
            if (pConn != NULL)
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionConstruct - Connection 0x%p found in hash and will be used",pConn));
                *phConn = hConnFromHash;
            }
        }
        else
        {
            RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
        }
    }
    /*connection was not in the hash or this is a server connection*/
    if(*phConn == NULL)
    {
        RvBool bInsertConnToHash = !bForceCreation; /*if we force creation we don't insert to hash*/

        status = TransportConnectionAllocate(pTransportMgr,phConn);
        if(status != RV_OK)
        {
            RvLogError( pTransportMgr->pLogSrc,( pTransportMgr->pLogSrc,
                "TransportConnectionConstruct- Failed to allocate a new connection (rv=%d)",status));
            return status;
        }

        pConn =(TransportConnection*)*phConn;
        status = TransportConnectionLockAPI(pConn);
        if(status != RV_OK)
        {
            return RV_ERROR_INVALID_HANDLE;
        }
        status = TransportConnectionInit(pConn,type,eTransport,hLocalAddress,pDestAddress,bInsertConnToHash);
		if (status != RV_OK)
		{
			RvLogError( pTransportMgr->pLogSrc,( pTransportMgr->pLogSrc,
                "TransportConnectionConstruct- Failed to init new connection 0x%p (rv=%d)", pConn, status));
			TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            return status;
		}
        if (NULL != pbNewConnCreated)
        {
            *pbNewConnCreated = RV_TRUE;
        }
    }

    /* set the handle to the related owner*/
    if(hOwner != NULL)
    {
        status = TransportConnectionAttachOwner(pConn,pfnConnStateChangeHandler,
                                                pfnConnStatusEvHandler,
                                                hOwner);
        if(status != RV_OK)
        {
            RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                       "TransportConnectionConstruct - hConnPool Can't Attach Connection Owner"));

            /* If the Connection was found in the hash, don't terminate it,
               since it is in use by other owners. Just return error
               to the current owner. */
            if (NULL == hConnFromHash)
            {
                TransportConnectionTerminate(pConn,RVSIP_TRANSPORT_CONN_REASON_ERROR);
            }
            TransportConnectionUnLockAPI(pConn); /*unlock the connection*/
            *phConn = NULL;
            return status;
        }
    }
    TransportConnectionUnLockAPI(pConn); /*unlock the connection*/

    return RV_OK;

}


/************************************************************************************
 * TransportConnectionTerminate
 * ----------------------------------------------------------------------------------
 * General: Change the connection state to terminate internally and insert it to the
 *          processing queue.
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection    - Handle to the connection to terminate.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionTerminate(
                IN TransportConnection                       *pConn,
                IN RvSipTransportConnectionStateChangedReason eReason)
{

#ifdef SIP_DEBUG
    if (NULL == pConn)
    {
        return RV_ERROR_NULLPTR;
    }
#endif
    /*transaction was already terminated*/
    if(pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TERMINATED)
    {
        return RV_OK;
    }
   /* change the connection state to terminate with no application
      notification. The state change callback will be called after the
      event queue*/
    pConn->eState = RVSIP_TRANSPORT_CONN_STATE_TERMINATED;

    /* Multi-server connection can be destructed here, since it is not visible
       for application and no data is readed from it socket. */
    if (RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        TransportConnectionFree(pConn);
        return RV_OK;
    }

    /*remove connection from hash*/
    TransportConnectionHashRemove(pConn);
    /*kill timers on server connections*/
    if(SipCommonCoreRvTimerStarted(&pConn->hServerTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->hServerTimer);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionTerminate- Connection 0x%p - Release server connection timer 0x%p",
             pConn, &pConn->hServerTimer));
    }

    if(SipCommonCoreRvTimerStarted(&pConn->saftyTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->saftyTimer);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionTerminate- Connection 0x%p - Release safty timer 0x%p",
             pConn, &pConn->saftyTimer));
    }

    /*insert the transaction to the event queue*/
    RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionTerminate - connection 0x%p: Inserting Connection to the event queue", pConn));

    if(pConn->pTransportMgr->bIsShuttingDown == RV_FALSE)
    {
        return SipTransportSendTerminationObjectEvent(
                    (RvSipTransportMgrHandle)pConn->pTransportMgr,
                    (void *)pConn,
                    &(pConn->terminationInfo),
                    (RvInt32)eReason,
                    TerminateConnectionEventHandler,
                    RV_TRUE,
                    SipCommonConvertEnumToStrConnectionState(RVSIP_TRANSPORT_CONN_STATE_TERMINATED),
                    CONNECTION_OBJECT_NAME_STR);
    }
    else
    {
        TransportConnectionFree(pConn);
        return RV_OK;
    }
}


/***************************************************************************
 * TransportConnectionAttachOwner
 * ------------------------------------------------------------------------
 * General: set the connection owner with the connection callback function
 *
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 *          pfnStateChangedEvHandler - connection state changes callback
 *          pfnConnStatusEvHandle - notifies connection status.
 *            hOwner - handle to the owner.
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionAttachOwner(
                           IN  TransportConnection*                   pConn,
                           IN  RvSipTransportConnectionStateChangedEv pfnStateChangedEvHandler,
                           IN  RvSipTransportConnectionStatusEv       pfnConnStatusEvHandle,
                           IN RvSipTransportConnectionOwnerHandle     hOwner)
{
    RvStatus                     rv;
    RLIST_ITEM_HANDLE             hListItem;
    TransportConnectionOwnerInfo  *pOwnerInfo;

#ifdef SIP_DEBUG
    if(pConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif

    if (RV_TRUE == pConn->bConnErr ||
        RVSIP_TRANSPORT_CONN_STATE_CLOSING == pConn->eState ||
        RVSIP_TRANSPORT_CONN_STATE_CLOSED  == pConn->eState ||
        RVSIP_TRANSPORT_CONN_STATE_TERMINATED == pConn->eState)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionAttachOwner- trial to attaching Owner 0x%p to not usable Connection 0x%p (bConnErr=%d,state=%s)",
            hOwner, pConn, pConn->bConnErr, SipCommonConvertEnumToStrConnectionState(pConn->eState)));
        return RV_ERROR_UNKNOWN;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportConnectionAttachOwner- Attaching Owner 0x%p to Connection 0x%p",hOwner,pConn));

    /*set the connection status call back function*/
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    rv = RLIST_InsertTail(pConn->pTransportMgr->hConnectionOwnersPoolList,
                          pConn->hConnectionOwnersList,
                          &hListItem);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);


    if(rv != RV_OK)
    {

        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionAttachOwner - Couldn't insert owner to list (rv=%d)",rv));
        return rv;
    }

    pOwnerInfo = (TransportConnectionOwnerInfo*)hListItem;
    pOwnerInfo->hListItem                = hListItem;
    pOwnerInfo->hOwner                   = hOwner;
    pOwnerInfo->pfnStateChangedEvHandler = pfnStateChangedEvHandler;
    pOwnerInfo->pfnStausEvHandler        = pfnConnStatusEvHandle;

    rv = OwnersHashInsert(pConn,pOwnerInfo);
    if(rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionAttachOwner - Couldn't insert owner to hash(rv=%d)",rv));
        RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
        RLIST_Remove(pConn->pTransportMgr->hConnectionOwnersPoolList,
                     pConn->hConnectionOwnersList,
                     hListItem);
        RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
        return rv;
    }
    if(SipCommonCoreRvTimerStarted(&pConn->hServerTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->hServerTimer);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionAttachOwner - connection 0x%p: Release server connection timer 0x%p",
             pConn, &pConn->hServerTimer));
    }
    pConn->numOfOwners++;

    TransportConnectionRemoveFromConnectedNoOwnerList(pConn);
    
    return RV_OK;
}
/***************************************************************************
 * TransportConnectionDisconnect
 * ------------------------------------------------------------------------
 * General: Disconnects the connection.
 *          This process will eventually terminate the connection as it
 *          will assume the disconnected state.
 *          This function also removes the connection from the hash, so it will
 *          no longer be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionDisconnect(
                           IN  TransportConnection*                   pConn)
{
    RvStatus                     rv = RV_OK;

#ifdef SIP_DEBUG
    if(pConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#endif

    if(pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TERMINATED)
    {
        return RV_OK;
    }

    RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionDisconnect - Disconnecting connection 0x%p", pConn));

    TransportConnectionClose(pConn,RV_FALSE/*bCloseTlsAsTcp*/);
    return rv;
}

/***************************************************************************
 * TransportConnectionDetachOwner
 * ------------------------------------------------------------------------
 * General: remove the owner from the owners list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn   - Pointer to the connection.
 *          hOwner  - The owner handle
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionDetachOwner(
                            IN TransportConnection                *pConn,
                            IN RvSipTransportConnectionOwnerHandle hOwner)
{
    RLIST_ITEM_HANDLE   hListItem;
    TransportConnectionOwnerInfo *pOwner;
    RvStatus rv = RV_OK;

    if ((hOwner == NULL) ||
        (pConn->pTransportMgr == NULL)  ||
        (pConn->pTransportMgr->hConnectionOwnersPoolList == NULL) ||
        (pConn->hConnectionOwnersList == NULL))
    {
        if (pConn->pTransportMgr != NULL)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionDetachOwner- Failed. Owner 0x%p was not on connection 0x%p owners list",hOwner,pConn));
        }
        return RV_ERROR_UNKNOWN;
    }
    OwnersHashFind(pConn,hOwner, &pOwner);
    if(pOwner == NULL)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                 "TransportConnectionDetachOwner - Failed. Owner 0x%p was not on connection 0x%p owners hash",hOwner,pConn));
        return RV_ERROR_UNKNOWN;
    }
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    TransportConnectionRemoveOwnerMsgs(pConn,hOwner);
    /*remove the owner from the hash*/
    HASH_RemoveElement(pConn->pTransportMgr->hConnOwnersHash,pOwner->pHashElement);

    hListItem = pOwner->hListItem;
    RLIST_Remove(pConn->pTransportMgr->hConnectionOwnersPoolList,
                  pConn->hConnectionOwnersList,
                 hListItem);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    pOwner->hListItem = NULL;
    hListItem = NULL;
    pConn->numOfOwners--;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportConnectionDetachOwner - Detached  Owner 0x%p from  Connection 0x%p. owners left:%d",hOwner,pConn,pConn->numOfOwners));

    rv = TransportConnectionDisconnectOnNoOwners(pConn, RV_FALSE/*bForce*/);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionDetachOwner - Conn %p: failed to disconnect on no owners. hOwner=%p (rv=%d:%s)",
            pConn, hOwner, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * TransportConnectionDisconnectOnNoOwners
 * ----------------------------------------------------------------------------
 * General: checks, if the connection has owners.
 *          If it does, there is nothing to be done. Just return.
 *          Otherwise - initiate disconnection.
 * Return Value: void
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn  - Pointer to the Connection object.
 *          hOwner - Handle to the connection owner.
 *          bForce - If RV_TRUE, the Connection will be disconnected.
*****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDisconnectOnNoOwners(
                                            IN TransportConnection* pConn,
                                            IN RvBool               bForce)
{
    RLIST_ITEM_HANDLE   hListItem;
    RvInt32             disconnectTimeout = 0;
    RvBool              bForceDisconnect       = RV_FALSE;
    RvBool              bStartTimer            = RV_FALSE;
    RvBool              bAddConnToNoOwnersList = RV_FALSE;
    RvStatus            rv = RV_OK;
    RvStatus            rvTmp;

    /*check if the owner list is empty*/
    RLIST_GetHead(pConn->pTransportMgr->hConnectionOwnersPoolList,
        pConn->hConnectionOwnersList,
        &hListItem);

    /* Don't disconnect, if there are regular or security owners */
    if (hListItem != NULL
#ifdef RV_SIP_IMS_ON
        || NULL != pConn->pSecOwner
#endif /*RV_SIP_IMS_ON*/
       )
    {
        return RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionDisconnectOnNoOwners - Connection 0x%p, bForce=%d: type=%d, transport=%s, server timeout=%d, state=%s, tls state=%s",
        pConn, bForce, pConn->eConnectionType,
        SipCommonConvertEnumToStrTransportType(pConn->eTransport),
        pConn->pTransportMgr->serverConnectionTimeout,
        SipCommonConvertEnumToStrConnectionState(pConn->eState),
        TransportConnectionGetTlsStateName(pConn->eTlsState)));

    /*  Start timer if:
    1. I am a connected server and instructed to start timer by configuration.
    2. I am a connected TLS client
        Disconnect immediately if:
    1. I am a server and configuration timer is 0
    2. I am a TCP/SCTP client.
    3. Setting of timer failed.
        Insert the connection to no-owner list (and do not disconnect it) if:
    1. I am a client (all transports) and stack was configured with
    connectionCapacityPercent > 0.
    */
    if (RVSIP_TRANSPORT_CONN_TYPE_SERVER == pConn->eConnectionType)
    {
        if ( (0 < pConn->pTransportMgr->serverConnectionTimeout) && 
            (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED ||
            pConn->eState == RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED))
        {
            disconnectTimeout = pConn->pTransportMgr->serverConnectionTimeout;
            bStartTimer = RV_TRUE;
        }
        else if (0 == pConn->pTransportMgr->serverConnectionTimeout)
        {
            bForceDisconnect = RV_TRUE;
        }
        else if (RV_TRUE == bForce)
        {
            bForceDisconnect = RV_TRUE;
        }
        /* else if the timeout is -1, we don't close the connection */
    }
    else if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT == pConn->eConnectionType)
    {
        if (RVSIP_TRANSPORT_TCP  == pConn->eTransport ||
            RVSIP_TRANSPORT_SCTP == pConn->eTransport)
        {
            if(0 < pConn->pTransportMgr->connectionCapacityPercent)
            {
                bAddConnToNoOwnersList = RV_TRUE;
            }
            else
            {
                bForceDisconnect = RV_TRUE;
            }
        }
#if (RV_TLS_TYPE != RV_TLS_NONE)
        else if (RVSIP_TRANSPORT_TLS == pConn->eTransport)
        {
            if (pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED)
            {
                if(0 < pConn->pTransportMgr->connectionCapacityPercent)
                {
                    bAddConnToNoOwnersList = RV_TRUE;
                }
                else
                {
                    disconnectTimeout = OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
                    bStartTimer     = RV_TRUE;
                }
            }
            else
            {
                bForceDisconnect = RV_TRUE;
            }
        }
#endif /*RV_TLS_TYPE != RV_TLS_NONE*/
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionDisconnectOnNoOwners - Connection 0x%p: decisions: bStartTimer=%d, disconnectTimeout=%d, bForceDisconnect=%d",
        pConn,bStartTimer,disconnectTimeout,bForceDisconnect));

    if (RV_TRUE == bAddConnToNoOwnersList)
    {
        /* insert to the not-in-use list instead of terminating the connection */
        rvTmp = TransportConnectionAddToConnectedNoOwnerList(pConn);
        if (RV_OK != rvTmp)
        {
            RvLogExcep(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportConnectionDisconnectOnNoOwners - conn 0x%p: failed to add to not-in-use list. disconnect (rv=%d:%s)",
                pConn, rvTmp, SipCommonStatus2Str(rvTmp)));
            /* failed to insert the connection to the list - disconnect it. */
            bForceDisconnect = RV_TRUE;
        }
    }

    if (RV_TRUE == bStartTimer)
    {
        rv = SipCommonCoreRvTimerStartEx(&pConn->hServerTimer,
                                         pConn->pTransportMgr->pSelect,
                                         disconnectTimeout,
                                         ServerConnectionTimerExpiredEvHandler,
                                         pConn);
        pConn->bDisconnectionTimer = RV_TRUE;
        if (RV_OK != rv)
        {
            RvLogExcep(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportConnectionDisconnectOnNoOwners - conn 0x%p: No timer is available - closing connection (rv=%d:%s)",
                pConn, rv, SipCommonStatus2Str(rv)));
            bForceDisconnect = RV_TRUE;
            rv = RV_ERROR_OUTOFRESOURCES;
        }
        else
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportConnectionDisconnectOnNoOwners - conn 0x%p: server timer was set to %u ms", 
                pConn, disconnectTimeout));
        }
    }

    if (RV_TRUE == bForceDisconnect)
    {
        RvLogInfo(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionDisconnectOnNoOwners - conn 0x%p: connectionCapacityPercent=%d", 
            pConn, pConn->pTransportMgr->connectionCapacityPercent));
        rvTmp = TransportConnectionDisconnect(pConn);
        if (RV_OK != rvTmp)
        {
            RvLogExcep(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportConnectionDisconnectOnNoOwners - conn 0x%p: failed to disconenct (rv=%d:%s)",
                pConn, rvTmp, SipCommonStatus2Str(rvTmp)));
        }
    }

    return rv;
}

/******************************************************************************
 * TransportConnectionChangeState
 * ----------------------------------------------------------------------------
 * General: Changes the connection state and notify all owners.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn        - Pointer to the relevant connection.
 *          eState       - The connection new state
 *          eReason      - The reason for the new state.
 *          bSendToQueue - Pass the event through the TPQ (Transport Processing
 *                         Queue). All transport events must come from the TPQ.
 *          bChangeInternalState - change the state internally or not. we don't
 *                         want to change the state after queue event since
 *                         the state already changed.
 * Output:  RV_OK on success, error code - otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionChangeState(
            IN TransportConnection*                       pConn,
            IN RvSipTransportConnectionState              eState,
            IN RvSipTransportConnectionStateChangedReason eReason,
            IN RvBool                                     bSendToQueue,
            IN RvBool                                     bChangeInternalState)
{
    RvStatus      rv;
    TransportMgr* pTransportMgr = pConn->pTransportMgr;

    if (RV_TRUE == pConn->bClosedByLocal &&
        (
            RVSIP_TRANSPORT_CONN_STATE_READY == eState ||
            RVSIP_TRANSPORT_CONN_STATE_CONNECTING == eState ||
            RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED == eState ||
            RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED == eState
        )
       )
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionChangeState- Connection 0x%p: state to %s for closed locally connection. Ignore.",
            pConn, SipCommonConvertEnumToStrConnectionState(eState)));
        return RV_OK;
    }

    if(bChangeInternalState == RV_TRUE)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
             "TransportConnectionChangeState- Connection 0x%p state changed to %s internally",pConn,
              SipCommonConvertEnumToStrConnectionState(eState)));
        pConn->eState = eState;
    }

    if(eReason == RVSIP_TRANSPORT_CONN_REASON_UNDEFINED  &&
       pConn->bConnErr == RV_TRUE)
    {
        eReason = RVSIP_TRANSPORT_CONN_REASON_ERROR;
    }
    pConn->bConnErr = RV_FALSE;

    if(bSendToQueue == RV_TRUE)
    {
        pConn->eTcpReason = eReason;

        rv = SipTransportSendStateObjectEvent(
                        (RvSipTransportMgrHandle)pTransportMgr,
                        (void*)pConn,
                        pConn->connUniqueIdentifier,
                        (RvInt32)eState,
                        UNDEFINED,
                        ChangeConnStateEventHandler,
                        RV_TRUE,
                        SipCommonConvertEnumToStrConnectionState(eState),
                        CONNECTION_OBJECT_NAME_STR);
		if (rv != RV_OK)
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
             "TransportConnectionChangeState- Connection 0x%p failed to insert state change event to queue, closing the connection.",pConn));

			TransportConnectionDisconnect(pConn);
		}
        return rv;
    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportConnectionChangeState - Connection 0x%p state changed to %s",
        pConn, SipCommonConvertEnumToStrConnectionState(eState)));

    /* If a state is raised for incoming connection, report it to application */
    if (RVSIP_TRANSPORT_CONN_TYPE_SERVER == pConn->eConnectionType)
    {
        RvSipTransportConnectionState eConnState = pConn->eState;

        TransportCallbackIncomingConnStateChange(pConn, eState, eReason);

        if (eConnState != pConn->eState)
		{
			RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportConnectionChangeState - Connection 0x%p state was changed by application from TransportCallbackIncomingConnStateChange() callback. %s -> %s",
				pConn, SipCommonConvertEnumToStrConnectionState(eConnState),
                SipCommonConvertEnumToStrConnectionState(pConn->eState)));
			return RV_OK;
		}
	}

    /* Passes through the messages to send list and notifies the owners ONLY
       in state RVSIP_TRANSPORT_CONN_STATE_CLOSED */
    if (RVSIP_TRANSPORT_CONN_STATE_CLOSED == eState)
    {
        rv = NotifyConnUnsentMessagesStatus(pConn);
        if (RV_OK != rv)
        {
            RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
             "TransportConnectionChangeState- Connection 0x%p couldn't notify its owners about their unsent messages",pConn));
        }
    }

    /* Throw State Change event for all connection owners */
    NotifyConnectionStateChange(pConn, eState, eReason);
    return RV_OK;
}

/******************************************************************************
 * TransportConnectionNotifyStatus
 * ----------------------------------------------------------------------------
 * General: Notifies connection owners of the connection new status.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn        - Pointer to the relevant connection.
 *          hOwner       - Handle to the owner to be notified.
 *                         If  NULL, all owners will be notified.
 *          pMsgSendInfo - For MSG_WAS_SENT status - information about message
 *          eConnStatus  - The connection status.
 *          bSendToQueue - pass the event through the TPQ (transport Processing
 *                         Queue). All transport events must come from the TPQ.
 * Output:  none
 *****************************************************************************/
void RVCALLCONV TransportConnectionNotifyStatus(
                       IN TransportConnection                *pConn,
                       IN RvSipTransportConnectionOwnerHandle hOwner,
                       IN SipTransportSendInfo*               pMsgSendInfo,
                       IN RvSipTransportConnectionStatus      eConnStatus,
                       IN RvBool                              bSendToQueue)
{
    RvStatus      rv;
    TransportMgr* pTransportMgr = pConn->pTransportMgr;

    if(eConnStatus == RVSIP_TRANSPORT_CONN_STATUS_ERROR)
    {
        pConn->bConnErr = RV_TRUE;
    }
    if(bSendToQueue == RV_TRUE)
    {
        rv = SipTransportSendStateObjectEvent(
                (RvSipTransportMgrHandle)pTransportMgr,
                (void*)pConn,
                pConn->connUniqueIdentifier,
                (RvInt32)eConnStatus,
                UNDEFINED,
                NotifyConnStatusEventHandler,
                RV_TRUE,
                SipCommonConvertEnumToStrConnectionStatus(eConnStatus),
                CONNECTION_OBJECT_NAME_STR);
		if (rv != RV_OK)
		{
			RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportConnectionNotifyStatus- Connection 0x%p unable to send status notification to queue, closing connection.",
				pConn));
			TransportConnectionDisconnect(pConn);

		}
        return;
    }

    if (NULL != pMsgSendInfo)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionNotifyStatus- Connection 0x%p received status %s, hOwner=0x%p, hMsg=0x%p",
            pConn, SipCommonConvertEnumToStrConnectionStatus(eConnStatus),
            hOwner, pMsgSendInfo->msgToSend));
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionNotifyStatus- Connection 0x%p received status %s, hOwner=0x%p",
            pConn, SipCommonConvertEnumToStrConnectionStatus(eConnStatus), hOwner));
    }

    /* notify a specific owner */
    if (NULL != hOwner)
    {
        TransportConnectionOwnerInfo* pOwnerInfo;
        /* searching for the owner */
        OwnersHashFind(pConn, hOwner, &pOwnerInfo);
        if (NULL == pOwnerInfo)
        {
            /* no owner return */
            RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportConnectionNotifyStatus- Connection 0x%p owner 0x%p was not found",
                pConn, hOwner));
            return;
        }
        /* notify the owner */
        if(pOwnerInfo->pfnStausEvHandler != NULL)
        {
            NotifyConnectionStatusToOwner(pConn, pOwnerInfo->pfnStausEvHandler,
                pOwnerInfo->hOwner,eConnStatus,(void*)pMsgSendInfo,"One Owner");
        }
    }
    else
    {
        /* Notify all owners */
        NotifyConnectionStatus(pConn, eConnStatus, (void*)pMsgSendInfo);
    }
    return;
}

/************************************************************************************
 * TransportConnectionFreeConnPage
 * ----------------------------------------------------------------------------------
 * General: Free the connection page
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionFreeConnPage(IN TransportConnection    *pConn)
{
    if (NULL != pConn->hConnPage)
    {
        RPOOL_FreePage(pConn->pTransportMgr->hElementPool,pConn->hConnPage);

        pConn->hConnPage = NULL;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        pConn->strSipAddrOffset = UNDEFINED;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE)*/
    }
}

/************************************************************************************
 * TransportConnectionFree
 * ----------------------------------------------------------------------------------
 * General: Free connection resources and remove it from the hash and from the list.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionFree(IN TransportConnection    *pConn)
{
    TransportMgr*                   pTransportMgr;
    RvStatus                       rv;
    RLIST_ITEM_HANDLE               listElement = NULL;
    RLIST_ITEM_HANDLE               nextListElement = NULL;
    TransportConnectionOwnerInfo   *pOwnerInfo;

    if(pConn == NULL)
    {
        return;
    }
    pTransportMgr = pConn->pTransportMgr;
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportConnectionFree - Connection 0x%p freeing resources",pConn));

    pConn->connUniqueIdentifier = 0;
    RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
    /*release timers - for the case the timer was set and then a close event was received*/
    if(SipCommonCoreRvTimerStarted(&pConn->hServerTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->hServerTimer);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionFree- Connection 0x%p - Release server connection timer 0x%p",
             pConn, &pConn->hServerTimer));
    }

    if(SipCommonCoreRvTimerStarted(&pConn->saftyTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->saftyTimer);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
             "TransportConnectionFree- Connection 0x%p - Release safty Timer 0x%p",
             pConn, &pConn->saftyTimer));
    }

    /*remove connection from the hash*/
    TransportConnectionHashRemove(pConn);

    rv = RemoveAllPendingMessagesOnConnection((RvSipTransportConnectionHandle)pConn);
    if (rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "TransportConnectionFree - Connection 0x%p Can't Remove Owners",pConn));

    }

    /*remove all owners from the owners hash*/
    RLIST_GetHead(pConn->pTransportMgr->hConnectionOwnersPoolList,
                    pConn->hConnectionOwnersList,
                  &listElement);
    while (listElement !=NULL)
    {
        pOwnerInfo = (TransportConnectionOwnerInfo*)listElement;
        if(pOwnerInfo->pHashElement != NULL)
        {
            HASH_RemoveElement(pConn->pTransportMgr->hConnOwnersHash,pOwnerInfo->pHashElement);
            pOwnerInfo->pHashElement = NULL;
        }

        RLIST_GetNext(pConn->pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList,
                      listElement,
                      &nextListElement);

        listElement = nextListElement;
    }
    /*destruct the owners list*/
    if(pConn->hConnectionOwnersList != NULL)
    {
        RLIST_ListDestruct(pConn->pTransportMgr->hConnectionOwnersPoolList,
                           pConn->hConnectionOwnersList );
    }
    pConn->hConnectionOwnersList = NULL;
  
    TransportConnectionRemoveFromConnectedNoOwnerList(pConn);
    
    /*destruct the msgToSend list*/
    if(pConn->hMsgToSendList != NULL)
    {
         RLIST_ListDestruct(pConn->pTransportMgr->hMsgToSendPoolList,
                            pConn->hMsgToSendList );
        pConn->hMsgToSendList = NULL;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    TransportSctpConnectionDestruct(pConn);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    if (pConn->originalBuffer != NULL)
    {
        TransportMgrFreeRcvBuffer(pConn->pTransportMgr, pConn->originalBuffer);
        pConn->originalBuffer = NULL;
    }

    /*free recv buffer*/
    if (pConn->recvInfo.hMsgReceived != NULL_PAGE)
    {
        RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,    pConn->recvInfo.hMsgReceived);
        pConn->recvInfo.hMsgReceived = NULL_PAGE;
    }

    TransportConnectionFreeConnPage(pConn);
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (NULL != pConn->pTlsSession)
    {
        /* destructing the core TLS object */
        RvTLSSessionDestruct(pConn->pTlsSession,
                            &pConn->pTripleLock->hLock,
                            pConn->pTransportMgr->pLogMgr);
        /* Deallocating space */
        RA_DeAlloc(pTransportMgr->tlsMgr.hraSessions,(RA_Element)(pConn->pTlsSession));
        pConn->pTlsSession = NULL;
    }
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
    if (RV_TRUE == pConn->bFdAllocated)
    {
        RvSelectRemove(pTransportMgr->pSelect,&pConn->ccSelectFd);
        RvFdDestruct(&pConn->ccSelectFd);
        pConn->bFdAllocated = RV_FALSE;
    }

    /* remove oor */
    pConn->oorEvents            = 0;
    if (NULL != pConn->oorListLocation)
    {
		RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportConnectionFree - Connection 0x%p is about to remove from OOR list in location 0x%p",pConn, pConn->oorListLocation));
        RvMutexLock(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
        RLIST_Remove(pTransportMgr->oorListPool,pTransportMgr->oorList,pConn->oorListLocation);
        RvMutexUnlock(&pTransportMgr->hOORListLock,pTransportMgr->pLogMgr);
        pConn->oorListLocation= NULL;
    }

    /*remove the connection from the connection list*/
    RLIST_Remove(pTransportMgr->hConnPool,
                 pTransportMgr->hConnList,
                 (RLIST_ITEM_HANDLE)pConn);
    RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
}


/************************************************************************************
 * TransportConnectionHashRemove
 * ----------------------------------------------------------------------------------
 * General: Remove a given connection from the hash
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionHashRemove(IN TransportConnection    *pConn)
{
    TransportMgr* pTransportMgr;
    if(pConn == NULL)
    {
        return;
    }

    pTransportMgr = pConn->pTransportMgr;
    RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
    /*only client connections are in the hash*/
    if(pConn->pHashElement != NULL || pConn->pHashAliasElement != NULL)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
              "TransportConnectionHashRemove - removing connection 0x%p from hash",pConn));
        switch (pConn->eTransport)
        {
            case RVSIP_TRANSPORT_TLS:
            case RVSIP_TRANSPORT_TCP:
                if(pConn->pHashElement != NULL)
                {
                    HASH_RemoveElement(pTransportMgr->hConnHash, pConn->pHashElement);
                }
                if(pConn->pHashAliasElement != NULL)
                {
                    HASH_RemoveElement(pTransportMgr->hConnHash, pConn->pHashAliasElement);
                }
                break;
#if (RV_NET_TYPE & RV_NET_SCTP)
            case RVSIP_TRANSPORT_SCTP:
                TransportSctpConnectionRemoveFromHash(pConn);
                break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
            default:
                RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportConnectionHashRemove - failed to remove connection 0x%p from hash. Unsupported connection type %s",
                    pConn, SipCommonConvertEnumToStrTransportType(pConn->eTransport)));
                RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
                return;
        }
    }
    pConn->pHashElement = NULL;
    pConn->pHashAliasElement = NULL;

    RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
}

/*-----------------------------------------------------------------------
       C O N N E C T I O N:  L O C K   F U N C T I O N S
 ------------------------------------------------------------------------*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * TransportConnectionLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks connection according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionLockAPI(
                                   IN  TransportConnection*   pConn)
{
    RvStatus            rv;
    SipTripleLock        *pTripleLock;
    TransportMgr        *pMgr;
    RvInt32            identifier;


    if (pConn == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    for(;;)
    {

        /*take the triple lock from the connection*/
        RvMutexLock(&pConn->connTripleLock.hLock,pConn->pTransportMgr->pLogMgr);
        pMgr = pConn->pTransportMgr;
        pTripleLock = pConn->pTripleLock;
        identifier = pConn->connUniqueIdentifier;
        RvMutexUnlock(&pConn->connTripleLock.hLock,pConn->pTransportMgr->pLogMgr);

        if (pTripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportConnectionLockAPI - Locking conn 0x%p: Triple Lock 0x%p", pConn, pTripleLock));

        /*lock the connection*/
        rv = ConnectionLock(pConn,pMgr,pTripleLock,identifier);
        if (rv != RV_OK)
        {
            return rv;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pConn->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                    "TransportConnectionLockAPI - Locking conn 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
                    pConn, pTripleLock));
        ConnectionUnLock(pMgr->pLogMgr,&pTripleLock->hLock);

    }

    /*check if the connection is in API*/
    rv = CRV2RV(RvSemaphoreTryWait(&pTripleLock->hApiLock,NULL));
    if ((rv != RV_ERROR_TRY_AGAIN) && (rv != RV_OK))
    {
        ConnectionUnLock(pConn->pTransportMgr->pLogMgr, &pTripleLock->hLock);
        return rv;
    }

    pTripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(pTripleLock->objLockThreadId == 0)
    {
        pTripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&pTripleLock->hLock,pConn->pTransportMgr->pLogMgr,
                              &pTripleLock->threadIdLockCount);
    }


    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransportConnectionLockAPI - Locking connection 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pConn, pTripleLock, pTripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * TransportConnectionUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: unLocks connection according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
***********************************************************************************/
void RVCALLCONV TransportConnectionUnLockAPI(IN  TransportConnection*   pConn)
{
    SipTripleLock        *pTripleLock;
    TransportMgr          *pMgr;
    RvInt32               lockCnt = 0;

    if (pConn == NULL)
    {
        return;
    }

    pMgr = pConn->pTransportMgr;
    pTripleLock = pConn->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }
    RvMutexGetLockCounter(&pTripleLock->hLock,pConn->pTransportMgr->pLogMgr,&lockCnt);


    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransportConnectionUnLockAPI - UnLocking conn 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pConn, pTripleLock, pTripleLock->apiCnt,lockCnt));

    if (pTripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransportConnectionUnLockAPI - UnLocking conn 0x%p: Triple Lock 0x%p apiCnt=%d",
             pConn, pTripleLock, pTripleLock->apiCnt));
        return;
    }

    if (pTripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&pTripleLock->hApiLock,NULL);
    }

    pTripleLock->apiCnt--;
    if(lockCnt == pTripleLock->threadIdLockCount)
    {
        pTripleLock->objLockThreadId = 0;
        pTripleLock->threadIdLockCount = UNDEFINED;
    }
    ConnectionUnLock(pConn->pTransportMgr->pLogMgr,&pTripleLock->hLock);
}


/************************************************************************************
 * TransportConnectionLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks connection according to Event processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection.
***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionLockEvent(IN  TransportConnection*   pConn)
{
    RvStatus          rv = RV_OK;
    RvBool            didILockAPI = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *pTripleLock;
    TransportMgr     *pMgr;
    RvInt32           identifier;


    if (pConn == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pConn->connTripleLock.hLock,pConn->pTransportMgr->pLogMgr);
    pTripleLock = pConn->pTripleLock;
    pMgr = pConn->pTransportMgr;
    identifier = pConn->connUniqueIdentifier;
    RvMutexUnlock(&pConn->connTripleLock.hLock,pConn->pTransportMgr->pLogMgr);

    if (pTripleLock == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransportConnectionLockEvent - Exiting without locking Conn 0x%p: Triple Lock 0x%p apiCnt=%d already locked",
                   pConn, pTripleLock, pTripleLock->apiCnt));

        return RV_OK;
    }

    RvMutexLock(&pTripleLock->hProcessLock,pConn->pTransportMgr->pLogMgr);

    for (;;)
    {
        rv = ConnectionLock(pConn,pMgr, pTripleLock,identifier);
        if (rv != RV_OK)
        {
          RvMutexUnlock(&pTripleLock->hProcessLock,pConn->pTransportMgr->pLogMgr);
          if (didILockAPI)
          {
            RvSemaphorePost(&pTripleLock->hApiLock,NULL);
          }

          return rv;
        }

        if (pTripleLock->apiCnt == 0)
        {
            if (didILockAPI)
            {
              RvSemaphorePost(&pTripleLock->hApiLock,NULL);
            }
            break;
        }

        ConnectionUnLock(pConn->pTransportMgr->pLogMgr,&pTripleLock->hLock);

        rv = RvSemaphoreWait(&pTripleLock->hApiLock,NULL);
        if (rv != RV_OK)
        {
          RvMutexUnlock(&pTripleLock->hProcessLock,pConn->pTransportMgr->pLogMgr);
          return CRV2RV(rv);
        }

        didILockAPI = RV_TRUE;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransportConnectionLockEvent - Locking Conn 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
                   pConn, pTripleLock, pTripleLock->apiCnt));

    return rv;
}


/************************************************************************************
 * TransportConnectionUnLockEvent
 * ----------------------------------------------------------------------------------
 * General: UnLocks calleg according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV TransportConnectionUnLockEvent(IN  TransportConnection*   pConn)
{
    SipTripleLock   *pTripleLock;
    TransportMgr    *pMgr;
    RvThreadId       currThreadId = RvThreadCurrentId();

    if (pConn == NULL)
    {
        return;
    }

    pMgr = pConn->pTransportMgr;
    pTripleLock = pConn->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransportConnectionUnLockEvent - Exiting without unlocking Conn 0x%p: Triple Lock 0x%p apiCnt=%d wasn't locked",
                  pConn, pTripleLock, pTripleLock->apiCnt));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransportConnectionUnLockEvent - UnLocking conn 0x%p: Triple Lock 0x%p apiCnt=%d", pConn,pTripleLock,pTripleLock->apiCnt));
    ConnectionUnLock(pConn->pTransportMgr->pLogMgr,&pTripleLock->hLock);

    RvMutexUnlock(&pTripleLock->hProcessLock,pConn->pTransportMgr->pLogMgr);
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */



#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

#if (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * TransportConnectionGetSecurityNotificationName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given security status
 * Return Value: The string with the status name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eStatus - The status as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransportConnectionGetSecurityNotificationName (
                                IN  RvSipTransportConnectionTlsStatus eStatus)
{

    switch(eStatus)
    {
    case RVSIP_TRANSPORT_CONN_TLS_STATUS_HANDSHAKE_PROGRESS:
        return "Handshake Progress";
    default:
        return "Undefined";
    }
}

/***************************************************************************
 * TransportConnectionGetTlsStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given tls state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransportConnectionGetTlsStateName (
                                IN  RvSipTransportConnectionTlsState  eTlsState)
{

    switch(eTlsState)
    {
    case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED:
        return "TLS Handshake Completed";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED:
        return "TLS Handshake started";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED:
        return "TLS Connected";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY:
        return "TLS Handshake Ready";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED:
        return "TLS Handshake Failed";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED:
        return "TLS Close Seq started";
    case RVSIP_TRANSPORT_CONN_TLS_STATE_TERMINATED:
        return "TLS Terminated";
    default:
        return "Undefined";
    }
}
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


/***************************************************************************
 * TransportConnectionGetAddress
 * ------------------------------------------------------------------------
 * General: retrieves local or remote address from a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pConn           - the connection that its address we seek
 *          bIsRemote       - get the remote or local address of the connection
 * Output:  strAddress      - A previously allocated space for the address.
 *                            Should be of RVSIP_TRANSPORT_LEN_STRING_IP size.
 *          pPort           - a pointer to the variable to hold the port
 *          peAddressType   - ipv6/ipv4
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionGetAddress(
                                        IN  TransportConnection    *pConn,
                                        IN  RvBool             bIsRemote,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *pPort,
#if defined(UPDATED_BY_SPIRENT)
                                        OUT RvChar             *if_name,
					OUT RvUint16           *vdevblock,
#endif
                                        OUT RvSipTransportAddressType *peAddressType)
{

    RvStatus rv = RV_OK;
    if (RV_TRUE == bIsRemote)
    {
        if(0 == RvAddressGetIpPort(&pConn->destAddress.addr))
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionGetAddress - no remote address on connection 0x%p",pConn));
            return RV_ERROR_NOT_FOUND;
        }

        if (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(&pConn->destAddress.addr))
        {
            RvAddressGetString(&pConn->destAddress.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strAddress);
        }
#if(RV_NET_TYPE & RV_NET_IPV6)
        else if(RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&pConn->destAddress.addr))
        {
            strAddress[0] = '[';
            RvAddressGetString(&pConn->destAddress.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strAddress+1);
            strAddress[strlen(strAddress)] = ']';
        }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
        if (NULL != pPort)
            *pPort = RvAddressGetIpPort(&pConn->destAddress.addr);
        if (NULL != peAddressType)
            *peAddressType = SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pConn->destAddress.addr));
    }
    else
    {
        if(pConn->hLocalAddr == NULL)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionGetAddress - no local address on connection 0x%p",pConn));

            return RV_ERROR_NOT_FOUND;
        }
        rv = SipTransportLocalAddressGetAddressByHandle(
							(RvSipTransportMgrHandle)pConn->pTransportMgr,
							pConn->hLocalAddr, RV_FALSE,RV_TRUE,strAddress,pPort
#if defined(UPDATED_BY_SPIRENT)
							,if_name
							,vdevblock
#endif
							);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                  "TransportConnectionGetAddress - connection 0x%p: Failed to get info for local address 0x%p (rv=%d)",
                  pConn,pConn->hLocalAddr,rv));
            return rv;
        }
        if (NULL != peAddressType)
        {
            rv = SipTransportLocalAddressGetAddrType(pConn->hLocalAddr, peAddressType);
            if(RV_OK != rv  ||  RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == *peAddressType)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionGetAddress - connection 0x%p: Failed to get address type for local address 0x%p",
                    pConn, pConn->hLocalAddr));
                return ((RV_OK != rv) ? rv : RV_ERROR_UNKNOWN);
            }
        }
    }

    return rv;
}


/***************************************************************************
 * TransportConnectionHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a connection into the hash table.
 *          The key is generated from the connection information and the
 *          connection handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to insert to the hash table
 *               RV_OK - connection handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionHashInsert(
                               IN RvSipTransportConnectionHandle hConn,
                               IN RvBool                         bByAlias)
{
    TransportConnection*        pConn = (TransportConnection*)hConn;
    RvStatus                    rv = RV_OK;
    RvBool                      wasInsert = RV_TRUE;

    /*connection is already in the hash*/
    if(RV_FALSE == bByAlias && pConn->pHashElement != NULL)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionHashInsert - connection 0x%p - already in the hash",
            pConn));
        return RV_OK;
    }
    else if (RV_TRUE == bByAlias && pConn->pHashAliasElement != NULL)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionHashInsert - connection 0x%p - already in the hash by alias",
              pConn));
        return RV_OK;
    }

    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock, 
                pConn->pTransportMgr->pLogMgr);

    switch (pConn->eTransport)
    {
        case RVSIP_TRANSPORT_TLS:
        case RVSIP_TRANSPORT_TCP:
        {
            TransportConnectionHashKey  hashKey;
            void** ppHashElem = NULL;

            /*prepare hash key*/
            hashKey.pTransportMgr  = pConn->pTransportMgr;
            hashKey.eTransport = pConn->eTransport;
            hashKey.hLocalAddr = pConn->hLocalAddr;
            if(RV_FALSE == bByAlias)
            {
                hashKey.pAlias = NULL;
                hashKey.pRemoteAddress = &(pConn->destAddress.addr);
                ppHashElem = &(pConn->pHashElement);
            }
            else
            {
                hashKey.pRemoteAddress = NULL;
                hashKey.pAlias = &(pConn->pAlias);
                ppHashElem = &(pConn->pHashAliasElement);
            }

            /*insert to hash and save hash element id in the call-leg*/
            rv = HASH_InsertElement(pConn->pTransportMgr->hConnHash,
                                    &hashKey,
                                    &pConn,
                                    RV_FALSE,
                                    &wasInsert,
                                    ppHashElem,
                                    TransportConnectionHashCompare);
        }
        break;
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
        {
            rv = TransportSctpConnectionInsertIntoHash(pConn, bByAlias);
        }
        break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        default:
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionHashInsert - failed to insert connection 0x%p into hash. Unsupported connection type %s",
                pConn, SipCommonConvertEnumToStrTransportType(pConn->eTransport)));
            RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
            return RV_ERROR_UNKNOWN;
    }

    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    if(rv != RV_OK || wasInsert == RV_FALSE)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportConnectionHashInsert- Connection 0x%p - was inserted to hash ",pConn));
    return RV_OK;
}

/******************************************************************************
 * TransportConnectionRegisterSelectEvents
 * ----------------------------------------------------------------------------
 * General: This function is used to wrap Core Select Functions.
 *          For First calls the select FD will be constructed, and first select
 *          call will be placed.
 *          For calls (other than first) that have events, a select update call
 *          will be made.
 *          For calls with no events, select usage will be stoped:
 *              Fd will be destructed
 *              A call to select remove will be placed
 *
 * Return Value: RV_OK - completes succesfully.
 *               RV_ERROR_UNKNOWN - failed in one of como core operations
 *
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn      - Pointer to the relevant connection.
 *          bAddClose  - should the close event be called on
 *          bFirstCall - is this the first time we call for the socket?
 *                       if it is the first timem we will construct fd and 
 *                       add it to the select FDs
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionRegisterSelectEvents(
                                            IN TransportConnection  *pConn,
                                            IN RvBool               bAddClose,
                                            IN RvBool               bFirstCall)
{
    RvStatus rv = RV_OK;

    /* we always want to know if the connection has closed */
    if (bAddClose)
    {
        pConn->ccEvent |= RvSelectClose;
    }
    if (RV_TRUE == bFirstCall)
    {
        RvStatus crv = RV_OK;
        crv = RvFdConstruct(&pConn->ccSelectFd,pConn->pSocket,pConn->pTransportMgr->pLogMgr);
        if (RV_OK != crv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                       "TransportConnectionRegisterSelectEvents - pConn 0x%p: Failed to build select fd for the new connection, crv=%d",pConn,RvErrorGetCode(crv)))
            RvFdDestruct(&pConn->ccSelectFd);
            return RV_ERROR_UNKNOWN;
        }
        pConn->bFdAllocated = RV_TRUE;
        rv = ConnectionSelectEventAdd(pConn);
    }
    else if (0 != pConn->ccEvent)
    {
        /* if we have already destructed the FD, there is nothing
           to call-on on, we can return */
        if (RV_FALSE == pConn->bFdAllocated)
        {
            return RV_ERROR_UNKNOWN;
        }
        rv = ConnectionSelectEventUpdate(pConn);
    }
    if (RV_OK != rv || 0 == pConn->ccEvent)
    {
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                       "TransportConnectionRegisterSelectEvents - pConn 0x%p: Failed to add/update select. socket is closed(rv=%d:%s)",
                       pConn, rv, SipCommonStatus2Str(rv)));
        }
        if (RV_TRUE == pConn->bFdAllocated)
        {
            RvSelectRemove(pConn->pTransportMgr->pSelect,&pConn->ccSelectFd);
            RvFdDestruct(&pConn->ccSelectFd);
            pConn->bFdAllocated = RV_FALSE;
        }
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * TransportConnectionDecUsageCounter
 * ----------------------------------------------------------------------------
 * General: Decrements connection usage counter. In case usage counter is 0
 *          and connection is marked as 'deleted' this function actually
 *          deletes the connection.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDecUsageCounter(
                                            IN TransportConnection    *pConn)
{
    if (pConn == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    pConn->usageCounter--;
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionDecUsageCounter - connection 0x%p, socket %d, counter %d, isDeleted %d",
        pConn, pConn->ccSocket, pConn->usageCounter, pConn->isDeleted));
    
    if ((pConn->usageCounter == 0) && pConn->isDeleted)
    {
        RvStatus retCode;
        switch (pConn->eTransport)
        {
            case RVSIP_TRANSPORT_TCP:
            case RVSIP_TRANSPORT_TLS:
                retCode = TransportTCPClose(pConn);
                break;
#if (RV_NET_TYPE & RV_NET_SCTP)
            case RVSIP_TRANSPORT_SCTP:
                retCode = TransportSctpClose(pConn);
                break;
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */
            default:
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionDecUsageCounter - connection 0x%p, socket %d - unsupported transport type %d",
                    pConn, pConn->ccSocket, pConn->eTransport));
                return RV_ERROR_NOTSUPPORTED;
        }
        if (RV_OK != retCode)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionDecUsageCounter - connection 0x%p, socket %d - failed to close the connection",
                pConn, pConn->ccSocket));
            return retCode;
        }
    }
    return RV_OK;
}

/******************************************************************************
 * TransportConnectionDecUsageCounterThreadSafe
 * ----------------------------------------------------------------------------
 * General: Calls TransportConnectionDecUsageCounter() in thread safe way:
 *          locks the Event Lock of the connection before call and
 *          unlocks it after it.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the Transport Manager object.
 *          pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDecUsageCounterThreadSafe(
                                        IN TransportMgr*        pTransportMgr,
                                        IN TransportConnection* pConn)
{
    RvStatus rv;

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionDecUsageCounterThreadSafe - Conn %p was terminated"));
        return RV_OK;
    }
    rv = TransportConnectionDecUsageCounter(pConn);
    TransportConnectionUnLockEvent(pConn);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return rv;
}

/******************************************************************************
 * TransportConnectionIncUsageCounter
 * ----------------------------------------------------------------------------
 * General: Increments connection usage counter. In case connection
 *          is marked as 'deleted' this function returns RV_ERROR_UNKNOWN.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionIncUsageCounter(
                                            IN TransportConnection    *pConn)
{
    RvStatus retCode = RV_ERROR_UNKNOWN;

    if (pConn == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (!pConn->isDeleted)
    {
        pConn->usageCounter++;
        retCode = RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionIncUsageCounter - connection 0x%p, socket %d, counter %d, isDeleted %d",
        pConn, pConn->ccSocket, pConn->usageCounter, pConn->isDeleted));

    return retCode;
}

/******************************************************************************
 * TransportConnectionOpen
 * ----------------------------------------------------------------------------
 * General: opens the connection - creates a socket, binds it, listens/connects
 *          to the peer socket.
 * Return Value: RV_OK on success
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pConn           - Pointer to the connection.
 *         bConnect        - If RV_FALSE, the socket will not be connected.
 *                           In this case TransportTCPConnect should be called.
 *                           This paramater is actual for client connections.
 *         clientLocalPort - Port, to which the connection socket will be bound
 *                           This parameter is actual for client connections.
 *                           Listening connections use port of their Local
 *                           Address objects.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionOpen(
                                    IN TransportConnection* pConn,
                                    IN RvBool               bConnect,
                                    IN RvUint16             clientLocalPort)
{
    RvStatus rv = RV_OK;

    switch (pConn->eTransport)
    {
    case RVSIP_TRANSPORT_TCP:
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
        /* If Connection socket was bound to the port, connect it.
           Otherwise - open (construct socket, bind and connect/listen) */
        if (0 < pConn->localPort)
        {
            rv = TransportTCPConnect(pConn);
            if (RV_OK != rv)
            {
                RvSocketDestruct(pConn->pSocket, RV_FALSE /*cleanSocket*/,
                    NULL/*portRange*/, pConn->pTransportMgr->pLogMgr);
                pConn->pSocket = NULL;
            }
        }
        else
        {
            rv = TransportTCPOpen(pConn->pTransportMgr, pConn, bConnect,
                                  clientLocalPort);
        }
        break;
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        rv = TransportSctpOpen(pConn->pTransportMgr, pConn);
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        rv = RV_ERROR_NOTSUPPORTED;
    }
    return rv;
}

/******************************************************************************
 * TransportConnectionClose
 * ----------------------------------------------------------------------------
 * General: close the connection - moves it to CLOSING state, stop listening
 *          for incoming messages, removes it from hash.
 * Return Value: RV_OK on success
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - Pointer to the connection.
 *          bCloseTlsAsTcp  - If RV_TRUE, the TLS state machine is not affected
 *                            by the closing.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionClose(
                                        IN TransportConnection* pConn,
                                        IN RvBool               bCloseTlsAsTcp)
{
    RvStatus rv = RV_OK;
    
#if (RV_TLS_TYPE == RV_TLS_NONE)
	RV_UNUSED_ARG(bCloseTlsAsTcp)
#endif /*#if (RV_TLS_TYPE == RV_TLS_NONE)*/

	switch (pConn->eTransport)
    {
    case RVSIP_TRANSPORT_TCP:
        rv = TransportTCPClose(pConn);
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        if (RV_TRUE == bCloseTlsAsTcp)
        {
            rv = TransportTCPClose(pConn);
        }
        else
        {
            rv = TransportTlsClose(pConn);
        }
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        rv = TransportSctpClose(pConn);
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        rv = RV_ERROR_NOTSUPPORTED;
    }
    return rv;
}

/******************************************************************************
* TransportConnectionApplyServerConnReuseThreadSafe
* -----------------------------------------------------------------------------
* General: applies the connection-reuse procedure on the connection:
*          1. check that the top via contain an 'alias' parameter.
*          2. authorize connection alias.
*          3. insert the connection to the hash.
*          Locks connection Event Lock before & unlocks it after the procedure.
* Return Value: void
* -----------------------------------------------------------------------------
* Arguments:
*      pTransportMgr - handle to the Transport Manager object.
*      hConn         - handle to the connection.
*      hAppConn      - handle stored by the application in the connection.
*      hMsg          - the received message object
* Output: void
******************************************************************************/
RvStatus TransportConnectionApplyServerConnReuseThreadSafe(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvSipMsgHandle                    hMsg)
{
    RvSipHeaderListElemHandle   hListElem;
    RvSipViaHeaderHandle        hVia;
    RvStatus                    rv;

    /* Lock the connection */
    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
            "TransportConnectionApplyServerConnReuseThreadSafe - conn %p: failed to lock(rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    if(NULL == pConn->pTransportMgr->appEvHandlers.pfnEvConnServerReuse)
    {
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }
    if(RVSIP_TRANSPORT_CONN_TYPE_CLIENT == pConn->eConnectionType)
    {
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }
    if(pConn->pHashAliasElement != NULL)
    {
        /* the connection is already in the hash. nothing to do */
        TransportConnectionUnLockEvent(pConn);
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionApplyServerConnReuse - conn 0x%p already in the hash with alias. nothing to do",
            pConn));
        return RV_OK;

    }
    /*get the top via element*/
    hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(hMsg,RVSIP_HEADERTYPE_VIA,
                                                        RVSIP_FIRST_HEADER,&hListElem);
    if(hVia == NULL || hListElem == NULL)
    {
        TransportConnectionUnLockEvent(pConn);
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionApplyServerConnReuse - conn 0x%p Failed: No top via header in message 0x%p.",
            pConn,hMsg));
        return RV_OK;
    }
    if(RV_FALSE == RvSipViaHeaderGetAliasParam(hVia))
    {
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }

    /* found the alias parameter - get the sent-by address as the alias string */
    rv = GetConnectionAliasFromVia(pConn, hVia);
    if(rv!= RV_OK)
    {
        TransportConnectionUnLockEvent(pConn);
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportConnectionApplyServerConnReuse - conn 0x%p Failed to save alias in connection.",
            pConn));
        return rv;
    }

    TransportConnectionUnLockEvent(pConn);

    /* For all transports, application is responsible to insert the connection to the hash */
    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "TransportConnectionApplyServerConnReuse - conn 0x%p - give to application for authorization",
        pConn));
        
    TransportCallbackConnServerReuse(pConn->pTransportMgr, pConn, hAppConn);
            
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}   

/***************************************************************************
 * TransportConnectionAddToConnectedNoOwnerList
 * ------------------------------------------------------------------------
 * General: Add connection to the not-in-use connections list.
 *
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionAddToConnectedNoOwnerList(
                           IN  TransportConnection*  pConn)
{
    RvStatus                     rv;
    RvSipTransportConnectionHandle* phConn = NULL;
    RLIST_ITEM_HANDLE            pItem = NULL;

    if(pConn == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportConnectionAddToConnectedNoOwnerList - add Connection 0x%p",pConn));

    /*set the connection status call back function*/
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    rv = RLIST_InsertTail(pConn->pTransportMgr->hConnectedNoOwnerConnPool,
                          pConn->pTransportMgr->hConnectedNoOwnerConnList,
                          &pItem);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    if(rv != RV_OK)
    {
        /* not enough space in the list - impossible. the list should be in the same
          size as the connections list */
        RvLogExcep(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionAddToConnectedNoOwnerList - Couldn't insert conn 0x%p to list (rv=%d)",pConn,rv));
        return rv;
    }

    
    rv = TransportConnectionLockAPI(pConn);
	if (RV_OK != rv)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    pConn->hConnNoOwnerListItem = pItem;
    TransportConnectionUnLockAPI(pConn);
    
    phConn = (RvSipTransportConnectionHandle*)pItem;
    *phConn = (RvSipTransportConnectionHandle)pConn;
    
    return RV_OK;
}

/***************************************************************************
 * TransportConnectionRemoveFromConnectedNoOwnerList
 * ------------------------------------------------------------------------
 * General: remove the connection from the not-in-use list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Output:     pConn - pointer to the connection.
 ***************************************************************************/
void RVCALLCONV TransportConnectionRemoveFromConnectedNoOwnerList(
                               TransportConnection   *pConn)
{
    if(NULL == pConn->hConnNoOwnerListItem)
    {
        /* nothing to do */
        return;
    }

    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    RLIST_Remove(pConn->pTransportMgr->hConnectedNoOwnerConnPool,
                  pConn->pTransportMgr->hConnectedNoOwnerConnList,
                 pConn->hConnNoOwnerListItem);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    pConn->hConnNoOwnerListItem = NULL;
    
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TransportConnectionRemoveFromConnectedNoOwnerList - Connection 0x%p out of no-owner-list",pConn));
}

/***************************************************************************
 * TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe
 * ------------------------------------------------------------------------
 * General: calls TransportConnectionRemoveFromConnectedNoOwnerList in thread
 *          safe way: locks the connection Event Lock before and unlocks it
 *          after the call.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to the Transport Manager object.
 *          pConn         - pointer to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe(
                                        IN TransportMgr*        pTransportMgr,
                                        IN TransportConnection* pConn)
{
    RvStatus rv;
    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe - Conn %p was terminated"));
        return rv;
    }
    TransportConnectionRemoveFromConnectedNoOwnerList(pConn);
    TransportConnectionUnLockEvent(pConn);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return RV_OK;
}

/***************************************************************************
 * TransportConnectionRemoveOwnerMsgs
 * ------------------------------------------------------------------------
 * General: remove all the pending messages of a specific owner that
 *          are about to be send, from the connection send list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn  - Pointer to the Connection object.
 *          hOwner - Handle to the connection owner.
 ***************************************************************************/
void RVCALLCONV TransportConnectionRemoveOwnerMsgs(
                      IN TransportConnection*                pConn,
                      IN RvSipTransportConnectionOwnerHandle hOwner)
{
    RLIST_ITEM_HANDLE hListItem;
    RLIST_ITEM_HANDLE nextListElement;
    SipTransportSendInfo *MsgSendInfo;
    RvBool            bTerminateConnOnCopyFailure = RV_FALSE;

    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    RLIST_GetHead(pConn->pTransportMgr->hMsgToSendPoolList,
                  pConn->hMsgToSendList,
                  &hListItem);
	/*Safe counter was removed since it was detected that the current number of elements in the list
	  is not predictable*/
    while (hListItem !=NULL)
    {
        MsgSendInfo = (SipTransportSendInfo*)hListItem;

        RLIST_GetNext(pConn->pTransportMgr->hMsgToSendPoolList,
                      pConn->hMsgToSendList,
                      hListItem,
                      &nextListElement);

        if (MsgSendInfo->hOwner == hOwner)
        {
            RvStatus rv           = RV_OK;
			RvBool   bTriedToCopy = RV_FALSE;

            /* Check if the message was sent partially (msgSize > currPos),
               don't remove it from the sending queue in order to 
               enable sending of the rest message. This will prevent parser
               error on the receiver side */
			if ((MsgSendInfo->msgSize > MsgSendInfo->currPos && MsgSendInfo->currPos != 0) ||
                MsgSendInfo->bDontRemove == RV_TRUE)
            {
                HPAGE    hMsgToSendCopy;
                RvInt32  msgCopyOffset = 0;
                RvInt32  msgCopySize = MsgSendInfo->msgSize;

				bTriedToCopy = RV_TRUE; 

                rv = RPOOL_GetPage(pConn->pTransportMgr->hMsgPool,0,&hMsgToSendCopy);
                if (RV_OK != rv)
                {
                    RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                        "TransportConnectionRemoveOwnerMsgs - Failed to get new page. hConnection=0x%p, hPool=0x%p, MsgSendInfo=0x%p",
                        pConn,pConn->pTransportMgr->hMsgPool,MsgSendInfo));
                }
                if (RV_OK == rv)
                {
					/*The hMsgToSendCopy does not have to be consecutive since it is not the buffer to send*/
                    rv = RPOOL_Append(pConn->pTransportMgr->hMsgPool,
                        hMsgToSendCopy, msgCopySize, RV_FALSE, &msgCopyOffset);
                    if(rv != RV_OK)
                    {
                        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                            "TransportConnectionRemoveOwnerMsgs - Failed to append memory to the page. hMsg=0x%p,hMsgCopy=0x%p,hConnection=0x%p,MsgSendInfo=0x%p",
                            MsgSendInfo->msgToSend,hMsgToSendCopy,pConn,MsgSendInfo));
                        RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,hMsgToSendCopy);
                    }
                }
                if (0 != msgCopyOffset)
                {
                    RvLogExcep(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                        "TransportConnectionRemoveOwnerMsgs - Offset on a new page is not 0. hMsg=0x%p,hMsgCopy=0x%p,hConnection=0x%p,MsgSendInfo=0x%p",
                        MsgSendInfo->msgToSend,hMsgToSendCopy,pConn,MsgSendInfo));
                }
                if (RV_OK == rv)
                {
                    rv = RPOOL_CopyFrom(
                        pConn->pTransportMgr->hMsgPool,hMsgToSendCopy,0,
                        pConn->pTransportMgr->hMsgPool,MsgSendInfo->msgToSend,0,
                        msgCopySize);
                    if (RV_OK != rv)
                    {
                        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                            "TransportConnectionRemoveOwnerMsgs - Failed to copy hMsg=0x%p,hMsgCopy=0x%p,hConnection=0x%p,MsgSendInfo=0x%p",
                            MsgSendInfo->msgToSend,hMsgToSendCopy,pConn,MsgSendInfo));
                        RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,hMsgToSendCopy);
                    }
                }
                if (RV_OK == rv)
                {
                    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                        "TransportConnectionRemoveOwnerMsgs - Message was copied hMsg=0x%p,hMsgCopy=0x%p,hConnection=0x%p,MsgSendInfo=0x%p",
                        MsgSendInfo->msgToSend,hMsgToSendCopy,pConn,MsgSendInfo));

                    MsgSendInfo->msgToSend = hMsgToSendCopy;
                    MsgSendInfo->bFreeMsg = RV_TRUE;
                    MsgSendInfo->hOwner = NULL;
                }
            }

            if (RV_OK != rv  &&  MsgSendInfo->bDontRemove == RV_TRUE)
            {
                MsgSendInfo->bDontRemove = RV_FALSE;
                bTerminateConnOnCopyFailure = RV_TRUE;
            }

            /* Remove the message, if there is no part of it to be sent,
               or if the message copying failed */
            if ((RV_OK != rv  ||  bTriedToCopy == RV_FALSE) &&
                 MsgSendInfo->bDontRemove == RV_FALSE)
            {
				/*free the message page if require*/
                if(MsgSendInfo->bFreeMsg == RV_TRUE && MsgSendInfo->msgToSend != NULL_PAGE)
                {
                    RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,MsgSendInfo->msgToSend);
                    MsgSendInfo->msgToSend = NULL_PAGE;
                }
                RLIST_Remove(pConn->pTransportMgr->hMsgToSendPoolList,
                             pConn->hMsgToSendList, hListItem);
            }
        }
        hListItem = nextListElement;
    }

    if (bTerminateConnOnCopyFailure == RV_TRUE)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionRemoveOwnerMsgs - necessary message copeing failed - terminate connection(0x%p)",
            pConn));
        TransportConnectionClose(pConn,RV_FALSE/*bCloseTlsAsTcp*/);
    }

    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
}

/******************************************************************************
 * TransportConnectionAddWriteEventToTPQ
 * ----------------------------------------------------------------------------
 * General: allocates, initializes and tails the WRITE_EVENT event
 *          to the Transport Processing Queue (TPQ).
 *          This function is called on handling of write event raised by Select
 *          Engine for connection-oriented protocol, such as TCP or TLS.
 *          The function assumes that the Connection object is locked already.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - The Connection object
 * Output:  ppEv  - The allocated event
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionAddWriteEventToTPQ(
                                    IN  TransportConnection* pConn,
                                    OUT void**               ppEv)
{
    RvStatus  rv;
    TransportProcessingQueueEvent* pEv = NULL;

    rv = TransportProcessingQueueAllocateEvent(
                        (RvSipTransportMgrHandle)pConn->pTransportMgr,
                         NULL,WRITE_EVENT,RV_TRUE, &pEv);
    if (rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionHandleWriteEvent - connection 0x%p: failed to allocate event(rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    pEv->typeSpecific.write.pConn = pConn;
    rv = TransportConnectionIncUsageCounter(pConn);
    if (rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionHandleWriteEvent - connection 0x%p: failed to increment usage counter(rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,pEv);
        return rv;
    }
    rv = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,pEv);
    if (rv != RV_OK)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionHandleWriteEvent - connection 0x%p: failed to tail event to TPQ(rv=%d:%s)",
            pConn, rv, SipCommonStatus2Str(rv)));
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,pEv);
        TransportConnectionDecUsageCounter(pConn);
        return rv;
    }

    /* Indicate that there's waiting WRITE_EVENT in the processing queue */
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportConnectionHandleWriteEvent - connection 0x%p: Set pConn->bWriteEventInQueue to RV_TRUE",
        pConn));
    pConn->bWriteEventInQueue = RV_TRUE;

    *ppEv = (void*)pEv;

    return RV_OK;
}

/******************************************************************************
 * TransportConnectionReceive
 * ----------------------------------------------------------------------------
 * General: reads data from socket, that serves bit-stream connection,
 *          such as TCP or TLS.
 *          The function accumulates received bytes until the whole SIP message
 *          was received. After this the message is parsed and passed to upper
 *          layers.
 *          This function is called on handling of read event raised by select.
 *          The function assumes that the Connection object is locked already.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - The Connection object
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionReceive(IN  TransportConnection* pConn)
{
    RvSize_t  recvBytes    = 0;
    RvChar*   recvBuf      = NULL;
    RvStatus  status       = RV_OK;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
          "TransportConnectionReceive - connection 0x%p: read event received. starting to process",
           pConn));

    /* Initialize auxiliary structures, used for received data accumulation */
    status = TransportMsgBuilderAccumulateMsg(NULL,0,pConn,NULL);
    if (status != RV_OK)
    {
        return status;
    }
    if(pConn->originalBuffer != NULL)
    {
        TransportMgrFreeRcvBuffer(pConn->pTransportMgr,pConn->originalBuffer);
        pConn->originalBuffer = NULL;
    }

    /* Allocate buffer, where the data to be read will be stored */
    status = TransportMgrAllocateRcvBuffer(pConn->pTransportMgr,RV_TRUE,&recvBuf);
    if ((status != RV_OK) || (recvBuf == NULL))
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Read data from connection socket */
    switch (pConn->eTransport)
    {
        case RVSIP_TRANSPORT_TCP:
            status = TransportTCPReceive(pConn, recvBuf, &recvBytes);
            break;

    #if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            status = TransportTlsReceive(pConn, recvBuf, &recvBytes);
            break;
    #endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

        default:
            RvLogExcep(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionReceive - connection %p: unexpected transport type (%d)",
                pConn, pConn->eTransport));
            TransportMgrFreeRcvBuffer(pConn->pTransportMgr, recvBuf);
            return RV_ERROR_UNKNOWN;
    }

    if (status != RV_OK)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionReceive - connection 0x%p: error while receiving data(rv=%d:%s)",
            pConn, status, SipCommonStatus2Str(status)));
        TransportMgrFreeRcvBuffer(pConn->pTransportMgr, recvBuf);
        return status;
    }

    /* If data was read from connection, handle it */
    if (recvBytes > 0)
    {
        RvStatus rv;
        RvBool   bDiscardData = RV_FALSE;

        recvBuf[recvBytes]='\0';
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionReceive - Connection 0x%p (%s): bytes read=%d",
            pConn, SipCommonConvertEnumToStrTransportType(pConn->eTransport),
            recvBytes));

        /* Ask application for approval to handle the received bytes */
        if (RVSIP_TRANSPORT_CONN_TYPE_SERVER == pConn->eConnectionType)
        {
            RvSipTransportConnectionState eConnState = pConn->eState;
#if (RV_TLS_TYPE != RV_TLS_NONE)
            RvSipTransportConnectionTlsState eConnTlsState = pConn->eTlsState;
#endif
            TransportCallbackConnDataReceived(pConn->pTransportMgr, pConn, recvBuf,
                                              (RvInt32)recvBytes, &bDiscardData);

            /* Discard the received data, if the Application requested */
            if (RV_TRUE == bDiscardData)
            {
                RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionReceive - Connection %p: data was discarded by the Application",
                    pConn));
                TransportMgrFreeRcvBuffer(pConn->pTransportMgr, recvBuf);
                return RV_OK;
            }

            /* Ensure, that connection was not closed by Application from within
               TransportCallbackConnDataReceived() */
            if (eConnState != pConn->eState
#if (RV_TLS_TYPE != RV_TLS_NONE)
                ||
                eConnTlsState != pConn->eTlsState
#endif
               )
            {
                RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportConnectionReceive - Connection %p was closed from within DataReceivedEv callback",pConn));
                TransportMgrFreeRcvBuffer(pConn->pTransportMgr, recvBuf);
                return RV_OK;
            }
        }  /* if (RVSIP_TRANSPORT_CONN_TYPE_SERVER == pConn->eConnectionType)*/

        /* Try to build message from received bytes */
        rv = TransportMsgBuilderAccumulateMsg((RvChar*)recvBuf,
                (RvInt32)recvBytes, pConn, pConn->hLocalAddr);
        if (rv != RV_OK)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportConnectionReceive - Connection 0x%p: failed to build message(rv=%d:%s)",
                pConn, rv, SipCommonStatus2Str(rv)));
            if (rv != RV_ERROR_OUTOFRESOURCES)
            {
                TransportMgrFreeRcvBuffer(pConn->pTransportMgr, recvBuf);
            }
            return rv;
        }
    }

    /* If buffer was not saved into Connection due to some error, like
       out-of-resource, there is nothing to do with this buffer. Just free it*/
    if (pConn->originalBuffer == NULL)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportConnectionReceive - connection 0x%p: Freeing RcvBuffer", pConn));
        TransportMgrFreeRcvBuffer(pConn->pTransportMgr,recvBuf);
        return RV_OK;
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/
/*****************************************************************************
* DestChoosePort
* ---------------------------------------------------------------------------
* General: chosses a port from message/default for protocol.
* Return Value: port to use
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvUint16 RVCALLCONV ChoosePort(IN  RvSipViaHeaderHandle hVia)
{
    /*  In case port was not specified, default port value should be used  */
    RvSipTransport eTransport = RvSipViaHeaderGetTransport(hVia);

    return (RvUint16)((eTransport == RVSIP_TRANSPORT_TLS) ? RVSIP_TRANSPORT_DEFAULT_TLS_PORT : RVSIP_TRANSPORT_DEFAULT_PORT);
    
}

/***************************************************************************
* GetConnectionAliasFromVia
* ------------------------------------------------------------------------
* General: Gets the sent-by string from via header, and saves it as an
*          alias string in the connection object.
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
*      hConn - handle to the connection to be set.
*      hVia  - handle to the via header.
***************************************************************************/
static RvStatus GetConnectionAliasFromVia(TransportConnection* pConn,
                                          RvSipViaHeaderHandle hVia)
{
    RvStatus rv = RV_OK;
    RvUint   length = 0;
    RvInt32  port=0;
    RvChar*  pAlias;


    length = RvSipViaHeaderGetStringLength(hVia, RVSIP_VIA_HOST);
    if (0 == length)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "GetConnectionAliasFromVia - conn 0x%p Failed. no host parameter in top via.",
            pConn));
        return RV_ERROR_NOT_FOUND;
    }

    length +=6; /*we might concatenate a ":port\0" to the address */
    
    pConn->pAlias.hPool = pConn->pTransportMgr->hElementPool;
    if(pConn->hConnPage == NULL)
    {
        rv = RPOOL_GetPage(pConn->pTransportMgr->hElementPool, 0, &pConn->hConnPage);
        if(rv!= RV_OK)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "GetConnectionAliasFromVia - conn 0x%p Failed to allocate page.",
                pConn));
            return rv;
        }
    }    
    pConn->pAlias.hPage = pConn->hConnPage;
    rv = RPOOL_Append(pConn->pTransportMgr->hElementPool,
                      pConn->pAlias.hPage, 
                      length+1, 
                      RV_TRUE, 
                      &pConn->pAlias.offset);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "GetConnectionAliasFromVia - conn 0x%p: Faild to allocate buffer from pool",
            pConn));  
        return rv;
    }
    pAlias = RPOOL_GetPtr(pConn->pTransportMgr->hElementPool,pConn->pAlias.hPage, pConn->pAlias.offset);
    
    rv = RvSipViaHeaderGetHost(hVia, pAlias, length+1, &length );
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "GetConnectionAliasFromVia - conn 0x%p: Faild to get host from via",
            pConn));   
        pConn->pAlias.offset = UNDEFINED;
        return rv;
    }

    port = RvSipViaHeaderGetPortNum(hVia);
    if(port<=0)
    {
        RvBool              bIp = RV_FALSE;
        
        /* if the host is an ip address, and there is no port - add a default port.
           we are doing it, so we will be able to find this connection later in the hash */
        bIp = SipTransportISIpV4(pAlias);
        
#if(RV_NET_TYPE & RV_NET_IPV6)
        if(bIp == RV_FALSE)
        {
            /* the address was not ipv4 raw ip, may be it is ipv6 raw ip? */
            bIp = SipTransportISIpV6(pAlias);
        }
        
#endif /*#if(RV_NET_TYPE & RV_NET_IPV6)*/

        if(RV_TRUE == bIp)
        {
            port = ChoosePort(hVia);
        }
    }
    
    /*concatenate the port */
    if(port>0)
    {
        pAlias[length-1]=':';
        sprintf((RvChar*)&(pAlias[length]), "%d", port);
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "GetConnectionAliasFromVia - conn 0x%p: did not concatenate a port to the alias host %s",
            pConn, pAlias));
    }
    return RV_OK;
}
/***************************************************************************
* RemoveAllConnectionOwners
* ------------------------------------------------------------------------
* General: remove all owners from the owners list (the list is not destructed)
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
*      hConn - handle to the connection to be set.
*    Output: void
***************************************************************************/
static RvStatus RVCALLCONV RemoveAllConnectionOwners(
                                         RvSipTransportConnectionHandle hConn)
{
    TransportConnection               *pConn;
    RLIST_ITEM_HANDLE                 hListItem;
    TransportConnectionOwnerInfo      *pOwnerInfo;

    pConn  = (TransportConnection*)hConn;
    /*if the state is closing it means that the connection was already destructed*/
    if (pConn->eState==RVSIP_TRANSPORT_CONN_STATE_CLOSING)
    {
        return RV_OK;
    }
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    RLIST_GetHead(pConn->pTransportMgr->hConnectionOwnersPoolList,
                pConn->hConnectionOwnersList,
                &hListItem);

    while (hListItem !=NULL) 
    {
        /*remove from owners hash*/
        pOwnerInfo = (TransportConnectionOwnerInfo*)hListItem;
        if(pOwnerInfo->pHashElement != NULL)
        {
            HASH_RemoveElement(pConn->pTransportMgr->hConnOwnersHash,pOwnerInfo->pHashElement);
            pOwnerInfo->pHashElement = NULL;

        }
        /*remove from list*/
        RLIST_Remove(pConn->pTransportMgr->hConnectionOwnersPoolList,
                    pConn->hConnectionOwnersList,
                    hListItem );

        hListItem = NULL;

        RLIST_GetHead(pConn->pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList,
                      &hListItem);
        
    }
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);

    return RV_OK;
}
/***************************************************************************
 * RemoveAllPendingMessagesOnConnection
 * ------------------------------------------------------------------------
 * General: remove all messages waiting to be send on a given connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - handle to the connection to be set.
 ***************************************************************************/
static RvStatus RVCALLCONV RemoveAllPendingMessagesOnConnection(
                                    RvSipTransportConnectionHandle hConn)
{
    TransportConnection *pConnInfo;
    RLIST_ITEM_HANDLE hListItem;
    RLIST_ITEM_HANDLE nextListElement;
    SipTransportSendInfo    *msg;

    pConnInfo  = (TransportConnection*)hConn;

    if (pConnInfo == NULL)
    {
      return RV_ERROR_NULLPTR;
    }
    RvMutexLock(&pConnInfo->pTransportMgr->connectionPoolLock,pConnInfo->pTransportMgr->pLogMgr);

    if (pConnInfo->hMsgToSendList == NULL)
    { /* Previous processing already has freed the list */
        RvMutexUnlock(&pConnInfo->pTransportMgr->connectionPoolLock,pConnInfo->pTransportMgr->pLogMgr);
        return RV_ERROR_NULLPTR;
    }

    RLIST_GetHead(pConnInfo->pTransportMgr->hMsgToSendPoolList,
                    pConnInfo->hMsgToSendList,
                    &hListItem);
    while (hListItem !=NULL)
    {
           msg = (SipTransportSendInfo*)hListItem;

        RLIST_GetNext(pConnInfo->pTransportMgr->hMsgToSendPoolList,
            pConnInfo->hMsgToSendList,
            hListItem,
            &nextListElement);

        /*free the message page if require*/
        if(msg->bFreeMsg == RV_TRUE && msg->msgToSend != NULL_PAGE)
        {
            RPOOL_FreePage(pConnInfo->pTransportMgr->hMsgPool,msg->msgToSend);
            msg->msgToSend = NULL_PAGE;
        }

        RLIST_Remove(pConnInfo->pTransportMgr->hMsgToSendPoolList,
            pConnInfo->hMsgToSendList,
            hListItem );
        hListItem = nextListElement;

    }
    RvMutexUnlock(&pConnInfo->pTransportMgr->connectionPoolLock,pConnInfo->pTransportMgr->pLogMgr);
    return RV_OK;
}

/***************************************************************************
 * NotifyConnUnsentMessagesStatus
 * ------------------------------------------------------------------------
 * General: Notifies the current connection owners about their unsent
 *          messages .
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - Pointer to the connection to notify its owners about
 *                  its unsent messages .
 ***************************************************************************/
static RvStatus RVCALLCONV NotifyConnUnsentMessagesStatus(
                                    IN TransportConnection *pConn)
{
    RLIST_ITEM_HANDLE     hListItem = NULL;
    RLIST_ITEM_HANDLE     nextListElement = NULL;
    RLIST_ITEM_HANDLE     nextListElementBeforeCallBack = NULL;
    SipTransportSendInfo* pMsgSendInfo;

    if (pConn == NULL)
    {
      return RV_ERROR_NULLPTR;
    }

    if (pConn->hMsgToSendList == NULL)
    { /* Previous processing already has freed the list */
      return RV_ERROR_NULLPTR;
    }

    RLIST_GetHead(pConn->pTransportMgr->hMsgToSendPoolList,
                  pConn->hMsgToSendList, &hListItem);
	/*Safe counter was removed since it was detected that the current number of elements in the list
	  is not predictable*/
    while (hListItem !=NULL )
    {
        pMsgSendInfo = (SipTransportSendInfo*)hListItem;

        /* We take the next element before the callback.
        After the callback we will take the next element again and comapre it.
        If they are not equal - the list of was changed.
        Start to go over it from begining.
           This solution make possible to notify same owner about pMsgSendInfo_NOT_SENT
        status more than once for any message. Therefore the solution will be
        fixed in further versions. */
        RLIST_GetNext(pConn->pTransportMgr->hMsgToSendPoolList,
                      pConn->hMsgToSendList, hListItem,
                      &nextListElementBeforeCallBack);

        if(pMsgSendInfo->bWasNotifiedMsgNotSent == RV_FALSE)
        {
            pMsgSendInfo->bWasNotifiedMsgNotSent = RV_TRUE;
            TransportConnectionNotifyStatus(
                           pConn,
                           pMsgSendInfo->hOwner,
                           pMsgSendInfo,
                           RVSIP_TRANSPORT_CONN_STATUS_MSG_NOT_SENT,
                           RV_FALSE);
        }

        RLIST_GetNext(pConn->pTransportMgr->hMsgToSendPoolList,
                      pConn->hMsgToSendList,
                      hListItem,
                      &nextListElement);

        /* If list was changed - restart over again */
        if(nextListElement != nextListElementBeforeCallBack)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "NotifyConnUnsentMessagesStatus - Connection 0x%p pMsgSendInfoToSend list was changed while notifying - starting over again",
                pConn));
            RLIST_GetHead(pConn->pTransportMgr->hMsgToSendPoolList,
                          pConn->hMsgToSendList, &hListItem);
        }
        else
        {
            hListItem = nextListElement;
        }
    }

   
    return RV_OK;
}


/***************************************************************************
 * TerminateConnectionEventHandler
 * ------------------------------------------------------------------------
 * General: Causes immediate shut-down of the connection:
 *  Changes the connection state to "terminated" state.
 *  Calls StateChanged callback with "terminated" state, and the reason
 *  received as a parameter. Calls Free on the connection being
 *  terminated.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pConnObj - The connection to terminate.
 *            eReason -      The reason for termination.
 ***************************************************************************/
static RvStatus  RVCALLCONV TerminateConnectionEventHandler(IN void    *pConnObj,
                                                            IN RvInt32  reason)
{


    TransportConnection* pConn = (TransportConnection*)pConnObj;
    RvStatus rv;

    if (pConnObj == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return rv;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc, 
        "TerminateConnectionEventHandler - Conn 0x%p - event is out of the event queue", pConn));

    /* Call "state changed" callback */
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_TRANSPORT_TLS == pConn->eTransport &&
        RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED != pConn->eTlsState)
    {
        TransportCallbackTLSStateChange(pConn,
                                RVSIP_TRANSPORT_CONN_TLS_STATE_TERMINATED,
                                RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                                RV_FALSE,RV_FALSE);
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    TransportConnectionChangeState(pConn,RVSIP_TRANSPORT_CONN_STATE_TERMINATED,
        (RvSipTransportConnectionStateChangedReason)reason,RV_FALSE,RV_FALSE);
    TransportConnectionFree(pConn);
    TransportConnectionUnLockEvent(pConn);
    return RV_OK;
}


/***************************************************************************
 * ChangeConnStateEventHandler
 * ------------------------------------------------------------------------
 * General: call the state changed function of a connection.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pConnObj - The connection to terminate.
 *            state -      The connection new state
 ***************************************************************************/
static RvStatus  RVCALLCONV ChangeConnStateEventHandler(IN void    *pConnObj,
                                                        IN RvInt32 state,
                                                        IN RvInt32 notUsed,
                                                        IN RvInt32 objUniqueIdentifier)
{


    TransportConnection* pConn = (TransportConnection*)pConnObj;
    RvStatus rv;

    if (pConnObj == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return rv;
    }

    if(objUniqueIdentifier != pConn->connUniqueIdentifier)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "ChangeConnStateEventHandler - Conn 0x%p - different identifier. Ignore the event.",
            pConn));
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "ChangeConnStateEventHandler - Conn 0x%p - event is out of the event queue", pConn));

    /* Call "state changed" callback */
    rv = TransportConnectionChangeState(pConn,
            (RvSipTransportConnectionState)state,
            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED, RV_FALSE, RV_FALSE);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "ChangeConnStateEventHandler- Connection 0x%p: failed to chage state to %s",
            pConn, SipCommonConvertEnumToStrConnectionState((RvSipTransportConnectionState)state)));
        TransportConnectionClose(pConn, RV_FALSE /*bCloseTlsAsTcp*/);
        TransportConnectionUnLockEvent(pConn);
        return rv;
    }

    TransportConnectionUnLockEvent(pConn);
    RV_UNUSED_ARG(notUsed)
    return RV_OK;
}

/***************************************************************************
 * NotifyConnStatusEventHandler
 * ------------------------------------------------------------------------
 * General: call the notify status function of a connection.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pConnObj - The connection to terminate.
 *            status -      The connection status
 ***************************************************************************/
static RvStatus  RVCALLCONV NotifyConnStatusEventHandler(IN void    *pConnObj,
                                                         IN RvInt32 status,
                                                         IN RvInt32 notUsed,
                                                         IN RvInt32 objUniqueIdentifier)
{
    TransportConnection* pConn = (TransportConnection*)pConnObj;
    RvStatus rv;

    if (pConnObj == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return rv;
    }

    if(objUniqueIdentifier != pConn->connUniqueIdentifier)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc, 
            "NotifyConnStatusEventHandler - Conn 0x%p - different identifier. Ignore the event.",
            pConn));
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc, 
        "NotifyConnStatusEventHandler - Conn 0x%p - event is out of the event queue", pConn));

    /* Call "state changed" callback */
    TransportConnectionNotifyStatus(pConn,NULL,NULL,
                                    (RvSipTransportConnectionStatus)status,RV_FALSE);

    TransportConnectionUnLockEvent(pConn);
    RV_UNUSED_ARG(notUsed)
    return RV_OK;
}


/***************************************************************************
 * MarkOwnersAsNotNotified
 * ------------------------------------------------------------------------
 * General: mark all connections in the connection owners list as not notified.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn -    Handle of the connection
 ***************************************************************************/
static void RVCALLCONV MarkOwnersAsNotNotified(
                            IN TransportConnection* pConn)
{
    RLIST_ITEM_HANDLE                   listElement = NULL;
    TransportConnectionOwnerInfo        *pOwnerInfo;
    RLIST_ITEM_HANDLE                   nextListElement = NULL;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                 "MarkOwnersAsNotNotified- Connection 0x%p marking all owners as not notified",pConn));

    RLIST_GetHead(pConn->pTransportMgr->hConnectionOwnersPoolList,
                    pConn->hConnectionOwnersList,
                  &listElement);

    while (listElement !=NULL )
    {
        pOwnerInfo = (TransportConnectionOwnerInfo*)listElement;
        pOwnerInfo->bWasNotified = RV_FALSE;

        RLIST_GetNext(pConn->pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList,
                      listElement,
                      &nextListElement);

        listElement = nextListElement;
    }
}


/*-------------------- OWNERS HASH FUNCTIONS ----------------------*/
/***************************************************************************
 * OwnersHashFunction
 * ------------------------------------------------------------------------
 * General: call back function for calculating the hash value from the
 *          hash key.
 * Return Value: The value generated from the key.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     key  - The hash key (of type TransportConnectionHashKey)
 ***************************************************************************/
static RvUint  OwnersHashFunction(IN void *key)
{
    ConnectionOwnerHashKey *hashElem = (ConnectionOwnerHashKey *)key;
    RvInt32       i;
    RvUint        s = 0;
    RvInt32       mod = hashElem->pConn->pTransportMgr->connOwnersHashSize;
    static const int base = 1 << (sizeof(char)*8);

    RvChar        buffer[64];
    RvInt32       buffSize;
    RvSprintf(buffer,"%p%p",hashElem->hOwner,hashElem->pConn);
    buffSize = (RvInt32)strlen(buffer);
#if defined(UPDATED_BY_SPIRENT)

    for (i = 0; i < buffSize; i++)
    {
        s += buffer[i];
        s += (s << 10);
        s ^= (s >> 6);
    }

    s += (s << 3);
    s ^= (s >> 11);
    s += (s << 15);

    s %= mod;

#else /* defined(UPDATED_BY_SPIRENT) */
    for (i=0; i<buffSize; i++)
    {
        s = (s*base+buffer[i])%mod;
    }
#endif /* defined(UPDATED_BY_SPIRENT) */
    return s;
}


/***************************************************************************
* OwnersHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashElement  - a new hash element
*           oldHashElement  - an element from the hash
***************************************************************************/
static RvBool OwnersHashCompare(IN void *newHashElement,
                                 IN void *oldHashElement)
{
    ConnectionOwnerHashKey   *newHashElem = (ConnectionOwnerHashKey *)newHashElement;
    ConnectionOwnerHashKey   *oldHashElem = (ConnectionOwnerHashKey *)oldHashElement;

    if(newHashElem->hOwner != oldHashElem->hOwner ||
       newHashElem->pConn != oldHashElem->pConn)
    {
        return RV_FALSE;
    }
    return RV_TRUE;
}


/***************************************************************************
 * OwnersHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a connection owner into the hash table.
 *          The key is generated from the owner info  and the
 *          ownerInfo address is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to insert to the hash table
 *               RV_OK - connection handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn -       Pointer of the call-leg to insert to the hash
 *          pOwner -      The connection owner information.
 ***************************************************************************/
static RvStatus RVCALLCONV OwnersHashInsert(
                        IN TransportConnection*                 pConn,
                        IN TransportConnectionOwnerInfo*        pOwner)
{
    RvStatus                  rv;
    ConnectionOwnerHashKey     hashKey;

    RvBool           wasInsert;

    /*prepare hash key*/
    hashKey.pConn      = pConn;
    hashKey.hOwner     = pOwner->hOwner;

        /*insert to hash and save hash element id in the call-leg*/
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    rv = HASH_InsertElement(pConn->pTransportMgr->hConnOwnersHash,
                            &hashKey,
                            &pOwner,
                            RV_FALSE,
                            &wasInsert,
                            &(pOwner->pHashElement),
                            OwnersHashCompare);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);


    if(rv != RV_OK || wasInsert == RV_FALSE)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "OwnersHashInsert- owner 0x%p - was inserted to hash (conn = 0x%p) ",pOwner,pConn));
    return RV_OK;
}

/***************************************************************************
 * OwnersHashFind
 * ------------------------------------------------------------------------
 * General: find an owner in the owners hash table according to a given handle.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr -    Handle of the transport manager
 *          eTransport    -    The requested transport (Tcp/Tls/Sctp)
 *          hLocalAddr    -    Handle to the requested locat address
 *          pDestAddr     -    pointer to the requested destination address
 * Output:  phConn -           Handle to the connection found in the hash or NULL
 *                             if the connection was not found.
 ***************************************************************************/
static void RVCALLCONV OwnersHashFind(
                         IN TransportConnection                 *pConn,
                         IN RvSipTransportConnectionOwnerHandle hOwner,
                         OUT TransportConnectionOwnerInfo       **ppOwner)
{
    RvStatus                  rv;
    RvBool                    ownerFound;
    void                       *elementPtr;
    ConnectionOwnerHashKey     hashKey;

    /*prepare hash key*/
    hashKey.hOwner = hOwner;
    hashKey.pConn = pConn;
    RvMutexLock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);
    ownerFound = HASH_FindElement(pConn->pTransportMgr->hConnOwnersHash, &hashKey,
                                 OwnersHashCompare, &elementPtr);
    RvMutexUnlock(&pConn->pTransportMgr->connectionPoolLock,pConn->pTransportMgr->pLogMgr);


    /*return if record not found*/
    if(ownerFound == RV_FALSE || elementPtr == NULL)
    {
        *ppOwner = NULL;
        return;
    }
    rv = HASH_GetUserElement(pConn->pTransportMgr->hConnOwnersHash,
                             elementPtr,(void*)ppOwner);
    if(rv != RV_OK)
    {
        *ppOwner = NULL;
        return;
    }
}



/*-------------------- CONNECTION HASH FUNCTIONS ----------------------*/
/***************************************************************************
 * ConnectionHashFunction
 * ------------------------------------------------------------------------
 * General: call back function for calculating the hash value from the
 *          hash key.
 * Return Value: The value generated from the key.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     key  - The hash key (of type TransportConnectionHashKey)
 ***************************************************************************/
static RvUint  ConnectionHashFunction(IN void *key)
{
    TransportConnectionHashKey *hashElem = (TransportConnectionHashKey *)key;
    RvInt32       i;
    RvUint        s = 0;
    RvInt32       mod = hashElem->pTransportMgr->connHashSize;
    static const int base = 1 << (sizeof(char)*8);
    RvChar        buffer[160];
    RvChar        strRemoteAddr[128];
    RvInt32       buffSize;

    if(hashElem->pRemoteAddress != NULL)
    {
        /* the connection was inserted to the hash with remote ip and port */
        RvAddressGetString(hashElem->pRemoteAddress,128,strRemoteAddr);
        RvSnprintf(buffer,sizeof(buffer),"%p%s:%d-%d",hashElem->hLocalAddr,strRemoteAddr,
                  ((hashElem->pRemoteAddress != NULL)?RvAddressGetIpPort(hashElem->pRemoteAddress):-1),
                  hashElem->eTransport);
    }
    else
    {
        /* the connection was inserted to the hash with remote alias */
        RvChar* pAlias = RPOOL_GetPtr(hashElem->pAlias->hPool,
                                      hashElem->pAlias->hPage,
                                      hashElem->pAlias->offset);
        RvSnprintf(buffer,sizeof(buffer),"%p%s-%d",hashElem->hLocalAddr, pAlias, hashElem->eTransport);
    }
    buffSize = (RvInt32)strlen(buffer);
    for (i=0; i<buffSize; i++)
    {
        s = (s*base+buffer[i])%mod;
    }
    return s;
}

/***************************************************************************
 * CompareAliasToIp
 * ------------------------------------------------------------------------
 * General: compares an alias string to an ip address.
 * Return Value: true if equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: ipHashElem    - The hash element with a remote address in an ip format 
 *        aliasHashElem - The hash element with an alias.
 ***************************************************************************/
static RvBool CompareAliasToIp(TransportConnectionHashKey   *ipHashElem,
                               TransportConnectionHashKey   *aliasHashElem)
{
    RvChar        buffer[133];
    RvChar        strRemoteAddr[133];

    RvAddressGetString(ipHashElem->pRemoteAddress,133,strRemoteAddr);
    RvSprintf(buffer,"%s:%d",strRemoteAddr,RvAddressGetIpPort(ipHashElem->pRemoteAddress));

    return RPOOL_CmpToExternal(aliasHashElem->pAlias->hPool, 
                                aliasHashElem->pAlias->hPage,
                                aliasHashElem->pAlias->offset,
                                buffer, RV_FALSE);
}


/***************************************************************************
* TransportConnectionHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:    newHashElement  - a new hash element
*           oldHashElement  - an element from the hash
***************************************************************************/
RvBool TransportConnectionHashCompare(IN void *newHashElement,
                                      IN void *oldHashElement)
{
    TransportConnectionHashKey   *newHashElem = (TransportConnectionHashKey *)newHashElement;
    TransportConnectionHashKey   *oldHashElem = (TransportConnectionHashKey *)oldHashElement;

    /* Check transport types */
    if (newHashElem->eTransport != oldHashElem->eTransport)
    {
        return RV_FALSE;
    }
    /* Check Local Address */
/* SPIRENT_BEGIN */
#if !defined(UPDATED_BY_SPIRENT)
    if (newHashElem->eTransport == RVSIP_TRANSPORT_SCTP)

#endif
/* SPIRENT_END */
    {
        if (newHashElem->hLocalAddr != oldHashElem->hLocalAddr)
        {
            return RV_FALSE;
        }
    }
    
    if(oldHashElem->pRemoteAddress != NULL && newHashElem->pRemoteAddress != NULL)
    {
        /* Check IPs and ports*/
        if (RV_TRUE != TransportMgrRvAddressIsEqual(newHashElem->pRemoteAddress,
                                                oldHashElem->pRemoteAddress))
        {
            return RV_FALSE;
        }
    }
    else /* at least one of the connection keys has an alias */
    {
        if(newHashElem->pRemoteAddress == NULL && oldHashElem->pRemoteAddress == NULL)
        {
            /* compare aliases of 2 keys */
            if(RV_FALSE == RPOOL_Strcmp(oldHashElem->pAlias->hPool, 
                                         oldHashElem->pAlias->hPage,
                                         oldHashElem->pAlias->offset,
                                         newHashElem->pAlias->hPool, 
                                         newHashElem->pAlias->hPage,
                                         newHashElem->pAlias->offset))
            {
                return RV_FALSE;
            }
        }
        else
        {
            /* one hash key contains an ip+port - need to concatenate it, 
               and compare to the alias.*/
            if(newHashElem->pRemoteAddress == NULL)
            {
                if(RV_FALSE == CompareAliasToIp(oldHashElem, newHashElem))
                {
                    return RV_FALSE;
                }
            }
            else
            {
                if(RV_FALSE == CompareAliasToIp(newHashElem, oldHashElem))
                {
                    return RV_FALSE;
                }
            }
        }
    }
    return RV_TRUE;
}

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
void RVCALLCONV TransportConnectionHashFind(
                         IN TransportMgr                   *pTransportMgr,
                         IN RvSipTransport                 eTransport,
                         IN RvSipTransportLocalAddrHandle  hLocalAddress,
                         IN SipTransportAddress            *pDestAddress,
                         IN RPOOL_Ptr                      *pAlias,
                         OUT RvSipTransportConnectionHandle *phConn)
{
    ConnectionHashFind(pTransportMgr, eTransport, hLocalAddress, pDestAddress, pAlias, phConn);
}
/***************************************************************************
 * ConnectionHashFind
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
static void RVCALLCONV ConnectionHashFind(
                         IN TransportMgr                   *pTransportMgr,
                         IN RvSipTransport                 eTransport,
                         IN RvSipTransportLocalAddrHandle  hLocalAddress,
                         IN SipTransportAddress            *pDestAddress,
                         IN RPOOL_Ptr                      *pAlias,
                         OUT RvSipTransportConnectionHandle *phConn)
{
    RvStatus                  rv;
    RvBool                    connFound;
    void                       *elementPtr;
    TransportConnectionHashKey hashKey;

    /*prepare hash key*/
    hashKey.pTransportMgr = pTransportMgr;
    hashKey.eTransport    = eTransport;
    hashKey.hLocalAddr    = hLocalAddress;
    hashKey.pRemoteAddress= (pDestAddress != NULL)?&(pDestAddress->addr):NULL;
    hashKey.pAlias        = pAlias;
    RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
    connFound = HASH_FindElement(pTransportMgr->hConnHash, &hashKey,
                                 TransportConnectionHashCompare, &elementPtr);
    RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);

    if(pDestAddress == NULL && pAlias == NULL)
    {
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ConnectionHashFind: both pDestAddress and pAlias are NULL. "));
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    /* second try */
#ifdef TRANSPORT_SCTP_MIXEDIPV4IPV6_ON
    if(connFound == RV_FALSE || elementPtr == NULL)
    {
        if(pDestAddress != NULL)
        {
            RvSipTransportAddressType eLocalAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
            SipTransportLocalAddressGetAddrType(hashKey.hLocalAddr, &eLocalAddressType);
            
            /* SCTP IPv6 sockets are able to listen on both IPv4 and IPv6
            addresses. Therefore check, if the address in Hash of IPv6 type
            matches the seek address of IPv4 type */
            if (RVSIP_TRANSPORT_SCTP == pDestAddress->eTransport &&
                RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == eLocalAddressType &&
                RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(hashKey.pRemoteAddress))
            {
                RvStatus rv;
                RvAddress remoteAddressIPv6;
                /* Use copy of the Destination Address */
                memcpy(&remoteAddressIPv6, hashKey.pRemoteAddress, sizeof(RvAddress));
                hashKey.pRemoteAddress = &remoteAddressIPv6;
                rv = TransportMgrConvertIPv4RvAddressToIPv6(hashKey.pRemoteAddress);
                if (RV_OK != rv)
                {
                    *phConn = NULL;
                    return;
                }
                RvMutexLock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
                connFound = HASH_FindElement(pTransportMgr->hConnHash, &hashKey,
                    TransportConnectionHashCompare, &elementPtr);
                RvMutexUnlock(&pTransportMgr->connectionPoolLock,pTransportMgr->pLogMgr);
            }
        }    
    }
#endif /* #ifdef TRANSPORT_SCTP_MIXEDIPV4IPV6_ON */
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6)*/
    /*return if record not found*/
    if(connFound == RV_FALSE || elementPtr == NULL)
    {
        *phConn = NULL;
        return;
    }

    rv = HASH_GetUserElement(pTransportMgr->hConnHash,elementPtr,(void*)phConn);
    if(rv != RV_OK)
    {
        *phConn = NULL;
        return;
    }
}


/***************************************************************************
 * ServerConnectionTimerExpiredEvHandler
 * ------------------------------------------------------------------------
 * General: Called when a server connection timer is expired. This is the
 *          indication for closing server connections.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The connection using this timer.
 ***************************************************************************/
static RvBool ServerConnectionTimerExpiredEvHandler(
                                        IN void    *pContext,
                                        IN RvTimer *timerInfo)
{
    RvStatus rv;
    TransportConnection* pConn = (TransportConnection*)pContext;
    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pConn->hServerTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc  ,
            "ServerConnectionTimerExpiredEvHandler - Connection 0x%p : timer expired but was already released. do nothing...",
            pConn));
        
        TransportConnectionUnLockEvent(pConn);
        return RV_FALSE;
    }
    
    SipCommonCoreRvTimerExpired(&pConn->hServerTimer);
    SipCommonCoreRvTimerCancel(&pConn->hServerTimer);
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "ServerConnectionTimerExpiredEvHandler- Connection 0x%p - Release server connection timer 0x%p",pConn, &pConn->hServerTimer));

	RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
		"ServerConnectionTimerExpiredEvHandler- Connection 0x%p - timer expired, closing connection ",pConn));
    TransportConnectionDisconnect(pConn);
    TransportConnectionUnLockEvent(pConn);

    return RV_FALSE;
}

/******************************************************************************
 * NotifyConnectionStateChange
 * ----------------------------------------------------------------------------
 * General: goes througth list of the Connection owners and for each of them
 *          reports the State Change event
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - connection
 *          eConnState  - state to notify
 *          eConnReason - reason of state change to notify
 *****************************************************************************/
static void NotifyConnectionStateChange(
                    IN TransportConnection*                       pConn,
                    IN RvSipTransportConnectionState              eConnState,
                    IN RvSipTransportConnectionStateChangedReason eConnReason)
{
    TransportConnectionOwnerInfo* pOwnerInfo;
    RLIST_ITEM_HANDLE             nextListElementBeforeCallBack = NULL;
    RLIST_ITEM_HANDLE             nextListElement = NULL;
    RLIST_ITEM_HANDLE             listElement = NULL;
    TransportMgr*                 pTransportMgr = pConn->pTransportMgr;

    /* Report state to security owner */
#ifdef RV_SIP_IMS_ON
    if (NULL != pConn->pSecOwner &&
        NULL != pTransportMgr->secEvHandlers.pfnConnStateChangedHandler)
    {
        NotifyConnectionStateChangeToOwner(pConn,
            pTransportMgr->secEvHandlers.pfnConnStateChangedHandler,
            (RvSipTransportConnectionOwnerHandle)pConn->pSecOwner, eConnState,
            eConnReason, "Secure Owner");
    }
#endif /*#ifdef RV_SIP_IMS_ON*/
    
    /* Report state to rest owners */
    MarkOwnersAsNotNotified(pConn);
    RLIST_GetHead(pTransportMgr->hConnectionOwnersPoolList,
                  pConn->hConnectionOwnersList, &listElement);
    while (listElement != NULL)
    {
        pOwnerInfo = (TransportConnectionOwnerInfo*)listElement;

        /* We take the next element before the callback.
        After the callback we will take the next element again and comapre it.
        If they are not equal - the list of owners was changed. */
        RLIST_GetNext(pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList, listElement,
                      &nextListElementBeforeCallBack);

        if(pOwnerInfo->bWasNotified == RV_FALSE)
        {
            pOwnerInfo->bWasNotified = RV_TRUE;
            if(pOwnerInfo->pfnStateChangedEvHandler != NULL)
            {
                NotifyConnectionStateChangeToOwner(pConn,
                    pOwnerInfo->pfnStateChangedEvHandler, pOwnerInfo->hOwner,
                    eConnState, eConnReason, ""/*strCallbackName*/);
            }
        }
        RLIST_GetNext(pTransportMgr->hConnectionOwnersPoolList,
            pConn->hConnectionOwnersList, listElement, &nextListElement);

        /*the list was changed - start over*/
        if(nextListElement != nextListElementBeforeCallBack)
        {
             RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "NotifyConnectionStateChange - Connection 0x%p owner list was changed while notifying - starting over",
                pConn));

             RLIST_GetHead(pTransportMgr->hConnectionOwnersPoolList,
                           pConn->hConnectionOwnersList, &listElement);
        }
        else
        {
             listElement = nextListElement;
        }
        
    }
}

/******************************************************************************
 * NotifyConnectionStateChangeToOwner
 * ----------------------------------------------------------------------------
 * General: unlocks a connection, notifies new state and locks again
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn                   - pointer to connection
 *          pfnStateChangeEvHandler - state change event handler
 *          hConnOwner              - the owner of the connection
 *          eConnState                  - state to notify
 *          eConnReason                 - reason of state change to notify
 *          strCallbackName         - name of the callback for logging
 *****************************************************************************/
static void NotifyConnectionStateChangeToOwner(
        IN TransportConnection*                       pConn,
        IN RvSipTransportConnectionStateChangedEv     pfnStateChangeEvHandler,
        IN RvSipTransportConnectionOwnerHandle        hConnOwner,
        IN RvSipTransportConnectionState              eConnState,
        IN RvSipTransportConnectionStateChangedReason eConnReason,
        IN RvChar*                                    strCallbackName)
{
    RvInt32     nestedLock = 0;
    RvThreadId  currThreadId;
    RvInt32     threadIdLockCount = 0;
    RvStatus    rv;
    
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "NotifyConnectionStateChangeToOwner - Connection 0x%p - before %s callback",
        pConn, strCallbackName));
    
    SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,
        pConn->pTransportMgr->pLogSrc, pConn->pTripleLock, &nestedLock,
        &currThreadId, &threadIdLockCount);

    rv = pfnStateChangeEvHandler((RvSipTransportConnectionHandle)pConn, hConnOwner,
                                 eConnState, eConnReason);

    SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,
        pConn->pTripleLock, nestedLock, currThreadId, threadIdLockCount);
    
    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "NotifyConnectionStateChangeToOwner - Connection 0x%p - after %s callback(rv=%d)",
        pConn, strCallbackName, rv));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(strCallbackName);
#endif

}

/******************************************************************************
 * NotifyConnectionStatus
 * ----------------------------------------------------------------------------
 * General: goes througth list of the Connection owners and for each of them
 *          reports the Status
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - connection
 *          eConnStatus - status
 *          pInfo       - Additional information, to be provided with status
 *****************************************************************************/
static void NotifyConnectionStatus(
                        IN TransportConnection*           pConn,
                        IN RvSipTransportConnectionStatus eConnStatus,
                        IN void*                          pInfo)
{
    RLIST_ITEM_HANDLE             listElement = NULL;
    TransportConnectionOwnerInfo* pOwnerInfo;
    RLIST_ITEM_HANDLE             nextListElement = NULL;
    RLIST_ITEM_HANDLE             nextListElementBeforeCallBack = NULL;
    TransportMgr*                 pTransportMgr = pConn->pTransportMgr;

    MarkOwnersAsNotNotified(pConn);
    RLIST_GetHead(pTransportMgr->hConnectionOwnersPoolList,
                  pConn->hConnectionOwnersList, &listElement);
    while (listElement !=NULL)
    {
        pOwnerInfo = (TransportConnectionOwnerInfo*)listElement;

        /* We take the next element before the callback.
        After the callback we will take the next element again and comapre it.
        If they are not equal - the list of owners was changed. */
        RLIST_GetNext(pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList, listElement,
                      &nextListElementBeforeCallBack);
        
        if(pOwnerInfo->bWasNotified == RV_FALSE)
        {
            pOwnerInfo->bWasNotified = RV_TRUE;
            if(pOwnerInfo->pfnStausEvHandler != NULL)
            {
                NotifyConnectionStatusToOwner(pConn, pOwnerInfo->pfnStausEvHandler,
                    pOwnerInfo->hOwner, eConnStatus, pInfo, "All Owners");
            }
        }
        
        RLIST_GetNext(pTransportMgr->hConnectionOwnersPoolList,
                      pConn->hConnectionOwnersList, listElement,
                      &nextListElement);
        
        /* The list was changed - start over */
        if(nextListElement != nextListElementBeforeCallBack)
        {
            RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "NotifyConnectionStatus - Connection 0x%p owner list was changed while notifying - starting over",
                pConn));
            RLIST_GetHead(pTransportMgr->hConnectionOwnersPoolList,
                          pConn->hConnectionOwnersList, &listElement);
        }
        else
        {
            listElement = nextListElement;
        }
        
    }
}

/******************************************************************************
 * NotifyConnectionStatusToOwner
 * ----------------------------------------------------------------------------
 * General: unlocks a connection, notifies status and locks again
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn               - pointer to connection
 *          pfnStausEvHandler   - status event handler
 *          hConnOwner          - the owner of the connection
 *          eConnStatus         - status to notify
 *          pInfo               - additional info
 *          strCallbackName     - name of the callback for logging
 *****************************************************************************/
static void NotifyConnectionStatusToOwner(
                    IN TransportConnection*                 pConn,
                    IN RvSipTransportConnectionStatusEv     pfnStausEvHandler,
                    IN RvSipTransportConnectionOwnerHandle  hConnOwner,
                    IN RvSipTransportConnectionStatus       eConnStatus,
                    IN void*                                pInfo,
                    IN RvChar*                              strCallbackName)
{
    RvInt32     nestedLock = 0;
    RvThreadId  currThreadId;
    RvInt32     threadIdLockCount = 0;
    RvStatus    rv;

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "NotifyConnectionStatusToOwner - Connection 0x%p - before %s callback",
        pConn, strCallbackName));

    SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,
        pConn->pTransportMgr->pLogSrc, pConn->pTripleLock, &nestedLock,
        &currThreadId, &threadIdLockCount);

    rv = pfnStausEvHandler((RvSipTransportConnectionHandle)pConn,
        hConnOwner,
        eConnStatus,(void*)pInfo);

    SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,
        pConn->pTripleLock, nestedLock, currThreadId, threadIdLockCount);

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "NotifyConnectionStatusToOwner - Connection 0x%p - after %s callback(rv=%d)",
        pConn, strCallbackName, rv));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(strCallbackName);
#endif
}



#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/***************************************************************************
 * ConnectionLock
 * ------------------------------------------------------------------------
 * General: Locks a connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to connection to be locked
 *            pMgr        - pointer to transport manager
 *            hLock        - conn lock
 ***************************************************************************/
static RvStatus RVCALLCONV ConnectionLock(IN TransportConnection *pConn,
                                              IN TransportMgr  *pMgr,
                                              IN SipTripleLock *pTripleLock,
                                              IN RvInt32        identifier)
{

    if ((pConn == NULL) || (pTripleLock == NULL) || (pMgr == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pTripleLock->hLock,pConn->pTransportMgr->pLogMgr); /*lock the conn*/

    if (pConn->connUniqueIdentifier != identifier ||
        pConn->connUniqueIdentifier == 0)
    {
            /*The conn is invalid*/
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                      "ConnectionLock - Conn 0x%p: RLIST_IsElementVacant returned RV_TRUE",
                      pConn));
        RvMutexUnlock(&pTripleLock->hLock,pMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}

/***************************************************************************
 * ConnectionUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks a given lock.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLogMgr - The log mgr pointer.
 *            hLock   - A lock to unlock.
 ***************************************************************************/
static void RVCALLCONV ConnectionUnLock(IN  RvLogMgr   *pLogMgr,
                                        IN  RvMutex       *hLock)
{
   RvMutexUnlock(hLock,pLogMgr);
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
 * ConnectionSelectEventAdd
 * ------------------------------------------------------------------------
 * General: ???
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
static RvStatus ConnectionSelectEventAdd(IN TransportConnection* pConn)
{
    RvStatus crv = RV_OK;

    switch (pConn->eTransport)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        crv = RvSelectAdd(pConn->pTransportMgr->pSelect,
                          &pConn->ccSelectFd,
                          pConn->ccEvent,
                          TransportSctpEventCallback);
        break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_TCP:
        crv = RvSelectAdd(pConn->pTransportMgr->pSelect,
                          &pConn->ccSelectFd,
                          pConn->ccEvent,
                          TransportTcpEventCallback);
        break;
    default:
        crv = RV_ERROR_NOTSUPPORTED;
    }
    return CRV2RV(crv);
}


/***************************************************************************
 * ConnectionSelectEventUpdate
 * ------------------------------------------------------------------------
 * General: ???
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
static RvStatus ConnectionSelectEventUpdate(IN TransportConnection* pConn)
{
    RvStatus crv = RV_OK;

    switch (pConn->eTransport)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        crv = RvSelectUpdate(pConn->pTransportMgr->pSelect,
                             &pConn->ccSelectFd,
                             pConn->ccEvent,
                             TransportSctpEventCallback);
        break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_TCP:
        crv = RvSelectUpdate(pConn->pTransportMgr->pSelect,
                             &pConn->ccSelectFd,
                             pConn->ccEvent,
                             TransportTcpEventCallback);
        break;
    default:
        crv = RV_ERROR_NOTSUPPORTED;
    }
    return CRV2RV(crv);
}


#ifdef __cplusplus
}
#endif

