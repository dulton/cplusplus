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
 *                              <SubsNotify.h>
 *
 *  This file defines the functions for notifications handling.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SUBS_NOTIFY_H
#define SUBS_NOTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON

#include "SubsMgrObject.h"
#include "AdsRlist.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsNotifyCreate
* ------------------------------------------------------------------------
* General: Creates a new notification object, related to a given subscription
*          and exchange handles with the application notification object.
*          The function also construct a notify message, and fill it with all
*          the parameters it can (To, From, callId, Event etc.)
*          This notify message is given here to user, so he can set the notify
*          state information to it, and then send it with RvSipNotifySend() function.
*
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
*               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
*               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs      - Pointer to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
*          phNotifyMsg - Handle to the notify message that was constructed inside
*                        the notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyCreate(   IN  Subscription*        pSubs,
                                         IN  RvSipAppNotifyHandle hAppNotify,
                                         OUT RvSipNotifyHandle    *phNotify);

/***************************************************************************
* SubsNotifyCreateObjInSubs
* ------------------------------------------------------------------------
* General: Creates a new notification object, related to a given subscription.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyCreateObjInSubs(IN  Subscription*        pSubs,
                                               IN  RvSipAppNotifyHandle hAppNotify,
                                               OUT RvSipNotifyHandle    *phNotify);

/***************************************************************************
 * SubsNotifyGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message (request or response) that is going to be sent
 *          by the notification. You may get the message only in states where
 *          notification may send a message.
 *          You can call this function before you call a control API functions
 *          that sends a message (such as RvSipNotifySend() or
 *          RvSipNotifyAccept()).
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify -  The notification handle.
 * Output:     phMsg   -  pointer to the message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyGetOutboundMsg(
                                     IN  Notification   *pNotify,
                                     OUT RvSipMsgHandle    *phMsg);

/***************************************************************************
 * SubsNotifySetSubscriptionStateParams
 * ------------------------------------------------------------------------
 * General: Creates the Subscription-State header in the outbound NOTIFY request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_UNKNOWN       -  Failed to set the notify body.
 *               RV_OK       -  set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify    - Handle to the notification object.
 *          eSubsState - SubsState to set in Subscription-State header.
 *          eReason    - reason to set in Subscription-State header
 *                       (RVSIP_SUBSCRIPTION_REASON_UNDEFINED for no reason).
 *          expiresParamVal - expires parameter value to set in Subscription-State
 *                       header (may be UNDEFINED)
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifySetSubscriptionStateParams(
                                            IN  Notification            *pNotify,
                                            IN  RvSipSubscriptionSubstate eSubsState,
                                            IN  RvSipSubscriptionReason eReason,
                                            IN  RvInt32                expiresParamVal);

/***************************************************************************
 * SubsNotifySend
 * ------------------------------------------------------------------------
 * General: Sends the notify message placed in the notification object.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid subscripion state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to send the notify request.
 *               RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifySend(IN  Subscription* pSubs,
                                    IN  Notification* pNotify);

/***************************************************************************
 * SubsNotifyRespond
 * ------------------------------------------------------------------------
 * General: Sends a response on notify message.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid subscripion state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to send the notify request.
 *               RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 *          statusCode - response code.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyRespond(IN  Subscription* pSubs,
                                       IN  Notification* pNotify,
                                       IN  RvUint16    statusCode);
/***************************************************************************
 * SubsNotifyTerminateNotifications
 * ------------------------------------------------------------------------
 * General: Go over the subscription notification list,
 *          Terminate the notification's transaction,
 *          and remove notification from the list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs   - Pointer to the subscription.
 *        bOnlyFree - RV_FALSE - terminate notification using SubsNotifyTerminate()
 *                    RV_TRUE  - do not terminate, only free using SubsNotifyFree()
 ***************************************************************************/
void RVCALLCONV SubsNotifyTerminateNotifications(IN  Subscription* pSubs,
                                                 IN  RvBool        bOnlyFree);

/***************************************************************************
 * SubsNotifyTerminate
 * ------------------------------------------------------------------------
 * General: The function inform user of notification object termination, and
 *          insert the notification object to the termination queue.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification object.
 *          eReason    - reason for termination.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyTerminate(IN Notification*         pNotify,
                                         IN RvSipSubsNotifyReason eReason);

/***************************************************************************
 * SetNotificationParamsFromMsg
 * ------------------------------------------------------------------------
 * General: Sets parameters from Subscription-State header to a notification
 *          object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the notification.
 *        hMsg  - Handle to the notify msg (request or response).
 * Output: eNotifyReason - reason for the notify event.
 ***************************************************************************/
RvStatus RVCALLCONV SetNotificationParamsFromMsg(IN  Notification* pNotify,
                                                  IN  RvSipMsgHandle hMsg,
                                                  OUT RvSipSubsNotifyReason* eNotifyReason);

/***************************************************************************
 * UpdateSubsAfter2xxOnNotifyRcvd
 * ------------------------------------------------------------------------
 * General: When user 2xx received to a notify request, it may effect the
 *          subscription state-machine.
 *          This function update it:
 *          1. if expires parameter exists - reset the expiration timer.
 *          2. update the subscription state machine:
 *             Notify(active) SubsActivated -> SubsActive
 *             Notify(terminated): SubsTerminating --> SubsTerminated.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        pNotify - Pointer to the notification.
 * Output:bSubsDeleted - was subscription object terminated in this function or not.
 *        (we need this parameter to know whethe to terminate the notify
 *        object or not, because ig the subscription is terminated, and we will
 *        terminate the notification - it will be terminated twice...)
 ***************************************************************************/
RvStatus RVCALLCONV UpdateSubsAfter2xxOnNotifyRcvd(IN  Subscription* pSubs,
                                                   IN  Notification* pNotify,
                                                   IN  RvSipTranscHandle hNotifyTransc,
                                                    OUT RvBool*      bSubsDeleted);

/***************************************************************************
 * InformNotifyStatus
 * ------------------------------------------------------------------------
 * General: Informs application of the notification status.
 *          For a subscriber: Gives the received Notify message, and it's related
 *          notification object handle. Application may get all notification
 *          information from the message at this callback.
 *          The message will be destructed after calling to this callback.
 *          For a Notifier: Informs application of a response message that was
 *          received for a Notify request. At the end of this callback,
 *          the stack notification object is destructed.
 *          For both subscriber and notifier it indicates about termination of
 *          the notification object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification.
 *            eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received notify msg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV InformNotifyStatus(IN  Notification          *pNotify,
                               IN  RvSipSubsNotifyStatus eNotifyStatus,
                               IN  RvSipSubsNotifyReason eNotifyReason,
                               IN  RvSipMsgHandle        hNotifyMsg) ;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
* SubsNotifyGetStatusName
* ------------------------------------------------------------------------
* General: Returns the name of a given status
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SubsNotifyGetStatusName (RvSipSubsNotifyStatus  eStatus);

/***************************************************************************
* SubsNotifyGetReasonName
* ------------------------------------------------------------------------
* General: Returns the name of a given notify reason.
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SubsNotifyGetReasonName (RvSipSubsNotifyReason  eReason);

/***************************************************************************
* GetNotifySubsStateName
* ------------------------------------------------------------------------
* General: Returns the name of a given notify reason.
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV GetNotifySubsStateName (RvSipSubscriptionSubstate  eSubsState);

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
/*-----------------------------------------------------------------------
        D N S
 ------------------------------------------------------------------------*/
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
 /***************************************************************************
 * SubsNotifyDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, terminate the transaction and the
 *          notification object.
 *          The function also update the subscription state for those cases:
 *          if subscription state is ACTIVATED - Terminate the subscription.
 *          if subscription state is TERMINATING - Change state back to previos state.
 *          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
 *          status in RvSipSubsNotifyEv event.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSGiveUp (IN  Subscription* pSubs,
                                          IN  Notification* pNotify);
/***************************************************************************
* SubsNotifyDNSContinue
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the and the notify request from
*          the notification object.
*          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input: pSubs      - Pointer to the subscription object.
*        pNotify    - Pointer to the notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSContinue (IN  Subscription* pSubs,
                                            IN  Notification* pNotify);

/***************************************************************************
* SubsNotifyDNSReSendRequest
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the cloned notify transaction from
*          the notification object.
*          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input: pSubs      - Pointer to the subscription object.
*        pNotify    - Pointer to the notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSReSendRequest (IN  Subscription* pSubs,
                                                 IN  Notification* pNotify);

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/*-----------------------------------------------------------------------
         G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_NOTIFY_H*/


