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
 *                              <SubsMgrObject.c>
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
#ifdef RV_SIP_SUBS_ON

#include "SubsMgrObject.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "SubsObject.h"
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SubsMgrSubscriptionCreate
 * ------------------------------------------------------------------------
 * General: Creates a new subscription (regular subscriber subscription, or
 *          out of bandsubscription - subscriber/notifier):
 *          1. create subscription object.
 *          2. create a hidden call-leg, if needed.
 *          3. initaite subscription and set it in call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr      - Pointer to the subs manager.
 *          hCallLeg  - Handle to a connected call-leg, or NULL - to create a
 *                      hidden call-leg.
 *          hAppsubs   - Handle to the application subs object.
 *            eSubsType  - Subscriber or Notifier.
 *          bOutOfBand - a regular subscription, or out of band subscription.
 * Output:     phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
RvStatus SubsMgrSubscriptionCreate( IN    RvSipSubsMgrHandle    hMgr,
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
    RvStatus          rv;
    RvSipCallLegHandle hSubsCallLeg;
    RvSipCallLegDirection eDirection;
    SubsMgr* pMgr = (SubsMgr*)hMgr;

    /* 1. create a hidden call-leg (if needed) */
    if(hCallLeg == NULL)
    {
        /* create a new hidden cal-leg, and create the subscription in it */
        if(eSubsType == RVSIP_SUBS_TYPE_NOTIFIER)
        {
            eDirection = RVSIP_CALL_LEG_DIRECTION_INCOMING;
        }
        else
        {
            eDirection = RVSIP_CALL_LEG_DIRECTION_OUTGOING;
        }
        rv = SipCallLegMgrCreateCallLeg(pMgr->hCallLegMgr, eDirection, RV_TRUE, &hSubsCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        ,cctContext,URI_cfg
#endif
//SPIRENT_END
          );
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SubsMgrSubscriptionCreate: failed to create hidden call-leg"));
            return rv;
        }
    }
    else
    {
        hSubsCallLeg = hCallLeg;
    }

    rv = SubsMgrSubscriptionObjCreate(pMgr, hSubsCallLeg, hAppSubs, eSubsType, bOutOfBand, phSubs);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionCreate: failed to create a new subscription"));

        if (hCallLeg == NULL) /*hidden call-leg */
        {
            RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SubsMgrSubscriptionCreate: destruct hidden call-leg 0x%p because of subsCreate failure",
                hSubsCallLeg));
            SipCallLegDestruct(hSubsCallLeg);
        }
        return rv;

    }

    /* Enable forking */
    SipCallLegSetSubsForkingEnabledFlag(hSubsCallLeg, RV_TRUE/*bForkingEnabled*/);

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SubsMgrSubscriptionCreate: New subscription 0x%p was created successfully. related call-leg is 0x%p",
        *phSubs, hSubsCallLeg));

    return RV_OK;

}

/***************************************************************************
 * SubsMgrSubscriptionObjCreate
 * ------------------------------------------------------------------------
 * General: Internal function. Creates a new subscription object.
 *          The call-leg was already created before calling to this function.
 *          1. create subscription in subs manager subscriptions list.
 *          2. initialize the subscription structure, and set triple lock in it.
 *          3. set the subscription in the call-leg subscriptions list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr       - pointer to the subs manager object
 *          hCallLeg   - Handle to the subscription call-leg (already exists!).
 *          hAppSubs   - Handle to the application subscription
 *          eSubsType  - Subscription type (notifier or subscriber)
 *          bOutOfBand - Is the subscription out-of-band or not.
 * Output:  phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
RvStatus RVCALLCONV SubsMgrSubscriptionObjCreate(
                                        IN    SubsMgr*              pMgr,
                                        IN    RvSipCallLegHandle    hCallLeg,
                                        IN    RvSipAppSubsHandle    hAppSubs,
                                        IN    RvSipSubscriptionType eSubsType,
                                        IN    RvBool               bOutOfBand,
                                        OUT   RvSipSubsHandle      *phSubs)
{
    RvStatus           rv;
    Subscription       *pSubscription;
    RLIST_ITEM_HANDLE  listItem;
    HPAGE              hSubsPage;

    /* 1. create a subscription object */

    /* lock subsMgr when creating a new subscription */
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/
    rv = RLIST_InsertTail(pMgr->hSubsListPool,pMgr->hSubsList,&listItem);
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/

    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        *phSubs = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionObjCreate - Failed to insert new subscription to list (rv=%d)", rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pSubscription = (Subscription*)listItem;

    rv = RPOOL_GetPage(pMgr->hGeneralPool,0,&hSubsPage);
    if(rv != RV_OK)
    {
        pSubscription->hPage = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionObjCreate - Failed to get page for new subscription"));
        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,listItem);
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* 2. Initialize the subscription object */

    rv = SubsInitialize(pSubscription, hAppSubs, hCallLeg, pMgr, eSubsType, bOutOfBand, hSubsPage);
    if(rv != RV_OK)
    {
        RPOOL_FreePage(pMgr->hGeneralPool,hSubsPage);
        *phSubs = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionObjCreate - Failed to initialize new subscription (rv=%d)", rv));
        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,listItem);
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* set subscription's lock to point to call-leg's lock */
    RvMutexLock(&pSubscription->subsLock,pSubscription->pMgr->pLogMgr);
    pSubscription->tripleLock = SipCallLegGetMutexHandle(hCallLeg);
    RvMutexUnlock(&pSubscription->subsLock,pSubscription->pMgr->pLogMgr);

    /* for a not-hidden call-leg, we have to lock the subscription and call-leg now.
    we will lock it here anyway */
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        RPOOL_FreePage(pMgr->hGeneralPool,hSubsPage);
        RLIST_ListDestruct(pMgr->hNotifyListPool,pSubscription->hNotifyList);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionObjCreate: failed to lock subs 0x%p. subscription destruct",
            pSubscription));
        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,listItem);
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return rv;
    }

    /* 3. Bind the subscription object to the dialog */

    /* set subscription in call-leg */
    rv = SipCallLegSubsAddSubscription(hCallLeg, (RvSipSubsHandle)pSubscription,
                                        &(pSubscription->hItemInCallLegList));
    if(rv != RV_OK)
    {
        RPOOL_FreePage(pMgr->hGeneralPool,hSubsPage);
        RLIST_ListDestruct(pMgr->hNotifyListPool,pSubscription->hNotifyList);
        *phSubs = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsMgrSubscriptionObjCreate - Failed to add subscription to call-leg(rv=%d)", rv));

        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
        RLIST_Remove(pMgr->hSubsListPool,pMgr->hSubsList,listItem);
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        SubsUnLockAPI(pSubscription);
        return rv;
    }

    *phSubs = (RvSipSubsHandle)pSubscription;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SubsMgrSubscriptionObjCreate: New subscription 0x%p was created successfully. related call-leg is 0x%p",
        pSubscription, hCallLeg));

    SubsUnLockAPI(pSubscription);
    return RV_OK;
}

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES  */
