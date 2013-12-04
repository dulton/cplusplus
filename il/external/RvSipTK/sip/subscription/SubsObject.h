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
 *                              <SubsObjects.h>
 *
 * This file defines the subscription and notification objects.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SUBS_OBJECTS_H
#define SUBS_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON

#include "rvmutex.h"
#include "RvSipSubscriptionTypes.h"

#include "AdsRlist.h"
#include "_SipCommonTypes.h"
#include "_SipTransportTypes.h"
#include "RvSipCommonTypes.h"
#include "RvSipCommonList.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
    
/* subsCbName - internal enum, to print the callback name preventing the application
   to terminate the subscription */
typedef enum
{
    CB_SUBS_UNDEFINED = -1,
    CB_SUBS_CREATED,
    CB_SUBS_STATE_CHANGED,
    CB_SUBS_EXPIRES_ALERT,
    CB_SUBS_EXPIRATION,
    CB_SUBS_NOTIFY_CREATED,
    CB_SUBS_CREATED_DUE_TO_FORKING,
    CB_SUBS_NOTIFY_STATE_CHANGED,
    CB_SUBS_MSG_TO_SEND,
    CB_SUBS_MSG_RCVD,
    CB_SUBS_AUTH_CREDENTIALS_FOUND,
    CB_SUBS_AUTH_COMPLETED,
    CB_SUBS_OTHER_URL_FOUND,
    CB_SUBS_FINAL_DEST_RESOLVED,
    CB_SUBS_NEW_CONN_IN_USE,
    CB_SUBS_SIGCOMP_NOT_RESPONDED,
    CB_SUBS_REFER_NOTIFY_READY
}subsCbName;

/* ReferInfo
 * --------------------------------------------------------------------------
 * ReferInfo structure hold all parameters that relates to a refer subscription.
 * This structure is allocated on the subscription page, when a new refer
 * subscription is created.
 *
 * hReferTo         - Handle to the Refer-To header.
 * hReferredBy      - Handle to the Referred-By header
 * bFirstReferSubs  - Is this the first refer subscription for this dialog.
 * eReferResultObjType - Type of the object that is created when application
 *                    accepts a REFER request. (will be call-leg or subscription,
 *                    according to the method parameter in the Refer-To URI.)
 * pReferResultObj  - Pointer to the object that is created when application
 *                    accepts a REFER request.
 * referFinalStatus - The final status of the refer process that should be set
 *                    in the body of the outgoing final NOTIFY.
 * hReferToHeadersList - A list of headers that were found in the Refer-To url.
 *                    this list will be set in the new result obj, so those headers
 *                    will be set to the new request it will send.
*/

typedef struct
{
    RvSipReferToHeaderHandle    hReferTo;
    RvSipReferredByHeaderHandle hReferredBy;
    RvBool                      bFirstReferSubsInDialog;
    RvSipCommonStackObjectType  eReferResultObjType;
    void*                       pReferResultObj;
    RvUint16                    referFinalStatus;
    RvSipCommonListHandle       hReferToHeadersList;

}ReferInfo;

/* Subscription
 * --------------------------------------------------------------------------
 * Subscription represents a SIP Subscription as defined in draft-ietf-sip-events-05.
 * Subscription is a statefull object and has a set of states associated
 * with it.
 *
 * terminationInfo - Pointer to the next object in termination queue.
 * eTerminationReason - The reason for the termination.
 * pMgr        --   Pointer to the manager this subscription belongs to.
 * hPage       --   memory page used for dynamic memory allocation. for example
 *                  for Event header.
 * hAppSubs    --   Application handle to this subscription.
 * hCallLeg    --   Handle to the call-leg related to this subscription.
 * eState      --   The subscription state parameter. This parameter allows only a get function.
 * ePrevState  --   previous state, for use of giveupDNS function.
 * eLastState  --   The last state that was informed to application. for use in printing the
 *                  current state to log, on new state TERMINATED (prevent the terminated->terminated print)
 * eStateBeforeUnsubsWasSent -- Saves the state of the subscriber before sending an unsubscribe
 *                  request. if the unsubscribe is rejected, the subscription state will
 *                  change back to this state. (we need this parameter, and we cannot use
 *                  the ePrevState, because MSG_SEND_FAILURE state on the unsubscribe request,
 *                  will overwrite this state).
 * bActivatedOn2xxBeforeNotifyState -- relevant only for 2xxBeforeNotifyRcvd state.
 *                  this parameter helps us to decide whether to change state to active
 *                  or pending, when the 2xx finally received. (if we notifier sent 202,
 *                  and then notify(pending) and notify(active), and for some reason,
 *                  the 202 is received only after subscriber already sent 200 on the
 *                  notify(active), then the subscription state should be changed to
 *                  active and not pending, even though it got 202 and not 200.)
 * bOutOfBand  --   Indicates if this is an out of Band subscription or not.
 *                    (out of Band means that it was created with no subscribe request)
 * hEvent      --   The subscription Event header.
 * bEventHeaderExist - If false, the event default value is PINT.
 * eSubsType   --   The type parameter indicates whether the subscription represents
 *                  the subscriber or the notifier side of the subscription.
 * hOutboundMsg --  Handle to the outbound message.
 * hTranscList  --   Handle to list of transaction handled by this subscription.
 * hActiveTransc --  keep the transaction for an incoming subscribe request
 * hNotifyList  --   Handle to list of notification objects handled by this subscription.
 * alertTimeout --   Defines the time in milliseconds(!) before subscription expiration to give an alert to user.
 * hAlertTimer  --   The timer for the alert before expiration.
 * bWasAlerted  --   When the alert timer expires, this boolean indicates if this is the end of
 *                  the subscription, or time to send refresh.
 * expirationVal -- The value of subscription expiration seconds!!!
 * requestedExpirationVal -- The expires value in the subscribe request. if user accept this
 *                  request, and does not choose a different expiration value, this value is
 *                  set to the expirationVal parameter. in seconds!!!
 *
 *                  Is used also as an initial expiration value for Forked
 *                  Subscription, if it was not set neither in Original SUBSCRIBE
 *                  nor in NOTIFY for forked SUBSCRIBE.
 * autoRefresh --   Defines whether to send an automatic refresh request, when an alert is given.
 * hNoNotifyTimer -- noNotify timer is set when a 2xx response is received for first subscribe
 *                    or unsubscribe requests. this timer is released when the first notify request
 *                    is received . according to the enableNoNotifyTimer parameter os SubsMgr, we
 *                  will decide whether to set this timer or not.
 * noNotifyTimeout -- the timeout value for the no-notify timer.
 * subsEvHandlers - pointer to the subscription event handler struct
 * numOfNotifyObjs -- number of alive notify objects, related to this subscription. (this parameter
 *                  is needed for objects termination issue. the last notification object
 *                  (numOfNotifyObjs = 0) has to insert the subscription to the termination queue.
 * hItemInCallLegList -- hItem of the subscription in call-leg list.
 * subsLock --      subscription object own lock
 * tripleLock --    pointer to common for subscription & call-leg triple lock
 * subsUniqueIdentifier -- A unique identifier for this subscription.
 *                  It is generated randomly on subscription creation.
 *                  If it is 0, the subscription object is vacant.
 * pReferInfo -- Pointer to the refer parameters structure. relevant only for refer subscription.
 * parameters of subscription created by a refer request with method=subscribe/refer:
 * ----------------------------------------------------------------------------------
 * hReferredByHeader -- The referred-by header to insert to the first request, sent by this subs.
 * hHeadersListToSetInInitialRequest -- list of headers to be set to the first request,
 *                      sent by this subs. (the headers that were received in the refer-to url)
 * hReferSubsGenerator -- The refer subscription that generated this subscription.
 * pOriginalSubs       -- Pointer to the Original Subscription.
 *                        Is meaningful when the object is a Forked Subscription.
 * expectedCSeq      -- CSeq of the message, that is expected to be processed
 *                      by Forked Subscription. Is used in order to prevent
 *                      handling of NOTIFies before Active NOTIFY, which
 *                      causes Forked Subscription creating, handling.
 * -----------------------
 * version 5.0 parameters
 * -----------------------
 * cbBitmap          -- Bitmap that saves the callback information. 
 *                      When we enter a callback, we turn the callback matching bit on.  
 *                      When we leave a callback, we turn the callback matching bit off
 *                      This information can be used to block APIs that are forbidden to be
 *                      used in specific callbacks. 
 */
typedef struct Subs
{
    RvSipTransportObjEventInfo terminationInfo;
    RvSipSubsStateChangeReason eTerminationReason;

    struct SubsMgrStruct   *pMgr;

    HPAGE                  hPage;
    RvSipAppSubsHandle     hAppSubs;
    RvSipCallLegHandle     hCallLeg;
    RvSipTranscHandle      hActiveTransc;  /* keep the transaction for an incoming subscribe request */

    RvSipSubsState         eState;
    RvSipSubsState         ePrevState;
    RvSipSubsState         eLastState;
    RvSipSubsState         eStateBeforeUnsubsWasSent;
    RvBool                bActivatedOn2xxBeforeNotifyState;
    RvBool                bOutOfBand;
    RvSipEventHeaderHandle hEvent;
    RvBool                bEventHeaderExist;
    RvSipSubscriptionType  eSubsType;
    RvSipMsgHandle         hOutboundMsg;

    RLIST_HANDLE           hTranscList;
    RLIST_HANDLE           hNotifyList;

    SipTripleLock*        tripleLock;
    RvMutex               subsLock;
    RvInt32               subsUniqueIdentifier;

    RvInt32               alertTimeout;
    SipTimer              alertTimer;
    RvBool                bWasAlerted;

    RvInt32               expirationVal;
    RvInt32               requestedExpirationVal;

    RvInt32               noNotifyTimeout;

    RvBool                autoRefresh;
    SipTimer              noNotifyTimer;
    RvSipSubsEvHandlers   *subsEvHandlers;
#ifdef SUPPORT_EV_HANDLER_PER_OBJ
	RvSipSubsEvHandlers	   objEvHandlers;
#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/
    RvInt32               numOfNotifyObjs;
    RLIST_ITEM_HANDLE     hItemInCallLegList;

    ReferInfo             *pReferInfo;

    /* parameters of subscription created by a refer request with method=subscribe/refer:*/
    RvSipReferredByHeaderHandle hReferredByHeader;
    RvSipCommonListHandle       hHeadersListToSetInInitialRequest;
    RvSipSubsHandle             hReferSubsGenerator;

    struct Subs           *pOriginalSubs;
	SipCSeq				   expectedCSeq;
    RvUint16               createdRejectStatusCode;

     /* new parameters - version 5.0 */
    RvInt32                cbBitmap;
    RvUint32               retryAfter;
} Subscription;

/* Notification
 * --------------------------------------------------------------------------
 *
 * terminationInfo - Pointer to the next object in termination queue.
 * pSubs       --   Pointer to the subscription this notification belongs to.
 * hAppNotify  --   Application handle to this notification.
 * hTransc     --   Handle to the transaction related to this notification.
 * eSubstate   --   The sub-state from subscription-state header of the notify message.
 *                  This parameter is for updating the subscription state
 *                  machine if needed.
 * expiresParam--   The expires parameter from subscription-state header.
 * eReason     --   The reason from subscription-state header of notify message,
 *                  in case that eSubstate is 'terminated'
 * strReason   --   offset to a reason string, in case that eReason is 'other'.
 *                  (The string is located on the subscription page).
 * eStatus     --   status of the notification.
 * hSubscriptionStateHeader - The subscription-State header to set in the NOTIFY request.
 * pfnNotifyEvHandler - pointer to the notifyEcHandler.
 */
typedef struct
{
    RvSipTransportObjEventInfo  terminationInfo;
    RvSipAppNotifyHandle        hAppNotify;
    Subscription*               pSubs;
    RvSipTranscHandle           hTransc;
    RvSipMsgHandle              hOutboundMsg;
    RvSipSubscriptionSubstate   eSubstate;
    RvInt32                     expiresParam;
    RvSipSubscriptionReason     eReason;
    RvInt32                     strReason;
    RvSipSubsNotifyStatus       eStatus;
    RvSipSubsNotifyReason       eTerminationReason;
    /*RvSipSubscriptionStateHeaderHandle hSubscriptionStateHeader;*/
    RvSipSubsNotifyEv          *pfnNotifyEvHandler;

     /* new parameters - version 5.0 */
    RvUint32               retryAfter;
} Notification;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
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
                                 IN  RvSipSubsStateChangeReason eReason) ;

/***************************************************************************
 * SubsSetInitialParams
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with mandatory parameters: To and From headers
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
 *                    (This is not necessarily the final expires value. Notifier may
 *                    decide of a shorter expires value in the 2xx response.)
 *            hEvent - Handle to an Event header. this header identify the subscription.
 ***************************************************************************/
RvStatus SubsSetInitialParams(  IN  Subscription*          pSubscription,
                                 IN  RvSipPartyHeaderHandle hFrom,
                                 IN  RvSipPartyHeaderHandle hTo,
                                 IN  RvInt32               expiresVal,
                                 IN  RvSipEventHeaderHandle hEvent);

/***************************************************************************
 * SubsSetInitialDialogParams
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with dialog parameters: To, From, remote
 *          contact and local contact headers.
 *          This function is relevant only for subscription that was created
 *          outside of a call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
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
                                     IN  RvSipAddressHandle       hRemoteContactAddr);

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
                                   IN  HPAGE                 hSubsPage);

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
                                    IN  RvBool      bRefer);

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
                                  IN  RvInt32   expirationVal);

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
 *          expirationVal - if 0, this is an unsubscribe request. else, refresh.
 *                      If UNDEFINED, the expires value from the subscribe request,
 *                      is set to the 2xx response.
 *          bRefer      - TRUE if this is sending of 202 on refer request.
 ***************************************************************************/
RvStatus RVCALLCONV SubsAccept (IN  Subscription *pSubs,
                                 IN  RvUint16      responseCode,
                                 IN  RvInt32      expirationVal,
                                 IN  RvBool       bRefer);

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
                                          IN  RvChar*      strReasonPhrase);

/***************************************************************************
* SubsReject
* ------------------------------------------------------------------------
* General: Reject a subscription. a response message is send
*          and subscription moves to terminated state.
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSubs       - Pointer to the subscription the user wishes to subscribe.
*          statusCode  - statusCode for the response message.
*          strReasonPhrase - reason phrase for this response message. if NULL, default is set.
***************************************************************************/
RvStatus RVCALLCONV SubsReject (IN  Subscription *pSubs,
                                 IN  RvUint16     statusCode,
                                 IN  RvChar*      strReasonPhrase);

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
*          eAlgorithm -     Enumeration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
*                           the algorithm value is taken from the the next argument.
*          strAlgorithm -   String of algorithm. this paraemeters will be set only if
*                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
*          eQop -           Enumeration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
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
                                        IN  RvChar              *strQop);
#endif /* #ifdef RV_SIP_AUTH_ON */

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
									 IN  RvBool			bCreateNewTransc);

/***************************************************************************
* SubsTerminate
* ------------------------------------------------------------------------
* General: Terminate a subscription, without sending any message.
*          subscription moves to state TERMINATED, and subscription is destructed,
*          unless manual termination is set ??????????????????????
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSubs       - Pointer to the subscription the user wishes to subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsTerminate (IN  Subscription *pSubs,
                                    IN  RvSipSubsStateChangeReason reason);

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
RvStatus RVCALLCONV SubsTerminateIfPossible (IN  Subscription *pSubs);

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
                                  RvInt32      timeToSet);

/***************************************************************************
* SubsSetNoNotifyTimer
* ------------------------------------------------------------------------
* General: Sets the subscription's noNotify timer.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - The subscription.
***************************************************************************/
RvStatus RVCALLCONV SubsSetNoNotifyTimer(IN Subscription* pSubs);

/***************************************************************************
 * SubsNoNotifyTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the noNotify timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransaction - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV SubsNoNotifyTimerRelease(IN Subscription *pSubs);
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
RvStatus RVCALLCONV SubsLockAPI(IN  Subscription*   pSubs);

/************************************************************************************
 * CallLegUnlockAPI
 * ----------------------------------------------------------------------------------
 * General: Unlocks subscription according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
void RVCALLCONV SubsUnLockAPI(IN  Subscription*   pSubs);

/************************************************************************************
 * SubsLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks subscription according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
RvStatus RVCALLCONV SubsLockMsg(IN  Subscription*   pSubs);

/************************************************************************************
 * SubsUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks subscription according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - pointer to the subscription.
***********************************************************************************/
void RVCALLCONV SubsUnLockMsg(IN  Subscription*   pSubs);
#else
#define SubsLockAPI(a) (RV_OK)
#define SubsUnLockAPI(a)
#define SubsLockMsg(a) (RV_OK)
#define SubsUnLockMsg(a)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/


/***************************************************************************
* HandleInitialSubsRequest
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
                                       IN RvSipMsgHandle    hMsg);

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
                                                    RvBool        bAuthValid);

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
                                              RvBool        bRefer);

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
*          bRefer       = TRUE if this is response on REFER request.
*          bAuthValid   - Relevant for 401/407 response. indicates if the challenge is
*                         valid or not.
***************************************************************************/
RvStatus RVCALLCONV SubsHandleResponseOnInitialSubscribe(
                                          Subscription*  pSubs,
                                          RvSipMsgHandle hMsg,
                                          RvUint16       responseCode,
                                          RvBool        bRefer,
                                          RvBool        bAuthValid);

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
                                                 RvBool        bAuthValid);

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
RvStatus RVCALLCONV SubsDestruct (IN  Subscription *pSubs);

/***************************************************************************
 * SubsFree
 * ------------------------------------------------------------------------
 * General: Free the subscription resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the Subscription to free.
 ***************************************************************************/
void RVCALLCONV SubsFree(IN  Subscription *pSubs);

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
                                                  RvInt32       expiresVal);

/***************************************************************************
 * SubsAlertTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the alert timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - The subscription to release the timer from.
 ***************************************************************************/
void RVCALLCONV SubsAlertTimerRelease(IN Subscription *pSubs);

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
										 OUT  RvInt32*       pExpiresVal);

/*-----------------------------------------------------------------------
         G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * SubsSetEventHeader
 * ------------------------------------------------------------------------
 * General: Set the Event header associated with the subscription.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Event header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the subscription.
 *            hEvent - Handle to an application constructed event header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsSetEventHeader (
                                      IN  Subscription*           pSubs,
                                      IN  RvSipEventHeaderHandle  hEvent);

/***************************************************************************
 * SubsFindSubsWithSameEventHeader
 * ------------------------------------------------------------------------
 * General: Search for a subscription with the same event header.
 * Return Value: RV_TRUE if found a subscription with same Event header.
 *               RV_FALSE if not.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pOrigSubs - Pointer to the subscription that we wish to insert, that
 *                     for this insert we verify that there is no other subscription
 *                    with same event.
 *          hEvent   - Handle to the Event header.
 ***************************************************************************/
RvBool RVCALLCONV SubsFindSubsWithSameEventHeader(RvSipCallLegHandle     hCallLeg,
                                                   Subscription*          pOrigSubs,
                                                   RvSipEventHeaderHandle hEvent);

/***************************************************************************
 * SubsAttachReferResultSubsToReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Sets the new subscription created by refer acceptness, to point to the
 *          refer subscription triple lock, and to point to the
 *          refer subscription that created it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSubs                 - The new subscription handle.
 *        hReferSubsGenerator   - Handle to the refer subscription.
 *        pReferSubsTriplelock  - pointer to the refer subscription triple lock
 ***************************************************************************/
RvStatus RVCALLCONV SubsAttachReferResultSubsToReferSubsGenerator(
                            IN RvSipSubsHandle hSubs,
                            IN RvSipSubsHandle hReferSubsGenerator,
                            IN SipTripleLock*  pReferSubsTriplelock);

/***************************************************************************
 * SubsDetachReferResultSubsFromReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Detach the subscription from the refer subscription that generated it.
 *          The function sets the subscription triple lock pointer, to point back
 *          to its call-leg triple lock.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSubs - The new subscription handle.
 ***************************************************************************/
RvStatus RVCALLCONV SubsDetachReferResultSubsFromReferSubsGenerator(
                                            IN RvSipSubsHandle hSubs);

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
                                         IN  RvSipTranscHandle hTransc);

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
                                              IN  RvSipTranscHandle  hTransc);

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
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        hTransc - Handle to the transaction to add to remove.
 ***************************************************************************/
void RVCALLCONV SubsDetachTranscAndRemoveFromList(IN  Subscription*      pSubs,
                                                       IN  RvSipTranscHandle  hTransc);

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
void RVCALLCONV SubsUpdateStateAfterReject(Subscription* pSubs, RvSipSubsStateChangeReason eReason);

/*------------------------------------------------------------------------
          N A M E    F U N C T I O N S
 -------------------------------------------------------------------------*/

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
                                      IN  RvSipSubsState  eState);

/***************************************************************************
 * SubsGetStateEnum
 * ------------------------------------------------------------------------
 * General: Returns the enum of a given string state. This function
 *          is being used by the HighAval functions.
 * Return Value: The the state enum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     strState - The state as string
 ***************************************************************************/
RvSipSubsState RVCALLCONV SubsGetStateEnum(
                                      IN  RvChar *strState);

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
const RvChar *RVCALLCONV SubsGetTypeName(IN RvSipSubscriptionType eType);

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
RvSipSubscriptionType RVCALLCONV SubsGetTypeEnum(IN RvChar *strType);

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
const RvChar*  RVCALLCONV SubsGetChangeReasonName (IN  RvSipSubsStateChangeReason  eReason);

#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

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
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs - Handle to the subscription that sent the request.
 *          hNotify  - Handle to the notify object, in case of notify request.
 ***************************************************************************/
RvStatus RVCALLCONV SubsDNSGiveUp (IN  Subscription* pSubs);

/***************************************************************************
* SubsDNSContinue
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the message to the next ip
*          address, and change state of the state machine back to the sending
*          state.
*          You can use this function for a subscribe and notify requests.
*          For a SUBSCRIBE messages, use it if state was changed to
*          MSG_SEND_FAILURE state. (in this case set hNotify to be NULL).
*          Use this function for a NOTIFY request, if you got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event. In this case,
*          you should supply the notify handle.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs - Handle to the subscription that sent the request.
*          hNotify  - Handle to the notify object, in case of notify request.
***************************************************************************/
RvStatus RVCALLCONV SubsDNSContinue (IN  Subscription* pSubs);

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
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs - Handle to the subscription that sent the request.
*          hNotify  - Handle to the notify object, in case of notify request.
***************************************************************************/
RvStatus RVCALLCONV SubsDNSReSendRequest (IN  Subscription* pSubs);

#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT */

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
                            IN Subscription* pSubsOriginal);

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
                                       IN  RvSipMsgHandle     hMsg); 

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
RvChar* SubsGetCbName(RvInt32 MaskResults);


#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_OBJECTS_H*/


