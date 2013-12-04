/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD.                   *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              <RvSipCallLeg.h>
 *
 * The Call-Leg functions of the RADVISION SIP Stack enable you to create and
 * manage call-leg objects, connect and disconnect calls and control call-leg
 * parameters.
 * Call-Leg API functions are grouped as follows:
 * The Call-Leg Manager API
 * ------------------------
 * The call-leg manager (Call-legMgr) is in charge of all the call-legs. It is used
 * to set the event handlers of the call-leg module and to create
 * new call-legs.
 *
 * The Call-Leg API
 * -----------------
 * A call-leg represents a SIP call-leg as defined in RFC 2543. This
 * means that a call-leg is defined using the Call-ID, From and To
 * headers. Using the call-leg API, the user can initiate calls, react
 * to incoming calls and disconnect calls. Functions to set and
 * access the call-leg fields are also available in the API. A call-leg
 * is a statefull object and has a set of states associated with it.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/


#ifndef RV_SIP_CALL_LEG_H
#define RV_SIP_CALL_LEG_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLegTypes.h"
#include "RvSipMsg.h"
#include "RvSipPartyHeader.h"
#include "RvSipAddress.h"
#include "RvSipTransaction.h"
#include "RvSipSubscriptionTypes.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipAuthenticator.h"

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#include "RvSipSecurityTypes.h"
#endif

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "RvSipAuthenticationHeader.h"
#endif
/* SPIRENT_END */


/*-----------------------------------------------------------------------*/
/*                CALL_LEG MANAGER  API                                  */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RvSipCallLegMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all call-leg events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the
 *                                        Call-legMgr is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the event handler structure.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - The handle to the Call-legMgr.
 *            pEvHandlers - A pointer to the structure containing application event
 *                        handler pointers.
 *            structSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrSetEvHandlers(
                                   IN  RvSipCallLegMgrHandle  hMgr,
                                   IN  RvSipCallLegEvHandlers *pEvHandlers,
                                   IN  RvInt32               structSize);



/***************************************************************************
 * RvSipCallLegMgrCreateCallLeg
 * ------------------------------------------------------------------------
 * General: Creates a new outgoing call-leg and exchanges handles with the
 *          application. The new call-leg assumes the IDLE state.
 *          To establish a new session:
 *          1. Create a new call-leg with this function.
 *          2. Set the To and From headers, at least.
 *          3. Call the Connect() function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the Call-legMgr is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the call-leg handle is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - The call list is full; a call-leg was not
 *                                   created.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - The handle to the Call-legMgr.
 *            hAppCallLeg - The application handle to the newly created call-leg.
 * Output:     RvSipCallLegHandle -   The RADVISION SIP Stack handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrCreateCallLeg(
                                   IN  RvSipCallLegMgrHandle hCallLegMgr,
                                   IN  RvSipAppCallLegHandle hAppCallLeg,
                                   OUT RvSipCallLegHandle    *hCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                   );

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
int RvSipCallLegGetCct(IN  RvSipCallLegHandle hCallLeg);
void RvSipCallLegSetCct(IN  RvSipCallLegHandle hCallLeg,int cct);
int RvSipCallLegGetURIFlag(IN  RvSipCallLegHandle hCallLeg);
void RvSipCallLegSetURIFlag(IN  RvSipCallLegHandle hCallLeg,int uri_cfg);
const RvChar* RvSipCallLegGetLocalIpStrFromActiveTransc(IN RvSipCallLegHandle hCallLeg);
RvInt32 RvSipCallLegGetLocalPortFromActiveTransc(IN RvSipCallLegHandle hCallLeg);
#endif
//SPIRENT_END

/***************************************************************************
 * RvSipCallLegMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Saves the Call-legMgr handle to the application in the SIP Stack
 *          Call-legMgr.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - The handle to the Stack Call-legMgr.
 *           pAppCallLegMgr - The application Call-legMgr handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrSetAppMgrHandle(
                                   IN RvSipCallLegMgrHandle hCallLegMgr,
                                   IN void*               pAppCallLegMgr);


/***************************************************************************
 * RvSipCallLegMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application Call-legMgr.
 *          You set this handle in the SIP Stack using the
 *          RvSipCallLegMgrSetAppMgrHandle() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - The handle to the SIP Stack Call-legMgr.
 * Output:     pAppCallLegMgr - The application Call-legMgr handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrGetAppMgrHandle(
                                   IN RvSipCallLegMgrHandle hCallLegMgr,
                                   OUT void**               pAppCallLegMgr);

/***************************************************************************
 * RvSipCallLegMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the SIP Stack instance to which this Call-legMgr belongs.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr     - The handle to the SIP Stack Call-legMgr.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the SIP Stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrGetStackInstance(
                                   IN   RvSipCallLegMgrHandle   hCallLegMgr,
                                   OUT  void*       *phStackInstance);

/*-----------------------------------------------------------------------*/
/*                 CALL_LEG API                                          */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
       C A L L  - L E G:  S E S S I O N   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegMake
 * ------------------------------------------------------------------------
 * General: Sets the To and From header in the call-leg and
 *          initiates an outgoing call. This function will cause
 *          an INVITE to be sent out and the Call-leg state machine
 *          will progress to the INVITING state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_BADPARAM - A bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - A bad pointer was given by the application.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Could not send a message to the given
 *                            Request-URI).
 *               RV_OK - An Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to connect
 *          strFrom  - The string with the From party header. For example:
 *                     "From:sip:172.20.1.1:5060"
 *          strTo -   The string with the To party header. For example:
 *                     "To:sip:172.20.5.5:5060"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMake (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvChar*             strFrom,
                                         IN  RvChar*             strTo);

/***************************************************************************
 * RvSipCallLegConnect
 * ------------------------------------------------------------------------
 * General: Initiates an outgoing call. This method may be called
 *          only after the To and From fields were set. Calling
 *          this function causes an INVITE to be sent out and the
 *          Call-leg state machine to progress to the Inviting state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Could not send a message to the given
 *                            Request-URI).
 *               RV_OK - The Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to connect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegConnect (
                                         IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegAccept
 * ------------------------------------------------------------------------
 * General: Called by the application to indicate that it is willing to accept
 *          an incoming call or an incoming re-Invite.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 200 response).
 *               RV_OK       -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to accept.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAccept (
                                        IN  RvSipCallLegHandle   hCallLeg);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvBool RVCALLCONV RvSipCallLegCanAccept (
                                        IN  RvSipCallLegHandle   hCallLeg,
                                        IN RvSipSecAgreeHandle auxsa1,
                                        IN RvSipSecAgreeHandle auxsa2,
                                        IN RvSipSecAgreeHandle auxsa3);

#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegByeAccept
 * ------------------------------------------------------------------------
 * General: Called by the application to indicate that it is willing to accept
 *          an incoming BYE. By default, BYE requests are responded to
 *          automatically with 200 OK. If the application wishes to respond
 *          differently, it should register to the RvSipCallLegByeCreatedEv()
 *          and RvSipCallLegByeStateChangedEv() callbacks. When a BYE is received,
 *          the application will be informed at the RVSIP_CALL_LEG_BYE_REQUEST_RCVD
 *          state. The application should use the RvSipCallLegByeAccept() function
 *          to accept the BYE request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (Failed
 *                                   while trying to send the 200 response.)
 *               RV_OK       -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg whose BYE the user wishes to accept.
 *          hTransc  - The handle to the BYE transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegByeAccept (IN  RvSipCallLegHandle   hCallLeg,
                                                  IN  RvSipTranscHandle    hTransc);


/***************************************************************************
 * RvSipCallLegReject
 * ------------------------------------------------------------------------
 * General: Can be used in the OFFERING state to reject an incoming call.
 *          This function can also be used to reject a modify (re-Invite)
 *          request received by a connected call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (Failed
 *                                     while trying to send the reject response.)
 *               RV_OK -          The reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to reject
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReject (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16 status);

/***************************************************************************
 * RvSipCallLegByeReject
 * ------------------------------------------------------------------------
 * General: Rejects a BYE request.By default,BYE requests are responded to
 *          automatically with 200 OK.If the application wishes to respond
 *          differently, it should register to the RvSipCallLegByeCreatedEv()
 *          and RvSipCallLegByeStateChangedEv() callbacks. When a BYE is received,
            the application will be informed at the RVSIP_CALL_LEG_BYE_REQUEST_RCVD
            state. The application should use the RvSipCallLegByeReject() function
            to accept the BYE request.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (Failed
 *                                     while trying to send the reject response.)
 *               RV_OK -          The reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to reject.
 *          hTransc  - The handle to the BYE transaction.
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegByeReject (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvSipTranscHandle    hTransc,
                                         IN  RvUint16            status);


/***************************************************************************
 * RvSipCallLegProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Sends a provisional response (1xx class) to the remote party.
 *          This function can be called when ever a request is received, for
 *          example, in the OFFERING state.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send the provisional response.
 *               RV_OK - The provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            status -   The provisional response status code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegProvisionalResponse (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16 status);

/***************************************************************************
 * RvSipCallLegAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request from the call-leg to the remote party.
 *          When the SIP Stack is configured to work in a manual ACK mode,
 *          the call-leg will not send the ACK message after receiving a 2xx
 *          response. The application should use the RvSipCallLegAck() function
 *          to trigger the call-leg to send the ACK. This function can be called
 *          only in the REMOTE_ACCEPTED state. It sends an ACK only for the initial
 *          Invite request. You can use this function for re-Invite only if you configured
 *          the SIP Stack to work with the "old Invite" behavior (according to bis-2543).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAck (
                                         IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegProvisionalResponseReliable
 * ------------------------------------------------------------------------
 * General: Sends a reliable provisional response (1xx class other than 100)
 *          to the remote party. This function can be called when an Invite
 *          request is received, for example, in the OFFERING state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            status -   The provisional response status code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegProvisionalResponseReliable (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16            status);

/***************************************************************************
 * RvSipCallLegSendPrack
 * ------------------------------------------------------------------------
 * General:When the SIP Stack is configured to work in a manual PRACK mode,
 *         the application is responsible for generating the PRACK message
 *         whenever a reliable provisional response is received. When a reliable
 *         provisional response is received, the Call-leg PRACK state machine
 *         assumes the REL_PROV_RESPONSE_RCVD state. You should then call the
 *         RvSipCallLegSendPrack() function to send the PRACK message to the
 *         remote party. The Call-leg PRACK state machine will then assume
 *         the PRACK_SENT state.
 *         Note: The SIP Stack is responsible for adding the RAck header
 *         to the PRACK message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The transaction handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSendPrack(
                                    IN RvSipCallLegHandle            hCallLeg);

/***************************************************************************
 * RvSipCallLegSendPrackResponse
 * ------------------------------------------------------------------------
 * General: When the SIP Stack is configured to work in a manual PRACK mode,
 *          the application is responsible for responding to any PRACK request
 *          that is received for a previously-sent reliable provisional response.
 *          When a PRACK request is received, the Call-leg PRACK state machine
 *          assumes the PRACK_RCVD state. You should then call the
 *          RvSipCallLegSendPrackResponse() function to send a response to the
 *          PRACK request. The Call-leg PRACK state machine will then assume
 *          the PRACK_FINAL_RESPONSE_SENT state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The transaction handle.
 *            responseCode - The response code to send.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSendPrackResponse(
                                    IN RvSipCallLegHandle            hCallLeg,
                                    IN RvUint16                     responseCode);


/***************************************************************************
 * RvSipCallLegDisconnect
 * ------------------------------------------------------------------------
 * General: Causes the call to disconnect. RvSipCallLegDisconnect() may be
 *          called in any state, but you may not use this
 *          function when the stack is in the middle of processing one of
 *          the following callbacks:
 *          pfnMsgReceivedEvHandler, pfnMsgToSendEvHandler,
 *          pfnFinalDestResolvedEvHandler, pfnNewConnInUseEvHandler.
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 *          The behavior of the function depends on the following call-leg states:
 *          -INVITING: CANCEL is sent and the call moves to the CANCELLING state.
 *          -CONNECTED and ACCEPTED: BYE is sent and the call moves to the
 *           DISCONNECTING state.
 *          -OFFERING: The incoming INVITE is rejected with status code 403.
 *          -IDLE, DISCONNECTING, DISCONNECTED, UNAUTHENTICATED, REDIRECTED:
 *           The call is terminated.
 *          If the functions fail to send the BYE request, the call-leg will be terminated.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to disconnect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDisconnect (
                                        IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a call-leg without sending any messages (Cancel or
 *          Bye). The call-leg will assume the TERMINATED state.
 *          Calling this function will cause an abnormal
 *          termination. All transactions related to the call-leg will be
 *          terminated as well.
 *            
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnCallLegCreatedEvHandler, pfnReInviteCreatedEvHandler,
 *          pfnTranscCreatedEvHandler, pfnByeCreatedEvHandler, pfnSessionTimerNegotiationFaultEvHandler
 *          pfnSessionTimerRefreshAlertEvHandler, pfnSigCompMsgNotRespondedEvHandler,
 *          pfnNestedInitialReqRcvdEvHandler, pfnNewConnInUseEvHandler, 
 *          pfnCallLegCreatedDueToForkingEvHandler, pfnProvisionalResponseRcvdEvHandler
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTerminate (
                                            IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegDetachOwner
 * ------------------------------------------------------------------------
 * General: Detaches the call-leg owner. After calling this function the user
 *          will stop receiving events for this call-leg.This function can be
 *          called only when the object is in the TERMINATED state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg whose owner the user wishes
 *                       to detach.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDetachOwner(IN RvSipCallLegHandle  hCallLeg);

/***************************************************************************
 * RvSipCallLegCancel
 * ------------------------------------------------------------------------
 * General: Cancels an INVITE request (or re-Invite request). Calling this
 *          function causes a CANCEL message to be sent to the remote party.
 *          You can call this function only in the INVITING state, in the
 *          following call-leg states:
 *          -RVSIP_CALL_LEG_STATE_INVITING
 *          -RVSIP_CALL_LEG_STATE_PROCEEDING
 *          -PROCEEDING_TIMEOUT
 *          and in the following MODIFY states:
 *          -RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT
 *          -RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING
 *          -RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT
 *
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnMsgReceivedEvHandler, pfnMsgToSendEvHandler,
 *          pfnFinalDestResolvedEvHandler, pfnNewConnInUseEvHandler.
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to cancel.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegCancel (
                                    IN  RvSipCallLegHandle   hCallLeg);


/***************************************************************************
 * RvSipCallLegReInviteCreate
 * ------------------------------------------------------------------------
 * General: Creates a new re-Invite object in a call-leg.
 *          A re-invite object may be created in the following situations:
 *          -The call-leg had already sent/received the 2xx response
 *           for the initial Invite request (the call-leg state is
 *           CONNECTED or ACCEPTED or REMOTE-ACCEPTED),
 *          -There are no other pending re-Invite transactions (no other
 *           re-Invite objects that are waiting for a final response).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to modify.
 *            hAppReInvite - The application handle to the new re-Invite object.
 * Output:    phReInvite - A pointer to the new re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteCreate(
                                     IN   RvSipCallLegHandle          hCallLeg,
                                     IN   RvSipAppCallLegInviteHandle hAppReInvite,
                                     OUT  RvSipCallLegInviteHandle*   phReInvite);

/***************************************************************************
 * RvSipCallLegReInviteSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the re-Invite application handle. Usually the application
 *          replaces handles with the SIP Stack in the RvSipCallLegReInviteCreatedEv()
            callback or the RvSipCallLegReInviteCreate() function. This function
            is used if the application wishes to set a new application handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The handle to the call-leg.
 *            hReInvite    - The handle to the re-Invite object.
 *            hAppReInvite - The handle to the application re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteSetAppHandle (
                                IN  RvSipCallLegHandle          hCallLeg,
                                IN  RvSipCallLegInviteHandle    hReInvite,
                                IN  RvSipAppCallLegInviteHandle hAppReInvite);

/***************************************************************************
 * RvSipCallLegReInviteRequest
 * ------------------------------------------------------------------------
 * General: Sends a re-INVITE request from a call-leg to the remote party.
 *			NOTE: This function does not refer to the parameters of the
 *          Session Timer call-leg. Thus, when used during a Session Timer
 *          call, it turns off the Session Timer mechanism. Consequently,
 *          to keep the mechanism up, use the
 *          RvSipCallLegSessionTimerInviteRefresh() function instead.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg the user wishes to modify.
 *            hReInvite - The handle to the re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteRequest(
                                     IN   RvSipCallLegHandle         hCallLeg,
                                     IN   RvSipCallLegInviteHandle   hReInvite );

/***************************************************************************
 * RvSipCallLegReInviteAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request for 2xx response on re-Invite. When the SIP Stack
 *          is configured to work in manual ACK mode, the call-leg will not
 *          send the ACK message after receiving a 2xx response by itself.
 *          The application should use this function to trigger the call-leg
            to send the ACK. This function can be called only in the
            MODIFY_REINVITE_REMOTE_ACCEPTED state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg.
 *            hReInvite - The handle to the re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteAck (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite);

/***************************************************************************
 * RvSipCallLegReInviteTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a re-invite object without sending any messages (CANCEL
 *          or BYE). The re-invite object will assume the TERMINATED state.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            hReInvite - The handle to the re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteTerminate (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite);

/***************************************************************************
 * RvSipCallLegReInviteGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the current state of the re-Invite of the call-leg.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hCallLeg - The call-leg handle.
 *             hReInvite - The handle to the re-Invite object.
 * Output:     peState -  The current state of the re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteGetCurrentState (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite,
                                            OUT RvSipCallLegModifyState  *peState);

/***************************************************************************
 * RvSipCallLegTranscCreate
 * ------------------------------------------------------------------------
 * General:Creates a new general transaction that is related to the supplied
 *         call-leg.
 *         The transaction will have the call-leg characteristics such as:
 *         To Header, From header, Call-ID and local and outbound addresses.
 *         The application can define an application handle to the transaction
 *         and supply it to the SIP Stack when calling this function.
 *         The application handle will be supplied back to the application
 *         when the  RvSipCallLegTranscStateChangedEv() callback will be called.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg that will send the new request.
 *          hAppTransc - The application handle to the new transaction.
 * Output:  hTransc    - The handle to the newly created transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscCreate(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipAppTranscHandle hAppTransc,
                                           OUT RvSipTranscHandle    *hTransc);


/***************************************************************************
 * RvSipCallLegTranscRequest
 * ------------------------------------------------------------------------
 * General: Sends a request message with a given method using a transaction.
 *          You can use this function in any call-leg state for sending requests,
 *          such as INFO. The request will have the To header, From header and
 *          Call-ID of the call-leg, and will be sent with a correct CSeq step.
 *          The request will be record-routed if needed.
 *          Note:Before calling this function, you should create a new call-leg
 *          transaction using the RvSipCallLegTranscCreate() function. You can
 *          then use the transaction outbound message mechanism to add headers
 *          and a body to the outgoing request. If you supply the function with
 *          a NULL transaction, the SIP Stack will create a new call-leg transaction
 *          automatically. In this case you will not be able to use the transaction
 *          outbound message.
 *          Note: A transaction that was supplied by the application
 *          will not be terminated if this function fails. It is the responsibility
 *          of the application to terminate the transaction using the
 *          RvSipCallLegTranscTerminate() function.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that will send the new request.
 *          strMethod - A string with the request method.
 * Input Output:  hTransc - The handle to the newly created transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscRequest(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvChar*             strMethod,
                                           INOUT RvSipTranscHandle    *hTransc);


/***************************************************************************
 * RvSipCallLegTranscResponse
 * ------------------------------------------------------------------------
 * General: Sends a response to a call-leg related transaction. When a call-leg
 *          receives a general request, such as INFO (but not BYE or PRACK),
 *          the call-leg first notifies the application that a new call-leg
 *          transaction was created using the RvSipCallLegTranscCreatedEv()
 *          callback. At this stage, the application can specify whether or
 *          not it wishes to handle the transaction, and the application can
 *          also replace handles with the SIP Stack. The call-leg will then
 *          notify the application about the new transaction state,
 *          GENERAL_REQUEST_RCVD, using the RvSipCallLegTranscStateChangedEv()
 *          callback. In this state, the application should use the
 *          RvSipCallLegTranscResponse() function to send a response to the
 *          request.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that will send the new request.
 *          hTransc - The handle to the transaction.
 *          status - The response code that is sent in the response message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscResponse(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc,
                                           IN  RvUint16            status);

/***************************************************************************
 * RvSipCallLegTranscTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a transaction related to a specified call-leg. You
 *         can use this function only for call-leg general transactions
 *         that are handled by the application. You can also use this function
 *         to terminate a BYE transaction in case you registered to the
 *         RvSipCallLegByeCreatedEv() and RvSipCallLegByeStateChangedEv()
 *         callbacks.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg to which the terminated
 *                       transaction relates.
 *          hTransc  - The handle to the transaction that is terminated.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscTerminate(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc);

/***************************************************************************
 * RvSipCallLegNoActiveTransc
 * ------------------------------------------------------------------------
 * General: This function is for B2B usage.
 *          The function verifies that there is no transaction in an active state
 *          in the call-leg.
 *          For every transaction in the call-leg that is not in active state,
 *          the function sets the AppTranscHandle to be NULL.
 *
 * Return Value: RV_OK - If there is no active transaction.
 *               RV_ERROR_UNKNOWN - If there is an active transaction.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegNoActiveTransc(
                              IN  RvSipCallLegHandle           hCallLeg);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipCallLegAuthenticate - client authentication.
 * ------------------------------------------------------------------------
 * General: Sends a request with authentication information.
 *          When a call-leg receives a 401 or 407 response indicating that a
 *          request was not authenticated by the server or proxy, the
 *          call-leg assumes the UNAUTHENTICATED state.
 *          You may use RvSipCallLegAuthenticate() in the UNAUTHENTICATED
 *          state to re-send the request with authentication information.
 *          You can use this function to authenticate both the INVITE
 *          and BYE requests.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Cannot send request due to a resource problem.
 *               RV_ERROR_UNKNOWN - An error occurred. The request was not sent.
 *               RV_OK - The request was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to authenticate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthenticate(
                                           IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegAuthBegin - Server authentication
 * ------------------------------------------------------------------------
 * General: Begins the server authentication process by challenging an incoming
 *          request. If the request is an active transaction (Invite, Bye),
 *          hCallLeg handle should be given, and hTransaction should be NULL.
 *          For a general request, hTransaction should be given, and
 *          hCallLeg should be NULL.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to challenge
 *          hTransaction - The handle to the transaction user wishes to challenge
 *                         (in case of a general request).
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegAuthBegin(
                                           IN  RvSipCallLegHandle hCallLeg,
                                           IN  RvSipTranscHandle  hTransaction);

/***************************************************************************
 * RvSipCallLegAuthProceed - Server authentication
 * ------------------------------------------------------------------------
 * General: Orders the SIP Stack to proceed with the authentication process.
 *          The information on how to proceed is given to the SIP Stack in
 *          the action parameter. The action options are:
 *
 *          -RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD
 *          Checks the given Authorization header with the given password.
 *
 *          -RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          Orders to stop the authentication process with a success
 *          indication possibly because the user checked the Authorization
 *          header and it is correct. (This will cause AuthCompletedEv to
 *          be called, with Success status.)
 *
 *          -RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          The user wants to stop the loop that searches for Authorization
 *          headers. (This will cause AuthCompletedEv to be called, with Failure
 *          status).
 *
 *          -RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          Orders to skip the given header, and continue the authentication
 *          procedure with the next header, if it exists. (This will cause
 *          AuthCredentialFoundEv to be called, or AuthCompletedEv with Failure
 *          status, if there are no more Authorization headers.)
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       The call-leg handle.
 *          hTransaction -   The transaction handle.
 *          action -         With which action to proceed. (See above.)
 *          hAuthorization - The handle of the Authorization header for which
 *                           the function will check authentication. (Needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD, otherwise NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (Needed if the action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD, otherwise NULL.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthProceed(
                                   IN  RvSipCallLegHandle             hCallLeg,
                                   IN  RvSipTranscHandle              hTransaction,
                                   IN  RvSipTransactionAuthAction     action,
                                   IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                   IN  RvChar                        *password);

/***************************************************************************
 * RvSipCallLegRespondUnauthenticated - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends a 401/407 response. This function adds a copy of the given
 *          header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       The call-leg handle.
 *          hTransaction -   The transaction handle.
 *          responseCode -   401 or 407.
 *          strReasonPhrase - May be NULL for the default reason phrase.
 *          headerType -     The type of the given header.
 *          hHeader -        A pointer to the header to be set in the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegRespondUnauthenticated(
                                   IN  RvSipCallLegHandle   hCallLeg,
                                   IN  RvSipTranscHandle    hTransaction,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar*             strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader);

/***************************************************************************
 * RvSipCallLegRespondUnauthenticatedDigest - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends a 401/407 response. This function builds an authentication
 *          header containing all given parameters, and adds it to the
 *          response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -     The call-leg handle.
 *          hTransaction -   The transaction handle.
 *          responseCode -   401 or 407.
 *          strReasonPhrase - The reason phrase for this response code.
 *          strRealm -       The realm is mandatory.
 *          strDomain -      An optional string that may be NULL.
 *          strNonce -       An optional string that may be NULL.
 *          strOpaque -      An optional string that may be NULL.
 *          bStale -         TRUE or FALSE.
 *          eAlgorithm -     The enumeration of the algorithm. If
 *                           RVSIP_AUTH_ALGORITHM_OTHER, the algorithm value
 *                           is taken from the next argument.
 *          strAlgorithm -   The string of the algorithm. This parameter will
 *                           be set only if the eAlgorithm parameter is set to
 *                           be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           The enumeration of Quality-of-Protection (Qop).
 *                           If RVSIP_AUTH_QOP_OTHER, the Qop value will be
 *                           taken from the next argument.
 *          strQop -         The string of Quality-of-Protection (Qop). This
 *                           parameter will be set only if the eQop parameter
 *                           is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegRespondUnauthenticatedDigest(
                                   IN  RvSipCallLegHandle hCallLeg,
                                   IN  RvSipTranscHandle  hTransaction,
                                   IN  RvUint16          responseCode,
                                   IN  RvChar           *strReasonPhrase,
                                   IN  RvChar           *strRealm,
                                   IN  RvChar           *strDomain,
                                   IN  RvChar           *strNonce,
                                   IN  RvChar           *strOpaque,
                                   IN  RvBool            bStale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop);
#endif /* #ifdef RV_SIP_AUTH_ON */


/***************************************************************************
 * RvSipCallLegReplacesGetMatchedCallExt
 * ------------------------------------------------------------------------
 * General: Called when the call-leg is in the OFFERING state.
 *          This function searches for the call-leg that has the same Call-ID,
 *          to tag and from tag as the Replaces header in the original call-leg.
 *          If a matched call-leg is found, this call-leg is returned as
 *          the function output. Otherwise this pointer will be NULL.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that received the INVITE with
 *                       the Replaces header.
 * Output:  peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog does not fit.
 *                     The application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces
 *                     header.
 *          hMatchedCallLeg - The handle to the call-leg matched the Replaces header.
 *                     If there is no such call-leg, this handle will be NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesGetMatchedCallExt(
                                  IN  RvSipCallLegHandle         hCallLeg,
                                  OUT RvSipCallLegReplacesReason *peReason,
                                  OUT RvSipCallLegHandle        *hMatchedCallLeg);

/***************************************************************************
 * RvSipCallLegReplacesCompareReplacesToCallLeg
 * ------------------------------------------------------------------------
 * General: A utility function that compares a call-leg to a Replaces header.
 *          The call-leg and Replaces header are equal if the Call-ID,
 *          from-tag and to-tag are equal.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - The handle to the call-leg to compare with the Replaces header.
 *          hReplacesHeader - The handle to a Replaces header.
 * Output:  pbIsEqual       - The result of the comparison. RV_TRUE if the call-leg and
 *                            Replaces header are equal, otherwise RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus    RVCALLCONV RvSipCallLegReplacesCompareReplacesToCallLeg(IN  RvSipCallLegHandle        hCallLeg,
                                                                        IN  RvSipReplacesHeaderHandle hReplacesHeader,
                                                                        OUT RvBool                  *pbIsEqual);


/*-----------------------------------------------------------------------
       C A L L  - L E G:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the call-leg page, and returns the new
 *         element handle.The application may use this function to allocate
 *         a message header or a message  address. It should then fill the
 *         element information and set it back to the call-leg using the
 *         relevant Set function.
 *
 *         The function supports the following elements:
 *         Party: You should set these headers back with the  RvSipCallLegSetToHeader()
 *                 or RvSipCallLegSetFromHeader() API functions.
 *         Replaces: You should set this header back with the RvSipCallLegSetReplaces()
 *                  API function.
 *         Authorization: You should set these headers back with the
 *                        RvSipCallLegSetInitialAuthorization() API function that is
 *                        available in the IMS add-on only.
 *         Address of any type: You should set the address back using the
 *                                RvSipCallLegSetRemoteContactAddress() or
 *                                RvSipCallLegSetLocalContactAddress() API functions.

 *         Note: You may use this function only on initial state (before sending a message).
 *         In any other state you must construct the header on an application page,
 *         and then set it to the SIP Stack object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg object.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - The handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewMsgElementHandle (
                                      IN   RvSipCallLegHandle   hCallLeg,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader);

/***************************************************************************
 * RvSipCallLegSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the call-leg Call-ID.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - The Call-ID was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The SIP Stack handle to the call-leg.
 *            strCallId - A NULL terminating string with the new Call-ID.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCallId (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvChar              *strCallId);



/***************************************************************************
 * RvSipCallLegGetCallId
 * ------------------------------------------------------------------------
 * General:Copies the call-leg Call-ID into a given buffer.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the call-leg.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The call-leg does not yet contain a Call-ID.
 *               RV_OK            - The Call-ID was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The SIP Stack handle to the call-leg.
 *          bufSize    - The size of the application buffer for the Call-Id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The actual size of the Call-ID.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCallId (
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvInt32              bufSize,
                            OUT RvChar              *strCallId,
                            OUT RvInt32             *actualSize);
/***************************************************************************
 * RvSipCallLegSetCSeq
 * ------------------------------------------------------------------------
 * General: Sets the outgoing call-leg CSeq step counter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The SIP Stack handle to the call-leg.
 *            cseq - The CSeq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCSeq (
                                      IN  RvSipCallLegHandle  hCallLeg,
                                      IN  RvInt32            cseq);

/***************************************************************************
 * RvSipCallLegGetCSeq
 * ------------------------------------------------------------------------
 * General: Gets the outgoing call-leg CSeq step counter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The SIP Stack handle to the call-leg.
 * Output:     cseq       - The CSeq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCSeq (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *cseq);

/***************************************************************************
 * RvSipCallLegGetRemoteCSeq
 * ------------------------------------------------------------------------
 * General: Gets the incoming call-leg CSeq step counter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The SIP Stack handle to the call-leg.
 * Output:     cseq       - The remote cseq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetRemoteCSeq (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *cseq);

/***************************************************************************
 * RvSipCallLegSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the From header associated with the call-leg. Note
 *          that attempting to alter the From header after a call has
 *          been initiated may cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - The From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            hFrom    - The handle to an application constructed from header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetFromHeader (
                                      IN  RvSipCallLegHandle      hCallLeg,
                                      IN  RvSipPartyHeaderHandle  hFrom);


/***************************************************************************
 * RvSipCallLegGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the From header associated with the call-leg.
 *          Attempting to alter the From header after the
 *          call has been initiated may cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_OK        - The From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:     phFrom -   A pointer to the call-leg From header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetFromHeader (
                                      IN    RvSipCallLegHandle      hCallLeg,
                                      OUT RvSipPartyHeaderHandle    *phFrom);


/***************************************************************************
 * RvSipCallLegSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the call-leg. Note
 *          that attempting to alter the To header after the call has
 *          been initiated may cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - The to header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            hTo      - The handle to an application constructed To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetToHeader (
                                      IN  RvSipCallLegHandle       hCallLeg,
                                      IN  RvSipPartyHeaderHandle   hTo);



/***************************************************************************
 * RvSipCallLegGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To address associated with the call.
 *          Attempting to alter the To address after
 *          call has been initiated might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:     phTo     - A pointer to the call-leg To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetToHeader (
                                      IN    RvSipCallLegHandle        hCallLeg,
                                      OUT   RvSipPartyHeaderHandle    *phTo);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipCallLegGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given CallLeg object.
 *          The call-id is kept on the Call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV RvSipCallLegGenerateCallId(IN RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * RvSipCallLegGetReqURI
 * ------------------------------------------------------------------------
 * General: Returns the Request URI address associated with the call.
 * Return Value: RV_InvalidHandle  - The call-leg handle is invalid.
 *               RV_BadPointer     - Bad pointer was given by the application. 
 *               RV_Success        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg - Handle to the call-leg.
 * Output: 	hReqUri  - Pointer to the Request URI address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReqURI (
                                      IN    RvSipCallLegHandle        hCallLeg,
                                      OUT   RvSipAddressHandle    *hReqUri);

/***************************************************************************
 * RvSipCallLegRouteListAdd
 * ------------------------------------------------------------------------
 * General: Given route value or values (the values are seperated by comma), 
 *          the function adds Route header(s) to the given hCallLeg.
 *          The header(s) are always inserted to the head of the route list.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 *            routeStr - Route header value.
 ***************************************************************************/
RVAPI RvStatus RvSipCallLegRouteListAdd(
                        IN  RvSipCallLegHandle      hCallLeg,
                        IN  RvChar                  *nexthop,
                        IN  RvChar                  *routeStr);

#endif /* defined(UPDATED_BY_SPIRENT) */ 

/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegSetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Sets the contact address of the remote party. This is the address
 *          with which the remote party may be contacted. This function
 *          may be used for outgoing calls when the user wishes to use a
 *          Request-URI that is different from the To header of the call-leg.
 *          You should set the remote contact address only in the IDLE state.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - The new remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            hContactAddress - The handle to the Address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetRemoteContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvSipAddressHandle   hContactAddress);




/***************************************************************************
 * RvSipCallLegGetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Gets the contact address of the remote party. This is the address
 *          the remote party supplies for contact in future requests.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg         - The handle to the call-leg.
 * Output:     phContactAddress - A pointer to the remote contact address of the
 *                                call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetRemoteContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle   *phContactAddress);


/***************************************************************************
 * RvSipCallLegSetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Sets the local contact address that the SIP Stack uses to identify
 *          itself to the remote party. The remote party uses the contact address
 *          to directly contact the local party..
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - The new local contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg
 *            hContactAddress - The handle to the local contact Address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetLocalContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvSipAddressHandle   hContactAddress);


/***************************************************************************
 * RvSipCallLegGetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Gets the local contact address that the SIP Stack uses to
 *          identify itself to the remote party.  If no value is supplied,
 *          the From header address part is taken. The remote party uses
 *          the local contact address to directly contact the local party.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - The handle to the call-leg.
 * Output:     phContactAddress - The handle to the local contact Address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetLocalContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle   *phContactAddress);



/***************************************************************************
 * RvSipCallLegSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the call-leg persistency definition at runtime.
 * This function receives a Boolean value that indicates whether or not the
 * application wishes this call-leg to be persistent. A persistent call-leg
 * object will apply its persistency to the transactions it creates. A persistent
 * transaction will try to locate a suitable connection in the connection hash before
 * opening a new connection. A persistent call-leg will also try to use the
 * same connection for all outgoing requests by applying the last-used connection
 * object to the next-created transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLeg - The call-leg handle
 *          bIsPersistent - Determines the call-leg persistency definition.
 *                          RV_TRUE indicates that the call-leg is persistent.
 *                          Otherwise, RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetPersistency(
                           IN RvSipCallLegHandle       hCallLeg,
                           IN RvBool                  bIsPersistent);

/***************************************************************************
 * RvSipCallLegGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the call-leg persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLeg - The call-leg handle
 * Output:  pbIsPersistent - The call-leg persistency definition.
 *                           RV_TRUE indicates that the call-leg is persistent. Otherwise,
 *                           RV_FALSE.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetPersistency(
        IN  RvSipCallLegHandle                   hCallLeg,
        OUT RvBool                             *pbIsPersistent);


/***************************************************************************
 * RvSipCallLegSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the call-leg transactions.
 * The call-leg object will hold this connection in its internal database.
 * Whenever the call-leg creates a new transaction, it supplies the
 * transaction with the given connection. The transaction will use the connection
 * only if it fits the local and remote addresses of the transaction. Otherwise, the
 * transaction will either locate a suitable connection in the connection hash or
 * create a new connection. The call-leg object will be informed that the
 * supplied connection did not fit, and that a different connection was used and will
 * update its database.
 * Note: this function can be used only if the call-leg is persistent.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg
 *          hConn - The handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetConnection(
                   IN  RvSipCallLegHandle                hCallLeg,
                   IN  RvSipTransportConnectionHandle   hConn);

/***************************************************************************
 * RvSipCallLegGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that the call-leg transactions are currently using.
 * Note: Only persistent call-legs keep the currently-used connection. NULL
 * will be returned if the call-leg is not persistent.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:    phConn - The handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetConnection(
                            IN  RvSipCallLegHandle             hCallLeg,
                           OUT  RvSipTransportConnectionHandle *phConn);

/***************************************************************************
 * RvSipCallLegGetActiveTransmitter
 * ------------------------------------------------------------------------
 * General: Returns the transmitter that is currently being used by the
 *          call-leg. This function may be used only inside the final-dest-resolved
 *          callback.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeghTransc - Handle to the call-leg.
 * Output:    phTrx - Handle to the currently used transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetActiveTransmitter(
                            IN  RvSipCallLegHandle      hCallLeg,
                           OUT  RvSipTransmitterHandle *phTrx);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvSipCallLegGetActiveTransaction(
                            IN  RvSipCallLegHandle      hCallLeg,
                           OUT  RvSipTranscHandle *phTransc);

RVAPI RvStatus RVCALLCONV RvSipCallLegGetMsgRcvdTransaction(
                            IN  RvSipCallLegHandle      hCallLeg,
                           OUT  RvSipTranscHandle *phTransc);

#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegGetReplacesStatus
 * ------------------------------------------------------------------------
 * General: Returns the replaces option tag status of a received INVITE/REFER request.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The call-leg handle.
 * Output:  replacesStatus - The replaces status received in the Invite/REFER request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReplacesStatus(
                                    IN  RvSipCallLegHandle             hCallLeg,
                                    OUT RvSipCallLegReplacesStatus    *replacesStatus);

/***************************************************************************
 * RvSipCallLegGetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Gets the Replaces header from a call-leg. This function should be called
 *          before sending an Invite to get the Replaces header that the REFER message
 *          set, if it exists. Or the function should be called from the evStateChanged
 *          when receiving an Invite, to know if the Invite contained a Replaces header.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - A new remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:    hReplacesHeader - The returned Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReplacesHeader(IN  RvSipCallLegHandle        hCallLeg,
                                                         OUT RvSipReplacesHeaderHandle *phReplacesHeader);

/***************************************************************************
 * RvSipCallLegSetReplacesHeader
 * ------------------------------------------------------------------------
 * General:Sets the Replaces header in a call-leg. This function can be called before
 *         sending the INVITE request, when the call is in the IDLE state. The
 *         application should call this function to add a Replaces header to the
 *         INVITE request. If the application does not want to add the Replaces
 *         header that was received in the Refer to the Invite (that is triggered
 *         by the Refer), it should call this function with NULL as the Replaces header.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - A new remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            hReplacesHeader - The handle to a Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetReplacesHeader(IN  RvSipCallLegHandle        hCallLeg,
                                                         IN  RvSipReplacesHeaderHandle hReplacesHeader);


/***************************************************************************
 * RvSipCallLegGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the current state of the call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE - If the given call-leg handle is invalid
 *               RV_ERROR_NULLPTR    - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     peState -  The current state of the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrentState (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipCallLegState    *peState);



/***************************************************************************
 * RvSipCallLegSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the call-leg application handle. Usually the application
 *          replaces handles with the SIP Stack in the RvSipCallLegCreatedEv()
 *          callback or the RvSipCallLegMgrCreateCallLeg() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - The handle to the call-leg.
 *            hAppCallLeg - The new application handle to the call-leg
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetAppHandle (
                                      IN  RvSipCallLegHandle     hCallLeg,
                                      IN  RvSipAppCallLegHandle  hAppCallLeg);

/***************************************************************************
 * RvSipCallLegGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:     phAppCallLeg     - The application handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetAppHandle (
                                      IN    RvSipCallLegHandle    hCallLeg,
                                      OUT   RvSipAppCallLegHandle *phAppCallLeg);

/***************************************************************************
 * RvSipCallLegGetDirection
 * ------------------------------------------------------------------------
 * General: Queries the call-leg direction. A call-leg can be either an incoming
 *          or outgoing call. When you create a call, it is always an outgoing call.
 *          If the call is created because an Invite has arrived, the call is an
 *          incoming call.
 * Return Value: RV_ERROR_INVALID_HANDLE - If the given call-leg handle is invalid.
 *               RV_ERROR_NULLPTR    - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -     The call-leg handle.
 * Output:     *peDirection - The call-leg direction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetDirection (
                                 IN  RvSipCallLegHandle     hCallLeg,
                                 OUT RvSipCallLegDirection  *peDirection);

/***************************************************************************
 * RvSipCallLegGetTranscByMsg
 * ------------------------------------------------------------------------
 * General: Gets the transaction that owns a given message.
 *          Use this function to find the transaction handle to a specific
 *          message.
 *          Note: You cannot get Cancel transactions with this function.
 * Return Value: RV_ERROR_INVALID_HANDLE - If the given call-leg handle is invalid.
 *               RV_ERROR_NULLPTR    - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 *          hMsg     - The message handle.
 *          bIsMsgRcvd - RV_TRUE if this is a received message. Otherwise, RV_FALSE.
 * Output:     phTransc -  The handle to the transaction to which this message belongs.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetTranscByMsg (
                                 IN  RvSipCallLegHandle     hCallLeg,
                                 IN  RvSipMsgHandle         hMsg,
                                 IN  RvBool                bIsMsgRcvd,
                                 OUT RvSipTranscHandle      *phTransc);

/***************************************************************************
 * RvSipCallLegGet100RelStatus
 * ------------------------------------------------------------------------
 * General: Returns the 100rel option tag status of a received INVITE request.
 *          The 100rel option tag indicates whether the remote party
 *          Support/Require the PRACK extension. In case of a Require status,
 *          the application should use the RvSipCallLegProvisionalResponseReliable
 *          function and send a reliable provisional response.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The transaction handle.
 * Output:  relStatus - The reliable status received in the INVITE request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGet100RelStatus(
                                    IN RvSipCallLegHandle            hCallLeg,
                                    OUT RvSipTransaction100RelStatus *relStatus);
/***************************************************************************
 * RvSipCallLegGetCallLegMgr
 * ------------------------------------------------------------------------
 * General: Returns the call-leg manager (Call-legMgr) handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The call-leg handle.
 * Output:  phCallLegMgr - The Call-legMgr handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCallLegMgr(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    OUT RvSipCallLegMgrHandle         *phCallLegMgr);


/***************************************************************************
 * RvSipCallLegGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the call-leg. You can call
 *          this function from the state changed call back function,
 *          RvSipCallLegStateChangedEv(), when the new state indicates that
 *          a message was received. If there is no valid received message,
 *          NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     phMsg    - A pointer to the received message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReceivedMsg(
                                            IN  RvSipCallLegHandle  hCallLeg,
                                             OUT RvSipMsgHandle       *phMsg);

/***************************************************************************
 * RvSipCallLegGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the call-leg. You
 *          can call this function before calling a function that causes a
 *          message to be sent, such as RvSipCallLegConnect(),
 *          RvSipCallLegAccept(), and RvSipCallLegReject().The function
 *          returns an empty message object. You can fill the message object
 *          with headers and a body and then call the function. The call-leg
 *          will add more headers to the same message object and the message
 *          will be sent.
 *          Note: You must not use this function to add headers that are part of
 *          the call-leg or the transaction key such as To, From, Call-ID
 *          and Via. Doing so will cause unexpected behavior of the call-leg.
 *          Note: If your application is multithreaded, you must lock your application
 *          object before calling the RvSipCallLegGetOutboundMsg() function and
 *          unlock it only after calling the function that sends the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     phMsg   -  A pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     OUT RvSipMsgHandle            *phMsg);


/***************************************************************************
 * RvSipCallLegResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the call-leg to NULL. If the
 *          call-leg is about to send a message, it will create a new message
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegResetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg);

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT 
/***************************************************************************
 * RvSipCallLegMgrMatchTranscToCallLeg
 * ------------------------------------------------------------------------
 * General: The function checks if the given transaction matches one of the
 *          call-legs in the call-legs hash.
 *          If it matches, the function returns the handle of the call-leg.
 *          If not, the function returns NULL.
 *          The function handles only incoming transactions.
 *          This is an internal function for the RV SIP Server. no need to document.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc    - The handle to the transaction we want to check.
 ***************************************************************************/
RVAPI RvSipCallLegHandle RVCALLCONV RvSipCallLegMgrMatchTranscToCallLeg(
                                   IN   RvSipCallLegMgrHandle hCallLegMgr,
                                   IN   RvSipTranscHandle      hTransc);
#endif /*#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* RvSipCallLegGetSubscription
* ------------------------------------------------------------------------
* General: Gets a subscription handle from call-leg subscriptions list.
*          Users can use the location and hItem parameters to go over the list.
* Return Value: RV_ERROR_INVALID_HANDLE - If the given call-leg handle is invalid.
*               RV_ERROR_NULLPTR    - A bad pointer was given by the application.
*               RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
*          location - The location in list: next, previous, first or last.
*          hRelative - The handle to the current position in the list (a relative
*                      subscription from the list). Supply this value if you choose
*                      next or previous in the location parameter.
* Output:     phSubs   -  The handle to the returned subscription.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSubscription (
                                            IN  RvSipCallLegHandle     hCallLeg,
                                            IN  RvSipListLocation      location,
                                            IN  RvSipSubsHandle        hRelative,
                                            OUT RvSipSubsHandle        *phSubs);
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipCallLegGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the SIP Stack instance to which this call-leg
 *          belongs.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - The handle to the call-leg object.
 * Output:     phStackInstance - A valid pointer that will be updated with a
 *                            handle to the SIP Stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetStackInstance(
                                   IN   RvSipCallLegHandle   hCallLeg,
                                   OUT  void*       *phStackInstance);

/***************************************************************************
 * RvSipCallLegSetForceOutboundAddrFlag
 * ------------------------------------------------------------------------
 * General: Sets the ForceOutboundAddr flag. This flag forces the call-leg
 *          to send every request to the outbound address regardless of the
 *          message content or object state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg           - The call-leg handle.
 *     	    bForceOutboundAddr - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetForceOutboundAddrFlag(
                                IN  RvSipCallLegHandle  hCallLeg,
			                    IN  RvBool              bForceOutboundAddr);

/***************************************************************************
 * RvSipCallLegSetAddAuthInfoToMsgFlag
 * ------------------------------------------------------------------------
 * General: Sets the AddAuthInfoToMsg Flag. If this flag is set to RV_FALSE,
 *          outgoing messages will not contain authorization information.
 *          The default value of this flag is RV_TRUE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg           - The call-leg handle.
 *     	    bAddAuthInfoToMsg - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetAddAuthInfoToMsgFlag(
                                IN  RvSipCallLegHandle  hCallLeg,
			                    IN  RvBool              bAddAuthInfoToMsg);

/***************************************************************************
 * RvSipCallLegGetReceivedFromAddress
 * ------------------------------------------------------------------------
 * General: Gets the address from which the last message was received.
 * Return Value: RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg  - The call-leg handle.
 * Output: pAddr     - Basic details about the received From address.
 ***************************************************************************/
RVAPI RV_Status RVCALLCONV RvSipCallLegGetReceivedFromAddress(
										IN  RvSipCallLegHandle   hCallLeg,
										OUT RvSipTransportAddr  *pAddr);

#ifdef RV_SIP_HIGHAVAL_ON
/*-----------------------------------------------------------------------
       C A L L  - L E G   HIGH AVAILABILITY   F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegGetConnectedCallStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of a connected call.
 *          (The size of buffer should be supplied in RvSipCallLegStoreConnectedCall()).
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg .
 * Output:  len - The size of buffer. The value is -1 in case of failure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetConnectedCallStorageSize(
                                       IN  RvSipCallLegHandle hCallLeg,
                                       OUT RvInt32*           len);

/***************************************************************************
 * RvSipCallLegStoreConnectedCall
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call-leg.
 *          To store call-leg information, the call-leg must be in the
 *          CONNECTED state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg .
 *            memBuff  - The buffer that will be filled with the call-leg information.
 *          buffLen  - The length of the given buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegStoreConnectedCall(
                                       IN RvSipCallLegHandle hCallLeg,
                                       IN void*              memBuff,
                                       IN RvUint32          buffLen);

/***************************************************************************
 * RvSipCallLegStoreConnectedCallExt
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call-leg.
 *          To store call-leg information, the call-leg must be in the
 *          CONNECTED state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            memBuff  - The buffer that will be filled with the call-leg information.
 *          buffLen  - The length of the given buffer.
 * Output   usedBuffLen - The number of bytes that were actually used in the buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegStoreConnectedCallExt(
                                       IN RvSipCallLegHandle hCallLeg,
                                       IN void*              memBuff,
                                       IN RvUint32           buffLen,
                                       OUT RvUint32          *pUsedBuffLen);


/***************************************************************************
 * RvSipCallLegRestoreConnectedCall
 * ------------------------------------------------------------------------
 * General: Restores all call-leg information into a given call-leg. The call-leg
 *          will assume the CONNECTED state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - The handle to the call-leg.
 *          memBuff    - The buffer that stores the call-leg information.
 *          buffLen    - The buffer size.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRestoreConnectedCall(
                                       IN RvSipCallLegHandle   hCallLeg,
                                       IN void                *memBuff,
                                       IN RvUint32             buffLen);
#ifdef RV_SIP_HIGH_AVAL_3_0
/***************************************************************************
 * RvSipCallLegRestoreOldVersionConnectedCall
 * ------------------------------------------------------------------------
 * General: Restores call-legs that were stored by SIP Stack version 3.0.
 *          This function restored call-leg information into a given call-leg.
 *          The call-leg
 *          will assume the CONNECTED state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - The handle to the call-leg.
 *          memBuff    - The buffer that stores the call-leg information
 *          buffLen    - The buffer size.
 *          eHAmode    - Defines the exact mode, used to store the given buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRestoreOldVersionConnectedCall(
                                       IN RvSipCallLegHandle        hCallLeg,
                                       IN void*                     memBuff,
                                       IN RvUint32                  buffLen,
                                       IN RvSipCallLegHARestoreMode eHAmode);
#endif /*#ifdef RV_SIP_HIGH_AVAL_3_0*/

#endif /* #ifdef RV_SIP_HIGHAVAL_ON */

/*-----------------------------------------------------------------------
       C A L L  - L E G   SESSION TIMER   F U N C T I O N S
 ------------------------------------------------------------------------*/



/***************************************************************************
 * RvSipCallLegSessionTimerInviteRefresh
 * ------------------------------------------------------------------------
 * General: Note: Within a call-leg that handles a Session-Timer mechanism,
 *          this function has to be used instead of RvSipCallLegReInviteRequest().
 *          Otherwise, the Session-Timer parameter will be omitted from the
 *          re-INVITE request and the Session-Timer parameter will be turned off.
 *          This function replaces the deprecated RvSipCallLegSessionTimerRefresh()
 *          function.
 *
 *          Sends re-INVITE to refresh the session time. The refreshing re-INVITE
 *          request may be called in the following two situations:
 *          --The call-leg already sent/received the 2xx response for the initial
 *          INVITE request (the call-leg state is CONNECTED or ACCEPTED or
 *          REMOTE-ACCEPTED).
 *          --There are no other pending re-INVITE transactions (there is no other
 *           re-INVITE object that is waiting for a final response).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The handle to the call-leg that the user wishes to refresh.
 *          hInvite  - The handle to the Re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerInviteRefresh(
                               IN  RvSipCallLegHandle   hCallLeg,
                               IN  RvSipCallLegInviteHandle hInvite);

/***************************************************************************
* RvSipCallLegTranscSessionTimerGeneralRefresh
* ------------------------------------------------------------------------
* General: Creates a transaction related to the call-leg and sends a
*          Request message with the given method in order to refresh the call.
*          The only general transaction which allowed is "UPDATE".
*          The request will have the To, From and Call-ID of the call-leg and
*          will be sent with a correct CSeq step. The request will be record-routed,
*          if needed, and will include session timer information.
* Return Value: RV_ERROR_OUTOFRESOURCES - The request failed due to a resource problem.
*               RV_ERROR_UNKNOWN - Failed to send.
*               RV_OK - Message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - A pointer to the call-leg the user wishes to modify.
*          strMethod - A string with the request method.
*          sessionExpires - The session time that will attach to this call.
*          minSE - The minimum Session Expires time of this call.
*          eRefresher - The refresher preference for this call
* Output:  hTransc - The handle to the newly created transaction.
*                    If a transaction was supplied, this transaction will be used,
*                    and a new one will not be created.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerGeneralRefresh (
      IN  RvSipCallLegHandle                             hCallLeg,
      IN  RvChar                                        *strMethod,
      IN  RvInt32                                       sessionExpires,
      IN  RvInt32                                       minSE,
      IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher,
      INOUT RvSipTranscHandle                              *hTransc);

/***************************************************************************
 * RvSipCallLegSessionTimerSetPreferenceParams
 * ------------------------------------------------------------------------
 * General:Sets the preference SessionTimer parameters associated with this call.
 *         These parameters may not be equal to the SessionTimer parameters
 *         of the call in the end of the negotiation.(after a 200 OK has been sent or
 *         received).
 *
 * NOTE: 1. If sessionExpires is set to 0, the whole SessionTimer mechanism
 *			is turned off immediately. Moreover, the mechanism can be turned
 *			on by calling this function with non-zero a sessionExpires value
 *			in the middle of a call.
 *		 2. If the input minSE parameter equals UNDEFINED, a default value
 *          according to the standard (90) is set, and the Min-SE header will not
 *          be added to the call-leg requests from then on, except for a Refresh,
 *          which follows a 422 response.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - The handle to the call-leg.
 *          sessionExpires - The session time that will attach to this call.
 *          minSE          - The minimum Session-Expires time of this call.
 *          eRefresher     - The refresher preference for this call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerSetPreferenceParams(
         IN  RvSipCallLegHandle                             hCallLeg,
         IN  RvInt32                                       sessionExpires,
         IN  RvInt32                                       minSE,
         IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher);
/***************************************************************************
 * RvSipCallLegTranscSessionTimerSetPreferenceParams
 * ------------------------------------------------------------------------
 * General:Sets the preference SessionTimer parameters associated with this
 *         transaction."UPDATE" is the only general transaction that is allowed.
 *         These parameters may not be equal to the SessionTimer
 *         parameters of the call at the end of the negotiation.
 *         (after a 200 OK has been sent or received).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 *          hTransc  - The handle to the to which the transaction the request
 *                     belongs.
 *          sessionExpires - The session time that will attach to this call.
 *          minSE - The minimum Session Expires time of this call.
 *          eRefresher - The refresher preference for this call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerSetPreferenceParams (
         IN  RvSipCallLegHandle                             hCallLeg,
         IN  RvSipTranscHandle                              hTransc,
         IN  RvInt32                                       sessionExpires,
         IN  RvInt32                                       minSE,
         IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher);
/***************************************************************************
 * RvSipCallLegSessionTimerGetNegotiationParams
 * ------------------------------------------------------------------------
 * General:Gets the negotiation Session Timer parameters associated with this call.
 *         These parameters may not be equal to the Session Timer parameters
 *         of the call in the end of the negotiation.(after a 200 OK has been sent or
 *         received).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 * OutPut:  sessionExpires - The session time that will attach to this call.
 *          minSE - The minimum Session Expires time of this call.
 *          eRefresherType - The refresher type for this call.
 *          eRefresherPref  - The refresher request for this call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetNegotiationParams(
         IN  RvSipCallLegHandle                             hCallLeg,
         OUT  RvInt32                                      *sessionExpires,
         OUT  RvInt32                                      *minSE,
         OUT  RvSipSessionExpiresRefresherType              *eRefresherType,
         OUT RvSipCallLegSessionTimerRefresherPreference    *eRefresherPref);
/***************************************************************************
* RvSipCallLegTranscSessionTimerGetNegotiationParams
* ------------------------------------------------------------------------
* General:Gets the negotiation Session Timer parameters associated with this
*         transaction. These parameters may not be equal to the Session Timer
*         parameters of the call at the end of the negotiation.
*         (after a 200 OK has been sent or received).
*
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg   - The handle to the call-leg.
*          hTransc  - The handle to the transaction to which the request belongs.
* OutPut:  sessionExpires - The session time that will attach to this call.
*          minSE - The minimum Session Expires time of this call.
*          eRefresherType - The refresher type for this call.
*          eRefresherPref  - The refresher request for this call.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerGetNegotiationParams(
           IN  RvSipCallLegHandle                             hCallLeg,
           IN  RvSipTranscHandle                              hTransc,
           OUT  RvInt32                                      *sessionExpires,
           OUT  RvInt32                                      *minSE,
           OUT  RvSipSessionExpiresRefresherType              *eRefresherType,
           OUT RvSipCallLegSessionTimerRefresherPreference    *eRefresherPref);

/***************************************************************************
 * RvSipCallLegSessionTimerGetAlertTime
 * ------------------------------------------------------------------------
 * General: Returns the time in which the Refresh will be sent after the session
 *          starts in automode, or calls the RvSipCallLegSessionTimerRefreshAlertEv()
 *          callback in manual mode.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 * Output:  alertTime  - The time in which the Refresh will be sent
 *                       before the session ends.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetAlertTime (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *alertTime);

/***************************************************************************
 * RvSipCallLegSessionTimerSetAlertTime
 * ------------------------------------------------------------------------
 * General: This function enables the application to modify the time in
 *          in which refresh will be send before the session ends.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  alertTime  - the time in which refresh will be send
 *                       before the session ends.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerSetAlertTime (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    IN  RvInt32             alertTime);

/***************************************************************************
 * RvSipCallLegSessionTimerGetRefresherType
 * ------------------------------------------------------------------------
 * General: Returns the refresher type of this call.
 *          The value of the refresher type can be different from the refresher
 *          preference (which is the application request).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 * Output:  eRefresher  - The refresher request for this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetRefresherType (
                 IN  RvSipCallLegHandle               hCallLeg,
                 OUT RvSipSessionExpiresRefresherType *eRefresher);
/***************************************************************************
 * RvSipCallLegSessionTimerGetMinSEValue
 * ------------------------------------------------------------------------
 * General:Returns the Min-SE value associated with this call.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 * Output:  minSE - The minimum Session-Expires time of this call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetMinSEValue (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *minSE);

/***************************************************************************
 * RvSipCallLegSessionTimerGetSessionExpiresValue
 * ------------------------------------------------------------------------
 * General:Returns the Session-Expires value associated with this call.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The handle to the call-leg.
 * Output:  sessionExpires - The Session-Expires time of this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetSessionExpiresValue (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *sessionExpires);



/***************************************************************************
 * RvSipCallLegSessionTimerStopTimer
 * ------------------------------------------------------------------------
 * General: Turns off the loaded Session Timer (due to a previous negotiation).
 *          In the case of a non-existent Session Timer, this function would be
 *          meaningless.
 *          NOTE: It is highly recommended to use this function carefully
 *                when the local endpoint wishes to turn off the mechanism
 *                completely, for example, by sending a final response with
 *                a no Session-Expires header. Otherwise, the SIP Stack may
 *                act inconsistently, with regards to the defined SIP standard.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN - Failed to send re-INVITE.
 *               RV_OK - The timer was shut down successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to refresh the call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerStopTimer(
                               IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Copies an application-constructed message into the outbound message
 *          of the call-leg. It is the responsibility of the application to free this
 *          message after calling the set function.
 *          It is not recommended to use this function and it is not documented.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 *             hMsg     - The handle to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     IN  RvSipMsgHandle            hMsg);



/***************************************************************************
 * RvSipCallLegSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates a call-leg to a SigComp compartment. The call-leg will
 *          use the compartment in the message compression process.
 *          Note: The message will be compressed only if the remote
 *          URI incudes the comp=sigcomp parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The call-leg handle.
 *             hCompartment - The handle to the SigComp compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCompartment(
                            IN RvSipCallLegHandle     hCallLeg,
                            IN RvSipCompartmentHandle hCompartment);

/***************************************************************************
 * RvSipCallLegGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the associated the SigComp compartment associated with
 *          a call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg      - The call-leg handle.
 *         phCompartment - The handle to the SigComp compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCompartment(
                            IN  RvSipCallLegHandle      hCallLeg,
                            OUT RvSipCompartmentHandle *phCompartment);

/***************************************************************************
 * RvSipCallLegDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a call-leg for outgoing
 *          requests. This means that even if the message indicates compression,
            it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg      - The call-leg handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDisableCompression(
                                      IN RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * RvSipCallLegIsCompartmentRequired
 * ------------------------------------------------------------------------
 * General: Checks if the call-leg has to be related to a compartment.
 *          Applications that would like to manage the SIP Stack compartments
 *          manually should use this function. For instance, if you wish to
 *          relate an incoming call-leg that handles a SigComp message to a
 *          specific compartment, you can call this function in the
 *          RvSipCallLegCreatedEv() callback and attach your chosen compartment
 *          to the call-leg, if needed. When the callback returns, the SIP Stack
 *          will automatically attach the call-leg to a compartment if it does
 *          not yet have one.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg      - The call-leg handle.
 * Output:    pbRequired    - Indicates whether or not a compartment is required.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegIsCompartmentRequired(
                                      IN  RvSipCallLegHandle  hCallLeg,
                                      OUT RvBool             *pbRequired);


/*-----------------------------------------------------------------------
       C A L L  - L E G:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegDNSGiveUp
 * ------------------------------------------------------------------------
 * General: Stops retrying to send a request after send failure. When a call-leg
 *          fails to send a request due to a timeout, network error, or 503 response,
 *          the call-leg moves to the MSG_SEND_FAILURE state. In this state, the
 *          application can do one of the following:
 *          -Send the request to the next DNS-resolved IP.
 *          -Give up on this request.
 *          Calling RvSipCallLegDNSGiveUp() indicates that the application wishes
 *          to give up on this request. Retrying to send the request will stop and
 *          the call-leg will change its state back to the previous state. You can
 *          use this function for a call-leg request or for a general request, as follows:
 *          -Use this function for Invite, BYE and re-INVITE messages, if the state was
 *          changed to MSG_SEND_FAILURE state. For these methods, you should set the
 *          transaction handle parameter to NULL.
 *          -Use this function for a general request if you received a MSG_SEND_FAILURE
 *          status in the RvSipCallLegTranscStateChangedEv() event. For such a general
 *          request, you should supply the general transaction handle.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The request failed due to a resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send a re-INVITE.
 *               RV_OK - The Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that sent the request.
 *          hTransc  - The handle to the transaction, in the case of a general request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSGiveUp (
                                                  IN  RvSipCallLegHandle   hCallLeg,
                                                  IN  RvSipTranscHandle    hTransc);

/***************************************************************************
 * RvSipCallLegDNSContinue
 * ------------------------------------------------------------------------
 * General: Prepares the call-leg to retry sending a request after the
 *          previous attempt failed. When a call-leg fails to send a
 *          request due to a timeout, network error, or 503 response,
 *          the call-leg moves to the MSG_SEND_FAILURE state. In this state,
 *          the application can do one of the following:
 *          -Send the request to the next DNS-resolved IP.
 *          -Give up on this request.
 *          To retry sending the message, the application must first call
 *          the RvSipCallLegDNSContinue() function. Calling this function
 *          closes the failure transaction and updates the DNS list.
 *          (To re-send the request to the next IP address, use the
 *          RvSipCallLegDNSReSendRequest() function).
 *          You can use this function for a call-leg request or for a general
 *          request, as follows:
 *          -Use this function for Invite, BYE and re-INVITE messages, if the
 *          state was changed to MSG_SEND_FAILURE state. For these methods,
 *          you should set the transaction handle parameter to NULL.
 *          -Use this function for a general request if you received a
 *          MSG_SEND_FAILURE status in the RvSipCallLegTranscStateChangedEv()
 *          event. For such a general request, you should supply the general
 *          transaction handle.
 *          Calling this function for a failure resulting from a 503 response
 *          will send an ACK on the INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The request failed due to a resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-INVITE.
 *               RV_OK - The Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that sent the request.
 *          hTransc  - The handle to the transaction, in the case of a general request.
 * Output:  phClonedTransc - The handle to the cloned transaction, in the case of a general
 *                           transaction only.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSContinue (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipTranscHandle    hTransc,
                                            OUT RvSipTranscHandle*   phClonedTransc);

/***************************************************************************
 * RvSipCallLegDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: Re-sends a request after the previous attempt failed. When a
 *          call-leg fails to send a request due to a timeout, network error,
 *          or 503 response, the call-leg object moves to the MSG_SEND_FAILURE
 *          state. In this state, the application can do one of the following:
 *          -Send the request to the next DNS resolved IP.
 *          -Give up on this request.
 *          To re-send the Register request, first call RvSipCallLegDNSContinue().
 *          You should then call RvSipCallLegDNSReSendRequest(). The request will
 *          automatically be sent to the next resolved IP address in the DNS list.
 *          You can use this function for a call-leg request or for a general request,
 *          as follows:
 *          -Use this function for Invite, BYE and re-INVITE messages, if the state
 *          was changed to MSG_SEND_FAILURE state. For these methods, you should set
 *          the transaction handle parameter to NULL.
 *          Use this function for a general request if you received a MSG_SEND_FAILURE
 *          status in the RvSipCallLegTranscStateChangedEv() event. For such a general
 *          request, you should supply the general transaction handle.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The request failed due to a resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send a re-INVITE.
 *               RV_OK - The Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that sent the request.
 *          hTransc  - The handle to the transaction in the case of a general request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSReSendRequest (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipTranscHandle    hTransc);

/***************************************************************************
 * RvSipCallLegDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves a DNS list object from the call-leg current active
 *          or requested transaction. When a call-leg fails to send a
 *          request due to a timeout, network error, or 503 response,
 *          the call-leg moves to the MSG_SEND_FAILURE state. In this
 *          state you can use RvSipCallLegDNSGetList() to get the DNS
 *          list if you wish to view or change it.
 *          You can use this function for a call-leg request or for a
 *          general request,as follows:
 *          -Use this function for Invite, BYE and re-INVITE messages,
 *          if the state was changed to MSG_SEND_FAILURE state. For
 *          these methods, you should set the transaction handle
 *          parameter to NULL.
 *          -Use this function for a general request if you received a
 *          MSG_SEND_FAILURE status in the RvSipCallLegTranscStateChangedEv()
 *          event. For such a general request, you should supply the general
 *          transaction handle.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg that sent the request.
 *          hTransc  - The handle to the transaction in the case of a general request.
 * Output   phDnsList - The DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSGetList(
                                          IN  RvSipCallLegHandle           hCallLeg,
                                          IN  RvSipTranscHandle            hTransc,
                                          OUT RvSipTransportDNSListHandle *phDnsList);

/*--------------------------------------------------------------------------------
       C A L L  - L E G:  L O C A L   A N D   R E M O T E   A D D R E S S   A P I
 ---------------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the call-leg will send outgoing
 *          requests. The SIP Stack can be configured to listen to many local
 *          addresses. Each local address has a transport type (UDP, TCP, TLS
 *          or SCTP) and an address type (IPv4/IPv6). When the SIP Stack sends
 *          an outgoing request, the local address (from where the request is
 *          sent) is chosen according to the characteristics of the remote address.
 *          Both the local and remote addresses must have the same characteristics
 *          (transport type and address type). If several configured local addresses
 *          match the remote address characteristics, the first configured address
 *          is taken. You can use RvSipCallLegSetLocalAddress() to force the call-leg
 *          to choose a specific local address for a specific transport and address type.
 *          For example, you can force the call-leg to use the second configured
 *          UDP/IPv4 local address. If the call-leg sends a request to a UDP/IPv4
 *          remote address, it will use the local address that you set instead of
 *          the default first local address.
 *          Note: The localAddress string you provide for this function must match
 *          exactly with the local address that was inserted in the configuration
 *          structure in the initialization of the SIP Stack. If you configured
 *          the SIP Stack to listen to a 0.0.0.0 local address, you must use the
 *          same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *            eTransportType - The transport type (UDP, TCP, SCTP or TLS).
 *            eAddressType - The address type(IPv4 or IPv6).
 *            localAddress - The local address to be set to this call-leg.
 *            localPort - The local port to be set to this call-leg.
 *            if_name - local interface name.
 *            vdevblock - local hardware interface or else.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    );

/***************************************************************************
 * RvSipCallLegGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address which the call-leg will use to send
 *          outgoing requests to a destination that listens to a specific
 *          transport and address type. This is the address the user set
 *          using the RvSipCallLegSetLocalAddress() function. If no address
 *          was set, the function will return the default, first configured
 *          local address of the requested transport and address type.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg
 *          eTransportType - The transport type (UDP, TCP, SCTP or TLS).
 *          eAddressType - The address type (IPv4 or IPv6).
 * Output:    localAddress - The local address this call-leg is using.
 *          localPort - The local port this call-leg is using.
 * if_name - local interface name
 * vdevblock - local hardware interface or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *localAddress,
                            OUT  RvUint16                *localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    );

/***************************************************************************
 * RvSipCallLegGetTranscCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address that is used by a specific call-leg
 *          transaction. You can supply a specific transaction handle or use
 *          NULL to get the address of the active transaction (INVITE
 *          transaction). In this version this function can be called
 *          only in the OFFERING state to get the local address of
 *          an incoming INVITE transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 *          hTransc - The handle to a specific call-leg transaction or NULL to
 *                    indicate an active transaction.
 * Output:    eTransporType - The transport type (UDP, TCP, SCTP or TLS).
 *          eAddressType  - The address type (IPv4 or IPv6).
 *          localAddress - The local address this transaction is using
 *                         (must be longer than 48 bytes).
 *          localPort - The local port this transaction is using.
 *          if_name - local interface name.
 *          vdevblock - local hardware interface or else.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetTranscCurrentLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *eTransportType,
                            OUT RvSipTransportAddressType *eAddressType,
                            OUT RvChar                   *localAddress,
                            OUT RvUint16                 *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT RvChar                 *if_name
			    , OUT RvUint16               *vdevblock
#endif
			    );

/***************************************************************************
 * RvSipCallLegSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the call-leg.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as the IP address or host name
 *          transport, port and compression type.
 *          Request sent by this object will use the outbound detail specifications
 *          as a remote address. The Request-URI will be ignored.
 *          However, the outbound proxy will be ignored if the message contains
 *          a Route list.
 *
 *          NOTE: If you specify both IP address and a host name in the
 *                configuration structure, both of them will be set BUT
 *                the IP address will be used.
 *                If you do not specify port or transport, both will be determined
 *                according to the DNS procedures specified in RFC 3263
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The handle to the call-leg.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with the relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundDetails(
                            IN  RvSipCallLegHandle              hCallLeg,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg);

/***************************************************************************
 * RvSipCallLegGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all outbound proxy details that the call-leg object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as the IP address or host name
 *          transport, port and compression type.
 *          If the outbound details were not set to the specific
 *          call-leg object, rather the outbound proxy was defined to the SIP Stack
 *          upon initialization, these parameters will be returned.
 *          If the call-leg is not using an outbound address NULL/UNDEFINED
 *          values are returned.
 *          NOTE: You must supply a valid consecutive buffer in the
 *                RvSipTransportOutboundProxyCfg structure to get the
 *                outbound strings (host name and IP address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - The handle to the call-leg.
 *          sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to the outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundDetails(
                            IN  RvSipCallLegHandle              hCallLeg,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);

/***************************************************************************
 * RvSipCallLegSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility
 *          only.
 *          To set outbound proxy information, use
 *          RvSipCallLegSetOutboundDetails().
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg          - The handle to the call-leg.
 *            strOutboundAddrIp   - The outbound IP to be set to this
 *                              call-leg.
 *          outboundAddressPort - The outbound port to be set to this
 *                              call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundAddress(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvChar               *strOutboundAddrIp,
                            IN  RvUint16              outboundAddressPort);

/***************************************************************************
 * RvSipCallLegGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility
 *          only.
 *          To get outbound proxy information, use RvSipCallLegGetOutboundDetails().
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hCallLeg      - The handle to the call-leg.
 * Output:
 *            strOutboundHost   -  A buffer with the outbound host name.
 *          pOutboundPort - The outbound host port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundHostName(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvChar              *strOutboundHost,
                            OUT  RvUint16            *pOutboundPort);


/***************************************************************************
 * RvSipCallLegSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy host name of the call-leg object.
 * The outbound host name will be resolved each time a request is sent to this host.
 * the request will be sent to the resolved IP address.
 * Note: To set a specific IP address, use RvSipCallLegSetOutboundAddress().
 * If you configure a call-leg with both an outbound IP address and an
 * outbound host name, the call-leg will ignore the outbound host name and
 * will use the outbound IP address.
 * When using an outbound host all outgoing requests be sent directly to the call-leg
 * outbound proxy host (unless the call-leg is using a Record-Route path).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg           - The handle to the call-leg.
 *            strOutboundHostName    - The outbound proxy host to be set to this
 *                               call-leg.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               call-leg. If you set the port to zero, it
 *                               will be determined using the DNS procedure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundHostName(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvChar                *strOutboundHostName,
                            IN  RvUint16              outboundPort);

/***************************************************************************
 * RvSipCallLegSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility
 *          only.
 *          To get outbound proxy information, use RvSipCallLegGetOutboundDetails().
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg           - The handle to the call-leg.
 *          eOutboundTransport - The outbound transport to be set
 *                               to this call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundTransport(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvSipTransport         eOutboundTransport);

/***************************************************************************
 * RvSipCallLegGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Gets the address of outbound proxy the call-leg is using. If an
 *          outbound address was set to the call-leg this address will be
 *          returned. If no outbound address was set to the call-leg
 *          but an outbound proxy was configured for the SIP Stack, the configured
 *          address is returned. '\0' is returned if the call-leg is not using
 *          an outbound proxy.
 *          NOTE: you must supply a valid consecutive buffer of size 48 to
 *          get the outboundProxyIp.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hCallLeg      - The handle to the call-leg.
 * Output:
 *            outboundProxyIp   -  A buffer with the outbound proxy IP the call-leg.
 *                               is using (must be longer than 48).
 *          pOutboundProxyPort - The outbound proxy port that the call-leg is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundAddress(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvChar              *outboundProxyIp,
                            OUT  RvUint16            *pOutboundProxyPort);

/***************************************************************************
 * RvSipCallLegGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: This function is deprecated and is kept for backward compatibility
 *          only.
 *          To get all outbound proxy information, use
 *          RvSipCallLegGetOutboundDetails().
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg           - The handle to the call-leg
 * Output: eOutboundTransport - The outbound proxy transport the call-leg is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundTransport(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvSipTransport       *eOutboundProxyTransport);

/*-----------------------------------------------------------------------
       C A L L  - L E G:  F O R K I N G   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegGetOriginalCallLeg
 * ------------------------------------------------------------------------
 * General: An initial INVITE request, might be forked by a proxy. as a result,
 *          several 1xx and 2xx responses may be received from several UASs.
 *          The first incoming response will be mapped to the original call-leg,
 *          that sent the INVITE request. every other incoming response (with a
 *          different to-tag parameter), creates a new "forked call-leg".
 *
 *          This function returns the original call-leg of a given call-leg:
 *          If the given call-leg is a forked call-leg the function will
 *          return the original call-leg handle.
 *          If the given call-leg is an original call-leg the function will
 *          return the same call-leg handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg      - The handle to a call-leg.
 * Output:  phOrigCallLeg - The handle to the original call-leg.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetOriginalCallLeg(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    OUT RvSipCallLegHandle  *phOrigCallLeg);

/***************************************************************************
 * RvSipCallLegSetRejectStatusCodeOnCreation
 * ------------------------------------------------------------------------
 * General: This function can be used synchronously from the
 *          RvSipCallLegCreatedEv callback to instruct the SIP Stack to
 *          automatically reject the request that created this call-leg.
 *          In this function you should supply the reject status code.
 *          If you set this status code, the call-leg will be destructed
 *          automatically when the RvSipCallLegCreatedEv returns. The
 *          application will not get any further callbacks
 *          that relate to this call-leg. The application will not get the
 *          RvSipCallLegMsgToSendEv for the reject response message or the
 *          TERMINATED state for the call-leg.
 *          This function should not be used for rejecting a request in
 *          a normal scenario. For this you should use the RvSipCallLegReject()
 *          API function. You should use this function only if your application is
 *          incapable of handling this new call-leg at all, for example, in a
 *          situation where an application is out of resources.
 *          Note: When this function is used to reject a request, the application
 *          cannot use the outbound message mechanism to add information
 *          to the outgoing response message. If you wish to change the response
 *          message you must use the regular reject function in the OFFERING
 *          state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - The handle to the call-leg.
 *           rejectStatusCode - The reject status code for rejecting the request
 *                              that created this object. The value must be
 *                              between 300 to 699.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetRejectStatusCodeOnCreation(
                              IN  RvSipCallLegHandle     hCallLeg,
                              IN  RvUint16               rejectStatusCode);


/***************************************************************************
 * RvSipCallLegSetForkingEnabledFlag
 * ------------------------------------------------------------------------
 * General: Sets the "forking-enabled" flag of the call-leg.
 *          This flag defines the call-leg behavior upon receiving a forked
 *          1xx/2xx response.
 *          If this flag is set to TRUE, a new call-leg is created for
 *          every 1xx/2xx response with a new to-tag.
 *          If this flag is set to FALSE, the response will be mapped
 *          to the original call-leg. Every 1xx response will update the
 *          to-tag parameter of the call-leg.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForkingEnabledFlag (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvBool               bForkingEnabled);

/***************************************************************************
 * RvSipCallLegGetForkingEnabledFlag
 * ------------------------------------------------------------------------
 * General: Returns the "forking-enabled" flag of the call-leg.
 *          This flag defines the call-leg behavior upon receiving a forked
 *          1xx/2xx response.
 *          If this flag is set to TRUE, a new call-leg is created for
 *          every 1xx/2xx response with a new to-tag.
 *          If this flag is set to FALSE, the response will be mapped
 *          to the original call-leg. Every 1xx response will update the
 *          to-tag parameter of the call-leg.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetForkingEnabledFlag(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvBool               *pbForkingEnabled);


/***************************************************************************
 * RvSipCallLegSetForkedAckTimerTimeout
 * ------------------------------------------------------------------------
 * General: This function is deprecated and is kept for backward compatibility
 *          only.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The handle to the call-leg.
 *          timeout  - The timeout value for the ACK timer. (If 0, the timer will
 *                     not be set.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForkedAckTimerTimeout (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvInt32              timeout);

/***************************************************************************
 * RvSipCallLegSetForked1xxTimerTimeout
 * ------------------------------------------------------------------------
 * General: Sets the timeout value for the forked-1xx-timer. An initial
 *          INVITE request may be forked by a proxy, and as a result,
 *          several 1xx and 2xx responses may be received from several
 *          UASs. The first incoming response will be mapped to the original
 *          call-leg that sent the INVITE request. Every other incoming response
 *          (with a different to-tag parameter) creates a new "forked call-leg".
 *          A forked call-leg that received a 1xx response sets a timer
 *          (forked-1xx-timer). This timer will be released when this
 *          call-leg receives a 2xx response. If it expires before the
 *          2xx was received, the call-leg is terminated. (usually the
 *          timer timeout value is taken from the SIP Stack configuration).
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The handle to the call-leg.
 *          timeout  - The timeout value for the 1xx timer. (If 0, the timer will
 *                     not be set.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForked1xxTimerTimeout (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvInt32              timeout);

/***************************************************************************
 * RvSipCallLegSetTranscTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the transactions timers of the call-leg.
 *          If some of the fields in pTimers are not set (UNDEFINED), this
 *          function will calculate it, or take the values from configuration.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - The handle to the call-leg.
 *           pTimers - A pointer to the structure that contains all the timeout values.
 *           sizeOfTimersStruct - The size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetTranscTimers(
                              IN  RvSipCallLegHandle     hCallLeg,
                              IN  RvSipTimers           *pTimers,
                              IN  RvInt32               sizeOfTimersStruct);

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipCallLegSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets the SCTP transport parameters to the call-leg. All the possible
 *          SCTP parameters are located in the RvSipTransportSctpMsgSendingParams
 *          structure and will be used only if the call-leg uses SCTP as the
 *          destination transport.
 *          This function is optional. If specific SCTP parameters are not set,
 *          the default SCTP parameters will be used.
 *          Calling this function does not affect the chosen transport. The
 *          transport will be chosen in the address resolution process.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - The handle to the call-leg object.
 *           sizeOfStruct - The size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - A pointer to the structure that contains the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSctpMsgSendingParams(
                     IN  RvSipCallLegHandle                 hCallLeg,
                     IN  RvUint32                            sizeOfParamsStruct,
                     IN  RvSipTransportSctpMsgSendingParams *pParams);

/******************************************************************************
 * RvSipCallLegGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets the SCTP parameters that are used by the call-leg while
 *          sending a message over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - The handle to the call-leg.
 *           sizeOfStruct - The size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - A pointer to the structure that contains the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSctpMsgSendingParams(
                    IN  RvSipCallLegHandle                 hCallLeg,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipCallLegSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets a security-object into the call-leg.
 *          As a result of this operation, all messages, sent by this call-leg,
 *          will be protected with the security mechanism that is
 *          provided by the security-object.
 * Return Value: RV_OK on error. Otherwise, the error code defined in the
 *               RV_SIP_DEF.h or rverror.h files.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hCallLeg - The handle to the call-leg.
 *          hSecObj  - The handle to the security-object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSecObj(
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        IN  RvSipSecObjHandle     hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipCallLegGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets a security-object set into the call-leg.
 * Return Value: RV_OK on error. Otherwise, the error code defined in the
 *               RV_SIP_DEF.h or rverror.h files.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hCallLeg - The handle to the call-leg object.
 *  Output: phSecObj - The handle to the security-object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSecObj(
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        OUT RvSipSecObjHandle*    phSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * RvSipCallLegUseFirstRouteForInitialRequest
 * ------------------------------------------------------------------------
 * General: An application may want to use a preloaded Route header when sending
 *          an initial request. For this purpose, the application should add
 *          the Route header to the outbound message, and call this function
 *          to notify the SIP Stack that it should send the request to the
 *          address of the first Route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegUseFirstRouteForInitialRequest (
                                      IN  RvSipCallLegHandle       hCallLeg);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipCallLegGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: Retrieves the authentication-object that is currently being
 *          processed by the authenticator.
 *          (for application usage in the RvSipAuthenticatorGetSharedSecretEv
 *          callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg          - The handle to the call-leg.
 * Output:    phAuthObj         - The authentication-object handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrProcessedAuthObj (
                                      IN   RvSipCallLegHandle    hCallLeg,
                                      OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * RvSipCallLegAuthObjGet
 * ------------------------------------------------------------------------
 * General: Retrieves authentication-objects from the list in the call-leg.
 *          You can get the first, last, or next object. If you get the next
 *          object, you must supply the current object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg          - The handle to the call-leg.
 *            eLocation         - The location in the list (first/next/last)
 *            hRelativeAuthObj  - The relative object in the list (relevant for the next location)
 * Output:    phAuthObj         - The authentication-object handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthObjGet (
                                      IN   RvSipCallLegHandle     hCallLeg,
                                      IN   RvSipListLocation      eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * RvSipCallLegAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an authentication-object from the list in
 *          the call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg.
 *            hAuthObj - The authentication-object handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthObjRemove (
                                      IN   RvSipCallLegHandle   hCallLeg,
                                      IN   RvSipAuthObjHandle   hAuthObj);
#endif /*RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON

/***************************************************************************
 * RvSipCallLegSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header into the call-leg.
 *          The header will be headed to outgoing requests.
 *          An initial Authorization header contains
 *          only the private identity of the client, and no real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The call-leg will set the initial header to the message only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The call-leg handle.
 *            hAuthorization - The initial Authorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetInitialAuthorization (
                                         IN RvSipCallLegHandle hCallLeg,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization);

/***************************************************************************
 * RvSipCallLegSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to a call-leg. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the call-leg will take upon itself this agreement and the data
 *          it brings. If not, the call-leg will use this security-agreement
 *          object to negotiate an agreement with the remote party as defined
 *          in RFC 3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - The handle to the call-leg.
 *          hSecAgree  - The handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSecAgree(
                    IN  RvSipCallLegHandle           hCallLeg,
                    IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * RvSipCallLegGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement associated with the call-leg.
 *          The security-agreement captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg    - The handle to the call-leg.
 *          hSecAgree   - The handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSecAgree(
									IN   RvSipCallLegHandle      hCallLeg,
									OUT  RvSipSecAgreeHandle    *phSecAgree);
#endif /*#ifdef RV_SIP_IMS_ON*/









/*-----------------------------------------------------------------------
       C A L L  - L E G:  D P R E C A T E D   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegModify
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility
 *          only.
 *          Use RvSipCallLegReInviteCreate() and RvSipCallLegReInviteRequest()
 *          instead.
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegModify (
                                        IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegGetCurrentModifyState
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrentModifyState (
                            IN  RvSipCallLegHandle       hCallLeg,
                            OUT RvSipCallLegModifyState  *peModifyState);

/***************************************************************************
 * RvSipCallLegGetOutboundAckMsg
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegGetOutboundMsg() to get the ACK message,
 *          before sending the ACK.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:    phMsg   -  A pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetOutboundAckMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     OUT RvSipMsgHandle            *phMsg);

/***************************************************************************
 * RvSipCallLegSessionTimerRefresh
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegReInviteCreate() and
 *              RvSipCallLegSessionTimerInviteRefresh()
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to refresh the call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerRefresh(
                               IN  RvSipCallLegHandle   hCallLeg);

/***************************************************************************
 * RvSipCallLegReplacesPrepareHeaderFromCallLeg
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg        - The handle to the call-leg to make the Replaces header from.
 *          hReplacesHeader - The handle to a Constructed Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesPrepareHeaderFromCallLeg(
                                      IN    RvSipCallLegHandle         hCallLeg,
                                      IN    RvSipReplacesHeaderHandle  hReplacesHeader);



/*- D E P R E C A T E D   S E T / G E T   F U N C T I O N S --------------------*/
/***************************************************************************
 * RvSipCallLegGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 * Output:     phParty - The handle to the newly created party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewPartyHeaderHandle (
                                      IN   RvSipCallLegHandle      hCallLeg,
                                      OUT  RvSipPartyHeaderHandle  *phParty);


#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetNewReferToHeaderHandle
 * ------------------------------------------------------------------------
 * General:This function is deprecated and kept for backward compatibility only.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer to party handle was given.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to allocate.
 *               RV_OK        - A new party header was allocated successfully
 *                                   and its handle was returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg.
 * Output:     phReferTo - The handle to the newly created Refer-To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReferToHeaderHandle (
                                      IN   RvSipCallLegHandle         hCallLeg,
                                      OUT  RvSipReferToHeaderHandle  *phReferTo);
#endif /* #ifdef RV_SIP_SUBS_ON */


/***************************************************************************
 * RvSipCallLegGetNewReplacesHeaderHandle
 * ------------------------------------------------------------------------
 * General:This function is deprecated and kept for backward compatibility only.
 *         Use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg.
 * Output:     phReplaces - The handle to the newly created Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReplacesHeaderHandle (
                                      IN   RvSipCallLegHandle         hCallLeg,
                                      OUT  RvSipReplacesHeaderHandle  *phReplaces);


#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetNewReferredByHeaderHandle
 * ------------------------------------------------------------------------
 * General:This function is deprecated and kept for backward compatibility only.
 *         For Refer implementation, use the Subscription Refer API. To use the
 *         Call-leg Refer deprecated API for code that was written for a previous
 *         SIP Stack version, you must set the "disableRefer3515Behavior"
 *         configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer to party handle was given.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to allocate.
 *               RV_OK        - A new party header was allocated successfully
 *                                   and its handle was returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The handle to the call-leg.
 * Output:     phReferredBy - The handle to the newly created Referred-By header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReferredByHeaderHandle (
                                      IN   RvSipCallLegHandle            hCallLeg,
                                      OUT  RvSipReferredByHeaderHandle  *phReferredBy);
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipCallLegGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The handle to the call-leg.
 *            eAddrType - Type of address the application wishes to create.
 * Output:     phAddr    - The handle to the newly created address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewAddressHandle (
                                      IN   RvSipCallLegHandle      hCallLeg,
                                      IN  RvSipAddressType         eAddrType,
                                      OUT  RvSipAddressHandle      *phAddr);
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetReferToAddress
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - The handle to the call-leg.
 * Output:     phReferToAddress - The handle to the Refer-To address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReferToAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle  *phReferToAddress);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetReferredByAddress
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 * For Refer implementation, use the Subscription Refer API. To use the
 * Call-leg Refer deprecated API for code that was written for a previous
 * SIP Stack version, you must set the "disableRefer3515Behavior"
 * configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - A bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - The handle to the call-leg.
 * Output:     phReferredByAddress - The handle to the Referred-By address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReferredByAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle  *phReferredByAddress);
#endif /* #ifdef RV_SIP_SUBS_ON */


/*- D E P R E C A T E D   R E F E R   F U N C T I O N S --------------------*/
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegRefer
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Could not send a message to the given
 *                            Request-URI).
 *               RV_OK - A REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - The handle to the call-leg the user wishes to send REFER.
 *          hReferTo     - The Refer-To header to be sent in the REFER request.
 *                         The Refer-To header will be kept in the call-leg object,
 *                         and can contain Replaces header.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Refer-To header will
 *                         be taken from the call-leg object.
 *          hReferredBy  - The Referred-By header to be sent in the REFER request.
 *                         The Referred-By header will be kept in the call-leg object.
 *                         This parameter is optional. If not specified the call-leg
 *                         will use a default Referred-By header.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Referred-By header will
 *                         be taken from the call-leg object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRefer (
                                      IN  RvSipCallLegHandle          hCallLeg,
                                      IN  RvSipReferToHeaderHandle    hReferTo,
                                      IN  RvSipReferredByHeaderHandle hReferredBy);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferAccept
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the new call-leg handle is
 *                                  invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 202 response, or
 *                                   to create and initialize the new call-leg).
 *               RV_OK       -  Accepted final response was sent successfully,
 *                                   and a new call-leg was created and initialized
 *                                   successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to accept its refer.
 *          hAppCallLeg - The application handle to the newly created call-leg.
 * Output:  phNewCallLeg - The new call-leg that is created and initialized by
 *                        this function.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferAccept (
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        IN  RvSipAppCallLegHandle hAppCallLeg,
                                        OUT RvSipCallLegHandle   *phNewCallLeg);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferReject
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - An invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          The final Reject response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to reject the refer
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferReject (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16            status);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferStr
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - An invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - The call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (could not send a message to the given
 *                            Request-URI).
 *               RV_OK - A REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - The handle to the call-leg the user wishes to send REFER.
 *          strReferTo   - The Refer-To address to be sent in the Refer-To
 *                         header of the REFER request. or the Refer-To header itself.
 *                         The Refer-To header will be kept in the call-leg object.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Refer-To header will
 *                         be taken from the call-leg object.
 *          strReferredBy - The Referred-By address to be sent in the
 *                         Referred-By header of the REFER request, or the
 *                         Referred-By header itself.
 *                         The Referred-By header will be kept in the call-leg object.
 *                         This parameter is optional. If not specified the call-leg
 *                         will note set a Referred-By header.
 *          strReplaces  - The Replaces header to be sent in the Refer-To
 *                         header of the REFER request. The Replaces header string doesn't
 *                         contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                         the call-leg object.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Replaces header will
 *                         be taken from Refer-To header in the call-leg object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferStr (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvChar             *strReferTo,
                                      IN  RvChar             *strReferredBy,
                                      IN  RvChar              *strReplaces);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferCancel
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferCancel (
                                    IN  RvSipCallLegHandle   hCallLeg);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferNotify
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_OUTOFRESOURCES - The call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (could not send a message to the given
 *                            Request-URI).
 *               RV_OK - A Notify message was sent successfully (if needed).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to send REFER.
 *          status   - The status code that will be used to create a status
 *                     line for the NOTIFY request message body.
 *          cSeqStep - The Cseq step of the REFER transaction that this
 *                     NOTIFY relate to. This value will be set to the cseq
 *                     parameter of the Event header of the NOTIFY request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferNotify (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvInt16             status,
                                         IN  RvInt32             cSeqStep);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferEnd
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 *          Note: If you call this function with a call-leg in the IDLE state,
 *          the call-leg will be terminated.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_OK - REFER process was completed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to send a Refer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferEnd (
                                         IN  RvSipCallLegHandle   hCallLeg);
#endif /* #ifdef RV_SIP_SUBS_ON */


#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferGetCurrentState
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          you should use the subscription refer API.
 *          For Refer implementation, use the Subscription Refer API. To use the
 *          Call-leg Refer deprecated API for code that was written for a previous
 *          SIP Stack version, you must set the "disableRefer3515Behavior"
 *          configuration parameter to TRUE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The handle to the call-leg whose REFER state
 *                           the application wishes to get.
 * Output:  peReferState - The REFER state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferGetCurrentState(
                               IN  RvSipCallLegHandle      hCallLeg,
                               OUT RvSipCallLegReferState *peReferState);
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipCallLegReplacesDisconnect
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          To disconnect a replaces call use RvSipCallLegDisconnect().
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  -  The handle to the call-leg is invalid.
 *               RV_ERROR_UNKNOWN        - Failed to send message (BYE or final
 *                                   rejection response).
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK        - A BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg from which the user wishes
 *            to disconnect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesDisconnect(IN RvSipCallLegHandle hCallLeg);


/***************************************************************************
 * RvSipCallLegReplacesGetMatchedCall
 * ------------------------------------------------------------------------
 * General: This function is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegReplacesGetMatchedCallExt instead.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg               - The handle to the call-leg that received the INVITE with
 *                                   the Replaces header.
 * Output:  pbFoundNonInviteDialog - RV_TRUE if a matched dialog was found which was not established
 *                                   with INVITE, such as Subscription. In this case,
 *                                   hMatchedCallLeg will be NULL and the application should
 *                                   return 401/481/501 to the INVITE with the Replaces.
 *          hMatchedCallLeg        - The handle to the call-leg matched to the Replaces header.
 *                                   If there is no such call-leg, this handle will be NULL,
 *                                   and a 481 response will be sent to the original call-leg
 *                                   (hCallLeg)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesGetMatchedCall(
                              IN  RvSipCallLegHandle         hCallLeg,
                              OUT RvBool                   *pbFoundNonInviteDialog,
                              OUT RvSipCallLegHandle        *hMatchedCallLeg);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipCallLegRouteListEmpty
 * ------------------------------------------------------------------------
 * General: Return is Route list of call leg empty or not 
 *
 * Return Value: RvInt32
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg               - Handle to the call leg.
 * Return value: RV_TRUE when route list is empty
 *               RV_FALSE when route list is not empty
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipCallLegRouteListEmpty(IN  RvSipCallLegHandle         hCallLeg);

/***************************************************************************
 * RvSipCallLegRouteListReset
 * ------------------------------------------------------------------------
 * General: Reset tail entry of route list of call leg with new route 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg               - Handle to the call leg.
 *          nexthop                - New route
 * Return value: RV_TRUE when route list is empty
 *               RV_FALSE when route list is not empty
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRouteListReset(IN  RvSipCallLegHandle      hCallLeg,
                                                     IN  RvChar                  *nexthop);
#endif 
/* SPIRENT_END */

#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_CALL_LEG_H */

