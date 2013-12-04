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
 *                              <SubsObject.c>
 *
 * This file contains implementation for the subscription object.
 * Creating a subscription, handling messages related to it, annd so on.
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
#include "_SipCommonConversions.h"

#include "SubsObject.h"
#include "SubsNotify.h"
#include "SubsCallbacks.h"
#include "SubsRefer.h"

#include "RvSipCallLeg.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "RvSipEventHeader.h"
#include "_SipEventHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "RvSipContactHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipMsg.h"
#include "_SipMsg.h"
#include "RvSipTransaction.h"
#include "_SipTransport.h"
#include "RvSipCommonList.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"

#include "AdsCopybits.h"
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define SUBS_STATE_IDLE_STR                     "Idle"
#define SUBS_STATE_SUBS_SENT_STR                "Subs Sent"
#define SUBS_STATE_REDIRECTED_STR               "Redirected"
#define SUBS_STATE_UNAUTHENTICATED_STR          "Unauthenticated"
#define SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD_STR   "Notify Before 2xx Rcvd"
#define SUBS_STATE_2XX_RCVD_STR                 "2xx Rcvd"
#define SUBS_STATE_REFRESHING_STR               "Refreshing"
#define SUBS_STATE_REFRESH_RCVD_STR             "Refresh Rcvd"
#define SUBS_STATE_UNSUBSCRIBING_STR            "Unsubscribing"
#define SUBS_STATE_UNSUBSCRIBE_RCVD_STR         "Unsubscribe Rcvd"
#define SUBS_STATE_UNSUBSCRIBE_2XX_RCVD_STR     "Unsubscribe 2xx Rcvd"
#define SUBS_STATE_SUBS_RCVD_STR                "Subs Rcvd"
#define SUBS_STATE_ACTIVATED_STR                "Subs Activated"
#define SUBS_STATE_TERMINATING_STR              "Subs Terminating"
#define SUBS_STATE_PENDING_STR                  "Subs Pending"
#define SUBS_STATE_ACTIVE_STR                   "Subs Active"
#define SUBS_STATE_TERMINATED_STR               "Subs Terminated"
#define SUBS_STATE_MSG_SEND_FAILURE_STR         "Subs Msg Send Failure"
#define SUBS_STATE_UNDEFINED_STR                "Undefined"

#define SUBS_TYPE_SUBSCRIBER_STR  "Subscriber"
#define SUBS_TYPE_NOTIFIER_STR    "Notifier"
#define SUBS_TYPE_UNDEFINED_STR   "Undefined"

#define SUBS_OBJECT_NAME_STR "Subs"

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void RVCALLCONV SubsTerminateAllTransc(IN  Subscription*   pSubs,
                                              IN  RvBool          bOnlyFree);


static RvStatus RVCALLCONV SubsPrepareSubscribeTransc (IN  Subscription *pSubs,
                                                       IN  RvInt32       expirationVal,
                                                       IN  RvBool        bRefer,
													   IN  RvBool        bCreateNewTransc);

static RvStatus RVCALLCONV SubsSetSubsParamsInMsg ( IN  Subscription      *pSubs,
                                                     IN  RvSipMsgHandle    hMsg,
                                                     IN  RvInt32          expirationVal,
                                                     IN  RvBool           bRequest);

static RvStatus RVCALLCONV SetAlertTimerFrom2xxResponse(Subscription* pSubs,
                                                        RvSipMsgHandle hMsg);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus RVCALLCONV SubsLock(IN Subscription        *pSubs,
                                     IN SubsMgr            *pMgr,
                                     IN SipTripleLock        *tripleLock,
                                     IN    RvInt32            identifier);

static void RVCALLCONV SubsUnLock(IN  IN RvLogMgr        *pLogMgr,
                                  IN  RvMutex           *hLock);

static RvStatus RVCALLCONV SubsTerminateUnlock(IN  RvLogMgr  *pLogMgr,
                                               IN  RvLogSource* pLogSrc,
                                               IN  void      *pSubsForPrint,
                                               IN  void      *tripleLockForPrint,
                                               IN  RvMutex   *hObjLock,
                                               IN  RvMutex   *hProcessingLock);

#else
#define SubsLock(a,b,c,d) (RV_OK)
#define SubsUnLock(a,b)
#define SubsTerminateUnlock(a,b,c,d,e,f) (RV_OK)

#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */



static RvStatus RVCALLCONV SaveEventFromMsgInSubs(Subscription* pSubs, RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV TerminateSubsEventHandler(IN void *obj, 
                                                     IN RvInt32  reason);

static RvStatus RVCALLCONV HandleInitialSubsRequest(IN Subscription*     pSubs,
                                                      IN RvSipTranscHandle hTransc,
                                                      IN RvSipMsgHandle    hMsg,
                                                      IN RvInt32          expires);

static RvStatus RVCALLCONV HandleRefreshSubsRequest(IN Subscription     *pSubs,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RvSipMsgHandle    hMsg,
                                                    IN RvInt32           expires);

static RvStatus RVCALLCONV HandleUnsubscribeRequest(Subscription     *pSubs,
                                                    RvSipTranscHandle hTransc,
                                                    RvSipMsgHandle    hMsg);

static void RVCALLCONV SubsResponseRcvdStateChange(
										IN Subscription*				pSubs,
										IN RvSipSubsState				eNewState,
										IN RvSipSubsStateChangeReason	eReason);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsSetInitialParams
 * ------------------------------------------------------------------------
 * General: Inits a subscription with mandatory parameters: To and From headers
 *          of the dialog, expiresVal of the subscription, and Event header of
 *            the subscription.
 *            This function initialized the subscription, but do not send any request.
 *          You should call RvSipSubsSubscribe() in order to send a Subscribe
 *          request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs - Handle to the subscription the user wishes to initialize.
 *          hFrom - Handle to a party header, contains the from header information.
 *          hTo -   Handle to a party header, contains the from header information.
 *          expiresVal - Expires value to be set in first SUBSCRIBE request.
 *                    (This is not necceseraly the final expires value. Notifier may
 *                    decide of a shorter expires value in the 2xx response.)
 *            hEvent - Handle to an Event header. this header identify the subscription.
 ***************************************************************************/
RvStatus SubsSetInitialParams(  IN  Subscription*          pSubs,
                                 IN  RvSipPartyHeaderHandle hFrom,
                                 IN  RvSipPartyHeaderHandle hTo,
                                 IN  RvInt32               expiresVal,
                                 IN  RvSipEventHeaderHandle hEvent)
{
    RvStatus  rv;
    RvBool    bIsHidden;

    bIsHidden = SipCallLegIsHiddenCallLeg(pSubs->hCallLeg);

     /*---------------------------------------------------------------
      if this is not a hidden call-leg, there may be several subscriptions
      in the call-leg, therefore we need to check here if there is no existing
      subscription with same event header.
    ----------------------------------------------------------------- */
    if(bIsHidden == RV_FALSE)
    {
        if(SubsFindSubsWithSameEventHeader(pSubs->hCallLeg, pSubs, hEvent) == RV_TRUE)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialParams - subscription 0x%p - subscription with same Event header already exists",
                pSubs));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    /* from + to */
    rv = SubsSetInitialDialogParams(pSubs, hFrom, hTo, NULL, NULL);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* expires */
    if(expiresVal > UNDEFINED)
    {
        pSubs->expirationVal = expiresVal;
        pSubs->requestedExpirationVal = expiresVal;
    }

    /* event */
    if(hEvent != NULL)
    {
        rv = SubsSetEventHeader(pSubs, hEvent);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialParams - subscription 0x%p - Failed to set Event header (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    pSubs->hActiveTransc = NULL;
    return RV_OK;
}


/***************************************************************************
 * SubsSetInitialDialogParams
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with dialog parameters: To, From, remote
 *          contact and local contact headers.
 *          This function is relevan only for subscription that was created
 *          outside of a call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs - Handle to the subscription the user wishes to initialize.
 *          hFrom - Handle to a party header, contains the from header information.
 *          hTo -   Handle to a party header, contains the from header information.
 *          hLocalRemote - Handle to address, contains the local contact
 *                  address information.
 *            hRemoteContact - Handle to address, contains the remote
 *                  contact address information.
 ***************************************************************************/
RvStatus SubsSetInitialDialogParams(IN  Subscription*            pSubs,
                                     IN  RvSipPartyHeaderHandle   hFrom,
                                     IN  RvSipPartyHeaderHandle   hTo,
                                     IN  RvSipAddressHandle       hLocalContactAddr,
                                     IN  RvSipAddressHandle       hRemoteContactAddr)
{
    RvStatus  rv;

    if(pSubs->bOutOfBand == RV_TRUE)
    {
        /* if this is an out of band subscription, the to and from headers must
           contain tags!!! */
        RvInt32           tagOffset = UNDEFINED;
        RvSipCallLegState eDialogState;

        rv = RvSipCallLegGetCurrentState(pSubs->hCallLeg, &eDialogState);
        if(eDialogState == RVSIP_CALL_LEG_STATE_IDLE)
        {
            /* out-of-band subscription with idle call-leg. verify that to-tag
            and from-tag are given */
            if(hFrom == NULL || hTo == NULL)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetInitialDialogParams - subscription 0x%p - out-of-band must get both to and from headers (hTo=0x%p hFrom=0x%p)",
                    pSubs, hTo, hFrom));
                return RV_ERROR_BADPARAM;
            }

            tagOffset = SipPartyHeaderGetTag(hFrom);
            if(tagOffset == UNDEFINED)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetInitialDialogParams - subscription 0x%p - out-of-band must get from header with a tag",
                    pSubs));
                return RV_ERROR_BADPARAM;
            }

            tagOffset = SipPartyHeaderGetTag(hTo);
            if(tagOffset == UNDEFINED)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetInitialDialogParams - subscription 0x%p - out-of-band must get to header with a tag",
                    pSubs));
                return RV_ERROR_BADPARAM;
            }
        }
        else
        {
            /* out-of-band subscription in a connected call-leg.
            we should not get a to and from headers here... */
            if(hFrom != NULL || hTo != NULL)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetInitialDialogParams - subscription 0x%p - out-of-band in a connected call-leg must not get to and from headers (hTo=0x%p hFrom=0x%p)",
                    pSubs, hTo, hFrom));
                return RV_ERROR_BADPARAM;
            }
        }
    }
    /* from */
    if(hFrom != NULL)
    {
        rv = RvSipCallLegSetFromHeader(pSubs->hCallLeg, hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialDialogParams - subscription 0x%p - Failed to set From header (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    /* to */
    if(hTo != NULL)
    {
        rv = RvSipCallLegSetToHeader(pSubs->hCallLeg, hTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialDialogParams - subscription 0x%p - Failed to set To header (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    /* local contact */
    if(hLocalContactAddr != NULL)
    {
        rv = RvSipCallLegSetLocalContactAddress(pSubs->hCallLeg, hLocalContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialDialogParams - subscription 0x%p - Failed to set local contact (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    /* remote contact */
    if(hRemoteContactAddr != NULL)
    {
        rv = RvSipCallLegSetRemoteContactAddress(pSubs->hCallLeg, hRemoteContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSetInitialDialogParams - subscription 0x%p - Failed to set remote contact (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    /* the following checking for a non out of band subscription are done
    in the Subscribe() functions */
    if(pSubs->bOutOfBand == RV_TRUE)
    {
        RvBool    bIsHidden = SipCallLegIsHiddenCallLeg(pSubs->hCallLeg);
        /*---------------------------------------------------------------
          if this is a hidden call-leg and out of band subscription -
          add the call-leg to the hash table. but first we must check that
          there is no other call-leg with the same identifiers in the hash.
          (this is because this call-leg is inserted with it's to-tag)
        ----------------------------------------------------------------- */
        if(bIsHidden == RV_TRUE)
        {
            rv = SipCallLegSubsInsertCallToHash(pSubs->hCallLeg, RV_TRUE);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                        "SubsSetInitialDialogParams - Subscription 0x%p - Failed to insert hidden call-leg 0x%p to hash table (status %d)",
                        pSubs, pSubs->hCallLeg, rv));
                return rv;
            }
        }
    }

    pSubs->hActiveTransc = NULL;
    return RV_OK;
}

/***************************************************************************
 * SubsSubscribe
 * ------------------------------------------------------------------------
 * General: Send SUBSCRIBE or REFER request.
 *          This function may be called only after subscription mandatory
 *          parameters were set. (To, From for subscription not inside a call-leg,
 *          Event for regular subscription, Refer-To for refer subscription).
 *          Calling Subscribe causes the request to be sent out and the
 *          subscription state machine progresses to the Subs_Sent state.
 *          If this fails, user should terminate the related subscription object.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Cannot subscribe due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send the subscribe message.
 *               RV_OK - subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs  - Pointer to the subscription the user wishes to subscribe.
 *          bRefer - TRUE for sending REFER. FALSE for sending SUBSCRIBE.
 ***************************************************************************/
RvStatus RVCALLCONV SubsSubscribe (IN  Subscription *pSubs,
                                    IN  RvBool      bRefer)
{
    RvStatus rv;


    rv = SubsSendRequest(pSubs, pSubs->expirationVal, bRefer, RV_TRUE);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsSubscribe - Subscription 0x%p - Failed to send request (status %d)",
            pSubs, rv));
        return rv;
    }

    /*------------------------------------------
         change subscription to SUBS_SENT state
     -------------------------------------------*/
    SubsChangeState(pSubs, RVSIP_SUBS_STATE_SUBS_SENT, RVSIP_SUBS_REASON_UNDEFINED);

    return RV_OK;
}


/***************************************************************************
 * SubsRefresh
 * ------------------------------------------------------------------------
 * General: Refresh a subscription. This method may be called
 *          only for an active subscription. Calling
 *          Refresh causes a SUBSCRIBE request to be sent out and the
 *          subscription state machine progresses to the Subs_Refreshing
 *          or Subs_Unsubscribing state.
 *          If this fails, user should terminate the related subscription object.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Cannot subscribe due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send the subscribe message.
 *               RV_OK - subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription the user wishes to subscribe.
 *          expirationVal - if 0, this is an unsubscribe request. else, refresh.
 ***************************************************************************/
RvStatus RVCALLCONV SubsRefresh (IN  Subscription *pSubs,
                                  IN  RvInt32      expirationVal)
{
    RvStatus rv;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsRefresh - Subscription 0x%p sending SUBSCRIBE...",pSubs));

    rv = SubsSendRequest(pSubs, expirationVal, RV_FALSE, RV_TRUE);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsRefresh - Subscription 0x%p - Failed to send subscribe request (status %d)",
            pSubs, rv));
        return rv;
    }

    /*-----------------------------------
        update subscription state machine
    ------------------------------------- */
    if(expirationVal == 0)
    {
        /* unsubscribe */
        pSubs->eStateBeforeUnsubsWasSent = pSubs->eState;
        SubsChangeState(pSubs,RVSIP_SUBS_STATE_UNSUBSCRIBING, RVSIP_SUBS_REASON_UNDEFINED);
    }
    else
    {
        SubsChangeState(pSubs,RVSIP_SUBS_STATE_REFRESHING,RVSIP_SUBS_REASON_UNDEFINED);
    }

    return RV_OK;
}


/***************************************************************************
 * SubsAccept
 * ------------------------------------------------------------------------
 * General: Accept a subscription (with 200 or 202).
 *          Response message is sent, and state updated to active or pending.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send the response message.
 *               RV_OK - response message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription the user wishes to accept.
 *          responseCode - 200 or 202.
 *          expirationVal - if 0, this is an unsubscribe request. else, refresh. in seconds.
 *                          of UNDEFINED, the expires value of the subscribe request, is
 *                          set in the 2xx response.
 *          bRefer      - TRUE if this is sending of 202 on refer request.
 ***************************************************************************/
RvStatus RVCALLCONV SubsAccept (IN  Subscription *pSubs,
                                 IN  RvUint16     responseCode,
                                 IN  RvInt32      expirationVal,
                                 IN  RvBool       bRefer)
{
    RvStatus      rv;
    RvSipMsgHandle hMsg;

    if(pSubs->hOutboundMsg == NULL)
    {
        rv = RvSipTransactionGetOutboundMsg(pSubs->hActiveTransc, &hMsg);
        if(rv != RV_OK || hMsg == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsAccept - Subscription 0x%p - failed to get message to send from transaction",pSubs));
            return rv;
        }
    }
    else
    {
        hMsg = pSubs->hOutboundMsg;
        rv = SipTransactionSetOutboundMsg(pSubs->hActiveTransc, hMsg);
        if(RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsAccept - Subscription 0x%p - failed to set outbound message in transaction",pSubs));
            return rv;
        }
        pSubs->hOutboundMsg = NULL;
    }

    if (RV_FALSE == bRefer)
    {
        /* set the expires headers only in responses to a SUBSCRIBE request */
        rv = SubsSetSubsParamsInMsg(pSubs, hMsg, expirationVal, RV_FALSE);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsAccept - Subscription 0x%p - Failed to set parmeters in 2xx response (status %d)",
                pSubs, rv));
            return rv;
        }

        /* set expiration value in subscription */
        if(expirationVal > UNDEFINED)
        {
            pSubs->expirationVal = expirationVal;
            pSubs->requestedExpirationVal = UNDEFINED;
        }
        else
        {
            pSubs->expirationVal = pSubs->requestedExpirationVal;
            pSubs->requestedExpirationVal = UNDEFINED;
        }
    }
    else
    {
        pSubs->expirationVal = UNDEFINED;
        pSubs->requestedExpirationVal = UNDEFINED;
    }
    /*----------------------------------
         send the 2xx response
     -----------------------------------*/
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsAccept - Subscription 0x%p sending %d response",pSubs, responseCode));

    rv = RvSipTransactionRespond(pSubs->hActiveTransc,responseCode,NULL);

    if(rv != RV_OK) /* return status of Respond function */
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsAccept - Subscription 0x%p - Failed to send %d response (status %d)",
            pSubs, responseCode, rv));
        return rv;
    }

    pSubs->hActiveTransc = NULL;

    /* set the subscription timer - only when accepting SUBSCRIBE*/
    if(RV_FALSE == bRefer)
    {
        /* if expiration val is 0, we leave the alert timer as it was, and not
            reset it. if it is null, however, we will set it, so timer expiration
            event will be given. (we don't want that a subscription will not have any
            timer...)
           (pSubs->expirationVal > 0) -->
                             there is a new value for alert timer.
           (pSubs->expirationVal == 0 && pSubs->hAlertTimer == NULL) -->
                             this is a 'fetch' case, and not a regular unsubscribe*/
        if((pSubs->expirationVal > 0) ||
            (pSubs->expirationVal == 0 &&
			 pSubs->eState		  == RVSIP_SUBS_STATE_SUBS_RCVD))
        {
            /* expirationVal is in seconds. we need to convert it to milliseconds */
            rv = SubsObjectSetAlertTimer(pSubs, (pSubs->expirationVal)*1000);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsAccept - Subscription 0x%p - Failed to set subscription timer",
                    pSubs));
                return rv;
            }
        }
    }

    if(responseCode == 202 && bRefer == RV_FALSE)
    {
        SubsChangeState(pSubs,RVSIP_SUBS_STATE_PENDING,RVSIP_SUBS_REASON_UNDEFINED);
    }
    else /*200 or 202 on REFER */
    {
        if(pSubs->eState == RVSIP_SUBS_STATE_REFRESH_RCVD ||
           pSubs->eState == RVSIP_SUBS_STATE_SUBS_RCVD)
        {
            SubsChangeState(pSubs,RVSIP_SUBS_STATE_ACTIVE,RVSIP_SUBS_REASON_UNDEFINED);
        }
        /* for state RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD,
           remain in the same state, until notify(terminated) is sent. */
    }

    return RV_OK;
}

/***************************************************************************
* SubsReject
* ------------------------------------------------------------------------
* General: Reject a subscription: the response message is send, and subscription state
*          is updated: If the reject is for initial subscribe - the subscription is tarminated.
*          If the reject is for refresh subscribe - change state back to active.
*          (we handle refresh only in active state, for other states, the refresh is rejected
*           automaicaly with 400).
*          If the reject is for unsubscribe - change state back to previous state (can be
*          active, pending, or activated).
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription.
*          statusCode  - statusCode for the response message.
*          strReasonPhrase - reason phrase for this response message. if NULL, default is set.
***************************************************************************/
RvStatus RVCALLCONV SubsReject (IN  Subscription *pSubs,
                                 IN  RvUint16     statusCode,
                                 IN  RvChar*      strReasonPhrase)
{
    RvStatus                   rv;
    RvSipMsgHandle              hMsg;

    if(pSubs->hOutboundMsg != NULL)
    {
        hMsg = pSubs->hOutboundMsg;
        rv = SipTransactionSetOutboundMsg(pSubs->hActiveTransc, hMsg);
        if(RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReject - Subscription 0x%p - failed to set outbound message in transaction",pSubs));
            return rv;
        }
        pSubs->hOutboundMsg = NULL;
    }

    /* -------------------
       send the response
       ------------------- */
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsReject - Subscription 0x%p sending %d response...",pSubs, statusCode));

    rv = RvSipTransactionRespond(pSubs->hActiveTransc,statusCode,strReasonPhrase);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReject - Subscription 0x%p - Failed to send %d response (status %d)",
            pSubs, statusCode, rv));
        return rv;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsReject - Subscription 0x%p  - %d response was sent successfuly",pSubs, statusCode));

    SubsUpdateStateAfterReject(pSubs, RVSIP_SUBS_REASON_UNDEFINED);
    return RV_OK;
}

/***************************************************************************
* SubsSendProvisional
* ------------------------------------------------------------------------
* General: Send a provisional response on a subscription:
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription.
*          statusCode  - statusCode for the response message.
*          strReasonPhrase - reason phrase for this response message.
*                        if NULL, default is set.
***************************************************************************/
RvStatus RVCALLCONV SubsSendProvisional (IN  Subscription *pSubs,
                                          IN  RvUint16     statusCode,
                                          IN  RvChar*      strReasonPhrase)
{
    RvStatus                   rv;
    RvSipMsgHandle              hMsg;

    if(pSubs->hOutboundMsg != NULL)
    {
        hMsg = pSubs->hOutboundMsg;
        rv = SipTransactionSetOutboundMsg(pSubs->hActiveTransc, hMsg);
        if(RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSendProvisional - Subscription 0x%p - failed to set outbound message in transaction",pSubs));
            return rv;
        }
        pSubs->hOutboundMsg = NULL;
    }

    /* -------------------
       send the response
       ------------------- */
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsSendProvisional - Subscription 0x%p sending %d response...",pSubs, statusCode));

    rv = RvSipTransactionRespond(pSubs->hActiveTransc,statusCode,strReasonPhrase);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsSendProvisional - Subscription 0x%p - Failed to send %d response (status %d)",
            pSubs, statusCode, rv));
        return rv;
    }

    /* user still need to send final response on this transaction */

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsSendProvisional - Subscription 0x%p - %d response was sent successfuly",pSubs, statusCode));


    return RV_OK;
}

/***************************************************************************
 * SubsSendRequest
 * ------------------------------------------------------------------------
 * General: Send SUBSCRIBE or REFER request.
 *          The function does the actual sending, and is the only message
 *          that sends subscription requests.
 *          In this function we set the message parameters.
 *
 * Return Value: RV_ERROR_OUTOFRESOURCES - Cannot subscribe due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send the subscribe message.
 *               RV_OK - subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs			 - Pointer to the subscription the user wishes to subscribe.
 *          expiresVal		 - expires to set in request msg.
 *          bRefer			 - TRUE for sending REFER. FALSE for sending SUBSCRIBE.
 *          bCreateNewTransc - RV_TRUE if this is a new transaction. RV_FALSE
 *                             if the transaction was already created, and
 *                             resides in hActiveTransc
 ***************************************************************************/
RvStatus RVCALLCONV SubsSendRequest (IN  Subscription	*pSubs,
                                     IN  RvInt32		expiresVal,
                                     IN  RvBool			bRefer,
									 IN  RvBool			bCreateNewTransc)
{
    RvStatus rv;
    SipTransactionMethod eMethod;
    RvSipCallLegState eCallLegState;

    eMethod = (RV_TRUE==bRefer)?SIP_TRANSACTION_METHOD_REFER:SIP_TRANSACTION_METHOD_SUBSCRIBE;

    /*------------------------------------------
        Prepare Subscribe transaction and message.
      -------------------------------------------*/
    rv = SubsPrepareSubscribeTransc(pSubs, expiresVal, bRefer, bCreateNewTransc);
    if(rv != RV_OK)
    {
		if (pSubs->hActiveTransc != NULL)
		{
			SubsRemoveTranscFromList(pSubs, pSubs->hActiveTransc);
			SipTransactionTerminate(pSubs->hActiveTransc);
			pSubs->hActiveTransc = NULL;
		}
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsSendRequest - Subscription 0x%p - Failed to create subscribe request (status %d)",
            pSubs, rv));
        return rv;
    }

    /*----------------------------------------------------------------
        Insert hidden call-leg to the hash - if the subs state is idle,
        and the call-leg is hidden, or refer idle call-leg.
        (when working with refer wrappers, the call-leg might be idle, but
        not hidden, if application created if, and then called to RvSipCallLegRefer)
        here we do not check if there is another same call-leg in the hash,
        because the to-tag will distinguish the call-legs.
        (if state is not idle, a subscribe was already sent once)
      ----------------------------------------------------------------*/
    RvSipCallLegGetCurrentState(pSubs->hCallLeg, &eCallLegState);

    if(pSubs->eState == RVSIP_SUBS_STATE_IDLE &&
       (RV_TRUE == SipCallLegIsHiddenCallLeg(pSubs->hCallLeg) ||
       (RV_TRUE == bRefer && RVSIP_CALL_LEG_STATE_IDLE == eCallLegState)))
    {
        rv = SipCallLegSubsInsertCallToHash(pSubs->hCallLeg, RV_FALSE);
        if(rv != RV_OK)
        {
			SubsRemoveTranscFromList(pSubs, pSubs->hActiveTransc);
			SipTransactionTerminate(pSubs->hActiveTransc);
			pSubs->hActiveTransc = NULL;
			
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsSendRequest - Subscription 0x%p - Failed to insert hidden call-leg 0x%p to hash table (status %d)",
                pSubs, pSubs->hCallLeg, rv));
            return rv;
        }
    }
    /*----------------------------------
         send the request
     -----------------------------------*/
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsSendRequest - Subscription 0x%p sending %s",pSubs, SipTransactionGetStringMethodFromEnum(eMethod)));

    rv = SipCallLegSendRequest(pSubs->hCallLeg,pSubs->hActiveTransc,eMethod,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsSendRequest - Subscription 0x%p - Failed to send %s request (status %d)",
            pSubs, SipTransactionGetStringMethodFromEnum(eMethod),rv));
        /* The request transaction should not be handled by the subscription anymore.
           we remove it from the subscription, and we also reset the subscription information 
           in the transaction */
        SubsRemoveTranscFromList(pSubs, pSubs->hActiveTransc);
        SipTransactionDetachOwner(pSubs->hActiveTransc);
        SipTransactionSetSubsInfo(pSubs->hActiveTransc, NULL);
        SipTransactionTerminate(pSubs->hActiveTransc);
        pSubs->hActiveTransc = NULL;
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
* SubsTerminate
* ------------------------------------------------------------------------
* General: Terminate a subscription, without sending any message.
*          subscription stateis changed to TERMINATED, and subscription is destructed,
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription the user wishes to subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsTerminate (IN  Subscription               *pSubs,
                                    IN  RvSipSubsStateChangeReason reason)
{
    if (NULL == pSubs)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsTerminate: subs 0x%p internal state changes %s->TERMINATED. reason=%s",
            pSubs, SubsGetStateName(pSubs->eState), SubsGetChangeReasonName(reason)));

    pSubs->eState = RVSIP_SUBS_STATE_TERMINATED;
    if(RVSIP_SUBS_REASON_UNDEFINED == pSubs->eTerminationReason)
    {
        pSubs->eTerminationReason = reason;
    }

    /* release timers */
    SubsAlertTimerRelease(pSubs);
    SubsNoNotifyTimerRelease(pSubs);

    /* we need to take out a hidden call-leg from the hash.
       this will prevent the case that a new SUBSCRIBE request will find the
       already exist call-leg, and then for some time a hidden call-leg will
       have 2 subscriptions....
       (scenario - answer 401 on SUBSCRIBE, and before the org subs is out of
        the termination queue, a new SUBSCRIBE message, with authorization is
        received and find this call-leg.) */
    if(SipCallLegIsHiddenCallLeg(pSubs->hCallLeg) == RV_TRUE)
    {
        SipCallLegSubsRemoveHiddenCallFromHash(pSubs->hCallLeg);
    }

    /* delete association with object created by refer subscription */
    SubsReferDetachReferResultObjAndReferSubsGenerator(pSubs);

    SubsTerminateAllTransc(pSubs, RV_FALSE);
    SubsTerminateIfPossible(pSubs);
    return RV_OK;
}

/***************************************************************************
* SubsTerminateIfPossible
* ------------------------------------------------------------------------
* General: the order of objects termination is: transactions, notifications,
*          subscription.
*          This function send subscription to the termination queue, only if
*          it has no transaction and notification alive.
*          Otherwise, it terminates the transactions/notifications objects
*          first.
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription the user wishes to subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsTerminateIfPossible (IN  Subscription *pSubs)
{

    RvStatus rv = RV_OK;
    RLIST_ITEM_HANDLE pItem = NULL;

    /* --------------------------------------------------------------------
       1. if there are transactions in the list - break from the function.
          must wait for the last transaction to be terminated
       --------------------------------------------------------------------*/
    RLIST_GetHead(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, &pItem);
    if(pItem != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsTerminateIfPossible - Subs 0x%p - wait Transc 0x%p to be terminated",
            pSubs, *((RvSipTranscHandle*)pItem)));
        return RV_OK;
    }

    /* --------------------------------------------------------------------
        2. no transactions in the list. terminate notifications
       -------------------------------------------------------------------- */
    if(pSubs->numOfNotifyObjs > 0)
    {
        /* terminate all notification objects */
        SubsNotifyTerminateNotifications(pSubs,RV_FALSE);
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsTerminateIfPossible - Subs 0x%p - wait %d Notify objects to be terminated",
            pSubs, pSubs->numOfNotifyObjs));
        return RV_OK;
    }

    /* --------------------------------------------------------------------
        3. in this stage there are no transactions and no notifications.
           we can terminate the subscription.
       -------------------------------------------------------------------- */
    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsTerminateIfPossible - Subs 0x%p - set subscription obj in termination queue",
        pSubs));
    rv = SipTransportSendTerminationObjectEvent(pSubs->pMgr->hTransportMgr,
                                      (void *)pSubs,
                                      &(pSubs->terminationInfo),
                                      (RvInt32)pSubs->eTerminationReason,
                                      TerminateSubsEventHandler,
                                      RV_TRUE,
                                      SUBS_STATE_TERMINATED_STR,
                                      SUBS_OBJECT_NAME_STR);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsTerminateIfPossible - Subs 0x%p - Failed to set subscription obj in termination queue (rv %d)",
            pSubs, rv));
    }

    return rv;
}

/***************************************************************************
* SubsDestruct
* ------------------------------------------------------------------------
* General: Destruct a subscription, without sending it or any of it's objects
*          (notify, transaction) to the termination queue.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription to destruct.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsDestruct (IN  Subscription *pSubs)
{

    RvStatus rv = RV_OK;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDestruct: subs 0x%p is going to be destructed",pSubs));

    /* release timers */
    SubsAlertTimerRelease(pSubs);
    SubsNoNotifyTimerRelease(pSubs);

    /* delete association with object created by refer subscription */
    SubsReferDetachReferResultObjAndReferSubsGenerator(pSubs);

    SubsTerminateAllTransc(pSubs, RV_TRUE);
    SubsNotifyTerminateNotifications(pSubs, RV_TRUE);
    SubsFree(pSubs);
    return rv;
}

/***************************************************************************
 * TerminateSubsEventHandler
 * ------------------------------------------------------------------------
 * General: Terminates a subscription object, Free the resources taken by it and
 *          remove it from the manager subscription-leg list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     obj        - pointer to subscription to be deleted.
 *          reason    - reason of the subscription termination
 ***************************************************************************/
static RvStatus RVCALLCONV TerminateSubsEventHandler(IN void *obj, 
                                                     IN RvInt32  reason)
{
    Subscription        *pSubs;
    RvStatus            retCode = RV_OK;
    RvMutex *hObjLock;
    RvMutex *hProcessingLock;
    RvLogMgr  *pLogMgr;
    RvLogSource* pLogSrc;
    void      *pSubsForPrint;
    void      *tripleLockForPrint;

    pSubs = (Subscription *)obj;
    if (pSubs == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    retCode = SubsLockMsg(pSubs);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    hObjLock = &pSubs->tripleLock->hLock;
    hProcessingLock = &pSubs->tripleLock->hProcessLock;
    pLogMgr = pSubs->pMgr->pLogMgr;
    pLogSrc = pSubs->pMgr->pLogSrc;
    pSubsForPrint = (void*)pSubs;
    tripleLockForPrint = pSubs->tripleLock;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "TerminateSubsEventHandler - Subs 0x%p - is out of the termination queue", pSubs));

    /* inform user with state changed */
    SubsChangeState(pSubs, RVSIP_SUBS_STATE_TERMINATED, (RvSipSubsStateChangeReason)reason);

    if(pSubs->numOfNotifyObjs == 0)
    {
        /* if there are alive notification objects - the last notification will
        destruct the subscription */
        SubsFree(pSubs);
    }
    else
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "TerminateSubsEventHandler - Subs 0x%p - exists %d notify object",
            pSubs, pSubs->numOfNotifyObjs));
    }

    retCode = SubsTerminateUnlock(pLogMgr,pLogSrc,pSubsForPrint,tripleLockForPrint,
                                  hObjLock,hProcessingLock);
    return RV_OK;
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
* SubsRespondUnauth
* ------------------------------------------------------------------------
* General: Sends 401 or 407 response, for a subscribe or notify request.
*          If a response for subscribe was sent, update subscription state-machine.
*          If a response for notify was sent, remove the notification object from list.
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription the user wishes to subscribe.
*          responseCode -   401 or 407
*          strReasonPhrase - May be NULL, for default reason phrase.
*          strRealm -       mandatory.
*          strDomain -      Optional string. may be NULL.
*          strNonce -       Optional string. may be NULL.
*          strOpaque -      Optional string. may be NULL.
*          bStale -         TRUE or FALSE
*          eAlgorithm -     Enumuration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
*                           the algorithm value is taken from the the next argument.
*          strAlgorithm -   String of algorithm. this paraemters will be set only if
*                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
*          eQop -           Enumuration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
*                           will be taken from the next argument.
*          strQop -         String of qop. this parameter will be set only
***************************************************************************/
RvStatus RVCALLCONV SubsRespondUnauth (IN  Subscription        *pSubs,
                                        IN  Notification        *pNotify,
                                        IN  RvUint16            responseCode,
                                        IN  RvChar              *strReasonPhrase,
                                        IN  RvSipHeaderType      headerType,
                                        IN  void*                hHeader,
                                        IN  RvChar              *strRealm,
                                        IN  RvChar              *strDomain,
                                        IN  RvChar              *strNonce,
                                        IN  RvChar              *strOpaque,
                                        IN  RvBool              bStale,
                                        IN  RvSipAuthAlgorithm   eAlgorithm,
                                        IN  RvChar              *strAlgorithm,
                                        IN  RvSipAuthQopOption   eQop,
                                        IN  RvChar              *strQop)
{
    RvStatus    rv;
    RvSipTranscHandle hTransc;

    RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
              "SubsRespondUnauth - Subscription 0x%p Notification 0x%p: about to send %d response",
              pSubs, pNotify, responseCode));

    /* 1. send the response */
    if(pNotify != NULL)
    {
        hTransc = pNotify->hTransc;
        if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD)
        {
            RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
              "SubsRespondUnauth - Subscription 0x%p Notification 0x%p: illegal notify status %d",
              pSubs, pNotify, pNotify->eStatus));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    else
    {
        hTransc = pSubs->hActiveTransc;
        if((pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD) &&
            (pSubs->eState != RVSIP_SUBS_STATE_REFRESH_RCVD) &&
            (pSubs->eState != RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD))
        {
            RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
              "SubsRespondUnauth - Subscription 0x%p Notification 0x%p: illegal subs state %s",
              pSubs, pNotify, SubsGetStateName(pSubs->eState)));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    if(hTransc == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsRespondUnauth - Failed. transaction is NULL for subscription 0x%p",
               pSubs));

        SubsTerminate(pSubs, RVSIP_SUBS_REASON_LOCAL_FAILURE);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pNotify == NULL && pSubs->hOutboundMsg != NULL)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pSubs->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsRespondUnauth - subs 0x%p, failed to set outbound message in transaction 0x%p",
               pSubs, hTransc));
            return rv;
        }
        pSubs->hOutboundMsg = NULL;
    }
    if(hHeader != NULL)
    {
        rv = RvSipTransactionRespondUnauthenticated(hTransc,
                                                    responseCode,
                                                    strReasonPhrase,
                                                    headerType,
                                                    hHeader);
    }
    else
    {
        rv = RvSipTransactionRespondUnauthenticatedDigest(hTransc, responseCode, strReasonPhrase,
                                                      strRealm, strDomain, strNonce, strOpaque,
                                                      bStale, eAlgorithm, strAlgorithm, eQop, strQop);
    }
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsRespondUnauth -  Subscription 0x%p - Failed to send %d response (rv=%d)",
                pSubs, responseCode, rv));
        return rv;
    }

    /* 2. update the state machine */
    if(pNotify == NULL) /* update subscription state-machine */
    {
        /* the response was sent on a subscribe request */
        SubsUpdateStateAfterReject(pSubs, RVSIP_SUBS_REASON_UNDEFINED);
    }
    else
    {
        /*notify the notify sent status*/
        InformNotifyStatus(pNotify,RVSIP_SUBS_NOTIFY_STATUS_FINAL_RESPONSE_SENT,
                            RVSIP_SUBS_NOTIFY_REASON_UNDEFINED,NULL);
        
        /*subs was terminated from callback*/
        if(pNotify->pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsRespondUnauth - Subs 0x%p notification 0x%p - subscription was terminated from callback. returning",
                pSubs, pNotify));
            return RV_OK;
        }
    }

    /* Notify will be destructed later on Transaction free.
       The Transaction will be destructed on
       OBJ_TERMINATION_DEFAULT_TCP/TLS_TIMEOUT timer */

    return rv;
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/*-----------------------------------------------------------------------
        L O C K   F U N C T I O N S
 ------------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * SubsLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks subscription according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
RvStatus RVCALLCONV SubsLockAPI(IN  Subscription*   pSubs)
{
    RvStatus            retCode;
    SipTripleLock        *tripleLock;
    SubsMgr                *pMgr;
    RvInt32            identifier;
    if (pSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {

        RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);
        pMgr = pSubs->pMgr;
        tripleLock = pSubs->tripleLock;
        identifier = pSubs->subsUniqueIdentifier;
        RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

        if (tripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "SubsLockAPI - Locking subscription 0x%p: Triple Lock 0x%p", pSubs,
                  tripleLock));

        retCode = SubsLock(pSubs,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pSubs->tripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                    "SubsLockAPI - Locking subscription 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
                    pSubs,tripleLock));
        SubsUnLock(pMgr->pLogMgr,&tripleLock->hLock);

    } 

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        SubsUnLock(pSubs->pMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }
    tripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock,pSubs->pMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SubsLockAPI - Locking subscription 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pSubs, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * SubsUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Unlocks subscription according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
void RVCALLCONV SubsUnLockAPI(IN  Subscription*   pSubs)
{
    SipTripleLock        *tripleLock;
    SubsMgr                *pMgr;
    RvInt32               lockCnt = 0;

    if (pSubs == NULL)
    {
        return;
    }

    RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);
    pMgr = pSubs->pMgr;
    tripleLock = pSubs->tripleLock;
    RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock,pSubs->pMgr->pLogMgr,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SubsUnLockAPI - UnLocking subscription 0x%p: Triple Lock 0x%p, apiCnt=%d lockCnt=%d",
             pSubs, tripleLock, tripleLock->apiCnt,lockCnt));

    if (tripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SubsUnLockAPI - UnLocking subscription 0x%p: Triple Lock 0x%p, apiCnt=%d",
             pSubs, tripleLock, tripleLock->apiCnt));
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,pSubs->pMgr->pLogMgr);
    }

    tripleLock->apiCnt--;
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }
    SubsUnLock(pSubs->pMgr->pLogMgr, &tripleLock->hLock);
}

/************************************************************************************
 * SubsLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks subscription according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
RvStatus RVCALLCONV SubsLockMsg(IN  Subscription*   pSubs)
{
    RvStatus        retCode      = RV_OK;
    RvBool          didILockAPI  = RV_FALSE;
    RvThreadId      currThreadId = RvThreadCurrentId();
    SipTripleLock  *tripleLock;
    SubsMgr        *pMgr;
    RvInt32         identifier;


    if (pSubs == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);
    tripleLock = pSubs->tripleLock;
    pMgr = pSubs->pMgr;
    identifier = pSubs->subsUniqueIdentifier;
    RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "SubsLockMsg - Exiting without locking Subs 0x%p: Triple Lock 0x%p apiCnt=%d already locked",
                   pSubs, tripleLock, tripleLock->apiCnt));

        return RV_OK;
    }

    RvMutexLock(&tripleLock->hProcessLock,pSubs->pMgr->pLogMgr);

    for(;;)
    {
        retCode = SubsLock(pSubs,pMgr, tripleLock,identifier);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
          if (didILockAPI)
          {
            RvSemaphorePost(&tripleLock->hApiLock,NULL);
          }

          return retCode;
        }
        if (tripleLock->apiCnt == 0)
        {
            if (didILockAPI)
            {
                RvSemaphorePost(&tripleLock->hApiLock,NULL);
            }

            break;
        }

        SubsUnLock(pSubs->pMgr->pLogMgr, &tripleLock->hLock);

        retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
          return retCode;
        }

        didILockAPI = RV_TRUE;
    } 

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SubsLockMsg - Locking subscription 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pSubs, tripleLock, tripleLock->apiCnt));
    return retCode;
}


/************************************************************************************
 * SubsUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks subscription according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
void RVCALLCONV SubsUnLockMsg(IN  Subscription*   pSubs)
{
    SipTripleLock   *tripleLock;
    SubsMgr         *pMgr;
    RvThreadId       currThreadId = RvThreadCurrentId();

    if (pSubs == NULL)
    {
        return;
    }

    RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);
    pMgr = pSubs->pMgr;
    tripleLock = pSubs->tripleLock;
    RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return;
    }
    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "SubsUnLockMsg - Exiting without unlocking Subs 0x%p: Triple Lock 0x%p wasn't locked",
                   pSubs, tripleLock));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SubsUnLockMsg - UnLocking subscription 0x%p: Triple Lock 0x%p", pSubs,
             tripleLock));

    SubsUnLock(pSubs->pMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
}
#endif
/*-----------------------------------------------------------------------
       G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * SubsSetEventHeader
 * ------------------------------------------------------------------------
 * General: Set the Event header associated with the subscription.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - Event header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the subscription.
 *            hEvent - Handle to an application constructed event header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsSetEventHeader (
                                      IN  Subscription*           pSubs,
                                      IN  RvSipEventHeaderHandle  hEvent)
{

    RvStatus rv;

    /* first check if the given header was constructed on the subscription page
      and if so attach it*/
    if( SipEventHeaderGetPool(hEvent) == pSubs->pMgr->hGeneralPool &&
        SipEventHeaderGetPage(hEvent) == pSubs->hPage)
    {
        pSubs->hEvent = hEvent;
        return RV_OK;
    }

    /* construct and copy*/

    /* if there is no Event header, construct it*/
    if(pSubs->hEvent == NULL)
    {
        rv = RvSipEventHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                       pSubs->pMgr->hGeneralPool,
                                       pSubs->hPage,
                                       &(pSubs->hEvent));
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsSetEventHeader - subscription 0x%p - Failed to construct Event header (rv=%d)",
                   pSubs,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /*copy information from the given hEvent the the subscription's event header*/
    rv = RvSipEventHeaderCopy(pSubs->hEvent, hEvent);
    if(rv != RV_OK)
    {
         RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsSetEventHeader - subscription 0x%p - Failed to set Event header - copy failed (rv=%d)",
                   pSubs,rv));
         return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;
}


/***************************************************************************
* SubsInitialize
* ------------------------------------------------------------------------
* General: Initialize a new subscription in the Idle state. Sets all values to their
*          initial values, allocates a memory page for this subscription.
* Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize subscription due to a
*                                   resources problem.
*               RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs - pointer to the subscription.
*           hAppSubs - Handle to the app subs.
*           hCallLeg - Handle to the call-leg of the subscription.
*           pMgr  - Pointer to the subs manager
*           eType - Subscriber or Notifier.
*           bOutOfBand - a regular subscription, or out of band subscription.
*           hSubsPage - Page, which will be used by the Subscriptions.
***************************************************************************/
RvStatus RVCALLCONV SubsInitialize(
                                   IN  Subscription          *pSubs,
                                   IN  RvSipAppSubsHandle    hAppSubs,
                                   IN  RvSipCallLegHandle    hCallLeg,
                                   IN  struct SubsMgrStruct  *pMgr,
                                   IN  RvSipSubscriptionType eType,
                                   IN  RvBool                isOutOfBand,
                                   IN  HPAGE                 hSubsPage)
{

    pSubs->hActiveTransc   = NULL;
    pSubs->alertTimeout    = pMgr->alertTimeout;
    pSubs->noNotifyTimeout = pMgr->noNotifyTimeout;
    pSubs->autoRefresh     = pMgr->autoRefresh;
    pSubs->bOutOfBand      = isOutOfBand;
    pSubs->hCallLeg        = hCallLeg;
    if(isOutOfBand == RV_TRUE)
    {
        pSubs->eState = RVSIP_SUBS_STATE_ACTIVE;
    }
    else
    {
        pSubs->eState = RVSIP_SUBS_STATE_IDLE;
    }
    pSubs->ePrevState                       = RVSIP_SUBS_STATE_UNDEFINED;
    pSubs->eLastState                       = RVSIP_SUBS_STATE_UNDEFINED;
    pSubs->eStateBeforeUnsubsWasSent        = RVSIP_SUBS_STATE_UNDEFINED;
    pSubs->bActivatedOn2xxBeforeNotifyState = RV_FALSE;
    pSubs->eSubsType                        = eType;
    pSubs->expirationVal                    = UNDEFINED;
    pSubs->requestedExpirationVal           = UNDEFINED;
    SipCommonCoreRvTimerInit(&pSubs->alertTimer);
    pSubs->bWasAlerted       = RV_FALSE;
    pSubs->hAppSubs          = hAppSubs;
    pSubs->hEvent            = NULL;
    pSubs->bEventHeaderExist = RV_TRUE;
    pSubs->hOutboundMsg      = NULL;
    SipCommonCoreRvTimerInit(&pSubs->noNotifyTimer);
    pSubs->subsEvHandlers = &(pMgr->subsEvHandlers);
    pSubs->numOfNotifyObjs    = 0;
    pSubs->hItemInCallLegList = NULL;
    pSubs->hTranscList        = NULL;
    pSubs->hNotifyList        = NULL;
    pSubs->hPage              = hSubsPage;
    pSubs->eTerminationReason = RVSIP_SUBS_REASON_UNDEFINED;
    memset(&(pSubs->terminationInfo), 0, sizeof(RvSipTransportObjEventInfo));
    pSubs->pReferInfo                        = NULL;
    pSubs->hReferredByHeader                 = NULL;
    pSubs->hHeadersListToSetInInitialRequest = NULL;
    pSubs->hReferSubsGenerator               = NULL;
    pSubs->pOriginalSubs                     = NULL;
	SipCommonCSeqSetStep(0,&pSubs->expectedCSeq);
    pSubs->createdRejectStatusCode           = 0;
    pSubs->cbBitmap                          = 0;
    pSubs->retryAfter                        = 0;

    /* allocate transactions list on the subscription page */
   /* pSubs->hTranscList = RLIST_RPoolListConstruct(pSubs->pMgr->hGeneralPool,
                                                pSubs->hPage,
                                                sizeof(RvSipTranscHandle),
                                                pMgr->pLogSrc);*/
    /*initialize a list of transactions for this call*/
    pSubs->hTranscList = RLIST_ListConstruct(pMgr->hTranscHandlesPool);
    if(pSubs->hTranscList == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsInitialize - subs 0x%p - Failed to construct transaction list", pSubs));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Set unique identifier only on initialization success */
    RvRandomGeneratorGetInRange(pSubs->pMgr->seed, RV_INT32_MAX,
        (RvRandom*)&pSubs->subsUniqueIdentifier);

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsInitialize - subs 0x%p - transaction list 0x%p was constructed well", pSubs, pSubs->hTranscList));

    /*initialize a list of notifications for this subscription */
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
    pSubs->hNotifyList = RLIST_ListConstruct(pMgr->hNotifyListPool);
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    if(pSubs->hNotifyList == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsInitialize - subs 0x%p - Failed to construct notification list", pSubs));
        pSubs->hNotifyList = NULL;
        RLIST_ListDestruct(pMgr->hTranscHandlesPool,pSubs->hTranscList);
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}



/***************************************************************************
 * SubsChangeState
 * ------------------------------------------------------------------------
 * General: Changes the subscription state and notify the application about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsChangeState( IN  Subscription              *pSubs,
                                 IN  RvSipSubsState             eState,
                                 IN  RvSipSubsStateChangeReason eReason)
{
    RvStatus   rv = RV_OK;

    if(eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsChangeState - subscription 0x%p state changed: %s->%s, (reason = %s)",
               pSubs, SubsGetStateName(pSubs->eLastState),SubsGetStateName(eState),
               SubsGetChangeReasonName(eReason)));
    }
    else
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsChangeState - subscription 0x%p state changed: %s->%s, (reason = %s)",
               pSubs, SubsGetStateName(pSubs->eState),SubsGetStateName(eState),
               SubsGetChangeReasonName(eReason)));
    }

    /*change state in the subscription */
    pSubs->ePrevState = pSubs->eState;
    pSubs->eState = eState;
    pSubs->eLastState = eState;

    /*notify the application about the new state*/
    if((pSubs->subsEvHandlers != NULL &&
        pSubs->subsEvHandlers->pfnStateChangedEvHandler != NULL) ||
       pSubs->pMgr->bDisableRefer3515Behavior == RV_TRUE)
    {
        SubsCallbackChangeSubsStateEv(pSubs,eState, eReason);
    }
    else
    {
        /* if the pfnStateChangedEvHandler was not implemented,
           we send "501 not implemented" response (in state subs_rcvd).*/
        if(eState == RVSIP_SUBS_STATE_SUBS_RCVD)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsChangeState - subscription 0x%p - respond with 501 - pfnStateChangedEvHandler was not implemented",
                pSubs));
            rv = RvSipTransactionRespond(pSubs->hActiveTransc, 501, NULL);
            if(rv != RV_OK)
            {
                RvSipTransactionTerminate(pSubs->hActiveTransc);
            }

            /*detach the transaction owner, otherwise, the 501 won't be sent in tcp */
            SubsDetachTranscAndRemoveFromList(pSubs, pSubs->hActiveTransc);
            pSubs->hActiveTransc = NULL;

            SubsTerminate(pSubs, RVSIP_SUBS_REASON_UNDEFINED);
        }
    }

    /* if the subscription was created because of a refer request, inform the creating subs */
    if(NULL != pSubs->hReferSubsGenerator &&
       RVSIP_SUBS_STATE_TERMINATED == eState)
    {
        RvSipSubsReferNotifyReadyReason eReferReason;

        if(RVSIP_SUBS_REASON_TRANSC_TIME_OUT == eReason)
        {
            eReferReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_TIMEOUT;
        }
        else
        {
            eReferReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_ERROR_TERMINATION;
        }

        /* notify the associate refer subscription about the termination */
        SubsReferGeneratorCallNotifyReady(
            pSubs->hReferSubsGenerator, eReferReason, NULL);
    }
}

/***************************************************************************
 * SubsNoNotifyTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the noNotify timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransaction - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV SubsNoNotifyTimerRelease(IN Subscription *pSubs)
{
    if ((NULL != pSubs) && SipCommonCoreRvTimerStarted(&pSubs->noNotifyTimer))
    {
        RvLogDebug((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
            "SubsNoNotifyTimerRelease - Subscription 0x%p: Timer 0x%p was released",
            pSubs, &pSubs->noNotifyTimer));

        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pSubs->noNotifyTimer);
    }
}

/***************************************************************************
 * SubsAlertTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the alert timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - The subscription to release the timer from.
 ***************************************************************************/
void RVCALLCONV SubsAlertTimerRelease(IN Subscription *pSubs)
{
    if ((NULL != pSubs) && SipCommonCoreRvTimerStarted(&pSubs->alertTimer))
    {
        RvLogDebug((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
            "SubsAlertTimerRelease - Subscription 0x%p: Timer 0x%p was released",
            pSubs, &pSubs->alertTimer));

        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pSubs->alertTimer);
    }
}
/***************************************************************************
* SubsAlertTimerHandleTimeout
* ------------------------------------------------------------------------
* General: Called when ever the alert timer expires.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: timerHandle - The timer that has expired.
*          pContext - The subscription this timer was called for.
***************************************************************************/
RvBool SubsAlertTimerHandleTimeout(IN void  *pContext,
                                   IN RvTimer *timerInfo)
{
    Subscription        *pSubs;
    RvStatus           rv;

    pSubs = (Subscription*)pContext;
    if (RV_OK != SubsLockMsg(pSubs))
    {
        RvLogWarning((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
            "SubsAlertTimerHandleTimeout - Subscription 0x%p: failed to lock subs",
            pSubs));
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pSubs->alertTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc  ,
            "SubsAlertTimerHandleTimeout - Subs 0x%p: alert timer expired but was already released. do nothing...",
            pSubs));
        
        SubsUnLockMsg(pSubs);
        return RV_FALSE;
    }

    SipCommonCoreRvTimerExpired(&pSubs->alertTimer);

    /* check state */
    if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        SubsAlertTimerRelease(pSubs);
        SubsUnLockMsg(pSubs);
        return RV_FALSE;
    }

    if(pSubs->eSubsType == RVSIP_SUBS_TYPE_SUBSCRIBER &&
        pSubs->bWasAlerted == RV_FALSE &&
        pSubs->alertTimeout > 0 &&
        pSubs->alertTimeout <= pSubs->expirationVal*1000)/* alertTimeout is in mili-sec, expirationVal is in sec */
    {
        /* -------------------------------------------------------------------------
         Subscriber that was not alerted yet, and want to be alerted (alertTimeout>0) -
          1. set the alert timer to the rest of subscription timer.
          2. give an alert to user, or send auto refresh.
         we also check here(pSubs->alertTimeout < pSubs->expirationVal) because if
         the subscription expirationVal is lower than alertTimeout value, we
         had set the alertTimeout to the expirationVal and not to
         expirationVal-alertTimeout, so at this point it is expiration and not alert.
        ---------------------------------------------------------------------------- */

        /* alert timeout is already in milliseconds */
        rv = SubsObjectSetAlertTimer(pSubs, (pSubs->alertTimeout));
        pSubs->bWasAlerted = RV_TRUE;
        if(rv != RV_OK)
        {
            RvLogError((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
                "SubsAlertTimerHandleTimeout - Subscription 0x%p: failed to set again alert timer %p",
                pSubs, &pSubs->alertTimer));
            SubsTerminate(pSubs, RVSIP_SUBS_REASON_NO_TIMERS);
            SubsUnLockMsg(pSubs);
            return RV_FALSE;
        }

        if(pSubs->autoRefresh == RV_TRUE)
        {
            /* -----------------------
                Auto Refresh
               ----------------------- */
            if(pSubs->eState != RVSIP_SUBS_STATE_ACTIVE)
            {
                RvLogWarning((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
                    "SubsAlertTimerHandleTimeout - Subscription 0x%p: can't send auto refresh at state %s",
                    pSubs, SubsGetStateName(pSubs->eState)));

            }
            else
            {
                RvLogInfo((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
                    "SubsAlertTimerHandleTimeout - Subscription 0x%p: send automatic refreshing subscribe",
                    pSubs));
                if(pSubs->bOutOfBand == RV_FALSE &&
                   pSubs->expirationVal > 0 &&
                   pSubs->eState == RVSIP_SUBS_STATE_ACTIVE)
                {
                    rv = SubsRefresh(pSubs, pSubs->expirationVal);
                    if(rv!= RV_OK)
                    {
                        RvLogError((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
                            "SubsAlertTimerHandleTimeout - Subscription 0x%p: failed to send refresh",
                            pSubs));
                    }
                }
                else
                {
                    RvLogWarning((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
                            "SubsAlertTimerHandleTimeout - Subs 0x%p: failed to auto refresh. bOutOfBand=%d, expirationVal=%d, State=%s",
                            pSubs, pSubs->bOutOfBand, pSubs->expirationVal,
                            SubsGetStateName(pSubs->eState)));
                }
            }
        }
        else
        {
            /* ------------------------------
                Give an alert to application
               ------------------------------ */
            SubsCallbackExpirationAlertEv(pSubs);
        }
    }
    else
    {
        /* --------------------------------------------------------------------
            Notifier, or subsciber that don't want to be alerted (AlertTimeout=0 )
            or Subscriber that was already alerted.
           --------------------------------------------------------------------*/
        SubsAlertTimerRelease(pSubs);
        SubsCallbackExpirationEv(pSubs);
    }

    SubsUnLockMsg(pSubs);

    return RV_FALSE;
}

/***************************************************************************
 * SubsNoNotifyTimerHandleTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever a noNotify timer expires.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The subscription this timer was called for.
 ***************************************************************************/
 RvBool SubsNoNotifyTimerHandleTimeout(IN void *pContext,
                                       IN RvTimer *timerInfo)
{
    Subscription        *pSubs;

    pSubs = (Subscription*)pContext;
    if (RV_OK != SubsLockMsg(pSubs))
    {
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pSubs->noNotifyTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc  ,
            "SubsNoNotifyTimerHandleTimeout - Subs 0x%p: no notify timer expired but was already released. do nothing...",
            pSubs));
        
        SubsUnLockMsg(pSubs);
        return RV_FALSE;
    }

    RvLogDebug((pSubs->pMgr)->pLogSrc ,((pSubs->pMgr)->pLogSrc ,
        "SubsNoNotifyTimerHandleTimeout - Subscription 0x%p: timer expired on state %s. terminate subscription",
        pSubs, SubsGetStateName(pSubs->eState)));

    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pSubs->noNotifyTimer);
    SubsNoNotifyTimerRelease(pSubs);

    SubsTerminate(pSubs, RVSIP_SUBS_REASON_NO_NOTIFY_TIME_OUT);
    SubsUnLockMsg(pSubs);

    return RV_FALSE;
}

/***************************************************************************
* SubsCalculateAndSetAlertTimer
* ------------------------------------------------------------------------
* General: Sets the subscription's alert timer, according to the value in
*          SUBSCRIBE 2xx response, or NOTIFY request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
*          expiresVal - The expires val from the 2xx response, or notify request.
***************************************************************************/
RvStatus RVCALLCONV SubsCalculateAndSetAlertTimer(Subscription* pSubs,
                                                  RvInt32       expiresVal)
{
    RvInt32                  alertToSet;
    RvStatus rv;

    if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_SENT &&
       0 == expiresVal) 
    {
        /* If this is an initial SUBSCRIBE, and the expires value is 0, then this is a
           polling-resource case. in this case we will set the alert timer to 
           RVSIP_SUBS_EXTRA_PROTECTION_EXPIRES_VAL for protection:
           if the NOTIFY(terminated) will arrive (as defined in chapter 3.3.6 of RFC 3265), 
           then it will terminate the subscription as it should. 
           if a NOTIFY won't arrive at all, the no-notify timer will terminate the subscription.
           if NOTIFY(active) will arrive, the alert timer expiration will cause the 
           subscription termination. */
        alertToSet = RVSIP_SUBS_EXTRA_PROTECTION_EXPIRES_VAL*1000;
    }
    else /* the usual case */
    {
         /* set the alert timer:
            expiresVal * 1000  ---  Convert it to milliseconds.
            minus alertTimeout ---  To give user the expiration alert on time.*/

            alertToSet = (expiresVal*1000 - pSubs->alertTimeout);

            if(alertToSet < 0)
            {
                alertToSet = expiresVal*1000;
            }
            if (0 == alertToSet)
            {
                alertToSet = 1; /* one millisecond, so the expiration callback will be called asynchronyc */
            }
    }

    rv = SubsObjectSetAlertTimer(pSubs, alertToSet);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsCalculateAndSetAlertTimer - Subscription 0x%p - Couldn't set expiration timer",
            pSubs));
        return rv;
    }

    /* set the bWasAlerted to false, so an alert will be given when timer is expired */
    pSubs->bWasAlerted = RV_FALSE;
    return RV_OK;
}

/***************************************************************************
* SubsObjectSetAlertTimer
* ------------------------------------------------------------------------
* General: Sets the subscription's alert timer.
*          Gets the time in seconds, and
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
*          timeToSet - exact time to set timer for.
***************************************************************************/
RvStatus SubsObjectSetAlertTimer(Subscription* pSubs,
                                   RvInt32      timeToSet)
{
    RvStatus rv;

    SipCommonCoreRvTimerCancel(&pSubs->alertTimer);

    rv = SipCommonCoreRvTimerStartEx(&pSubs->alertTimer,
                pSubs->pMgr->pSelect,
                timeToSet,
                SubsAlertTimerHandleTimeout,
                pSubs);

    if (rv != RV_OK)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsObjectSetAlertTimer - Subscription 0x%p - No timer is available",
            pSubs));
        return RV_ERROR_OUTOFRESOURCES;
    }
    else
    {
        RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsObjectSetAlertTimer - Subscription 0x%p - timer 0x%p was set to %u milliseconds",
            pSubs, &pSubs->alertTimer, timeToSet));
    }
    return RV_OK;
}

/***************************************************************************
 * SubsAddTranscToList
 * ------------------------------------------------------------------------
 * General: Add SUBSCRIBE/REFER transaction to the subscription transactions list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        hTransc - Handle to the transaction to add to list.
 ***************************************************************************/
RvStatus RVCALLCONV SubsAddTranscToList(IN  Subscription*     pSubs,
                                         IN  RvSipTranscHandle hTransc)
{
    RLIST_ITEM_HANDLE  listItem;
    RvStatus          rv;

    rv = RLIST_InsertTail(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, (RLIST_ITEM_HANDLE *)&listItem);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsAddTranscToList - Subscription 0x%p - failed to insert transaction 0x%p to list",
            pSubs, hTransc));
        return rv;
    }
    *(RvSipTranscHandle*)listItem = hTransc;
    return RV_OK;
}

/***************************************************************************
 * SubsDetachTranscAndRemoveFromList
 * ------------------------------------------------------------------------
 * General: Detach and remove a given transaction from the subscription
 *          transaction list.
 *          The function is used for every sending of a response message,
 *          that also causes a subscription termination.
 *          detach is needed so when transaction is terminated, it won't
 *          inform the already terminated subscription.
 *          removing from list is needed, so subscription termination, won't
 *          terminate the transaction with it.
 *          if the function fail, then on of the following happen:
 *          1. detach failed - then the transaction is still in the list,
 *             and will be terminated with the subscription.
 *          2. the transaction is not in the list - nothing to do.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        hTransc - Handle to the transaction to add to remove.
 ***************************************************************************/
void RVCALLCONV SubsDetachTranscAndRemoveFromList(IN  Subscription*      pSubs,
                                                  IN  RvSipTranscHandle  hTransc)
{
    RvStatus rv;

    rv = SipTransactionDetachOwner(hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDetachTranscAndRemoveFromList: subs 0x%p - failed to detach transc 0x%p. response message might not be sent on TCP",
            pSubs, hTransc));
        RvSipTransactionTerminate(hTransc);
        return;
    }

    rv = SubsRemoveTranscFromList(pSubs, hTransc);
    if(rv != RV_OK)
    {
        /* did not find the transaction in the list, nothing to do... */
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDetachTranscAndRemoveFromList: subs 0x%p - failed to remove transc 0x%p",
            pSubs, hTransc));
    }

}
/***************************************************************************
 * SubsRemoveTranscFromList
 * ------------------------------------------------------------------------
 * General: Removes a given transaction from the subscription transaction list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        hTransc - Handle to the transaction to add to remove.
 ***************************************************************************/
RvStatus RVCALLCONV SubsRemoveTranscFromList(IN  Subscription*      pSubs,
                                              IN  RvSipTranscHandle  hTransc)
{
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    RvSipTranscHandle    *phTransc;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = 1000;

    /*get the first element on the list*/
    RLIST_GetHead(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, &pItem);

    /*cast it to a transaction handle pointer*/
    phTransc = (RvSipTranscHandle*)pItem;

    /*keep looking until the given transaction is found or the end of the list
      is reached*/
    while(phTransc != NULL && *phTransc != hTransc && safeCounter < maxLoops)
    {
        RLIST_GetNext(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, pItem, &pNextItem);
        /*terminate the transaction*/
        pItem = pNextItem;
        phTransc = (RvSipTranscHandle*)pItem;
        safeCounter++;
    }

    if (safeCounter == maxLoops)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsRemoveTranscFromList - subs 0x%p - Reached MaxLoops %d, inifinte loop",
                  pSubs,maxLoops));
        return RV_ERROR_UNKNOWN;
    }

    if(phTransc != NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsRemoveTranscFromList - subs 0x%p remove transc 0x%p from transc list",
                  pSubs,hTransc));

        RLIST_Remove(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList,pItem);

        return RV_OK;
    }
    else
    {
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsRemoveTranscFromList - subs 0x%p - transaction 0x%p not found on the list. already terminated",
                  pSubs,hTransc));
        return RV_ERROR_INVALID_HANDLE; /*element not found*/
    }
}



/***************************************************************************
 * SubsGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state. This function is being used
 *          by the HighAval function.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SubsGetStateName(
                                      IN  RvSipSubsState  eState)
{

    switch(eState)
    {
    case RVSIP_SUBS_STATE_IDLE:
        return SUBS_STATE_IDLE_STR;
    case RVSIP_SUBS_STATE_SUBS_SENT:
        return SUBS_STATE_SUBS_SENT_STR;
    case RVSIP_SUBS_STATE_REDIRECTED:
        return SUBS_STATE_REDIRECTED_STR;
    case RVSIP_SUBS_STATE_UNAUTHENTICATED:
        return SUBS_STATE_UNAUTHENTICATED_STR;
    case RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD:
        return SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD_STR;
    case RVSIP_SUBS_STATE_2XX_RCVD:
        return SUBS_STATE_2XX_RCVD_STR;
    case RVSIP_SUBS_STATE_REFRESHING:
        return SUBS_STATE_REFRESHING_STR;
    case RVSIP_SUBS_STATE_REFRESH_RCVD:
        return SUBS_STATE_REFRESH_RCVD_STR;
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:
        return SUBS_STATE_UNSUBSCRIBING_STR;
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
        return SUBS_STATE_UNSUBSCRIBE_RCVD_STR;
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
        return SUBS_STATE_UNSUBSCRIBE_2XX_RCVD_STR;
    case RVSIP_SUBS_STATE_SUBS_RCVD:
        return SUBS_STATE_SUBS_RCVD_STR;
    case RVSIP_SUBS_STATE_ACTIVATED:
        return SUBS_STATE_ACTIVATED_STR;
    case RVSIP_SUBS_STATE_TERMINATING:
        return SUBS_STATE_TERMINATING_STR;
    case RVSIP_SUBS_STATE_PENDING:
        return SUBS_STATE_PENDING_STR;
    case RVSIP_SUBS_STATE_ACTIVE:
        return SUBS_STATE_ACTIVE_STR;
    case RVSIP_SUBS_STATE_TERMINATED:
        return SUBS_STATE_TERMINATED_STR;
    case RVSIP_SUBS_STATE_MSG_SEND_FAILURE:
        return SUBS_STATE_MSG_SEND_FAILURE_STR;
    default:
        return SUBS_STATE_UNDEFINED_STR;
    }
}

/***************************************************************************
 * SubsGetStateEnum
 * ------------------------------------------------------------------------
 * General: Returns the enum of a given string state. This function
 *          is being used by the HighAval functions.
 * Return Value: The the state type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     strState - The state as string
 ***************************************************************************/
RvSipSubsState RVCALLCONV SubsGetStateEnum(
                                      IN  RvChar *strState)
{
    if (SipCommonStricmp(strState,SUBS_STATE_IDLE_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_IDLE;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_SUBS_SENT_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_SUBS_SENT;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_REDIRECTED_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_REDIRECTED;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_UNAUTHENTICATED_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_UNAUTHENTICATED;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_2XX_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_2XX_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_REFRESHING_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_REFRESHING;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_REFRESH_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_REFRESH_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_UNSUBSCRIBING_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_UNSUBSCRIBING;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_UNSUBSCRIBE_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_UNSUBSCRIBE_2XX_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_SUBS_RCVD_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_SUBS_RCVD;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_ACTIVATED_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_ACTIVATED;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_TERMINATING_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_TERMINATING;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_PENDING_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_PENDING;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_ACTIVE_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_ACTIVE;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_TERMINATED_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_TERMINATED;
    }
    else if (SipCommonStricmp(strState,SUBS_STATE_MSG_SEND_FAILURE_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_STATE_MSG_SEND_FAILURE;
    }

    return RVSIP_SUBS_STATE_UNDEFINED;
}

/***************************************************************************
 * SubsGetTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given Subs type. This function is being
 *          used by the HighAval function.
 * Return Value: The string with the type name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eType - The type as enum
 ***************************************************************************/
const RvChar *RVCALLCONV SubsGetTypeName(IN RvSipSubscriptionType eType)
{
    switch (eType)
    {
    case RVSIP_SUBS_TYPE_SUBSCRIBER:
        return SUBS_TYPE_SUBSCRIBER_STR;
    case RVSIP_SUBS_TYPE_NOTIFIER:
        return SUBS_TYPE_NOTIFIER_STR;
    default:
        return SUBS_TYPE_UNDEFINED_STR;
    }
}

/***************************************************************************
 * SubsGetTypeEnum
 * ------------------------------------------------------------------------
 * General: Returns the enum of a given string type. This function
 *          is being used by the HighAval functions.
 * Return Value: The the type enum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     strState - The type as string
 ***************************************************************************/
RvSipSubscriptionType RVCALLCONV SubsGetTypeEnum(IN RvChar *strType)
{
    if (SipCommonStricmp(strType,SUBS_TYPE_SUBSCRIBER_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_TYPE_SUBSCRIBER;
    }
    else if (SipCommonStricmp(strType,SUBS_TYPE_NOTIFIER_STR) == RV_TRUE)
    {
        return RVSIP_SUBS_TYPE_NOTIFIER;
    }

    return RVSIP_SUBS_TYPE_UNDEFINED;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * SubsGetChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SubsGetChangeReasonName(IN  RvSipSubsStateChangeReason  eReason)
{
    switch(eReason)
    {
    case RVSIP_SUBS_REASON_LOCAL_REJECT:
        return "Local Reject";
    case RVSIP_SUBS_REASON_LOCAL_FAILURE:
        return "Local Failure";
    case RVSIP_SUBS_REASON_REMOTE_REJECT:
        return "Remote Reject";
    case RVSIP_SUBS_REASON_481_RCVD:
        return "481 rcvd";
    case RVSIP_SUBS_REASON_UNAUTHENTICATED:
        return "Unauthenticated";
    case RVSIP_SUBS_REASON_NO_NOTIFY_TIME_OUT:
        return "No Notify Timeout";
    case RVSIP_SUBS_REASON_TRANSC_TIME_OUT:
        return "Transaction Timeout";
    case RVSIP_SUBS_REASON_TRANSC_ERROR:
        return "Transaction Error";
    case RVSIP_SUBS_REASON_NO_TIMERS:
        return "No Timers";
    case RVSIP_SUBS_REASON_NETWORK_ERROR:
        return "Network error";
    case RVSIP_SUBS_REASON_503_RECEIVED:
        return "503 received";
    case RVSIP_SUBS_REASON_GIVE_UP_DNS:
        return "Give Up DNS";
    case RVSIP_SUBS_REASON_CONTINUE_DNS:
        return "Continue DNS";
    case RVSIP_SUBS_REASON_NOTIFY_ACTIVE_SENT:
        return "Notify Active Sent";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_SENT:
        return "Notify Terminated Sent";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_DEACTIVATED_RCVD:
        return "Notify Terminated Deactiveted Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_PROBATION_RCVD:
        return "Notify Terminated Probation Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_REJECTED_RCVD:
        return "Notify Terminated Rejected Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_TIMEOUT_RCVD:
        return "Notify Terminated Timeout Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_GIVEUP_RCVD:
        return "Notify Terminated Giveup Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_NORESOURCE_RCVD:
        return "Notify Terminated No Resource Rcvd";
    case RVSIP_SUBS_REASON_NOTIFY_2XX_SENT:
        return "Notify 2xx Sent";
    case RVSIP_SUBS_REASON_NOTIFY_2XX_RCVD:
        return "Notify 2xx Rcvd";
    case RVSIP_SUBS_REASON_SUBS_TERMINATED:
        return "SubsTerminated";
    case RVSIP_SUBS_REASON_DIALOG_TERMINATION:
        return "Dialog Termination";
    case RVSIP_SUBS_REASON_UNDEFINED:
    default:
        return "Undefined";
    }
}

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/***************************************************************************
 * SubsFree
 * ------------------------------------------------------------------------
 * General: Free the subscription resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the Subscription to free.
 ***************************************************************************/
void RVCALLCONV SubsFree(IN  Subscription *pSubs)
{
    RvSipCallLegHandle     hCallLeg;
    RvSipCallLegState      eCallLegState;
    RvSipCallLegReferState eReferState;
    RLIST_ITEM_HANDLE      tempItemInCalllegList;
    RvStatus			   rv;

    pSubs->subsUniqueIdentifier = 0;

    hCallLeg = pSubs->hCallLeg;

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsFree - Subs 0x%p is destructed",
        pSubs));

    /*outbound message*/
    if(pSubs->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pSubs->hOutboundMsg);
        pSubs->hOutboundMsg = NULL;
    }

    /* terminate headers list. */
    if(pSubs->pReferInfo != NULL && pSubs->pReferInfo->hReferToHeadersList != NULL)
    {
        RvSipCommonListDestruct(pSubs->pReferInfo->hReferToHeadersList);
        pSubs->pReferInfo->hReferToHeadersList = NULL;
    }
    if(pSubs->hHeadersListToSetInInitialRequest != NULL)
    {
        RvSipCommonListDestruct(pSubs->hHeadersListToSetInInitialRequest);
        pSubs->hHeadersListToSetInInitialRequest = NULL;
    }

    /* Terminate subscribe transaction if needed */
    if(pSubs->hActiveTransc != NULL)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsFree - subscription 0x%p transaction is 0x%p. should be terminated before!!!",
            pSubs, pSubs->hActiveTransc));
    }

    /* we are going to free the notify list and the subscription.
    both seat on a shared memory of the subscription manager. -> lock the manager */
    RvMutexLock(&pSubs->pMgr->hMutex,pSubs->pMgr->pLogMgr);

    /*free the memory page allocated for this subscription*/
    RPOOL_FreePage(pSubs->pMgr->hGeneralPool,
                   pSubs->hPage);

    /*destruct the list of notifications*/
    if(pSubs->hNotifyList != NULL)
    {
        RLIST_ListDestruct(pSubs->pMgr->hNotifyListPool,pSubs->hNotifyList);
    }

    /*destruct the list of notifications*/
    if(pSubs->hTranscList != NULL)
    {
        RLIST_ListDestruct(pSubs->pMgr->hTranscHandlesPool,pSubs->hTranscList);
    }

    /*the subscription should be removed from the manager subscription list*/
    RLIST_Remove(pSubs->pMgr->hSubsListPool, pSubs->pMgr->hSubsList,
                (RLIST_ITEM_HANDLE)pSubs);

	/*fix bug 48771: We are taking the hItemInCallLegList value into a local variable since 
	  after the unlock of the subsMgr the sub is available for allocation and can be initialize*/
	tempItemInCalllegList = pSubs->hItemInCallLegList;
	
    RvMutexUnlock(&pSubs->pMgr->hMutex,pSubs->pMgr->pLogMgr);

    /* remove from call-leg's subscription list */
    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsFree - subscription 0x%p remove from call-leg 0x%p (hItem=0x%p)",
            pSubs, hCallLeg, pSubs->hItemInCallLegList));

	rv = SipCallLegSubsRemoveSubscription(hCallLeg, tempItemInCalllegList);
    if (RV_OK != rv)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsFree - subscription 0x%p transaction is 0x%p. SipCallLegSubsRemoveSubscription() failed",
            pSubs, pSubs->hActiveTransc));
    }

    
    
    RvSipCallLegGetCurrentState(hCallLeg, &eCallLegState);

    /* terminate call-leg if needed*/
    if(SipCallLegIsHiddenCallLeg(hCallLeg) == RV_TRUE)
    {
        /* terminate call-leg */
        if(eCallLegState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            /* hidden call-leg might be in state connected, when this is a refer hidden
               call-leg, and application works with old refer. in this case the call-leg
               is exposed to application, and application might terminate it with
               RvSipCallLegTerminate()... */
            SipCallLegTerminateIfPossible(hCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        }
        else
        {
            RvSipCallLegTerminate(hCallLeg);
        }
    }
    else
    {
        /* if there is no other subscription, and call-leg states are idles */
        if(SipCallLegSubsIsSubsListEmpty(hCallLeg)== RV_FALSE)
        {
            /* do not terminate the call-leg */
            return;
        }
        else
        {
            SipCallLegGetCurrentReferState(hCallLeg, &eReferState);

            if((eCallLegState == RVSIP_CALL_LEG_STATE_IDLE ||
                eCallLegState == RVSIP_CALL_LEG_STATE_UNDEFINED ||
                eCallLegState == RVSIP_CALL_LEG_STATE_DISCONNECTED) &&
                (eReferState == RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED ||
                eReferState == RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE))
            {
                /* terminate call-leg */
                RvSipCallLegTerminate(hCallLeg);
            }
            else if(eCallLegState == RVSIP_CALL_LEG_STATE_TERMINATED)
            {
                SipCallLegTerminateIfPossible(hCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            }
        }
    }
}

/***************************************************************************
* HandleSubsRequest
* ------------------------------------------------------------------------
* General: handle an incoming subscribe request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription.
*          hTransc - Handle to the subscribe transaction.
*          hMsg    - Handle to the subscribe request message.
***************************************************************************/
RvStatus RVCALLCONV HandleSubsRequest(IN Subscription*     pSubs,
                                       IN RvSipTranscHandle hTransc,
                                       IN RvSipMsgHandle    hMsg)
{
    RvStatus rv = RV_OK;
    RvInt32  expires;

    /* set expires and event parameters in subscription */
    rv = GetExpiresValFromMsg(hMsg, RV_FALSE, &expires);
    if(rv != RV_OK)
    {

        /* refresh and subscribe with no expires header -
           must use the correct default value for this event package.
           we set a very big value to the 'requested expires'.
           application should answer with the correct default value
           in RvSipSubsRespondAccept(). */

        expires = RVSIP_SUBS_EXPIRES_VAL_LIMIT;
    }

    if(expires != UNDEFINED && (RvUint32)expires > RVSIP_SUBS_EXPIRES_VAL_LIMIT)
    {
        expires = RVSIP_SUBS_EXPIRES_VAL_LIMIT;
    }
    if((expires >= 0) && (pSubs->eState == RVSIP_SUBS_STATE_IDLE))
    {
        /* initial subscribe */
        rv = HandleInitialSubsRequest(pSubs, hTransc, hMsg, expires);
    }
    else if(expires > 0)
    {
        rv = HandleRefreshSubsRequest(pSubs, hTransc, hMsg, expires);
    }
    else if(expires == 0)
    {
        rv = HandleUnsubscribeRequest(pSubs, hTransc, hMsg);
    }
    else
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleSubsRequest: Subscription 0x%p - expires<0 ???", pSubs));

    }

    if(rv != RV_OK) /* in case we fail to send an automatic response */
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleSubsRequest: Subscription 0x%p - failed to handle the request. terminate subs transaction", pSubs));
        RvSipTransactionTerminate(hTransc);
    }
    return rv;
}

/***************************************************************************
* SubsHandleResponseOnInitialSubscribe
* ------------------------------------------------------------------------
* General: handle a response for subscribe request in Subs_Sent state.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs        - Handle to the subscription
*          hMsg         - Handle to the response message.
*          responseCode - The response code received on the subscribe request.
*          bRefer       - TRUE if this is response on REFER request.
*          bAuthValid   - Relevant for 401/407 response. indicates if the challenge is
*                         valid or not.
***************************************************************************/
RvStatus RVCALLCONV SubsHandleResponseOnInitialSubscribe(
                                          Subscription*  pSubs,
                                          RvSipMsgHandle hMsg,
                                          RvUint16       responseCode,
                                          RvBool        bRefer,
                                          RvBool        bAuthValid)
{
    RvStatus rv = RV_OK;
    RvSipSubsState eNewState = RVSIP_SUBS_STATE_UNDEFINED;
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_UNDEFINED;

    if (responseCode >= 200 && responseCode < 300)
    {
        /* 2xx (200 or 202) */
        eNewState = RVSIP_SUBS_STATE_2XX_RCVD;

        if(pSubs->hHeadersListToSetInInitialRequest != NULL)
        {
            /* we had a list of headers to set in the initial request.
               now we can destruct it */
            RvLogDebug(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                    "SubsHandleResponseOnInitialSubscribe - Subs 0x%p - destructing hHeadersListToSetInInitialRequest 0x%p...",
                    pSubs, pSubs->hHeadersListToSetInInitialRequest));
            RvSipCommonListDestruct(pSubs->hHeadersListToSetInInitialRequest);
            pSubs->hHeadersListToSetInInitialRequest  = NULL;
        }

        /* set the alert timer */
        if(RV_FALSE == bRefer)
        {
            rv = SetAlertTimerFrom2xxResponse(pSubs, hMsg);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                    "SubsHandleResponseOnInitialSubscribe - Subscription 0x%p - Failed to set expiration timer",
                    pSubs));
                return rv;
            }
        }
        /* set the noNotify timer */
        rv = SubsSetNoNotifyTimer(pSubs);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                "SubsHandleResponseOnInitialSubscribe - Subscription 0x%p - Couldn't set NoNotify timer",
                pSubs));
        }
    }
#ifdef RV_SIP_AUTH_ON
    else if((responseCode == 401 || responseCode == 407) &&
             bAuthValid == RV_TRUE)
    {
        /* unauthenticated */

        eNewState = RVSIP_SUBS_STATE_UNAUTHENTICATED;
        eReason = RVSIP_SUBS_REASON_UNAUTHENTICATED;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
    else if (responseCode >= 300 && responseCode < 400)
    {
        /* 3xx */
        SipCallLegSetRedirectContact(pSubs->hCallLeg);

        eNewState = RVSIP_SUBS_STATE_REDIRECTED;

    }
    else if (responseCode >= 400)
    {
        /* other reject on initial subscribe terminate the subscription */
        if(pSubs->pReferInfo != NULL)
        {
            /* save the response code in refer subscription, so in case of
            refer wrappers, we will be able to inform application with corret reason */
            pSubs->pReferInfo->referFinalStatus = responseCode;
        }
        rv = SubsTerminate(pSubs, RVSIP_SUBS_REASON_REMOTE_REJECT);
        return rv;
    }

    SubsResponseRcvdStateChange(pSubs, eNewState, eReason);
	
    RV_UNUSED_ARG(bAuthValid)
    return rv;
}


/***************************************************************************
* SubsHandleResponseOnNotifyBefore2xx
* ------------------------------------------------------------------------
* General: handle a response for subscribe request in NotifyBefore2xxRcvd state.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription
*          hMsg - Handle to the response message.
*          responseCode - The response code received on the subscribe request.
*          bRefer - TRUE if the response is on a REFER request.
***************************************************************************/
RvStatus RVCALLCONV SubsHandleResponseOnNotifyBefore2xx(
                                              Subscription*  pSubs,
                                              RvSipMsgHandle hMsg,
                                              RvInt32       responseCode,
                                              RvBool        bRefer)
{
    RvStatus rv = RV_OK;
    RvSipSubsState eNewState;
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_UNDEFINED;

    if (responseCode >= 200 && responseCode < 300)
    {
        if(RV_FALSE == pSubs->bActivatedOn2xxBeforeNotifyState)
        {
            /* we already received notify(pending). did not received
            notify(active) yet --> new state is pending */
            eNewState = RVSIP_SUBS_STATE_PENDING;
            eReason = RVSIP_SUBS_REASON_UNDEFINED;
        }
        else
        {
            /* we already received notify(active)*/
            eNewState = RVSIP_SUBS_STATE_ACTIVE;
            eReason = RVSIP_SUBS_REASON_UNDEFINED;
        }

        /* set the alert timer */
        if(RV_FALSE == bRefer)
        {
            rv = SetAlertTimerFrom2xxResponse(pSubs, hMsg);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                    "SubsHandleResponseOnNotifyBefore2xx - Subscription 0x%p - Failed to set expiration alert timer",
                    pSubs));
                return rv;
            }
        }

        SubsResponseRcvdStateChange(pSubs, eNewState, eReason);
        return RV_OK;
    }
    else /* non 2xx */
    {
        /* error situation, notify request should not be received,
           if the subscribe request was rejected */
        RvLogWarning(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsHandleResponseOnNotifyBefore2xx - Subscription 0x%p - got %d response after notify was received",
            pSubs, responseCode));
        SubsTerminate(pSubs, RVSIP_SUBS_REASON_REMOTE_REJECT);
        return RV_OK;
    }

}

/***************************************************************************
* SubsHandleResponseOnRefresh
* ------------------------------------------------------------------------
* General: handle a response for subscribe request in Subs_Refreshing state.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
*          hMsg   - The response message.
*          responseCode - The response code received on the subscribe request.
*          bAuthValid   - Relevant for 401/407 response. indicates if the challenge is
*                         valid or not.
***************************************************************************/
RvStatus RVCALLCONV SubsHandleResponseOnRefresh(Subscription*  pSubs,
                                                 RvSipMsgHandle hMsg,
                                                 RvInt32       responseCode,
                                                 RvBool        bAuthValid)
{
    RvStatus rv = RV_OK;
    RvSipSubsState eNewState;
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_UNDEFINED;

    if(responseCode == 481)
    {
        /* terminate subscription */
        SubsTerminate(pSubs, RVSIP_SUBS_REASON_481_RCVD);
        return RV_OK;
    }
    else if (responseCode >= 200 && responseCode < 300) /* 2xx */
    {
        eNewState = RVSIP_SUBS_STATE_ACTIVE;

        /* set the alert timer */
        rv = SetAlertTimerFrom2xxResponse(pSubs, hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                "SubsHandleResponseOnRefresh - Subscription 0x%p - Couldn't set expiration timer",
                pSubs));
            return rv;
        }
    }
#ifdef RV_SIP_AUTH_ON
    else if((responseCode == 401 || responseCode == 407) &&
             bAuthValid == RV_TRUE)
    {
        eNewState = RVSIP_SUBS_STATE_ACTIVE;
        eReason = RVSIP_SUBS_REASON_UNAUTHENTICATED;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
    else
    {
        /* other rejects */
        eNewState = RVSIP_SUBS_STATE_ACTIVE;
        eReason = RVSIP_SUBS_REASON_REMOTE_REJECT;
    }

    SubsResponseRcvdStateChange(pSubs, eNewState, eReason);
	
    RV_UNUSED_ARG(bAuthValid)
    return rv;
}


/***************************************************************************
* SubsHandleResponseOnUnsubscribe
* ------------------------------------------------------------------------
* General: handle a response for unsubscribe request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
*          responseCode - The response code received on the subscribe request.
*          bAuthValid   - Relevant for 401/407 response. indicates if the challenge is
*                         valid or not.
***************************************************************************/
RvStatus RVCALLCONV SubsHandleResponseOnUnsubscribe(Subscription*  pSubs,
                                                    RvInt32       responseCode,
                                                    RvBool        bAuthValid)
{
    RvStatus rv = RV_OK;
    RvSipSubsState eNewState;
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_UNDEFINED;

    if(responseCode == 481)
    {
        /* terminate subscription */
        SubsTerminate(pSubs, RVSIP_SUBS_REASON_481_RCVD);
        return RV_OK;
    }
    else if (responseCode >= 200 && responseCode < 300) /* 2xx */
    {
        eNewState = RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD;
        rv = SubsSetNoNotifyTimer(pSubs);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                "SubsHandleResponseOnUnsubscribe - Subscription 0x%p - Couldn't set NoNotify timer",
                pSubs));
        }
    }
#ifdef RV_SIP_AUTH_ON
    else if((responseCode == 401 || responseCode == 407) &&
             bAuthValid == RV_TRUE)
    {
        eNewState = pSubs->eStateBeforeUnsubsWasSent;
        eReason = RVSIP_SUBS_REASON_UNSUBSCRIBE_UNAUTHENTICATED;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
    else
    {
        /* other rejects */
        eNewState = pSubs->eStateBeforeUnsubsWasSent;
        eReason = RVSIP_SUBS_REASON_UNSUBSCRIBE_REJECTED;
    }
    pSubs->eStateBeforeUnsubsWasSent = RVSIP_SUBS_STATE_UNDEFINED;

    SubsResponseRcvdStateChange(pSubs, eNewState, eReason);
	
    RV_UNUSED_ARG(bAuthValid)
    return rv;
}

/***************************************************************************
* GetExpiresValFromMsg
* ------------------------------------------------------------------------
* General: Gets the Expires header value from a message..
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hMsg        - The message containing the Expires header.
*          bIsNotify   - Indication whether this is a NOTIFY request. If it is
*                        NOTIFY the expires value is in the Subscription-State
*                        header. If it is SUBSCRIBE or 2xx to SUBSCRIBE, the
*                        expires value is in the Expires header
* Output:  pExpiresVal - The expires header value.
***************************************************************************/
RvStatus RVCALLCONV GetExpiresValFromMsg(IN   RvSipMsgHandle hMsg, 
										 IN   RvBool         bIsNotify,
										 OUT  RvInt32*       pExpiresVal)
{
    RvSipExpiresHeaderHandle			hExpires;
    RvSipHeaderListElemHandle			hListElem;
	RvSipSubscriptionStateHeaderHandle	hSubsState;
    RvStatus							rv = RV_OK;

    *pExpiresVal = UNDEFINED;

	if (bIsNotify == RV_FALSE)
	{
		hExpires = (RvSipExpiresHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
																	  RVSIP_HEADERTYPE_EXPIRES,
																	  RVSIP_FIRST_HEADER,
																	  &hListElem);
		if(hExpires != NULL)
		{
			rv = RvSipExpiresHeaderGetDeltaSeconds(hExpires, (RvUint32*)pExpiresVal);
		}
	}
	else
	{
		hSubsState = (RvSipSubscriptionStateHeaderHandle)RvSipMsgGetHeaderByType(
																hMsg, 
																RVSIP_HEADERTYPE_SUBSCRIPTION_STATE, 
																RVSIP_FIRST_HEADER, 
																&hListElem);
        if(hSubsState != NULL) 
        {
            *pExpiresVal = RvSipSubscriptionStateHeaderGetExpiresParam(hSubsState);
        }
    }

	if (rv == RV_OK && *pExpiresVal == UNDEFINED)
	{
		rv = RV_ERROR_NOT_FOUND;
	}

    return rv;
}

/***************************************************************************
* SubsFindSubsWithSameEventHeader
* ------------------------------------------------------------------------
* General: Search for a subscription with the same event header.
* Return Value: RV_TRUE if found a subscription with same Event header.
*               RV_FALSE if not.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hCallLeg - Handle to the call-leg holding the subscription list.
*          pOrigSubs - Pointer to the subscription that we wish to insert, that
*                     for this insert we verify that there is no other subscription
*                    with same event.
*          hEvent   - Handle to the Event header.
***************************************************************************/
RvBool RVCALLCONV SubsFindSubsWithSameEventHeader(RvSipCallLegHandle     hCallLeg,
                                                   Subscription*          pOrigSubs,
                                                   RvSipEventHeaderHandle hEvent)
{
    RvSipSubsHandle hSubs;
    RvStatus       rv;
    RvSipEventHeaderHandle hTempEvent;

    rv = RvSipCallLegGetSubscription(hCallLeg, RVSIP_FIRST_ELEMENT, NULL, &hSubs);

    while(hSubs != NULL && rv == RV_OK)
    {
        if (pOrigSubs->eSubsType == ((Subscription*)hSubs)->eSubsType)
        {
            hTempEvent = ((Subscription*)hSubs)->hEvent;
            if(RvSipEventHeaderIsEqual(hEvent, hTempEvent) == RV_TRUE)
            {
                return RV_TRUE;
            }
        }
        rv = RvSipCallLegGetSubscription(hCallLeg, RVSIP_NEXT_ELEMENT, hSubs, &hSubs);
    }

    return RV_FALSE;
}

/***************************************************************************
 * SubsAttachReferResultSubsToReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Sets the new subscription created by refer acceptness, to point to the
 *          refer subscription triple lock, and to point to the
 *          refer subscription that created it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSubs - The new subscription handle.
 *        hReferSubsGenerator - Handle to the refer subscription.
 *        pReferSubsTriplelock - pointer to the refer subscription triple lock
 ***************************************************************************/
RvStatus RVCALLCONV SubsAttachReferResultSubsToReferSubsGenerator(
                                            IN RvSipSubsHandle hSubs,
                                            IN RvSipSubsHandle hReferSubsGenerator,
                                            IN SipTripleLock*  pReferSubsTriplelock)
{
    Subscription* pSubs = (Subscription*) hSubs;

    /* before changing the call-leg triple lock pointing, protect the call-leg
    object by locking it's own object lock */
    RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    pSubs->hReferSubsGenerator = hReferSubsGenerator;

    /* the new subscription object will have the same lock as the refer subscription.
       note that we must set the new lock pointer in the new subscription's
       call-leg too!!! */
    pSubs->tripleLock = pReferSubsTriplelock;
    SipCallLegSetTripleLockPointer(pSubs->hCallLeg, pReferSubsTriplelock);

    RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * SubsDetachReferResultSubsFromReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Detach the subscription from the refer subscription that created it.
 *          The function does not(!)set the subscription triple lock pointer,
 *          to point back to its call-leg triple lock.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSubs - The new subscription handle.
 ***************************************************************************/
RvStatus RVCALLCONV SubsDetachReferResultSubsFromReferSubsGenerator(
                                            IN RvSipSubsHandle hSubs)
{
    Subscription* pSubs = (Subscription*) hSubs;
    if(NULL == pSubs)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

   RvMutexLock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    pSubs->hReferSubsGenerator = NULL;
    /*pSubs->tripleLock = SipCallLegGetMutexHandle(pSubs->hCallLeg);*/

    RvMutexUnlock(&pSubs->subsLock,pSubs->pMgr->pLogMgr);

    return RV_OK;
}

/*-----------------------------------------------------------------------
        D N S   A P I
 ------------------------------------------------------------------------*/
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
 /***************************************************************************
 * SubsDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, delete the sending of the message, and
 *          change state of the state machine back to previous state.
 *          You can use this function for a subscribe and notify requests.
 *          For a SUBSCRIBE messages, use it if state was changed to
 *          MSG_SEND_FAILURE state. (in this case set hNotify to be NULL).
 *          Use this function for a NOTIFY request, if you got MSG_SEND_FAILURE
 *          status in RvSipSubsNotifyEv event. In this case,
 *          you should supply the notify handle.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs - Handle to the subscription that sent the request.
 *          hNotify  - Handle to the notify object, in case of notify request.
 ***************************************************************************/
RvStatus RVCALLCONV SubsDNSGiveUp (IN  Subscription* pSubs)
{
    RvStatus rv = RV_OK;
    RvSipSubsState eNewState = RVSIP_SUBS_STATE_UNDEFINED;

    switch(pSubs->ePrevState)
    {
    case RVSIP_SUBS_STATE_SUBS_SENT:
        /* terminate subscription */
        rv = SubsTerminate(pSubs, RVSIP_SUBS_REASON_GIVE_UP_DNS);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSGiveUp: subscription 0x%p - Failed to terminate subscription",
                pSubs));
            return rv;
        }
        break;
    case RVSIP_SUBS_STATE_REFRESHING:
        eNewState = RVSIP_SUBS_STATE_ACTIVE;
        /* no break - continue to the next case */
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:
        /* terminate transaction and move back to state active */
		if (pSubs->hActiveTransc != NULL)
		{
			rv = RvSipTransactionTerminate(pSubs->hActiveTransc);
			if(rv != RV_OK)
			{
				RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
					"SubsDNSGiveUp: subscription 0x%p - Failed to terminate transaction 0x%p",
					pSubs, pSubs->hActiveTransc));
				return rv;
			}
			pSubs->hActiveTransc = NULL;
		}
        if(RVSIP_SUBS_STATE_UNDEFINED == eNewState)
            eNewState = pSubs->eStateBeforeUnsubsWasSent;
        SubsChangeState(pSubs, eNewState, RVSIP_SUBS_REASON_GIVE_UP_DNS);
        break;
    default:
        /* error */
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDNSGiveUp: subscription 0x%p - illegal prev state %s,",
            pSubs, SubsGetStateName(pSubs->ePrevState)));
        return RV_ERROR_UNKNOWN;
    }
    return rv;
}
/***************************************************************************
* SubsDNSContinue
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, create a new transaction with a new dns list.
*          After this function, user should call to RvSipSubsDNSReSendRequest
*          at the next step, to re-send the cloned transaction to the next ip.
*          You can use this function for a subscribe and notify requests.
*          For a SUBSCRIBE messages, use it if state was changed to
*          MSG_SEND_FAILURE state. (in this case set hNotify to be NULL).
*          Use this function for a NOTIFY request, if you got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event. In this case,
*          you should supply the notify handle.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs - Handle to the subscription that sent the request.
*          hNotify  - Handle to the notify object, in case of notify request.
***************************************************************************/
RvStatus RVCALLCONV SubsDNSContinue (IN  Subscription* pSubs)
{
    RvStatus rv = RV_OK;
    RvSipTranscHandle hNewTransc;
    RvBool           bRefer;
    RvInt32      expirationVal;
    SipTransactionMethod eMethod;

    switch(pSubs->ePrevState)
    {
    case RVSIP_SUBS_STATE_SUBS_SENT:
    case RVSIP_SUBS_STATE_REFRESHING:
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:

        rv = SipTransactionGetMethod(pSubs->hActiveTransc, &eMethod);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSContinue: subscription 0x%p - Failed to get method from transaction 0x%p. (status %d)",
                pSubs, pSubs->hActiveTransc, rv));
            return rv;
        }
        /* continueDNS*/
        rv = SipTransactionDNSContinue(pSubs->hActiveTransc,
                                        (RvSipTranscOwnerHandle)pSubs->hCallLeg,
                                        RV_FALSE,
                                        &hNewTransc);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSContinue: subscription 0x%p - Failed to continue DNS for transaction 0x%p. (status %d)",
                pSubs, pSubs->hActiveTransc, rv));
            return rv;
        }
        /* save new transaction in subscription transactions list,
           and set it as active transaction*/
        rv = SubsAddTranscToList(pSubs, hNewTransc);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSContinue - Subscription 0x%p - Failed to set new transaction 0x%p in subs transactions list (status %d)",
                pSubs, hNewTransc, rv));
            SipTransactionTerminate(hNewTransc);
            return rv;
        }
        pSubs->hActiveTransc = hNewTransc;

        bRefer = (SIP_TRANSACTION_METHOD_REFER == eMethod)? RV_TRUE:RV_FALSE;
        expirationVal = (RVSIP_SUBS_STATE_UNSUBSCRIBING==pSubs->ePrevState)?0:pSubs->expirationVal;
        
        rv = SubsPrepareSubscribeTransc(pSubs, expirationVal, bRefer, 
										RV_FALSE /* The transaction was already created in DnsContinue by cloning*/);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSContinue - Subscription 0x%p - Failed to set params in new transaction 0x%p message (status %d)",
                pSubs, pSubs->hActiveTransc, rv));

            return rv;
        }

        return RV_OK;
    default:
        /* error */
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDNSContinue: subscription 0x%p - illegal prev state %s,",
            pSubs, SubsGetStateName(pSubs->ePrevState)));
        return RV_ERROR_UNKNOWN;
    }

}

/***************************************************************************
* SubsDNSReSendRequest
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the cloned transaction to the next ip
*          address, and change state of the state machine back to the sending
*          state.
*          You can use this function for a subscribe and notify requests.
*          For a SUBSCRIBE messages, use it if state was changed to
*          MSG_SEND_FAILURE state. (in this case set hNotify to be NULL).
*          Use this function for a NOTIFY request, if you got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event. In this case,
*          you should supply the notify handle.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs - Handle to the subscription that sent the request.
*          hNotify  - Handle to the notify object, in case of notify request.
***************************************************************************/
RvStatus RVCALLCONV SubsDNSReSendRequest (IN  Subscription* pSubs)
{
    RvStatus rv = RV_OK;
    RvBool   bRefer;

    switch(pSubs->ePrevState)
    {
    case RVSIP_SUBS_STATE_SUBS_SENT:
    case RVSIP_SUBS_STATE_REFRESHING:
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:
        bRefer = (NULL == pSubs->pReferInfo)?RV_FALSE:RV_TRUE;
        rv = SubsSendRequest(pSubs, pSubs->expirationVal, bRefer,
			                 RV_FALSE /* The transaction was already created in DnsContinue by cloning*/);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsDNSReSendRequest: subscription 0x%p - Failed to send new SUBSCRIBE. (status %d)",
                pSubs, rv));
            return rv;
        }

        /* change state back to previous state */
        SubsChangeState(pSubs, pSubs->ePrevState, RVSIP_SUBS_REASON_CONTINUE_DNS);
        return RV_OK;
    default:
        /* error */
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsDNSReSendRequest: subscription 0x%p - illegal prev state %s,",
            pSubs, SubsGetStateName(pSubs->ePrevState)));
        return RV_ERROR_UNKNOWN;
    }

}

#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT*/


/******************************************************************************
 * SubsForkedSubsInit
 * ----------------------------------------------------------------------------
 * General: The function initialize a Forked Subscription, copies data from
 *          Original Subscription into it and bring it to '2XX Received' state.
 * Return Value: RV_OK on success, error code, defined in RV_SIP_DEF.h otherwis
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs         - Pointer to the Forked Subscription object
 *            pSubsOriginal - Pointer to the Original Subscription object
 * Output:    none.
 *****************************************************************************/
RvStatus SubsForkedSubsInit(IN Subscription* pSubs,
                            IN Subscription* pSubsOriginal)
{
    RvStatus rv;
    SubsMgr  *pMgr;

    if (NULL==pSubs || NULL==pSubsOriginal ||
        NULL==pSubsOriginal->pMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    pMgr = pSubsOriginal->pMgr;

    pSubs->eSubsType = pSubsOriginal->eSubsType;

    pSubs->requestedExpirationVal = pSubsOriginal->requestedExpirationVal;
    pSubs->noNotifyTimeout = pSubsOriginal->noNotifyTimeout;
    pSubs->autoRefresh = pSubsOriginal->autoRefresh;
	
    if (NULL != pSubsOriginal->pReferInfo)
    {
        rv = SubsReferSetInitialReferParams(pSubs,
                                    pSubsOriginal->pReferInfo->hReferTo,
                                    pSubsOriginal->pReferInfo->hReferredBy);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SubsForkedSubsInit - hSubs=0x%p,hSubsOriginal=0x%p - failed SubsReferSetInitialReferParams() failed (rv=%d:%s)",
                pSubs, pSubsOriginal, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Overwrite created by SubsReferSetInitialReferParams() event package.
        Initial package doesn't contain Event ID parameter, which is set on
        SUBSCRIBE request sending. */
        rv = RvSipEventHeaderCopy(pSubs->hEvent, pSubsOriginal->hEvent);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SubsForkedSubsInit - hSubs=0x%p,hSubsOriginal=0x%p - failed RvSipEventHeaderCopy(hSubs=0x%p, hEvent=0x%p) (rv=%d:%s)",
                pSubs, pSubsOriginal, pSubs, pSubsOriginal->hEvent, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    if (NULL==pSubs->hEvent && NULL!=pSubsOriginal->hEvent)
    {
        rv = SubsSetEventHeader(pSubs,pSubsOriginal->hEvent);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "SubsForkedSubsInit - hSubs=0x%p,hSubsOriginal=0x%p - failed SubsSetEventHeader(hSubs=0x%p, hEvent=0x%p) (rv=%d:%s)",
                pSubs, pSubsOriginal, pSubs, pSubsOriginal->hEvent, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    pSubs->bEventHeaderExist = RV_TRUE;

    pSubs->hReferSubsGenerator = pSubsOriginal->hReferSubsGenerator;

    pSubs->pOriginalSubs = pSubsOriginal;

    pSubs->eState       = RVSIP_SUBS_STATE_2XX_RCVD;
    pSubs->ePrevState   = RVSIP_SUBS_STATE_IDLE; /* IDLE was used intently instead of SUBS_SENT state in order to catch differences between Forked Subscription and regular Subscription */

    return RV_OK;
}

/***************************************************************************
* SubsUpdateStateAfterReject
* ------------------------------------------------------------------------
* General: Update the subscription state machine, after responding with
*          non-2xx on SUBSCRIBE request (initial, refresh or unsubscribe).
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs   - the subscription
***************************************************************************/
void RVCALLCONV SubsUpdateStateAfterReject(Subscription* pSubs,
                                           RvSipSubsStateChangeReason eReason)
{
    if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_RCVD ||
       pSubs->eState == RVSIP_SUBS_STATE_IDLE) /* idle when the stack rejected initial request */
    {
        RvSipSubsStateChangeReason eReason2;
        /* we must detach the transaction, otherwise the response won't be sent on TCP,
           since the subscription termination will also terminate the transaction */
        SubsDetachTranscAndRemoveFromList(pSubs, pSubs->hActiveTransc);
        pSubs->hActiveTransc = NULL;

        if(eReason == RVSIP_SUBS_REASON_UNDEFINED)
        {
            eReason2 = RVSIP_SUBS_REASON_LOCAL_REJECT;
        }
        else
        {
            eReason2 = eReason;
        }
        SubsTerminate(pSubs, eReason2);
    }
    else if (pSubs->eState == RVSIP_SUBS_STATE_REFRESH_RCVD)
    {
        pSubs->hActiveTransc = NULL;
        SubsChangeState(pSubs,RVSIP_SUBS_STATE_ACTIVE, RVSIP_SUBS_REASON_LOCAL_REJECT);
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD)
    {
        pSubs->hActiveTransc = NULL;
        SubsChangeState(pSubs,pSubs->ePrevState, RVSIP_SUBS_REASON_LOCAL_REJECT);
    }
}

/***************************************************************************
 * SubsSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the message (request or response) that is going to be sent
 *          by the Subscription. You may set the message only in states where
 *          subscription may send a message.
 *          You can call this function before you call a control API functions
 *          that sends a message (such as RvSipSubsSubscribe()), or from
 *          the expirationAlert call-back function when a refresh message is
 *          going to be sent.
 *			IMPORTANT: There's no need to call this function for setting 
 *			           a RvSipMsgHandle which was returned from the 
 *					   complementary function RvSipSubsGetOutboundMsg().
 *
 *          NOTES: 
 *			1. The outbound message you set might be incomplete.
 *			2. The set outbound message might be modified by the SIP Stack 
 *			   if needed, for example: if the set outbound message is a 
 *			   response and the next message to be sent is a request. 
 *          3. You should use this function to add headers to the message before
 *             it is sent. 
 *			4. To view the complete message use event message to send.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs  - The subscription handle.
 *			  hMsg   - Handle to the set message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsSetOutboundMsg(IN  Subscription      *pSubscription,
                                       IN  RvSipMsgHandle     hMsg)
{
    RvStatus            rv;
	HRPOOL				hMsgPool      = SipMsgGetPool(hMsg);

	/* If the Subscription already owns an outbound message it should be freed first */ 
    if (NULL != pSubscription->hOutboundMsg)
    {
        RvSipMsgDestruct(pSubscription->hOutboundMsg); 
		pSubscription->hOutboundMsg = NULL; /* Reset the current outbound message */ 
    }
    
	/* Construct and copy the given message only if its' allocated in external memory pool */ 
	if (hMsgPool != pSubscription->pMgr->hGeneralPool)
	{
		rv = RvSipMsgConstruct(pSubscription->pMgr->hMsgMgr,
                               pSubscription->pMgr->hGeneralPool,
                               &pSubscription->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                       "SubsSetOutboundMsg - Subs 0x%p error in constructing message",
                      pSubscription));
            return rv;
        }
		rv = RvSipMsgCopy(pSubscription->hOutboundMsg,hMsg); 
		if (RV_OK != rv)
        {
			/* Free the just allocated message */ 
			RvSipMsgDestruct(pSubscription->hOutboundMsg); 
			pSubscription->hOutboundMsg = NULL; 
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
				"SubsSetOutboundMsg - Subs 0x%p error while copying message 0x%p to message 0x%p",
				pSubscription,hMsg,pSubscription->hOutboundMsg));
            return rv;
        }
    }
	else
	{
		pSubscription->hOutboundMsg = hMsg;
	}

    
    return RV_OK;
}


/******************************************************************************
 * SubsGetCbName
 * ----------------------------------------------------------------------------
 * General:  Print the name of callback represented by the bit turned on in the 
 *          mask results.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  MaskResults - the bitmap holding the callback representing bit.
 *****************************************************************************/
RvChar* SubsGetCbName(RvInt32 MaskResults)
{
    /* check which bit is turned on in the mask result, and print its name */
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_CREATED)))
        return "Subs Created Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_STATE_CHANGED)))
        return "Subs State Changed Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_EXPIRES_ALERT)))
        return "Subs Alert Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_EXPIRATION)))
        return "Subs Expiration Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_NOTIFY_CREATED)))
        return "Notify Created Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_CREATED_DUE_TO_FORKING)))
        return "Subs Created Due To Forking Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_NOTIFY_STATE_CHANGED)))
        return "Subs Notify State Changed Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_MSG_TO_SEND)))
        return "Subs Msg To Send Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_MSG_RCVD)))
        return "Subs Msg Rcvd Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_AUTH_CREDENTIALS_FOUND)))
        return "Subs Auth Credentials Found Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_AUTH_COMPLETED)))
        return "Subs Auth Completed Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_OTHER_URL_FOUND)))
        return "Subs Other Url Found Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_FINAL_DEST_RESOLVED)))
        return "Subs Final Dest Resolved Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_NEW_CONN_IN_USE)))
        return "New Conn In Use Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_SIGCOMP_NOT_RESPONDED)))
        return "SigComp Not Responded Ev";
    if(0 < (BITS_GetBit((void*)&MaskResults, CB_SUBS_REFER_NOTIFY_READY)))
        return "Notify Ready Ev";
    else
        return "Unknown";
}



/*-----------------------------------------------------------------------
       S T A T I C     F U N C T I O N S
 ------------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * SubsLock
 * ------------------------------------------------------------------------
 * General: Locks subscription.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs        - pointer to subscription to be locked
 *            pMgr        - pointer to subs manager
 *            hLock        - subs lock
 ***************************************************************************/
static RvStatus RVCALLCONV SubsLock(IN Subscription        *pSubs,
                                     IN SubsMgr            *pMgr,
                                     IN SipTripleLock        *tripleLock,
                                     IN    RvInt32            identifier)
{

    if ((pSubs == NULL) || (tripleLock == NULL) || (pMgr == NULL))
    {
        return RV_ERROR_BADPARAM;
    }


    RvMutexLock(&tripleLock->hLock,pMgr->pLogMgr); /*lock the subscription*/

    if (pSubs->subsUniqueIdentifier != identifier ||
        pSubs->subsUniqueIdentifier == 0)
    {
        /*The subscription is invalid*/
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SubsLock - Subs 0x%p: subscription object was destructed",
            pSubs));
        RvMutexUnlock(&tripleLock->hLock,pMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}

/***************************************************************************
 * SubsUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks a given lock.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLogMgr - The log mgr pointer
 *            hLock   - A lock to unlock.
 ***************************************************************************/
static void RVCALLCONV SubsUnLock(IN  RvLogMgr  *pLogMgr,
                                  IN  RvMutex   *hLock)
{
   RvMutexUnlock(hLock,pLogMgr); /*unlock the call-leg*/
}

/***************************************************************************
 * SubsTerminateUnlock
 * ------------------------------------------------------------------------
 * General: Unlocks processing and subscription locks. This function is called
 *          to unlock the locks that were locked by the LockMsg() function.
 *          The UnlockMsg() function is not called because in this stage the
 *          subscription object was already destructed.
 * Return Value: - RV_Succees
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pLogMgr            - The log manger pointer.
 *           pLogSrc            - The log source for printing in this function.
 *           pSubsForPrint      - pointer to the subscription that was already
 *                                destructed, the pointer is needed for log only.
 *           tripleLockForPrint - pointer to the subs triple lock that was already
 *                                destructed, the pointer is needed for log only.
 *           hObjLock           - The subscription lock.
 *           hProcessingLock    - The subscription processing lock
 ***************************************************************************/
static RvStatus RVCALLCONV SubsTerminateUnlock(IN  RvLogMgr  *pLogMgr,
                                               IN  RvLogSource* pLogSrc,
                                               IN  void      *pSubsForPrint,
                                               IN  void      *tripleLockForPrint,
                                               IN  RvMutex   *hObjLock,
                                               IN  RvMutex   *hProcessingLock)
{
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((tripleLockForPrint,pSubsForPrint,pLogSrc));
#endif

    RvLogSync(pLogSrc ,(pLogSrc ,
             "SubsTerminateUnlock - UnLocking subscription 0x%p: Triple Lock 0x%p (SubsTerminateUnlock:hObjLock=0x%p: hProcessingLock=0x%p) ",
             pSubsForPrint, tripleLockForPrint, hObjLock, hProcessingLock));
    if (hObjLock != NULL)
    {
        RvMutexUnlock(hObjLock,pLogMgr);
    }

    if (hProcessingLock != NULL)
    {
        RvMutexUnlock(hProcessingLock,pLogMgr);
    }
    return RV_OK;
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
* HandleInitialSubsRequest
* ------------------------------------------------------------------------
* General: handle an incoming initial subscribe request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription
*          hTransc - Handle to the subscribe transaction.
*          hMsg    - Handle to the subscribe request message.
*          expires - expires value in the refresh request.
***************************************************************************/
static RvStatus RVCALLCONV HandleInitialSubsRequest(IN Subscription*     pSubs,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RvSipMsgHandle    hMsg,
                                                    IN RvInt32          expires)
{
    RvStatus rv;
    RvSipSubsStateChangeReason eReason;

    pSubs->hActiveTransc = hTransc;
    if(pSubs->pReferInfo == NULL)
    {
        /* this is non-refer subscription */
        pSubs->requestedExpirationVal = expires;
        rv = SaveEventFromMsgInSubs(pSubs, hMsg);
        eReason = RVSIP_SUBS_REASON_UNDEFINED;
    }
    else /* this is a REFER request */
    {
        rv = SubsReferAnalyseSubsReferToHeader(pSubs);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "HandleInitialSubsRequest: subs 0x%p - failed in SubsReferAnalyseSubsReferToHeader (rv=%d)",
                pSubs, rv));
            /* continue with message processing, even though analysis failed */
        }
        eReason = SubsReferDecideOnReferRcvdReason(pSubs,hMsg);
    }

    SubsChangeState(pSubs, RVSIP_SUBS_STATE_SUBS_RCVD, eReason);
    return RV_OK;
}

/***************************************************************************
* HandleRefreshSubsRequest
* ------------------------------------------------------------------------
* General: handle an incoming refresh subscribe request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription
*          hTransc - Handle to the refresh transaction.
*          hMsg    - Handle to the subscribe request message.
*          expires - expires value in the refresh request.
***************************************************************************/
static RvStatus RVCALLCONV HandleRefreshSubsRequest(IN Subscription     *pSubs,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RvSipMsgHandle    hMsg,
                                                    IN RvInt32           expires)
{
    RvStatus rv;

    RV_UNUSED_ARG(hMsg);
    pSubs->requestedExpirationVal = expires;

    switch(pSubs->eState)
    {
    case RVSIP_SUBS_STATE_ACTIVE:
        pSubs->hActiveTransc = hTransc;
        SubsChangeState(pSubs, RVSIP_SUBS_STATE_REFRESH_RCVD,  RVSIP_SUBS_REASON_UNDEFINED);
        break;

    case RVSIP_SUBS_STATE_SUBS_RCVD:
    case RVSIP_SUBS_STATE_PENDING:
    case RVSIP_SUBS_STATE_ACTIVATED:
    case RVSIP_SUBS_STATE_REFRESH_RCVD:
    case RVSIP_SUBS_STATE_TERMINATING:
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
    case RVSIP_SUBS_STATE_TERMINATED:
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleRefreshSubsRequest -  Subs 0x%p - Illegal refresh request at state %s rejecting with 400",
            pSubs, SubsGetStateName(pSubs->eState)));
        rv = RvSipTransactionRespond(hTransc,400,NULL); /*bad request*/
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "HandleRefreshSubsRequest -  Subs 0x%p - failed to send 400", pSubs));
            return rv;
        }
        break;
    default:
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleRefreshSubsRequest -  Subs 0x%p - Illegal state %s to get a refresh",
            pSubs, SubsGetStateName(pSubs->eState)));
    }

    return RV_OK;
}

/***************************************************************************
* HandleUnsubscribeRequest
* ------------------------------------------------------------------------
* General: handle an incoming unsubscribe request.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs   - Handle to the subscription.
*        hTransc - Handle to the unsubscribe transaction.
*        hMsg    - Handle to the subscribe request message.
***************************************************************************/
static RvStatus RVCALLCONV HandleUnsubscribeRequest(Subscription     *pSubs,
                                                    RvSipTranscHandle hTransc,
                                                    RvSipMsgHandle    hMsg)
{
    RvStatus rv;

    RV_UNUSED_ARG(hMsg);
    switch(pSubs->eState)
    {
    case RVSIP_SUBS_STATE_REFRESH_RCVD:
        /* reject the refresh and move to state unsubs rcvd */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleUnsubscribeRequest -  Subs 0x%p - unsubs request at state REFRESH_RCVD send 400 on refresh",
            pSubs));
        rv = RvSipTransactionRespond(pSubs->hActiveTransc,400,NULL); /*bad request*/
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "HandleUnsubscribeRequest -  Subs 0x%p - failed to send 400 on refresh",
                pSubs));
            /* this is the old refresh transaction, we must terminate it here,
            and we must not return failure, because this will terminate the unsubs transc */
            RvSipTransactionTerminate(pSubs->hActiveTransc);
            pSubs->hActiveTransc = NULL;
        }
        else
        {
            pSubs->hActiveTransc = NULL;
        }

        /* change eState to active, so in changeState function the prevState will be active
        and not refresh-rcvd, so if user will reject the unsubscirbe we won't move
        back to state refresh-rcvd */

        pSubs->eState = RVSIP_SUBS_STATE_ACTIVE;

        /* no break - continue to next case */
    case RVSIP_SUBS_STATE_ACTIVE:
    case RVSIP_SUBS_STATE_PENDING:
    case RVSIP_SUBS_STATE_ACTIVATED:
        pSubs->hActiveTransc = hTransc;
        SubsChangeState(pSubs, RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD, RVSIP_SUBS_REASON_UNDEFINED);
        break;

    case RVSIP_SUBS_STATE_TERMINATING:
        /* accept the unsubscribe */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleUnsubscribeRequest -  Subs 0x%p - unsubscribe request at state TERMINATING send 200",
            pSubs));
        rv = RvSipTransactionRespond(hTransc,200,NULL); /*bad request*/
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "HandleUnsubscribeRequest -  Subs 0x%p - failed to send 200", pSubs));
            return rv;
        }
        break;

    case RVSIP_SUBS_STATE_SUBS_RCVD:
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
    case RVSIP_SUBS_STATE_TERMINATED:
        /* reject with 400 */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleUnsubscribeRequest -  Subs 0x%p - unsubscribe request at state %s send 400",
            pSubs, SubsGetStateName(pSubs->eState)));
        rv = RvSipTransactionRespond(hTransc,400,NULL); /*bad request*/
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "HandleUnsubscribeRequest -  Subs 0x%p - failed to send 400", pSubs));
            return rv;
        }
        break;

    case RVSIP_SUBS_STATE_IDLE:
        /* SUBSCRIBE with expires 0 on IDLE state must be mapped to the regular
        HandleInitialSubsRequest function, as a 'fetch' request */

        RvLogExcep(pSubs->pMgr->pLogSrc, (pSubs->pMgr->pLogSrc,
            "HandleUnsubscribeRequest - Subs 0x%p - unsubscribe request at IDLE state, can't be!!!",
            pSubs));

            /* reject with 400 */
        /*
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "HandleUnsubscribeRequest -  Subs 0x%p - unsubscribe request at state %s send 400",
                    pSubs, SubsGetStateName(pSubs->eState)));
                rv = RvSipTransactionRespond(hTransc,400,NULL); / *bad request* /
                if(rv != RV_OK)
                {
                    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                        "HandleUnsubscribeRequest -  Subs 0x%p - failed to send 400", pSubs));
                    / * we terminate the transaction here, and do not return failure,
                       because we must reach the SubsTerminate() line * /
                    RvSipTransactionTerminate(hTransc);
                }
                else
                {
                    / * detach the transaction, otherwise the respond won't be sent on tcp, because
                       subscription termination will terminate the transaction as well.
                       (since the transaction is in the subscription transcList) * /
                    SubsDetachTranscAndRemoveFromList(pSubs,hTransc);
                }
                SubsTerminate(pSubs, RVSIP_SUBS_REASON_UNDEFINED);*/

        break;

    default:
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "HandleUnsubscribeRequest -  Subs 0x%p - Illegal state %s to get an unsubscribe",
            pSubs, SubsGetStateName(pSubs->eState)));
    }

    return RV_OK;
}

/***************************************************************************
 * SubsPrepareSubscribeTransc
 * ------------------------------------------------------------------------
 * General: Create a transaction for the subscribe request, and set the
 *          Event and Expires headers in it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription the user wishes to subscribe.
 *          expirationVal - if 0, this is an unsubscribe request. else, refresh. in seconds.
 *          bRefer - TRUE for refer request. FALSE otherwise.
 *          bCreateNewTransc - RV_TRUE if this is a new transaction. RV_FALSE
 *                             if the transaction was already created, and
 *                             resides in hActiveTransc
 ***************************************************************************/
static RvStatus RVCALLCONV SubsPrepareSubscribeTransc (IN  Subscription *pSubs,
                                                       IN  RvInt32       expirationVal,
                                                       IN  RvBool        bRefer,
													   IN  RvBool        bCreateNewTransc)
{
    RvSipMsgHandle  hMsg;
    RvStatus rv;

	if (bCreateNewTransc == RV_TRUE)
	{
		/*-----------------------
		  create a new transaction
		  -------------------------*/
		rv = SipCallLegCreateTransaction(pSubs->hCallLeg, RV_FALSE, &(pSubs->hActiveTransc));

		if (rv != RV_OK || NULL == pSubs->hActiveTransc)
		{
			pSubs->hActiveTransc = NULL;
			/*transc construction failed*/
			RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
					   "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to create transaction",pSubs));
			return rv;
		}

		/*----------------------------------
		  add transaction to subs transcList
		  ----------------------------------*/
		rv = SubsAddTranscToList(pSubs,pSubs->hActiveTransc);
		if(rv != RV_OK)
		{
			RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
						"SubsPrepareSubscribeTransc - Subscription 0x%p - failed to insert transaction 0x%p to list",
						pSubs, pSubs->hActiveTransc));
			return rv;
		}
	}

    /* create message inside the transaction */
    if(pSubs->hOutboundMsg == NULL)
    {
        rv = RvSipTransactionGetOutboundMsg(pSubs->hActiveTransc, &hMsg);
        if(rv != RV_OK || hMsg == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to get message to send from transaction",pSubs));
            return rv;
        }
    }
    else
    {
        hMsg = pSubs->hOutboundMsg;
        rv = SipTransactionSetOutboundMsg(pSubs->hActiveTransc, hMsg);
        if(RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to set outbound message in transaction",pSubs));
            return rv;
        }
        pSubs->hOutboundMsg = NULL;
    }

    if(pSubs->hHeadersListToSetInInitialRequest != NULL)
    {
        /* push headers to outbound message */
        rv = SipMsgPushHeadersFromList(hMsg, pSubs->hHeadersListToSetInInitialRequest);
        if(rv != RV_OK || hMsg == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to push headers from list 0x%p",
                pSubs, pSubs->hHeadersListToSetInInitialRequest));
            return rv;
        }
    }

    if(RV_FALSE == bRefer)
    {
        /*-----------------------------
        set event and expires header
        -----------------------------*/
        rv = SubsSetSubsParamsInMsg(pSubs, hMsg, expirationVal, RV_TRUE);
        if(rv != RV_OK || hMsg == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to set subs params in subscribe request message",pSubs));
            return rv;
        }
    }
    else /*refer*/
    {
        /*---------------------------------------------
        set event id in subscription and do not set
        event header in message.
        -----------------------------------------------*/
        rv = SubsReferSetRequestParamsInMsg(pSubs, pSubs->hActiveTransc, hMsg);
        if(rv != RV_OK || hMsg == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to set refer params in refer request",pSubs));
            return rv;
        }
    }

    /* set the referred-by header if exists (case of subscription created because of a
    refer request) */
    if(pSubs->hReferredByHeader != NULL)
    {
        void *tempHeader;
        RvSipHeaderListElemHandle hElem = NULL;

        rv = RvSipMsgPushHeader(hMsg, RVSIP_FIRST_HEADER, (void*)pSubs->hReferredByHeader,
                                RVSIP_HEADERTYPE_REFERRED_BY, &hElem, &tempHeader);
        if(RV_OK != rv)
        {
            RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to set referred-by header in request",pSubs));
        }
    }
    /*----------------------------------------------
    set subscription handle in transaction.
    -----------------------------------------------*/
    rv = SipTransactionSetSubsInfo(pSubs->hActiveTransc, (void*)pSubs);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsPrepareSubscribeTransc - Subscription 0x%p - failed to set subs info in transaction 0x%p",
            pSubs,pSubs->hActiveTransc));
        return rv;
    }

    return RV_OK;

}

/***************************************************************************
 * SubsSetSubsParamsInMsg
 * ------------------------------------------------------------------------
 * General: sets Event and Expires headers in any message (request or response),
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription the user wishes to subscribe.
 *          hMsg        - Handle to the message that will hold the new headers.
 *          expirationVal - value for Expires header. in seconds.
 *          bRequest    - TRUE if this is a request message. FALSE if response
 ***************************************************************************/
static RvStatus RVCALLCONV SubsSetSubsParamsInMsg (IN  Subscription      *pSubs,
                                                    IN  RvSipMsgHandle    hMsg,
                                                    IN  RvInt32          expirationVal,
                                                    IN  RvBool           bRequest)
{
    RvSipHeaderListElemHandle   hElem;
    void*                       hHeader;
    RvSipExpiresHeaderHandle    hExpires;
    RvInt32                    expireToSet;
    RvStatus                   rv;
    void*                       pTempHeader;

    /*------------------------------------------------
      set event (only for request) and expires header
      ------------------------------------------------*/
    if(bRequest == RV_TRUE && pSubs->hEvent != NULL)
    {
        /* before pushing event header, check that there is no other event header
        in this message */
        pTempHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_EVENT,
                                                 RVSIP_FIRST_HEADER,
                                                 RVSIP_MSG_HEADERS_OPTION_ALL,
                                                 &hElem);
        if(pTempHeader == NULL)
        {
            /* there is no event header. insert header to msg */
            rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER, (void*)pSubs->hEvent,
                                    RVSIP_HEADERTYPE_EVENT, &hElem, &hHeader);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetSubsParamsInMsg - Subscription 0x%p - Failed to push Event header",pSubs));
                return rv;
            }
        }
    }


	/*First the expireToSet should be the requested expiration val.*/
	expireToSet = expirationVal;

	/*If the expireToSet is -1 and this is a response we will put the first expiration value since response must include expires header.*/
	if(expirationVal == UNDEFINED && bRequest == RV_FALSE)
	{
		expireToSet = pSubs->requestedExpirationVal;
	}

    if(expireToSet != UNDEFINED)
    {
        /* before constructing expires header, check that there is no other expires header
        in this message */
        pTempHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_EXPIRES,
                                                 RVSIP_FIRST_HEADER,
                                                 RVSIP_MSG_HEADERS_OPTION_ALL,
                                                 &hElem);
        if(pTempHeader == NULL)
        {
            /* there is no expires header - construct one */
            rv = RvSipExpiresHeaderConstructInMsg(hMsg, RV_TRUE, &hExpires);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetSubsParamsInMsg - Subscription 0x%p - Failed to construct Expires header",pSubs));
                return rv;
            }

            rv = RvSipExpiresHeaderSetDeltaSeconds(hExpires, expireToSet);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsSetSubsParamsInMsg - Subscription 0x%p - Failed to set Expires value",pSubs));
                return rv;
            }
        }
    }

    return RV_OK;
}

/***************************************************************************
* SetAlertTimerFrom2xxResponse
* ------------------------------------------------------------------------
* General: Sets the subscription's alert timer according to the 2xx response
*          message parameters.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
*          hMsg - The 2xx msg.
***************************************************************************/
static RvStatus RVCALLCONV SetAlertTimerFrom2xxResponse(Subscription* pSubs,
                                                        RvSipMsgHandle hMsg)
{
    RvInt32                  expiresVal;
    RvStatus rv;

    /* get the expires value from message, and set the alertTimer */

    rv = GetExpiresValFromMsg(hMsg, RV_FALSE, &expiresVal);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SetAlertTimerFrom2xxResponse - Subscription 0x%p - Couldn't get value from expires header",
            pSubs));
        if (pSubs->expirationVal != UNDEFINED)
		{
			/* There is no expires header in the 2xx response. We will use the expires value that
			   was previously set to the subscription */
			expiresVal = pSubs->expirationVal;
		}
		else
		{
			/* There is no expires header in the 2xx response. There is also no expires value set
			   to the subscription. We use the extra protection expires value to make sure that the
			   subscription has an expiration timer */
			expiresVal = RVSIP_SUBS_EXTRA_PROTECTION_EXPIRES_VAL;
		}
    }
    else
    {
        /* set the expires in 2xx in subscription */
        pSubs->expirationVal = expiresVal;
    }

    rv = SubsCalculateAndSetAlertTimer(pSubs, expiresVal);
    return rv;
}

/***************************************************************************
* SubsSetNoNotifyTimer
* ------------------------------------------------------------------------
* General: Sets the subscription's noNotify timer.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
***************************************************************************/
RvStatus RVCALLCONV SubsSetNoNotifyTimer(Subscription* pSubs)
{
    RvStatus rv;

    if(pSubs->noNotifyTimeout == 0)
    {
        return RV_OK;
    }

    SipCommonCoreRvTimerCancel(&pSubs->noNotifyTimer);

    /* set noNotify timer */
    rv = SipCommonCoreRvTimerStartEx(&pSubs->noNotifyTimer,
                pSubs->pMgr->pSelect,
                pSubs->noNotifyTimeout,
                SubsNoNotifyTimerHandleTimeout,
                pSubs);

    if (rv != RV_OK)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsSetNoNotifyTimer - Subscription 0x%p - No timer is available",
            pSubs));
        return RV_ERROR_OUTOFRESOURCES;
    }
    else
    {
        RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsSetNoNotifyTimer - Subscription 0x%p - timer 0x%p was set to %u milliseconds",
            pSubs, &pSubs->noNotifyTimer, pSubs->noNotifyTimeout));
    }
    return RV_OK;
}

/***************************************************************************
* SaveEventFromMsgInSubs
* ------------------------------------------------------------------------
* General: Gets the Event header from a message, and set it in the subscription.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
*          hMsg - The msg containing Event header.
***************************************************************************/
static RvStatus RVCALLCONV SaveEventFromMsgInSubs(Subscription* pSubs, RvSipMsgHandle hMsg)
{
    RvSipEventHeaderHandle hEvent;
    RvSipHeaderListElemHandle hListElem;
    RvStatus rv;


    hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                                                            RVSIP_HEADERTYPE_EVENT,
                                                            RVSIP_FIRST_HEADER,
                                                            &hListElem);
    if(hEvent == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }

    if(pSubs->hEvent == NULL)
    {
        rv = RvSipEventHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                        pSubs->pMgr->hGeneralPool,
                                        pSubs->hPage,
                                        &(pSubs->hEvent));
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SaveEventFromMsgInSubs - subscription 0x%p - Failed to construct Event header (rv=%d)",
                pSubs,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /*copy information from the msg hEvent the the subscription's event header*/
    rv = RvSipEventHeaderCopy(pSubs->hEvent, hEvent);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SaveEventFromMsgInSubs - subscription 0x%p - Failed to set Event header - copy failed (rv=%d)",
            pSubs,rv));
        return RV_ERROR_OUTOFRESOURCES;

    }

    return RV_OK;
}

/***************************************************************************
 * SubsTerminateAllTransc
 * ------------------------------------------------------------------------
 * General: Go over the subscription transaction list, Terminate the transcation
 *          using the transaction API. The transaction is not removed from the
 *          list. It will be removed when the event terminated will be received.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs     - Pointer to the subscription.
 *        bOnlyFree - FALSE for setting the transaction in the termination queue
 *                    TRUE for destructing the transaction at once.
 ***************************************************************************/
static void RVCALLCONV SubsTerminateAllTransc(IN  Subscription*   pSubs,
                                              IN  RvBool          bOnlyFree)
{
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    RvSipTranscHandle    *phTransc;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = 1000;

	if(pSubs->hTranscList == NULL)
	{
		return;
	}


	RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsTerminateAllTransc - Subs 0x%p - terminating all its transactions."
             ,pSubs));

    
	
	/*get the first item from the list*/
    RLIST_GetHead(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, &pItem);

    phTransc = (RvSipTranscHandle*)pItem;

    /*go over all the transactions*/
    while(phTransc != NULL && safeCounter < maxLoops)
    {
        RLIST_GetNext(pSubs->pMgr->hTranscHandlesPool,
                      pSubs->hTranscList,
                      pItem,
                      &pNextItem);
        if(bOnlyFree == RV_FALSE)
        {
            RvStatus rv;
            RvSipTranscOwnerHandle hOwnerHandle;

            /*Terminate the transaction*/
            RvSipTransactionTerminate(*phTransc);
            /* If the Subscription was not set yet as an Transaction's owner,
            remove the Transaction from the Subscriptions hTranscList now,
            because TERMINATED state, raised by transcation, will never reach
            the Subscription */
            rv = RvSipTransactionGetOwner(*phTransc, &hOwnerHandle);
            if (RV_OK != rv)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsTerminateAllTransc - Subsg 0x%p - failed to get owner of Transc 0x%p",
                    pSubs, *phTransc));
                hOwnerHandle = NULL;
                /* don't return here */
            }
            if (hOwnerHandle != (RvSipTranscOwnerHandle)pSubs->hCallLeg)
            {
                RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsTerminateAllTransc - subs 0x%p remove transc 0x%p from transc list",
                    pSubs, *phTransc));
                RLIST_Remove(pSubs->pMgr->hTranscHandlesPool, pSubs->hTranscList, pItem);
            }
        }
        else
        {
            SipTransactionTerminate(*phTransc);
            if(*phTransc == pSubs->hActiveTransc)
            {
                pSubs->hActiveTransc = NULL;
            }
        }
        pItem = pNextItem;
        phTransc = (RvSipTranscHandle*)pItem;
        safeCounter++;
    }
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsTerminateAllTransc - Subsg 0x%p - Reached MaxLoops %d, inifinte loop",
                  pSubs,maxLoops));
    }

}

/***************************************************************************
 * SubsResponseRcvdStateChange
 * ------------------------------------------------------------------------
 * General: Used to change the state of a subscription as a result of a response
 *          receipt.  
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs         - Pointer to the subscription.
 *        hStoredTransc - Handled to the stored transaction. The active transaction
 *                        will be reset only if it is equivalent to the stored
 *                        transaction.
 ***************************************************************************/
static void RVCALLCONV SubsResponseRcvdStateChange(
										IN Subscription*				pSubs,
										IN RvSipSubsState				eNewState,
										IN RvSipSubsStateChangeReason	eReason)
{
	RvSipTranscHandle  hStoredTransc;

	/* We store the active transaction. This is important since within the event
	   callback, the application can call Refresh() or Unsubscribe() that would change
	   the active transaction */
	hStoredTransc = pSubs->hActiveTransc;
	
	/* Invoke the state change callback */
	SubsChangeState(pSubs, eNewState, eReason);
	
	/* We check if the current active transaction was the same before invoking the callback */
	if (pSubs->hActiveTransc == hStoredTransc)
	{
		/* If it was the same, the active transaction is no longer required and is being reset */
		pSubs->hActiveTransc = NULL;
	}
}

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

