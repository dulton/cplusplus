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
 *                              <SubsCallbacks.c>
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

#include "SubsObject.h"
#include "_SipCallLeg.h"
#include "SubsRefer.h"

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

/*-----------------------------------------------------------------------*/
/*                            SUBS CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackSubsCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnSubsCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the subs manager.
*         hSubs       - The new sip stack subs handle
* Output:(-)
***************************************************************************/
void RVCALLCONV SubsCallbackSubsCreatedEv(IN  SubsMgr             *pMgr,
                                          IN  Subscription*        pSubs)
{
    RvInt32                 nestedLock = 0;
    RvSipCallLegHandle      hCallLeg = NULL;
    RvSipAppCallLegHandle   hAppCallLeg = NULL;
    RvSipAppSubsHandle      hAppSubs = NULL;
    RvSipSubsEvHandlers    *subsEvHandlers = pSubs->subsEvHandlers;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pSubs->tripleLock;

    if (NULL != pSubs->hCallLeg &&
        RV_FALSE == SipCallLegIsHiddenCallLeg(pSubs->hCallLeg))
    {
        /* get the subscription's call-leg handles */
        hAppCallLeg = SipCallLegGetAppHandle(pSubs->hCallLeg);
		hCallLeg = pSubs->hCallLeg;
    }
    

    if(pMgr->bDisableRefer3515Behavior == RV_TRUE &&
       pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsCreatedEv((RvSipSubsHandle)pSubs, hCallLeg, hAppCallLeg, pSubs->hActiveTransc, &hAppSubs);
        pSubs->hAppSubs = hAppSubs;
        return;
    }

    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnSubsCreatedEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackSubsCreatedEv: subs 0x%p - Before callback", pSubs));

        OBJ_ENTER_CB(pSubs, CB_SUBS_CREATED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        (*subsEvHandlers).pfnSubsCreatedEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hCallLeg,
                                                hAppCallLeg,
                                                &hAppSubs);
        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_CREATED);
        pSubs->hAppSubs = hAppSubs;
        
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackSubsCreatedEv: subs 0x%p - After callback. hAppSubs 0x%p", pSubs, hAppSubs));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
           "SubsCallbackSubsCreatedEv - hSubs 0x%p - Application did not registered to callback.",
            pSubs));
    }
}

/***************************************************************************
 * SubsCallbackChangeSubsStateEv
 * ------------------------------------------------------------------------
 * General: calls to pfnStateChangedEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackChangeSubsStateEv( IN  Subscription              *pSubs,
                                             IN  RvSipSubsState             eState,
                                             IN  RvSipSubsStateChangeReason eReason)
{
    RvInt32                 nestedLock     = 0;
    RvSipSubsEvHandlers    *subsEvHandlers = pSubs->subsEvHandlers;
    RvSipAppSubsHandle      hAppSubs       = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsStateChangedEv((RvSipSubsHandle)pSubs,hAppSubs, eState, eReason);
        return;
    }

    pTripleLock = pSubs->tripleLock;

    /*notify the application about the new state*/
    if(subsEvHandlers != NULL && (*subsEvHandlers).pfnStateChangedEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackChangeSubsStateEv: subs 0x%p - Before callback",pSubs));

        OBJ_ENTER_CB(pSubs, CB_SUBS_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*subsEvHandlers).pfnStateChangedEvHandler(
                                   (RvSipSubsHandle)pSubs,
                                   hAppSubs,
                                   eState,
                                   eReason);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_STATE_CHANGED);
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackChangeSubsStateEv: subs 0x%p - After callback", pSubs));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackChangeSubsStateEv - hSubs 0x%p - Application did not registered to callback.",
            pSubs));
    }
}

/***************************************************************************
 * SubsCallbackExpirationAlertEv
 * ------------------------------------------------------------------------
 * General: calls to pfnExpirationAlertEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackExpirationAlertEv( IN  Subscription   *pSubs)
{
    RvInt32                 nestedLock = 0;
    RvSipSubsEvHandlers    *subsEvHandlers = pSubs->subsEvHandlers;
    RvSipAppSubsHandle      hAppSubs   = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
       pSubs->pReferInfo != NULL)
    {
         RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationAlertEv: subs 0x%p - refer+bDisableRefer3515Behavior=true. nothing to do",pSubs));
        return;
    }

    pTripleLock = pSubs->tripleLock;

    /*notify the application about the new state*/
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnExpirationAlertEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationAlertEv: subs 0x%p - Before callback",pSubs));
        OBJ_ENTER_CB(pSubs, CB_SUBS_EXPIRES_ALERT);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(subsEvHandlers)).pfnExpirationAlertEvHandler(
                                        (RvSipSubsHandle)pSubs,
                                        hAppSubs);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_EXPIRES_ALERT);
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationAlertEv: subs 0x%p - After callback",pSubs));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackExpirationAlertEv - hSubs 0x%p - Application did not registered to callback.",
            pSubs));
    }
}

/***************************************************************************
 * SubsCallbackExpirationEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSubsExpiredEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackExpirationEv( IN  Subscription   *pSubs)
{
    RvInt32                 nestedLock = 0;
    RvSipSubsEvHandlers    *subsEvHandlers = pSubs->subsEvHandlers;
    RvSipAppSubsHandle      hAppSubs   = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
       pSubs->pReferInfo != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationEv: subs 0x%p - refer+bDisableRefer3515Behavior=true. terminating subscription...",
            pSubs));
        SubsTerminate(pSubs, RVSIP_SUBS_REASON_SUBS_TERMINATED);
        return;
    }

    pTripleLock = pSubs->tripleLock;

    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnSubsExpiredEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationEv: subs 0x%p - Before callback",pSubs));

        OBJ_ENTER_CB(pSubs, CB_SUBS_EXPIRATION);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(subsEvHandlers)).pfnSubsExpiredEvHandler(
                                            (RvSipSubsHandle)pSubs,
                                            hAppSubs);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
         OBJ_LEAVE_CB(pSubs, CB_SUBS_EXPIRATION);
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackExpirationEv: subs 0x%p - After callback",
            pSubs));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackExpirationEv - hSubs 0x%p - Application did not registered to callback.",
            pSubs));
    }
}
/*-----------------------------------------------------------------------*/
/*                            NOTIFY CALLBACKS                           */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsCallbackNotifyTranscCreatedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNotifyCreatedEvHandler
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr    - Pointer to the subs manager
 *            pSubs   - Pointer to the subscription object.
 *          pNotify - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsCallbackNotifyTranscCreatedEv(
                                         IN    SubsMgr*             pMgr,
                                         IN    Subscription*        pSubs,
                                         IN    Notification*        pNotify)
{
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs = pSubs->hAppSubs;
    RvSipAppNotifyHandle    hAppNotify = NULL;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
	

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsNotifyCreatedEv((RvSipSubsHandle)pSubs,
                                            hAppSubs,
                                            (RvSipNotifyHandle)pNotify,
                                            &hAppNotify);
        return RV_OK;
    }

    pTripleLock = pSubs->tripleLock;


    /*At this point we have a new incoming notification object
    - inform user of it */
    if(pSubs->subsEvHandlers->pfnNotifyCreatedEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackNotifyTranscCreatedEv: subs 0x%p notify 0x%p - Before callback", pSubs, pNotify));

        OBJ_ENTER_CB(pSubs, CB_SUBS_NOTIFY_CREATED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        pSubs->subsEvHandlers->pfnNotifyCreatedEvHandler(
                                            (RvSipSubsHandle)pSubs,
                                            hAppSubs,
                                            (RvSipNotifyHandle)pNotify,
                                            &hAppNotify);
        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_NOTIFY_CREATED);
        
        pNotify->hAppNotify = hAppNotify;
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackNotifyTranscCreatedEv: subs 0x%p notify 0x%p - After callback. hAppNotify 0x%p",
            pSubs, pNotify, hAppNotify));        
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
           "SubsCallbackNotifyTranscCreatedEv - hSubs 0x%p - Application did not registered to callback. hNotify 0x%p",
            pSubs, pNotify));
    }
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif

    return RV_OK;
}

/******************************************************************************
 * SubsCallbackSubsCreatedDueToForkingEv
 * ----------------------------------------------------------------------------
 * General: calls to pfnNotifyForMultipleSubsEvHandler
 *
 * Return Value: RV_OK on success, error code, defined in RV_SIP_DEF.h or
 *               rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs       - Pointer to the Subscription Object.
 * Output:  pExpires    - Pointer to the memory, where Application can set
 *                      expiration value.
 *          phAppSubs   - Pointer to the memory, where Application can store
 *                      any data of sizeof(RvSipAppSubsHandle) bytes.
 *          pRejectStatus-If Application decides to terminate this Subscription
 *                      it should set the pointer to point to positive integer,
 *                      representing Error Code.
 *                      In this case Stack will respond to the forked NOTIFY
 *                      with error code and will free Subscription object and
 *                      all the Subscription related resources.
 *****************************************************************************/
RvStatus RVCALLCONV SubsCallbackSubsCreatedDueToForkingEv(
                                    IN      Subscription*       pSubs,
                                    IN OUT  RvInt32             *pExpires,
                                    OUT     RvSipAppSubsHandle  *phAppSubs,
                                    OUT     RvUint16            *pRejectStatus)
{
    RvInt32        nestedLock = 0;
    SubsMgr*       pMgr       = pSubs->pMgr;
    SipTripleLock* pTripleLock;
    RvThreadId     currThreadId; RvInt32 threadIdLockCount;
    RvStatus       rv;

    pTripleLock    = pSubs->tripleLock;

    if(pMgr->subsEvHandlers.pfnSubsCreatedDueToForkingEvHandler != NULL)
    {
        *pRejectStatus = 0;
        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsCallbackSubsCreatedDueToForkingEv: subs 0x%p, expires=%d - Before callback",
            pSubs,*pExpires));

        OBJ_ENTER_CB(pSubs, CB_SUBS_CREATED_DUE_TO_FORKING);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = pMgr->subsEvHandlers.pfnSubsCreatedDueToForkingEvHandler(
                   (RvSipSubsHandle)pSubs, pExpires, phAppSubs, pRejectStatus);
        
        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_CREATED_DUE_TO_FORKING);
        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsCallbackSubsCreatedDueToForkingEv: subs 0x%p, expires=%d, hAppSubs=0x%p, reject status=%d. After callback(rv=%d)",
            pSubs,*pExpires,*phAppSubs,*pRejectStatus,rv));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsCallbackSubsCreatedDueToForkingEv: subs 0x%p, - Application didn't registered to callback.",pSubs));
    }
    return RV_OK;
}

/***************************************************************************
 * SubsCallbackChangeNotifyStateEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNotifyEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification.
 *            eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received notify msg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackChangeNotifyStateEv(IN  Notification          *pNotify,
                                               IN  RvSipSubsNotifyStatus eNotifyStatus,
                                               IN  RvSipSubsNotifyReason eNotifyReason,
                                               IN  RvSipMsgHandle        hNotifyMsg)
{
    RvSipAppNotifyHandle    hAppNotify = pNotify->hAppNotify;
    RvSipAppSubsHandle      hAppSubs   = pNotify->pSubs->hAppSubs;
    RvSipSubsHandle         hSubs      = (RvSipSubsHandle)pNotify->pSubs;
    RvSipSubsNotifyEv      *pfnNotifyEvHandler = pNotify->pfnNotifyEvHandler;
    RvInt32                 nestedLock = 0;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pNotify->pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
       pNotify->pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsNotifyEv(hSubs,
                                 hAppSubs,
                                 (RvSipNotifyHandle)pNotify,
                                 hAppNotify,
                                 eNotifyStatus,
                                 eNotifyReason,
                                 hNotifyMsg);
        return;
    }

    pTripleLock = pNotify->pSubs->tripleLock;

    if(pfnNotifyEvHandler != NULL && (*(pfnNotifyEvHandler)) != NULL)
    {
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsCallbackChangeNotifyStateEv: subs 0x%p notify 0x%p - Before callback",hSubs, pNotify));

        OBJ_ENTER_CB(pNotify->pSubs, CB_SUBS_NOTIFY_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pNotify->pSubs->pMgr->pLogMgr,pNotify->pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(pfnNotifyEvHandler))( hSubs,
                                 hAppSubs,
                                 (RvSipNotifyHandle)pNotify,
                                 hAppNotify,
                                 eNotifyStatus,
                                 eNotifyReason,
                                 hNotifyMsg);

        SipCommonLockAfterCallback(pNotify->pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pNotify->pSubs, CB_SUBS_NOTIFY_STATE_CHANGED);
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsCallbackChangeNotifyStateEv: subs 0x%p notify 0x%p- After callback",
            hSubs, pNotify));
    }
    else
    {
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
           "SubsCallbackChangeNotifyStateEv - hSubs 0x%p - Application did not registered to callback. hNotify 0x%p",
            hSubs,pNotify));
    }
}

/*-----------------------------------------------------------------------*/
/*                            MESSAGE CALLBACKS                          */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackMsgRcvdEv
* ------------------------------------------------------------------------
* General: calls to pfnMsgReceivedEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - Pointer to the subscription object.
*          hNotify - handle to the notify object.
*          hAppNotify - handle to the notify app object.
*          hMsg - msg received.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackMsgRcvdEv(
                            IN  Subscription*        pSubs,
                            IN  RvSipNotifyHandle    hNotify,
                            IN  RvSipAppNotifyHandle hAppNotify,
                            IN  RvSipMsgHandle       hMsg)
{
    RvStatus                rv = RV_OK;
    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsMsgReceivedEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hMsg);
        return rv;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnMsgReceivedEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsCallbackMsgRcvdEv: subs 0x%p notify 0x%p - Before callback",pSubs, hNotify));

        OBJ_ENTER_CB(pSubs, CB_SUBS_MSG_RCVD);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*(subsEvHandlers)).pfnMsgReceivedEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hMsg);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_MSG_RCVD);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgRcvdEv: subs 0x%p notify 0x%p - After callback",
            pSubs, hNotify));

        if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                 "SubsCallbackMsgRcvdEv - subs 0x%p was terminated in cb. return failure", pSubs));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgRcvdEv: subs 0x%p notify 0x%p  - callback was not implemented",
            pSubs, hNotify));
    }

    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgRcvdEv: subs 0x%p notify 0x%p  - returned from callback with rv %d",
            pSubs, hNotify, rv));
    }
    return rv;
}

/***************************************************************************
* SubsCallbackMsgToSendEv
* ------------------------------------------------------------------------
* General: calls to pfnMsgToSendEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - Pointer to the subscription object.
*          hNotify - handle to the notify object.
*          hAppNotify - handle to the notify app object.
*          hMsg - msg to send.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackMsgToSendEv(
                            IN  Subscription*        pSubs,
                            IN  RvSipNotifyHandle    hNotify,
                            IN  RvSipAppNotifyHandle hAppNotify,
                            IN  RvSipMsgHandle       hMsg)
{
    RvStatus              rv = RV_OK;
    RvSipSubsEvHandlers  *subsEvHandlers;
    RvInt32               nestedLock = 0;
    RvSipAppSubsHandle    hAppSubs   = pSubs->hAppSubs;
    SipTripleLock        *pTripleLock;
    RvThreadId            currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsMsgToSendEv((RvSipSubsHandle)pSubs,
                                        hAppSubs,
                                        hNotify,
                                        hAppNotify,
                                        hMsg);
        return rv;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnMsgToSendEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsCallbackMsgToSendEv: subs 0x%p notify 0x%p  - Before callback",pSubs, hNotify));

        OBJ_ENTER_CB(pSubs, CB_SUBS_MSG_TO_SEND);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*(subsEvHandlers)).pfnMsgToSendEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hMsg);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_MSG_TO_SEND);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgToSendEv: subs 0x%p notify 0x%p  - After callback", pSubs, hNotify));

        if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                 "SubsCallbackMsgToSendEv - subs 0x%p was terminated in cb. return failure", pSubs));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgToSendEv: subs 0x%p notify 0x%p  - callback was not implemented", pSubs, hNotify));
    }

    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackMsgToSendEv: subs 0x%p notify 0x%p  - returned from callback with rv %d", pSubs, hNotify, rv));
    }
    return rv;
}

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                            AUTH CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackAuthCredentialsFoundEv
* ------------------------------------------------------------------------
* General: Calls to pfnAuthCredentialsFoundEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          hNotify      - handle to the notify object.
*          hAuthorization - Authorization header with credentials.
*          bCredentialsSupported - Are the credentials supported by the SIP stack or not.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackAuthCredentialsFoundEv(
                              IN  Subscription*                   pSubs,
                              IN  RvSipNotifyHandle               hNotify,
                              IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                              IN  RvBool                         bCredentialsSupported)
{

    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsAuthCredentialsFoundEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAuthorization,
                                                bCredentialsSupported);
        return RV_OK;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnAuthCredentialsFoundEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsCallbackAuthCredentialsFoundEv: subs 0x%p notify 0x%p  - Before callback",pSubs, hNotify));

        OBJ_ENTER_CB(pSubs, CB_SUBS_AUTH_CREDENTIALS_FOUND);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(subsEvHandlers)).pfnAuthCredentialsFoundEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAuthorization,
                                                bCredentialsSupported);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_AUTH_CREDENTIALS_FOUND);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackAuthCredentialsFoundEv: subs 0x%p notify 0x%p  - After callback", pSubs, hNotify));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackAuthCredentialsFoundEv: subs 0x%p notify 0x%p  - callback was not implemented", pSubs, hNotify));
    }

    return RV_OK;
}

/***************************************************************************
* SubsCallbackAuthCompletedEv
* ------------------------------------------------------------------------
* General: calls to pfnAuthCompletedEvHandler
* Return Value: RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          hNotify      - handle to the notify object.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                         RV_FALSE if we did not.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackAuthCompletedEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvBool              bAuthSucceed)
{
    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsAuthCompletedEv((RvSipSubsHandle)pSubs,
                                            hAppSubs,
                                            hNotify,
                                            bAuthSucceed);
        return RV_OK;
    }
    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnAuthCompletedEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                 "SubsCallbackAuthCompletedEv: subs 0x%p notify 0x%p  - Before callback",pSubs, hNotify));

        OBJ_ENTER_CB(pSubs, CB_SUBS_AUTH_COMPLETED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(subsEvHandlers)).pfnAuthCompletedEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                bAuthSucceed);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_AUTH_COMPLETED);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackAuthCompletedEv: subs 0x%p notify 0x%p  - After callback", pSubs, hNotify));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackAuthCompletedEv: subs 0x%p notify 0x%p  - callback was not implemented", pSubs, hNotify));
    }
    return RV_OK;
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
* SubsCallbackOtherURLAddressFoundEv
* ------------------------------------------------------------------------
* General: calls to pfnOtherURLAddressFoundEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs          - Pointer to the subscription object.
*           hNotify        - handle to the notify object.
*           hMsg           - The message that includes the other URL address.
*            hAddress       - Handle to unsupport address to be converted.
* Output:    hSipURLAddress - Handle to the known SIP URL address.
*            bAddressResolved-Indication of a successful/failed address
*                             resolving.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackOtherURLAddressFoundEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipMsgHandle       hMsg,
                              IN  RvSipAddressHandle   hAddress,
                              OUT RvSipAddressHandle   hSipURLAddress,
                              OUT RvBool               *bAddressResolved)
{
    RvStatus             rv            = RV_OK;
    RvSipSubsEvHandlers *subsEvHandlers;
    RvInt32              nestedLock = 0;
    RvSipAppSubsHandle   hAppSubs   = pSubs->hAppSubs;
    SipTripleLock       *pTripleLock;
    RvThreadId           currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsOtherURLAddressFoundEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hMsg,
                                                hAddress,
                                                hSipURLAddress,
                                                bAddressResolved);
        return rv;
    }
    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;

    if(subsEvHandlers && NULL != (*subsEvHandlers).pfnOtherURLAddressFoundEvHandler)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsCallbackOtherURLAddressFoundEv: subs 0x%p notify 0x%p - Before callback",pSubs, hNotify));
        
        OBJ_ENTER_CB(pSubs, CB_SUBS_OTHER_URL_FOUND);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*subsEvHandlers).pfnOtherURLAddressFoundEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hMsg,
                                                hAddress,
                                                hSipURLAddress,
                                                bAddressResolved);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_OTHER_URL_FOUND);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackOtherURLAddressFoundEv: subs 0x%p notify 0x%p- After callback",
            pSubs, hNotify));
        
        if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                 "SubsCallbackOtherURLAddressFoundEv - subs 0x%p was terminated in cb. return failure", pSubs));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        /* Indicate that the other URL address wasn't resolved */
        *bAddressResolved = RV_FALSE;
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackOtherURLAddressFoundEv - hSubs 0x%p - Application did not registered to callback. hNotify 0x%p",
            pSubs,hNotify));
    }

    return rv;
}


/***************************************************************************
* SubsCallbackFinalDestResolvedEv
* ------------------------------------------------------------------------
* General: calls to pfnFinalDestResolvedEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs          - Pointer to the subscription object.
*           hNotify        - handle to the notify object.
*           hAppNotify     - the notify application handle.
*           hTrasnc        - the transaction handle
*           hMsg           - The message that includes the other URL address.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackFinalDestResolvedEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipTranscHandle    hTransc,
                              IN  RvSipMsgHandle       hMsg)
{
    RvStatus                rv            = RV_OK;
    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsFinalDestResolvedEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hTransc,
                                                hMsg);
        return rv;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;

    if(NULL != subsEvHandlers &&
       NULL != (*subsEvHandlers).pfnFinalDestResolvedEvHandler)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsCallbackFinalDestResolvedEv: subs=0x%p, notify=0x%p, appNotify=0x%p, hTransc=0x%p, hMsg=0x%p- Before callback",
             pSubs, hNotify,hAppNotify,hTransc,hMsg));
        
        OBJ_ENTER_CB(pSubs, CB_SUBS_FINAL_DEST_RESOLVED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*subsEvHandlers).pfnFinalDestResolvedEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hTransc,
                                                hMsg);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_FINAL_DEST_RESOLVED);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackFinalDestResolvedEv: subs 0x%p notify 0x%p- After callback (rv=%d)",
            pSubs, hNotify,rv));

        if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                 "SubsCallbackFinalDestResolvedEv - subs 0x%p was terminated in cb. return failure", pSubs));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        /* Indicate that the other URL address wasn't resolved */
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackFinalDestResolvedEv - subs=0x%p, notify=0x%p, appNotify=0x%p, hTransc=0x%p, hMsg=0x%p - Application did not registered to callback.",
            pSubs,hNotify,hAppNotify,hTransc,hMsg));
    }
    return rv;
}

/***************************************************************************
 * SubsCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pSubs          - Pointer to the subscription object.
 *           hNotify        - handle to the notify object.
 *           hAppNotify     - the notify application handle.
 *           hConn          - The connection handle
 *           bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV SubsCallbackNewConnInUseEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipTransportConnectionHandle hConn,
                              IN  RvBool               bNewConnCreated)
{
    RvStatus                rv            = RV_OK;
    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs   = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsNewConnInUseEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hConn,
                                                bNewConnCreated);
        return rv;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;

    if(NULL != subsEvHandlers &&
       NULL != (*subsEvHandlers).pfnNewConnInUseEvHandler)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsCallbackNewConnInUseEv: subs=0x%p, notify=0x%p, appNotify=0x%p, hConn=0x%p - Before callback",
             pSubs, hNotify,hAppNotify,hConn));
     
        OBJ_ENTER_CB(pSubs, CB_SUBS_NEW_CONN_IN_USE);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*subsEvHandlers).pfnNewConnInUseEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hConn,
                                                bNewConnCreated);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_NEW_CONN_IN_USE);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackNewConnInUseEv: subs 0x%p notify 0x%p, appNotify=0x%p, hConn=0x%p - After callback (rv=%d)",
            pSubs, hNotify, hAppNotify,hConn, rv));
        
    }
    else
    {
        /* Indicate that the other URL address wasn't resolved */
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackNewConnInUseEv - subs=0x%p, notify=0x%p, appNotify=0x%p, hConn=0x%p - Application did not registered to callback.",
            pSubs,hNotify,hAppNotify,hConn));
    }
    return rv;
}


#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SubsCallbackSigCompMsgNotRespondedEv
* ------------------------------------------------------------------------
* General: calls to pfnSigCompMsgNotRespondedEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs          - Pointer to the subscription object.
*           hNotify        - handle to the notify object.
*           hAppNotify     - the notify application handle.
*           hTrasnc        - the transaction handle
*           hAppTransc     - The transaction application handle.
*           retransNo      - The number of retransmissions of the request
*                            until now.
*           ePrevMsgComp   - The type of the previous not responded request
* Output:   peNextMsgComp  - The type of the next request to retransmit (
*                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
*                            application wishes to stop sending requests).
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackSigCompMsgNotRespondedEv(
                              IN  Subscription*                pSubs,
                              IN  RvSipNotifyHandle            hNotify,
                              IN  RvSipAppNotifyHandle         hAppNotify,
                              IN  RvSipTranscHandle            hTransc,
                              IN  RvSipAppTranscHandle         hAppTransc,
                              IN  RvInt32                      retransNo,
                              IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                              OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    RvStatus                rv            = RV_OK;
    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs   = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        rv = SipCallLegReferSubsSigCompMsgNotRespondedEv(
                                                     (RvSipSubsHandle)pSubs,
                                                     hAppSubs,
                                                     hNotify,
                                                     hAppNotify,
                                                     hTransc,
                                                     hAppTransc,
                                                     retransNo,
                                                     ePrevMsgComp,
                                                     peNextMsgComp);
        return rv;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;

    if(NULL != subsEvHandlers &&
       NULL != (*subsEvHandlers).pfnSigCompMsgNotRespondedEvHandler)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsCallbackSigCompMsgNotRespondedEv: subs=0x%p, notify=0x%p, hTransc=0x%p, retrans=%d, prevMsg=%s- Before callback",
             pSubs, hNotify,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
        
        OBJ_ENTER_CB(pSubs, CB_SUBS_SIGCOMP_NOT_RESPONDED);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*subsEvHandlers).pfnSigCompMsgNotRespondedEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                hNotify,
                                                hAppNotify,
                                                hTransc,
                                                retransNo,
                                                ePrevMsgComp,
                                                peNextMsgComp);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_SIGCOMP_NOT_RESPONDED);
        
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackSigCompMsgNotRespondedEv: subs 0x%p notify 0x%p- After callback (rv=%d)",
            pSubs, hNotify, rv));
    }
    else
    {
        /* Indicate that the other URL address wasn't resolved */
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsCallbackSigCompMsgNotRespondedEv: subs=0x%p, notify=0x%p, hTransc=0x%p, retrans=%d, prevMsg=%s - Application did not registered to callback.",
            pSubs,hNotify,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */
/*-----------------------------------------------------------------------*/
/*                           REFER CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackReferNotifyReadyEv
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - what caused the decision, that a NOTIFY request
*                         should be sent
*          responseCode - .
*          hResponseMsg - .
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackReferNotifyReadyEv(
                                   IN  Subscription*                    pSubs,
                                   IN  RvSipSubsReferNotifyReadyReason  eReason,
                                   IN  RvInt16                         responseCode,
                                   IN  RvSipMsgHandle                   hResponseMsg)
{

    RvSipSubsEvHandlers    *subsEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppSubsHandle      hAppSubs   = pSubs->hAppSubs;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsCallbackReferNotifyReadyEv: subs 0x%p - ready to send notify. reason =%s, responseCode=%d",
        pSubs, SubsReferGetNotifyReadyReasonName(eReason), responseCode));

    if(pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE &&
        pSubs->pReferInfo != NULL)
    {
        SipCallLegReferSubsReferNotifyReadyEv((RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                eReason,
                                                responseCode,
                                                hResponseMsg);
        return RV_OK;
    }

    pTripleLock = pSubs->tripleLock;

    subsEvHandlers = pSubs->subsEvHandlers;
    if(subsEvHandlers != NULL && (*(subsEvHandlers)).pfnReferNotifyReadyEvHandler != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsCallbackReferNotifyReadyEv: subs 0x%p - Before callback", pSubs));

        OBJ_ENTER_CB(pSubs, CB_SUBS_REFER_NOTIFY_READY);
        SipCommonUnlockBeforeCallback(pSubs->pMgr->pLogMgr,pSubs->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(subsEvHandlers)).pfnReferNotifyReadyEvHandler(
                                                (RvSipSubsHandle)pSubs,
                                                hAppSubs,
                                                eReason,
                                                responseCode,
                                                hResponseMsg);

        SipCommonLockAfterCallback(pSubs->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pSubs, CB_SUBS_REFER_NOTIFY_READY);

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackReferNotifyReadyEv: subs 0x%p - After callback", 
            pSubs));
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsCallbackReferNotifyReadyEv: subs 0x%p - callback was not implemented", pSubs));
    }

    return RV_OK;
}

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES*/

