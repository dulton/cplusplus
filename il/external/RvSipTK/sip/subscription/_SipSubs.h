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
 *                              <_SipSubsMgrObject.h>
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SIP_SUBS_OBJECT_H
#define SIP_SUBS_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipSubscriptionTypes.h"
#include "RvSipCallLegTypes.h"
#include "RvSipResourcesTypes.h"
#include "_SipTransaction.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SipSubsSetSubsCurrentState
* ------------------------------------------------------------------------
* General: The function sets the subscription current state. (This function
*          is used by high-availability feature).
* Return Value: RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
*        eState - State of subscription.
***************************************************************************/
RvStatus RVCALLCONV SipSubsSetSubsCurrentState(RvSipSubsHandle hSubs,
                                                RvSipSubsState  eState);

/***************************************************************************
* SipSubsGetSubsCurrentState
* ------------------------------------------------------------------------
* General: The function gets the subscription current state.
* Return Value: RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
*        eState - State of subscription.
***************************************************************************/
/*RvStatus RVCALLCONV SipSubsGetSubsCurrentState(IN RvSipSubsHandle  hSubs,
                                                OUT RvSipSubsState  *peState);*/

/***************************************************************************
* SipSubsGetCurrState
* ------------------------------------------------------------------------
* General: returns the state of subscription
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification.
***************************************************************************/
RvSipSubsState SipSubsGetCurrState(IN RvSipSubsHandle hSubs);

/***************************************************************************
* SipSubsGetTransc
* ------------------------------------------------------------------------
* General: The function gets the subscription transaction handle
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipSubsGetTransc(RvSipSubsHandle hSubs);
/***************************************************************************
* SipSubsNotifyGetTransc
* ------------------------------------------------------------------------
* General: The function gets the notification transaction handle
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipSubsNotifyGetTransc(RvSipNotifyHandle hNotify);

/***************************************************************************
* SipSubsReferGetFinalStatus
* ------------------------------------------------------------------------
* General: The function gets the refer final status value,
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvUint16 RVCALLCONV SipSubsReferGetFinalStatus(RvSipSubsHandle hSubs);

/***************************************************************************
* SipSubsReferGetFinalStatus
* ------------------------------------------------------------------------
* General: The function gets the refer final status value,
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvStatus RVCALLCONV SipSubsReferGetEventId(IN RvSipSubsHandle hSubs,
                                           OUT RvInt32*      pEventId);

/***************************************************************************
* SipSubsSetSubsType
* ------------------------------------------------------------------------
* General: The function sets the subscription type. (This function
*          is used by high-availability feature).
* Return Value: RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
*        eType - Type of subscription.
***************************************************************************/
RvStatus RVCALLCONV SipSubsSetSubsType(IN RvSipSubsHandle       hSubs,
                                       IN RvSipSubscriptionType eType);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SipSubsGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of state of a given subscription.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs -
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetStateNameOfSubs (
                                      IN  RvSipSubsHandle  hSubs);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/***************************************************************************
 * SipSubsSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Set the subscription outbound msg.
 *          The function is used from call-leg layer for refer subscription,
 *          when we know that no one else knows about this subscription.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - The subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsSetOutboundMsg(RvSipSubsHandle hSubs,
                                           RvSipMsgHandle  hMsg);

/***************************************************************************
* SipSubsHandleTranscChangeState
* ------------------------------------------------------------------------
* General: Handle a transaction state change.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:hTransc      - Handle to the transaction whos state was changed.
*       hCallLeg     - The call-leg this transaction belongs to.
*       eTranscState - The new state of the transaction
*       eStateReason - The reason for the change of state.
*       eMethod      - SUBSCRIBE or NOTIFY
*       pObj         - Pointer to the subscription or notification object,
*                      saved inside the transaction.
* Output:bWasHandled - Was the state handled by this function or not.
***************************************************************************/
RvStatus RVCALLCONV SipSubsHandleTranscChangeState(
                    IN    RvSipTranscHandle                 hTransc,
                    IN  RvSipCallLegHandle                hCallLeg,
                    IN  RvSipTransactionState             eTranscState,
                    IN  RvSipTransactionStateChangeReason eStateReason,
                    IN  SipTransactionMethod              eMethod,
                    IN  void*                             pObj,
                    OUT RvBool*                          bWasHandled);

/***************************************************************************
* SipSubsMsgRcvdEvHandling
* ------------------------------------------------------------------------
* General: Handle a message received event, for a SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to any subscription,
*          bWasMsgHandled is set to false, so call-leg continue in handling the
*          message.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          hMsg - the received msg.
*          eMethodType - Notify or Subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsMsgRcvdEvHandling(
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipMsgHandle       hMsg,
                            IN  SipTransactionMethod eMethodType);

/***************************************************************************
* SipSubsMsgToSendEvHandling
* ------------------------------------------------------------------------
* General: Handle a message to send event, for a SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to any subscription,
*          bWasMsgHandled is set to false, so call-leg continue in handling the
*          message.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          hMsg - the received msg.
*          eMethodType - Notify or Subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsMsgToSendEvHandling(
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipMsgHandle       hMsg,
                            IN  SipTransactionMethod eMethodType);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
* SipSubsNotifyHandleAuthCredentialsFound
* ------------------------------------------------------------------------
* General: Supply an authorization header, to pass it to the user, for a
*          SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc - handle to the transaction.
*          eMsgType - Notify or Subscribe.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleAuthCredentialsFound(
                          IN  RvSipTranscHandle               hTransc,
                          IN  SipTransactionMethod            eMethodType,
                          IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                          IN  RvBool                         bCredentialsSupported);

/***************************************************************************
* SipSubsNotifyHandleAuthCompleted
* ------------------------------------------------------------------------
* General: Notify that authentication procedure is completed, for a SUBSCRIBE or
*          NOTIFY message. If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc - handle to the transaction.
*          eMsgType - Notify or Subscribe.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleAuthCompleted(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvBool              bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
* SipSubsNotifyHandleOtherURLAddressFound
* ------------------------------------------------------------------------
* General: Notify that authentication procedure is completed, for a SUBSCRIBE or
*          NOTIFY message. If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc           - handle to the transaction.
*           eMethodType    - The transaction method (Notify or Subscribe.)
*            hMsg           - The message that includes the other URL address.
*            hAddress       - Handle to unsupport address to be converted.
* Output:    hSipURLAddress - Handle to the known SIP URL address.
*            bAddressResolved-Indication of a successful/failed address
*                             resolving.
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleOtherURLAddressFound(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipMsgHandle       hMsg,
                              IN  RvSipAddressHandle   hAddress,
                              OUT RvSipAddressHandle   hSipURLAddress,
                              OUT RvBool                  *bAddressResolved);

/***************************************************************************
* SipSubsNotifyHandleFinalDestResolved
* ------------------------------------------------------------------------
* General:  Indicates that the transaction is about to send
*           a message after the destination address was resolved.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc           - handle to the transaction.
*           eMethodType    - The transaction method (Notify or Subscribe.)
*            hMsg           - The message to be sent
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleFinalDestResolved(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipMsgHandle       hMsg);


/***************************************************************************
 * SipSubsNotifyHandleNewConnInUse
 * ------------------------------------------------------------------------
 * General:  Indicates that the transaction is using a new connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc      - handle to the transaction.
 *           eMethodType  - The transaction method (Notify or Subscribe.)
 *           hConn         - The connection handle,
 *            bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleNewConnInUse(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipTransportConnectionHandle hConn,
                              IN  RvBool               bNewConnCreated);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SipSubsNotifyHandleSigCompMsgNotResponded
* ------------------------------------------------------------------------
* General:  Indicates that SigComp request was sent with no response
*           received back. The application should determine what action
*           has to be taken.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc         - Handle to the transaction.
*          hAppTransc      - The transaction application handle.
*          eMethodType     - The transaction method (Notify or Subscribe.)
*          retransNo       - The number of retransmissions of the request
*                            until now.
*          ePrevMsgComp    - The type of the previous not responded request
* Output:  peNextMsgComp   - The type of the next request to retransmit (
*                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
*                            application wishes to stop sending requests).
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleSigCompMsgNotResponded(
                              IN  RvSipTranscHandle            hTransc,
                              IN  RvSipAppTranscHandle         hAppTransc,
                              IN  SipTransactionMethod         eMethodType,
                              IN  RvInt32                      retransNo,
                              IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                              OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
* SipSubsTerminate
* ------------------------------------------------------------------------
* General: Terminate a subscription, without sending any message.
*          If there are notification objects, it will be terminated first, and
*          when the last notification is free, the subscription is terminated.
*          if there are no notification object, the subscription is terminated at once.
*          subscription termination: subs state is changed to TERMINATED,
*          and subscription is destructed.
*          The function is used from call-leg layer, when user calls to RvSipCallLegTerminate.
*          subscription functions should use internal SubsTerminate function.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription the user wishes to subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsTerminate (IN  RvSipSubsHandle hSubs);

/***************************************************************************
* SipSubsGetListItemHandle
* ------------------------------------------------------------------------
* General: return the list item handle of the subscription in call-leg
*          subscriptions list.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RLIST_ITEM_HANDLE SipSubsGetListItemHandle(IN RvSipSubsHandle hSubs);

/***************************************************************************
* SipSubsIsEventHeaderExist
* ------------------------------------------------------------------------
* General: Define wether this subscription conains an event header or not.
*          (if not, it is the same as having 'event:PINT').
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvBool SipSubsIsEventHeaderExist(IN RvSipSubsHandle hSubs);

/***************************************************************************
* SipSubsInformOfRejectedRequest
* ------------------------------------------------------------------------
* General: If the call-leg layer rejected a subscription request, because
*          of a dialog mistake (such as wrong cseq) it will inform the
*          subscription of the rejection, with this function.
*          in this function, subscription updates it's state-machine.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvStatus SipSubsInformOfRejectedRequestByDialog(IN void*             pSubsInfo,
                                                 IN RvSipTranscHandle hTransc);

/***************************************************************************
* SipSubsGetSubscriptionStateOfNotifyObj
* ------------------------------------------------------------------------
* General: returns the state of subscription related to this notification object.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification.
***************************************************************************/
RvSipSubsState SipSubsGetSubscriptionStateOfNotifyObj(IN RvSipNotifyHandle hNotify);

/***************************************************************************
* SipSubsReferGeneratorCallNotifyReady
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - reason for the notify ready.
*          hResponseMsg - Handle to the received response message.
****************************************************************************/
RvStatus RVCALLCONV SipSubsReferGeneratorCallNotifyReady(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipSubsReferNotifyReadyReason eReason,
                                   IN  RvSipMsgHandle     hResponseMsg);

/***************************************************************************
* SipSubsIsFirstReferSubs
* ------------------------------------------------------------------------
* General: Define wether this subscription is the first refer subscription
*          created on this dialog.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvBool SipSubsIsFirstReferSubs(IN RvSipSubsHandle hSubs);

/***************************************************************************
 * SipSubsReferInit
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with mandatory parameters:
 *          refer-to and referred-by headers of the refer subscription.
 *          If the subscription was not created inside of a call-leg you must
 *          call RvSipSubsDialogInitStr() before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request.
 *          You should call RvSipSubsRefer() in order to send a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - Handle to the subscription the user wishes to initialize.
 *          strReferTo    - String of the Refer-To header value.
 *                          for Example: "A<sip:176.56.23.4:4444;method=INVITE>"
 *          strReferredBy - String of the Referred-By header value.
 *                          for Example: "<sip:176.56.23.4:4444>"
 *          strReplaces  - The Replaces header to be set in the Refer-To header of
 *                         the REFER request. The Replaces header string doesn't
 *                           contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                           the subscription object.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsReferInit(
                         IN  RvSipSubsHandle             hSubs,
                         IN  RvSipReferToHeaderHandle    hReferToHeader,
                         IN  RvSipReferredByHeaderHandle hReferredByHeader,
                         IN  RvChar*                    strReferTo,
                         IN  RvChar*                    strReferredBy,
                         IN  RvChar*                    strReplaces);


/******************************************************************************
* SipSubsGetTripleLock
* -----------------------------------------------------------------------------
* General: enable access to the Subscription triple lock from another modules
* -----------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
SipTripleLock* RVCALLCONV SipSubsGetTripleLock(IN RvSipSubsHandle hSubs);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SipSubsGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetStateName (
                                      IN  RvSipSubsState  eState);

/***************************************************************************
* SipSubsNotifyGetStatusName
* ------------------------------------------------------------------------
* General: Returns the name of a given status
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SipSubsNotifyGetStatusName (RvSipSubsNotifyStatus  eStatus);

/***************************************************************************
 * SipSubsReferGetNotifyReadyReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsReferGetNotifyReadyReasonName(
                               IN  RvSipSubsReferNotifyReadyReason  eReason);

/***************************************************************************
 * SipSubsGetChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetChangeReasonName(
                                    IN  RvSipSubsStateChangeReason  eReason);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef RV_SIP_HIGHAVAL_ON
/***************************************************************************
 * SipSubsGetActiveSubsStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of active subs.
 *          (The size of buffer, that should be supply in RvSipSubsStoreActiveSubs()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE in case
 *                           that a CallLeg object retrieves its subscription
 *                           storage size, in order to avoid infinity recursion.
 * Output:  pLen           - size of buffer. will be -1 in case of failure.
 ***************************************************************************/
RvStatus SipSubsGetActiveSubsStorageSize(IN  RvSipSubsHandle hSubs,
                                         IN  RvBool          bStandAlone,
                                         OUT RvInt32        *pLen);

/***************************************************************************
 * SipSubsStoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Gets all the important parameters of an already active subs.
 *          The subs must be in state active.
 *          User must give an empty page that will be fullfill with the information.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE if
 *                           a CallLeg object asks for its subscription
 *                           storage, in order to avoid infinity recursion.
 *          maxBuffLen     - The length of the given buffer.
 * Output:  memBuff        - The buffer that will store the Subs' parameters.
 *          pStoredLen     - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus SipSubsStoreActiveSubs(IN  RvSipSubsHandle  hSubs,
                                IN  RvBool           bStandAlone,
                                IN  RvUint32         maxBuffLen,
                                OUT void            *memBuff,
                                OUT RvInt32         *pStoredLen);

/***************************************************************************
 * SipSubsRestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Create a new Subs on state Active, and fill all it's parameters,
 *          which are strored on the given page. (the page was updated in the
 *          SubsStoreActiveSubs function).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff        - The buffer that stores all the Subs' parameters.
 *        buffLen        - The buffer length.
 *        bStandAlone    - Indicates if the Subs is independent.
 *                         This parameter should be RV_FALSE if
 *                         a CallLeg object asks for its subscription
 *                         restoration, in order to avoid infinity recursion.
 * Input/Output:
 *        pSubs   - Pointer to the restored subscription.
 ***************************************************************************/
RvStatus SipSubsRestoreActiveSubs(IN    void            *memBuff,
                                  IN    RvUint32         buffLen,
                                  IN    RvBool           bStandAlone,
                                  INOUT RvSipSubsHandle  hSubs);
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 

/***************************************************************************
* SipSubsRespondProvisional
* ------------------------------------------------------------------------
* General:  Send 1xx on subscribe request (initial subscribe, refresh
*           or unsubscribe).
*           Can be used in the RVSIP_SUBS_STATE_SUBS_RCVD, RVSIP_SUBS_STATE_REFRESH_RCVD
*           or RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD states.
*           Note - According to RFC 3265, sending provisional response on
*           SUBSCRIBE request is not allowed.
*           According to RFC 3515, only 100 response on REFER request is allowed.
* Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
*               RV_ERROR_BADPARAM          - The status code is invalid.
*               RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
*               RV_ERROR_UNKNOWN           - Failed to reject the subscription. (failed
*                                            while trying to send the reject response).
*               RV_OK                      - 1xx was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs      - Handle to the subscription the user wishes to send 1xx for.
*           statusCode - The 1xx value.
*           strReasonPhrase - Reason phrase to be set in the sent message.
*                        (May be NULL, to set a default reason phrase.)
***************************************************************************/
RvStatus SipSubsRespondProvisional(IN  RvSipSubsHandle  hSubs,
                                   IN  RvUint16         statusCode,
                                   IN  RvChar*          strReasonPhrase);

/***************************************************************************
 * SipSubsGetSubscriptionFromTransc
 * ------------------------------------------------------------------------
 * General: Get the subscription object from a transaction
 * Return Value: the subscription object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc     - handle to the transaction.
 ***************************************************************************/
RvSipSubsHandle RVCALLCONV SipSubsGetSubscriptionFromTransc(
											IN  RvSipTranscHandle    hTransc);

#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_SUBS_OBJECT_H */

