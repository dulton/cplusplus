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
 *                              <RvSipCallLegTypes.h>
 *
 * The RvSipCallLegTypes.h file contains all type definitions and call-back functions
 * type definitions for the Call-leg module.
 *
 * includes:
 * 1.Handle Type definitions
 * 2.Call-Leg Type definitions
 * 3.Call-Leg callback functions
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/


#ifndef RV_SIP_CALL_LEG_TYPES_H
#define RV_SIP_CALL_LEG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipMsg.h"
#include "RvSipTransactionTypes.h"

/*-----------------------------------------------------------------------*/
/*                         MACRO DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define RVSIP_CALL_LEG_MIN_MINSE 90

/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*
 * RvSipCallLegMgrHandle
 * ---------------------------------------------------------------------------
 * The declaration of a handle to a call-leg Mgr. The call-leg Mgr manages all
 * call-legs and the handle is needed for all Call-leg Manager API actions,
 * such as the creation of a call-leg.
 */
RV_DECLARE_HANDLE(RvSipCallLegMgrHandle);


/*
 * RvSipCallLegHandle
 * ---------------------------------------------------------------------------
 * The declaration of a call-leg handle. A call-leg handle is needed in all
 * Call-leg API functions and is used to reference the call-leg.
 */
RV_DECLARE_HANDLE(RvSipCallLegHandle);


/*
 * RvSipAppCallLegHandle
 * ---------------------------------------------------------------------------
 * The declaration of an application handle to a call-leg. The application uses
 * this handle to associate SIP Stack call-legs with application call-legs.
 * The application gives the application handle when a new call-leg is created.
 * The SIP Stack will give this handle back to the application in every callback
 * function.
 */

RV_DECLARE_HANDLE(RvSipAppCallLegHandle);

/*
 * RvSipCallLegInviteHandle
 * ---------------------------------------------------------------------------
 * The declaration of an INVITE object handle. An INVITE object is needed in
 * all call-leg re-INVITE functions and is used to reference the correct
 * re-INVITE object.
 */
RV_DECLARE_HANDLE(RvSipCallLegInviteHandle);


/*
 * RvSipAppCallLegInviteHandle
 * ---------------------------------------------------------------------------
 * The declaration of an application handle to an INVITE object. The application
 * uses this handle to associate a SIP Stack INVITE object with an application
 * INVITE object. The application gives the application handle when a new INVITE
 * object is created. The SIP Stack will give this handle back to the application
 * in every callback function.
 */

RV_DECLARE_HANDLE(RvSipAppCallLegInviteHandle);



/*-----------------------------------------------------------------------*/
/*                    CALL_LEG TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/


/* RvSipCallLegState
 * ----------------------------------------------------------------------
 * RvSipCallLegState represents a state in a session. The state changes
 * are reported using the RvSipCallLegStateChangedEv() call back function
 * together with a state change reason that indicates to the application
 * how the new state was reached
 *
 * RVSIP_CALL_LEG_STATE_UNDEFINED: No state for this call-leg.
 * RVSIP_CALL_LEG_STATE_IDLE:      The initial state of the Call-leg state machine.
 *                                 Upon call-leg creation, the call-leg assumes the
 *                                 IDLE state. It remains in this state until
 *                                 RvSipCallLegConnect() is called, whereupon it
 *                                 should move to the INVITING state.
 * RVSIP_CALL_LEG_STATE_INVITING:  After calling RvSipCallLegConnect(), which
 *                                 will send an INVITE request, the call-leg
 *                                 enters the INVITING state. The call-leg remains
 *                                 in this state until it receives a final response
 *                                 from the remote party. If a 1xx class response is
 *                                 received, the call-leg moves to the PROCEEDING
 *                                 state. If a 2xx class response is received, the
 *                                 call-leg assumes the CONNECTED state. If a 3xx
 *                                 class response is received, the call-leg moves
 *                                 to the REDIRECTED state. If the call is rejected
 *                                 with a 4xx, 5xx or 6xx class response, the call-leg
 *                                 assumes the DISCONNECTED state.
 * RVSIP_CALL_LEG_STATE_PROCEEDING:The call assumes the PROCEEDING state when it
 *                                 receives a provisional response in the INVITING
 *                                 state. If a 2xx class response is received, the
 *                                 call-leg assumes the CONNECTED state. If a 3xx
 *                                 class response is received, the call-leg moves
 *                                 to the REDIRECTED state. If the call is rejected
 *                                 with a 4xx, 5xx or 6xx class response, the call-leg
 *                                 assumes the DISCONNECTED state. If no response is
 *                                 received and the provisional timer expired, the
 *                                 call-leg moves to the PROCEEDING_TIMEOUT state
 *                                 if the SIP Stack is configured with
 *                                 enableInviteProceedingTimeoutState=RV_TRUE.
 *                                 Otherwise, the call-leg will be terminated.
 * RVSIP_CALL_LEG_STATE_REDIRECTED:A call-leg in the INVITING state may receive a
 *                                 3xx class response. In this case, the call-leg
 *                                 assumes the REDIRECTED state. The call-leg
 *                                 automatically sends an ACK message and then
 *                                 updates the remote contact to the first contact
 *                                 address found in the 3xx message. At this point,
 *                                 you may confirm the redirection by calling the
 *                                 RvSipCallLegConnect() function again and the request
 *                                 will be sent to the updated remote contact. You
 *                                 can also decide to terminate the call using the
 *                                 RvSipCallLegTerminate() function.
 * RVSIP_CALL_LEG_STATE_OFFERING:  Upon receipt of the initial INVITE by an incoming call,
 *                                 the call-leg assumes the OFFERING state. In this state,
 *                                 it is up to you to decide whether to accept or reject
 *                                 the call using the Call-Leg API.
 * RVSIP_CALL_LEG_STATE_ACCEPTED:  If you accept a call in the OFFERING state, the call-leg
 *                                 assumes the ACCEPTED state. The call-leg moves to the
 *                                 CONNECTED state upon receipt of an ACK message from
 *                                 the calling party.
 * RVSIP_CALL_LEG_STATE_UNAUTHENTICATED: A call-leg sending an INVITE request to a server
 *                                       or Proxy may receive a 401 or 407 response. In
 *                                       this case, the call-leg assumes the UNAUTHENTICATED
 *                                       state. At this point, you may re-send your request
 *                                       with authentication information by calling the
 *                                       RvSipCallLegAuthenticate() function. You can also
 *                                       terminate the call using the RvSipCallLegTerminate()
 *                                       function.
 * RVSIP_CALL_LEG_STATE_CONNECTED: The call-leg is connected. This state indicates a successful
 *                                 session setup. The call-leg reaches this state either when
 *                                 a 200 final response is received on an initial INVITE, or
 *                                 when an ACK is received on a 200 final response. While
 *                                 in this state, you can use the RvSipCallLegReInviteCreate()
 *                                 and RvSipCallLegReInviteRequest() functions to initiate a
 *                                 re-INVITE. You can also cause the call to disconnect by
 *                                 calling the RvSipCallLegDisconnect() function. Calling
 *                                 the RvSipCallLegDisconnect() function causes the state
 *                                 to change to DISCONNECTING.
 * RVSIP_CALL_LEG_STATE_DISCONNECTING: If RvSipCallLegDisconnect() is called while in the
 *                                     CONNECTED state, a BYE request is sent to the remote
 *                                     party and the call assumes the DISCONNECTING state.
 *                                     Upon receipt of a response on the BYE request, the
 *                                     call-leg assumes the DISCONNECTED state.
 * RVSIP_CALL_LEG_STATE_DISCONNECTED: When a call-leg receives a BYE request from the remote
 *                                    party, or a final response to a previously sent BYE
 *                                    request, it assumes the DISCONNECTED state. The call-leg
 *                                    can also reach the DISCONNECTED state when an incoming
 *                                    call is rejected or when there is a time-out. This state
 *                                    notifies you that the call has disconnected and is about
 *                                    to be terminated. This is the final state at which you
 *                                    can still reference the call-leg using the Call-Leg
 *                                    API functions.
 * RVSIP_CALL_LEG_STATE_TERMINATED:   This state is the final call-leg state. When a call is
 *                                    terminated, the call-leg assumes the TERMINATED state.
 *                                    Upon reaching the TERMINATED state, you can no longer
 *                                    reference the call-leg.
 * RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED: This state is assumed only if the manualAckOn2xx
 *                                       configuration parameter is set to RV_TRUE when the
 *                                       SIP Stack initializes. If the call-leg received a
 *                                       2xx response to an initial INVITE request and the
 *                                       SIP Stack is configured to work in a manual ACK mode,
 *                                       the SIP Stack will assume the REMOTE_ACCEPTED state.
 *                                       In this state, the application must call the
 *                                       RvSipCallLegAck() function is order to send the ACK
 *                                       request to the remote party. After the ACK is sent,
 *                                       the call-leg will assume the CONNECTED state.
 * RVSIP_CALL_LEG_STATE_CANCELLED: Upon receiving a CANCEL request in the OFFERING state,
 *                                 the SIP Stack will automatically accept the CANCEL and move
 *                                 to the CANCELLED state. If the SIP Stack is configured to
 *                                 work in the manual behavior mode, your application will be
 *                                 responsible for responding with 487. Otherwise, 487 will
 *                                 be sent automatically. If you call the RvSipCallLegAccept()
 *                                 function in this state, the call-leg will move to the ACCEPTED state.
 * RVSIP_CALL_LEG_STATE_CANCELLING: If the application calls the RvSipCallLegCancel() or
 *                                  RvSipCallLegDisconnect() functions on a call-leg while in the
 *                                  PROCEEDING or PROCEEDING_TIMEOUT states, a CANCEL request is
 *                                  sent and the call-leg assumes the CANCELLING state. If a
 *                                  positive 200 response is received on the Invite request despite
 *                                  the cancel attempt, the call-leg generates a BYE request and
 *                                  moves to the DISCONNECTING state. If, however, a non-200
 *                                  response is received, the call-leg moves to the DISCONNECTED state.
 * RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT: This state is assumed only if the
 *                                          enableInviteProceedingTimeoutState configuration parameter
 *                                          is set to RV_TRUE when the SIP Stack initializes. The call
 *                                          moves to this state from the PROCEEDING state when the
 *                                          provisional timer expires before receiving a final response.
 *                                          In this state, the application can cancel the call-leg or
 *                                          terminate it.
 */

typedef enum
{
    RVSIP_CALL_LEG_STATE_UNDEFINED = -1,
    RVSIP_CALL_LEG_STATE_IDLE,
    RVSIP_CALL_LEG_STATE_INVITING,
    RVSIP_CALL_LEG_STATE_REDIRECTED,
    RVSIP_CALL_LEG_STATE_UNAUTHENTICATED,
    RVSIP_CALL_LEG_STATE_OFFERING,
    RVSIP_CALL_LEG_STATE_ACCEPTED,
    RVSIP_CALL_LEG_STATE_CONNECTED,
    RVSIP_CALL_LEG_STATE_DISCONNECTED,
    RVSIP_CALL_LEG_STATE_DISCONNECTING,
    RVSIP_CALL_LEG_STATE_TERMINATED,
    RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED,
    RVSIP_CALL_LEG_STATE_CANCELLED,
    RVSIP_CALL_LEG_STATE_CANCELLING,
    RVSIP_CALL_LEG_STATE_PROCEEDING,
    RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT,
    RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE

} RvSipCallLegState;

/* RvSipCallLegModifyState
 *-------------------------------------------------------------------------
 * RvSipCallLegModifyState represents a state of a re-INVITE process in
 * a connected session. The state changes
 * are reported using the RvSipCallLegModifyStateChangedEv call back function
 * together with a state change reason that may indicate to the application
 * how the new state was reached.
 * RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED - The call cannot send or receive a
 *                                         re-INVITE. Usually this state indicates
 *                                         that the call is not in the CONNECTED state.
 * RVSIP_CALL_LEG_MODIFY_STATE_IDLE      - The initial state of the Re-INVITE state machine.
 *                                         Upon creation of a re-Invite object, it assumes
 *                                         the IDLE modify state. It remains in this state
 *                                         until RvSipCallLegReInviteRequest() is called,
 *                                         whereupon it should move to the REINVITE_SENT
 *                                         modify state.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD  - A re-INVITE request was received.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT - A response was sent for an incoming
 *                                                      re-INVITE request.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT - A re-INVITE request was sent.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED - The remote party accepted the re-INVITE request.
 *                                                        This state will be reported only if the
 *                                                        manualAckOn2xx configuration parameter is set
 *                                                        to RV_TRUE. In this case, the ACK message will
 *                                                        not be sent automatically and the application
 *                                                        must initiate the ACK message by calling the
 *                                                        RvSipCallLegReInviteAck() function.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_RCVD - A final response was received on an outgoing
 *                                                      re-INVITE request.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING - A CANCEL request was sent on the re-INVITE request.
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING - A provisional response was received on an outgoing
 *                                                   re-INVITE request. The Modify state machine moves
 *                                                   to this state from the REINVITE_SENT state (meaning
 *                                                   that this state is assumed only when the first
 *                                                   provisional response is received).
 * RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED - Upon receiving a CANCEL request in the REINVITE_RCVD
 *                                                  state, the SIP Stack will automatically accept the CANCEL
 *                                                  and move to the REINVITE_CANCELLED state. A 487 response
 *                                                  will be sent automatically as a response to the INVITE
 *                                                  request unless the SIP Stack is configured to work in
 *                                                  manual behavior mode, in which your application will
 *                                                  be responsible for the INVITE final response.
 * RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE - This state is assumed only if the SIP Stack is compiled
 *                                                with the Enhanced DNS feature. The Modify state machine
 *                                                moves to this state from the REINVITE_SENT state when
 *                                                the re-INVITE request receives a network error, 503
 *                                                response, or timeout on the request. In this state
 *                                                your application can decide to:
 *                                                --Continue DNS: send an ACK request if the failure is
 *                                                because of a 503 response, and try to send the request
 *                                                to the next address in the transaction DNS list. For
 *                                                more information see the Working with DNS chapter.
 *                                                --Give up: send an ACK request if the failure is because
 *                                                of a 503 response, and terminate the re-Invite object.
 *                                                --Terminate the re-Invite object.
 * RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT   - An ACK was sent for a re-INVITE.
 * RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD   - An ACK was received for a re-INVITE.
 * RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED - The final state of the re-INVITE object. Upon reaching this
 *                                          state, you can no longer reference the re-INVITE object.
 */
typedef enum
{
    RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED = -1,
    RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_RCVD,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED,
    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT,
    RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE,
    RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
    RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD,
    RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED

}RvSipCallLegModifyState;

/* RvSipCallLegStateChangeReason
 * ------------------------------------------------------------------
 * A call-leg object has an inner state. The state changes according
 * to the state machine defined for the call-leg. Each state change
 * is explained by one of the reasons enumerated bellow.
 *
 * RVSIP_CALL_LEG_REASON_UNDEFINED           - The reason is undefined.
 * RVSIP_CALL_LEG_REASON_LOCAL_INVITING      - The local party invited the remote party.
 * RVSIP_CALL_LEG_REASON_REMOTE_INVITING     - The remote party invited the local party.
 * RVSIP_CALL_LEG_REASON_LOCAL_REFER         - The local party sent a REFER request to the remote party.
 * RVSIP_CALL_LEG_REASON_REMOTE_REFER        - The remote party sent a REFER request to the local party.
 * RVSIP_CALL_LEG_REASON_LOCAL_REFER_NOTIFY        - The local party sent a refer-NOTIFY request to the remote party.
 * RVSIP_CALL_LEG_REASON_REMOTE_REFER_NOTIFY       - The remote party sent a refer-NOTIFY request to the local party.
 * RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED     - The remote party accepted the call.
 * RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED      - The local party accepted the call.
 * RVSIP_CALL_LEG_REASON_REMOTE_ACK          - The remote party sent an ACK.
 * RVSIP_CALL_LEG_REASON_REDIRECTED          - A 3XX was received.
 * RVSIP_CALL_LEG_REASON_LOCAL_REJECT        - The local party rejected the call.
 * RVSIP_CALL_LEG_REASON_REQUEST_FAILURE     - A 4XX was received.
 * RVSIP_CALL_LEG_REASON_SERVER_FAILURE      - A 5XX was received.
 * RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE      - A 6XX was received.
 * RVSIP_CALL_LEG_REASON_LOCAL_DISCONNECTING - The local party sent a BYE request and is waiting for
 *                                             a response.
 * RVSIP_CALL_LEG_REASON_DISCONNECTED        - The local party received a final response
 *                                             to a previously sent BYE request.
 * RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECTED - The remote party sent a BYE request.
 * RVSIP_CALL_LEG_REASON_LOCAL_FAILURE       - The local party failed. For example
 *                                             it failed to send a request.
 * RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT      - There was a time-out. For example, an
 *                                             expected response did not arrived.
 * RVSIP_CALL_LEG_REASON_CALL_TERMINATED     - The call-leg was terminated.
 * RVSIP_CALL_LEG_REASON_AUTH_NEEDED         - A 401 or 407 response was received.
 * RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS - A 401 or 407 response was received
 *                                              but the authentication parameters
 *                                              are not supported.
 * RVSIP_CALL_LEG_REASON_REMOTE_PROVISIONAL_RESP -
 * The remote party sent a provisional response to the INVITE or reINVITE request.
 * RVSIP_CALL_LEG_REASON_REMOTE_REFER_REPLACES -
 * The remote party sent a REFER request with a Replaces header inside the Refer-To header..
 * RVSIP_CALL_LEG_REASON_REMOTE_INVITING_REPLACES -
 * The remote party sent an INVITE request with the Replaces header.
 * RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECT_REQUESTED -
 * The remote party sent a BYE request.
 * RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_REJECT -
 * The local party rejected the BYE request it received.
 * RVSIP_CALL_LEG_REASON_DISCONNECT_REMOTE_REJECT -
 * The remote party rejected the BYE request it received.
 * RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_ACCEPT -
 * The local party accepted the BYE request it received.
 * RVSIP_CALL_LEG_REASON_NETWORK_ERROR -
 * One of the reasons for moving to the MSG_SEND_FAILURE state. The message failed to
 * be sent due to a network error.
 * RVSIP_CALL_LEG_REASON_503_RECEIVED -
 * One of the reasons for moving to the MSG_SEND_FAILURE state. The message received a 503
 * response.
 * RVSIP_CALL_LEG_REASON_GIVE_UP_DNS -
 * The local party decided to give up sending a message after its previous send failed
 * (DNS feature).
 * RVSIP_CALL_LEG_REASON_CONTINUE_DNS -
 * The local party decided to send a message to the next address in its transaction DNS list,
 * after its previous send failed (DNS feature).
 * RVSIP_CALL_LEG_REASON_OUT_OF_RESOURCES -
 * Out-of-resource problem.
 * RVSIP_CALL_LEG_REASON_FORKED_CALL_NO_FINAL_RESPONSE -
 * A call-leg that was created by a 1xx response on a forked request did not get a final response.
 */
typedef enum
{
    RVSIP_CALL_LEG_REASON_UNDEFINED = -1,
    RVSIP_CALL_LEG_REASON_LOCAL_INVITING,
    RVSIP_CALL_LEG_REASON_REMOTE_INVITING,
    RVSIP_CALL_LEG_REASON_LOCAL_REFER,
    RVSIP_CALL_LEG_REASON_REMOTE_REFER,
    RVSIP_CALL_LEG_REASON_LOCAL_REFER_NOTIFY,
    RVSIP_CALL_LEG_REASON_REMOTE_REFER_NOTIFY,
    RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED,
    RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED,
    RVSIP_CALL_LEG_REASON_REMOTE_ACK,
    RVSIP_CALL_LEG_REASON_REDIRECTED,
    RVSIP_CALL_LEG_REASON_LOCAL_REJECT,
    RVSIP_CALL_LEG_REASON_REQUEST_FAILURE,
    RVSIP_CALL_LEG_REASON_SERVER_FAILURE,
    RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE,
    RVSIP_CALL_LEG_REASON_LOCAL_DISCONNECTING,
    RVSIP_CALL_LEG_REASON_DISCONNECTED,
    RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECTED,
    RVSIP_CALL_LEG_REASON_LOCAL_FAILURE,
    RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT,
    RVSIP_CALL_LEG_REASON_CALL_TERMINATED,
    RVSIP_CALL_LEG_REASON_AUTH_NEEDED,
    RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS,
    RVSIP_CALL_LEG_REASON_LOCAL_CANCELLING,
    RVSIP_CALL_LEG_REASON_REMOTE_CANCELED,
    RVSIP_CALL_LEG_REASON_ACK_SENT,
    RVSIP_CALL_LEG_REASON_CALL_CONNECTED,
    RVSIP_CALL_LEG_REASON_REMOTE_PROVISIONAL_RESP,
    RVSIP_CALL_LEG_REASON_REMOTE_REFER_REPLACES,
    RVSIP_CALL_LEG_REASON_REMOTE_INVITING_REPLACES,
    RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECT_REQUESTED,
    RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_REJECT,
    RVSIP_CALL_LEG_REASON_DISCONNECT_REMOTE_REJECT,
    RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_ACCEPT,
    RVSIP_CALL_LEG_REASON_NETWORK_ERROR,
    RVSIP_CALL_LEG_REASON_503_RECEIVED,
    RVSIP_CALL_LEG_REASON_GIVE_UP_DNS,
    RVSIP_CALL_LEG_REASON_CONTINUE_DNS,
    RVSIP_CALL_LEG_REASON_OUT_OF_RESOURCES,
    RVSIP_CALL_LEG_REASON_FORKED_CALL_NO_FINAL_RESPONSE
} RvSipCallLegStateChangeReason;


/* RvSipCallLegReferState
 * ----------------------------------------------------------------------
 * This enumeration is deprecated and is kept for backward compatibility only.
 *
 * RVSIP_CALL_LEG_REFER_STATE_UNDEFINED: No refer-state for this call leg.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE: Indicates that the call-leg is ready to
 *                                        process an incoming or outgoing REFER
 *                                        message (assuming that the call-leg regular
 *                                        state is IDLE or CONNECTED and there is no
 *                                        re-INVITE process). This is the initial
 *                                        refer-state of the call-leg. The call-leg
 *                                        remains in the REFER_IDLE state until the
 *                                        RvSipCallLegRefer() function is called,
 *                                        whereupon it should move to the REFER_SENT
 *                                        state. Receipt of the REFER request in the
 *                                        REFER_IDLE refer-state moves the call-leg
 *                                        to REFER_RECEIVED refer-state.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_SENT: Indicates that a REFER request was sent
 *                                        to the remote party. When the RvSipCallLegRefer()
 *                                        function is called, the call-leg sends a REFER
 *                                        request and assumes the REFER_SENT refer-state.
 *                                        The call-leg remains in this state until it
 *                                        receives a final response from the remote party.
 *                                        If a 3xx class response is received, the call-leg
 *                                        moves to the REFER_REDIRECTED refer-state. If a
 *                                        401 or 407 response is received, the call-leg moves
 *                                        to the REFER_UNAUTHENTICATED refer-state. For any
 *                                        other final response (2xx, 4xx, 5xx or 6xx),
 *                                        the call-leg moves back to the REFER_IDLE refer-state.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_CANCELLING: The client transaction was cancelled. The
 *                                              transaction is waiting for the final response
 *                                              of the server. Since it is no longer valid
 *                                              to cancel a general transaction, this state
 *                                              will not be reached.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED: Indicates that a 401 or 407 response
 *                                                   to a REFER request was received by
 *                                                   the call-leg. At this point, you may
 *                                                   re-send your request with authentication
 *                                                   information by calling the RvSipCallLegRefer()
 *                                                   function again. You can also end the refer
 *                                                   process by calling the RvSipCallLegReferEnd() function.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_REDIRECTED: Indicates a 3xx response to a REFER request was received
 *                                              by the call-leg. At this point, you may confirm the
 *                                              redirection by calling the RvSipCallLegRefer()
 *                                              function again. You can also decide to end the refer
 *                                              process by calling the RvSipCallLegReferEnd() function.
 * RVSIP_CALL_LEG_REFER_STATE_REFER_RCVD: A REFER request was received by the call-leg. In this state,
 *                                        it is up to the application to decide whether to accept or
 *                                        reject the REFER request using the Call-leg REFER API..
 *
 */

typedef enum
{
    RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED = -1,
    RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE,

    /*Outgoing REFER*/
    RVSIP_CALL_LEG_REFER_STATE_REFER_SENT,
    RVSIP_CALL_LEG_REFER_STATE_REFER_CANCELLING,
    RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED,
    RVSIP_CALL_LEG_REFER_STATE_REFER_REDIRECTED,

    /*Incoming REFER*/
    RVSIP_CALL_LEG_REFER_STATE_REFER_RCVD,

    RVSIP_CALL_LEG_REFER_STATE_MSG_SEND_FAILURE

} RvSipCallLegReferState;


/* RvSipCallLegReferNotifyEvents
 * ----------------------------------------------------------------------
 * This enumeration is deprecated and is kept for backward compatibility only.
 *
 * RVSIP_CALL_LEG_REFER_NOTIFY_UNDEFINED: No refer-notify event for this call leg.
 * RVSIP_CALL_LEG_REFER_NOTIFY_READY: Indicates that a connect attempt that was
 *                                    triggered by a REFER request has reached a
 *                                    final status. After being notified with this
 *                                    event, you need to decide whether to send a
 *                                    NOTIFY request to the REFER sender, or to
 *                                    end the REFER request without sending a
 *                                    NOTIFY request. To send NOTIFY request,
 *                                    use the RvSipCallLegReferNotify() function.
 *                                    To end the refer process without sending NOTIFY,
 *                                    use the RvSipCallLegReferEnd() function.
 * RVSIP_CALL_LEG_REFER_NOTIFY_SENT:  Indicates that a NOTIFY request (related to a
 *                                    previously received REFER request) was sent
 *                                    to the remote party.
 * RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED: Indicates that a 401 or 407 response
 *                                              was received for a REFER-related NOTIFY
 *                                              request. At this point, you may re-send
 *                                              a NOTIFY request with authentication
 *                                              information by calling the
 *                                              RvSipCallLegReferNotify() function again.
 * RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED:     Indicates that a 3xx class response was received
 *                                             for a REFER-related NOTIFY request. At this point,
 *                                             you may confirm the redirection by calling the
 *                                             RvSipCallLegReferNotify() function again.
 * RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RECVD: Indicates that a final response of classes 2xx,
 *                                             4xx (except 401 and 407), 5xx, or 6xx was received
 *                                             for a REFER-related NOTIFY request. If the call-leg
 *                                             is in the IDLE state, it will be automatically
 *                                             terminated after notifying the event.
 * RVSIP_CALL_LEG_REFER_NOTIFY_RECVD: Indicates that a REFER-related NOTIFY request was received
 *                                    by the call-leg. The call-leg automatically responds with
 *                                    a 200 response. If the call-leg was in the IDLE state,
 *                                    it is automatically terminated after notifying the event
 *                                    and responding with a 200 response.
 *
 */

typedef enum
{
    RVSIP_CALL_LEG_REFER_NOTIFY_UNDEFINED = -1,

    /*Outgoing REFER*/
    RVSIP_CALL_LEG_REFER_NOTIFY_READY,
    RVSIP_CALL_LEG_REFER_NOTIFY_SENT,
    RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED,
    RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED,
    RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RCVD,

    /*Incoming REFER*/
    RVSIP_CALL_LEG_REFER_NOTIFY_RCVD

} RvSipCallLegReferNotifyEvents;


/* RvSipCallLegDirection
 * ------------------------------------------------------------------
 * Indicates whether the call-leg represents an incoming or an
 * outgoing call.
 */
typedef enum
{
    RVSIP_CALL_LEG_DIRECTION_UNDEFINED = -1,
    RVSIP_CALL_LEG_DIRECTION_INCOMING,       /*incoming call-leg*/
    RVSIP_CALL_LEG_DIRECTION_OUTGOING        /*outgoing call-leg*/
}RvSipCallLegDirection;


/* RvSipCallLegTranscState
 * ------------------------------------------------------------------
 * Indicates the state of a general transaction, such as INFO that
 * belongs to a call-leg.
 */
typedef enum
{
    RVSIP_CALL_LEG_TRANSC_STATE_UNDEFINED = -1,
    RVSIP_CALL_LEG_TRANSC_STATE_IDLE,
    RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD,
    RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT,
    RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT,
    RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_PROCEEDING,
    RVSIP_CALL_LEG_TRANSC_STATE_TERMINATED,
    RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD,
    RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE
} RvSipCallLegTranscState;

/* RvSipCallLegByeState
 * ------------------------------------------------------------------
 * Indicates the state of a received BYE request.
 */

typedef enum
{
    RVSIP_CALL_LEG_BYE_STATE_UNDEFINED = -1,
    RVSIP_CALL_LEG_BYE_STATE_IDLE,
    RVSIP_CALL_LEG_BYE_STATE_REQUEST_RCVD,
    RVSIP_CALL_LEG_BYE_STATE_RESPONSE_SENT,
    RVSIP_CALL_LEG_BYE_STATE_TERMINATED,
    RVSIP_CALL_LEG_BYE_STATE_DETACHED  /*The call-leg is about to detach from the transaction;
                                          no more states will be available for this transaction.*/
} RvSipCallLegByeState;


/* RvSipCallLegTranscStatus
 * ------------------------------------------------------------------
 * This enumeration (in addition to RvSipCallLegTranscResolvedEv()),
 * is deprecated and kept for backward compatibility only.
 */
typedef enum
{
    RVSIP_CALL_LEG_TRANSC_STATUS_UNDEFINED = -1,
    RVSIP_CALL_LEG_TRANSC_STATUS_RESPONSE_RCVD,
    RVSIP_CALL_LEG_TRANSC_STATUS_TIME_OUT,
    RVSIP_CALL_LEG_TRANSC_STATUS_MSG_SEND_FAILURE
} RvSipCallLegTranscStatus;



/* RvSipCallLegPrackState
 * ------------------------------------------------------------------
 * Indicates the status of a PRACK process. A PRACK process can start in two cases:
 *     --When the application calls the
 *     RvSipCallLegProvisionalResponseReliable() function.
 *     --When a reliable provisional response is received by the call-leg.
 * The PRACK process is automated by the Transaction layer and involves
 * the PRACK request and response. The RvSipCallLegPrackStateChangedEv()
 * callback is only a notification to the application about the PRACK
 * process and no response is expected for the different states.
 * RVSIP_CALL_LEG_PRACK_STATE_UNDEFINED - No defined state.
 * RVSIP_CALL_LEG_PRACK_STATE_REL_PROV_RESPONSE_RCVD - The call-leg received
 *                                                     a reliable provisional response.
 * RVSIP_CALL_LEG_PRACK_STATE_PRACK_SENT - A PRACK request was automatically sent
 *                                         for this call-leg.
 * RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD - A final response was
 *                                         received for a previously sent PRACK.
 * RVSIP_CALL_LEG_PRACK_STATE_PRACK_PRACK_RCVD - The call-leg received a PRACK
 *                                         after sending a reliable provisional
 *                                         response.
 * RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_SENT - A final response was sent
 *                                                        on a received PRACK.
 */

typedef enum
{
    RVSIP_CALL_LEG_PRACK_STATE_UNDEFINED = -1,
    RVSIP_CALL_LEG_PRACK_STATE_REL_PROV_RESPONSE_RCVD,
    RVSIP_CALL_LEG_PRACK_STATE_PRACK_SENT,
    RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD,
    RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD,
    RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_SENT
} RvSipCallLegPrackState;

/* RvSipCallLegReplacesStatus
 * ---------------------------------
 * Indicates in which type of header the Replaces option tag appeared
 * in an incoming request. You can get the Replaces status by calling
 * the RvSipCallLegGetReplacesStatus() function.
 */
typedef enum
{
    RVSIP_CALL_LEG_REPLACES_UNDEFINED = -1, /*The Replaces option tag was not found in the request.*/
    RVSIP_CALL_LEG_REPLACES_SUPPORTED,      /*The Replaces option tag was found in a Supported header.*/
    RVSIP_CALL_LEG_REPLACES_REQUIRED        /*The Replaces option tag was found in a Require header.*/
} RvSipCallLegReplacesStatus;

/* RvSipCallLegReplacesReason
 * ---------------------------------
 * Indicates why the call-leg that matches the identifiers in the
 * Replaces header was not found. It is used in
 * RvSipCallLegReplacesGetMatchedCallExt() function.
 * RVSIP_CALL_LEG_REPLACES_REASON_UNDEFINED - No specified reason.
 * RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_FOUND_OK -
 *                      The matched call-leg was found correctly.
 * RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_NOT_FOUND -
 *                      The matched call-leg was not found.
 * RVSIP_CALL_LEG_REPLACES_REASON_FOUND_NON_INVITE_DIALOG -
 *                      A dialog was found with same dialog identifiers,
 *                      but the dialog was not established with an INVITE request.
 * RVSIP_CALL_LEG_REPLACES_REASON_FOUND_CONFIRMED_DIALOG -
 *                      A confirmed dialog was found, but the Replaces header
 *                      contains the "early-only" parameter. Therefore, the
 *                      application should reject the request with 486.
 * RVSIP_CALL_LEG_REPLACES_REASON_FOUND_INCOMING_EARLY_DIALOG - An incoming
 *                      dialog was found in an early state (OFFERING), which
 *                      is not allowed by the Replaces draft.
 * RVSIP_CALL_LEG_REPLACES_REASON_FOUND_TERMINATED_DIALOG -
 *                      A dialog was found in the TERMINATED state..
 */
typedef enum
{
    RVSIP_CALL_LEG_REPLACES_REASON_UNDEFINED = -1,
    RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_FOUND_OK,
    RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_NOT_FOUND,
    RVSIP_CALL_LEG_REPLACES_REASON_FOUND_NON_INVITE_DIALOG,
    RVSIP_CALL_LEG_REPLACES_REASON_FOUND_CONFIRMED_DIALOG,
    RVSIP_CALL_LEG_REPLACES_REASON_FOUND_INCOMING_EARLY_DIALOG,
    RVSIP_CALL_LEG_REPLACES_REASON_FOUND_TERMINATED_DIALOG
}RvSipCallLegReplacesReason;

/* RvSipCallLegSessionTimerNotificationReason
 * ------------------------------------------
 * The session timer notification reason, which describes
 * the reason for the session timer notification.
 * RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_UNDEFINED -
 *                      No specified reason.
 * RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES -
 *                      The session time is about to expire.
 * RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_422_RECEIVED -
 *                      A 422 response was received during
 *                      the session timer.
 */
typedef enum
{
    RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_UNDEFINED = -1,
    RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES,
    RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_422_RECEIVED
} RvSipCallLegSessionTimerNotificationReason;

/* RvSipCallLegSessionTimerNegotiationReason
 * ------------------------------------------
 * The session-timer negotiation reason, which describes the
 * reason for the negotiation failure.
 * RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED,
 *   The remote party does not support the session-timer extension.
 *
 * RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT
 *   The refresher preference does not match the call refresher.
 */
typedef enum
{
    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_UNDEFINED = -1,
    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED,
    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT
} RvSipCallLegSessionTimerNegotiationReason;

/* RvSipCallLegSessionTimerRefresherPreference
 * -------------------------------------------
 * Saves the application preference for the call.
 * RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL
 *            The UA wishes to be the refresher.
 * RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE
 *            The UA wishes the remote party to be the refresher.
 * RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE
 *            The UA does not care who will be the refresher.
 */
typedef enum
{
    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_NONE = -1,
    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL,
    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE,
    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE
}RvSipCallLegSessionTimerRefresherPreference;

/* RvSipCallLegHARestoreMode
 * -------------------------------------------
 * Notifies the Stack of high-availability mode when
 * calling the RvSipCallLegRestoreOldVersionConnectedCall() function.
 * RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITH_TLS
 *            The store is performed from a file generated by Stack
 *            version 3.0, compiled with TLS.
 * RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITHOUT_TLS
 *            The store is performed from a file generated by Stack
 *            version 3.0, compiled without TLS.
 */
typedef enum
{
    RVSIP_CALL_LEG_H_A_RESTORE_MODE_UNDEFINED = -1,
    RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITH_TLS,
    RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITHOUT_TLS
}RvSipCallLegHARestoreMode;


/*-----------------------------------------------------------------------
       C A L L  - L E G:  C A L L - B A C K  F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegCreatedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a new call-leg was created
 *           and exchanges handles with the application.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The new SIP Stack call-leg handle.
 * Output:     phAppCallLeg - The application handle for this call-leg.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegCreatedEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   OUT RvSipAppCallLegHandle         *phAppCallLeg);




/***************************************************************************
 * RvSipCallLegStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application of a call-leg state change. For each state
 * change, the new state is supplied with the reason for the state change.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            eState -      The new call-leg state.
 *            eReason -     The reason for the state change.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegStateChangedEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg,
                                   IN  RvSipCallLegState             eState,
                                   IN  RvSipCallLegStateChangeReason eReason);

/***************************************************************************
 * RvSipCallLegMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a call-leg related outgoing
 *          message is about to be sent.
 * Return Value: RV_OK (The returned status is ignored in the current
 *                           SIP Stack version.)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hMsg -        The handle to the outgoing message.
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * RvSipCallLegMsgToSendEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg,
                                   IN  RvSipMsgHandle                hMsg);


/***************************************************************************
 * RvSipCallLegMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that an incoming call-leg-related
 *          message has been received.
 * Return Value: RV_OK (The returned status is ignored in the current
 *                           SIP Stack version. If you return a value other
 *                           than RV_OK, the incoming message will be ignored.
 *                           The underlying transaction will terminate with an
 *                           error and the call-leg will terminate as well.)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The new SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hMsg -        The handle to the outgoing message.
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * RvSipCallLegMsgReceivedEv)(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    IN  RvSipAppCallLegHandle         hAppCallLeg,
                                    IN  RvSipMsgHandle                hMsg);


/***************************************************************************
 * RvSipCallLegModifyStateChangedEv
 * ------------------------------------------------------------------------
 * General: This callback is DEPRECATED. It will be called only if stack was
 *          configured to work with old invite behavior.
 *          For normal stack configuration, use RvSipCallLegReInviteCreatedEv and 
 *          RvSipCallLegReInviteStateChangedEv.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            eModifyState -The new MODIFY state.
 *          eReason     - The reason for the new state.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegModifyStateChangedEv)(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    IN  RvSipAppCallLegHandle         hAppCallLeg,
                                    IN  RvSipCallLegModifyState       eModifyState,
                                    IN  RvSipCallLegStateChangeReason eReason);

/***************************************************************************
 * RvSipCallLegReInviteCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new INVITE object was created
 *          as a result of an incoming re-INVITE request.
 *          The SIP Stack exchanges handles with the application in
 *          this callback function.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hReInvite   - The handle to the new INVITE object.
 * Output:    phAppReInvite - The application handle for this INVITE object.
 *            pResponseCode - If the application wants to reject the re-INVITE
 *            request, it can supply the response code here.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegReInviteCreatedEv)(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    IN  RvSipAppCallLegHandle         hAppCallLeg,
                                    IN  RvSipCallLegInviteHandle      hReInvite,
                                    OUT RvSipAppCallLegInviteHandle   *phAppReInvite,
                                    OUT RvUint16                      *pResponseCode);


/***************************************************************************
 * RvSipCallLegReInviteStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the MODIFY state, related to a
 *          re-INVITE process, was changed. Before this callback function
 *          is called, a re-INVITE object must be created. (To create an
 *          outgoing re-INVITE, the application should call
 *          RvSipCallLegReInviteCreate().
 *          An incoming re-INVITE object is created by the Stack, using
 *          the RvSipCallLegReInviteCreatedEv() callback function.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hReInvite   - The SIP Stack re-INVITE handle.
 *            hAppReInvite - The application handle for this re-INVITE.
 *            eModifyState -The new re-INVITE state.
 *            eReason     - The reason for the new state.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegReInviteStateChangedEv)(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    IN  RvSipAppCallLegHandle         hAppCallLeg,
                                    IN  RvSipCallLegInviteHandle      hReInvite,
                                    IN  RvSipAppCallLegInviteHandle   hAppReInvite,
                                    IN  RvSipCallLegModifyState       eModifyState,
                                    IN  RvSipCallLegStateChangeReason eReason);


/***************************************************************************
 * RvSipCallLegModifyRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 * Use RvSipCallLegModifyStateChangedEv() instead.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegModifyRequestRcvdEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg);


/***************************************************************************
 * RvSipCallLegModifyResultRcvdEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegModifyStateChangedEv() instead.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            status -      The final response code.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegModifyResultRcvdEv)(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    IN  RvSipAppCallLegHandle         hAppCallLeg,
                                    IN  RvUint16                     status);



/***************************************************************************
 * RvSipCallLegTrancRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 *          Use RvSipCallLegTranscStateChangedEv() instead.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the transaction that received the response.
 *            hAppTransc -  The application handle to the call-leg transaction.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegTrancRequestRcvdEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg,
                                   IN  RvSipTranscHandle             hTransc,
                                   IN  RvSipAppTranscHandle          hAppTransc);

/***************************************************************************
 * RvSipCallLegTranscResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
*           Use RvSipCallLegTranscStateChangedEv() instead.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the transaction that received the response.
 *            hAppCallLeg - The application handle for this call-leg.
 *            eStatus -     The resolution status. If the status is
 *                          RVSIP_CALL_TRANSC_STATUS_RESPONSE_RECVD,
 *                          the responseCode parameter will contain the
 *                          response message status code.
 *            responseCode - The response status code.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegTranscResolvedEv)(
                              IN  RvSipCallLegHandle               hCallLeg,
                              IN  RvSipAppCallLegHandle            hAppCallLeg,
                              IN  RvSipTranscHandle                hTransc,
                              IN  RvSipAppTranscHandle             hAppTransc,
                              IN  RvSipCallLegTranscStatus         eStatus,
                              IN  RvUint16                        responseCode);


/***************************************************************************
 * RvSipCallLegTranscCreatedEv
 * ------------------------------------------------------------------------
 * General:Notifies that a new general transaction other then BYE was created
 * and relates to the specified call-leg. The application can replace handles
 * with the transaction and specify whether it wants to handle the request.
 * If so, it will be notified of the transaction states and will have to
 * respond to the request.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the new transaction
 * Output:  hAppTransc - The application handle to the transaction.
 *          bAppHandleTransc -  RV_TRUE if the application wants to
 *                        handle the request. RV_FALSE if the application
 *                        wants the call-leg to handle the request.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegTranscCreatedEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg,
                                   IN  RvSipTranscHandle             hTransc,
                                   OUT RvSipAppTranscHandle          *hAppTransc,
                                   OUT RvBool                       *bAppHandleTransc);


/***************************************************************************
 * RvSipCallLegTranscStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application of a state change for a general
 * transaction that belongs to the supplied call-leg. When the state
 * indicated that a request was received, the application can call
 * RvSipCallLegTranscResponse() and respond to the request. If the
 * application does not want to handle the request, it should set
 * the bHandleRequest to RV_FALSE. In this case, the Stack will
 * either handle the transaction by itself or respond with a 501
 * not implemented. When the state indicates that a response was
 * received, the responseCode parameter will hold the response code.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The stack handle to the transaction.
 *          hAppTransc -  The application handle to the transaction.
 *          eTranscState - The new state of the call-leg transaction.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegTranscStateChangedEv)(
                                   IN  RvSipCallLegHandle                hCallLeg,
                                   IN  RvSipAppCallLegHandle             hAppCallLeg,
                                   IN  RvSipTranscHandle                 hTransc,
                                   IN  RvSipAppTranscHandle              hAppTransc,
                                   IN  RvSipCallLegTranscState           eTranscState,
                                   IN  RvSipTransactionStateChangeReason eReason);


/***************************************************************************
 * RvSipCallLegByeCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies that a new BYE transaction was created and relates to
 * the specified call-leg. The application can replace handles with the
 * transaction and specify whether it wishes to handle the request.
 * If so, it will be notified of transaction states and will have to
 * respond to the request. If the application will not register to this
 * callback, BYE will automatically be responded to with 200 OK.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the BYE transaction
 * Output:  hAppTransc - The application handle to the transaction
 *          bAppHandleTransc -  RV_TRUE if the application wishes to handle the
 *                           request. RV_FALSE if the application wants the
 *                           call-leg to handle the request.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegByeCreatedEv)(
                                   IN  RvSipCallLegHandle            hCallLeg,
                                   IN  RvSipAppCallLegHandle         hAppCallLeg,
                                      IN  RvSipTranscHandle             hTransc,
                                   OUT RvSipAppTranscHandle          *hAppTransc,
                                   OUT RvBool                       *bAppHandleTransc);


/***************************************************************************
 * RvSipCallLegByeStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application of a state change for a BYE request
 * that belongs to the supplied call-leg. When the state indicated that
 * a request was received the application can use RvSipCallLegByeAccept()
 * or RvSipCallLegByeReject() to respond the BYE request.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the transaction.
 *          hAppTransc -  The application handle to the transaction.
 *          eTranscState - The new state of the call-leg transaction.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegByeStateChangedEv)(
                                   IN  RvSipCallLegHandle                hCallLeg,
                                   IN  RvSipAppCallLegHandle             hAppCallLeg,
                                   IN  RvSipTranscHandle                 hTransc,
                                   IN  RvSipAppTranscHandle              hAppTransc,
                                   IN  RvSipCallLegByeState              eByeState,
                                   IN  RvSipTransactionStateChangeReason eReason);

/***************************************************************************
 * RvSipCallLegReferStateChangedEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 * For REFER implementation, use the Subscription REFER API. To use the Call-leg
 * REFER deprecated API for code that was written for a previous SIP Stack version,
 * you must set the "disableRefer3515Behavior" configuration parameter to TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            eState -      The new refer-state of the call-leg.
 *            eReason -     The reason for the state change.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegReferStateChangedEv)(
                            IN  RvSipCallLegHandle                    hCallLeg,
                            IN  RvSipAppCallLegHandle                 hAppCallLeg,
                            IN  RvSipCallLegReferState                eState,
                            IN  RvSipCallLegStateChangeReason         eReason);

/***************************************************************************
 * RvSipCallLegReferCancelledEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 * Since it is no longer valid to cancel a general transaction, you must
 * not implement this callback.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipCallLegReferCancelledEv)(
                              IN  RvSipCallLegHandle               hCallLeg,
                              IN  RvSipAppCallLegHandle            hAppCallLeg);

/***************************************************************************
 * RvSipCallLegReferNotifyEv
 * ------------------------------------------------------------------------
 * General: This callback is deprecated and kept for backward compatibility only.
 * For REFER implementation, use the Subscription REFER API. To use the Call-leg
 * REFER deprecated API for code that was written for a previous SIP Stack
 * version, you must set the "disableRefer3515Behavior" configuration parameter to TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *            eNotification - The refer notify event.
 *            eReason -     The reason for the event occurrence.
 *          responseCode - This parameter has different meaning for
 *                         different events.
 *                         --For the RVSIP_CALL_LEG_REFER_NOTIFY_READY event,
 *                         RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED and
 *                         RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED events the
 *                         response code represents the final status of the
 *                         connect attempt.
 *                         --For the RVSIP_CALL_LEG_REFER_NOTIFY_RCVD and
 *                         RVSIP_CALL_LEG_REFER_NOTIFY_SENT events,
 *                         the response code represents the status code found in
 *                         the NOTIFY Request message body. In all other states,
 *                         the response code represents the status code of the
 *                         response to the NOTIFY request.
 *          cSeqStep - This parameter is relevant in the
 *                     RVSIP_CALL_LEG_REFER_NOTIFY_READY,
 *                     RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED and
 *                     RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED events. It indicates
 *                     the CSeq step of the REFER transaction to which this NOTIFY
 *                     is associated with.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegReferNotifyEv)(
                            IN  RvSipCallLegHandle              hCallLeg,
                            IN  RvSipAppCallLegHandle           hAppCallLeg,
                            IN  RvSipCallLegReferNotifyEvents   eNotification,
                            IN  RvSipCallLegStateChangeReason   eReason,
                            IN  RvInt16                         responseCode,
                            IN  RvInt32                         cSeqStep);

/***************************************************************************
 * RvSipCallLegPrackStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application of a call-leg PRACK event.
 * The PRACK state indicates the status of a PRACK process.
 * This callback is only a notification to the application and
 * no response is expected for the different states.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *          eState      - The PRACK process state.
 *          eReason     - The state reason.
 *          prackResponseCode - If the state indicates that a response
 *                              was received for a PRACK request, this
 *                              parameter will include the response code.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipCallLegPrackStateChangedEv) (
                      IN  RvSipCallLegHandle              hCallLeg,
                      IN  RvSipAppCallLegHandle           hAppCallLeg,
                      IN  RvSipCallLegPrackState          eState,
                      IN  RvSipCallLegStateChangeReason   eReason,
                      IN  RvInt16                        prackResponseCode);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipCallLegAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that credentials were found in the
 * Request message. This callback is called only if the application started
 * the authentication procedure by calling RvSipCallLegAuthBegin().
 * The callback supplies the application with the Authorization header that
 * contains these credentials.
 * At this stage the application should use the RvSipCallLegAuthProceedExt()
 * function. This callback also indicates whether the SIP Stack is capable
 * of verifying the credentials that were found. Whenever the SIP Stack does
 * not support the credentials (for example, if the algorithm is not MD5)
 * the application can verify the credentials by itself or instruct the
 * SIP Stack to continue to the next header.
 * Note: You must get the credentials from the given Authorization header
 * synchronically, or you should copy the header to use it a-synchronically.
 * When you have the credentials, you can check the validity and continue
 * with RvSipAuthProceedExt a-synchronically.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -   The SIP Stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the transaction that received the response.
 *          hAuthorization - The handle to the authorization handle that is found.
 *          bCredentialsSupported - Determines whether or not the SIP Stack supports
 *                                  the algorithm in the Authorization header.
 *                                  If the SIP Stack supports the algorithm, the user
 *                                  has to return the password of this username from
 *                                  the realm database, and give it to the SIP Stack
 *                                  in the RvSipXXXAuthProceedExt() function, indicating
 *                                  the proceeding action as
 *                                  RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD. If the
 *                                  algorithm is not supported, to continue the procedure,
 *                                  the user can skip the unsupported header by calling
 *                                  RvSipXXXAuthProceedExt(), indicating the proceeding
 *                                  action as RVSIP_TRANSC_AUTH_ACTION_SKIP. Or, to end
 *                                  the authentication procedure successfully, the user
 *                                  can check the header by him/herself and call the
 *                                  RvSipXXXAuthProceedExt() function, indicating the
 *                                  proceeding action as RVSIP_TRANSC_AUTH_ACTION_SUCCESS.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipCallLegAuthCredentialsFoundEv)(
                      IN    RvSipCallLegHandle              hCallLeg,
                      IN    RvSipAppCallLegHandle           hAppCallLeg,
                      IN    RvSipTranscHandle               hTransc,
                      IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                      IN    RvBool                         bCredentialsSupported);

/***************************************************************************
 * RvSipCallLegAuthCompletedEv
 * ------------------------------------------------------------------------
 * General:  Called when the authentication procedure is completed. If the
 * procedure is completed because the correct Authorization header was found,
 * bAuthSucceed is RV_TRUE. If the procedure is completed because there are
 * no more Authorization headers to check, or because user ordered to stop
 * the search for the correct header, bAuthSucceed is RV_FALSE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The sip stack call-leg handle
 *            hAppCallLeg - The application handle for this call-leg.
 *            hTransc -     The handle to the transaction that received the response.
 *          bAuthSucceed - RV_TRUE if the correct authorization header
 *                         was found. Otherwise, RV_FALSE.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipCallLegAuthCompletedEv)(
                      IN    RvSipCallLegHandle       hCallLeg,
                      IN    RvSipAppCallLegHandle    hAppCallLeg,
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvBool                  bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * RvSipCallLegSessionTimerRefreshAlertEv
 * ------------------------------------------------------------------------
 * General:  Notifies that the alert time (the time in which the application
 * needs to send a re-INVITE or UPDATE to refresh the call) is expired.
 * The application needs to send a refresh using the
 * RvSipCallLegSessionTimerRefresh() or
 * RvSipCallLegTranscSessionTimerGeneralRefresh() functions.
 * (This callback is called only in manual mode).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipCallLegSessionTimerRefreshAlertEv)(
                      IN    RvSipCallLegHandle       hCallLeg,
                      IN    RvSipAppCallLegHandle    hAppCallLeg);

/***************************************************************************
 * RvSipCallLegSessionTimerNotificationEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application about events related to the session-timer
 * feature. When the callback is called with the
 * RVSIP_CALL_LEG_SESSION_TIMER_SESSION_EXPIRES reason, it notifies the application
 * that the session time is about to expire. It is the responsibility of the
 * application to decide whether to send BYE or to do something else.
 * (This callback is called only in manual mode).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *          eReason - The reason for this notification.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipCallLegSessionTimerNotificationEv)(
                      IN RvSipCallLegHandle                         hCallLeg,
                      IN RvSipAppCallLegHandle                      hAppCallLeg,
                      IN RvSipCallLegSessionTimerNotificationReason eReason);

/***************************************************************************
 * RvSipCallLegSessionTimerNegotiationFaultEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application about negotiation problems while defining
 * the session-timer parameters for a call. When the callback is called with
 * the RVSIP_CALLLEG_SESSION_TIMER_DEST_NOT_SUPPORTED reason, it notifies the
 * local party that a 2xx final response was received, but the server does not
 * support the session-timer. The application should return synchronously
 * whether or not it wants to execute session-timer.
 * The session-timer mechanism can be operated as long as one of the two UAs
 * in the call-leg supports the extension. If the application decides to operate
 * the session-timer, that side will send the refresh. The other side will see
 * the refreshes as repetitive re-Invites. The default behavior is to execute
 * the session-timer mechanism for the call.
 * When the callback is called with the
 * RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT reason, it 
 * notifies the application that the refresher preference did not match the call refresher.
 * The application should return synchronously whether or not it wants to execute
 * the session-timer. If the application decides to operate the session-timer
 * mechanism, the refresher will be different from the application request. The
 * default behavior is to execute the session-timer mechanism for the call.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The SIP Stack call-leg handle.
 *            hAppCallLeg - The application handle for this call-leg.
 *          eReason - The reason for this negotiation fault .
 * Output:  bHandleSessionTimer - RV_TRUE if the application wants to
 *                                 operate the session time of this call.
 *                                 RV_FALSE if the application does not
 *                                 want to operate the session-timer of
 *                                 this call.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipCallLegSessionTimerNegotiationFaultEv)(
                      IN  RvSipCallLegHandle                         hCallLeg,
                      IN  RvSipAppCallLegHandle                      hAppCallLeg,
                      IN  RvSipCallLegSessionTimerNegotiationReason  eReason,
                      OUT RvBool                                    *bHandleSessionTimer);


/***************************************************************************
 * RvSipCallLegOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a message needs to be sent and
 * the destination address is a URL type that is currently not supported
 * by the SIP Stack. The URL has to be converted to a SIP URL for the
 * message to be sent.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The SIP Stack call-leg handle
 *        hAppCallLeg    - The application handle for this call-leg.
 *        hTransc        - The transaction handle.
 *        hAppTransc     - The application handle for this transaction.
 *        hMsg           - The message that includes the other URL address.
 *        hAddress       - The handle to the unsupported address to be converted.
 * Output:    hSipURLAddress - The handle to the SIP URL address. This is an empty
 *                               address object that the application should fill.
 *        bAddressResolved-Indicates whether or not the SIP URL address was filled.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipCallLegOtherURLAddressFoundEv)(
                     IN  RvSipCallLegHandle     hCallLeg,
                     IN  RvSipAppCallLegHandle  hAppCallLeg,
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipAppTranscHandle   hAppTransc,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle     hAddress,
                     OUT RvSipAddressHandle     hSipURLAddress,
                     OUT RvBool                *bAddressResolved);




/***************************************************************************
 * RvSipCallLegFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the call-leg is about to send a
 * message after the destination address was resolved. This callback supplies
 * the final message object and the transaction that is responsible for
 * sending this message. Changes in the message at this stage will not effect
 * the destination address. When this callback is called, the application can
 * query the transaction about the destination address using the
 * RvSipTransactionGetCurrentDestAddress() function. If the application wishes,
 * it can update the "sent-by" part of the top-most Via. The application must
 * not update the branch parameter.
 *
 * Return Value: RvStatus. If the application returns a value other than RV_OK,
 * the message will not be sent. The transaction will terminate with an error.
 * The call-leg will also terminate unless the transaction was initiated by
 * the application (for example, INFO).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The SIP Stack call-leg handle.
 *            hAppCallLeg    - The application handle for this call-leg.
 *            hTransc        - The transaction handle.
 *            hAppTransc     - The application handle for this transaction.
 *                             For INVITE/BYE the hAppTransc is NULL.
 *            hMsgToSend     - The handle to the outgoing message.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipCallLegFinalDestResolvedEv)(
                      IN  RvSipCallLegHandle     hCallLeg,
                      IN  RvSipAppCallLegHandle  hAppCallLeg,
                      IN  RvSipTranscHandle      hTransc,
                      IN  RvSipAppTranscHandle   hAppTransc,
                      IN  RvSipMsgHandle         hMsgToSend);



#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RvSipCallLegSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the timer of the transaction
 * responsible for sending a request has expired and that the SIP Stack
 * has not yet received a response to the SigComp compressed request.
 * This callback is called in the following cases:
 * --Before there is a need to retransmit a compressed message. This happens
 * when the transaction timer expires and, according to RFC 3261, the SIP
 * Stack needs to retransmit the message.
 * --When the sigCompTcpTimer expires. The SIP Stack added a timer for SigComp
 * TCP transmissions. When this timer expires, the application can send the
 * message again in a plain format. (According to RFC 3261, there are no
 * retransmission in TCP).
 * This callback is part of a recovery mechanism. It supplies information
 * about the compression type that was used in the last transmission.
 * This callback also gives the application the ability to determine if
 * there will be additional retransmissions, and the compression type of
 * the next retransmission. For example, the application can decide that
 * the next retransmission will not be compressed.
 * Note: If you set peNextMsgComp to RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED,
 * the SIP Stack will stop the message retransmissions.
 * Return Value: RvStatus. If the application returns a value other than
 *               RV_OK, no further messages will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The SIP Stack call-leg handle.
 *            hAppCallLeg    - The application handle for this call-leg.
 *            hTransc        - The transaction handle.
 *            hAppTransc     - The transaction application handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous request to which there was no response.
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipCallLegSigCompMsgNotRespondedEv)(
                     IN  RvSipCallLegHandle           hCallLeg,
                     IN  RvSipAppCallLegHandle        hAppCallLeg,
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipAppTranscHandle         hAppTransc,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * RvSipCallLegNestedInitialReqRcvdEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a nested (second or more) INVITE
 * request was received on a call-leg that is not yet connected. (The second
 * INVITE request has a different cseq value, so it was not rejected by the
 * Transaction layer according to the transaction merging rules). When this
 * callback is called, the application can decide whether or not to order
 * the SIP Stack to create a new call-leg for this nested request. If not,
 * the SIP Stack will reject the request.
 * Return Value: RvStatus. If the application returns a value other than
 *               RV_OK, the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hExistCallLeg     - The existing call-leg handle.
 *            hExistAppCallLeg  - The application handle for the existing call-leg.
 *            hRcvdMsg          - The handle to the nested INVITE message.
 * Output:    pbCreateCallLeg   - RV_TRUE will cause the creation of a new call-leg.
 *                                RV_FALSE will cause the rejection of the nested INVITE.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipCallLegNestedInitialReqRcvdEv)(
                      IN  RvSipCallLegHandle     hExistCallLeg,
                      IN  RvSipAppCallLegHandle  hExistAppCallLeg,
                      IN  RvSipMsgHandle         hRcvdMsg,
                      OUT RvBool                 *pbCreateCallLeg);

/***************************************************************************
 * RvSipCallLegNewConnInUseEv
 * ------------------------------------------------------------------------
 * General:Notifies the application that the call-leg is now using a new
 * connection. The connection can be a new connection or a suitable one
 * that was found in the hash.
 *
 * Return Value: RV_OK (The returned status is ignored in the current
 *                      SIP Stack version.)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The SIP Stack call-leg handle.
 *            hAppCallLeg  - The application handle for this call-leg.
 *            hConn          - The connection handle.
 *            bNewConnCreated - RV_TRUE if the connection is a newly
 *                              created connection. RV_FALSE if the connection
 *                              was found in the connection hash.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipCallLegNewConnInUseEv) (
                     IN  RvSipCallLegHandle             hCallLeg,
                     IN  RvSipAppCallLegHandle          hAppCallLeg,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);

/***************************************************************************
 * RvSipCallLegCreatedDueToForkingEv
 * ------------------------------------------------------------------------
 * General:  An initial INVITE request may be forked by a proxy. As a result,
 * several 1xx and 2xx responses may be received from several UASs. The first
 * incoming response will be mapped to the original call-leg that sent the
 * INVITE request. Every other incoming response (with a different
 * "to-tag" parameter), creates a new "forked call-leg".
 * This callback function notifies the application that a new call-leg was
 * created due to a forked 1xx/2xx response, and exchanges handles with
 * the application. The application may choose to terminate the new call here,
 * using the pbTerminate parameter. If so, the call-leg will be destructed
 * immediately and a callback will no longer be given on this call-leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNewCallLeg - The new SIP Stack call-leg handle.
 * Output:    phNewAppCallLeg - The application handle for this call-leg.
 *            pbTerminated - Specifies whether or not to terminate the new
 *            call-leg immediately.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegCreatedDueToForkingEv)(
                       IN  RvSipCallLegHandle           hNewCallLeg,
                       OUT RvSipAppCallLegHandle        *phNewAppCallLeg,
                       OUT RvBool                       *pbTerminate);


/***************************************************************************
 * RvSipCallLegProvisionalResponseRcvdEv
 * ------------------------------------------------------------------------
 * General:Notifies the application that a Provisional Response was received.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The SIP Stack call-leg handle
 *            hAppCallLeg       -  The application handle for this call-leg.
 *            hTransc           -  The handle to the transaction to which
 *                                 the request belongs..
 *            hAppTransc        -  The application handle to the transaction.
 *            hCallInvite       -  The handle to the CInvite object.
 *            hAppCallInvite    -  The handle to the application CInvite object.
 *            RvSipMsgHandle    -  The handle to the outgoing message.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipCallLegProvisionalResponseRcvdEv)(
                       IN  RvSipCallLegHandle            hCallLeg,
                       IN  RvSipAppCallLegHandle         hAppCallLeg,
                       IN  RvSipTranscHandle             hTransc,
                       IN  RvSipAppTranscHandle          hAppTransc,
                       IN  RvSipCallLegInviteHandle      hCallInvite,
                       IN  RvSipAppCallLegInviteHandle   hAppCallInvite,
                       IN  RvSipMsgHandle                hRcvdMsg);


/* RvSipCallLegEvHandlers
 * ------------------------------------------------------------------------
 * A structure with function pointers to the callback of the module.
 * This structure is used to set the application callbacks in the
 * RvSipCallLegMgrSetEvHandlers() function.
 *
 * pfnCallLegCreatedEvHandler    - Notifies the application of a new call.
 * pfnStateChangedEvHandler      - Notifies the application of a call-leg state change.
 * pfnMsgToSendEvHandler         - nNotifies the application that a message is about to be sent.
 * pfnMsgReceivedEvHandler       - Notifies the application that a message was received.
 * pfnModifyRequestRcvdEvHandler - Notifies the application of a modify request.
 * pfnModifyResultRcvdEvHandler  - Notifies the application of a modify request result.
 * pfnTrancRequestRcvdEvHandler  - Notifies the application of an incoming request.
 * pfnTranscResolvedEvHandler    - Notifies the application of the result of an outgoing request.
 * pfnReferStateChangedEvHandler - Notifies the application of a REFER state change.
 * pfnReferNotifyEvHandler       - Notifies the application of REFER-NOTIFY related events.
 * pfnAuthCredentialsFoundEvHandler - Notifies the application of new credentials for server authentication.
 * pfnAuthCompletedEvHandler     - Notifies the application that the server authentication procedure
 *                                 was completed (with success or failure).
 * pfnSessionTimerRefreshAlertEvHandler - Notifies the application that the alert time (the time in which
 *                                          the application needs to send a re-INVITE to refresh the call)
 *                                          is expired.
 * pfnSessionTimerNotificationEvHandler -  Notifies the application of session-timer related events.
 * pfnSessionTimerNegotiationFaultEvHandler - Notifies the application of session-timer negotiation
 *                                              fault events, such as refresher preference rejected.
 * pfnByeCreatedEvHandler      - Notifies the application of a new BYE transaction.
 * pfnByeStateChangedEvHandler - Notifies the application of a BYE state change.
 * pfnOtherURLAddressFoundEvHandler - Notifies the application that another URL address was found
 *                                      and has to be converted to the SIP URL address.
 * pfnTranscCreatedEvHandler   - Notifies the application of a new general transaction.
 * pfnTranscStateChangedEvHandler - Notifies the application of a general transaction state change.
 * pfnFinalDestResolvedEvHandler  - Notifies the application that the final destination address
 *                                 of the register request was resolved.
 * pfnSigCompMsgNotRespondedEvHandler - Notifies the application that no response was received
 *                                        to the SigComp message that was sent last.
 * pfnNestedInitialReqRcvdEvHandler - Notifies the application that a second (or more) INVITE
 *                                 was received on a call-leg that is not in state connected yet.
 *                                 application can decide how to act with this request.
 * pfnCallLegCreatedDueToForkingEvHandler - Notifies the application of a new call-leg, created
 *                                  due to a forked 1xx/2xx response on INVITE.
 * pfnReInviteCreatedEvHandler - Notifies the application of a new INVITE object, caused by a new
 *                              INVITE transaction.
 * pfnProvisionalResponseRcvdEvHandler - Notifies the application of a provisional response event.
 */
typedef struct
{
    RvSipCallLegCreatedEv                       pfnCallLegCreatedEvHandler;
    RvSipCallLegStateChangedEv                  pfnStateChangedEvHandler;
    RvSipCallLegMsgToSendEv                     pfnMsgToSendEvHandler;
    RvSipCallLegMsgReceivedEv                   pfnMsgReceivedEvHandler;
    RvSipCallLegModifyStateChangedEv            pfnModifyStateChangedEvHandler;
    RvSipCallLegModifyRequestRcvdEv             pfnModifyRequestRcvdEvHandler;
    RvSipCallLegModifyResultRcvdEv              pfnModifyResultRcvdEvHandler;
    RvSipCallLegTrancRequestRcvdEv              pfnTranscRequestRcvdEvHandler;
    RvSipCallLegTranscResolvedEv                pfnTranscResolvedEvHandler;
    RvSipCallLegReferStateChangedEv             pfnReferStateChangedEvHandler;
    RvSipCallLegReferNotifyEv                   pfnReferNotifyEvHandler;
    RvSipCallLegPrackStateChangedEv             pfnPrackStateChangedEvEvHandler;
    RvSipCallLegReferCancelledEv                pfnReferCancelledEvHandler;
#ifdef RV_SIP_AUTH_ON
    RvSipCallLegAuthCredentialsFoundEv          pfnAuthCredentialsFoundEvHandler;
    RvSipCallLegAuthCompletedEv                 pfnAuthCompletedEvHandler;
#endif /* #ifdef RV_SIP_AUTH_ON */

    /* Session Timer call-backs */
    RvSipCallLegSessionTimerRefreshAlertEv      pfnSessionTimerRefreshAlertEvHandler;
    RvSipCallLegSessionTimerNotificationEv      pfnSessionTimerNotificationEvHandler;
    RvSipCallLegSessionTimerNegotiationFaultEv  pfnSessionTimerNegotiationFaultEvHandler;
    /* BYE callbacks */
    RvSipCallLegByeCreatedEv                    pfnByeCreatedEvHandler;
    RvSipCallLegByeStateChangedEv               pfnByeStateChangedEvHandler;
    RvSipCallLegTranscCreatedEv                 pfnTranscCreatedEvHandler;
    RvSipCallLegTranscStateChangedEv            pfnTranscStateChangedEvHandler;
    RvSipCallLegOtherURLAddressFoundEv          pfnOtherURLAddressFoundEvHandler;
    RvSipCallLegFinalDestResolvedEv             pfnFinalDestResolvedEvHandler;
#ifdef RV_SIGCOMP_ON
    RvSipCallLegSigCompMsgNotRespondedEv        pfnSigCompMsgNotRespondedEvHandler;
#endif
    RvSipCallLegNestedInitialReqRcvdEv          pfnNestedInitialReqRcvdEvHandler;
    RvSipCallLegNewConnInUseEv                  pfnNewConnInUseEvHandler;
    RvSipCallLegCreatedDueToForkingEv           pfnCallLegCreatedDueToForkingEvHandler;
    RvSipCallLegReInviteCreatedEv               pfnReInviteCreatedEvHandler;
    RvSipCallLegReInviteStateChangedEv          pfnReInviteStateChangedEvHandler;
    RvSipCallLegProvisionalResponseRcvdEv       pfnProvisionalResponseRcvdEvHandler;
} RvSipCallLegEvHandlers;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_CALL_LEG_TYPES_H */
