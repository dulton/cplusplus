
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
 *                              <_SipCallLeg.h>
 *
 *  The _SipCallLeg.h file contains Internal Api functions of the
 *  call-leg layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/


#ifndef SIP_CALL_LEG_H
#define SIP_CALL_LEG_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipCallLegTypes.h"
#include "_SipTransaction.h"
#include "RvSipSubscriptionTypes.h"
#include "RvSipCommonList.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     SUBSCRIPTION FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipCallLegSubsAddSubscription
 * ------------------------------------------------------------------------
 * General: Add a new subscription handle to the subscription list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg holding the subscription.
 *          hSubs    - Handle to the subscription to be added.
 * Output:  phItem   - the position item in call-leg subcription list.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegSubsAddSubscription(
                                        IN RvSipCallLegHandle  hCallLeg,
                                        IN RvSipSubsHandle     hSubs,
                                        OUT RLIST_ITEM_HANDLE  *phItem);



/***************************************************************************
* SipCallLegSubsRemoveSubscription
* ------------------------------------------------------------------------
* General: Removes a subscription from the subscription list.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg holding the new subscription.
*           hItem    - Position of the subscription in call-leg list.
***************************************************************************/
RvStatus RVCALLCONV SipCallLegSubsRemoveSubscription(
                                                  IN RvSipCallLegHandle   hCallLeg,
                                                  IN RLIST_ITEM_HANDLE    hItem);

/***************************************************************************
 * SipCallLegSubsFindSubscription
 * ------------------------------------------------------------------------
 * General: Find a subscription in the subscription list, by it's Event header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg holding the subscription.
 *          hEvent   - Event of subscription.
 *          eSubsType - type of subscription (RVSIP_SUBS_TYPE_SUBSCRIBER or RVSIP_SUBS_TYPE_NOTIFIER).
 * Output:     phSubs - Pointer to the Found subscription handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegSubsFindSubscription(
                                       IN  RvSipCallLegHandle     hCallLeg,
                                       IN  RvSipEventHeaderHandle hEvent,
                                       IN  RvSipSubscriptionType  eSubsType,
                                       OUT RvSipSubsHandle        *phSubs);

/***************************************************************************
* SipCallLegIsHiddenCallLeg
* ------------------------------------------------------------------------
* General: indicate if this is a hidden call-leg or not.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg holding the new subscription.
***************************************************************************/
RvBool RVCALLCONV SipCallLegIsHiddenCallLeg(RvSipCallLegHandle hCallLeg);

/***************************************************************************
* SipCallLegIsImsProtected
* ------------------------------------------------------------------------
* General: indicate if the provided CallLeg is served by SecAgree or SecObj.
* Return Value: RV_TRUE, if it is IMS protected.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
***************************************************************************/
RvBool RVCALLCONV SipCallLegIsImsProtected(RvSipCallLegHandle hCallLeg);

/***************************************************************************
* SipCallLegGetActiveReferSubs
* ------------------------------------------------------------------------
* General: returns the active refer subscription handle.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg holding the subscription.
***************************************************************************/
RvSipSubsHandle RVCALLCONV SipCallLegGetActiveReferSubs(RvSipCallLegHandle hCallLeg);

/***************************************************************************
* SipCallLegGetActiveTransaction
* ------------------------------------------------------------------------
* General: returns the active transaction handle.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg holding the subscription.
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipCallLegGetActiveTransaction(RvSipCallLegHandle hCallLeg);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
* SipCallLegGetMsgRcvdTransaction
* ------------------------------------------------------------------------
* General: returns the last transaction that received a message.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg holding the transaction.
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipCallLegGetMsgRcvdTransaction(RvSipCallLegHandle hCallLeg);
#endif /* #ifdef RV_SIP_IMS_ON */

/***************************************************************************
* SipCallLegIsFirstReferExists
* ------------------------------------------------------------------------
* General: indicate if first refer in call-leg exists.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
***************************************************************************/
RvBool RVCALLCONV SipCallLegIsFirstReferExists(RvSipCallLegHandle hCallLeg);

/***************************************************************************
* SipCallLegSetFirstReferExists
* ------------------------------------------------------------------------
* General: set the bFirstReferExists flag in call-leg to true.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
***************************************************************************/
RvStatus RVCALLCONV SipCallLegSetFirstReferExists(RvSipCallLegHandle hCallLeg); 

/***************************************************************************
* SipCallLegSubsInsertCallToHash
* ------------------------------------------------------------------------
* General: Insert a call-leg to the calls hash table.
*          This function is called when the first subscribe request is sent for
*          a hidden call-leg (function SubsSubscribe()).
*          Note that the function does not check if the call-leg is in deed hidden.
*          this checking should be done outside this function.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
*           bCheckInHash - check call-leg in the hash before inserting the
*                       new call-leg. (relevant for out-of-band subscription).
*                       If found, return failure.
***************************************************************************/
RvStatus RVCALLCONV SipCallLegSubsInsertCallToHash(RvSipCallLegHandle hCallLeg,
                                                    RvBool            bCheckInHash);

/***************************************************************************
* SipCallLegSubsRemoveHiddenCallFromHash
* ------------------------------------------------------------------------
* General: Remove a hidden call-leg from the calls hash table.
*          This function is called when a subscription on a hidden call-leg
*          is being terminated.
* Return Value: RV_Status
* ------------------------------------------------------------------------
* Arguments:
* Input:    hCallLeg - Handle to the call-leg.
***************************************************************************/
RV_Status RVCALLCONV SipCallLegSubsRemoveHiddenCallFromHash(RvSipCallLegHandle hCallLeg);
/***************************************************************************
* CallLegSubsGetSubscription
* ------------------------------------------------------------------------
* General: Gets a subscription handle from call-leg subscriptions list.
*          User may use the location and hItem parameters, to go over the list.
* Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
*               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
*               RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
*          location - The location in list - next, previous, first or last.
*          hRelative - Handle to the current position in the list (a relative
*                      subscription from the list). Supply this value if you choose
*                      next or previous in the location parameter.
* Output:     phSubs   -  The handle of the returned subscription.
***************************************************************************/
RvStatus RVCALLCONV SipCallLegSubsGetSubscription (
                                            IN  RvSipCallLegHandle hCallLeg,
                                            IN  RvSipListLocation      location,
                                            IN  RvSipSubsHandle        hRelative,
                                            OUT RvSipSubsHandle        *phSubs);

/***************************************************************************
* SipCallLegSubsIsSubsListEmpty
* ------------------------------------------------------------------------
* General: indicate if the call-leg's subscription list is empty.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer the call-leg holding the subscription list.
***************************************************************************/
RvBool RVCALLCONV SipCallLegSubsIsSubsListEmpty(RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * SipCallLegGetCurrentReferState
 * ------------------------------------------------------------------------
 * General: Gets the call-leg current refer state
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     peState -  The call-leg current refer state.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegGetCurrentReferState (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipCallLegReferState *peState);

/***************************************************************************
* SipCallLegGetAppHandle
* ------------------------------------------------------------------------
* General: retrieves the call-leg's application handle.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg .
***************************************************************************/
RvSipAppCallLegHandle RVCALLCONV SipCallLegGetAppHandle(
                                      IN RvSipCallLegHandle     hCallLeg);

/***************************************************************************
* SipCallLegTerminateIfPossible
* ------------------------------------------------------------------------
* General: Terminates a CallLeg object if possible. If there are transactions in
*          the call leg transaction list or subscription in the call leg then the call
*          leg will not be terminated. The last subscription / transaction in the call
*          leg will terminate the call leg.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg .
***************************************************************************/
void RVCALLCONV SipCallLegTerminateIfPossible(IN RvSipCallLegHandle hCallLeg,
                                              IN RvSipCallLegStateChangeReason eReason);


/***************************************************************************
* SipCallLegGetMutexHandle
* ------------------------------------------------------------------------
* General: retrieves the call-leg's mutex handle.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg .
***************************************************************************/
SipTripleLock* RVCALLCONV SipCallLegGetMutexHandle(
                                      IN RvSipCallLegHandle     hCallLeg);

/***************************************************************************
* SipCallLegSetTripleLockPointer
* ------------------------------------------------------------------------
* General: set the call-leg triple lock pointer(!).
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
*            pTripleLock - pointer to the new triple lock of the call-leg.
***************************************************************************/
void RVCALLCONV SipCallLegSetTripleLockPointer(
                               IN RvSipCallLegHandle hCallLeg,
                               IN SipTripleLock*     pTripleLock);

/***************************************************************************
 * SipCallLegCreateTransaction
 * ------------------------------------------------------------------------
 * General: creates a new transaction, attach the call as owner and insert
 *          the new transaction into the call-leg transactions list.
 * Return Value: RV_ERROR_UNKNOWN - If the call doesn't have all the information
 *                            needed to create the transaction for example
 *                            To header is missing or there is a general
 *                            failure when creating the transaction.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create transaction because of
 *                                   resource problem.
 *               RV_OK        - Transaction was created successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call-leg creating the new transaction.
 *          bAddToTranscList - Add the transaction to call-leg transactions list
 *                      or not. (subscription and notify transaction will be saved
 *                      in their objects, and not in the call-leg's list)
 * Output:     *phTransc - Handle to the newly created transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegCreateTransaction(
                                         IN  RvSipCallLegHandle  hCallLeg,
                                         IN  RvBool             bAddToTranscList,
                                         OUT RvSipTranscHandle  *phTransc);

/***************************************************************************
 * SipCallLegSendRequest
 * ------------------------------------------------------------------------
 * General: Sends a request to the remote party. First the Call-leg's request
 *          URI is determined and then a request is sent using the transaction API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The request message was not
 *                           sent.
 *               RV_OK - Request message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the request.
 *            hTransc - The transaction used to send the request.
 *            eRequestMethod - The request method to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegSendRequest(
                            IN RvSipCallLegHandle          hCallLeg,
                            IN RvSipTranscHandle           hTransc,
                            IN SipTransactionMethod        eRequestMethod,
                            IN RvChar*                    strMethod);


/***************************************************************************
* SipCallLegDestruct
* ------------------------------------------------------------------------
* General: Destruct a call-leg without informing application, and without
*          pushing it ti the termination queue.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
***************************************************************************/
void RVCALLCONV SipCallLegDestruct(RvSipCallLegHandle hCallLeg);

/***************************************************************************
* SipCallLegSetRedirectContact
* ------------------------------------------------------------------------
* General: Sets the call-leg redirectAddress in call-leg remote contact.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
***************************************************************************/
void RVCALLCONV SipCallLegSetRedirectContact(RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * SipCallLegGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipCallLegGetStateName(IN  RvSipCallLegState  eState);

/***************************************************************************
 * SipCallLegGetStateEnum
 * ------------------------------------------------------------------------
 * General: Returns the state enum according to given state name string
 * Return Value: The the state enum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strState - The state name string.
 ***************************************************************************/
RvSipCallLegState RVCALLCONV SipCallLegGetStateEnum(IN RvChar *strState);

/***************************************************************************
 * SipCallLegGetDirectionName
 * ------------------------------------------------------------------------
 * General: The function converts a Direction type enum to string.
 * Return Value: The suitable string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eDirection - The direction type.
 ***************************************************************************/
RvChar *SipCallLegGetDirectionName(RvSipCallLegDirection eDirection);

/***************************************************************************
 * SipCallLegGetDirectionEnum
 * ------------------------------------------------------------------------
 * General: The function converts a Direction string to enum type.
 * Return Value: The suitable enum value.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strDirection - The direction string.
 ***************************************************************************/
RvSipCallLegDirection SipCallLegGetDirectionEnum(RvChar *strDirection);

/***************************************************************************
 * SipCallLegUpdateReplacesStatus
 * ------------------------------------------------------------------------
 * General: Update the replaces status of a Call-Leg from a received
 *          INVITE message or REFER message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegUpdateReplacesStatus(
                                            IN RvSipCallLegHandle hCallLeg,
                                            IN RvSipMsgHandle     hMsg);

/***************************************************************************
 * SipCallLegAttachReferResultCallLegToReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Sets the new call-leg created as result of refer acceptness,
 *          to point to the refer subscription triple lock, and to point to the
 *          refer subscription that generated it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg             - The call-leg handle.
 *        hReferSubsGenerator  - Handle to the refer subscription.
 *        pReferSubsTriplelock - pointer to the refer subscription triple lock
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegAttachReferResultCallLegToReferSubsGenerator(
                                            IN RvSipCallLegHandle hCallLeg,
                                            IN RvSipSubsHandle    hCreatingReferSubs,
                                            IN SipTripleLock*     pReferSubsTriplelock);

/***************************************************************************
 * SipCallLegDetachReferResultCallLegFromReferSubsGenerator
 * ------------------------------------------------------------------------
 * General: Detach the call-leg from the refer subscription that generated it.
 *          The function sets the call-leg triple lock pointer, to point back
 *          to itself triple lock.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegDetachReferResultCallLegFromReferSubsGenerator(
                                            IN RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * SipCallLegGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given CallLeg object.
 *          The call-id is kept on the Call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegGenerateCallId(IN RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * SipCallLegSetReferredBy
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
RvStatus RVCALLCONV SipCallLegSetReferredBy (
                            IN  RvSipCallLegHandle           hCallLeg,
                            IN  RvSipReferredByHeaderHandle  hReferredByHeader);

/***************************************************************************
 * SipCallLegSetHeadersListToSetInInitialRequest
 * ------------------------------------------------------------------------
 * General: Sets the HeadersListToSetInInitialRequest to the call-leg,
 *          the list headers will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call-leg.
 *           hHeadersList - Handle to the headers list.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegSetHeadersListToSetInInitialRequest (
                            IN  RvSipCallLegHandle          hCallLeg,
                            IN  RvSipCommonListHandle       hHeadersList);


/* ---------------------------------------------------------------------
        R F C     3 5 1 5 (new refer)   W R A P P E R S
   --------------------------------------------------------------------- */
/***************************************************************************
 * SipCallLegReferSubsCreatedEv
 * ------------------------------------------------------------------------
 * General:  Indicates that a new subscription was created, and exchanges
 *           handles with the application.
 *           The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs     - The new sip stack subscription handle
 *          hCallLeg  - Handle to the related call-leg. NULL if this is a subscription
 *                      outside of a call-leg.
 *          hAppCallLeg - Handle to the related application call-leg. NULL if this is a
 *                      subscription outside of a call-leg.
 * Output:     phAppSubs - The application handle for this subscription.
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsCreatedEv(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipCallLegHandle hCallLeg,
                                   IN  RvSipAppCallLegHandle hAppCallLeg,
                                   IN  RvSipTranscHandle    hReferTransc,
                                   OUT RvSipAppSubsHandle *phAppSubs);

/***************************************************************************
 * SipCallLegReferSubsStateChangedEv
 * ------------------------------------------------------------------------
 * General: Indicates the application of a subscription state change.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs    - The sip stack subscription handle
 *            hAppSubs - The application handle for this subscription.
 *            eState   - The new subscription state.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsStateChangedEv(
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipAppSubsHandle         hAppSubs,
                                   IN  RvSipSubsState             eState,
                                   IN  RvSipSubsStateChangeReason eReason);

/***************************************************************************
 * SipCallLegReferSubsNotifyCreatedEv
 * ------------------------------------------------------------------------
 * General:  Indicates that a new notification was created, and exchanges
 *           handles with the application.
 *           The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hAppSubs      - The application handle for this subscription.
 *          hNotification - The new created notification object handle.
 *
 * Output:     phAppNotification - The application handle for this notification.
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsNotifyCreatedEv(
                                IN  RvSipSubsHandle     hSubs,
                                IN  RvSipAppSubsHandle  hAppSubs,
                                IN  RvSipNotifyHandle   hNotification,
                                OUT RvSipAppNotifyHandle *phAppNotification);

/***************************************************************************
 * SipCallLegReferSubsNotifyEv
 * ------------------------------------------------------------------------
 * General: Indicates of a notify status.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hAppSubs      - The application handle for this subscription.
 *          hNotification - The new created notification object handle.
 *          hAppNotification - The application handle for this notification. (relevant
 *                          only for a notifier subscription, which set the hAppNotification
 *                          in RvSipSubsCreateNotify().
 *          eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received msg (notify request or response).
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsNotifyEv(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipAppSubsHandle hAppSubs,
                                   IN  RvSipNotifyHandle  hNotification,
                                   IN  RvSipAppNotifyHandle  hAppNotification,
                                   IN  RvSipSubsNotifyStatus eNotifyStatus,
                                   IN  RvSipSubsNotifyReason eNotifyReason,
                                   IN  RvSipMsgHandle     hNotifyMsg);

/***************************************************************************
 * SipCallLegReferSubsReferNotifyReadyEv
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
void RVCALLCONV SipCallLegReferSubsReferNotifyReadyEv(
                           IN  RvSipSubsHandle                  hSubs,
                           IN  RvSipAppSubsHandle               hAppSubs,
                           IN  RvSipSubsReferNotifyReadyReason  eReason,
                           IN  RvInt16                         responseCode,
                           IN  RvSipMsgHandle                   hResponseMsg);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * SipCallLegReferSubsAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  This callback indicates the application that credentials were found
 *           in the request message (subscribe or notify request).
 *           The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The sip stack subscription handle
 *            hAppSubs       - The application handle for this subscription.
 *          hNotification  - The handle to the notification that received the
 *                           request.
 *          hAuthorization - The handle to the authorization header that contains the credentials.
 *          bCredentialsSupported - Determines whether or not the SIP Stack supports the
 *                           credentials in the authorization header.
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsAuthCredentialsFoundEv(
                      IN    RvSipSubsHandle                 hSubs,
                      IN    RvSipAppSubsHandle              hAppSubs,
                      IN    RvSipNotifyHandle               hNotification,
                      IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                      IN    RvBool                         bCredentialsSupported);

/***************************************************************************
 * SipCallLegReferSubsAuthCompletedEv
 * ------------------------------------------------------------------------
 * General:  RvSipSubsAuthCompletedEv() is called when the authentication procedure
 *           is completed.
 *           The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The handle to the subscription that received  the
 *                           Subscribe request. for a NOTIFY request, it is the
 *                           handle to the notification related subscription.
 *            hAppSubs       - The application handle for this subscription.
 *          hNotification  - The handle to the notification that received the
 *                           NOTIFY request. NULL for a SUBSCRIBE request.
 *          bAuthSucceed   - RV_TRUE if we found correct authorization header,
 *                           RV_FALSE if we did not.
 ***************************************************************************/
void RVCALLCONV SipCallLegReferSubsAuthCompletedEv(
                      IN    RvSipSubsHandle    hSubs,
                      IN    RvSipAppSubsHandle hAppSubs,
                      IN    RvSipNotifyHandle  hNotification,
                      IN    RvBool            bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */ 

#if defined(RV_SIP_SUBS_ON) || defined(RV_SIP_AUTH_ON)
/***************************************************************************
 * SipCallLegAuthenticatorCheckValidityOfAuthObj
 * ------------------------------------------------------------------------
 * General:  calls to SipAuthenticatorCheckValidityOfAuthObj with authentication 
 *           objects list, located in the call-leg. (the function is used 
 *           by the subscription layer).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The handle to the subscription that received  the
 *                           Subscribe request. for a NOTIFY request, it is the
 *                           handle to the notification related subscription.
 *            hAppSubs       - The application handle for this subscription.
 *          hNotification  - The handle to the notification that received the
 *                           NOTIFY request. NULL for a SUBSCRIBE request.
 *          bAuthSucceed   - RV_TRUE if we found correct authorization header,
 *                           RV_FALSE if we did not.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegAuthenticatorCheckValidityOfAuthObj(
                      IN    RvSipCallLegHandle    hCallLeg,
                      OUT   RvBool*              pbValid);
#endif /* #if defined(RV_SIP_SUBS_ON) || defined(RV_SIP_AUTH_ON) */

/***************************************************************************
 * SipCallLegReferSubsMsgToSendEv
 * ------------------------------------------------------------------------
 * General: An event indicating that a subscription related outgoing
 *          message is about to be sent.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs -      The sip stack subscription handle
 *            hAppSubs -   The application handle for this subscription.
 *          hNotify -    The notify object handle (relevant only for notify message)
 *          hAppNotify - The application notify object handle (relevant only for notify message)
 *            hMsg -       Handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsMsgToSendEv(
                                    IN    RvSipSubsHandle      hSubs,
                                    IN    RvSipAppSubsHandle   hAppSubs,
                                    IN    RvSipNotifyHandle    hNotify,
                                    IN    RvSipAppNotifyHandle hAppNotify,
                                    IN    RvSipMsgHandle       hMsg);

/***************************************************************************
 * SipCallLegReferSubsMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: An event indicating that a subscription related incoming
 *          message has been received.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs -      The sip stack subscription handle
 *            hAppSubs -   The application handle for this subscription.
 *            hNotify -    The notify object handle (relevant only for notify message)
 *          hAppNotify - The application notify object handle (relevant only for notify message)
 *            hMsg -       Handle to the incoming message.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsMsgReceivedEv(
                                    IN  RvSipSubsHandle      hSubs,
                                    IN  RvSipAppSubsHandle   hAppSubs,
                                    IN  RvSipNotifyHandle    hNotify,
                                    IN  RvSipAppNotifyHandle hAppNotify,
                                    IN  RvSipMsgHandle       hMsg);


/***************************************************************************
 * SipCallLegReferSubsOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The sip stack subscription handle
 *            hAppSubs       - The application handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *          hAppNotify     - The application notify object handle (relevant only for notify request or response)
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsOtherURLAddressFoundEv(
                                    IN  RvSipSubsHandle         hSubs,
                                    IN  RvSipAppSubsHandle      hAppSubs,
                                    IN  RvSipNotifyHandle       hNotify,
                                    IN  RvSipAppNotifyHandle    hAppNotify,
                                    IN  RvSipMsgHandle          hMsg,
                                    IN  RvSipAddressHandle      hAddress,
                                    OUT RvSipAddressHandle      hSipURLAddress,
                                    OUT RvBool                 *bAddressResolved);



/***************************************************************************
 * SipCallLegReferSubsFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the subscription is about to send
 *          a message after the destination address was resolved.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The sip stack subscription handle
 *            hAppSubs       - The application handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *          hAppNotify     - The application notify object handle (relevant only for notify request or response)
 *            hTransc        - The transaction handle
 *            hMsgToSend     - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsFinalDestResolvedEv(
                      IN  RvSipSubsHandle            hSubs,
                      IN  RvSipAppSubsHandle        hAppSubs,
                      IN  RvSipNotifyHandle            hNotify,
                      IN  RvSipAppNotifyHandle        hAppNotify,
                      IN  RvSipTranscHandle         hTransc,
                       IN  RvSipMsgHandle            hMsgToSend);


/***************************************************************************
 * SipCallLegReferSubsNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: This function is called only if the Refer is implemented
 *          with wrappers. We notfiy that a new connection was created.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The sip stack subscription handle
 *            hAppSubs       - The application handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *            hAppNotify     - The application notify object handle (relevant only for notify request or response)
 *            hConn          - The connection handle
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsNewConnInUseEv(
                      IN  RvSipSubsHandle                hSubs,
                      IN  RvSipAppSubsHandle             hAppSubs,
                      IN  RvSipNotifyHandle              hNotify,
                      IN  RvSipAppNotifyHandle           hAppNotify,
                      IN  RvSipTransportConnectionHandle hConn,
                      IN  RvBool                         bNewConnCreated);


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipCallLegReferSubsSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp request was sent over
 *          the subscription and wasn't responded. The application should
 *          determine if to re-sent the request with/without a bytecode.
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK additional message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs          - The sip stack subscription handle
 *            hAppSubs       - The application handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *            hAppNotify     - The application notify object handle (relevant only for notify request or response)
 *            hTransc        - The transaction handle
 *            hAppTransc     - The transaction application handle.
 *            retransNo      - The number of retransmissions of the request
 *                             until now.
 *            ePrevMsgComp   - The type of the previous not responded request
 * Output:    peNextMsgComp  - The type of the next request to retransmit (
 *                             RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                             application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegReferSubsSigCompMsgNotRespondedEv(
                      IN  RvSipSubsHandle              hSubs,
                      IN  RvSipAppSubsHandle           hAppSubs,
                      IN  RvSipNotifyHandle            hNotify,
                      IN  RvSipAppNotifyHandle         hAppNotify,
                      IN  RvSipTranscHandle            hTransc,
                      IN  RvSipAppTranscHandle         hAppTransc,
                      IN  RvInt32                      retransNo,
                      IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                      OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

#ifdef RV_SIP_HIGHAVAL_ON
/*-----------------------------------------------------------------------*/
/*                     HIGH AVAILABILITY FUNCTIONS                       */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipCallLegGetCallStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of connected call.
 *          (The size of buffer, that should be supply in RvSipCallLegStoreConnectedCall()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR     - hCallleg or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - Handle to the call-leg.
 *          bStandAlone  - Indicates if the CallLeg is independent.
 *                         This parameter should be RV_FALSE if
 *                         a subscription object retrieves its CallLeg
 *                         storage size, in order to avoid infinity recursion.
 * Output:  pLen     - size of buffer. will be UNDEFINED (-1) in case of failure.
 ***************************************************************************/
RvStatus SipCallLegGetCallStorageSize(IN  RvSipCallLegHandle hCallLeg,
                                      IN  RvBool             bStandAlone,
                                      OUT RvInt32           *pLen);

/***************************************************************************
 * SipCallLegStoreCall
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call leg.
 *          In order to store call-leg information the call leg must be in the
 *          connceted state.
 * Return Value:RV_ERROR_INVALID_HANDLE    - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR       - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION    - If the state is not on state CONNECTED, or the
 *                                    refer state is not REFER_IDLE.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES   - Resource problems.
 *              RV_OK          - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCallLeg    - Handle to the call-leg .
 *        maxBuffLen  - The length of the given buffer.
 *        bStandAlone - Indicates if the CallLeg is independent .
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg
 *                      storage size, in order to avoid infinity recursion.
 * Output: memBuff    - The buffer that will store the CallLeg's parameters.
 *         pStoredLen - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus SipCallLegStoreCall(IN  RvSipCallLegHandle hCallLeg,
                             IN  RvBool             bStandAlone,
                             IN  RvUint32           maxBuffLen,
                             OUT void              *memBuff,
                             OUT RvInt32           *pStoredLen);

/***************************************************************************
 * SipCallLegRestoreCall
 * ------------------------------------------------------------------------
 * General: Restore all call-leg information into a given call-leg. The call-leg
 *          will assume the connceted state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not on state CONNECTED, or the
 *                                 refer state is not REFER_IDLE.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff     - The buffer that stores all the callLeg's parameters.
 *        buffLen     - The buffer length.
 *        bStandAlone - Indicates if the CallLeg is independent.
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg
 *                      storage size, in order to avoid infinity recursion.
 * InOut: hCallLeg    - Handle to the call-leg.
 ***************************************************************************/
RvStatus SipCallLegRestoreCall(IN    void              *memBuff,
                               IN    RvUint32           buffLen,
                               IN    RvBool             bStandAlone,
                               INOUT RvSipCallLegHandle hCallLeg);
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 

/******************************************************************************
 * SipCallLegForkingInitCallLeg
 * ----------------------------------------------------------------------------
 * General: Loads data from Original Call-Leg and from Message into forked
 *          Call-Leg. Inserts forked Call-Leg into hash. Gets forked Call-Leg
 *          to RVSIP_CALL_LEG_STATE_PROCEEDING / RVSIP_CALL_LEG_STATE_ACCEPTED
 *          state. Dialog identifiers for the forked Call-Leg are took from
 *          the Message.
 * Return Value:
 *          RV_OK on success; error code, defined in RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: hCallLegMgr       - Handle to the Call-Leg Manager.
 *        hCallLeg          - Handle to the Forked Call-Leg.
 *        hOriginalCallLeg  - Handle to the Original Call-Leg.
 *        hMsg              - Handle to Message, caused forked dialog creation.
 *        bSwitchToAndFromTags - If RV_TRUE, TO and FROM tags from the Message,
 *                            will be switched while being inserted into
 *                            Forked Call-Leg (Forked Call-Leg can be created
 *                            as a result of response arrival, or as a result
 *                            of new request arrival (e.g. NOTIFY).
 *****************************************************************************/
RvStatus SipCallLegForkingInitCallLeg(
                            IN  RvSipCallLegMgrHandle   hCallLegMgr,
                            IN  RvSipCallLegHandle      hCallLeg,
                            IN  RvSipCallLegHandle      hOriginalCallLeg,
                            IN  RvSipMsgHandle          hMsg,
                            IN  RvBool                  bSwitchToAndFromTags);


/******************************************************************************
 * SipCallLegSetForkingEnabledFlag
 * ----------------------------------------------------------------------------
 * General: Set value of Forking Enabled flag into Call-Leg.
 *          If RV_FALSE - 2XX response, arrived in response to forked INVITE,
 *          will not create a new (forked) Call-Leg.
 *
 * Return Value: RV_OK on success, error code,defined in RV_SIP_DEF.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg        - Handle to the Call-Leg.
 *          bForkingEnabled - Flag value to be set.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegSetForkingEnabledFlag(
                                        IN RvSipCallLegHandle hCallLeg,
                                        IN RvBool             bForkingEnabled);

/******************************************************************************
 * SipCallLegGetForkingEnabledFlag
 * ----------------------------------------------------------------------------
 * General: Get value of Forking Enabled flag of the Call-Leg.
 *          If RV_FALSE - 2XX response, arrived in response to forked INVITE,
 *          will not create a new (forked) Call-Leg.
 *
 * Return Value: RV_OK on success, error code,defined in RV_SIP_DEF.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg         - Handle to the Call-Leg.
 * Output:  pbForkingEnabled -pointer to memory where flag value will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV SipCallLegGetForkingEnabledFlag(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvBool               *pbForkingEnabled);

#ifdef RV_SIP_SUBS_ON
/******************************************************************************
 * SipCallLegSetSubsForkingEnabledFlag
 * ----------------------------------------------------------------------------
 * General: Set value of Forking Enabled flag into Call-Leg, while checking
 *          if the subscription forking is enabled.
 *
 * Return Value: RV_OK on success, error code,defined in RV_SIP_DEF.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg        - Handle to the Call-Leg.
 *          bForkingEnabled - Flag value to be set.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegSetSubsForkingEnabledFlag(
                                        IN RvSipCallLegHandle hCallLeg,
                                        IN RvBool             bForkingEnabled);
#endif /*#ifdef RV_SIP_SUBS_ON*/

/***************************************************************************
 * SipCallLegGetOriginalCallLeg
 * ------------------------------------------------------------------------
 * General: This function gives the original call-leg of a given
 *          'forked call-leg'.
 *          The original call-leg is the call-leg that had sent the initial
 *          INVITE request.
 *          The 'forked call-leg' is the call-leg that was created when a
 *          1xx/2xx response was received, due to the INVITE request forking.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the 'forked call-leg'.
 * Output:  phOrigCallLeg -   SIP stack handle to the original call-leg
 ***************************************************************************/
RvStatus SipCallLegGetOriginalCallLeg(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    OUT RvSipCallLegHandle  *phOrigCallLeg);

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipCallLegSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Call-Leg object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          pParams  - Parameters to be set.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipCallLegSetSctpMsgSendingParams(
                    IN  RvSipCallLegHandle                 hCallLeg,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/***************************************************************************
 * SipCallLegGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Call-Leg object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 * Output:  pParams  - Parameters to be get.
 ***************************************************************************/
void RVCALLCONV SipCallLegGetSctpMsgSendingParams(
                    IN  RvSipCallLegHandle                 hCallLeg,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/


#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipCallLegInviteFindObjByCseq
 * ------------------------------------------------------------------------
 * General: Finds the invite object that matches the given cseq.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg      - Handle to the call-leg
 *          cseq          - The CSeq number we wish to find its Invite Object
 *          eDirection    - The direction of the requested Invite object
 * Output:  phInviteObj   - Handle to the invite object.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegInviteFindObjByCseq(
											   IN  RvSipCallLegHandle		 hCallLeg,
											   IN  RvInt32					 cseq,
                                               IN  RvSipCallLegDirection	 eDirection,
                                               OUT RvSipCallLegInviteHandle *phInviteObj);

/***************************************************************************
 * SipCallLegInviteGetTrx
 * ------------------------------------------------------------------------
 * General: Gets the transmitter owned by this Invite object
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg      - Handle to the call-leg
 *          hInviteObj    - The Invite object to retrieve its transmitter
 * Output:  phTrx         - The Invite object transmitter
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegInviteGetTrx(IN  RvSipCallLegHandle		 hCallLeg,
										   IN  RvSipCallLegInviteHandle  hInviteObj,
										   OUT RvSipTransmitterHandle   *phTrx);

#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_CALL_LEG_H */



