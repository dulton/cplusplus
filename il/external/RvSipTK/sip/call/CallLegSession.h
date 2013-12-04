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
 *                              <sipCallLegSession.h>
 *
 *  Handles all session request initiated by the user such as
 *  connecting a call-leg, accepting or rejecting a call-leg, disconnecting
 *  call-leg and more.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef CALL_LEG_SESSION_H
#define CALL_LEG_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegObject.h"
#include "_SipTransaction.h"



/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegConnect
 * ------------------------------------------------------------------------
 * General: Initiate an outgoing call. This method may be called
 *          only after the To, From fields have been set. Calling
 *          Connect causes an INVITE to be sent out and the
 *          call-leg state machine progresses to the Inviting state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Cannot connect due to a resource problem.
 *               RV_ERROR_UNKNOWN - An error occurred. Invite message was not sent.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to connect
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionConnect (IN  CallLeg *pCallLeg);




/***************************************************************************
 * RvSipCallLegAccept
 * ------------------------------------------------------------------------
 * General: This method may called by the application to indicate that it
 *          is willing to accept an incoming call or an incoming re-Invite.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN - Failed to accept the call. (usually a failure
 *                            while trying to send the 200 response.
 *               RV_OK - Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to accept
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionAccept (IN  CallLeg *pCallLeg);


/***************************************************************************
 * CallLegSessionByeAccept
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is willing to accept an incoming BYE request.
 *          If the funciton failes the call-leg will be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Failed to accept the call.
 *               RV_OK - 200 final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to accept its BYE
 *          hTransc  - handle to the BYE transaction
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionByeAccept (IN  CallLeg          *pCallLeg,
                                              IN  RvSipTranscHandle hTransc);


/***************************************************************************
 * CallLegSessionReject
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is rejecting an incoming call. This method is valid only
 *          in the offering state or the connected/modified.
 *          If the funciton failes the call-leg will be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send reject response.
 *               RV_OK - Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to reject
 *            status - The rejection response code.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionReject (
                                           IN  CallLeg   *pCallLeg,
                                           IN  RvUint16 status);

/***************************************************************************
 * CallLegSessionByeReject
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is rejecting an incoming BYE request
 * Return Value: RV_ERROR_UNKNOWN - Failed to send reject response.
 *               RV_OK - Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to reject its BYE
 *          hTransc  - Handle to the BYE transaction
 *            status   - The rejection response code.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionByeReject (
                                         IN  CallLeg          *pCallLeg,
                                         IN  RvSipTranscHandle hTransc,
                                         IN  RvUint16         status);


/***************************************************************************
 * CallLegSessionProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Sends a provisional response (1xx) to the remote party.
 *          This function can be called when ever a request is received for
 *          example in the offering state.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg.
 *            status - The provisional response code.
 *          b_isReliable - is the provisional response reliable
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionProvisionalResponse (
                                         IN  CallLeg   *pCallLeg,
                                         IN  RvUint16 status,
                                         IN  RvBool   b_isReliable);


/***************************************************************************
 * CallLegSessionSendPrack
 * ------------------------------------------------------------------------
 * General: Sends a manual PRACK message to a reliable provisional
 *            response.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to ack.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendPrack(IN  CallLeg   *pCallLeg);

/***************************************************************************
 * CallLegSessionSendPrackResponse
 * ------------------------------------------------------------------------
 * General: Sends a manual response to PRACK message.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - Pointer to the call leg the user wishes to ack.
 *            responseCode - The response code to send.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendPrackResponse(IN  CallLeg   *pCallLeg,
                                                     IN  RvUint16  responseCode);

/***************************************************************************
 * CallLegSessionAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request from the call-leg to the remote party.
 *          This function can be called in the remote accepted state
 *          for initial invite or in the ModifyResultRcvd call back for
 *          a re-Invite request.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to ack.
 *            pInvite  - Pointer to the invite object.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionAck (IN  CallLeg   *pCallLeg,
                                       IN  CallLegInvite *pInvite);

/***************************************************************************
 * CallLegSeesionCreateAndModify
 * ------------------------------------------------------------------------
 * General: The function is deprecated. 
 *          You should use RvSipCallLegCreateReInvite() and RvSipCallLegReInvite().
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSeesionCreateAndModify (IN  CallLeg* pCallLeg);

/***************************************************************************
 * CallLegSessionModify
 * ------------------------------------------------------------------------
 * General: Causes a re-INVITE to be sent. Can be called only in the connected
 *          state when there is no other pending re-Invite transaction.
 *          The remote-party's response to the re-INVITE will be given
 *          in the EvModifyResultRecvd callback.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to modify.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionModify (IN  CallLeg   *pCallLeg,
                                          IN  CallLegInvite* pInvite);


/***************************************************************************
 * CallLegSessionDisconnect
 * ------------------------------------------------------------------------
 * General:Disconnect causes the call to disconnect. Disconnect may
 *         be called at any state. The behavior of the function depends on
 *         the call-leg state:
 *         Connected,Accepted,Inviting - Bye is send and the call moves
 *                                         the Disconnecting.
 *         Offering  - The incoming invite is rejected with status code 603.
 *         Idle,Disconnecting,Disconnected - The call is terminated.
 *         If the functions failes to send the BYE request the call-leg will
 *         be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send bye message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionDisconnect(IN  CallLeg   *pCallLeg);



/***************************************************************************
 * CallLegSessionSendBye
 * ------------------------------------------------------------------------
 * General:Bye is send and the call moves the Disconnecting state.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send bye message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendBye(IN  CallLeg   *pCallLeg);

/***************************************************************************
 * CallLegSessionRequest
 * ------------------------------------------------------------------------
 * General:Sends a general reqest related to this call-leg.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send request message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - request message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 *          strMethod - The request method
 *          hTransc - The transaction handle to use for the request. if NULL
 *                     create a new transaction.
 * Output:  hTransc - The handle of the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionRequest(
                                           IN  CallLeg*             pCallLeg,
                                           IN  RvChar*             strMethod,
                                           OUT RvSipTranscHandle    *hTransc);

/***************************************************************************
 * CallLegSessionRequestRefresh
 * ------------------------------------------------------------------------
 * General:Creates a transaction related to the call-leg and sends a
 *          Request message with the given method in order to refresh the call.
 *          The request will have the To, From and Call-ID of the call-leg and
 *          will be sent with a corrent CSeq step. It will be record routed if needed.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send cancel message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - CANCEL message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 *          sessionExpires - session time that will attach to this call.
 *          minSE - minimum session expires time of this call
 *          eRefresher - the refresher preference for this call
 * Output:  hTransc - The handle to the newly created transaction.
 *                    if a transaction was supplied, this transaction will be used,
 *                    and a new transaction will not be created
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionRequestRefresh(
    IN  CallLeg*                                       pCallLeg,
    IN  RvChar*                                       strMethod,
    IN  RvInt32                                       sessionExpires,
    IN  RvInt32                                       minSE,
    IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher,
    INOUT RvSipTranscHandle                              *hTransc);
/***************************************************************************
 * CallLegSessionCancel
 * ------------------------------------------------------------------------
 * General:Cancels an Invite active transaction.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send cancel message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - CANCEL message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionCancel(IN  CallLeg   *pCallLeg);

#endif  /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_SESSION_H */

