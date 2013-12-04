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
 *                              <CallLegFroking.c>
 *
 * This file defines the callLeg functions related to forking issue.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 March 2004
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifndef RV_SIP_PRIMITIVES
#include "CallLegForking.h"
#include "CallLegObject.h"
#include "CallLegMsgEv.h"
#include "CallLegTranscEv.h"
#include "CallLegRouteList.h"
#include "CallLegCallbacks.h"
#include "CallLegInvite.h"

#include "_SipMsg.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipOtherHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipContactHeader.h"
#include "_SipTransmitterMgr.h"
#include "_SipTransmitter.h"
#include "RvSipTransmitter.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV CreateAndInitForkedCallLeg(
                                  IN  CallLegMgr* pMgr,
                                  IN  CallLeg*    pOrigCallLeg,
                                  IN  RvSipMsgHandle hMsg,
                                  OUT CallLeg**   ppNewForkedCall,
								  OUT RvBool*	  bNotifyApp);

static RvStatus CallLegSetForked1xxTimer(IN CallLeg* pCallLeg);

static RvStatus InitCallLegFromMsg( IN CallLeg         *pCallLeg,
                                    IN RvSipMsgHandle  hMsg,
                                    IN RvBool          bSwitchToAndFromTags);

static RvStatus InitCallLegFromOriginalCallLeg( IN CallLeg* pNewCallLeg,
                                                IN CallLeg* pOriginalCallLeg);

RvBool Handle1xxTimerTimeout(IN void *pContext,
                             IN RvTimer *timerInfo);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegForkingCreateForkedCallLegFromMsg
 * ------------------------------------------------------------------------
 * General: Creates a forked call-leg from 1xx/2xx response message that has no call-leg.
 *          1. search for the original call-leg.
 *          2. create a new forked created call-leg.
 *          Note that in the end of this function the new forked call-leg is locked!
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLegMgr -  Handle to the call-leg manager.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingCreateForkedCallLegFromMsg(
                                     IN CallLegMgr*      pMgr,
                                     IN RvSipMsgHandle   hMsg,
                                     OUT CallLeg**       ppCallLeg,
                                     OUT RvInt32*        pCurIdentifier,
									 OUT RvBool*		 pbNotifyApp)
{
    RvStatus rv = RV_OK;
    CallLeg* pForkedCallLeg = NULL;
    CallLeg* pOriginalCallLeg = NULL;
    SipTransactionKey   msgKey;
    RvBool   bForkingEnabled;

    *ppCallLeg = NULL;
    *pCurIdentifier = UNDEFINED;

	*pbNotifyApp = RV_FALSE;
    
    memset(&msgKey,0,sizeof(SipTransactionKey));
    rv = SipTransactionMgrGetKeyFromMessage(pMgr->hTranscMgr,hMsg,&msgKey);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegForkingCreateForkedCallLegFromMsg(hMsg=0x%p): SipTransactionMgrGetKeyFromMessage() failed (rv=%d:%s)",
            hMsg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    /* --------- At this point the Call-Leg Manager is locked ---------------*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    /* find original call-leg, and then create new forked call-leg object. */
    CallLegMgrHashFindOriginal(pMgr, &msgKey, RVSIP_CALL_LEG_DIRECTION_OUTGOING,
                               (RvSipCallLegHandle**)&pOriginalCallLeg);
    
    if(pOriginalCallLeg == NULL)
    {
        /* original call-leg was not found - ignore the response message */
        RvLogWarning(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - did not find original call. ignore message!!!",
            pMgr));
        /* ----------- Unlock the Call-Leg Manager -----------*/
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return RV_OK;
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - found original call 0x%p",
            pMgr, pOriginalCallLeg));
    }
    /* ----------- Unlock the Call-Leg Manager -----------*/
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    rv = CallLegLockAPI(pOriginalCallLeg);
    if(RV_OK != rv)
    {
        bForkingEnabled = RV_FALSE;
        RvLogError(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - Failed to lock original call 0x%p.",
            pMgr, pOriginalCallLeg));
        return RV_OK;
    }
    bForkingEnabled = pOriginalCallLeg->bForkingEnabled;
	CallLegUnLockAPI(pOriginalCallLeg);
   
    if(bForkingEnabled == RV_FALSE)
    {
        /* The original call-leg does not support forking - the response message is ignored */
        RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - found original call 0x%p with bForkingEnabled=%d. ignore message ",
        pMgr, pOriginalCallLeg, bForkingEnabled));
        return RV_OK;

    }

    RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - found original call 0x%p.",
        pMgr, pOriginalCallLeg));


    rv = CreateAndInitForkedCallLeg(pMgr, pOriginalCallLeg, hMsg, &pForkedCallLeg, pbNotifyApp);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegForkingCreateForkedCallLegFromMsg: mgr 0x%p - failed to create new forked call (rv=%d).",
            pMgr, rv));
        return RV_OK;
    }

    *ppCallLeg = pForkedCallLeg;
    *pCurIdentifier = pForkedCallLeg->callLegUniqueIdentifier;
    return RV_OK;
}


/***************************************************************************
 * CallLegForkingHandle1xxMsg
 * ------------------------------------------------------------------------
 * General: This function handles forked - 1xx response.
 *          Calling this function causes the following actions to happen:
 *          1. RvSipCallLegMsgRcvdEv is called on the given call-leg.
 *          2. RvSipCallLegChangeStateEv is called if needed, change state
 *             from IDLE to PROCEEDING - if this is the first 1xx on this call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingHandle1xxMsg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    IN RvSipMsgHandle   hResponseMsg)
{
    RvStatus    rv;
    RvBool      bReliableResponse = RV_FALSE;
    RvUint32    rseqStep;

    /* 1xx message outside of a transaction is handled only for forked call-leg! */
    if(pCallLeg->bOriginalCall == RV_TRUE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegForkingHandle1xxMsg - call-leg 0x%p - illegal action for original call-leg",
            pCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    /* if this is a reliable 1xx message - check if we need to ignore it first.
       (usually this check is done from the transaction layer, before calling
        to the msg-rcvd-callback) */

    SipTransactionIs1xxLegalRel(hResponseMsg,&rseqStep,&bReliableResponse);
    if(RV_TRUE == bReliableResponse)
    {
        RvBool bIgnore;
        CallLegTranscEvIgnoreRelProvRespEvHandler(NULL, (RvSipTranscOwnerHandle)pCallLeg,
                                                  rseqStep, &bIgnore);
        if(RV_TRUE == bIgnore)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegForkingHandle1xxMsg - pCallLeg 0x%p - ignore rel 1xx with rseq %u. (last rseq is %u)",
                pCallLeg, rseqStep, pCallLeg->incomingRSeq.val));
            return RV_OK;
        }

    }

    rv = CallLegMsgForkedInviteResponseRcvdHandler(pCallLeg, hResponseMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegForkingHandle1xxMsg - pCallLeg 0x%p - failed in msg-rcvd-cb. rv=%d",
            pCallLeg, rv));
        return rv;
    }

    /* set the response message in call-leg received message */
    pCallLeg->hReceivedMsg = hResponseMsg;

    /* call the change state call back only for the first 1xx */
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING)
    {
        rv = HandleInviteProvisionalResponseRecvd(pCallLeg, pInvite);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegForkingHandle1xxMsg - pCallLeg 0x%p - failed to handle 1xx response. rv=%d",
                pCallLeg, rv));
            pCallLeg->hReceivedMsg = NULL;
            return rv;
        }
    }
    /* if the 1xx is reliable - call also the reliable handling function */
    if(bReliableResponse == RV_TRUE)
    {
        rv = HandleRelProvRespRcvd(pCallLeg, rseqStep, hResponseMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegForkingHandle1xxMsg - pCallLeg 0x%p - failed to handle the reliable 1xxresponse. rv=%d",
                pCallLeg, rv));
            pCallLeg->hReceivedMsg = NULL;
            return rv;
        }
    }

    /* set the forked 1xx timer */
    CallLegSetForked1xxTimer(pCallLeg);
    
    pCallLeg->hReceivedMsg = NULL;

    return RV_OK;
}

/***************************************************************************
 * CallLegForkingTerminateOnFailure
 * ------------------------------------------------------------------------
 * General: This function terminates a forked call-leg, in case it do not
 *          have a forked timer yet.
 *          The function is called when handling a forked 1xx fails, and so 
 *          the call remains with no timer...
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
void RVCALLCONV CallLegForkingTerminateOnFailure(CallLeg* pCallLeg)
{
    RvBool   bTimerStarted;
    if(pCallLeg->bOriginalCall == RV_TRUE)
    {
        return;
    }
    
    bTimerStarted = SipCommonCoreRvTimerStarted(&pCallLeg->forkingTimer);
    if(bTimerStarted == RV_TRUE)
    {
        /* the call-leg already has a timer --> this is not the first 1xx -->
           keep it alive */
        return;
    }

    CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
}

/***************************************************************************
 * CallLegForkingHandleFirst2xxMsg
 * ------------------------------------------------------------------------
 * General: This function handles first 2xx response, received in a forked call-leg.
 *          Calling this function causes the following actions to happen:
 *          1. RvSipCallLegMsgRcvdEv is called on the given call-leg.
 *          2. Handle the 2xx as a regular first 2xx from the transaction layer.
 *             (update state machine, create and send ACK request, etc.)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingHandleFirst2xxMsg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    IN RvSipMsgHandle   hResponseMsg)
{
    RvStatus   rv;
    
    rv = CallLegMsgForkedInviteResponseRcvdHandler(pCallLeg, hResponseMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegForkingHandleFirst2xxMsg - pCallLeg 0x%p - failed to handle 2xx. rv=%d",
            pCallLeg, rv));
        return rv;
    }

    /* set the response message in call-leg received message */
    pCallLeg->hReceivedMsg = hResponseMsg;

    /* release the forked 1xx timer */
    CallLegForkingTimerRelease(pCallLeg);

    /* Handle the 2xx response as if a transaction exists */
    rv = CallLegTranscEvHandleInviteFinalResponseRcvd(pCallLeg, pInvite, NULL, RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegForkingHandleFirst2xxMsg - pCallLeg 0x%p - failed to handle the response. rv=%d",
            pCallLeg, rv));
        pCallLeg->hReceivedMsg = NULL;
        return rv;
    }

    pCallLeg->hReceivedMsg = NULL;

    return RV_OK;
}


/*  T R A N S M I T T E R    E V E N T     H A N D L E R S  */
/***************************************************************************
 * CallLegForkingTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the call-Leg's froking 1xx timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-Leg to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegForkingTimerRelease(IN CallLeg *pCallLeg)
{
    if (NULL != pCallLeg &&
        SipCommonCoreRvTimerStarted(&pCallLeg->forkingTimer))
    {
        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pCallLeg->forkingTimer);
        RvLogDebug((pCallLeg->pMgr)->pLogSrc ,((pCallLeg->pMgr)->pLogSrc ,
                  "CallLegForkingTimerRelease - Call-Leg 0x%p: Timer 0x%p was released",
                  pCallLeg, &pCallLeg->forkingTimer));
    }
}

/***************************************************************************
* CallLegSetForked1xxTimer
* ------------------------------------------------------------------------
* General: sets the forked 1xx timer.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pContext - The call-Leg this timer was called for.
***************************************************************************/
static RvStatus CallLegSetForked1xxTimer(IN CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;
    RvBool   bTimerStarted;

    bTimerStarted = SipCommonCoreRvTimerStarted(&pCallLeg->forkingTimer);

    if( pCallLeg->forked1xxTimerTimeout > 0 &&
        bTimerStarted == RV_FALSE)
    {
        rv = SipCommonCoreRvTimerStartEx(&pCallLeg->forkingTimer,
                                       pCallLeg->pMgr->pSelect,
                                       pCallLeg->forked1xxTimerTimeout,
                                       Handle1xxTimerTimeout,
                                       (void*)pCallLeg);
        if (rv != RV_OK)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSetForked1xxTimer - Call-Leg  0x%p: No timer is available",
                pCallLeg));
            return rv;
        }
        else
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSetForked1xxTimer - Call-Leg  0x%p: 1xx timer was set to %u milliseconds",
                pCallLeg, pCallLeg->forked1xxTimerTimeout));
        }
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
        "CallLegSetForked1xxTimer - Call-Leg  0x%p: did not set timer. pCallLeg->forked1xxTimerTimeout=%d, bTimerStarted=%d ",
        pCallLeg, pCallLeg->forked1xxTimerTimeout, bTimerStarted));
    }
    return rv;

}


/***************************************************************************
* Handle1xxTimerTimeout
* ------------------------------------------------------------------------
* General: Called when ever a the forked 1xx timer expires.
*          On expiration we terminate the call-leg
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pContext - The call-Leg this timer was called for.
***************************************************************************/
RvBool Handle1xxTimerTimeout(IN void *pContext,
                             IN RvTimer *timerInfo)
{
    CallLeg* pCallLeg;

    pCallLeg = (CallLeg *)pContext;
    if (RV_OK != CallLegLockMsg(pCallLeg))
    {
        CallLegForkingTimerRelease(pCallLeg);
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pCallLeg->forkingTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
            "Handle1xxTimerTimeout - Call-Leg  0x%p: forking 1xx timer expired but was already released. do nothing...",
            pCallLeg));
        
         CallLegUnLockMsg(pCallLeg);
        return RV_FALSE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
        "Handle1xxTimerTimeout - Call-Leg  0x%p: forking 1xx timer was expired. terminate call-leg",
        pCallLeg));

    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pCallLeg->forkingTimer);
    CallLegForkingTimerRelease(pCallLeg);

    CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_FORKED_CALL_NO_FINAL_RESPONSE);
    CallLegUnLockMsg(pCallLeg);
    return RV_TRUE;
}

/******************************************************************************
 * CallLegForkingInitCallLeg
 * ----------------------------------------------------------------------------
 * General: The function must get a locked(!!!) original call-leg as an argument.
 *          1. Loads data from Original Call-Leg and from Message into forked
 *          Call-Leg.
 *          2. Set forked Call-Leg state to RVSIP_CALL_LEG_STATE_PROCEEDING /
 *          RVSIP_CALL_LEG_STATE_ACCEPTED.
 *          Dialog identifiers for the forked Call-Leg are taken from the Message.
 * Return Value:
 *          RV_OK on success; error code, defined in RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pMgr              - Pointer to Call-Leg Manager object.
 *        pCallLeg          - Pointer to the Forked Call-Leg object.
 *        pOriginalCallLeg  - Pointer to the Original Call-Leg object.
 *        hMsg              - Handle to Message, caused forked dialog creation.
 *        bSwitchToAndFromTags - If RV_TRUE, TO and FROM tags from the Message,
 *                            will be switched while being inserted into
 *                            Forked Call-Leg (Forked Call-Leg can be created
 *                            as a result of response arrival, or as a result
 *                            of new request arrival (e.g. NOTIFY).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegForkingInitCallLeg(
                                    IN  CallLegMgr      *pMgr,
                                    IN  CallLeg         *pCallLeg,
                                    IN  CallLeg         *pOriginalCallLeg,
                                    IN  RvSipMsgHandle  hMsg,
                                    IN  RvBool          bSwitchToAndFromTags)
{
    RvStatus    rv;

    if (NULL==pMgr || NULL==pCallLeg || NULL==pOriginalCallLeg ||
        NULL==hMsg)
    {
        return RV_ERROR_BADPARAM;
    }

    /* initialize the call-leg parameters from given response message */
    rv = InitCallLegFromMsg(pCallLeg, hMsg, bSwitchToAndFromTags);
    if(RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegForkingInitCallLeg - call-leg 0x%p - Failed to Init Call-Leg From Msg. hMsg=0x%p, bSwitchTags=%s (rv=%d:%s)",
            pCallLeg, hMsg, (RV_TRUE==bSwitchToAndFromTags)?"TRUE":"FALSE",
            rv,SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegForkingInitCallLeg - call-leg 0x%p - Init Call-Leg From Msg succeed.",
            pCallLeg));

    /* Set parameters from Original Call-Leg */
    rv = InitCallLegFromOriginalCallLeg(pCallLeg, pOriginalCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegForkingInitCallLeg - call-leg 0x%p - Failed to Init Call-Leg From Original CallLeg 0x%p (rv=%d:%s)",
            pCallLeg, pOriginalCallLeg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegForkingInitCallLeg - call-leg 0x%p - Init Call-Leg From original call-leg succeed.",
            pCallLeg));

    /* Get the Call-Leg to INVITING state */
    if (RV_FALSE == CallLegGetIsHidden(pCallLeg))
    {
        pCallLeg->eState = RVSIP_CALL_LEG_STATE_INVITING;
    }
    else
    {
        pCallLeg->eState = RVSIP_CALL_LEG_STATE_IDLE;
    }

    pCallLeg->bOriginalCall = RV_FALSE;

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CreateAndInitForkedCallLeg
 * ------------------------------------------------------------------------
 * General: This function creates a new forked call-leg in
 *          state RVSIP_CALL_LEG_STATE_INVITING.
 *          The call-leg parameters - To, From, Call-Id, CSeq, and remote
 *          contact will be set by the message headers, and the original
 *          call-leg parameters.
 * Return Value: RV_OutOfResources - Call list is full, no call-leg created.
 *               RV_OK - a new call-leg was created and initialized
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr             - The manager of the new call-leg
 *          pOriginalCallLeg - The original call-leg that sent the original
 *                         INVITE request.
 *          hMsg             - The received response message.
 * Output:  ppNewForkedCall  - Pointer to a newly created callLeg.
 ***************************************************************************/
static RvStatus RVCALLCONV CreateAndInitForkedCallLeg(IN  CallLegMgr* pMgr,
                                                      IN  CallLeg*    pOrigCallLeg,
                                                      IN  RvSipMsgHandle hMsg,
                                                      OUT CallLeg**   ppNewForkedCall,
													  OUT RvBool*	  pbNotifyApp)
{
    RvStatus rv = RV_OK;
    CallLeg* pNewForkedCallLeg = NULL;
    CallLeg* pHashForkedCallLeg = NULL;
    SipTransactionKey   msgKey;

    *ppNewForkedCall = NULL;

	*pbNotifyApp = RV_FALSE;

    /* 1. Lock the original call-leg */
    rv = CallLegLockAPI(pOrigCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CreateAndInitForkedCallLeg: failed to lock the original call 0x%p (rv=%d).",
            pOrigCallLeg, rv));
        return rv;
    }

    if(pOrigCallLeg->bForkingEnabled == RV_FALSE)
    {
       /* if original call-leg does not support forking, we should not create a new
          object in here */
        RvLogError(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CreateAndInitForkedCallLeg: mgr 0x%p, original call 0x%p does not support forking",
            pMgr, pOrigCallLeg));

        CallLegUnLockAPI(pOrigCallLeg);
        return RV_ERROR_UNKNOWN;
    }

    /* 2. Create the new forked Call-Leg object -
          The function does not insert the new call to the hash .*/
    rv = CallLegMgrCreateCallLeg(pMgr,
                                RVSIP_CALL_LEG_DIRECTION_OUTGOING,
                                RV_FALSE /*bIsHidden*/,
                                (RvSipCallLegHandle*)&pNewForkedCallLeg);
    if(RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateAndInitForkedCallLeg - Failed to create new Call-Leg object(rv=%d:%s)",
            rv,SipCommonStatus2Str(rv)));

        CallLegUnLockAPI(pOrigCallLeg);
        return rv;
    }

    /* no need to lock the new forked call-leg, because no one knows about it. */

    /* 3. Init new Call-Leg with information from the Message and from the original call-leg*/
    rv = CallLegForkingInitCallLeg(pMgr,
                                   pNewForkedCallLeg,
                                   pOrigCallLeg,
                                   hMsg,
                                   RV_FALSE /*bSwitchToAndFromTags*/);
    if(RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateAndInitForkedCallLeg - call-leg 0x%p - Failed to Init Call-Leg From Msg 0x%p(rv=%d:%s)",
            pNewForkedCallLeg, hMsg, rv, SipCommonStatus2Str(rv)));

        CallLegUnLockAPI(pOrigCallLeg);
        CallLegDestruct(pNewForkedCallLeg);
        return rv;
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    pNewForkedCallLeg->cctContext = pOrigCallLeg->cctContext;
    pNewForkedCallLeg->URI_Cfg_Flag = pOrigCallLeg->URI_Cfg_Flag;
#endif
       //SPIRENT_END

    /* 4. Unlock the original call-leg */
    CallLegUnLockAPI(pOrigCallLeg);

    /* Checks if another thread created a same call-leg object in the hash.
       If so, the new object should be destructed, and the object
       in the hash should be used.
       Else, we should insert the new call-leg to the hash. */

    /* 5. Lock the Call-leg manager */
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    memset(&msgKey,0,sizeof(SipTransactionKey));

    rv = SipTransactionMgrGetKeyFromMessage(pMgr->hTranscMgr,hMsg,&msgKey);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CreateAndInitForkedCallLeg: Call-leg 0x%p - failed to Get Key From Message 0x%p (rv=%d:%s)",
            pNewForkedCallLeg, hMsg, rv, SipCommonStatus2Str(rv)));
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        CallLegDestruct(pNewForkedCallLeg);
        return rv;
    }

    /* 5. Find Same new Call-leg in the hash */
    CallLegMgrHashFind(pMgr,
                        &msgKey,
                        RVSIP_CALL_LEG_DIRECTION_OUTGOING,
                        RV_FALSE /*bOnlyEstablishedCall*/,
                        (RvSipCallLegHandle**)&pHashForkedCallLeg);
    if(pHashForkedCallLeg == NULL)
    {
        /* no same call-leg in the hash. insert the new call-leg to the hash */
        rv = CallLegMgrHashInsert((RvSipCallLegHandle)pNewForkedCallLeg);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CreateAndInitForkedCallLeg: Call-leg 0x%p - failed to insert call-leg to the hash (rv=%d:%s)",
                pNewForkedCallLeg, rv, SipCommonStatus2Str(rv)));
            RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
            CallLegDestruct(pNewForkedCallLeg);
            return rv;
        }
		/*A new forked call-leg was created so we need to indicate that the application sould get notification*/
		*pbNotifyApp = RV_TRUE;
        *ppNewForkedCall = pNewForkedCallLeg;
    }
    else
    {
        /* a same call-leg already exist in the hash */
        CallLegDestruct(pNewForkedCallLeg);
        *ppNewForkedCall = pHashForkedCallLeg;
    }

    /* 6. Lock the Call-leg manager */
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    
    return RV_OK;
}



/******************************************************************************
 * InitCallLegFromMsg
 * ----------------------------------------------------------------------------
 * General: Loads dialog identifiers (To, From, Call-Id, CSeq, and remote
 *          contact) from a Message into a Call-Leg.
 * Return Value: RV_OK on success,
 *               Error code, defined in RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Pointer to the Call-Leg.
 *          hMsg     - The Message.
 *          bSwitchToAndFromTags - RV_TRUE, if the FROM and TO tag from Message
 *                     should be set in opposite direction into the Call-Leg.
 * Output:  none.
 *****************************************************************************/
static RvStatus InitCallLegFromMsg( IN  CallLeg         *pCallLeg,
                                    IN  RvSipMsgHandle  hMsg,
                                    IN  RvBool          bSwitchToAndFromTags)
{
    RvSipPartyHeaderHandle   hToMsgHeader, hFromMsgHeader;
    RvSipPartyHeaderHandle  *phTo, *phFrom;
    RvSipOtherHeaderHandle   hCallId;
	RvSipContactHeaderHandle hContact = NULL;
	RvSipAddressHandle		 hContactAddr;
    RPOOL_Ptr                callIdVal;
    RvSipCSeqHeaderHandle    hCSeq;
    RvStatus                 rv;
	RvInt32					 cseqStep;

    /* get to, from, call-Id, CSeq and remote Contact headers from message*/
    hToMsgHeader = RvSipMsgGetToHeader(hMsg);
    if(NULL == hToMsgHeader)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - RvSipMsgGetToHeader(0x%p) failed", pCallLeg, hMsg));
        return RV_ERROR_UNKNOWN;
    }
    hFromMsgHeader = RvSipMsgGetFromHeader(hMsg);
    if(NULL == hFromMsgHeader)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - RvSipMsgGetFromHeader(0x%p) failed", pCallLeg, hMsg));
        return RV_ERROR_UNKNOWN;
    }
    hCallId = SipMsgGetCallIDHeaderHandle(hMsg);
    if(NULL == hCallId)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - SipMsgGetCallIDHeaderHandle(0x%p) failed", pCallLeg, hMsg));
        return RV_ERROR_UNKNOWN;
    }
    hCSeq = RvSipMsgGetCSeqHeader(hMsg);
    if(NULL == hCSeq)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - RvSipMsgGetCSeqHeader(0x%p) failed", pCallLeg, hMsg));
        return RV_ERROR_UNKNOWN;
    }
    
    if (RV_TRUE == bSwitchToAndFromTags)
    {
        phFrom = &hToMsgHeader;
        phTo   = &hFromMsgHeader;
    }
    else
    {
        phTo   = &hToMsgHeader;
        phFrom = &hFromMsgHeader;
    }

	/* The Contact header is optional in the message */ 
	hContact = RvSipMsgGetHeaderByType(
		hMsg,RVSIP_HEADERTYPE_CONTACT,RVSIP_FIRST_HEADER,(RvSipHeaderListElemHandle*)&hContact);
	
    /* set TO, FROM, Call-Id, CSeq headers in Call-Leg */
    rv = CallLegSetToHeader(pCallLeg, *phTo);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - CallLegSetToHeader(0x%p) failed", pCallLeg, *phTo));
        return rv;
    }
    rv = CallLegSetFromHeader(pCallLeg, *phFrom);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - CallLegSetFromHeader(0x%p) failed", pCallLeg, *phFrom));
        return rv;
    }

    callIdVal.hPage = SipOtherHeaderGetPage(hCallId);
    callIdVal.hPool = SipOtherHeaderGetPool(hCallId);
    callIdVal.offset = SipOtherHeaderGetValue(hCallId);
    rv = CallLegSetCallId(pCallLeg, &callIdVal);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - CallLegSetCallId() failed", pCallLeg));
        return rv;
    }

	rv = SipCSeqHeaderGetInitializedCSeq(hCSeq,&cseqStep); 
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - SipCSeqHeaderGetInitializedCSeq() failed (CSeq might be uninitialized)", pCallLeg));
        return rv;
	}
	rv = SipCommonCSeqSetStep(cseqStep,&pCallLeg->localCSeq); 
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InitCallLegFromMsg: Call-Leg 0x%p - SipCommonCSeqSetStep() failed", pCallLeg));
        return rv;
	}

    /* Hidden CallLeg doesn't deal with INVITEs */
    if (RV_FALSE == pCallLeg->bIsHidden)
    {
        CallLegInvite* pInvite;
        rv = CallLegInviteCreateObj(pCallLeg, NULL, RV_FALSE, 
            RVSIP_CALL_LEG_DIRECTION_OUTGOING, &pInvite, NULL);
        if(RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "InitCallLegFromMsg: Call-Leg 0x%p - failed to create invite object.", pCallLeg));
            return rv;
        }
        SipCommonCSeqCopy(&pCallLeg->localCSeq,&pInvite->cseq); 
    }

	/* Set the remote Contact (if exists) */ 
	if (hContact != NULL)
	{
		hContactAddr = RvSipContactHeaderGetAddrSpec(hContact); 
		if (hContactAddr == NULL)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"InitCallLegFromMsg: Call-Leg 0x%p - RvSipContactHeaderGetAddrSpec() returned NULL - impossible case", pCallLeg));
			return RV_ERROR_UNKNOWN;
		}
		rv = CallLegSetRemoteContactAddress(pCallLeg,hContactAddr);
		if (rv != RV_OK)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"InitCallLegFromMsg: Call-Leg 0x%p - CallLegSetRemoteContactAddress() failed", pCallLeg));
			return rv; 
		}
	}
		
    return RV_OK;
}

/***************************************************************************
 * InitCallLegFromOriginalCallLeg
 * ------------------------------------------------------------------------
 * General: save the original call-leg parameters. in case this call-leg will
 *          get final reject response, it should set this original call-leg
 *          to be not-original, and itself to original.
 * Return Value: RV_OutOfResources - Call list is full, no call-leg created.
 *               RV_OK - a new call-leg was created and initialized
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pNewCallLeg -      Pointer to a newly created callLeg
 *          hOriginalCallLeg - The original call-leg that sent the original
 *                             INVITE request.
 ***************************************************************************/
static RvStatus InitCallLegFromOriginalCallLeg(
                                  IN    CallLeg*       pNewCallLeg,
                                  IN    CallLeg*       pOriginalCallLeg)
{
    RvStatus rv;

    /* save the original call-leg parameters. in case this call-leg will get final
    reject response, it should set this original call-leg to be not-original,
    and itself to original */
    pNewCallLeg->hOriginalForkedCall = (RvSipCallLegHandle)pOriginalCallLeg;
    pNewCallLeg->originalForkedCallUniqueIdentifier = pOriginalCallLeg->callLegUniqueIdentifier;

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    pNewCallLeg->cctContext = pOriginalCallLeg->cctContext;
    pNewCallLeg->URI_Cfg_Flag = pOriginalCallLeg->URI_Cfg_Flag;
#endif
//SPIRENT_END

    /* outbound proxy */
    SipTransportInitObjectOutboundAddr(pNewCallLeg->pMgr->hTransportMgr,&pNewCallLeg->outboundAddr);
    memcpy(&(pNewCallLeg->outboundAddr),&(pOriginalCallLeg->outboundAddr), sizeof(pNewCallLeg->outboundAddr));

    if(UNDEFINED != pOriginalCallLeg->outboundAddr.rpoolHostName.offset)
    {
        RvInt32 hostNameLen = 0;
        RvInt32 strHostName;

        pNewCallLeg->outboundAddr.rpoolHostName.hPool = pNewCallLeg->pMgr->hGeneralPool;
        pNewCallLeg->outboundAddr.rpoolHostName.hPage = pNewCallLeg->hPage;

        /* add 1 to the strlen - for NULL termination */
        hostNameLen = 1+ RPOOL_Strlen(pOriginalCallLeg->outboundAddr.rpoolHostName.hPool,
                                  pOriginalCallLeg->outboundAddr.rpoolHostName.hPage,
                                  pOriginalCallLeg->outboundAddr.rpoolHostName.offset);
        rv = RPOOL_Append(pNewCallLeg->outboundAddr.rpoolHostName.hPool,
                          pNewCallLeg->outboundAddr.rpoolHostName.hPage,
                          hostNameLen, RV_FALSE, &strHostName);
        if(RV_OK != rv)
        {
            RvLogError(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
                "InitCallLegFromOriginalCallLeg - call-leg 0x%p. failed to allocate outboundAddr host name (rv=%d).",
                pOriginalCallLeg, rv));
            return rv;
        }
        rv = RPOOL_CopyFrom(pNewCallLeg->outboundAddr.rpoolHostName.hPool,
                          pNewCallLeg->outboundAddr.rpoolHostName.hPage,
                          strHostName,
                          pOriginalCallLeg->outboundAddr.rpoolHostName.hPool,
                          pOriginalCallLeg->outboundAddr.rpoolHostName.hPage,
                          pOriginalCallLeg->outboundAddr.rpoolHostName.offset,
                          hostNameLen);
        if(RV_OK != rv)
        {
            RvLogError(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
                "InitCallLegFromOriginalCallLeg - call-leg 0x%p. failed to copy outboundAddr host name (rv=%d).",
                pOriginalCallLeg, rv));
            return rv;
        }

        pNewCallLeg->outboundAddr.rpoolHostName.offset = strHostName;
#ifdef SIP_DEBUG
        pNewCallLeg->outboundAddr.strHostName = RPOOL_GetPtr(pNewCallLeg->outboundAddr.rpoolHostName.hPool,
                                                          pNewCallLeg->outboundAddr.rpoolHostName.hPage,
                                                          pNewCallLeg->outboundAddr.rpoolHostName.offset);
#endif
    }
    /* local contact */
    if(pOriginalCallLeg->hLocalContact != NULL)
    {
        rv = CallLegSetLocalContactAddress(pNewCallLeg, pOriginalCallLeg->hLocalContact);
        if(RV_OK != rv)
        {
            RvLogError(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
                "InitCallLegFromOriginalCallLeg - Failed to set local contact in new call-leg 0x%p. rv=%d",
                pNewCallLeg, rv));
            return rv;
        }

        RvLogDebug(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
            "InitCallLegFromOriginalCallLeg - call-leg 0x%p - copied hLocalContact from original call-leg.",
            pNewCallLeg));
    }
/* MULTSUBS - new code */
    if(pOriginalCallLeg->hRemoteContact != NULL)
    {
        rv = CallLegSetRemoteContactAddress(pNewCallLeg, pOriginalCallLeg->hRemoteContact);
        if(RV_OK != rv)
        {
            RvLogError(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
                "InitCallLegFromOriginalCallLeg - Failed to set remote contact in new call-leg 0x%p. rv=%d",
                pNewCallLeg, rv));
            return rv;
        }

        RvLogDebug(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
            "InitCallLegFromOriginalCallLeg - call-leg 0x%p - copied hRemoteContact from original call-leg.",
            pNewCallLeg));
    }

    /* session timer parameters */
    if(pOriginalCallLeg->pSessionTimer != NULL)
    {
        pNewCallLeg->pSessionTimer->alertTime         = pOriginalCallLeg->pSessionTimer->alertTime;
        pNewCallLeg->pSessionTimer->defaultAlertTime  = pOriginalCallLeg->pSessionTimer->defaultAlertTime;
        pNewCallLeg->pSessionTimer->eCurrentRefresher = pOriginalCallLeg->pSessionTimer->eCurrentRefresher;
        pNewCallLeg->pSessionTimer->eNegotiationReason = pOriginalCallLeg->pSessionTimer->eNegotiationReason;
        pNewCallLeg->pSessionTimer->eRefresherType    = pOriginalCallLeg->pSessionTimer->eRefresherType;
        pNewCallLeg->pSessionTimer->minSE             = pOriginalCallLeg->pSessionTimer->minSE;
        pNewCallLeg->pSessionTimer->bAddOnceMinSE     = pOriginalCallLeg->pSessionTimer->bAddOnceMinSE;
        pNewCallLeg->pSessionTimer->bAddReqMinSE      = pOriginalCallLeg->pSessionTimer->bAddReqMinSE;
        pNewCallLeg->pSessionTimer->sessionExpires    = pOriginalCallLeg->pSessionTimer->sessionExpires;
    }
    if(pOriginalCallLeg->pNegotiationSessionTimer != NULL)
    {
        pNewCallLeg->pNegotiationSessionTimer->bInRefresh     = pOriginalCallLeg->pNegotiationSessionTimer->bInRefresh;
        pNewCallLeg->pNegotiationSessionTimer->eRefresherPreference = pOriginalCallLeg->pNegotiationSessionTimer->eRefresherPreference;
        pNewCallLeg->pNegotiationSessionTimer->eRefresherType = pOriginalCallLeg->pNegotiationSessionTimer->eRefresherType;
        pNewCallLeg->pNegotiationSessionTimer->minSE          = pOriginalCallLeg->pNegotiationSessionTimer->minSE;
        pNewCallLeg->pNegotiationSessionTimer->sessionExpires = pOriginalCallLeg->pNegotiationSessionTimer->sessionExpires;
    }

    /* SigComp parameters */
#ifdef RV_SIGCOMP_ON
    pNewCallLeg->bSigCompEnabled = pOriginalCallLeg->bSigCompEnabled;
#endif /*#ifdef RV_SIGCOMP_ON*/

#ifdef RV_SIP_AUTH_ON
    /* authorization call-leg information -
    if initial INVITE was sent with authorization header, all call-legs must have
    the same header, to use in general request for example. The authorization
    headers are built on message send callback, based on the authentication
    header. Therefore copy the authentication headers here. */
    if (NULL != pOriginalCallLeg->hListAuthObj)
    {
        rv = SipAuthenticatorConstructAndCopyAuthObjList(
                            pNewCallLeg->pMgr->hAuthMgr,
                            pNewCallLeg->hPage,
                            pOriginalCallLeg->hListAuthObj,
                            pOriginalCallLeg->pAuthListInfo,
                            pNewCallLeg,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
                            (ObjLockFunc)CallLegLockAPI, (ObjUnLockFunc)CallLegUnLockAPI,
#else
                            NULL, NULL,
#endif
                            &pNewCallLeg->pAuthListInfo,
                            &pNewCallLeg->hListAuthObj);
        if(RV_OK != rv)
        {
            RvLogError(pNewCallLeg->pMgr->pLogSrc,(pNewCallLeg->pMgr->pLogSrc,
                "InitCallLegFromOriginalCallLeg - Failed to copy 0x%x authentication list into the new call-leg 0x%p (rv=%d:%s)",
                pOriginalCallLeg->hListAuthObj, pNewCallLeg, rv,
                SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    pNewCallLeg->bForceOutboundAddr = pOriginalCallLeg->bForceOutboundAddr;
#ifdef RV_SIP_SUBS_ON
    pNewCallLeg->hReferSubsGenerator = pOriginalCallLeg->hReferSubsGenerator;
#endif /*#ifdef RV_SIP_SUBS_ON*/
    return RV_OK;
}

#endif /*RV_SIP_PRIMITIVES */
