
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
 *                              <CallLegRefer.h>
 *
 *  Handles REFER process methods. Such as:
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

#ifndef CALL_LEG_REFER_H
#define CALL_LEG_REFER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
#include "RvSipCallLegTypes.h"
#include "CallLegObject.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegReferCreateSubsAndRefer
 * ------------------------------------------------------------------------
 * General: Sends a REFER assosiated with the call-leg. This function may be
 *          called only after the To and From header fields were set.
 *          Calling Refer causes a REFER request to be sent out and the
 *          call-leg refer state machine to progress to the Refer Sent
 *          state.
 *          This function is also used to send an authenticated refer
 *          request in the RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call leg the user wishes to send REFER.
 *          hReferTo     - The Refer-To header to be sent in the REFER request.
 *          hReferredBy  - The Referred-By header to be sent in the REFER request.
 *          strReferTo   - The Refer-To header in a string format.
 *          strReferredBy - The Referred-By header in a string format.
 *          strReplaces  - The Replaces for Refer-To header in a string format.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferCreateSubsAndRefer (
                                      IN  CallLeg                 *pCallLeg,
                                      IN  RvSipReferToHeaderHandle    hReferTo,
                                      IN  RvSipReferredByHeaderHandle hReferredBy,
                                      IN  RvChar             *strReferTo,
                                      IN  RvChar             *strReferredBy,
                                      IN  RvChar              *strReplaces);
/***************************************************************************
 * CallLegReferChangeReferState
 * ------------------------------------------------------------------------
 * General: Changes the call-leg refer state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new refer sub state.
 *            eReason  - The refer sub state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegReferChangeReferState(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferState                eState,
                               IN  RvSipCallLegStateChangeReason         eReason);

/***************************************************************************
 * CallLegReferRequest
 * ------------------------------------------------------------------------
 * General: Sends a REFER assosiated with the call-leg. The
 *          call-leg refer sub state machine will progress to the Refer Sent
 *          state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg the user wishes to send REFER.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferRequest (IN  CallLeg   *pCallLeg);

/***************************************************************************
 * CallLegReferNotifyRequest
 * ------------------------------------------------------------------------
 * General: Sends a NOTIFY assosiated with the call-leg. The
 *          call-leg refer sub state machine will progress to the Refer Sent
 *          state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg the user wishes to send REFER.
 *          status   - The status code that will be used to create a status
 *                     line for the NOTIFY request message body.
 *          cSeqStep - The Cseq step of the REFER transaction that this
 *                     NOTIFY relate to. This value will be set to the cseq
 *                     parameter of the Event header of the NOTIFY request.
 * Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV CallLegReferNotifyRequest (
                                                IN  CallLeg            *pCallLeg,
                                                IN  RvInt16            status,
                                                IN  RvInt32            cSeqStep);


/***************************************************************************
 * CallLegReferAccept
 * ------------------------------------------------------------------------
 * General: Called to indicate that the application is willing to accept
 *          an incoming REFER request. This function will respond with
 *          202-Accepted response to the REFER request,
 *          create a new call-leg object, assosiate the new call-leg and
 *          the call-leg, and set to the new call-leg the following paramateres:
 *          Call-Id: the Call-Id of the REFER request, To: The Refer-To header
 *          of the REFER request, From: The local contact,  Referred-By: The
 *          Referred-By header of the RFEER request.
 *          To complete the acceptens of the REFER request the application must
 *          use the newly created call-leg and cal RvSipCallLegConnect on it.
 * Return Value: RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 202 response, or
 *                                   to create and inititialize the new call-leg).
 *               RV_OK       -  Accepted final response was sent successfully,
 *                                   and a new call-leg was created and initialized
 *                                   successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to accept refer from.
 *          hAppCallLeg - Application handle to the newly created call-leg.
 * Output:  phNewCallLeg - The new call-leg that is created and initialized by
 *                        this function.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferAccept (
                                         IN  CallLeg               *pCallLeg,
                                         IN  RvSipAppCallLegHandle hAppCallLeg,
                                         OUT RvSipCallLegHandle    *phNewCallLeg);


/***************************************************************************
 * CallLegReferReject
 * ------------------------------------------------------------------------
 * General: Can be used in the Refer Received refer state to reject an
 *          incoming REFER request.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg the user wishes to reject
 *            status   - The rejection response code.
 * Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV CallLegReferReject (IN  CallLeg       *pCallLeg,
                                         IN  RvUint16      status);

/***************************************************************************
 * CallLegReferEnd
 * ------------------------------------------------------------------------
 * General: When the application receives the Notify Ready notification
 *          from the RvSipCallLegReferNotifyEv callback it must decide
 *          weather to send a NOTIFY request to the REFER sender or not.
 *          If the application decides not to send a NOTIFY request to the
 *          REFER sender, it must call this function to complete the REFER
 *          process. If the application chooses to send a NOTIFY request to
 *          the REFER sender, it must not call this function.
 *          Note: If you call this function with a call-leg in the Idle state,
 *          the call-leg will be terminated, and hence must not be referenced
 *          after the call.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_OK - REFER process was completed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to send REFER.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferEnd (  IN  CallLeg   *pCallLeg);

/***************************************************************************
 * CallLegReferNotifyReferGenerator
 * ------------------------------------------------------------------------
 * General: Notify the refer generator (if exists) about the call-leg received
 *          response.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg.
 *            hTransc  - Handle to the transaction.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferNotifyReferGenerator (
                                         IN  CallLeg          *pCallLeg,
                                         IN  RvSipSubsReferNotifyReadyReason eReason,
                                         IN  RvSipMsgHandle     hResponseMsg);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * CallLegReferGetNotifyEventName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given notify sub state
 * Return Value: The string with the notify sub state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegReferGetNotifyEventName (
                                    IN  RvSipCallLegReferNotifyEvents  eState);

#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * CallLegReferGetSubsStateName
 * ------------------------------------------------------------------------
 * General: The function returns the hActiveReferSubs state as string.
 *          (The function is used to print the state to log)
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
const RvChar* RVCALLCONV CallLegReferGetSubsStateName(IN  CallLeg *pCallLeg);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

/***************************************************************************
* CallLegReferGetCurrentReferSubsState
* ------------------------------------------------------------------------
* General: Gets the call-leg current refer subscription state;
* Return Value: RvSipSubsState
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
***************************************************************************/
RvSipSubsState RVCALLCONV CallLegReferGetCurrentReferSubsState (
                                     IN  CallLeg* pCallLeg);

/****************************************************************************
* CallLegReferGetCurrentReferState
* ------------------------------------------------------------------------
* General: Gets the call-leg current refer state
* Return Value: RvSipCallLegReferState
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
***************************************************************************/
RvSipCallLegReferState RVCALLCONV CallLegReferGetCurrentReferState (
                                     IN  RvSipCallLegHandle   hCallLeg);

/*------------------------------------------------------------------------
                            REFER DNS
  ------------------------------------------------------------------------ */
/***************************************************************************
 * CallLegReferDNSGiveUp
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSGiveUp for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSGiveUp(IN  CallLeg *pCallLeg);

/***************************************************************************
 * CallLegReferDNSContinue
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSContinue for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSContinue(IN  CallLeg *pCallLeg);

/***************************************************************************
 * CallLegReferDNSReSend
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSReSendRequest for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSReSend(IN  CallLeg *pCallLeg);

/***************************************************************************
 * CallLegReferDNSGetList
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSGetList for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSGetList(
                            IN  CallLeg                     *pCallLeg,
                            OUT RvSipTransportDNSListHandle *phDnsList);

/***************************************************************************
 * CallLegReferSubsStateChangedEvHandler
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription change state, in case application choose
 *          to disable the new refer behavior.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eState   - The new subscription state.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsStateChangedEvHandler(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsState             eState,
                                   IN  RvSipSubsStateChangeReason eReason);

/***************************************************************************
 * CallLegReferSubsNotifyStateChangedEv
 * ------------------------------------------------------------------------
 * General: Handles the refer notify state.
 *          The function wraps the notify statuses, and call to the old
 *          refer notify event.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hNotification - The new created notification object handle.
 *          eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received msg (notify request or response).
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsNotifyStateChangedEv(
                                   IN  CallLeg*           pCallLeg,
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipNotifyHandle  hNotification,
                                   IN  RvSipSubsNotifyStatus eNotifyStatus,
                                   IN  RvSipSubsNotifyReason eNotifyReason,
                                   IN  RvSipMsgHandle     hNotifyMsg);

/***************************************************************************
 * CallLegReferSubsNotifyReadyEv
 * ------------------------------------------------------------------------
 * General: Indicates that a refer notifier subscription is ready to
 *          send a notify request.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hAppSubs      - The application handle for this subscription.
 *          eReason       - The reason for a NOTIFY request to be sent
 *          responseCode  - The response code that should be set in the
 *                          NOTIFY message body.
 *          hResponseMsg  - The message that was received on the refer related
 *                          object (provisional or final response).
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsNotifyReadyEv(
                           IN  CallLeg*                         pCallLeg,
                           IN  RvSipSubsHandle                  hSubs,
                           IN  RvSipSubsReferNotifyReadyReason  eReason,
                           IN  RvInt16                         responseCode,
                           IN  RvSipMsgHandle                   hResponseMsg);

/***************************************************************************
 * CallLegReferSubsOtherURLAddressFoundEvHandler
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - Pointer to the call-leg
 *            hSubs          - The handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *          hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferSubsOtherURLAddressFoundEvHandler(
                                    IN  CallLeg                 *pCallLeg,
                                    IN  RvSipSubsHandle            hSubs,
                                    IN  RvSipNotifyHandle        hNotify,
                                    IN  RvSipMsgHandle          hMsg,
                                    IN  RvSipAddressHandle        hAddress,
                                    OUT RvSipAddressHandle        hSipURLAddress,
                                    OUT RvBool                    *bAddressResolved);

/***************************************************************************
 * CallLegReferSetReferredBy
 * ------------------------------------------------------------------------
 * General: Sets the Referred-By header to the call-leg,
 *          this header will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg.
 *           hReferredByHeader - Handle to the Referred-By hedaer to be set.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferSetReferredBy (
                            IN  CallLeg                     *pCallLeg,
                            IN  RvSipReferredByHeaderHandle  hReferredByHeader);

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_REFER_H */


