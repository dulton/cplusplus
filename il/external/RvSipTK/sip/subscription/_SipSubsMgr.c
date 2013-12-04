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
 *                              <SubsMgrObject.h>
 *     The subscription manager object holds all the event notification module
 *   configuration parameters, and a call-leg manager handle.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"
#include "_SipSubsMgr.h"
#include "SubsCallbacks.h"
#include "SubsRefer.h"
#include "SubsNotify.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "rvmemory.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
#include "rvlog.h"
#include "RvSipCallLegTypes.h"
#include "AdsCopybits.h"

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#include "RvSipCallLeg.h"
#endif
//SPIRENT_END

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV CreateSubscriptionList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs);

static void RVCALLCONV DestructSubscriptionList(
                               IN SubsMgr  *pMgr);


static RvStatus RVCALLCONV CreateNotificationList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs);


static RvStatus RVCALLCONV CreateTransactionList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs);

static RvStatus RVCALLCONV SubsInitiateLock(IN Subscription  *pSubs);

static RvStatus FindCallLegForNotify(IN  SubsMgr*            pMgr,
                                     IN  RvSipMsgHandle      hMsg,
                                     OUT RvSipCallLegHandle  *phCallLeg);

static RvStatus FindCallLegForNotifyOfForkedSubs(
                                     IN  SubsMgr*            pMgr,
                                     IN  RvSipMsgHandle      hMsg,
                                     OUT RvSipCallLegHandle  *phCallLeg);

static RvStatus FindSubsInCallLegByMsg(IN  SubsMgr             *pSubsMgr,
                                       IN  RvSipCallLegHandle  hCallLeg,
                                       IN  RvSipMsgHandle      hMsg,
                                       OUT Subscription        **ppSubs);

static RvStatus ForkedSubsCreate(IN  SubsMgr             *pMgr,
                                 IN  Subscription        *pSubsOriginal,
                                 IN  RvSipMsgHandle      hMsg,
                                 OUT Subscription        **ppSubs);

static void RVCALLCONV DestructAllSubscriptions(IN SubsMgr  *pMgr);

static void RVCALLCONV PrepareCallbackMasks(IN SubsMgr* pMgr);
#endif /* #ifdef RV_SIP_SUBS_ON - static functions */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipSubsMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new subscription manager.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the call-leg mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   call-leg manager.
 *               RV_OK -        Call-leg manager was created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     cfg - struct with configuration parameters.
 * Output:     *phSubsMgr - Handle to the newly created subs manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrConstruct(
                            IN  SipSubsMgrCfg          *cfg,
                            OUT RvSipSubsMgrHandle     *phSubsMgr)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus   rv;
    SubsMgr    *pMgr;
    RvStatus    crv;

    *phSubsMgr = NULL;

    if(cfg->maxNumOfSubscriptions <= 0)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
            "SipSubsMgrConstruct - Failed , given maxNumOfSubscriptions = 0"));
        return RV_ERROR_UNKNOWN;
    }
    /*------------------------------------------------------------
       allocates the subs manager structure and initialize it
    --------------------------------------------------------------*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(SubsMgr), cfg->pLogMgr, (void*)&pMgr))
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                  "SipSubsMgrConstruct - Failed ,RvMemoryAlloc failed"));
        return RV_ERROR_UNKNOWN;
    }

    /* Initializing the struct members to NULL (0) */
    memset(pMgr, 0, sizeof(SubsMgr));

    /* init callLegMgr parameters*/
    pMgr->pLogMgr         = cfg->pLogMgr;
    pMgr->hStack          = cfg->hStack;
    pMgr->pLogSrc           = cfg->pLogSrc;
    pMgr->hMsgMgr         = cfg->hMsgMgr;
    pMgr->hCallLegMgr     = cfg->hCallLegMgr;
    pMgr->hTransportMgr   = cfg->hTransportMgr;
    pMgr->hTransactionMgr = cfg->hTransactionMgr;
    pMgr->hGeneralPool    = cfg->hGeneralPool;
    pMgr->hElementPool     = cfg->hElementPool;
    pMgr->alertTimeout    = cfg->alertTimeout;
    pMgr->autoRefresh     = cfg->autoRefresh;
    pMgr->noNotifyTimeout = cfg->noNotifyTimeout;
    pMgr->seed            = cfg->seed;
    pMgr->bDisableRefer3515Behavior = cfg->bDisableRefer3515Behavior;
    RvSelectGetThreadEngine(pMgr->pLogMgr, &pMgr->pSelect);
    PrepareCallbackMasks(pMgr);

    /*set a mutex for the manager. */
    crv = RvMutexConstruct(pMgr->pLogMgr, &pMgr->hMutex);
    if(crv != RV_OK)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                 "SipSubsMgrConstruct - Failed to construct manager mutex (rv=%d)", CRV2RV(crv)));
        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return CRV2RV(crv);
    }


    /*-------------------------------------------------------
    initialize notifications lists
    ---------------------------------------------------------*/
    rv = CreateNotificationList(pMgr, cfg->maxNumOfSubscriptions);
    if(rv != RV_OK)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
            "SipSubsMgrConstruct - Failed to initialize notifications list (rv=%d)", rv));
           RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*-------------------------------------------------------
    initialize subs transaction lists
    ---------------------------------------------------------*/
    rv = CreateTransactionList(pMgr, cfg->maxNumOfSubscriptions);
    if(rv != RV_OK)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
            "SipSubsMgrConstruct - Failed to initialize transactions list (rv=%d)", rv));
        RLIST_PoolListDestruct(pMgr->hNotifyListPool);
        RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }



    /*-------------------------------------------------------
    initialize subscriptions list
    ---------------------------------------------------------*/
    rv = CreateSubscriptionList(pMgr, cfg->maxNumOfSubscriptions);
    if(rv != RV_OK)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
            "SipSubsMgrConstruct - Failed to initialize subscriptions list (rv=%d)", rv));
        RLIST_PoolListDestruct(pMgr->hNotifyListPool);
        RLIST_PoolListDestruct(pMgr->hTranscHandlesPool);
        RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);
        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }


    RvLogInfo(cfg->pLogSrc,(cfg->pLogSrc,
              "SipSubsMgrConstruct - Subscription manager was constructed successfully, Num of subscriptions = %d",
              cfg->maxNumOfSubscriptions));

    *phSubsMgr = (RvSipSubsMgrHandle)pMgr;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(cfg);
    *phSubsMgr = NULL;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the subscription manager freeing all resources.
 * Return Value:  RV_OK -  subscription manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubsMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrDestruct(
                                   IN RvSipSubsMgrHandle hSubsMgr)
{
#ifdef RV_SIP_SUBS_ON
    SubsMgr            *pMgr = (SubsMgr*)hSubsMgr;
    RvLogSource*  pLogSrc;

    if(hSubsMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/

    pLogSrc = pMgr->pLogSrc;

    DestructAllSubscriptions(pMgr);

    /*free the notification pool and notification list */
    if(pMgr->hNotifyListPool != NULL)
    {
        RLIST_PoolListDestruct(pMgr->hNotifyListPool);
    }

    if(pMgr->hTranscHandlesPool != NULL)
    {
        RLIST_PoolListDestruct(pMgr->hTranscHandlesPool);
    }

    DestructSubscriptionList(pMgr);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/
    RvMutexDestruct(&pMgr->hMutex,pMgr->pLogMgr);

    /* free subscription mgr data structure */
    RvMemoryFree((void*)pMgr, pMgr->pLogMgr);

    RvLogInfo(pLogSrc,(pLogSrc,
              "SipSubsMgrDestruct - Subscription manager was destructed successfully"));
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubsMgr);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the subscription module. It includes the subscription list, the
 *          notification lists and the list items.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrGetResourcesStatus (
                                 IN  RvSipSubsMgrHandle   hMgr,
                                 OUT RvSipSubsResources  *pResources)
{
#ifdef RV_SIP_SUBS_ON
    SubsMgr *pMgr = (SubsMgr*)hMgr;

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

    memset(pResources,0,sizeof(RvSipSubsResources));
    if(pMgr == NULL)
    {
        return RV_OK;
    }

    /* subscription list */
    if(pMgr->hSubsListPool != NULL)
    {
        RLIST_GetResourcesStatus(pMgr->hSubsListPool,
                             &allocNumOfLists, /*allways 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*allways 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

        pResources->subscriptions.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->subscriptions.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->subscriptions.maxUsageOfElements = maxUsageInUserElement;
    }
    /*notification lists and lists items*/
    if(pMgr->hNotifyListPool != NULL)
    {
        RLIST_GetResourcesStatus(pMgr->hNotifyListPool,
                             &allocNumOfLists,
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

        pResources->notifyLists.numOfAllocatedElements = allocNumOfLists;
        pResources->notifyLists.currNumOfUsedElements = currNumOfUsedLists;
        pResources->notifyLists.maxUsageOfElements = maxUsageInLists;
        pResources->notifications.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->notifications.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->notifications.maxUsageOfElements = maxUsageInUserElement;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hMgr);
    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    memset(pResources,0,sizeof(RvSipSubsResources));

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of subscriptions that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The subscription manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrResetMaxUsageResourcesStatus (
                                     IN  RvSipSubsMgrHandle  hMgr)
{
#ifdef RV_SIP_SUBS_ON
    SubsMgr *pMgr = (SubsMgr*)hMgr;

    RLIST_ResetMaxUsageResourcesStatus(pMgr->hSubsListPool);

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hMgr);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}


/***************************************************************************
 * SipSubsMgrSubscriptionCreate
 * ------------------------------------------------------------------------
 * General: Creates a new subscription (regular subscriber subscription, or
 *          out of band subscription - subscriber/notifier):
 *          1. create subscription object.
 *          2. create a hidden call-leg, if needed.
 *          3. initiate subscription and set it in call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr      - Pointer to the subs manager.
 *          hCallLeg  - Handle to a connected call-leg, or NULL - to create a
 *                      hidden call-leg.
 *          hAppsubs   - Handle to the application subs object.
 *          eSubsType  - Subscriber or Notifier.
 *          bOutOfBand - a regular subscription, or out of band subscription.
 * Output:  phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
RvStatus SipSubsMgrSubscriptionCreate( IN    RvSipSubsMgrHandle    hMgr,
                                       IN    RvSipCallLegHandle    hCallLeg,
                                       IN    RvSipAppSubsHandle    hAppSubs,
                                       IN    RvSipSubscriptionType eSubsType,
                                       IN    RvBool                bOutOfBand,
                                       OUT   RvSipSubsHandle      *phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                       )
{
#ifdef RV_SIP_SUBS_ON
    RvStatus      rv;
    SubsMgr       *pMgr = (SubsMgr*)hMgr;

    if(phSubs == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    *phSubs = NULL;
    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(hCallLeg) {
      cctContext=RvSipCallLegGetCct(hCallLeg);
      URI_cfg=RvSipCallLegGetURIFlag(hCallLeg);
    }
#endif
    //SPIRENT_END

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SipSubsMgrSubscriptionCreate - Creating a new subscription. hCallLeg = 0x%p", hCallLeg));

    /*create a new subscription. This will lock the subscription manager*/
    rv = SubsMgrSubscriptionCreate(hMgr, hCallLeg, hAppSubs,
                                   eSubsType, bOutOfBand, phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,cctContext,URI_cfg
#endif
//SPIRENT_END
                                   );
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrSubscriptionCreate - Failed, SubsMgr failed to create a subscription"));
        return rv;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hMgr);
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hAppSubs);
    RV_UNUSED_ARG(eSubsType);
    RV_UNUSED_ARG(bOutOfBand);

    *phSubs = NULL;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsMgrSubsTranscCreated
 * ------------------------------------------------------------------------
 * General: When a new SUBSCRIBE request is received the callLeg manager find the
 *          related call-leg (or create a hidden one),
 *          and then calls this function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr   - Handle to the subs manager
 *            hCallLeg   - Handle to the subscription call-leg.
 *            hTransc    - Handle to the new transaction.
 *            eMethod    - The method of the new transaction (SUBSCRIBE/REFER/NOTIFY)
 * Output:    pRejectCode - the reject-code to use, in case of transaction rejection.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrSubsTranscCreated(IN RvSipSubsMgrHandle     hSubsMgr,
                                                IN RvSipCallLegHandle     hCallLeg,
                                                IN RvSipTranscHandle      hTransc,
                                                IN SipTransactionMethod   eMethod,
                                                OUT RvUint16*             pRejectCode)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus                    rv;
    RvSipSubsHandle             hSubs           = NULL;
    RvSipMsgHandle              hMsg;
    RvSipEventHeaderHandle      hEvent          = NULL;
    RvSipHeaderListElemHandle   hListElem;
    SubsMgr*                    pMgr            = (SubsMgr*)hSubsMgr;
    Subscription*               pSubs           = NULL;
    RvBool                      bFoundExistSubs = RV_FALSE;

    if(hSubsMgr == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrSubsTranscCreated - max subscriptions = 0"));
        return RV_ERROR_UNKNOWN;
    }

    /* find the correct subscription or create it */
    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrSubsTranscCreated - Failed to get received msg from transc 0x%p (rv=%d)",hTransc, rv));
        return rv;
    }

    /* if the method is REFER, we know we must create a new subscription.
    (the cseq of the refer transaction must have a new value, otherwise the call-leg
     would reject this invalid request. and for each refer cseq, we create a new
     refer request, with a new event id)
    if SUBSCRIBE it may be a refresh or unsubscribe to an already exists subscription */
    if(SIP_TRANSACTION_METHOD_REFER != eMethod)
    {
       hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EVENT,
                                                            RVSIP_FIRST_HEADER, &hListElem);

        /* When a subscribe transaction is created, we look for a notifier subscription */
        rv = SipCallLegSubsFindSubscription(hCallLeg, hEvent, RVSIP_SUBS_TYPE_NOTIFIER, &hSubs);
        if(rv == RV_OK)
        {
            bFoundExistSubs = RV_TRUE;
            pSubs = (Subscription*)hSubs;
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SipSubsMgrSubsTranscCreated - subscription 0x%p was found for transaction 0x%p",
                hSubs,hTransc));
        }
    }
    else /* REFER */
    {
        if(RV_TRUE == SipCallLegIsHiddenCallLeg(hCallLeg))
        {
            if(RV_TRUE == SipCallLegIsFirstReferExists(hCallLeg))
            {
             /* if this is a REFER request on a hidden call-leg, and refer subscription already
                exist - we must reject it!!!
                (in established call-leg case, each refer request has a different cseq, and
                therefore a different event-id, so each one creates a new refer subscription...)*/
                RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "SipSubsMgrSubsTranscCreated - refer subscription already exist on hidden call-leg 0x%p. reject refer transaction 0x%p",
                    hCallLeg,hTransc));
                *pRejectCode = 400;
                return RV_OK;

            }
        }
        else /* not hidden call-leg */
        {
            /* if we are working with refer wrappers, we don't support refer inside refer. */
            if( RV_TRUE == pMgr->bDisableRefer3515Behavior &&
                NULL != SipCallLegGetActiveReferSubs(hCallLeg))
            {
                RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "SipSubsMgrSubsTranscCreated - call-leg 0x%p - REFER inside REFER is not supported when working with old refer",
                     hCallLeg));
                return RV_OK;
            }
        }
    }

    /* if there is no exists subscription - create subscription */
    if(RV_FALSE == bFoundExistSubs)
    {
        rv = SubsMgrSubscriptionObjCreate(pMgr, hCallLeg, NULL, RVSIP_SUBS_TYPE_NOTIFIER, RV_FALSE, &hSubs);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SipSubsMgrSubsTranscCreated - Failed to create new subscription(rv=%d)", rv));
            return rv;
        }
        pSubs = (Subscription*)hSubs;

        if(SIP_TRANSACTION_METHOD_REFER == eMethod)
        {
            rv = SubsReferIncomingReferInitialize(pMgr, pSubs, hTransc, hMsg);
            if(rv!= RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "SipSubsMgrSubsTranscCreated - Failed to set refer info in subscription 0x%p (rv=%d)", pSubs, rv));
                SubsFree(pSubs);
                return rv;
            }
        }
        else /* SUBSCIRBE */
        {
            /* set the event header in subscription */
            if(hEvent != NULL)
            {
                rv = SubsSetEventHeader(pSubs, hEvent);
                if(rv!= RV_OK)
                {
                    RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "SipSubsMgrSubsTranscCreated - Failed to set Event header in subscription(rv=%d)", rv));
                    SubsFree(pSubs);
                    return rv;
                }
            }
            else /* no event header - case of default value 'PINT' */
            {
                pSubs->bEventHeaderExist = RV_FALSE;
            }
        }
        /* This is an incoming subscription - inform user of it */
        /* save the transaction in the subscription, only to the time of
        the created callback. */
        pSubs->hActiveTransc = hTransc;
        SubsCallbackSubsCreatedEv(pMgr, pSubs);
        pSubs->hActiveTransc = NULL;

        if(pSubs->createdRejectStatusCode > 0 || pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            if(pSubs->createdRejectStatusCode > 0)
            {
                RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "SipSubsMgrSubsTranscCreated - subscription 0x%p - the application indicated that the SUBSCRIBE should be rejected. The subscription object will be destructed", 
                        hSubs));
                /*the subscription was rejected from the created callback*/
                *pRejectCode = pSubs->createdRejectStatusCode;
				SubsFree(pSubs);
            }
            else /* eState == RVSIP_SUBS_STATE_TERMINATED */
            {
                RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "SipSubsMgrSubsTranscCreated - subscription 0x%p - subscription was terminated in callback. reject with 600", 
                        hSubs));
                /*the subscription was rejected from the created callback*/
                *pRejectCode = 600;
            }
            return RV_OK;

        }
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "SipSubsMgrSubsTranscCreated - A new subscription 0x%p was created for transc 0x%p",
              hSubs, hTransc));
    }

    /*Attach the subscription to the transaction */
    rv = SipTransactionSetSubsInfo(hTransc, (void*)hSubs);
    if (rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrSubsTranscCreated - Subscription 0x%p - failed to set subs info in transaction 0x%p",
            hSubs,hTransc));
        return rv;
    }

    rv = SubsAddTranscToList(pSubs, hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SipSubsMgrSubsTranscCreated: Subscription 0x%p - failed to add transaction 0x%p to list. terminate it",
            pSubs, hTransc));
        if(RV_FALSE == bFoundExistSubs)
        {
            /* this is a newly created subscription. terminate it */
            SubsTerminate(pSubs, RVSIP_SUBS_REASON_LOCAL_FAILURE);
            return rv;
        }
        else
        {
            /* returning failure will reject the request,
              and will not associate it with a dialog */
            return rv;
        }
    }

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubsMgr);
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethod);

    *pRejectCode = 0;
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/******************************************************************************
 * SipSubsMgrNotifyTranscCreated
 * ----------------------------------------------------------------------------
 * General: When a new NOTIFY request is received the callLeg manager find the
 *          related call-leg, and then calls this function.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubsMgr        - Handle to the subs manager
 *          hCallLeg        - Handle to the subscription call-leg.
 *          hTransc         - Handle to the NOTIFY transaction.
 * Output:  bWasHandled     - Was the NOTIFY transaction handled.
 *****************************************************************************/
RvStatus RVCALLCONV SipSubsMgrNotifyTranscCreated(
                                        IN  RvSipSubsMgrHandle  hSubsMgr,
                                        IN  RvSipCallLegHandle  hCallLeg,
                                        IN  RvSipTranscHandle   hTransc,
                                        OUT RvBool              *bWasHandled)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus              rv;
    RvSipSubsHandle        hSubs;
    RvSipNotifyHandle      hNotify;
    RvSipMsgHandle         hMsg;
    RvSipEventHeaderHandle hEvent;
    RvSipHeaderListElemHandle hListElem;
    SubsMgr*               pMgr = (SubsMgr*)hSubsMgr;
    Subscription*          pSubs = NULL;
    Notification*          pNotify = NULL;
	RvInt32				   exCseqStep; 

    *bWasHandled = RV_TRUE;

    if(hSubsMgr == NULL)
    {
        *bWasHandled = RV_FALSE;
        return RV_OK;
    }

    /*find subscription*/
    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EVENT, RVSIP_FIRST_HEADER, &hListElem);

    /* when a new notify transaction is created, we look for a subscriber subscription */
    rv = SipCallLegSubsFindSubscription(hCallLeg, hEvent, RVSIP_SUBS_TYPE_SUBSCRIBER, &hSubs);
    if(rv != RV_OK)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrNotifyTranscCreated - Failed to find a subscription for this notify transaction"));
        *bWasHandled = RV_FALSE;
        return RV_OK;
    }

    /* else - a subscription was found */
    pSubs = (Subscription*)hSubs;

    /*  Ensure, that NOTIFY, which caused creation of Forked Subscription,
    is processed before any other NOTIFY.
        Such situation is possible, when some Notifier sends two sequential
    NOTIFies, the first of which is creating forked Subscription in another
    thread, the second of which is being processed by the current thread. */
    /* Check only Forked Subscription, for which the first NOTIFY processing
    was not finished yet */
	rv = SipCommonCSeqGetStep(&pSubs->expectedCSeq,&exCseqStep);
    if (NULL != pSubs->pOriginalSubs && RV_OK == rv)
    {
        RvInt32 cseqTransc = UNDEFINED;

        rv = RvSipTransactionGetCSeqStep(hTransc,&cseqTransc);
		if (RV_OK != rv)
		{	
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SipSubsMgrNotifyTranscCreated - hCallLeg=0x%p,hTransc=0x%p - failed in RvSipTransactionGetCSeqStep()",
                hCallLeg,hTransc));
            *bWasHandled = RV_FALSE;
            return rv;
		}
        if (exCseqStep != cseqTransc)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SipSubsMgrNotifyTranscCreated - hCallLeg=0x%p,hTransc=0x%p - processing of the first NOTIFY for Forked Subscription 0x%p was not finished yet",
                hCallLeg,hTransc,pSubs));
            *bWasHandled = RV_FALSE;
            return RV_ERROR_UNKNOWN;
        }
        /* Cancel the check for future NOTIFies */
		SipCommonCSeqReset(&pSubs->expectedCSeq);  
    }

   /*------------------------------
    create a notification object
    ------------------------------ */
    rv = SubsNotifyCreateObjInSubs(pSubs, NULL, &hNotify);
    if(rv != RV_OK || hNotify == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrNotifyTranscCreated - Failed to create new notification in subscription 0x%p", hSubs));
        return rv;
    }

    pNotify = (Notification*)hNotify;

    /*At this point we have a new incoming notification object
    - inform user of it */
    SubsCallbackNotifyTranscCreatedEv(pMgr, pSubs, pNotify);

    /*Attach the transaction to the notification object */
    rv = SipTransactionSetSubsInfo(hTransc, (void*)hNotify);
    if (rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrNotifyTranscCreated - Subscription 0x%p - failed to set subs info in transaction 0x%p",
            pSubs,hTransc));
        return rv;
    }
    pNotify->hTransc = hTransc;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubsMgr);
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hTransc);

    *bWasHandled = RV_FALSE;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/******************************************************************************
 * SipSubsMgrHandleUnmatchedNotify
 * ----------------------------------------------------------------------------
 * General: The function is called by Call-Leg Layer, when it handles NOTIFY,
 *          which doesn't match any existing Call-Leg. It checks, if the NOTIFY
 *          doesn't match any Call-Leg because it was sent in response to
 *          forked SUBSCRIBE with TO-tag, different from TO-tag of existing
 *          Call-Legs.
 *          See RFC 3265 sections 3.3.3 and 4.4.9 for Forked Subscriptions.
 *          If it indeed was sent in response to forked SUBSCRIBE, mapped to
 *          existing Subscription (let's call it 'Original Subscription'),
 *          the function will create forked Subscription with underlying
 *          Call-Leg in state 2xx received, and will return handle of this
 *          hidden Call-Leg back to Call-Leg Layer. Call-Leg Layer will set
 *          this Call-Leg to be owner of the NOTIFY. As a result, the NOTIFY
 *          will be processed as a regular NOTIFY, arrived for existing
 *          Subscription on existing Call-Leg.
 *
 * Return Value: RV_OK on successful finish,
 *               error code, defined in RV_SIP_DEF.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr  - handle to Subscription Manager.
 *            hTtrans   - handle to Transaction, which handles incoming NOTIFY.
 * Output:    phCallLeg - pointer to the memory, where handle of the created
 *                        by function Call-Leg will be stored.
 *            pRejectStatus-pointer to the memory, where code of reject for
 *                        incoming NOTIFY will be stored by the function.
 *****************************************************************************/
RvStatus SipSubsMgrHandleUnmatchedNotify(
                                IN    RvSipSubsMgrHandle     hSubsMgr,
                                IN    RvSipTranscHandle      hTransc,
                                OUT   RvSipCallLegHandle     *phCallLeg,
                                OUT   RvUint16               *pRejectStatus)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus            rv;
    SubsMgr            *pMgr  = (SubsMgr*)hSubsMgr;
    Subscription       *pSubs = NULL, *pSubsOrig = NULL;
    RvBool              bForkedSubsEnabled = RV_TRUE;
    RvSipCallLegHandle  hCallLegOrigSubs   = NULL;
    RvSipCallLegHandle  hCallLeg           = NULL;
    RvSipMsgHandle      hMsgNotify;
	RvBool              bNotifyHasExpires  = RV_FALSE;
	RvInt32				notifyCSeq;
	RvInt32             expVal             = UNDEFINED;

    if (NULL==pMgr || NULL==hTransc || NULL==phCallLeg)
    {
        return RV_ERROR_BADPARAM;
    }

    *pRejectStatus = 0;

    /* Check, if Application registered to the ForkedSubscription callback.
    If it doesn't, there is no way to expose to it the Forked Subscription.
    So there is no need in Forked Subscription creation. In this case just
    return. The NOTIFY will be handled by General Transaction. */
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
    if (NULL == pMgr->subsEvHandlers.pfnSubsCreatedDueToForkingEvHandler)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - SubsCreatedDueToForking callback was not set by Application",
            hTransc));
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return RV_OK;
    }
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsgNotify);
    if(RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed RvSipTransactionGetReceivedMsg() (rv=%d:%s)",
            hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }

    /* Find dialog, all identifiers of which except the TO-tag are
    identical to these of the NOTIFY. If such dialog doesn't exist don't handle
    the NOTIFY,just return. The NOTIFY will be handled by General Transaction*/
    rv = FindCallLegForNotifyOfForkedSubs(pMgr,hMsgNotify,&hCallLegOrigSubs);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed FindCallLegForNotifyForForkedSubs() (rv=%d:%s)",
            hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }
    if (NULL == hCallLegOrigSubs)
    {
        return RV_OK;
    }
    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - found original call 0x%p of original subs",
         hTransc, hCallLegOrigSubs));

    /* The Call-Leg Manager should be unlocked at this point,
    because FindSubsInCallLegByMsg() will lock the Call-Leg
    when calling SipCallLegSubsFindSubscription() */

    /* Find Subscription, which is owner of the found Call-Leg and which has
    an Event Package equal to this of the NOTIFY. If such Subscription doesn't
    exist, don't handle the NOTIFY, just return. The NOTIFY will be handled by
    General Transaction. */
    /* This function call Call-Leg Layer API function, which locks the Call */
    rv = FindSubsInCallLegByMsg(pMgr,hCallLegOrigSubs,hMsgNotify,&pSubsOrig);
    if(rv!=RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed FindSubsInCallLegByMsg(hCallLegOrigSubs=0x%p,hTransc=0x%p) (rv=%d:%s)",
            hTransc, hCallLegOrigSubs, hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }
    if (NULL == pSubsOrig)
    {
        return RV_OK;
    }
    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - found original Subs 0x%p ",
        hTransc, pSubsOrig));


    /* Lock the Original Subscription (and underlying CallLeg) before
    it will be used for the Forked Subscription initialization */
    rv = SubsLockAPI(pSubsOrig);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed to lock Subscription 0x%p (rv=%d:%s)",
            hTransc, pSubsOrig, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }
    /* Check, that the Original Subscription still valid */
    if (RVSIP_SUBS_STATE_TERMINATING==pSubsOrig->eState ||
        RVSIP_SUBS_STATE_TERMINATED ==pSubsOrig->eState ||
        RVSIP_SUBS_STATE_IDLE       ==pSubsOrig->eState)
    {
        SubsUnLockAPI(pSubsOrig);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed to lock Subscription 0x%p (rv=%d:%s)",
            hTransc, pSubsOrig, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 481;
        return RV_ERROR_DESTRUCTED;
    }

    /* Check,if Application disabled Forking of the found Original Subscription
    If it does, no Forked Subscription should be created, the NOTIFY should be
    rejected with 481 response status. See RFC 3265, sections 3.3.3 and 4.4.9*/
    rv = SipCallLegGetForkingEnabledFlag(hCallLegOrigSubs,&bForkedSubsEnabled);
    if(RV_OK != rv)
    {
        SubsUnLockAPI(pSubsOrig);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed RvSipCallLegGetForkingEnabledFlag(hCallLeg=0x%p) (rv=%d:%s)",
            hTransc, hCallLegOrigSubs, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }
    if (RV_FALSE==bForkedSubsEnabled)
    {
        SubsUnLockAPI(pSubsOrig);
        *pRejectStatus = 481;
        return RV_OK;
    }

    /* Create Subscription object with underlying hidden Call-Leg. The object
    will represent Forked Subscription. The Call-Leg will be inserted
    in Call-Leg Manager hash later, while locking Call-Leg Manager. */
    rv = ForkedSubsCreate(pMgr,pSubsOrig,hMsgNotify,&pSubs);
    if(RV_OK != rv)
    {
        SubsUnLockAPI(pSubsOrig);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed ForkedSubsCreate(pSubs=0x%p,hTransc=0x%p) (rv=%d:%s)",
            hTransc, pSubs, hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        return rv;
    }

    /* Store CSeq of the NOTIFY. It processing should be finished before any other
    NOTIFY will be handled on the same dialog */
    rv = RvSipTransactionGetCSeqStep(hTransc,&notifyCSeq);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed RvSipTransactionGetCSeqStep() (rv=%d:%s)",
            hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        SubsFree(pSubs);
        SubsUnLockAPI(pSubsOrig);
        return rv;
    }
	rv = SipCommonCSeqSetStep(notifyCSeq,&pSubs->expectedCSeq);
	if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed SipCommonCSeqSetStep() (rv=%d:%s)",
            hTransc, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        SubsFree(pSubs);
        SubsUnLockAPI(pSubsOrig);
        return rv;
    }

    /* There is no need in Original Subscription/Call-Leg already.Unlock it. */
    SubsUnLockAPI(pSubsOrig);

    /* Lock the created Subscription */
    rv = SubsLockMsg(pSubs);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed SubsLockMsg(hSubs=0x%p) (rv=%d:%s)",
            hTransc, pSubs, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        SubsFree(pSubs);
        return rv;
    }

    /* Lock Call-Leg Manager before Call-Leg for new Forked Subscription
    will be inserted into hash */
    SipCallLegMgrLock(pMgr->hCallLegMgr);

    /* Ensure, that the Forked Subscription for the dialog with NOTIFY sender
    was not created by another thread meanwhile, as a result of handling of
    another NOTIFY from the same sender */
    rv = FindCallLegForNotify(pMgr,hMsgNotify,&hCallLeg);
    if (RV_OK != rv  ||  NULL != hCallLeg)
    {
        SipCallLegMgrUnlock(pMgr->hCallLegMgr);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed FindCallLegForNotify(hMsgNotify=0x%p). hCallLeg=0x%p. (rv=%d:%s)",
            hTransc, hMsgNotify, hCallLeg, rv, SipCommonStatus2Str(rv)));
        SubsUnLockMsg(pSubs);
        SubsFree(pSubs);
        *pRejectStatus = 480;
        return rv;
    }

    /* Insert new Subscription Call-Leg into the Call-Leg Manager hash */
    rv = SipCallLegMgrHashInsert(pSubs->hCallLeg);
    if (RV_OK != rv)
    {
        SipCallLegMgrUnlock(pMgr->hCallLegMgr);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed SipCallLegMgrHashInsert(hMsgNotify=0x%p) (rv=%d:%s)",
            hTransc, hMsgNotify, rv, SipCommonStatus2Str(rv)));
        SubsUnLockMsg(pSubs);
        SubsFree(pSubs);
        *pRejectStatus = 480;
        return rv;
    }

    /* Unlock Call-Leg Manager */
    SipCallLegMgrUnlock(pMgr->hCallLegMgr);

    /* Take the expiration value from the NOTIFY message */
    rv = GetExpiresValFromMsg(hMsgNotify, RV_TRUE, &expVal);
    if (RV_OK == rv)
	{
		/* An expires value was obtained successfully from the NOTIFY message */
		bNotifyHasExpires			  = RV_TRUE;
		pSubs->requestedExpirationVal = expVal;
	}
	else if (RV_OK != rv && RV_ERROR_NOT_FOUND != rv)
	{
		/* We failed to check if there is an expires value in the NOTIFY message */
		RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				   "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed GetExpiresValFromMsg(hMsg=0x%p) (rv=%d:%s)",
				   hTransc, hMsgNotify, rv, SipCommonStatus2Str(rv)));
	}
	
    
    /* Inform Application about Forked Subscription creation.
    Application should not terminate the Subscription from the callback */
    SubsCallbackSubsCreatedDueToForkingEv(
        pSubs, &pSubs->requestedExpirationVal, &pSubs->hAppSubs, pRejectStatus);
    /* If Application rejects the creation, destruct the Subscription and return */
    if (0 != *pRejectStatus)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - Application rejects forked Subs=0x%p. Reject Code=%d",
            hTransc, pSubs,*pRejectStatus));
        SipCallLegSubsRemoveHiddenCallFromHash(pSubs->hCallLeg);
        SubsDestruct(pSubs);
        SubsUnLockMsg(pSubs);
        return RV_OK;
    }

    /* Run Timers */
    /* Run noNotify timer */
    rv = SubsSetNoNotifyTimer(pSubs);
    if(RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed SubsSetNoNotifyTimer(pSubs=0x%p) (rv=%d:%s)",
            hTransc, pSubs, rv, SipCommonStatus2Str(rv)));
        *pRejectStatus = 480;
        SubsTerminate(pSubs,RVSIP_SUBS_REASON_LOCAL_FAILURE);
        SubsUnLockMsg(pSubs);
        return rv;
    }

	if (pSubs->requestedExpirationVal <= 0 && RV_FALSE == bNotifyHasExpires)
	{
		/* There was no expires value in the NOTIFY but there is also no expires value
		   set to the subscription before hand (from the SUBSCRIBE or from the NOTIFY).
		   We use the extra protection expires value in order to set an expiration time
		   to the subscription. Otherwise the subscription would have become zombie */
		pSubs->requestedExpirationVal = RVSIP_SUBS_EXTRA_PROTECTION_EXPIRES_VAL;
	}

    /* Set expiration timer only if the NOTIFY doesn't have an expiration header
       Otherwise the timer will be set during NOTIFY processing. */
    pSubs->expirationVal = pSubs->requestedExpirationVal;
    if (0 < pSubs->expirationVal && RV_FALSE == bNotifyHasExpires)
    {
        rv = SubsCalculateAndSetAlertTimer(pSubs, pSubs->expirationVal);
        if(RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SipSubsMgrHandleUnmatchedNotify - hTransc=0x%p - failed SubsCalculateAndSetAlertTimer(pSubs=0x%p) (rv=%d:%s)",
                hTransc, pSubs, rv, SipCommonStatus2Str(rv)));
            *pRejectStatus = 480;
            SubsTerminate(pSubs,RVSIP_SUBS_REASON_LOCAL_FAILURE);
            SubsUnLockMsg(pSubs);
            return rv;
        }
    }

    *phCallLeg = pSubs->hCallLeg;

    SubsUnLockMsg(pSubs);

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubsMgr);
    RV_UNUSED_ARG(hTransc);

    *phCallLeg     = NULL;
    *pRejectStatus = 0;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SubsInitiateLock
 * ------------------------------------------------------------------------
 * General: Allocates & initiates subscription triple lock
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - subscription list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg   - pointer to Call-Leg
 **************************************************************************/
static RvStatus RVCALLCONV SubsInitiateLock(IN Subscription  *pSubs)
{
    RvStatus        crv;

    crv = RvMutexConstruct(pSubs->pMgr->pLogMgr, &pSubs->subsLock);
    if(crv != RV_OK)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    pSubs->tripleLock = NULL;
    return RV_OK;
}

/***************************************************************************
 * CreateSubscriptionList
 * ------------------------------------------------------------------------
 * General: Allocated the list of subscriptions managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Subscription list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr          - Pointer to the manager
 *            maxNumOfSubss - Max number of subscriptions to allocate
 ***************************************************************************/
static RvStatus RVCALLCONV CreateSubscriptionList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs)
{

    RvInt            i,j;
    RvStatus        retCode;
    Subscription    *pSubs;

    /*allocate a pool with 1 list*/
    pMgr->hSubsListPool = RLIST_PoolListConstruct(maxNumOfSubs, 1,
                                                 sizeof(Subscription),
                                                 pMgr->pLogMgr,
                                                 "Subscriptions List");

    if(pMgr->hSubsListPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pMgr->hSubsList = RLIST_ListConstruct(pMgr->hSubsListPool);

    if(pMgr->hSubsList == NULL)
    {
        RLIST_PoolListDestruct(pMgr->hSubsListPool);
        return RV_ERROR_OUTOFRESOURCES;
    }

    for (i=0; i < maxNumOfSubs; i++)
    {
        retCode = RLIST_InsertTail(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
        pSubs->pMgr = pMgr;
        retCode = SubsInitiateLock(pSubs);
        if(retCode != RV_OK)
        {
            /*free the locks that were already allocated and free the list.*/
            for (j=0; j < i; j++)
            {
                RLIST_GetHead(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
                RvMutexDestruct(&pSubs->subsLock, pMgr->pLogMgr);
            }
            RLIST_PoolListDestruct(pMgr->hSubsListPool);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /* remove all subscriptions from list */
    for (i=0; i < maxNumOfSubs; i++)
    {
        RLIST_GetHead(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE)pSubs);
    }

    pMgr->maxNumOfSubs = maxNumOfSubs;
    return RV_OK;
}

/***************************************************************************
 * DestructAllSubscriptions
 * ------------------------------------------------------------------------
 * General: Deallocated the list of subscriptions managed by the manager
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr          - Pointer to the manager
 ***************************************************************************/
static void RVCALLCONV DestructAllSubscriptions(IN SubsMgr  *pMgr)
{
    Subscription    *pNextToKill = NULL;
    Subscription    *pSubs2Kill = NULL;

    /* free the subscription list and list pool*/
    if(pMgr->hSubsListPool == NULL)
    {
        return;
    }

    RLIST_GetHead(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs2Kill);

    while (NULL != pSubs2Kill)
    {
        RLIST_GetNext(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE)pSubs2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
        pSubs2Kill->hTranscList = NULL; /*don't free transactions*/
		pSubs2Kill->hActiveTransc = NULL;
		SubsDestruct(pSubs2Kill);
        pSubs2Kill = pNextToKill;
    }

}

/***************************************************************************
 * DestructSubscriptionList
 * ------------------------------------------------------------------------
 * General: Deallocated the list of subscriptions managed by the manager
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr          - Pointer to the manager
 ***************************************************************************/
static void RVCALLCONV DestructSubscriptionList(
                               IN SubsMgr  *pMgr)
{
    RvInt32        i;
    Subscription    *pSubs;

    /* free the subscription list and list pool*/
    if(pMgr->hSubsListPool == NULL)
    {
        return;
    }

    for (i=0; i < pMgr->maxNumOfSubs; i++)
    {
        RLIST_GetHead(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
        if (pSubs == NULL)
            break;
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE)pSubs);
    }

    for (i=0; i < pMgr->maxNumOfSubs; i++)
    {
        RLIST_InsertTail(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
    }

    for (i=0; i < pMgr->maxNumOfSubs; i++)
    {
        RLIST_GetHead(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE *)&pSubs);
        RvMutexDestruct(&pSubs->subsLock, pMgr->pLogMgr);
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,(RLIST_ITEM_HANDLE)pSubs);
    }

    RLIST_PoolListDestruct(pMgr->hSubsListPool);
}


/***************************************************************************
 * CreateNotificationList
 * ------------------------------------------------------------------------
 * General: Allocated the list of notification objects managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - notification list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr          - Pointer to the manager
 *            maxNumOfSubss - Max number of subscriptions.
 ***************************************************************************/
static RvStatus RVCALLCONV CreateNotificationList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs)
{

    /*allocate a pool with maxNumOfSubs lists. one list per subscription object.
    number of notification objects is :
    num of subscriptions * num of notifications per one subscription. */
    pMgr->hNotifyListPool = RLIST_PoolListConstruct(maxNumOfSubs * NOTIFY_PER_SUBSCRIPTION,
                                                    maxNumOfSubs,
                                                    sizeof(Notification),
                                                    pMgr->pLogMgr,
                                                    "Notification List");

    if(pMgr->hNotifyListPool == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateNotificationList - Subs mgr failed to initialize notify lists pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}

/***************************************************************************
 * CreateTransactionList
 * ------------------------------------------------------------------------
 * General: Allocated the list of transactions saved in each subscription
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr          - Pointer to the manager
 *            maxNumOfSubs  - Max number of subscriptions.
 ***************************************************************************/
static RvStatus RVCALLCONV CreateTransactionList(
                               IN SubsMgr  *pMgr,
                               IN RvInt32  maxNumOfSubs)
{
    /*---------------------------------------------------------------------
        Initialize pool of transaction handles to the usage of the subscription.
        This list holds only the SUBSCRIBE transactions (initial, refresh, unsubs).
        The number of lists in the pool equals to the max number of subscriptions.
        The number of elements in all the list equals the number of subscriptions*2
        (assuming that the Refresh request are sent once a while, and not every 
        minute...)
        -----------------------------------------------------------------------*/
        pMgr->hTranscHandlesPool = RLIST_PoolListConstruct(maxNumOfSubs*2,
                                                           maxNumOfSubs,
                                                           sizeof(RvSipTranscHandle),
                                                           pMgr->pLogMgr,
                                                           "Subs Transc List");

        if(pMgr->hTranscHandlesPool == NULL)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CreateTransactionList - Subs mgr failed to initialize transacion lists pool"));
            return RV_ERROR_OUTOFRESOURCES;
        }
        return RV_OK;
}
/******************************************************************************
* FindCallLegForNotify
* -----------------------------------------------------------------------------
* General:  refer to Call-Leg Layer in order to find Call-Leg with dialog
*           identifiers, matching these of NOTIFY. Call-Leg should be hidden,
*           since the function is called for out-of-dialog Subscription
*           processing.
*
* Return Value: RV_OK on success,
*               Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pMgr      - Pointer to Subscription Manager object.
*           hTransc   - Handle of the NOTIFY transaction.
* Output:   phCallLeg - Pointer to memory, where the found Call-Leg handle will
*                       be stored.NULL will be stored,if no Call-Leg was found.
******************************************************************************/
static RvStatus FindCallLegForNotify(IN  SubsMgr*            pMgr,
                                     IN  RvSipMsgHandle      hMsgNotify,
                                     OUT RvSipCallLegHandle  *phCallLeg)
{
    RvStatus                rv;
    SipTransactionKey       notifyKey;
    RvSipCallLegDirection   eNotifyDirection;

    *phCallLeg = NULL;

    /* Get Dialog key from the message */
    memset(&notifyKey,0,sizeof(SipTransactionKey));
    rv = SipTransactionMgrGetKeyFromMessage(
                         pMgr->hTransactionMgr,hMsgNotify,&notifyKey);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "FindCallLegForNotify - hMsgNotify=0x%p - failed SipTransactionMgrGetKeyFromMessage() failed (rv=%d:%s)",
            hMsgNotify, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Seek for matching Call-Leg */
    eNotifyDirection = RVSIP_CALL_LEG_DIRECTION_INCOMING;
    rv = SipCallLegMgrFindHiddenCallLeg(
        pMgr->hCallLegMgr, &notifyKey, eNotifyDirection, phCallLeg);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "FindCallLegForNotify - hMsgNotify=0x%p - failed SipCallLegMgrFindOriginalHiddenCallLeg() failed (rv=%d:%s)",
            hMsgNotify, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}


/******************************************************************************
* FindCallLegForNotifyOfForkedSubs
* -----------------------------------------------------------------------------
* General:  refer to Call-Leg Layer in order to find Call-Leg, which serves
*           Subscription, to which the Notification should be mapped.
*           Inasmuch the NOTIFY arrived in response to forked SUBSCRIBE,
*           it's FROM tag doesn't match the TO-tag of the Call-Leg.
*           Such Call-Leg is called 'Original Call-Leg'.
*           When Call-Leg database is searched for Original Call-Leg, Call-Leg
*           TO-tag is ignored.
*           In additional, the Call-Leg should be hidden, since NOTIFIES with
*           different FROM-tags can arrive only if TO-tag was not present in
*           SUBSCRIBE request. In turns this is possible only for Subscription,
*           created in out-of-dialog manner. Such Subscription uses hidden
*           Call-Leg only.
*
* Return Value: RV_OK on success,
*               Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pMgr      - Pointer to Subscription Manager object.
*           hTransc   - Handle of the NOTIFY transaction.
* Output:   phCallLeg - Pointer to memory, where the found Call-Leg handle will
*                       be stored.NULL will be stored,if no Call-Leg was found.
******************************************************************************/
static RvStatus FindCallLegForNotifyOfForkedSubs(
                                    IN  SubsMgr*            pMgr,
                                    IN  RvSipMsgHandle      hMsgNotify,
                                    OUT RvSipCallLegHandle  *phCallLeg)
{
    RvStatus                rv;
    SipTransactionKey       notifyKey;
    RvSipCallLegDirection   eNotifyDirection;

    *phCallLeg = NULL;

    /* Get Dialog key from the message */
    memset(&notifyKey,0,sizeof(SipTransactionKey));
    rv = SipTransactionMgrGetKeyFromMessage(
                      pMgr->hTransactionMgr,hMsgNotify,&notifyKey);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "FindCallLegForNotifyOfForkedSubs - hMsgNotify=0x%p - failed SipTransactionMgrGetKeyFromMessage() (rv=%d:%s)",
            hMsgNotify, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Seek for matching Call-Leg */
    eNotifyDirection = RVSIP_CALL_LEG_DIRECTION_INCOMING;
    rv = SipCallLegMgrFindOriginalHiddenCallLeg(
          pMgr->hCallLegMgr, &notifyKey, eNotifyDirection, phCallLeg);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "FindCallLegForNotifyOfForkedSubs - hMsgNotify=0x%p - failed SipCallLegMgrFindOriginalHiddenCallLeg() (rv=%d:%s)",
            hMsgNotify, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
* FindSubsInCallLegByMsg
* -----------------------------------------------------------------------------
* General:  refer to Call-Leg Layer in order to get pointer to Subscription,
*           object, stored in Call-Leg.Subscription should have an Event header
*           identical to Event header in the message,handled by the Transaction
* Return Value: RV_OK on success,
*           Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pMgr      - Pointer to Subscription Manager object.
*           hCallLeg  - Handle of the Call-Leg.
*           hTransc   - Handle of the Transaction.
* Output:   ppSubs    - Pointer to memory, where the found pointer
*                       to Subscription object will be stored.
*                       NULL will be stored, if no matching pointer was found.
******************************************************************************/
static RvStatus FindSubsInCallLegByMsg(IN  SubsMgr             *pSubsMgr,
                                       IN  RvSipCallLegHandle  hCallLeg,
                                       IN  RvSipMsgHandle      hMsg,
                                       OUT Subscription        **ppSubs)
{
    RvStatus                    rv;
    RvSipEventHeaderHandle      hEvent;
    RvSipSubsHandle             hSubs;
    RvSipHeaderListElemHandle   hListElem;

    *ppSubs = NULL;

    hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(
        hMsg, RVSIP_HEADERTYPE_EVENT, RVSIP_FIRST_HEADER, &hListElem);
    if (NULL==hEvent)
    {
        RvLogError(pSubsMgr->pLogSrc,(pSubsMgr->pLogSrc,
            "FindSubsInCallLegByMsg - hCallLeg=0x%p,hMsg=0x%p - failed RvSipMsgGetHeaderByType(hMsg=0x%p)",
            hCallLeg, hMsg, hMsg));
        return RV_ERROR_BADPARAM;
    }

    rv = SipCallLegSubsFindSubscription(hCallLeg, hEvent,
                                        RVSIP_SUBS_TYPE_SUBSCRIBER, &hSubs);
    if(RV_OK != rv)
    {
        if (RV_ERROR_NOT_FOUND == rv)
        {
            return RV_OK;
        }
        RvLogError(pSubsMgr->pLogSrc,(pSubsMgr->pLogSrc,
            "FindSubsInCallLegByMsg - hCallLeg=0x%p,hMsg=0x%p - failed SipCallLegSubsFindSubscription(hCallLeg=0x%p,hEvent=0x%p) (rv=%d:%s)",
            hCallLeg, hMsg, hCallLeg, hEvent, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    *ppSubs = (Subscription*)hSubs;
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSubsMgr);
#endif

    return RV_OK;
}

/******************************************************************************
* ForkedSubsCreate
* -----------------------------------------------------------------------------
* General:  creates Subscription object, representing Forked Subscription.
*           Is called upon receiption NOTIFY request for forked SUBSCIRBE.
*           Creates underlying Call-Leg with dialog identifiers identical
*           to NOTIFY request. Gets new Subscription to '2XX received' state.
* Return Value: RV_OK on success,
*           Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pMgr      - Pointer to Subscription Manager object.
*           pSubsOriginal -Pointer to object,representing Original Subscription
*                       (Subscription, which sent SUBSCRIBE request).
*           hTransc   - Handle of the NOTIFY Transaction.
* Output:   ppSubs    - Pointer to memory,where pointer to the new Subscription
*                       object will be stored by the function.
******************************************************************************/
static RvStatus ForkedSubsCreate(IN  SubsMgr             *pMgr,
                                 IN  Subscription        *pSubsOriginal,
                                 IN  RvSipMsgHandle      hMsg,
                                 OUT Subscription        **ppSubs)
{
    RvStatus        rv;
    RvSipSubsHandle hSubs;
    Subscription    *pSubs;

    rv = SubsMgrSubscriptionCreate((RvSipSubsMgrHandle)pMgr,
                                    NULL /*hCallLeg=NULL*/,
                                    NULL /*hAppSubs=NULL*/,
                                    RVSIP_SUBS_TYPE_SUBSCRIBER,
                                    RV_FALSE/*bOutOfBand=RV_FALSE*/,
                                    &hSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,-1/*cctContext*/,0xFF/*URI_cfg*/  /* these parameters will be initialized in the fork call leg function */
#endif
//SPIRENT_END
                                    );
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "ForkedSubsCreate - hSubsOrig=0x%p,hMsg=0x%p - failed SipSubsMgrSubscriptionCreate() (rv=%d:%s)",
            pSubsOriginal, hMsg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "ForkedSubsCreate - hSubsOrig=0x%p,hMsg=0x%p - new forked subs = 0x%p",
            pSubsOriginal, hMsg, hSubs));

    pSubs = (Subscription*)hSubs;

    /* Update new Call-Leg with information from the Message and from Original
    Call-Leg, inserts the Call-Leg into hash */
    rv = SipCallLegForkingInitCallLeg(pMgr->hCallLegMgr, pSubs->hCallLeg,
            pSubsOriginal->hCallLeg, hMsg, RV_TRUE /*bSwitchToAndFromTags=RV_TRUE*/);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "ForkedSubsCreate - hSubsOrig=0x%p,hMsg=0x%p - failed SipCallLegForkingInitCallLeg() rv=%d:%s)",
            pSubsOriginal, hMsg, rv, SipCommonStatus2Str(rv)));
        SubsFree(pSubs);
        return rv;
    }

    /* Update new Forked Subscription with information from the Original
    Subscription. Bring the Forked Subscription to '2XX received' state. */
    rv = SubsForkedSubsInit(pSubs, pSubsOriginal);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "ForkedSubsCreate - hSubsOrig=0x%p,hMsg=0x%p - failed SubsForkedSubsInit(pSubs=0x%p) failed (rv=%d:%s)",
            pSubsOriginal, hMsg, pSubs, rv, SipCommonStatus2Str(rv)));
        SubsFree(pSubs);
        return rv;
    }

    *ppSubs = pSubs;

    return RV_OK;
}

/******************************************************************************
 * PrepareCallbackMasks
 * ----------------------------------------------------------------------------
 * General: The function prepare the masks enabling us to block the RvSipSubsTerminate()
 *          API, if we are in a middle of a callback, that do 
 *          not allow terminate inside.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg      - Pointer to the Call-Leg object.
 ******************************************************************************/
static void RVCALLCONV PrepareCallbackMasks(IN SubsMgr* pMgr)
{
    pMgr->terminateMask = 0;
    
    /* turn on all the callbacks that should block the termination */
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_CREATED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_NOTIFY_CREATED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_CREATED_DUE_TO_FORKING, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_NEW_CONN_IN_USE, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_SIGCOMP_NOT_RESPONDED, 1);
    BITS_SetBit((void*)(&pMgr->terminateMask), CB_SUBS_REFER_NOTIFY_READY, 1);
}

#endif /* #ifdef RV_SIP_SUBS_ON - static functions */

#endif /*#ifndef RV_SIP_PRIMITIVES */

