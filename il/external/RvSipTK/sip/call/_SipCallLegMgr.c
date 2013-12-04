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
*********************************************************************************
*/


/*********************************************************************************
 *                              <SipCallLegMgr.c>
 *
 *  The SipCallLegMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the Call-leg module and getting the module
 *  resource status.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"

#include "CallLegMgrObject.h"
#include "CallLegObject.h"
#include "CallLegTranscEv.h"
#include "CallLegMsgEv.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "_SipPartyHeader.h"
#include "RvSipCallLeg.h"
#include "CallLegCallbacks.h"
#include "rvmemory.h"
#include "CallLegRefer.h"
#include "_SipSubsMgr.h"
#include "RvSipExpiresHeader.h"
#include "CallLegSubs.h"
#include "CallLegForking.h"
#include "CallLegSession.h"
#include "CallLegInvite.h"

#include "RvSipResourcesTypes.h"

#include "RvSipTransportDNSTypes.h"
#include "_SipCommonTypes.h"
#include "_SipCommonCore.h"
#include "AdsCopybits.h"

#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeTypes.h"
#include "_SipSecAgreeMgr.h"
#include "CallLegSecAgree.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV CreateCallLegList(
                                    IN CallLegMgr *pMgr,
                                    IN RvInt32   maxNumOfCalls);

static void RVCALLCONV DestructCallLegList(CallLegMgr  *pMgr);


static void RVCALLCONV   SetTranscEvHandlers(
                                    IN CallLegMgr          *pMgr,
                                    IN RvSipTranscMgrHandle hTranscMgr);

static void RVCALLCONV   CallLegMgrTranscCreatedEvHandler(
                                  IN    void                    *hCallLegMgr,
                                  IN    RvSipTranscHandle        hTransc,
                                  INOUT SipTransactionKey        *pKey,
                                  OUT   void                     **ppOwner,
                                  OUT   SipTripleLock            **tripleLock,
                                  OUT   RvUint16               *pStatusCode);

static void NotifyAppOnGeneralTransactionIfNeeded(
                                         IN CallLeg              *pCallLeg,
                                         IN RvSipTranscHandle    hTransc,
                                         IN SipTransactionMethod eMethod,
                                         OUT RvBool             *bAppHandleTransc);

static RvBool ContinueHandlingNewTranscAtStateTerminated(
                                             IN  CallLeg              *pCallLeg,
                                             IN  RvSipTranscHandle     hTransc,
                                             IN  SipTransactionMethod  eMethod);

static RvBool IsInitialRequest(IN  CallLegMgr           *pMgr,
                              IN  RvSipTranscHandle     hTransc,
                              IN  SipTransactionKey     *pKey,
                              IN  SipTransactionMethod  eMethod);

static RvStatus CreateAndInitNewCallLeg(IN  CallLegMgr*           pMgr,
                                         IN  RvSipTranscHandle     hNewTransc,
                                         IN  SipTransactionKey     *pKey,
                                         IN  SipTransactionMethod  eMethod,
                                         IN  RvBool                bInsertToHash,
                                         OUT CallLeg               **pCallLeg);

static RvStatus CallLegHandleNewTransaction(IN  CallLegMgr*           pMgr,
                                             IN  CallLeg*              pCallLeg,
                                             IN  RvSipTranscHandle     hTransc,
                                             IN  SipTransactionMethod  eMethod,
                                             OUT RvUint16*            pStatusCode);

static RvStatus HandleNestedInviteInInitialStateCall(
                                  IN  CallLeg               *pCallLeg,
                                  IN  RvSipTranscHandle     hTransc,
                                  IN  SipTransactionMethod  eMethod,
                                  IN  SipTransactionKey     *pKey,
                                  OUT RvBool                *pbCreateNewCall);

static void RVCALLCONV DestructAllCallLegs(CallLegMgr  *pMgr);

static void RVCALLCONV InitSubsReferData(IN  CallLegMgr        *pMgr,
                                         IN  SipCallLegMgrCfg  *cfg,
                                         OUT RvInt32           *pSubsCallsNo);
#ifdef RV_SIP_SUBS_ON
static void RVCALLCONV HandleUnmatchedNotifyIfNeeded(
                                IN  CallLegMgr         *pMgr,
                                IN  RvSipTranscHandle   hTransc,
                                OUT RvSipCallLegHandle *phCallLeg,
                                OUT RvUint16           *pStatusCode);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_IMS_ON
static void RVCALLCONV SetSecAgreeEvHandlers(
                               IN RvSipSecAgreeMgrHandle   hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

static void RVCALLCONV PrepareCallbackMasks(IN CallLegMgr* pCallMgr);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipCallLegMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new call-leg manager. The call-leg manager is
 *          encharged of all the calls-legs. The manager holds a list of
 *          call-legs and is used to create new call-legs.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the call-leg mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   call-leg manager.
 *               RV_OK -        Call-leg manager was created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLog           - Handle to the log module.
 *          moduleLogId    - The module log id used for printing.
 *          hTransport     - The transport manager handle.
 *          hTranscMgr     - The transaction manager handle.
 *          hAuthMgr       - The authentication manager handle
 *          hGeneralPool   - Pool used bye each call-leg for internal memory
 *                           allocations.
 *          maxNumOfCalls  - Max number of concurrent calls.
 *          maxNumOfTransc - Max number of concurrent transactions.
 *          hStack         - A handle to the stack manager.
 * Output:     *phCallLegMgr - Handle to the newly created call leg manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrConstruct(
                            IN  SipCallLegMgrCfg          *pCfg,
                            IN  void                      *hStack,
                            OUT RvSipCallLegMgrHandle     *phCallLegMgr)
{

    RvStatus     rv;
    RvStatus     crv;
    CallLegMgr  *pMgr;
    RvInt32      numOfCallsToAlloc = pCfg->maxNumOfCalls;
    RvInt32      numOfSubsCallsToAlloc;
    RvInt32      i;

    *phCallLegMgr = NULL;

    /*------------------------------------------------------------
       allocates the call leg manager structure and initialize it
    --------------------------------------------------------------*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(CallLegMgr), pCfg->pLogMgr, (void*)&pMgr))
    {
        RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                  "SipCallLegMgrConstruct - Failed ,RvMemoryAlloc failed"));
        return RV_ERROR_UNKNOWN;
    }

    /* Initializing the struct members to NULL (0) */
    memset(pMgr, 0, sizeof(CallLegMgr));

    /* init callLegMgr parameters*/
    pMgr->pLogMgr                   = pCfg->pLogMgr;
    pMgr->pLogSrc                   = pCfg->moduleLogId;
    pMgr->hTransportMgr             = pCfg->hTransport;
    pMgr->hMsgMgr                   = pCfg->hMsgMgr;
    pMgr->hTranscMgr                = pCfg->hTranscMgr; /*set transaction manager in db*/
    pMgr->hTrxMgr                   = pCfg->hTrxMgr;
#ifdef RV_SIP_IMS_ON
    pMgr->hSecMgr                   = pCfg->hSecMgr;
#endif /* #ifdef RV_SIP_IMS_ON */
    pMgr->hGeneralPool              = pCfg->hGeneralPool;
    pMgr->hMsgMemPool               = pCfg->hMessagePool;
    pMgr->hElementPool              = pCfg->hElementPool;
    pMgr->maxNumOfCalls             = pCfg->maxNumOfCalls;
    pMgr->numOfAllocatedCalls       = 0;
    pMgr->bIsPersistent             = pCfg->bIsPersistent;
    /*pMgr->totalNumOfCalls = numOfCallsToAlloc;*/
    pMgr->maxNumOfTransc            = pCfg->maxNumOfTransc;
    pMgr->seed                      = pCfg->seed;
    pMgr->hStack                    = hStack;
    pMgr->enableInviteProceedingTimeoutState= pCfg->enableInviteProceedingTimeoutState;
#ifdef RV_SIP_AUTH_ON
    pMgr->hAuthMgr                  = pCfg->hAuthMgr;
#endif /* #ifdef RV_SIP_AUTH_ON */
    pMgr->manualAckOn2xx            = pCfg->manualAckOn2xx;
    pMgr->manualPrack               = pCfg->manualPrack;
    pMgr->manualBehavior            = pCfg->manualBehavior;
    pMgr->statusReplaces            = RVSIP_CALL_LEG_REPLACES_UNDEFINED;
    pMgr->bEnableForking            = pCfg->bEnableForking;
    pMgr->ackTrxTimerTimeout        = pCfg->forkedAckTrxTimeout;
    pMgr->forked1xxTimerTimeout     = pCfg->forked1xxTimerTimeout;
    pMgr->inviteLingerTimeout       = pCfg->inviteLingerTimeout;
    pMgr->bOldInviteHandling    = pCfg->bOldInviteHandling;
    pMgr->trxEvHandlers.pfnOtherURLAddressFoundEvHandler = CallLegInviteTrxOtherURLAddressFoundEv;
    pMgr->trxEvHandlers.pfnStateChangedEvHandler         = CallLegInviteTrxStateChangeEv;
    
    PrepareCallbackMasks(pMgr)    ;
    
    InitSubsReferData(pMgr,pCfg,&numOfSubsCallsToAlloc);
    numOfCallsToAlloc += numOfSubsCallsToAlloc;
    RvSelectGetThreadEngine(pMgr->pLogMgr, &pMgr->pSelect);

    /*set call back on the transaction module*/
    SetTranscEvHandlers(pMgr,pCfg->hTranscMgr);

    /*set a mutex for the manager*/
    crv = RvMutexConstruct(pMgr->pLogMgr, &pMgr->hMutex);
    if(crv != RV_OK)
    {
        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return CRV2RV(crv);
    }

    /*if(pCfg->maxNumOfCalls != 0)*/
    if(numOfCallsToAlloc != 0)
    {
        /*-------------------------------------------------------
        initialize call-leg list
        ---------------------------------------------------------*/
        /*rv = CreateCallLegList(pMgr, pCfg->maxNumOfCalls);*/
        rv = CreateCallLegList(pMgr, numOfCallsToAlloc);
        if(rv != RV_OK)
        {
            RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                "SipCallLegMgrConstruct - Failed to initialize call-leg list (rv=%d)",rv));
            RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /*---------------------------------------------------------------------
        initialize pool of invite objects to the usage of the call-legs
        The number of lists in the pool equals to the max number of calls * 3.
        -----------------------------------------------------------------------*/
        pMgr->hInviteObjsListPool = RLIST_PoolListConstruct(pMgr->maxNumOfCalls * 3,
                                                        pMgr->maxNumOfCalls,
                                                        sizeof(CallLegInvite),
                                                        pMgr->pLogMgr, 
                                                        "Invite Objs List");
        
        if(pMgr->hInviteObjsListPool == NULL)
        {
            RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                "SipCallLegMgrConstruct - Failed - failed initialize invite objects lists pool"));
            DestructCallLegList(pMgr);
            RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }
        /*---------------------------------------------------------------------
        initialize pool of transaction handles to the usage of the call-legs
        The number of lists in the pool equals to the max number of calls.
        The number of elements in all the list equals the max number of transactions
        -----------------------------------------------------------------------*/
        pMgr->hTranscHandlesPool = RLIST_PoolListConstruct(pCfg->maxNumOfTransc,
                                                           numOfCallsToAlloc,
                                                           sizeof(RvSipTranscHandle),
                                                           pMgr->pLogMgr,
                                                           "Call-Leg Transc List");

        if(pMgr->hTranscHandlesPool == NULL)
        {
            RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                "SipCallLegMgrConstruct - Failed - failed initialize transacion lists pool"));
            DestructCallLegList(pMgr);
            RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /*--------------------------------------------
        Initialize hash table to store call-legs
        --------------------------------------------*/
        rv = CallLegMgrHashInit(pMgr);
        if(rv != RV_OK)
        {
            RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                "SipCallLegMgrConstruct - Failed - failed initialize hash table"));
            DestructCallLegList(pMgr);
            RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

    }

    /* Session Timer parameters */
    pMgr->eSessiontimeStatus = RVSIP_CALL_LEG_SESSION_TIMER_UNDEFINED;
    pMgr->MinSE              = UNDEFINED;
    pMgr->sessionExpires     = UNDEFINED;
    pMgr->manualSessionTimer = RV_FALSE;

    pMgr->addSupportedListToMsg = pCfg->addSupportedListToMsg;
    for(i=0;i < pCfg->extensionListSize;i++)
    {
        if(strcmp(pCfg->supportedExtensionList[i],"timer") == 0)
        {
            pMgr->eSessiontimeStatus = RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED;
            if (pCfg->MinSE > pCfg->sessionExpires && pCfg->sessionExpires > 0)
            {
                RvLogWarning(pCfg->moduleLogId,(pCfg->moduleLogId,
                  "SipCallLegMgrConstruct - sessionExpires (%d) is lower than MinSE (%d). It's updated",
                  pCfg->sessionExpires,pCfg->MinSE));
                pMgr->sessionExpires = pCfg->MinSE;
            }
            else
            {
                pMgr->sessionExpires = pCfg->sessionExpires;
            }

            if (pCfg->MinSE == UNDEFINED)
            {
                pMgr->MinSE = RVSIP_CALL_LEG_MIN_MINSE;
            }
            else
            {
                pMgr->MinSE        = pCfg->MinSE;
                pMgr->bAddReqMinSE = RV_TRUE;
            }
            pMgr->manualSessionTimer = pCfg->manualSessionTimer;
            break;
        }
    }

    for(i=0;i< pCfg->extensionListSize;i++)
    {
        if(strcmp(pCfg->supportedExtensionList[i],"replaces") == 0)
        {
            pMgr->statusReplaces = RVSIP_CALL_LEG_REPLACES_SUPPORTED;
            break;
        }
    }
    RvLogInfo(pCfg->moduleLogId,(pCfg->moduleLogId,
              "SipCallLegMgrConstruct - Call-leg manager was constructed successfully, Num of Call legs = %d",
              /*pCfg->maxNumOfCalls*/numOfCallsToAlloc));

    *phCallLegMgr = (RvSipCallLegMgrHandle)pMgr;
    return RV_OK;
}



/***************************************************************************
 * SipCallLegMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the call-leg manager freeing all resources.
 * Return Value:  RV_OK -  Call-leg manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLegMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrDestruct(
                                   IN RvSipCallLegMgrHandle hCallLegMgr)
{

    CallLegMgr            *pMgr = (CallLegMgr*)hCallLegMgr;
    RvLogSource*          pLogSrc;

    pLogSrc = pMgr->pLogSrc;
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/


    /* destructing all the calls. no resources will stay unfreed */
    DestructAllCallLegs(pMgr);

    /*free the transaction pool*/
    if(pMgr->hTranscHandlesPool != NULL)
    {
        RLIST_PoolListDestruct(pMgr->hTranscHandlesPool);
    }
    /*free the Invite Objects pool*/
    if(pMgr->hInviteObjsListPool != NULL)
    {
        RLIST_PoolListDestruct(pMgr->hInviteObjsListPool);
    }
    /*free the hash table*/
    if(pMgr->hHashTable != NULL)
    {
        HASH_Destruct(pMgr->hHashTable);
        pMgr->hHashTable = NULL;
    }

    /* free the call-leg list and list pool*/
    DestructCallLegList(pMgr);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/
    RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);


    RvLogInfo(pLogSrc,(pLogSrc,
              "SipCallLegMgrDestruct - Call-Leg manager was destructed successfully"));

    /* free call-leg mgr data structure */
    RvMemoryFree((void*)pMgr, pMgr->pLogMgr);

    return RV_OK;

}



/***************************************************************************
 * SipCallLegMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the call module. It includes the call-leg list, the transactions
 *          handles lists and the list items.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrGetResourcesStatus (
                                 IN  RvSipCallLegMgrHandle     hMgr,
                                 OUT RvSipCallLegResources  *pResources) {

    CallLegMgr *pMgr = (CallLegMgr*)hMgr;

    RvUint32 allocNumOfLists;
    RvUint32 allocMaxNumOfUserElement;
    RvUint32 currNumOfUsedLists;
    RvUint32 currNumOfUsedUserElement;
    RvUint32 maxUsageInLists;
    RvUint32 maxUsageInUserElement;

    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(pMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    memset(pResources,0,sizeof(RvSipCallLegResources));
    /*call - leg list*/
    if(pMgr->hCallLegPool != NULL)
    {
        RLIST_GetResourcesStatus(pMgr->hCallLegPool,
                             &allocNumOfLists, /*allways 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*allways 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

        pResources->calls.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->calls.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->calls.maxUsageOfElements = maxUsageInUserElement;
    }
    /*transaction handles lists and lists items*/
    if(pMgr->hTranscHandlesPool != NULL)
    {
        RLIST_GetResourcesStatus(pMgr->hTranscHandlesPool,
                             &allocNumOfLists,
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);


        pResources->transcLists.numOfAllocatedElements = allocNumOfLists;
        pResources->transcLists.currNumOfUsedElements = currNumOfUsedLists;
        pResources->transcLists.maxUsageOfElements = maxUsageInLists;
        pResources->transcHandles.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->transcHandles.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->transcHandles.maxUsageOfElements = maxUsageInUserElement;
    }

    /*invite objects lists and lists items*/
    if(pMgr->hInviteObjsListPool != NULL)
    {
        RLIST_GetResourcesStatus(pMgr->hInviteObjsListPool,
                                &allocNumOfLists,
                                &allocMaxNumOfUserElement,
                                &currNumOfUsedLists,
                                &currNumOfUsedUserElement,
                                &maxUsageInLists,
                                &maxUsageInUserElement);
        
        
        pResources->inviteLists.numOfAllocatedElements = allocNumOfLists;
        pResources->inviteLists.currNumOfUsedElements = currNumOfUsedLists;
        pResources->inviteLists.maxUsageOfElements = maxUsageInLists;
        pResources->inviteObjects.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->inviteObjects.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->inviteObjects.maxUsageOfElements = maxUsageInUserElement;
    }
    return RV_OK;
}

/***************************************************************************
 * SipCallLegMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of call-legs that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLegMgr - The call-leg manager.
 ***************************************************************************/
void RVCALLCONV SipCallLegMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipCallLegMgrHandle  hCallLegMgr)
{
    CallLegMgr *pMgr = (CallLegMgr*)hCallLegMgr;

    RLIST_ResetMaxUsageResourcesStatus(pMgr->hCallLegPool);
}


/***************************************************************************
 * SipCallLegMgrGetReplacesStatus
 * ------------------------------------------------------------------------
 * General: Get the replaces status of a Call-Leg mgr.
 * Return Value: RvSipCallLegReplacesStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 ***************************************************************************/
RvSipCallLegReplacesStatus RVCALLCONV SipCallLegMgrGetReplacesStatus(
                                            IN RvSipCallLegMgrHandle hCallLegMgr)
{
    return ((CallLegMgr*)hCallLegMgr)->statusReplaces;
}

/******************************************************************************
 * SipCallLegMgrFindHiddenCallLeg
 * ----------------------------------------------------------------------------
 * General: search Call-Leg Manager database for Hidden Call-Leg with dialog
 *          identifiers, equal to those,supplied by Transaction Key parameter.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrFindHiddenCallLeg(
                                    IN  RvSipCallLegMgrHandle   hMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  RvSipCallLegDirection   eKeyDirection,
                                    OUT RvSipCallLegHandle      *phCallLeg)
{
    CallLegMgr  *pMgr = (CallLegMgr*)hMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    CallLegMgrHashFindHidden(pMgr, pKey, eKeyDirection,
        (RvSipCallLegHandle**)phCallLeg);
    return RV_OK;
}

/******************************************************************************
 * SipCallLegMgrFindOriginalHiddenCallLeg
 * ----------------------------------------------------------------------------
 * General: search Call-Leg Manager database for Hidden and Original Call-Leg
 *          with dialog identifiers, equal to those,supplied by Transaction Key
 *          parameter.
 *          Call-Leg is Hidden, if it serves out-of-dialog Subscription.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrFindOriginalHiddenCallLeg(
                                    IN  RvSipCallLegMgrHandle   hMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  RvSipCallLegDirection   eKeyDirection,
                                    OUT RvSipCallLegHandle      *phCallLeg)
{
    CallLegMgr  *pMgr = (CallLegMgr*)hMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    CallLegMgrHashFindOriginalHidden(pMgr, pKey, eKeyDirection,
                                     (RvSipCallLegHandle**)phCallLeg);
    return RV_OK;
}


/******************************************************************************
 * SipCallLegMgrHashInsert
 * ----------------------------------------------------------------------------
 * General: Insert a Call-Leg into the Call-Leg Manager hash table.
 *          The key is generated in according to the data in Call-Leg object.
 * Return Value: RV_OK on success,
 *          error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 *****************************************************************************/
RvStatus SipCallLegMgrHashInsert(IN RvSipCallLegHandle hCallLeg)
{
    return CallLegMgrHashInsert(hCallLeg);
}

/******************************************************************************
 * SipCallLegMgrLock
 * ----------------------------------------------------------------------------
 * General: locks Call-Leg Manager.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMgr - Handle to the Call-Leg Manager.
 * Output:  none.
 *****************************************************************************/
void SipCallLegMgrLock(IN RvSipCallLegMgrHandle hMgr)
{
    RvMutexLock(&((CallLegMgr*)hMgr)->hMutex,((CallLegMgr*)hMgr)->pLogMgr);
}

/******************************************************************************
 * SipCallLegMgrUnlock
 * ----------------------------------------------------------------------------
 * General: unlocks Call-Leg Manager.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMgr - Handle to the Call-Leg Manager.
 * Output:  none.
 *****************************************************************************/
void SipCallLegMgrUnlock(IN RvSipCallLegMgrHandle hMgr)
{
    RvMutexUnlock(&((CallLegMgr*)hMgr)->hMutex,((CallLegMgr*)hMgr)->pLogMgr);
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CreateCallLegList
 * ------------------------------------------------------------------------
 * General: Allocated the list of call-legs managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of calls to allocate
 ***************************************************************************/
static RvStatus RVCALLCONV CreateCallLegList(
                               IN CallLegMgr *pMgr,
                               IN RvInt32   maxNumOfCalls)
{
    RvInt32    i;
    CallLeg        *call;
    RvStatus    rv;

    /*allocate a pool with 1 list*/
    pMgr->hCallLegPool = RLIST_PoolListConstruct(maxNumOfCalls,1,
                               sizeof(CallLeg),pMgr->pLogMgr, "Call Leg List");

    if(pMgr->hCallLegPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pMgr->hCallLegList = RLIST_ListConstruct(pMgr->hCallLegPool);

    if(pMgr->hCallLegList == NULL)
    {
        RLIST_PoolListDestruct(pMgr->hCallLegPool);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* initiate locks of all calls */
    for (i=0; i < maxNumOfCalls; i++)
    {
        RLIST_InsertTail(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&call);
        call->pMgr = pMgr;
        rv = SipCommonConstructTripleLock(&call->callTripleLock, pMgr->pLogMgr);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                      "CreateCallLegList - SipCommonConstructTripleLock (rv=%d)",rv));
            DestructCallLegList(pMgr);
            return rv;
        }
        call->tripleLock = NULL;
    }

    /* initiate locks of all calls */
    for (i=0; i < maxNumOfCalls; i++)
    {
        RLIST_GetHead(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&call);
        RLIST_Remove(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE)call);
    }

    /*pMgr->maxNumOfCalls = maxNumOfCalls;*/
    return RV_OK;
}

/***************************************************************************
 * DestructAllCallLegs
 * ------------------------------------------------------------------------
 * General: Destructs all call-legs. (all call resources will be released)
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of calls to allocate
 ***************************************************************************/
static void RVCALLCONV DestructAllCallLegs(CallLegMgr  *pMgr)
{
    CallLeg        *pNextToKill = NULL;
    CallLeg        *pCall2Kill = NULL;

    if(pMgr->hCallLegPool==NULL)
    {
        return;
    }

    RLIST_GetHead(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&pCall2Kill);
    while (NULL != pCall2Kill)
    {
        RLIST_GetNext(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE)pCall2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
        if (NULL != pCall2Kill->hashElementPtr)
        {
            HASH_RemoveElement(pMgr->hHashTable,pCall2Kill->hashElementPtr);
            pCall2Kill->hashElementPtr = NULL;
        }
		CallLegResetAllInviteObj(pCall2Kill);
        CallLegDestruct(pCall2Kill);
        pCall2Kill = pNextToKill;
    }
}

/***************************************************************************
 * DestructCallLegList
 * ------------------------------------------------------------------------
 * General: Destructs the list of call-legs.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr   - Handle to the manager
 *            maxNumOfCalls - Max number of calls to allocate
 ***************************************************************************/
static void RVCALLCONV DestructCallLegList(CallLegMgr  *pMgr)
{
    RvUint32   i;
    CallLeg    *pCallLeg;
    RvUint32   allocNumOfLists = 0;
    RvUint32   allocMaxNumOfUserElement = 0;
    RvUint32   currNumOfUsedLists = 0;
    RvUint32   currNumOfUsedUsreElement = 0;
    RvUint32   maxUsageInLists = 0;
    RvUint32   maxUsageInUserElement = 0;

    if(pMgr->hCallLegPool==NULL)
    {
        return;
    }

    RLIST_GetResourcesStatus(pMgr->hCallLegPool,
                             &allocNumOfLists,    &allocMaxNumOfUserElement,
                             &currNumOfUsedLists, &currNumOfUsedUsreElement,
                             &maxUsageInLists,    &maxUsageInUserElement);

    for (i=0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_GetHead(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&pCallLeg);
        if (pCallLeg == NULL)
            break;
        RLIST_Remove(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE)pCallLeg);
    }

    for (i=0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_InsertTail(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&pCallLeg);
    }

    for (i=0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_GetHead(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE *)&pCallLeg);
        SipCommonDestructTripleLock(&pCallLeg->callTripleLock, pMgr->pLogMgr);
        RLIST_Remove(pMgr->hCallLegPool,pMgr->hCallLegList,(RLIST_ITEM_HANDLE)pCallLeg);
    }

    RLIST_PoolListDestruct(pMgr->hCallLegPool);
    pMgr->hCallLegPool = NULL;
}

/***************************************************************************
 * SetTranscEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the transaction events handlers on the transaction module.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle to the call leg manager
 *          hTranscMgr - Handle to the transaction manager.
 ***************************************************************************/
static void RVCALLCONV SetTranscEvHandlers(
                               IN CallLegMgr           *pMgr,
                               IN RvSipTranscMgrHandle  hTranscMgr)
{

    RvSipTransactionEvHandlers    transcEvHandlers;
    SipTransactionMgrEvHandlers   transcMgrEvHandlers;

    RV_UNUSED_ARG(hTranscMgr);

    memset(&transcMgrEvHandlers,0,sizeof(SipTransactionMgrEvHandlers));
    transcMgrEvHandlers.pMgr = pMgr;
    transcMgrEvHandlers.pfnEvTransactionCreated  = CallLegMgrTranscCreatedEvHandler;
    transcMgrEvHandlers.pfnRelProvRespRcvd       = CallLegTranscEvRelProvRespRcvdEv;
    transcMgrEvHandlers.pfnIgnoreRelProvRespRcvd = CallLegTranscEvIgnoreRelProvRespEvHandler;
    transcMgrEvHandlers.pfnInviteResponseNoTranscRcvd = CallLegTranscEvInviteResponseNoTranscRcvdEvHandler;
    transcMgrEvHandlers.pfnAckNoTranscRcvd       = CallLegTranscEvAckNoTranscEvHandler;
    transcMgrEvHandlers.pfnInviteTranscMsgSent   = CallLegTranscEvInviteTranscMsgSent;

    memset(&transcEvHandlers,0,sizeof(RvSipTransactionEvHandlers));
    transcEvHandlers.pfnEvStateChanged = CallLegTranscEvStateChangedHandler;
    transcEvHandlers.pfnEvMsgReceived  = CallLegMsgEvMsgRcvdHandler;
    transcEvHandlers.pfnEvMsgToSend    = CallLegMsgEvMsgToSendHandler;
    transcEvHandlers.pfnEvSupplyParams         = CallLegTranscEvSupplyTranscParamsEv;
#ifdef RV_SIP_AUTH_ON
    transcEvHandlers.pfnEvAuthCompleted        = CallLegTranscEvAuthCompleted;
    transcEvHandlers.pfnEvAuthCredentialsFound = CallLegTranscEvAuthCredentialsFound;
#endif /* #ifdef RV_SIP_AUTH_ON */
    transcEvHandlers.pfnEvOtherURLAddressFound = CallLegTranscEvOtherURLAddressFound;
    transcEvHandlers.pfnEvFinalDestResolved    = CallLegTranscFinalDestResolvedEv;
    transcEvHandlers.pfnEvNewConnInUse  = CallLegTranscNewConnInUseEv;
    transcEvHandlers.pfnEvTranscCancelled        = CallLegTranscEvCancelled;
#ifdef RV_SIGCOMP_ON
    transcEvHandlers.pfnEvSigCompMsgNotResponded = CallLegTranscSigCompMsgNotRespondedEv;
#endif
    SipTransactionMgrSetEvHandlers(
                             pMgr->hTranscMgr,
                             SIP_TRANSACTION_OWNER_CALL,
                             &transcMgrEvHandlers,
                             sizeof(transcMgrEvHandlers),
                             &transcEvHandlers,
                             sizeof(transcEvHandlers));



}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SetSecAgreeEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the security-agreement events handlers to the security-agreement 
 *          module.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
static void RVCALLCONV SetSecAgreeEvHandlers(
                               IN RvSipSecAgreeMgrHandle   hSecAgreeMgr)
{
	SipSecAgreeEvHandlers   secAgreeEvHandlers;

    memset(&secAgreeEvHandlers,0,sizeof(SipSecAgreeEvHandlers));
	secAgreeEvHandlers.pfnSecAgreeAttachSecObjEvHanlder = CallLegSecAgreeAttachSecObjEv;
    secAgreeEvHandlers.pfnSecAgreeDetachSecAgreeEvHanlder = CallLegSecAgreeDetachSecAgreeEv;
	
    SipSecAgreeMgrSetEvHandlers(
                             hSecAgreeMgr,
                             &secAgreeEvHandlers,
							 RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG);
}
#endif /* RV_SIP_IMS_ON */

/***************************************************************************
*   CallLegMgrTranscCreatedEvHandler
* ------------------------------------------------------------------------
* General: This function is called by the transaction layer when ever a
*          new transaction is created. Using the transaction key the relevant
*          call-leg is found or a new call-leg is created. The new call-leg
*          becomes the transaction owner and gives it its lock.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLegMgr - Handle to the call-leg manager.
*            hTransc     - Handle to the newly created transaction.
* In-Out   pKey        - pointer to a temporary transaction key. The key is
*                        replaced with the call-leg key (on the call-leg memory
*                        page).
* Output:     ppOwner     - pointer to the new transaction owner.
*          hLock       - Pointer to the new transaction lock.
*           pStatusCode  - defines the response status code in case of failure.
***************************************************************************/
static void RVCALLCONV CallLegMgrTranscCreatedEvHandler(
                                       IN    void               *hCallLegMgr,
                                       IN    RvSipTranscHandle   hTransc,
                                       INOUT SipTransactionKey  *pKey,
                                       OUT   void              **ppOwner,
                                       OUT   SipTripleLock     **tripleLock,
                                       OUT   RvUint16           *pStatusCode)
{
    RvStatus              rv;
    CallLegMgr            *pMgr       = (CallLegMgr*)hCallLegMgr;
    CallLeg               *pCallLeg;
    RvBool                bNewCallLeg          = RV_FALSE;
    RvBool                bCreateSecCallLeg    = RV_FALSE;
    RvBool                bInsertNewCallToHash = RV_TRUE;
    RvBool                bInitialRequest      = RV_FALSE;
    SipTransactionMethod  eMethod;

    SipTransactionGetMethod(hTransc,&eMethod);
    bInitialRequest = IsInitialRequest(pMgr, hTransc, pKey, eMethod);

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - Call-leg Mgr 0x%p received notification about a new transaction 0x%p",
              pMgr,hTransc));

    /*---------------------------------------------
          search for the call-leg in the hash
      ---------------------------------------------*/
    /*lock the call-leg manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    if(*ppOwner != NULL)
    {
        /* the transaction layer had already found the call-leg that own this transaction
           - because this is a cancel transaction, and the owner is the same as in the
           canceled transaction */
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - CANCEL transaction 0x%p already found its owner 0x%p",
              hTransc, *ppOwner));
        pCallLeg = (CallLeg*)*ppOwner;
    }
    else
    {
        RvBool  bOnlyEstablishedCall;

        /*set the output parameters to NULL*/
        *ppOwner    = NULL;
        *tripleLock = NULL;
        /* for initial request, we want to find all call-legs, with or without to-tag.
           (If we will find such call-leg this will be the case of a nested request).
           for non-initial, we don't want to find call-legs without to-tag */
        bOnlyEstablishedCall = (bInitialRequest == RV_TRUE) ? RV_FALSE: RV_TRUE;
        if(eMethod == SIP_TRANSACTION_METHOD_CANCEL)
        {
            /* for cancel we need to find also call-legs with no to-tag */
            bOnlyEstablishedCall = RV_FALSE;
        }
#ifdef RV_SIP_SUBS_ON
        if(eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
        {
            /* a NOTIFY before 2xx on subscription creates the dialog,
               therefore for a notify, we want also call-legs with no to-tag */
            bOnlyEstablishedCall = RV_FALSE;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        
#ifndef RFC_2543_COMPLIANT
        if(SipPartyHeaderGetTag(pKey->hToHeader) == UNDEFINED &&
           RV_FALSE == bInitialRequest && eMethod != SIP_TRANSACTION_METHOD_CANCEL)
        {
            /* non initial request that has no to-tag should not be handled
               by the call-leg layer. */
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrTranscCreatedEvHandler - Call-leg Mgr 0x%p general request with no to-tag. ignore transc 0x%p. ",
                hCallLegMgr,hTransc));
			RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
            return;
        }
#endif /*#ifndef RFC_2543_COMPLIANT*/    

        /*look for a call-leg in the call-leg manager.*/
        CallLegMgrHashFind(pMgr,pKey,RVSIP_CALL_LEG_DIRECTION_INCOMING,
                           bOnlyEstablishedCall,
                           (RvSipCallLegHandle**)&pCallLeg);
        /* If no Call-Leg leg was matched for incoming NOTIFY,
        it can be a forked NOTIFY (for Forked Subscription).
        See RFC 3265, sections 3.3.3 and 4.4.9.
        Ask Subscription Manager to handle the suspected NOTIFY.
        If Subscription Manager finds out, that this is a forked NOTIFY,
        and decides to handle the Notification, it should create Call-Leg and
        supply it back in order to enable set Transaction owner in this function.
        */
#ifdef RV_SIP_SUBS_ON
        if(NULL == pCallLeg && SIP_TRANSACTION_METHOD_NOTIFY == eMethod)
        {
            HandleUnmatchedNotifyIfNeeded(
                    pMgr,hTransc,(RvSipCallLegHandle*)&pCallLeg,pStatusCode);
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrTranscCreatedEvHandler - new call-leg 0x%p was created for unmatched NOTIFY transc 0x%p",
                pCallLeg, hTransc));
            
        }
        else 
#endif /* #ifdef RV_SIP_SUBS_ON */
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p was found in hash",pCallLeg));            
        }
    }

    /* Call-leg was found - we do not release the manager lock. As long as the manager is locked this
      call-leg cannot be terminated (in order to terminate a call-leg it must be removed from the
      hash */
    if(pCallLeg != NULL) /*call-leg was found - lock it*/
    {
        RvInt32 curIdentifier = pCallLeg->callLegUniqueIdentifier;

       RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

        rv = CallLegLockMsg(pCallLeg);
        if(rv != RV_OK && rv != RV_ERROR_DESTRUCTED) /* General Failure */
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - failed to lock call-leg 0x%p. rv=%d, respond with 600. ",
              pCallLeg, rv));
            *pStatusCode = 600; /* 600 response on this request */
            return;
        }

        /* The call-leg was terminated (in and out of the queue)
           before we had a chance to lock it */
        if(rv == RV_ERROR_DESTRUCTED)
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p lock returned RV_ERROR_DESTRUCTED. ignore the call-leg we found",
              pCallLeg));

            pCallLeg = NULL;    /*behave as if we did not find a call-leg*/
        }
        /*the call-leg was destructed and is out of the hash and out the queue
        but it was constructed again so the locking succeed on the wrong
        object*/
        else if(pCallLeg->callLegUniqueIdentifier != curIdentifier)
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p uniqeIdentifier not equal. ignore the call-leg we found",
              pCallLeg));

            CallLegUnLockMsg(pCallLeg);  /*unlock the call-leg*/
            pCallLeg = NULL;             /*behave as if we did not find a call-leg*/
        }
        /*call leg is in the event queue (or will soon be in the queue)for termination
        we continue to process only for subscriptions that belongs to this call-leg*/
        else if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            if(ContinueHandlingNewTranscAtStateTerminated(pCallLeg,hTransc,eMethod) == RV_FALSE)
            {
                RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p is in TERMINATED state. Ignore the call-leg we found",
                    pCallLeg));

                CallLegUnLockMsg(pCallLeg);  /*unlock the call-leg*/
                pCallLeg = NULL;             /*behave as if we did not found a call-leg*/
            }
        }
        /* if the call-leg was took out of the hash, before we locked it */
        else if(pCallLeg->hashElementPtr == NULL)
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p is not in the hash. Ignore the call-leg we found",
              pCallLeg));
            CallLegUnLockMsg(pCallLeg);  /*unlock the call-leg*/
            pCallLeg = NULL;             /*behave as if we did not find a call-leg*/
        }
#ifdef RV_SIP_SUBS_ON
        else if(RV_TRUE == CallLegGetIsHidden(pCallLeg) &&
                eMethod != SIP_TRANSACTION_METHOD_NOTIFY &&
                eMethod != SIP_TRANSACTION_METHOD_SUBSCRIBE &&
                eMethod != SIP_TRANSACTION_METHOD_REFER)
        {
            *pStatusCode = 400;
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegMgrTranscCreatedEvHandler - call-leg 0x%p - hidden call-leg. Illegal request transaction 0x%p - reject with 400",
              pCallLeg, hTransc));
            CallLegUnLockMsg(pCallLeg);
            return;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        if(pCallLeg == NULL)
        {
            RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager as if we never found a call-leg*/
        }
    }

    if(pCallLeg != NULL)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrTranscCreatedEvHandler - Call-leg Mgr 0x%p found call-leg 0x%p for transc 0x%p",
                hCallLegMgr,pCallLeg,hTransc));
    }

    if(RV_TRUE == pMgr->bEnableNestedInitialRequestHandling &&
       pCallLeg != NULL &&
       RV_TRUE == bInitialRequest)
    {
        /* check if this is a second INVITE in a call-leg that is not yet in state
           connected */
        HandleNestedInviteInInitialStateCall(pCallLeg, hTransc, eMethod, pKey, &bCreateSecCallLeg);

        if(RV_TRUE == bCreateSecCallLeg)
        {
            bInsertNewCallToHash = RV_FALSE;
            CallLegUnLockMsg(pCallLeg);
            RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
        }

    }

    /*---------------------------------------------------
          call-leg not found - create call-leg if needed
      ---------------------------------------------------*/
    /*here we need to check again if we have a call-leg or not*/
    if(pCallLeg == NULL || RV_TRUE == bCreateSecCallLeg)  /*manager is still locked*/
    {

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegMgrTranscCreatedEvHandler - Call-leg Mgr 0x%p did not find call-leg for transc 0x%p (pCallLeg=0x%x,bCreateSecCallLeg=%s)",
                  hCallLegMgr, hTransc, pCallLeg, (RV_TRUE==bCreateSecCallLeg)?"RV_TRUE":"FALSE"));

        /*---------------------------------------------------------
           Create a new call-leg if needed
          ---------------------------------------------------------*/
        if(bInitialRequest == RV_FALSE)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegMgrTranscCreatedEvHandler - Call-leg Mgr 0x%p did not find call-leg for a non-creating dialog transc 0x%p. ",
                  hCallLegMgr,hTransc));
            RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
            return;
        }
        rv = CreateAndInitNewCallLeg(pMgr, hTransc,pKey,eMethod,bInsertNewCallToHash,&pCallLeg);
        if(rv != RV_OK)
        {
            if(rv == RV_ERROR_OUTOFRESOURCES)
            {
                *pStatusCode = 486; /* 486 response on this request */
            }
            else
            {
                *pStatusCode = 600; /* 600 response on this request */
            }
            RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
            return;
        }
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

        /*lock the new call-leg*/
        rv = CallLegLockMsg(pCallLeg);
        if(rv != RV_OK)
        {
            *pStatusCode = 600; /* 600 response on this request */
            RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
            if (rv != RV_ERROR_DESTRUCTED)
            {
                CallLegDestruct(pCallLeg);
            }
            return;
        }

        /*notify that a new call-leg was created*/
        rv = CallLegMgrNotifyCallLegCreated(pMgr,(RvSipCallLegHandle)pCallLeg,hTransc);
        if(rv != RV_OK || pCallLeg->createdRejectStatusCode != 0)
        {
            if(pCallLeg->createdRejectStatusCode != 0)
            {
                *pStatusCode = pCallLeg->createdRejectStatusCode;
            }
            else
            {
                *pStatusCode = 600;
            }
            CallLegDestruct(pCallLeg);
            CallLegUnLockMsg(pCallLeg);
            return;
        }
        bNewCallLeg = RV_TRUE;
    }
    /*If we reached this place we have a locked call-leg to work with
      (not necessarily a new call-leg) - the manager is unlocked*/
    rv = CallLegHandleNewTransaction(pMgr, pCallLeg, hTransc, eMethod, pStatusCode);
    if(rv != RV_OK)
    {
        if(bNewCallLeg == RV_TRUE)
        {
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        }
        CallLegUnLockMsg(pCallLeg);
        return;
    }

    /*set parameters back to the transaction*/
    /*the transaction received the call-leg key*/
    if(pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)
    {
        pKey->hToHeader = pCallLeg->hToHeader;
        pKey->hFromHeader = pCallLeg->hFromHeader;
    }
    else
    {
        pKey->hFromHeader = pCallLeg->hToHeader;
        pKey->hToHeader = pCallLeg->hFromHeader;
    }
    pKey->strCallId.offset = pCallLeg->strCallId;
    pKey->strCallId.hPage  = pCallLeg->hPage;
    pKey->strCallId.hPool  = pCallLeg->pMgr->hGeneralPool;
    pKey->localAddr        = pCallLeg->localAddr;

    *ppOwner = (void*)pCallLeg; /*set the call-leg as the owner of the transaction*/
    *tripleLock =  pCallLeg->tripleLock; /*give the transaction the mutex of the call*/

    CallLegUnLockMsg(pCallLeg);
}



/***************************************************************************
 * SipCallLegMgrGetCallLegPool
 * ------------------------------------------------------------------------
 * General: Returns the call-leg pool handle.
 * Return Value: handle to the call-leg pool.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 ***************************************************************************/
RLIST_POOL_HANDLE RVCALLCONV SipCallLegMgrGetCallLegPool(IN  RvSipCallLegMgrHandle  hMgr)
{
    CallLegMgr* pMgr = (CallLegMgr*)hMgr;
    return pMgr->hCallLegPool;
}

/***************************************************************************
 * SipCallLegMgrGetCallLegList
 * ------------------------------------------------------------------------
 * General: Returns the call-leg list handle.
 * Return Value: handle to the call-leg list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 ***************************************************************************/
RLIST_HANDLE RVCALLCONV SipCallLegMgrGetCallLegList(IN  RvSipCallLegMgrHandle  hMgr)
{
    CallLegMgr* pMgr = (CallLegMgr*)hMgr;
    return pMgr->hCallLegList;
}

/***************************************************************************
 * SipCallLegMgrCreateCallLeg
 * ------------------------------------------------------------------------
 * General: Creates a new Outgoing call-leg and replace handles with the
 *          application.  The new call-leg assumes the Idle state.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create a new call-leg. Not
 *                                   enough resources.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - Handle to the call-leg manager
 *            eDirection - Direction (incoming/outgoing) of the new callLeg.
 *          bIsHidden   - is this is a hidden call-leg (for independent subscription)
 *                       or not.
 * Output:     phCallLeg  - sip stack handle to the new call-leg
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrCreateCallLeg(
                                        IN    RvSipCallLegMgrHandle hMgr,
                                        IN    RvSipCallLegDirection eDirection,
                                        IN    RvBool               bIsHidden,
                                        OUT   RvSipCallLegHandle   *phCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                        )
{
  //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
  RvStatus rv=CallLegMgrCreateCallLeg((CallLegMgr*)hMgr, eDirection, bIsHidden, phCallLeg);
  CallLeg  *pCallLeg =(CallLeg*)*phCallLeg;
  if(pCallLeg) {
    pCallLeg->cctContext = cctContext;
    pCallLeg->URI_Cfg_Flag = URI_cfg;
  }
  return rv;
#else
  return CallLegMgrCreateCallLeg((CallLegMgr*)hMgr, eDirection, bIsHidden, phCallLeg);
#endif
  //SPIRENT_END
}

/***************************************************************************
 * SipCallLegMgrSetSubsMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the subsMgr handle in call-leg mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr     - Handle to the call-leg manager.
 *          hSubsMgr - Handle to the subs manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrSetSubsMgrHandle(
                                        IN RvSipCallLegMgrHandle hCallLegMgr,
                                        IN RvSipSubsMgrHandle    hSubsMgr)
{
#ifdef RV_SIP_SUBS_ON
    CallLegMgr* pMgr = (CallLegMgr*)hCallLegMgr;
    pMgr->hSubsMgr = hSubsMgr;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hCallLegMgr)
	RV_UNUSED_ARG(hSubsMgr)

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipCallLegMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in call-leg mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the call-leg manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrSetSecAgreeMgrHandle(
                                        IN RvSipCallLegMgrHandle     hCallLegMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr)
{
    CallLegMgr* pMgr = (CallLegMgr*)hCallLegMgr;

	/* Update handle */
    pMgr->hSecAgreeMgr = hSecAgreeMgr;

	/* Set event handlers */
	SetSecAgreeEvHandlers(hSecAgreeMgr);

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

/***************************************************************************
*   NotifyAppOnGeneralTransactionIfNeeded
* ------------------------------------------------------------------------
* General: Calls the CallLegTranscCreated callback if the transaction is
*          not INVITE,BYE,CANCEL or PRACK when 100 rel is supported
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg  - pointer to the call-leg.
*            hTransc   - Handle to the newly created transaction.
*           eMethod   -   The transaction method.
* Output:   bAppHandleTransc - indicates if the application will
*                              handle the transaction
***************************************************************************/
static void NotifyAppOnGeneralTransactionIfNeeded(
                                         IN CallLeg              *pCallLeg,
                                         IN RvSipTranscHandle    hTransc,
                                         IN SipTransactionMethod eMethod,
                                         OUT RvBool             *bAppHandleTransc)
{
    RvSipMsgHandle hMsg;
    RvStatus      rv;

    RvSipTransaction100RelStatus e100RelStatus =
                    SipTransactionMgrGet100relStatus(pCallLeg->pMgr->hTranscMgr);
    *bAppHandleTransc = RV_FALSE;
    /* If this is a NOTIFY transaction, which is used to notify the status of
       a REFER request, do not allow the application the control on this transaction*/
    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if (RV_OK != rv)
    {
        return;
    }

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        /* this is a hidden call-leg of subscription, or a refer call-leg.
        no way for a transaction to be of application */
        return;
    }
    if(eMethod ==  SIP_TRANSACTION_METHOD_INVITE ||
       eMethod ==  SIP_TRANSACTION_METHOD_CANCEL)
    {
        return;
    }
    if(eMethod == SIP_TRANSACTION_METHOD_BYE)
    {
        if(pCallLeg->callLegEvHandlers->pfnByeCreatedEvHandler != NULL &&
           pCallLeg->callLegEvHandlers->pfnByeStateChangedEvHandler != NULL)
        {
            RvSipAppTranscHandle hAppTransc = NULL;
            CallLegCallbackByeCreatedEv((RvSipCallLegHandle)pCallLeg,hTransc,&hAppTransc,bAppHandleTransc);
            RvSipTransactionSetAppHandle(hTransc,hAppTransc);
            if(*bAppHandleTransc == RV_TRUE)
            {
                SipTransactionSetAppInitiator(hTransc);
            }
        }
        return;
    }

    if(eMethod == SIP_TRANSACTION_METHOD_PRACK &&
        e100RelStatus != RVSIP_TRANSC_100_REL_UNDEFINED)
    {
        return;
    }



    if(pCallLeg->callLegEvHandlers->pfnTranscCreatedEvHandler != NULL)
    {
        RvSipAppTranscHandle hAppTransc = NULL;
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "NotifyAppOnGeneralTransactionIfNeeded - New general transaction 0x%p was created for call-leg 0x%p - notifying application",
            hTransc,pCallLeg));
        CallLegCallbackTranscCreatedEv((RvSipCallLegHandle)pCallLeg,hTransc,&hAppTransc,bAppHandleTransc);
        RvSipTransactionSetAppHandle(hTransc,hAppTransc);
        if(*bAppHandleTransc == RV_TRUE)
        {
            SipTransactionSetAppInitiator(hTransc);
        }
    }
}




/***************************************************************************
 * ContinueHandlingNewTranscAtStateTerminated
 * ------------------------------------------------------------------------
 * General: When the call-leg is in the terminated state we still need to handle
 *          certain transactions such as SUBSCRIBE and NOTIFY that belongs to
 *          this call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - pointer to the call-leg.
 *            hTransc        - Handle to the newly created transaction.
 *           eMethod        - The transaction method.
 ***************************************************************************/
static RvBool ContinueHandlingNewTranscAtStateTerminated(
                                             IN  CallLeg              *pCallLeg,
                                             IN  RvSipTranscHandle     hTransc,
                                             IN  SipTransactionMethod  eMethod)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus rv    = RV_OK;
    RvSipMsgHandle            hMsg;
    RvSipEventHeaderHandle    hEvent;
    RvSipHeaderListElemHandle hListElem;
    RvSipSubsHandle           hSubs = NULL;
    RvSipSubscriptionType     eSubsType;

    if((eMethod != SIP_TRANSACTION_METHOD_SUBSCRIBE) &&
       (eMethod != SIP_TRANSACTION_METHOD_NOTIFY))
    {
        return RV_FALSE;
    }
    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK)
    {
        return RV_FALSE;
    }
    /*according to the event we can see if the message belongs to a subscription
      inside the call-leg*/
    hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EVENT, RVSIP_FIRST_HEADER, &hListElem);
    /*incoming subscribe belongs to a notifier*/
    eSubsType = (eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE)?
                            RVSIP_SUBS_TYPE_NOTIFIER:RVSIP_SUBS_TYPE_SUBSCRIBER;

    /*look for a subscription in the call-leg*/
    rv = CallLegSubsFindSubscription((RvSipCallLegHandle)pCallLeg, hEvent, eSubsType, &hSubs);
    if(rv != RV_OK)
    {
        return RV_FALSE;
    }
    if(hSubs != NULL)
    {
        return RV_TRUE;
    }
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(eMethod)
	RV_UNUSED_ARG(hTransc)
	RV_UNUSED_ARG(pCallLeg)

#endif /* #ifdef RV_SIP_SUBS_ON */
    return RV_FALSE;
}

/***************************************************************************
* IsInitialRequest
* ------------------------------------------------------------------------
* General: check whether we need to create a new call-leg for the incoming
*          transaction.
* Return Value: RvBool
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr           - Handle to the call-leg manager
*            hTransc        - Handle to the newly created transaction.
*           pKey           - The transaction key.
*           eMethod        - The transaction method.
***************************************************************************/
static RvBool IsInitialRequest(IN  CallLegMgr           *pMgr,
                               IN  RvSipTranscHandle     hTransc,
                               IN  SipTransactionKey     *pKey,
                               IN  SipTransactionMethod  eMethod)
{
    RV_UNUSED_ARG(hTransc);

#ifdef RV_SIP_SUBS_ON
    /*only invite/refer/subscribe we create a new call-leg*/
    if((eMethod != SIP_TRANSACTION_METHOD_INVITE) &&
       (eMethod != SIP_TRANSACTION_METHOD_REFER)  &&
       (eMethod != SIP_TRANSACTION_METHOD_SUBSCRIBE))
    {
        return RV_FALSE;
    }
#else /* #ifdef RV_SIP_SUBS_ON */ 
	if (eMethod != SIP_TRANSACTION_METHOD_INVITE) 
	{
		return RV_FALSE;
	}
#endif /* #ifdef RV_SIP_SUBS_ON */

    /*we never create a new call-leg when there is a to tag*/
    if(SipPartyHeaderGetTag(pKey->hToHeader) != UNDEFINED)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "IsInitialRequest - Initial request has a To tag - call-leg will not be created"));

        return RV_FALSE;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif

    return RV_TRUE;
}

/***************************************************************************
* HandleNestedInviteInInitialStateCall
* ------------------------------------------------------------------------
* General: Call to application callback, to decide how to act with a second
*          INVITE received in a call-leg that is not in state connected yet.
* Return Value: RvBool
* ------------------------------------------------------------------------
* Arguments:
* Input:    pCallLeg        - Handle to the exists call-leg.
*           pKey            - The transaction key.
* Output;   pbCreateNewCall - Create a new call-leg for the new INVITE request,
*                             or not.
***************************************************************************/
static RvStatus HandleNestedInviteInInitialStateCall(
                                  IN  CallLeg               *pCallLeg,
                                  IN  RvSipTranscHandle     hTransc,
                                  IN  SipTransactionMethod  eMethod,
                                  IN  SipTransactionKey     *pKey,
                                  OUT RvBool                *pbCreateNewCall)
{
    RvSipMsgHandle hMsg;

    *pbCreateNewCall = RV_FALSE;

#ifdef RV_SIP_SUBS_ON
    if(SIP_TRANSACTION_METHOD_SUBSCRIBE == eMethod ||
       SIP_TRANSACTION_METHOD_REFER == eMethod)
    {
        /* do not handle the second subscribe/refer case */
        return RV_OK;
    }
#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(eMethod)
#endif /* #ifdef RV_SIP_SUBS_ON */

    if(SipPartyHeaderGetTag(pKey->hToHeader) != UNDEFINED)
    {
        /* the new INVITE request has to-tag, this is not the case of
           a second initial INVITE. we can continue as usual */
        return RV_OK;
    }

    RvSipTransactionGetReceivedMsg(hTransc, &hMsg);

    CallLegCallbackNestedInitialReqRcvdEv(pCallLeg, hMsg, pbCreateNewCall);


    return RV_OK;
}


/***************************************************************************
* CreateAndInitNewCallLeg
* ------------------------------------------------------------------------
* General: Creates a new call-leg and initialize it with the transaction key.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr       - Pointer to the call-leg manager.
*            hNewTransc - The transaction that caused this call leg to be created.
*            pKey       - The transaction key.
*            eMethod    - The transaction method.
* Output:    pCallLeg   - The newly created call-leg
***************************************************************************/
static RvStatus CreateAndInitNewCallLeg( IN  CallLegMgr*           pMgr,
                                         IN  RvSipTranscHandle     hNewTransc,
                                         IN  SipTransactionKey     *pKey,
                                         IN  SipTransactionMethod  eMethod,
                                         IN  RvBool                bInsertToHash,
                                         OUT CallLeg               **pCallLeg)
{

    RvStatus          rv = RV_OK;
    RvSipCallLegHandle hNewCallLeg;
    RvBool            bIsHidden;   

    *pCallLeg = NULL;

#ifdef RV_SIP_SUBS_ON
    bIsHidden = (eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
                 eMethod == SIP_TRANSACTION_METHOD_REFER)? RV_TRUE:RV_FALSE;
#else /* #ifdef RV_SIP_SUBS_ON */ 
	bIsHidden = RV_FALSE;
	RV_UNUSED_ARG(eMethod)
#endif /* #ifdef RV_SIP_SUBS_ON */


    /*create a new call-leg object*/
    rv = CallLegMgrCreateCallLeg(pMgr,RVSIP_CALL_LEG_DIRECTION_INCOMING, bIsHidden,&hNewCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateAndInitNewCallLeg - Failed to create new call-leg for transc 0x%p (rv=%d)",
            hNewTransc,rv));
        return rv;
    }

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(hNewCallLeg && hNewTransc) {

       RvSipTransportAddr* localTranscAddr = RvSipTransactionGetLocalAddr(hNewTransc);

       ((CallLeg*)(hNewCallLeg))->cctContext = RvSipTransactionGetCct(hNewTransc);
       ((CallLeg*)(hNewCallLeg))->URI_Cfg_Flag=RvSipTransactionGetURIFlag(hNewTransc);

       if(localTranscAddr) {
          RvSipCallLegSetLocalAddress(hNewCallLeg,
				      localTranscAddr->eTransportType,
				      localTranscAddr->eAddrType,
				      localTranscAddr->strIP,
				      localTranscAddr->port,
				      localTranscAddr->if_name,
				      localTranscAddr->vdevblock);
       }
    }
#endif
//SPIRENT_END

    /*set the key information in the new callLeg*/
    rv = CallLegSetKey((CallLeg*)hNewCallLeg,pKey);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateAndInitNewCallLeg - Failed to set key for the new call-leg 0x%p - freeing the object (rv=%d)",
            hNewCallLeg,rv));
        CallLegDestruct((CallLeg*)hNewCallLeg);
        return rv;
    }
    if(RV_TRUE == bInsertToHash)
    {
        /*insert the new call-leg to the hash*/
        rv = CallLegMgrHashInsert(hNewCallLeg);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CreateAndInitNewCallLeg - Failed to insert new call-leg 0x%p to the hash- freeing the object (rv=%d)",
                hNewCallLeg,rv));
            CallLegDestruct((CallLeg*)hNewCallLeg);
            return rv;
        }
    }

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CreateAndInitNewCallLeg - A new call-leg 0x%p was created for transc 0x%p",
                  hNewCallLeg,hNewTransc));

    *pCallLeg = (CallLeg*)hNewCallLeg;
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hNewTransc);
#endif

    return RV_OK;
}

/***************************************************************************
 * CallLegHandleNewTransaction
 * ------------------------------------------------------------------------
 * General: Handles a new transaction just received by the call-leg.
 *          This function notifies other modules/application about the new
 *          incoming transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr       - Pointer to the call-leg manager.
 *            pCallLeg   - The newly created call-leg
 *            hTransc    - The transaction that caused this call leg to be created.
 *            eMethod    - The transaction method.
 * Output:    pStatusCode - Status code for response on the transaction in case
 *            of failure.
 ***************************************************************************/
static RvStatus CallLegHandleNewTransaction(IN  CallLegMgr*           pMgr,
                                             IN  CallLeg*              pCallLeg,
                                             IN  RvSipTranscHandle     hTransc,
                                             IN  SipTransactionMethod  eMethod,
                                             OUT RvUint16*             pStatusCode)
{
    RvBool bAppHandleTransc = RV_FALSE;
    RvStatus rv             = RV_OK;

#ifdef RV_SIGCOMP_ON
    if (pCallLeg->bSigCompEnabled == RV_FALSE)
    {
        RvSipTransactionDisableCompression(hTransc);
    }
#endif /* RV_SIGCOMP_ON */

    if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
    {
        CallLegInvite* pInvite = NULL;
        RvBool         bReInvite;
        RvUint16       rejectCode = 0;

        bReInvite = (pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)? RV_FALSE:RV_TRUE;
        
        if(RV_TRUE == bReInvite &&
           (pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING ||
            pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING ||
            pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING))
        {
            *pStatusCode = 491;
            RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction - call-leg 0x%p - incoming INVITE on call state %s. reject with %d",
                pCallLeg, CallLegGetStateName(pCallLeg->eState), *pStatusCode));
            return RV_ERROR_ILLEGAL_ACTION;
        }

        rv = CallLegInviteCreateObj(pCallLeg, NULL, bReInvite, 
                                    RVSIP_CALL_LEG_DIRECTION_INCOMING,&pInvite, &rejectCode);
        if(rejectCode > 0)
        {
            SipTransactionSetResponseCode(hTransc, rejectCode);
            return RV_OK;
        }
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction - Failed to create new invite obj in new call-leg 0x%p - freeing the object (rv=%d)",
                pCallLeg,rv));
            return rv;
        }
        
        rv = CallLegInviteSetParams(pCallLeg, pInvite, hTransc);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction - Failed to init new invite obj in new call-leg 0x%p - freeing the object (rv=%d)",
                pCallLeg,rv));
            return rv;
        }

        /* if this is a re-invite - Notify application that a new invite object was created */
        if(bReInvite == RV_TRUE &&
           pMgr->bOldInviteHandling == RV_FALSE)
        {
            RvUint16 responseCode;
            CallLegCallbackReInviteCreatedEv(pCallLeg, pInvite, &responseCode);
            if(responseCode > 0)
            {
                RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CallLegHandleNewTransaction: call-leg 0x%p - application rejected re-invite 0x%p. reject with %d",
                    pCallLeg, pInvite, responseCode));
                *pStatusCode = responseCode;
                pCallLeg->createdRejectStatusCode = responseCode;
                return RV_OK;
            }
        }
    }

    /* if the call-leg holds a timers structure - set it here to the new transaction */
    if(pCallLeg->pTimers != NULL)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegHandleNewTransaction: call-leg 0x%p - set timers to the new transaction 0x%p",
            pCallLeg, hTransc));
        rv = SipTransactionSetTimers(hTransc, pCallLeg->pTimers);
        if(rv != RV_OK)
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction: call-leg 0x%p - failed to set timers structure in transc 0x%p. reject with 600",
                pCallLeg, hTransc));
            *pStatusCode = 600;
            return rv;
        }
    }

#ifdef RV_SIP_SUBS_ON
    if(eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
    {
        RvBool bWasNotifyHandled = RV_FALSE;
        rv = SipSubsMgrNotifyTranscCreated(CallLegMgrGetSubsMgr(pMgr),
                                          (RvSipCallLegHandle)pCallLeg,
                                          hTransc,
                                          &bWasNotifyHandled);
        if(rv != RV_OK)
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction: call-leg 0x%p - subs failed to handle incoming NOTIFY. reject with 600",
                pCallLeg));
            *pStatusCode = 600;
            return rv;
        }
        else if (bWasNotifyHandled            == RV_FALSE &&
                 CallLegGetIsHidden(pCallLeg) == RV_TRUE)
        {
            /* if this is a hidden call-leg, and the notify was not mapped to
               any subscription, then 481 should be sent by the transaction layer
               (not the call-leg layer) */
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction: call-leg 0x%p - hidden call-leg. did not find subs for NOTIFY. reject with 481",
                pCallLeg));
            *pStatusCode = 481;
            return RV_ERROR_DESTRUCTED;
        }

        if(bWasNotifyHandled == RV_TRUE)
        {
            return RV_OK;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
	
    if(eMethod != SIP_TRANSACTION_METHOD_INVITE)
    {
        /* 1. notify about the new transaction - only if call-leg is not in state terminated... */
        if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_TERMINATED ||
           RV_TRUE == ContinueHandlingNewTranscAtStateTerminated(pCallLeg, hTransc, eMethod))
        {
            NotifyAppOnGeneralTransactionIfNeeded(pCallLeg, hTransc,eMethod,&bAppHandleTransc);
        }
        /* 2. in case the call-leg is in state terminated (might happen in the previous
              NotifyAppOnGeneralTransactionIfNeeded callback) do not continue */
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED &&
           RV_FALSE == ContinueHandlingNewTranscAtStateTerminated(pCallLeg, hTransc, eMethod))
        {
                /* if the call-leg was terminated in the callback to application - 
               we don't want to handle this transaction */
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction: call-leg 0x%p - already in state Terminated. reject transc 0x%p with 481",
                pCallLeg, hTransc));
            *pStatusCode = 481;
            return RV_ERROR_DESTRUCTED;
        }
        else if(RV_TRUE == bAppHandleTransc)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegHandleNewTransaction: call-leg 0x%p - application will handle transc 0x%p method %d",
                pCallLeg, hTransc, eMethod));
        }
    }

#ifdef RV_SIP_SUBS_ON
    /* if this is a subscribe transaction, all handling actions are done in a different function */
    if((eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethod == SIP_TRANSACTION_METHOD_REFER)
        && bAppHandleTransc == RV_FALSE)
    {
        if(CallLegMgrGetSubsMgr(pMgr) != NULL)
        {
            rv = SipSubsMgrSubsTranscCreated(CallLegMgrGetSubsMgr(pMgr),
                                            (RvSipCallLegHandle)pCallLeg,
                                            hTransc,
                                            eMethod,
                                            pStatusCode);
            if(rv != RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CallLegHandleNewTransaction: call-leg 0x%p - SubsMgr failed to handle transc 0x%p (rv=%d). reject with %d",
                    pCallLeg, hTransc, rv, *pStatusCode));
                *pStatusCode = 600;
            }
            else if(*pStatusCode > 0)
            {
                RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CallLegHandleNewTransaction: call-leg 0x%p - SubsMgr rejected transc 0x%p with %d",
                    pCallLeg, hTransc, *pStatusCode));
                rv = RV_ERROR_UNKNOWN;
            }
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */ 
	
    /* Add transc to list - only general transactions. */
    if(eMethod != SIP_TRANSACTION_METHOD_INVITE)
    {
        rv = CallLegAddTranscToList(pCallLeg,hTransc);
        if(rv != RV_OK)
        {
            *pStatusCode = 600;
            return rv;
        }
    }
    return RV_OK;

}

/***************************************************************************
* InitSubsReferData
* ------------------------------------------------------------------------
* General: Initializes the Subscription-Refer data members in the CallLeg
*           manager object.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pMgr          - A handle to the CallLeg manager object.
*         pCfg          - Pointer to the configuration structure to be used
*                         during initialization.
*
* Output: pSubsCallsNo - The number of CallLegs that have to be allocated
*                         for subscription use.
***************************************************************************/
static void RVCALLCONV InitSubsReferData(IN  CallLegMgr        *pMgr,
                                         IN  SipCallLegMgrCfg  *pCfg,
                                         OUT RvInt32           *pSubsCallsNo)
{
#ifdef RV_SIP_SUBS_ON
    *pSubsCallsNo = pCfg->maxSubscriptions;

    pMgr->maxNumOfHiddenCalls       = pCfg->maxSubscriptions;
    pMgr->numOfAllocatedHiddenCalls = 0;
    pMgr->bEnableSubsForking        = pCfg->bEnableSubsForking;
    pMgr->bDisableRefer3515Behavior = pCfg->bDisableRefer3515Behavior;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pMgr); 
	RV_UNUSED_ARG(pCfg); 

    *pSubsCallsNo = 0;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* HandleUnmatchedNotifyIfNeeded
* ------------------------------------------------------------------------
* General: In case of unmatched incoming NOTIFY, the function checks if it
*          is forked NOTIFY (of Forked Subscription).
*          If Subscription Manager finds out, that this is a forked NOTIFY,
*          and decides to handle the Notification, it should create Call-Leg
*          and supply it back in order to enable set Transaction owner
*          in this function.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  hCallLegMgr   - A handle to the CallLeg manager object.
*         hTransc       - Handle to the newly created NOTIFY transaction.
* Output: phCallLeg     - Pointer to the CallLeg that the Subscription
*                         manager might create.
*         pStatusCode   - defines the response status code in case of failure.
***************************************************************************/
static void RVCALLCONV HandleUnmatchedNotifyIfNeeded(
                            IN  CallLegMgr         *pMgr,
                            IN  RvSipTranscHandle   hTransc,
                            OUT RvSipCallLegHandle *phCallLeg,
                            OUT RvUint16           *pStatusCode)
{
    RvStatus rv;

    *phCallLeg  = NULL;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "HandleUnmatchedNotifyIfNeeded - hTransc=0x%p",hTransc));

    /*  If no Call-Leg leg was matched for incoming NOTIFY,
        it can be a forked NOTIFY (for Forked Subscription).
        See RFC 3265, sections 3.3.3 and 4.4.9.
        Ask Subscription Manager to handle the suspected NOTIFY.
        If Subscription Manager finds out, that this is a forked NOTIFY,
        and decides to handle the Notification, it should create Call-Leg and
        supply it back in order to enable set Transaction owner in this function. */
    if(RV_TRUE==pMgr->bEnableSubsForking)
    {
    /* Unlock Manager in order to enable lock of Call-Legs from
        SipSubsMgrHandleUnmatchedNotify() function */
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

        rv = SipSubsMgrHandleUnmatchedNotify(pMgr->hSubsMgr, hTransc,
            (RvSipCallLegHandle*)phCallLeg, pStatusCode);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "HandleUnmatchedNotifyIfNeeded(hTransc=0x%p): SipSubsMgrHandleUnmatchedNotify(hTransc=0x%p) failed (rv=%d:%s)",
                hTransc, hTransc, rv, SipCommonStatus2Str(rv)));

            if(0==*pStatusCode)
            {
                *pStatusCode = 481;
            }
            /* Lock Manager back */
            RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
            return;
        }
        /* Lock Manager back */
        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */ 

/******************************************************************************
 * PrepareCallbackMasks
 * ----------------------------------------------------------------------------
 * General: The function prepare the masks enabling us to block the RvSipCallLegTerminate()
 *          and RvSipCallLegCancel() APIs, if we are in a middle of a callback, that do 
 *          not allow terminate/cancel inside.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg      - Pointer to the Call-Leg object.
 ******************************************************************************/
static void RVCALLCONV PrepareCallbackMasks(IN CallLegMgr* pCallMgr)
{
    pCallMgr->terminateMask = 0;
    
    /* turn on all the callbacks that should block the termination */
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_CALL_CREATED, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_REINVITE_CREATED, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_TRANSC_CREATED, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_BYE_CREATED, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_SESSION_TIMER_NEGOTIATION_FAULT, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_SESSION_TIMER_REFRESH_ALERT, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_SIGCOMP_NOT_RESPONDED, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_NESTED_INITIAL_REQ, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_NEW_CONN_IN_USE, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_CREATED_DUE_TO_FORKING, 1);
    BITS_SetBit((void*)(&pCallMgr->terminateMask), CB_CALL_PROV_RESP_RCVD, 1);

    pCallMgr->cancelMask = 0;
    BITS_SetBit((void*)(&pCallMgr->cancelMask), CB_CALL_MSG_RCVD, 1);
    BITS_SetBit((void*)(&pCallMgr->cancelMask), CB_CALL_MSG_TO_SEND, 1);
    BITS_SetBit((void*)(&pCallMgr->cancelMask), CB_CALL_FINAL_DEST_RESOLVED, 1);   
    BITS_SetBit((void*)(&pCallMgr->cancelMask), CB_CALL_NEW_CONN_IN_USE, 1);
}


#endif /*RV_SIP_PRIMITIVES */


