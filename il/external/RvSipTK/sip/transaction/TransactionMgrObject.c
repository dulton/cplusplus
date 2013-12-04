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
 *                              <TransactionMgr.c>
 *
 *  This files defines an interface between the transaction and the transactions
 *  manager.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include <string.h>

#include "rvmemory.h"
#include "_SipCommonConversions.h"

#include "TransactionMgrObject.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipViaHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipViaHeader.h"
#include "TransactionCallBacks.h"
#include "_SipTransactionMgr.h"
#include "_SipTransport.h"
#include "TransactionTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"
#include "_SipTransmitter.h"
#include "rvansi.h"
#include "AdsCopybits.h"

#ifdef RV_SIGCOMP_ON
#include "_SipCompartment.h"
#endif 

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV TranscHashInit(
                                  IN TransactionMgr *pMgr);

static void RVCALLCONV SetTimersConfigurationValues(
                                  IN    TransactionMgr           *pTranscMgr,
                                  INOUT SipTranscMgrConfigParams *pConfiguration);

static RvBool HashKeyMethodsAreEqual(
                                  IN TransactionMgrHashKey *newHashKey,
                                  IN TransactionMgrHashKey *oldHashKey);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar* HashKeyMethodToStr(IN TransactionMgrHashKey *hashKey);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static RvStatus RVCALLCONV CreateTranscList(
                                  IN TransactionMgr *pTranscMgr,
                                  IN RvInt32   maxNumOfTransc);

static void RVCALLCONV DestructTranscList(
                                  IN TransactionMgr  *pTranscMgr);

static RvBool CompareResponseToClientTransaction(
                                  IN TransactionMgrHashKey *newHashKey,
                                  IN TransactionMgrHashKey *oldHashKey);

static RvBool CompareRequestToServerTransaction(
                                  IN TransactionMgrHashKey *newHashKey,
                                  IN TransactionMgrHashKey *oldHashKey);

static RvBool CompareRequestToClientTransaction(
                                  IN TransactionMgrHashKey *newHashKey,
                                  IN TransactionMgrHashKey *oldHashKey);

static RvBool CompareRequestToTransaction(
                                  IN TransactionMgrHashKey *newHashKey,
                                  IN TransactionMgrHashKey *oldHashKey);

static RvUint HashFunction(void *key);

static RvBool HashCompare(void *Param1 ,void *Param2);

static RvBool HashCompareCancelledTransc(void *newKey ,void *oldKey);

static RvBool BasicHashCompare(void *newKey ,void *oldKey);

static RvBool HashCompareMatchAckTransc(void *newKey ,void *oldKey);

#ifndef RV_SIP_PRIMITIVES
static RvBool HashComparePrackMatchTransc(void *newKey ,void *oldKey);
#endif

static RvBool HashCompareMergedTransc(void *newKey ,void *oldKey);

static void RVCALLCONV DestructAllTransactions(TransactionMgr  *pTranscMgr);

static void RVCALLCONV PrepareCallbackMasks(IN TransactionMgr* pMgr);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionMgrInitialize
 * ------------------------------------------------------------------------
 * General: Called in the Stack initialization process.
 * Initiates data-structures. Sets global memory-pools, timer-pool,
 * log Id number to the manager. Signs transaction default callbacks to the
 * manager.
 * Return Value: RV_OK - The out parameter pTranscMgr now points
 *                            to a valid transaction manager object.
 *                 RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction manager object.
 *                 RV_ERROR_NULLPTR - pConfiguration or phTranscMgr are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr  - The new transaction manager object.
 *          pCfg         - Managers parameters as were defined in the
 *                             configuration.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrInitialize(
                              IN     TransactionMgr            *pTranscMgr,
                              INOUT  SipTranscMgrConfigParams  *pCfg)
{
#ifndef RV_SIP_PRIMITIVES
    RvInt32         i = 0;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    RvStatus        retStatus;
    SipTransportEvHandlers transportEvHandlers;


    pTranscMgr->pTimers = &(pTranscMgr->timers);
    pTranscMgr->pTimers->maxInviteRequestRetransmissions = MAX_INVITE_RETRANSMISSIONS;
    pTranscMgr->pTimers->maxGeneralRequestRetransmissions = UNDEFINED;
    pTranscMgr->pTimers->maxInviteResponseRetransmissions = UNDEFINED;

    /* Set configuration information into the transaction manager*/
    pTranscMgr->pLogMgr                   = pCfg->pLogMgr;
    pTranscMgr->hGeneralPool              = pCfg->hGeneralPool;
    pTranscMgr->hMessagePool              = pCfg->hMessagePool;
    pTranscMgr->pLogSrc                   = pCfg->pLogSrc;
    pTranscMgr->hTransport                = pCfg->hTransportHandle;
    pTranscMgr->hMsgMgr                   = pCfg->hMsgMgr;
    pTranscMgr->maxTransactionNumber      = pCfg->maxTransactionNumber;
    pTranscMgr->enableInviteProceedingTimeoutState =
                                pCfg->enableInviteProceedingTimeoutState;
    pTranscMgr->hStack                    = pCfg->hStack;
    pTranscMgr->hTrxMgr                   = pCfg->hTrxMgr;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* copy the configured maximum number of retransmit "INVITE" messages. ensure this parameter is positive */
    pTranscMgr->maxRetransmitINVITENumber = 
        (pCfg->maxRetransmitINVITENumber > 0 ? pCfg->maxRetransmitINVITENumber : 0);
    pTranscMgr->initialGeneralRequestTimeout = pCfg->initialGeneralRequestTimeout;
#endif
#if defined(UPDATED_BY_SPIRENT_ABACUS)    
    pTranscMgr->pfnRetransmitEvHandler = pCfg->pfnRetransmitEvHandler;
#else
    pTranscMgr->pfnRetransmitEvHandler     = pCfg->pfnRetransmitEvHandler;
    pTranscMgr->pfnRetransmitRcvdEvHandler = pCfg->pfnRetransmitRcvdEvHandler;
#endif    
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    /* Set timers configuration values in the transaction manager */
    SetTimersConfigurationValues(pTranscMgr, pCfg);

    pTranscMgr->seed              = pCfg->seed;
    pTranscMgr->hTrxMgr           = pCfg->hTrxMgr;

#ifdef SIP_DEBUG
    memset(&(pTranscMgr->msgStatistics), 0, sizeof(SipTransactionMgrStatistics));
#endif

#ifdef RV_SIP_AUTH_ON
    pTranscMgr->hAuthenticator          = pCfg->hAuthMgr;
    pTranscMgr->enableServerAuth        = pCfg->enableServerAuth;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
    pTranscMgr->status100rel            = RVSIP_TRANSC_100_REL_UNDEFINED;
#ifdef RV_SIP_SUBS_ON
    pTranscMgr->maxSubscriptions        = pCfg->maxSubscriptions;
#endif /* #ifdef RV_SIP_SUBS_ON */
    pTranscMgr->manualPrack             = pCfg->manualPrack;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    pTranscMgr->manualBehavior          = pCfg->manualBehavior;
    pTranscMgr->bIsPersistent           = pCfg->bIsPersistent;
    pTranscMgr->bDynamicInviteHandling  = pCfg->bDynamicInviteHandling;
    pTranscMgr->bEnableForking          = pCfg->bEnableForking;
    pTranscMgr->bDisableMerging         = pCfg->bDisableMerging;
    pTranscMgr->bOldInviteHandling      = pCfg->bOldInviteHandling;
#ifdef RV_SIGCOMP_ON
    pTranscMgr->sigCompTcpTimer         = pCfg->sigCompTcpTimer;
    pTranscMgr->hCompartmentMgr         = pCfg->hCompartmentMgr;
#endif
#ifdef RV_SIP_IMS_ON
    pTranscMgr->hSecMgr                 = pCfg->hSecMgr;
#endif /* #ifdef RV_SIP_IMS_ON */

    PrepareCallbackMasks(pTranscMgr);

    pTranscMgr->transcTrxEvHandlers.pfnOtherURLAddressFoundEvHandler = TransactionTransportTrxOtherURLAddressFoundEv;
    pTranscMgr->transcTrxEvHandlers.pfnStateChangedEvHandler         = TransactionTransportTrxStateChangeEv;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    pTranscMgr->transcTrxEvHandlers.pfnLastChangeBefSendEvHandler    =  NULL;
    pTranscMgr->mgrTrxEvHandlers.pfnLastChangeBefSendEvHandler       = NULL;
#endif
/* SPIRENT_END */

    pTranscMgr->mgrTrxEvHandlers.pfnOtherURLAddressFoundEvHandler = TransactionTransportOutOfContextTrxOtherURLAddressFoundEv;
    pTranscMgr->mgrTrxEvHandlers.pfnStateChangedEvHandler         = TransactionTransportOutOfContextTrxStateChangeEv;

    memset(&pTranscMgr->connEvHandlers,0,sizeof(pTranscMgr->connEvHandlers));
    pTranscMgr->connEvHandlers.pfnConnStateChangedEvHandler                   = TransactionTransportConnStateChangedEv;

    RvSelectGetThreadEngine(pTranscMgr->pLogMgr, &pTranscMgr->pSelect);
    /*-------------------------------
    Create the transaction list
    ---------------------------------*/
    retStatus = CreateTranscList(pTranscMgr,pCfg->maxTransactionNumber);

    if (retStatus != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "TransactionMgrInitialize - Failed to construct the transaction list pool"));
        return retStatus;
    }

    /*--------------------------------------------------------------------
     Set default callbacks - only the state change has a default callback.
    ----------------------------------------------------------------------*/
    (pTranscMgr->pDefaultEvHandlers).pfnEvStateChanged =
                                           TransactionCallBacksNoOwnerEvStateChanged;

    /*setting the managers event handlers to null*/
    memset(&(pTranscMgr->pCallMgrDetail),0,sizeof(SipTransactionMgrEvHandlers));
    memset(&(pTranscMgr->pMgrEvHandlers),0,sizeof(RvSipTransactionMgrEvHandlers));

    /* Set application callbacks to NULL */
    memset(&(pTranscMgr->pAppEvHandlers),0,sizeof(RvSipTransactionEvHandlers));

    pTranscMgr->pAppMgr                               = NULL;


   /*-------------------------------------
    Initialize the transactions hash table
    ---------------------------------------*/
    retStatus = TranscHashInit(pTranscMgr);
    if(retStatus != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "TransactionMgrInitialize - Failed to initialize transcation hash table"));
        return retStatus;
    }


    /*-------------------------------------------
       Set the transport call-backs
       -------------------------------------------*/
    transportEvHandlers.pfnMsgRcvdEvHandler = SipTransactionMgrMessageReceivedEv;


    retStatus = SipTransportSetEventHandler(pTranscMgr->hTransport,
                                            &transportEvHandlers,
                                            sizeof(transportEvHandlers),
                                            (SipTransportCallbackContextHandle)pTranscMgr);

    if (retStatus != RV_OK)
    {
        return retStatus;
    }


#ifndef RV_SIP_PRIMITIVES
    for(i=0;i<pCfg->extensionListSize;i++)
    {
        if(strcmp(pCfg->supportedExtensionList[i],"100rel") == 0)
        {
            pTranscMgr->status100rel = RVSIP_TRANSC_100_REL_SUPPORTED;
            break;
        }
    }
    pTranscMgr->extensionListSize = pCfg->extensionListSize;
    pTranscMgr->supportedExtensionList = pCfg->supportedExtensionList;
    pTranscMgr->rejectUnsupportedExtensions = pCfg->rejectUnsupportedExtensions;
    pTranscMgr->addSupportedListToMsg = pCfg->addSupportedListToMsg;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    pTranscMgr->isProxy = pCfg->isProxy;
    pTranscMgr->hTransportAddr = NULL_PAGE;
    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "TransactionMgrInitialize - pfnEvOutOfContextMsg=0x%p",
                  pTranscMgr->proxyEvHandlers.pfnEvOutOfContextMsg));

    return RV_OK;
}


/***************************************************************************
 * TransactionMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the transaction manager freeing all resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr          - The transaction manager object.
 ***************************************************************************/
void RVCALLCONV TransactionMgrDestruct(
                              IN     TransactionMgr            *pTranscMgr)
{

    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

    DestructAllTransactions(pTranscMgr);

    DestructTranscList(pTranscMgr);

    if(pTranscMgr->hHashTable != NULL)
    {
        HASH_Destruct(pTranscMgr->hHashTable);
        pTranscMgr->hHashTable = NULL;
    }

    RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
              "TransactionMgrDestruct - The transaction manager was destructed"));

	RvMutexDestruct(&pTranscMgr->hSwitchSafeLock, pTranscMgr->pLogMgr);
	
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    RvMutexDestruct(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
	
	

    RvMemoryFree((void*)pTranscMgr, pTranscMgr->pLogMgr);
}



/***************************************************************************
 * TransactionMgrHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a transaction into the hash table.
 *          The key is generated from the transaction information and the
 *          transaction handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No more resources.
 *               RV_OK - transaction handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - A handle to the transaction to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrHashInsert(RvSipTranscHandle hTransc)
{
    RvStatus             rv;
    TransactionMgrHashKey hashKey;
    RvBool               wasInsert;
    Transaction          *pTransc;

    pTransc = (Transaction *)hTransc;

    /*return if the element is already in the hash*/
    if(pTransc->hashElementPtr !=NULL)
    {
        return RV_OK;
    }

    /*prepare hash key*/
	SipCommonCSeqGetStep(&pTransc->cseq,&hashKey.cseqStep);
    hashKey.pCallId        = &(pTransc->strCallId);
    hashKey.hFrom          = &(pTransc->hFromHeader);
    hashKey.hTo            = &(pTransc->hToHeader);
    hashKey.bIsUac         = pTransc->bIsUac;
    hashKey.bSendRequest   = RV_FALSE;
    hashKey.eMethod        = pTransc->eMethod;
    hashKey.strMethod      = pTransc->strMethod;
    hashKey.pMgr           = pTransc->pMgr;
    hashKey.hRequestUri    = pTransc->hRequestUri;
    hashKey.hTopVia        = &(pTransc->hTopViaHeader);
    hashKey.pRespCode      = &(pTransc->responseCode);
#ifndef RV_SIP_PRIMITIVES
    hashKey.rseqVal       = &(pTransc->localRSeq.val);
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    hashKey.bIsProxy      = &(pTransc->bIsProxy);
    hashKey.bAllowCancellation = &(pTransc->bAllowCancellation);
    hashKey.bAllowAckHandling = &(pTransc->bAllowAckHandling);
    /*insert to hash and save hash element id in the transaction*/
    RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);

    rv = HASH_InsertElement(pTransc->pMgr->hHashTable,
                            &hashKey,
                            &pTransc,
                            RV_FALSE,
                            &wasInsert,
                            &(pTransc->hashElementPtr),
                            HashCompare);
    RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
    if(rv != RV_OK ||wasInsert == RV_FALSE)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionMgrHashInsert - Failed to insert Transc=0x%p to hash",pTransc));
        return RV_ERROR_OUTOFRESOURCES;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionMgrHashInsert - Transc=0x%p was inserted to hash",pTransc));
    return RV_OK;
}


/***************************************************************************
 * TransactionMgrHashRemove
 * ------------------------------------------------------------------------
 * General: Removes a transaction from the hash table.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - A handle to the transaction to remove from the hash
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashRemove(RvSipTranscHandle hTransc)
{
    Transaction          *pTransc;
    pTransc = (Transaction *)hTransc;

    if(pTransc->hashElementPtr != NULL)
    {
        RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
        HASH_RemoveElement(pTransc->pMgr->hHashTable, pTransc->hashElementPtr);
        RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
        pTransc->hashElementPtr = NULL;
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionMgrHashRemove - Transc=0x%p was removed from hash",pTransc));

    }
}


/***************************************************************************
 * TransactionMgrHashFind
 * ------------------------------------------------------------------------
 * General: find a transaction in the hash table acording to the transaction
 *          hash key.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          pKey -       The transaction hash key
 *          eMethod-     Method for calling the right comparison function.
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFind(
                           IN  TransactionMgr         *pMgr,
                           IN  TransactionMgrHashKey  *pKey,
                           OUT RvSipTranscHandle      *phTransaction,
                           RvSipMethodType             eMethod)
{
    RvStatus         rv;
    RvBool           transactionFound;
    void              *elementPtr;

    RvMutexLock(&pMgr->hLock,pMgr->pLogMgr);

    if(RVSIP_METHOD_ACK == eMethod)
    {
        transactionFound = HASH_FindElement(pMgr->hHashTable, pKey,
                                            HashCompareMatchAckTransc,&elementPtr);
    }
    else
    {
        transactionFound = HASH_FindElement(pMgr->hHashTable, pKey,
                                             HashCompare,&elementPtr);
    }


    /*return if record not found*/
    if(transactionFound == RV_FALSE || elementPtr==NULL)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)phTransaction);
    if(rv != RV_OK)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
}

/***************************************************************************
 * TransactionMgrHashFindCancelledTransc
 * ------------------------------------------------------------------------
 * General: find a transaction that a cancel transaction is cancelling.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          pCancelTransc - The cancel transaction.
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindCancelledTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipTranscHandle     hCancelTransc,
                           OUT RvSipTranscHandle     *phTransaction)
{
    RvStatus         rv;
    RvBool           transactionFound;
    void*              elementPtr;
    Transaction       *pCancelTransc = (Transaction*)hCancelTransc;
    TransactionMgrHashKey key;

    RvMutexLock(&pMgr->hLock,pMgr->pLogMgr);


	SipCommonCSeqGetStep(&pCancelTransc->cseq,&key.cseqStep); 
    key.bIsUac = RV_FALSE;  /*can cancel only server transactions*/
    key.eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    key.strMethod = NULL;
    key.hFrom = &(pCancelTransc->hFromHeader);
    key.hTo = &(pCancelTransc->hToHeader);
    key.hRequestUri = pCancelTransc->hRequestUri;
    key.pCallId = &(pCancelTransc->strCallId);
    key.pMgr = pCancelTransc->pMgr;
    key.hTopVia = &(pCancelTransc->hTopViaHeader);

    transactionFound = HASH_FindElement(pMgr->hHashTable, &key,
                                        HashCompareCancelledTransc,&elementPtr);

    /*return if record not found*/
    if(transactionFound == RV_FALSE || elementPtr == NULL)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)phTransaction);
    if(rv != RV_OK)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
}

/***************************************************************************
 * TransactionMgrHashFindCancelledTranscByMsg
 * ------------------------------------------------------------------------
 * General: find a transaction that a cancel msg is supposed to cancel.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          hCancelMsg - The cancel message
 *          pTranscKey - A transaction key retrieved from the message
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindCancelledTranscByMsg(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipMsgHandle         hCancelMsg,
                           IN  SipTransactionKey     *pTranscKey,
                           OUT RvSipTranscHandle     *phTransaction)
{
    RvStatus                 rv;
    RvBool                   transactionFound;
    void*                     elementPtr;
    RvSipHeaderListElemHandle hListElem;
    TransactionMgrHashKey     key;
    RvSipAddressHandle        hRequestUri = RvSipMsgGetRequestUri(hCancelMsg);
    RvSipViaHeaderHandle      hTopVia     = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(
                                                 hCancelMsg,RVSIP_HEADERTYPE_VIA,
                                                 RVSIP_FIRST_HEADER, &hListElem);

    if(hTopVia == NULL || hRequestUri == NULL)
    {
        *phTransaction = NULL;
    }

    key.bIsUac     = RV_FALSE;  /*can cancel only server transactions*/
    key.cseqStep   = pTranscKey->cseqStep;
    key.eMethod    = SIP_TRANSACTION_METHOD_UNDEFINED;
    key.strMethod  = NULL;
    key.hFrom      = &(pTranscKey->hFromHeader);
    key.hTo        = &(pTranscKey->hToHeader);
    key.hRequestUri= hRequestUri;
    key.pCallId    = &(pTranscKey->strCallId);
    key.pMgr       = pMgr;
    key.hTopVia    = &hTopVia;

    RvMutexLock(&pMgr->hLock,pMgr->pLogMgr);
    transactionFound = HASH_FindElement(pMgr->hHashTable, &key,
                                        HashCompareCancelledTransc,&elementPtr);

    /*return if record not found*/
    if(transactionFound == RV_FALSE || elementPtr == NULL)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)phTransaction);
    if(rv != RV_OK)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
}

/***************************************************************************
 * TransactionMgrHashFindMergedTransc
 * ------------------------------------------------------------------------
 * General: find a transaction with the same request identifiers:
 *          from-tag, call-id, and cseq, and the transaction is on ongoing state.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr -       Handle of the transaction manager
 *          pTranscKey - A transaction key retrieved from the message
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindMergedTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipMsgHandle        hReqMsg,
                           IN  SipTransactionKey     *pTranscKey,
                           OUT RvSipTranscHandle     *phTransaction)
{
    RvStatus                 rv;
    RvBool                   bTransactionFound;
    void*                    elementPtr;
    TransactionMgrHashKey    hashKey;
    RvChar                   strMethod[33];
    RvSipCSeqHeaderHandle    hCSeqHeader;
    RvUint                   size;
    RvSipMethodType          eMethod;

    *phTransaction = NULL;

    RvMutexLock(&pMgr->hLock,pMgr->pLogMgr);

    /* Set transaction method*/
    hCSeqHeader                = RvSipMsgGetCSeqHeader(hReqMsg);
    eMethod                    = RvSipCSeqHeaderGetMethodType(hCSeqHeader);
    hashKey.strMethod        = NULL;
    switch (eMethod)
    {
    case RVSIP_METHOD_INVITE:
    case RVSIP_METHOD_ACK:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_INVITE;
        break;
    case RVSIP_METHOD_BYE:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_BYE;
        break;
    case RVSIP_METHOD_REGISTER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_REGISTER;
        break;
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    case RVSIP_METHOD_REFER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_REFER;
        break;
    case RVSIP_METHOD_SUBSCRIBE:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_SUBSCRIBE;
        break;
    case RVSIP_METHOD_NOTIFY:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_NOTIFY;
        break;
#endif /* #ifdef RV_SIP_SUBS_ON */
    case RVSIP_METHOD_PRACK:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_PRACK;
        break;
#endif
    case RVSIP_METHOD_CANCEL:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_CANCEL;
        break;

    case RVSIP_METHOD_OTHER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_OTHER;
        rv = RvSipCSeqHeaderGetStrMethodType(
                                        hCSeqHeader, strMethod, 32, &size);
        if (RV_OK != rv)
        {
            *phTransaction = NULL;
            RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
            return;
        }
        hashKey.strMethod = strMethod;
        break;
    default:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    }

    hashKey.bIsUac         = RV_FALSE;
    hashKey.cseqStep       = pTranscKey->cseqStep;
    hashKey.hFrom          = &(pTranscKey->hFromHeader);
    hashKey.hTo            = &(pTranscKey->hToHeader);
    hashKey.pCallId        = &(pTranscKey->strCallId);
    hashKey.pMgr           = pMgr;

    bTransactionFound = HASH_FindElement(pMgr->hHashTable, &hashKey,
                                        HashCompareMergedTransc,&elementPtr);

    /*return if record not found*/
    if(bTransactionFound == RV_FALSE || elementPtr == NULL)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)phTransaction);
    if(rv != RV_OK)
    {
        *phTransaction = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
}


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionMgrHashFindMatchPrackTransc
 * ------------------------------------------------------------------------
 * General: find a transaction that a prack transaction relate to.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          hPrackTransc - The cancel transaction.
 *          hPrackMsg - The prack message
 * Output:  phPrackMatch - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindMatchPrackTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipTranscHandle     hPrackTransc,
                           IN  RvSipMsgHandle         hPrackMsg,
                           OUT RvSipTranscHandle     *phPrackMatch)
{
    RvStatus         rv;
    RvBool           transactionFound;
    void*           elementPtr;
    Transaction       *pPrackTransc = (Transaction*)hPrackTransc;
    TransactionMgrHashKey     key;
    RvSipRAckHeaderHandle     hRAck;
    RvSipCSeqHeaderHandle     hCSeq;
    RvSipHeaderListElemHandle hListElem;
    SipRSeq					  rseq;
    RvSipMethodType           msgMethod;


    *phPrackMatch = NULL;
    hRAck = (RvSipRAckHeaderHandle)RvSipMsgGetHeaderByType(
                                         hPrackMsg,RVSIP_HEADERTYPE_RACK,
                                         RVSIP_FIRST_HEADER,&hListElem);
    if(hRAck == NULL)
    {
        return;
    }
    hCSeq = RvSipRAckHeaderGetCSeqHandle(hRAck);
    if(hCSeq == NULL)
    {
        return;
    }
	rv = RvSipRAckHeaderIsResponseNumInitialized(hRAck,&rseq.bInitialized); 
	if (RV_OK != rv || RV_FALSE == rseq.bInitialized)
	{
		RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransactionMgrHashFindMatchPrackTransc: PrackTransc 0x%p - RSeq is uninitialized)", 
			pPrackTransc));
		return; 
	}
    rseq.val  = RvSipRAckHeaderGetResponseNum(hRAck);
    msgMethod = RvSipCSeqHeaderGetMethodType(hCSeq);
    /*we support prack only on invite*/
    if(msgMethod != RVSIP_METHOD_INVITE)
    {
        return;
    }

    RvMutexLock(&pMgr->hLock,pMgr->pLogMgr);

	rv = SipCSeqHeaderGetInitializedCSeq(hCSeq,&key.cseqStep); 
	if (RV_OK != rv)
	{
		RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransactionMgrHashFindMatchPrackTransc: PrackTransc 0x%p - SipCSeqHeaderGetInitializedCSeq() failed (CSeq might be uninitialized)", 
			pPrackTransc));
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
	}
    key.bIsUac		 = RV_FALSE;  /*can cancel only server transactions*/
    key.eMethod		 = SIP_TRANSACTION_METHOD_INVITE;
    key.strMethod    = NULL;
    key.hFrom        = &(pPrackTransc->hFromHeader);
    key.hTo			 = &(pPrackTransc->hToHeader);
    key.pCallId		 = &(pPrackTransc->strCallId);
    key.pMgr		 = pPrackTransc->pMgr;
    key.rseqVal		 = &rseq.val;
    transactionFound = HASH_FindElement(pMgr->hHashTable, &key,
                                        HashComparePrackMatchTransc,
                                        &elementPtr);

    /*return if record not found*/
    if(transactionFound == RV_FALSE || elementPtr == NULL)
    {
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)phPrackMatch);
    if(rv != RV_OK)
    {
        *phPrackMatch = NULL;
        RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
        return;
    }
    RvMutexUnlock(&pMgr->hLock,pMgr->pLogMgr);
}
#endif /* #ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
 * TransactionMgrCreateTransaction
 * ------------------------------------------------------------------------
 * General: Creates and Initialize a new transaction object.
 * Return Value: RV_OK - The transaction was successfully inserted to
 *                              the data-structure.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction that needs
 *                                 to be managed is a NULL pointer, or the
 *                                 pointer to the transaction manager is a
 *                                 NULL pointer.
 *                 RV_ERROR_OUTOFRESOURCES - The data-structure is full.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc - The transaction to be managed by the transaction
 *                         manager, in other words the transaction that will
 *                         be inserted to the data-structure.
 *          b_isUac    - RV_TRUE if the transaction is UAC. RV_FALSE for UAS.
 *          pTranscMgr -   The transactions manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrCreateTransaction(
                                     IN  TransactionMgr    *pTranscMgr,
                                     IN  RvBool           b_isUac,
                                     OUT RvSipTranscHandle *hTransc)
{
    RvStatus          rv;
    Transaction        *pTransc;
    RLIST_ITEM_HANDLE  listItem;


    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    *hTransc = NULL;
  
    RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
              "TransactionMgrCreateTransaction - About to create a new transaction"));

    /* Insert the new transaction to the end of the transaction's list.
       Insert allocates memory for the new transaction as well */
    rv = RLIST_InsertTail(pTranscMgr->hTranasactionsListPool ,
                          pTranscMgr->hTranasactionsList,
                          &listItem);

    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        *hTransc = NULL;
        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionMgrCreateTransaction - Failed to insert new transaction to list (rv=%d)",
                  rv));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    pTransc = (Transaction*)listItem;

    rv = TransactionInitialize((RvSipTranscMgrHandle)pTranscMgr,pTransc,b_isUac);
    if(RV_OK != rv)
    {
        RLIST_Remove(pTranscMgr->hTranasactionsListPool ,
                     pTranscMgr->hTranasactionsList,
                     (RLIST_ITEM_HANDLE)pTransc);
        *hTransc = NULL;
        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionMgrCreateTransaction - Failed to initialize the new transaction (rv=%d)",
                  rv));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    *hTransc = (RvSipTranscHandle)pTransc;
    RvLogInfo(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
              "TransactionMgrCreateTransaction - transaction created: 0x%p",pTransc));

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}

/***************************************************************************
 * TransactionMgrCheckIfExists
 * ------------------------------------------------------------------------
 * General: Checks if there exist a transaction that has the same key as the
 *          received key.
 * Return Value: RV_TRUE - The transaction exists.
 *               RV_FALSE - The transaction does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTranscMgr - The transaction manager in which to look for the
 *                       transaction.
 *          pKey - The transaction's key.
 *          eMethod - The transaction enumeration method.
 *          strMethod - The transaction string method.
 *          bIsUac - Is the transaction Uac.
 *          hMsg - message handle to retrive request method and top via.
 ***************************************************************************/
RvBool RVCALLCONV TransactionMgrCheckIfExists(
                                        IN TransactionMgr        *pTranscMgr,
                                        IN SipTransactionKey     *pKey,
                                        IN SipTransactionMethod eMethod,
                                        IN RvChar               *strMethod,
                                        IN RvBool                bIsUac,
                                        IN RvSipMsgHandle         hMsg)
{
    TransactionMgrHashKey hashKey;
    RvSipTranscHandle       hTransc;
    RvSipMethodType           eMsgMethod;
    RvSipHeaderListElemHandle hPos;
    RvSipViaHeaderHandle hViaFromMsg = NULL;
    RvBool              bIsProxy = RV_FALSE;

    if ((NULL == pTranscMgr) ||
        (NULL == pKey) ||
        (NULL == pKey->hFromHeader) ||
        (NULL == pKey->hToHeader)||
        (UNDEFINED == pKey->strCallId.offset) ||
        (NULL == pKey->strCallId.hPool) ||
        (NULL_PAGE == pKey->strCallId.hPage))
    {
        return RV_FALSE;
    }

    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    /* Check the transactions hash to see if a transaction with the
       same key as the received key already exist in the list */
    hashKey.bIsUac         = bIsUac;
    if(bIsUac == RV_TRUE)
    {   /* If we are here and the transaction is client transaction,
           we send a request message */
        hashKey.bSendRequest   = RV_TRUE;
    }
    hashKey.cseqStep       = pKey->cseqStep;
    hashKey.eMethod        = eMethod;
    hashKey.hFrom          = &(pKey->hFromHeader);
    hashKey.hTo            = &(pKey->hToHeader);
    hashKey.pCallId        = &(pKey->strCallId);
    hashKey.pMgr           = pTranscMgr;
    hashKey.strMethod      = strMethod;
    hViaFromMsg = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_VIA,
                                                     RVSIP_FIRST_HEADER, &hPos);
    hashKey.hRequestUri    = RvSipMsgGetRequestUri(hMsg);

    hashKey.hTopVia = &hViaFromMsg;

    /*TBD: shuld we need to know if the sending transaction is a porxy transaction or not ???*/
    /*Look for existing request, at this point we dont know if the request message
      to be sent is a porxy request or not ?????*/
    hashKey.bIsProxy = &bIsProxy;
    hashKey.pRespCode = NULL;
    eMsgMethod = RvSipMsgGetRequestMethod(hMsg);
    TransactionMgrHashFind(pTranscMgr, &hashKey, &hTransc, eMsgMethod);
    if (NULL == hTransc)
    {
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return RV_FALSE;
    }
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_TRUE;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionMgrFreeUnusedSigCompResources
 * ------------------------------------------------------------------------
 * General: Frees unused SigComp resources.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr     - The transaction manager.
 *          pSigCompInfo   - SigComp information structure.
 ***************************************************************************/
void RVCALLCONV TransactionMgrFreeUnusedSigCompResources(
                           IN TransactionMgr             *pTranscMgr,
                           IN SipTransportSigCompMsgInfo *pSigCompInfo)
{
    RvStatus rv;

    if (pSigCompInfo != NULL && pSigCompInfo->bExpectComparmentID == RV_TRUE)
    {
        RvSipCompartmentHandle hInternalComp =
                    (RvSipCompartmentHandle)RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID;

        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "TransactionMgrFreeUnusedSigCompResources - TransportMgr 0x%p frees SigComp unique ID %d",
                  pTranscMgr,pSigCompInfo->uniqueID));

        /* If the compartment handle is RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID */
        /* the unique ID is "released" */
        rv = SipCompartmentRelateCompHandleToInMsg(
                pTranscMgr->hCompartmentMgr,NULL,NULL,NULL,NULL,NULL,pSigCompInfo->uniqueID,&hInternalComp);
        if (rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "TransactionMgrFreeUnusedSigCompResources - TransportMgr 0x%p failed to free SigComp unique ID %d",
                  pTranscMgr,pSigCompInfo->uniqueID));
        }

        pSigCompInfo->bExpectComparmentID = RV_FALSE;
    }
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscHashInit
 * ------------------------------------------------------------------------
 * General: Initialize the transactions hash table. the number of entries is
 *          maxTransactionNumber*2+1.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to construct hash table
 *               RV_OK - hash table was constructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       The transactions manager.
 *          pLogSrcForPools - A handle to the log.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscHashInit(IN TransactionMgr *pMgr)
{
    pMgr->hashSize = HASH_DEFAULT_TABLE_SIZE(pMgr->maxTransactionNumber);
    pMgr->hHashTable = HASH_Construct(
                           HASH_DEFAULT_TABLE_SIZE(pMgr->maxTransactionNumber),
                           pMgr->maxTransactionNumber,
                           HashFunction,
                           sizeof(Transaction*),
                           sizeof(TransactionMgrHashKey),
                           pMgr->pLogMgr,
                           "Transaction Hash");
    if(pMgr->hHashTable == NULL)
        return RV_ERROR_OUTOFRESOURCES;
    return RV_OK;
}


/***************************************************************************
 * CreateTranscList
 * ------------------------------------------------------------------------
 * General: Allocated the list of call-legs managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of transc to allocate
 ***************************************************************************/
static RvStatus RVCALLCONV CreateTranscList(
                               IN TransactionMgr *pTranscMgr,
                               IN RvInt32   maxNumOfTransc)
{
    RvInt32        i;
    Transaction        *pTrans;
    RvStatus       rv;

    /*allocate a pool with 1 list*/
    pTranscMgr->hTranasactionsListPool = RLIST_PoolListConstruct(
                                          maxNumOfTransc,
                                          1, sizeof(Transaction),
                                          pTranscMgr->pLogMgr,
                                          "Transaction List");

    if (NULL == pTranscMgr->hTranasactionsListPool)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "CreateTranscList - Failed to construct the transaction list pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }


    pTranscMgr->hTranasactionsList = RLIST_ListConstruct(
                                        pTranscMgr->hTranasactionsListPool);
    if (NULL == pTranscMgr->hTranasactionsList)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "CreateTranscList - Failed to construct the transaction list"));
        RLIST_PoolListDestruct(pTranscMgr->hTranasactionsListPool);
        pTranscMgr->hTranasactionsListPool = NULL;
        pTranscMgr->hTranasactionsList = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }


    /* initiate locks of all calls */
    for (i=0; i < maxNumOfTransc; i++)
    {
        rv = RLIST_InsertTail(pTranscMgr->hTranasactionsListPool,
                              pTranscMgr->hTranasactionsList,
                              (RLIST_ITEM_HANDLE *)&pTrans);
        pTrans->pMgr = pTranscMgr;
        rv = SipCommonConstructTripleLock(
                        &pTrans->transactionTripleLock, pTranscMgr->pLogMgr);
        if (rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "CreateTranscList - SipCommonConstructTripleLock Failed"));
            DestructTranscList(pTranscMgr);
            return rv;
        }
        pTrans->tripleLock = NULL;
    }

    /* initiate locks of all transactions */
    for (i=0; i < maxNumOfTransc; i++)
    {
        RLIST_GetHead(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE *)&pTrans);
        RLIST_Remove(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE)pTrans);
    }
    return RV_OK;
}

/***************************************************************************
 * DestructAllTransactions
 * ------------------------------------------------------------------------
 * General: Destructs all the transactions
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of calls to allocate
 ***************************************************************************/
static void RVCALLCONV DestructAllTransactions(TransactionMgr  *pTranscMgr)
{

    Transaction    *pNextToKill = NULL;
    Transaction    *pTrans2Kill = NULL;

    if(pTranscMgr->hTranasactionsListPool==NULL)
    {
        return;
    }

    RLIST_GetHead(pTranscMgr->hTranasactionsListPool,
        pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE *)&pTrans2Kill);

    while (NULL != pTrans2Kill)
    {
        RLIST_GetNext(pTranscMgr->hTranasactionsListPool,pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE)pTrans2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
        pTrans2Kill->hTrx = NULL;
        pTrans2Kill->hCancelTranscPair = NULL;
		TransactionDestruct(pTrans2Kill);
        pTrans2Kill = pNextToKill;
    }
}

/***************************************************************************
 * DestructTranscList
 * ------------------------------------------------------------------------
 * General: Destructs the list of transactions
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of calls to allocate
 ***************************************************************************/
static void RVCALLCONV DestructTranscList(TransactionMgr  *pTranscMgr)
{

    RvInt32    i;
    Transaction    *pTrans;
    if(pTranscMgr->hTranasactionsListPool == NULL)
    {
        return;
    }

    for (i=0; i < pTranscMgr->maxTransactionNumber; i++)
    {
        RLIST_GetHead(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE *)&pTrans);
        if (pTrans == NULL)
            break;
        RLIST_Remove(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE)pTrans);
    }
    for (i=0; i < pTranscMgr->maxTransactionNumber; i++)
    {
        RLIST_InsertTail(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE *)&pTrans);
    }

    for (i=0; i < pTranscMgr->maxTransactionNumber; i++)
    {
        RLIST_GetHead(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE *)&pTrans);
        SipCommonDestructTripleLock(
            &pTrans->transactionTripleLock, pTranscMgr->pLogMgr);
        RLIST_Remove(pTranscMgr->hTranasactionsListPool,
            pTranscMgr->hTranasactionsList,(RLIST_ITEM_HANDLE)pTrans);
    }

    RLIST_PoolListDestruct(pTranscMgr->hTranasactionsListPool);
    pTranscMgr->hTranasactionsListPool = NULL;

}

/***************************************************************************
 * SetTimersConfigurationValues
 * ------------------------------------------------------------------------
 * General: Set the configuration values to the transaction manager.
 *          Set the values for all timers.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pConfiguration - A struct containing all relevant configuration
 *                             values.
 *          pTranscMgr - The transactions manager.
 ***************************************************************************/
static void RVCALLCONV SetTimersConfigurationValues(
                              IN    TransactionMgr             *pTranscMgr,
                              INOUT SipTranscMgrConfigParams *pConfiguration)
{
    /* Set configuration values in the transaction manager */
    if (-1 >= (RvInt32)pConfiguration->T1)
    {
        pTranscMgr->pTimers->T1Timeout = DEFAULT_T1;
        pConfiguration->T1 = DEFAULT_T1;
    }
    else
    {
        pTranscMgr->pTimers->T1Timeout = pConfiguration->T1;
    }
    if (DEFAULT_T2 > (RvInt32)pConfiguration->T2)
    {
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "SetTimersConfigurationValues - Default value %d was given to T2 configuration value. T2 must be larger or equal to this configuration value.",
                  DEFAULT_T2));
        pTranscMgr->pTimers->T2Timeout = DEFAULT_T2;
        pConfiguration->T2 = DEFAULT_T2;
    }
    else
    {
        pTranscMgr->pTimers->T2Timeout = pConfiguration->T2;
    }

    if (-1 >= (RvInt32)pConfiguration->T4)
    {
        pTranscMgr->pTimers->T4Timeout = DEFAULT_T4;
        pConfiguration->T4 = DEFAULT_T4;
    }
    else
    {
        pTranscMgr->pTimers->T4Timeout = pConfiguration->T4;
    }

    if (-1 >= pConfiguration->genLinger)
    {
        pTranscMgr->pTimers->genLingerTimeout = pTranscMgr->pTimers->T1Timeout * 64;
        pConfiguration->genLinger = pConfiguration->T1 * 64;
    }
    else
    {
        pTranscMgr->pTimers->genLingerTimeout = pConfiguration->genLinger;
    }
    if (-1 >= pConfiguration->inviteLinger)
    {
        pTranscMgr->pTimers->inviteLingerTimeout   = DEFAULT_INVITE_LINGER_TIMER;
        pConfiguration->inviteLinger = DEFAULT_INVITE_LINGER_TIMER;
    }
    else
    {
        pTranscMgr->pTimers->inviteLingerTimeout = pConfiguration->inviteLinger;
    }

    if (0 > pConfiguration->provisional)
    {
        pTranscMgr->pTimers->provisionalTimeout = DEFAULT_PROVISIONAL_TIMER;
        pConfiguration->provisional = DEFAULT_PROVISIONAL_TIMER;
    }
    else
    {
        pTranscMgr->pTimers->provisionalTimeout = pConfiguration->provisional;
    }

    if (-1 >= pConfiguration->generalRequestTimeoutTimer)
    {
        pTranscMgr->pTimers->generalRequestTimeoutTimeout = pTranscMgr->pTimers->T1Timeout * 64;
        pConfiguration->generalRequestTimeoutTimer = pConfiguration->T1 * 64;
    }
    else
    {
        pTranscMgr->pTimers->generalRequestTimeoutTimeout = pConfiguration->generalRequestTimeoutTimer;
    }

    if (0 >= pConfiguration->cancelGeneralNoResponseTimer)
    {
        if(pConfiguration->cancelGeneralNoResponseTimer == 0)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "SetTimersConfigurationValues - cancelGeneralNoResponseTimer can't be 0, setting timer to 64*T1."));
        }
        if(pTranscMgr->pTimers->T1Timeout > 0)
        {
            pTranscMgr->cancelGeneralNoResponseTimeout = pTranscMgr->pTimers->T1Timeout * 64;
            pConfiguration->cancelGeneralNoResponseTimer = pConfiguration->T1 * 64;
        }
        else
        {
            pTranscMgr->cancelGeneralNoResponseTimeout = DEFAULT_T1 * 64;
            pConfiguration->cancelGeneralNoResponseTimer = DEFAULT_T1 * 64;
        }
    }
    else
    {
        pTranscMgr->cancelGeneralNoResponseTimeout = pConfiguration->cancelGeneralNoResponseTimer;
    }
    if (0 >= pConfiguration->cancelInviteNoResponseTimer)
    {
        if(pConfiguration->cancelInviteNoResponseTimer == 0)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "SetTimersConfigurationValues - cancelInviteNoResponseTimer can't be 0, setting timer to T1*64."));
        }
        if(pTranscMgr->pTimers->T1Timeout > 0)
        {
            pTranscMgr->pTimers->cancelInviteNoResponseTimeout = pTranscMgr->pTimers->T1Timeout * 64;
            pConfiguration->cancelInviteNoResponseTimer = pConfiguration->T1 * 64;
        }
        else
        {
            pTranscMgr->pTimers->cancelInviteNoResponseTimeout = DEFAULT_T1*64;
            pConfiguration->cancelInviteNoResponseTimer = DEFAULT_T1*64;
        }
    }
    else
    {
        pTranscMgr->pTimers->cancelInviteNoResponseTimeout = pConfiguration->cancelInviteNoResponseTimer;
    }

    /* Set proxy timers*/
    if (-1 >= pConfiguration->proxy2xxRcvdTimer)
    {
        pTranscMgr->proxy2xxRcvdTimer = pTranscMgr->pTimers->T1Timeout * 64;
        pConfiguration->proxy2xxRcvdTimer = pTranscMgr->pTimers->T1Timeout * 64;
    }
    else
    {
        pTranscMgr->proxy2xxRcvdTimer = pConfiguration->proxy2xxRcvdTimer;
    }
    if (-1 >= pConfiguration->proxy2xxSentTimer)
    {
        pTranscMgr->proxy2xxSentTimer = 0;
        pConfiguration->proxy2xxSentTimer = 0;
    }
    else
    {
        pTranscMgr->proxy2xxSentTimer = pConfiguration->proxy2xxSentTimer;
    }

}

/*--------------------HASH FUNCTION CALL BACKS----------------------*/
/***************************************************************************
 * HashFunction
 * ------------------------------------------------------------------------
 * General: call back function for calculating the hash value from the
 *          hash key.
 * Return Value: The value generated from the key.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     key  - The hash key (of type TransactionMgrHashKey)
 ***************************************************************************/
static RvUint  HashFunction(void *key)
{
    TransactionMgrHashKey *hashKey = (TransactionMgrHashKey*)key;
    RvInt32 i,j;
    RvInt32 size;
    RvUint  s = 0;
    RvInt32 mod = hashKey->pMgr->hashSize;
    RvInt32 callIdSize = RPOOL_Strlen(hashKey->pCallId->hPool,
                                       hashKey->pCallId->hPage,
                                       hashKey->pCallId->offset);
    RvInt32    offset = hashKey->pCallId->offset;
    RvInt32    timesToCopy = callIdSize/32;
    RvInt32    leftOvers   = callIdSize%32;
    RvChar     buffer[45];
    static const int base = 1 << (sizeof(char)*8);
    
    for(i=0; i<timesToCopy; i++)
    {

        memset(buffer,0,33);
        RPOOL_CopyToExternal(hashKey->pCallId->hPool,
                             hashKey->pCallId->hPage,
                             offset,
                             buffer,
                             32);

#if defined(UPDATED_BY_SPIRENT)

        for ( j = 0; j < 32; j++)
        { 
            s += buffer[j];
            s += (s << 10);
            s ^= (s >> 6);
        }

        s += (s << 3);
        s ^= (s >> 11);
        s += (s << 15);

#else /* defined(UPDATED_BY_SPIRENT) */
        for (j=0; j<32; j++) {
            s = (s*base+buffer[j])%mod;
        }
#endif /* defined(UPDATED_BY_SPIRENT) */
        offset+=32;
    }
    memset(buffer,0,45);
    if(leftOvers > 0)
    {
        RPOOL_CopyToExternal(hashKey->pCallId->hPool,
                             hashKey->pCallId->hPage,
                             offset,
                             buffer,
                             leftOvers);
    }
    RvSprintf(buffer+leftOvers, "%d", hashKey->cseqStep);
    size = (RvInt32)strlen(buffer);
#if defined(UPDATED_BY_SPIRENT)

    for (i = 0; i < size; i++)
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
    for (j=0; j<size; j++) {
        s = (s*base+buffer[j])%mod;
    }
#endif /* defined(UPDATED_BY_SPIRENT) */
    return s;
}

/***************************************************************************
 * HashCompare
 * ------------------------------------------------------------------------
 * General: Used to compare to keys that where mapped to the same hash
 *          value in order to decide whether they are the same.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashCompare(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *newHashKey = (TransactionMgrHashKey*)newKey;
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;

    if(newHashKey->bIsUac == RV_TRUE)  /*the message belong to a client transaction*/
    {
        if(newHashKey->bSendRequest == RV_TRUE) /*the client transaction
                                                  is about to send a request*/
        {
            /* Check if a transaction with the same identifiers exists before
               inserting the transaction to the hash and send the request message */
            return CompareRequestToClientTransaction(newHashKey, oldHashKey);
        }
        else
        {
            /* Matching responses to client transactions */
            return CompareResponseToClientTransaction(newHashKey, oldHashKey);
        }
    }
    else /*the message belongs to a server transaction*/
    {
        /* Matching requests to server transaction */
        return CompareRequestToServerTransaction(newHashKey, oldHashKey);
    }
}

/***************************************************************************
 * HashCompareCancelledTransc
 * ------------------------------------------------------------------------
 * General: Used to compare to keys that where mapped to the same hash
 *          value in order to decide whether they are the same.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashCompareCancelledTransc(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *newHashKey = (TransactionMgrHashKey*)newKey;
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;

    /*a cancelled transaction cannot be a cancel transaction*/
    if(oldHashKey->eMethod == SIP_TRANSACTION_METHOD_CANCEL)
    {
        return RV_FALSE;
    }

    if(RV_FALSE == *(oldHashKey->bAllowCancellation))
    {
        return RV_FALSE;
    }

    if(oldHashKey->bIsUac != newHashKey->bIsUac)
    {
        return(RV_FALSE);
    }

    return CompareRequestToServerTransaction(newHashKey,oldHashKey);

}

/***************************************************************************
 * HashCompareCancelledTransc
 * ------------------------------------------------------------------------
 * General: find a transaction with the same request identifiers:
 *          from-tag, call-id, and cseq, and the transaction is on ongoing state.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashCompareMergedTransc(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *newHashKey = (TransactionMgrHashKey*)newKey;
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;
    RPOOL_Ptr newFromTagPtr, oldFromTagPtr;

    if(oldHashKey->bIsUac != newHashKey->bIsUac)
    {
        return(RV_FALSE);
    }

    /* Compare methods only if the method is undefined*/
    if(newHashKey->eMethod != oldHashKey->eMethod)
    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
              "HashCompareMergedTransc - Methods are not equal (%d != %d)",
               newHashKey->eMethod, oldHashKey->eMethod));
        return RV_FALSE;
    }
    if (newHashKey->eMethod == SIP_TRANSACTION_METHOD_OTHER)
    {
        if (NULL != newHashKey->strMethod)
        {
            if (NULL == oldHashKey->strMethod)
            {
                RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                          "HashCompareMergedTransc - Methods are not equal (%s != %s)",
                           HashKeyMethodToStr(newHashKey), HashKeyMethodToStr(oldHashKey)));
                return RV_FALSE;
            }
            if (0 != strcmp(newHashKey->strMethod, oldHashKey->strMethod))
            {
                RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                          "HashCompareMergedTransc - Methods are not equal (%s != %s)",
                           HashKeyMethodToStr(newHashKey), HashKeyMethodToStr(oldHashKey)));
                return RV_FALSE;
            }
        }
        else if (NULL != oldHashKey->strMethod)
        {
            RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                  "HashCompareMergedTransc - Methods are not equal (%s != %s)",
                   HashKeyMethodToStr(newHashKey), HashKeyMethodToStr(oldHashKey)));
            return RV_FALSE;
        }
    }

    if((*(oldHashKey->hFrom))==NULL)
    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                  "HashCompareMergedTransc - (*(oldHashKey.hFrom))=NULL"));
        return RV_FALSE;
    }
    newFromTagPtr.hPool = SipPartyHeaderGetPool(*(newHashKey->hFrom));
    newFromTagPtr.hPage = SipPartyHeaderGetPage(*(newHashKey->hFrom));
    newFromTagPtr.offset = SipPartyHeaderGetTag(*(newHashKey->hFrom));

    oldFromTagPtr.hPool = SipPartyHeaderGetPool(*(oldHashKey->hFrom));
    oldFromTagPtr.hPage = SipPartyHeaderGetPage(*(oldHashKey->hFrom));
    oldFromTagPtr.offset = SipPartyHeaderGetTag(*(oldHashKey->hFrom));

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    if ((oldHashKey->cseqStep == newHashKey->cseqStep) && /* cseq*/
        (0 == SPIRENT_RPOOL_Stricmp(newFromTagPtr.hPool,
                                  newFromTagPtr.hPage,
                                  newFromTagPtr.offset,
                                  oldFromTagPtr.hPool,
                                  oldFromTagPtr.hPage,
                                  oldFromTagPtr.offset)) &&  /*from-tag */
        (0 == SPIRENT_RPOOL_Strcmp(newHashKey->pCallId->hPool,
                                  newHashKey->pCallId->hPage,
                                  newHashKey->pCallId->offset,
                                  oldHashKey->pCallId->hPool,
                                  oldHashKey->pCallId->hPage,
                                  oldHashKey->pCallId->offset))) /* call-id */
#else
    if ((oldHashKey->cseqStep == newHashKey->cseqStep) && /* cseq*/
        (RV_TRUE == RPOOL_Stricmp(newFromTagPtr.hPool,
                                  newFromTagPtr.hPage,
                                  newFromTagPtr.offset,
                                  oldFromTagPtr.hPool,
                                  oldFromTagPtr.hPage,
                                  oldFromTagPtr.offset)) &&  /*from-tag */
        (RV_TRUE == RPOOL_Strcmp(newHashKey->pCallId->hPool,
                                  newHashKey->pCallId->hPage,
                                  newHashKey->pCallId->offset,
                                  oldHashKey->pCallId->hPool,
                                  oldHashKey->pCallId->hPage,
                                  oldHashKey->pCallId->offset))) /* call-id */
#endif
/* SPIRENT_END */
    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
            "HashCompareMergedTransc - succeed"));
        return RV_TRUE;
    }

    RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
        "HashCompareMergedTransc - failed"));
    return RV_FALSE;

}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * HashComparePrackMatchTransc
 * ------------------------------------------------------------------------
 * General: Used to compare to keys that where mapped to the same hash
 *          value in order to decide whether they are the same.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashComparePrackMatchTransc(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *newHashKey = (TransactionMgrHashKey*)newKey;
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;
    if(BasicHashCompare(newKey, oldKey) == RV_FALSE)
    {
        return RV_FALSE;
    }
    return (*(newHashKey->rseqVal) == *(oldHashKey->rseqVal));
}
#endif /*#ifndef RV_SIP_PRIMITIVES */


/***************************************************************************
 * HashCompareMatchAckTransc
 * ------------------------------------------------------------------------
 * General: Used to compare two keys that were mapped to the same hash
 *          value in order to decide whether they are the same.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashCompareMatchAckTransc(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;

    /*Compare as proxy receiving ack */
    if(RV_TRUE == *oldHashKey->bIsProxy)
    {
      /*The transaction we are comparing with is a proxy transaction
        with 2xx response, leave it to the application to find the match INVITE transaction */
        if(*(oldHashKey->pRespCode) >= 200 && *(oldHashKey->pRespCode) < 300)
        {
            return RV_FALSE; /* NOT FOUND */
        }
        /*On a proxy transaction with non 2xx response try to locate a match INVITE transaction*/
        /*compare To(+tag), From(+tag), CallId and CSeq step*/
        else
        {
            return HashCompare(newKey, oldKey);
        }
    }
    /*not a proxy compare as UA receving ACK*/
    /*compare To(+tag), From(+tag), CallId and CSeq step*/
    else
    {
        if(*(oldHashKey->pRespCode) >= 200 && *(oldHashKey->pRespCode) < 300)
        { 
            /* UA: In this case the branches are not equal so the Via compare is not needed */
            /* ack on 2xx - check the bAllowAckHandling flag */
            if(*(oldHashKey->bAllowAckHandling) == RV_FALSE)
            {
                return RV_FALSE;
            }
            return BasicHashCompare(newKey, oldKey);
        }
        else
        {
            return HashCompare(newKey, oldKey);
        }
    }
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * HashKeyMethodToStr
 * ------------------------------------------------------------------------
 * General: Converts a hash key method to a string.
 * Return Value: The method as a string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hashKey  - method enum
 ***************************************************************************/
static RvChar* HashKeyMethodToStr(IN TransactionMgrHashKey *hashKey)
{
    if(hashKey->eMethod != SIP_TRANSACTION_METHOD_OTHER &&
       hashKey->eMethod != SIP_TRANSACTION_METHOD_UNDEFINED)
    {
        return TransactionGetStringMethodFromEnum(hashKey->eMethod);
    }
    else if(hashKey->strMethod != NULL)
    {
        return hashKey->strMethod;
    }
    else
    {
        return "NULL";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


/***************************************************************************
 * HashKeyMethodsAreEqual
 * ------------------------------------------------------------------------
 * General: Compares the methods in two hash keys
 * Return Value: TRUE if the methods are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newHashKey  - a new key (of type TransactionMgrHashKey)
 *            oldHashKey  - a key from the hash.
 ***************************************************************************/
static RvBool HashKeyMethodsAreEqual(IN TransactionMgrHashKey *newHashKey,
                                     IN TransactionMgrHashKey *oldHashKey)
{

    /*if the method is undefined we are looking for a cancelled transaction.
      such a transaction matches any method but cancel*/
    if(newHashKey->eMethod == SIP_TRANSACTION_METHOD_UNDEFINED &&
       oldHashKey->eMethod != SIP_TRANSACTION_METHOD_CANCEL)
    {
        return RV_TRUE;
    }
    if (newHashKey->eMethod != oldHashKey->eMethod)
    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                   "HashKeyMethodsAreEqual - Methods are not equal (%s != %s)",
                   HashKeyMethodToStr(newHashKey),
                   HashKeyMethodToStr(oldHashKey)));

        return RV_FALSE;
    }
    if (newHashKey->eMethod == SIP_TRANSACTION_METHOD_OTHER)
    {
        if (NULL != newHashKey->strMethod)
        {
            if (NULL == oldHashKey->strMethod)
            {
                RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                          "HashKeyMethodsAreEqual - Methods are not equal (%s != %s)",
                           HashKeyMethodToStr(newHashKey),
                           HashKeyMethodToStr(oldHashKey)));
                return RV_FALSE;
            }
            if (0 != strcmp(newHashKey->strMethod, oldHashKey->strMethod))
            {
                RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                          "HashKeyMethodsAreEqual - Methods are not equal (%s != %s)",
                           HashKeyMethodToStr(newHashKey),
                           HashKeyMethodToStr(oldHashKey)));
                return RV_FALSE;
            }
        }
        else if (NULL != oldHashKey->strMethod)
        {
                RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                          "HashKeyMethodsAreEqual - Methods are not equal (%s != %s)",
                           HashKeyMethodToStr(newHashKey),
                           HashKeyMethodToStr(oldHashKey)));
            return RV_FALSE;
        }
    }
    RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
              "HashKeyMethodsAreEqual - Methods are equal (%s)",
               HashKeyMethodToStr(oldHashKey)));
    return RV_TRUE;
}

/***************************************************************************
 * BasicHashCompare
 * ------------------------------------------------------------------------
 * General: Used a basic compare of two keys that where mapped to the same hash
 *          Basic Compare: To, From, Callid, Method and IsUac.
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newKey  - a new key (of type TransactionMgrHashKey)
 *          oldKey  - a key from the hash.
 ***************************************************************************/
static RvBool BasicHashCompare(void *newKey ,void *oldKey)
{
    TransactionMgrHashKey *newHashKey = (TransactionMgrHashKey*)newKey;
    TransactionMgrHashKey *oldHashKey = (TransactionMgrHashKey*)oldKey;
    RPOOL_Ptr newFromTagPtr, oldFromTagPtr;
    RPOOL_Ptr newToTagPtr  , oldToTagPtr;

    /* Compare methods only if the method is undefined*/
    if(newHashKey->eMethod != SIP_TRANSACTION_METHOD_UNDEFINED)
    {
        if(HashKeyMethodsAreEqual(newHashKey, oldHashKey) == RV_FALSE)
        {
            return RV_FALSE;
        }
    }

    if((*(oldHashKey->hTo)) == NULL || (*(oldHashKey->hFrom)) == NULL)
    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
            "BasicHashCompare - *(oldHashKey->hTo))=0x%p (*(oldHashKey->hFrom))=0x%p",
            (*(oldHashKey->hTo)),(*(oldHashKey->hFrom))));
        return RV_FALSE;
    }
    newToTagPtr.hPool = SipPartyHeaderGetPool(*(newHashKey->hTo));
    newToTagPtr.hPage = SipPartyHeaderGetPage(*(newHashKey->hTo));
    newToTagPtr.offset = SipPartyHeaderGetTag(*(newHashKey->hTo));

    oldToTagPtr.hPool = SipPartyHeaderGetPool(*(oldHashKey->hTo));
    oldToTagPtr.hPage = SipPartyHeaderGetPage(*(oldHashKey->hTo));
    oldToTagPtr.offset = SipPartyHeaderGetTag(*(oldHashKey->hTo));

    newFromTagPtr.hPool = SipPartyHeaderGetPool(*(newHashKey->hFrom));
    newFromTagPtr.hPage = SipPartyHeaderGetPage(*(newHashKey->hFrom));
    newFromTagPtr.offset = SipPartyHeaderGetTag(*(newHashKey->hFrom));

    oldFromTagPtr.hPool = SipPartyHeaderGetPool(*(oldHashKey->hFrom));
    oldFromTagPtr.hPage = SipPartyHeaderGetPage(*(oldHashKey->hFrom));
    oldFromTagPtr.offset = SipPartyHeaderGetTag(*(oldHashKey->hFrom));

    if(oldToTagPtr.offset == UNDEFINED || newToTagPtr.offset == UNDEFINED)
    /* If the old to tag is NULL, then every new to-tag is OK */
    {
       newToTagPtr.offset = UNDEFINED;
       oldToTagPtr.offset = UNDEFINED;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    if ((oldHashKey->bIsUac == newHashKey->bIsUac) &&
        (oldHashKey->cseqStep == newHashKey->cseqStep) &&
        (0 == SPIRENT_RPOOL_Stricmp(newFromTagPtr.hPool,
                                  newFromTagPtr.hPage,
                                  newFromTagPtr.offset,
                                  oldFromTagPtr.hPool,
                                  oldFromTagPtr.hPage,
                                  oldFromTagPtr.offset)) &&
        (0 == SPIRENT_RPOOL_Stricmp(newToTagPtr.hPool,
                                  newToTagPtr.hPage,
                                  newToTagPtr.offset,
                                  oldToTagPtr.hPool,
                                  oldToTagPtr.hPage,
                                  oldToTagPtr.offset)) &&
        (0 == SPIRENT_RPOOL_Strcmp(newHashKey->pCallId->hPool,
                                  newHashKey->pCallId->hPage,
                                  newHashKey->pCallId->offset,
                                  oldHashKey->pCallId->hPool,
                                  oldHashKey->pCallId->hPage,
                                  oldHashKey->pCallId->offset)))
#else
    if ((oldHashKey->bIsUac == newHashKey->bIsUac) &&
        (oldHashKey->cseqStep == newHashKey->cseqStep) &&
        (RV_TRUE == RPOOL_Stricmp(newFromTagPtr.hPool,
                                  newFromTagPtr.hPage,
                                  newFromTagPtr.offset,
                                  oldFromTagPtr.hPool,
                                  oldFromTagPtr.hPage,
                                  oldFromTagPtr.offset)) &&
        (RV_TRUE == RPOOL_Stricmp(newToTagPtr.hPool,
                                  newToTagPtr.hPage,
                                  newToTagPtr.offset,
                                  oldToTagPtr.hPool,
                                  oldToTagPtr.hPage,
                                  oldToTagPtr.offset)) &&
        (RV_TRUE == RPOOL_Strcmp(newHashKey->pCallId->hPool,
                                  newHashKey->pCallId->hPage,
                                  newHashKey->pCallId->offset,
                                  oldHashKey->pCallId->hPool,
                                  oldHashKey->pCallId->hPage,
                                  oldHashKey->pCallId->offset)))
#endif
/* SPIRENT_END */


    {
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
            "BasicHashCompare - BasicHashCompare succeed"));
        return RV_TRUE;
    }
    RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
        "BasicHashCompare - BasicHashCompare failed"));
    return RV_FALSE;
}

/***************************************************************************
 * CompareResponseToClientTransaction
 * ------------------------------------------------------------------------
 * General: Compares responses to client transactions
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     newHashKey  - a new key (of type TransactionMgrHashKey)
 *          oldHashKey  - a key from the hash.
***************************************************************************/
static RvBool CompareResponseToClientTransaction(IN TransactionMgrHashKey *newHashKey,
                                                  IN TransactionMgrHashKey *oldHashKey)
{
    if(oldHashKey->bIsUac == RV_FALSE)
    {
        return RV_FALSE;
    }
    if(SipViaHeaderCompareBranchParams(*(newHashKey->hTopVia), *(oldHashKey->hTopVia)) == RV_FALSE)
    {
        return RV_FALSE;
    }

    return HashKeyMethodsAreEqual(newHashKey,oldHashKey);
}

/***************************************************************************
* CompareRequestToServerTransaction
* ------------------------------------------------------------------------
* General: Compares requests to server transactions
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashKey  - a new key (of type TransactionMgrHashKey)
*          oldHashKey  - a key from the hash.
***************************************************************************/
static RvBool CompareRequestToServerTransaction(IN TransactionMgrHashKey *newHashKey,
                                                 IN TransactionMgrHashKey *oldHashKey)
{
    if(oldHashKey->bIsUac == RV_TRUE)
    {
        return RV_FALSE;
    }

    return CompareRequestToTransaction(newHashKey, oldHashKey);
}


/***************************************************************************
* CompareRequestToClientTransaction
* ------------------------------------------------------------------------
* General: Compares requests to client transactions (before sending the
*          first request message in a transaction)
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashKey  - a new key (of type TransactionMgrHashKey)
*           oldHashKey  - a key from the hash.
***************************************************************************/
static RvBool CompareRequestToClientTransaction(IN TransactionMgrHashKey *newHashKey,
                                                 IN TransactionMgrHashKey *oldHashKey)
{
    if(oldHashKey->bIsUac != RV_TRUE)
    {
        return RV_FALSE;
    }

    return CompareRequestToTransaction(newHashKey, oldHashKey);
}

/***************************************************************************
* CompareRequestToTransaction
* ------------------------------------------------------------------------
* General: Compares new requests to existing requests
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashKey  - a new key (of type TransactionMgrHashKey)
*           oldHashKey  - a key from the hash.
***************************************************************************/
static RvBool CompareRequestToTransaction(IN TransactionMgrHashKey *newHashKey,
                                           IN TransactionMgrHashKey *oldHashKey)
{
    RPOOL_Ptr newBranchPtr;

    newBranchPtr.hPool  = SipViaHeaderGetPool(*(newHashKey->hTopVia));
    newBranchPtr.hPage  = SipViaHeaderGetPage(*(newHashKey->hTopVia));
    newBranchPtr.offset = SipViaHeaderGetBranchParam(*(newHashKey->hTopVia));

    if(RPOOL_CmpPrefixToExternal(newBranchPtr.hPool, newBranchPtr.hPage,
        newBranchPtr.offset, "z9hG4bK") == RV_FALSE)
        /* For backword compatibility  */
    {
        /* Compares Call-ID, to-tag, from-tag, cseq and uac/uas */
        if(RV_FALSE == BasicHashCompare((void *)newHashKey,(void *)oldHashKey))
        {
            return RV_FALSE;
        }
        /* Compares top Via */
        if(oldHashKey->hTopVia == NULL)
        {
            RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                "CompareRequestToTransaction - top vias 0x%p and NULL are not equal", *(newHashKey->hTopVia)));
            return RV_FALSE;
        }
        if ((SipViaHeaderIsEqual(*(newHashKey->hTopVia), *(oldHashKey->hTopVia))) == RV_FALSE)
        {
            RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                "CompareRequestToTransaction - top vias 0x%p and 0x%p are not equal", *(newHashKey->hTopVia), *(oldHashKey->hTopVia)));
            return RV_FALSE;
        }

        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
            "CompareRequestToTransaction - top vias 0x%p and 0x%p are equal", *(newHashKey->hTopVia), *(oldHashKey->hTopVia)));
        /* Compares Request URI */
        if(RvSipAddrUrlIsEqual(newHashKey->hRequestUri, oldHashKey->hRequestUri) == RV_FALSE)
        {
            RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
                "CompareRequestToTransaction - request-uris 0x%p and 0x%p are not equal", newHashKey->hRequestUri, oldHashKey->hRequestUri));
            return RV_FALSE;
        }
        RvLogDebug(newHashKey->pMgr->pLogSrc ,(newHashKey->pMgr->pLogSrc ,
            "CompareRequestToTransaction - request-uris 0x%p and 0x%p are equal", newHashKey->hRequestUri, oldHashKey->hRequestUri));
        return RV_TRUE;
    }
    else
    {
        /* Compares branches */
        if(SipViaHeaderCompareBranchParams(*(newHashKey->hTopVia), *(oldHashKey->hTopVia)) == RV_FALSE)
        {
            return RV_FALSE;
        }
        /* Compares sent-by value */
        if(SipViaHeaderCompareSentByParams(*(newHashKey->hTopVia), *(oldHashKey->hTopVia)) == RV_FALSE)
        {
            return RV_FALSE;
        }
        /* Compares method */
        if(HashKeyMethodsAreEqual(newHashKey,oldHashKey) == RV_FALSE)
        {
            return RV_FALSE;
        }
        return RV_TRUE;
    }
}

/******************************************************************************
 * PrepareCallbackMasks
 * ----------------------------------------------------------------------------
 * General: The function prepare the masks enabling us to block the RvSipTransactionTerminate()
 *          API, if we are in a middle of a callback, that do 
 *          not allow terminate inside.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pMgr      - Pointer to the manager.
 ******************************************************************************/
static void RVCALLCONV PrepareCallbackMasks(IN TransactionMgr* pMgr)
{
    pMgr->terminateMask = 0;
    
    /* turn on all the callbacks that should block the termination */
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_CREATED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_APP_INT_CREATED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_CALL_CREATED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_CANCELLED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_IGNORE_REL_PROV, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_SUPPLY_PARAMS, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_SIGCOMP_NOT_RESPONDED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_NEW_CONN_IN_USE, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_TRANSC_OPEN_CALL_LEG, 1);
    /*CB_TRANSC_INVITE_TRANSC_MSG_SENT,
    CB_TRANSC_SUPPLY_PARAMS,
    CB_TRANSC_AUTH_CREDENTIALS_FOUND,
    CB_TRANSC_AUTH_COMPLETED,
    ,
    ,*/

   
}

#endif /*#ifndef RV_SIP_LIGHT*/

