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
 *                              <CallLegCallbacks.h>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/

#ifndef CALL_LEG_CALLBACKS_H
#define CALL_LEG_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegObject.h"
#include "RvSipCallLegTypes.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the call-leg manager.
*         hCallLeg    - The new sip stack call-leg handle
* Output:(-)
***************************************************************************/
void RVCALLCONV CallLegCallbackCreatedEv(IN  CallLegMgr                *pMgr,
                                         IN  RvSipCallLegHandle        hCallLeg);


/***************************************************************************
 * CallLegCallbackChangeCallStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg state and notify the application about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeCallStateEv(
                                 IN  CallLeg                      *pCallLeg,
                                 IN  RvSipCallLegState             eState,
                                 IN  RvSipCallLegStateChangeReason eReason);

/*-----------------------------------------------------------------------*/
/*                           MODIFY CALLBACKS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackReInviteCreatedEv
 * ------------------------------------------------------------------------
 * General: Notify that a new re-invite object was created.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            pInvite  - The new invite object.
 * Output:    pResponseCode - application may decide how to reject the re-invite request.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackReInviteCreatedEv(
                                 IN  CallLeg        *pCallLeg,
                                 IN  CallLegInvite  *pInvite,
                                 OUT RvUint16        *pResponseCode);

/***************************************************************************
 * CallLegCallbackChangeReInviteStateEv
 * ------------------------------------------------------------------------
 * General: Changes the re-invite state of an invite object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eSubState - The Modify state.
 *          eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeReInviteStateEv(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  CallLegInvite                 *pInvite,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason);
/***************************************************************************
 * CallLegCallbackChangeModifyStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg connect sub state.
 *          The connect sub state is used for incoming and outgoing re-Invite
 *          messages.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eSubState - The Modify state.
 *          eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeModifyStateEv(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason);

/***************************************************************************
 * CallLegCallbackModifyRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnModifyRequestRcvdEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackModifyRequestRcvdEv(IN  CallLeg *pCallLeg);

/***************************************************************************
 * CallLegCallbackModifyResultRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnModifyResultRcvdEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackModifyResultRcvdEv(IN  CallLeg  *pCallLeg,
                                                  IN  RvUint16 responseStatusCode);

/*-----------------------------------------------------------------------*/
/*                           CALL TRANSACTION CALLBACKS                  */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackTranscCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegTranscCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg         - Handle to the call-leg.
*         hTransc    -  Handle to the transaction.
* Output:hAppTransc - Application handle to this transaction
*        bAppHandleTransc -
***************************************************************************/
void RVCALLCONV CallLegCallbackTranscCreatedEv(IN  RvSipCallLegHandle    hCallLeg,
                                               IN  RvSipTranscHandle     hTransc,
                                               OUT RvSipAppTranscHandle *hAppTransc,
                                               OUT RvBool               *bAppHandleTransc);

/***************************************************************************
 * CallLegCallbackTranscStateChangedEv
 * ------------------------------------------------------------------------
 * General: Call the pfnTranscStateChangedEvHandler callback.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The sip stack call-leg handle
 *            hTransc -     The handle to the transaction.
 *          eTranscState - The call-leg transaction new state.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
void RVCALLCONV CallLegCallbackTranscStateChangedEv(
                           IN  RvSipCallLegHandle                hCallLeg,
                           IN  RvSipTranscHandle                 hTransc,
                           IN  RvSipCallLegTranscState           eTranscState,
                           IN  RvSipTransactionStateChangeReason eReason);

/***************************************************************************
 * CallLegCallbackTranscRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: Notify the application of a new general transaction.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - the call-leg pointer
 *          hTransc     - the transaction handle
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackTranscRequestRcvdEv(
                                            IN  CallLeg           *pCallLeg,
                                            IN  RvSipTranscHandle  hTransc);


/***************************************************************************
* CallLegCallbackTranscResolvedEv
* ------------------------------------------------------------------------
* General: Call the pfnTranscResolvedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input:    hCallLeg    - The sip stack call-leg handle
*            hTransc     - The handle to the transaction that received the
*                         response
*           eStatus     - The resolution status. if the status is
*                         RVSIP_CALL_TRANSC_STATUS_RESPONSE_RECVD the
*                         responseCode parameter will contain the response
*                         message status code.
*            responseCode- The response status code.
***************************************************************************/
void RVCALLCONV CallLegCallbackTranscResolvedEv(
                       IN  RvSipCallLegHandle               hCallLeg,
                       IN  RvSipTranscHandle                hTransc,
                       IN  RvSipCallLegTranscStatus         eStatus,
                       IN  RvUint16                        responseCode);

/*-----------------------------------------------------------------------*/
/*                           MESSAGE CALLBACKS                           */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackMsgReceivedEv
* ------------------------------------------------------------------------
* General: Call the pfnMsgReceivedEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackMsgReceivedEv(IN  RvSipCallLegHandle      hCallLeg,
                                                  IN  RvSipMsgHandle          hMsg);

/***************************************************************************
* CallLegCallbackMsgToSendEv
* ------------------------------------------------------------------------
* General: Call the pfnMsgToSendEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackMsgToSendEv(IN  RvSipCallLegHandle      hCallLeg,
                                                IN  RvSipMsgHandle          hMsg);

/*-----------------------------------------------------------------------*/
/*                           PRACK CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackPrackStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notify the application of a new prack state
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg.
 *            ePrackState       - The new prack state
 *            prackResponseCode - The response code that was received on a prack.
 *            hPrackTransc      - The handle for the prack transaction.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackPrackStateChangedEv(
                      IN  CallLeg                        *pCallLeg,
                      IN  RvSipCallLegPrackState         ePrackState,
                      IN  RvSipCallLegStateChangeReason  eReason,
                      IN  RvInt16                       prackResponseCode,
					  IN  RvSipTranscHandle             hPrackTransc);


/*-----------------------------------------------------------------------*/
/*                           BYE CALLBACKS                               */
/*-----------------------------------------------------------------------*/


/***************************************************************************
* CallLegCallbackByeCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnByeCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg         - Handle to the call-leg.
*         hTransc    -  Handle to the bye transaction.
* Output:hAppTransc - Application handle to this transaction
*        bAppHandleTransc - indicates wh
***************************************************************************/
void RVCALLCONV CallLegCallbackByeCreatedEv(IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc,
                                           OUT RvSipAppTranscHandle *hAppTransc,
                                           OUT RvBool              *bAppHandleTransc);

/***************************************************************************
 * CallLegCallbackByeStateChangedEv
 * ------------------------------------------------------------------------
 * General: Call the pfnByeStateChangedEvHandler callback.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The sip stack call-leg handle
 *            hTransc -     The handle to the transaction.
 *          eTranscState - The call-leg bye transaction new state.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
void RVCALLCONV CallLegCallbackByeStateChangedEv(
                           IN  RvSipCallLegHandle                hCallLeg,
                           IN  RvSipTranscHandle                 hTransc,
                           IN  RvSipCallLegByeState              eByeState,
                           IN  RvSipTransactionStateChangeReason     eReason);



/*-----------------------------------------------------------------------*/
/*                           SESSION TIMER CALLBACKS                     */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackSessionTimerRefreshAlertEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerRefreshAlertEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerRefreshAlertEv(
                            IN  RvSipCallLegHandle      hCallLeg);

/***************************************************************************
* CallLegCallbackSessionTimerNotificationEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerNotificationEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
 *       eReason     - The reason for this notify .
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerNotificationEv(
                IN  RvSipCallLegHandle                        hCallLeg,
                IN RvSipCallLegSessionTimerNotificationReason eReason);

/***************************************************************************
* CallLegCallbackSessionTimerNegotiationFaultEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerNegotiationFaultEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*        eReason     - The reason for this negotiation fault .
* Output:b_handleSessionTimer - RV_TRUE if the application wishes to operate
*                               the session time of this call.
*                               RV_FALSE if the application wishes not to
*                               operate the session timer to this call.
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerNegotiationFaultEv(
           IN  RvSipCallLegHandle                         hCallLeg,
           IN  RvSipCallLegSessionTimerNegotiationReason  eReason,
           OUT RvBool                                    *b_handleSessionTimer);


#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                           REFER CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackChangeReferStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg refer sub state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new refer sub state.
 *            eReason  - The refer sub state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeReferStateEv(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferState                eState,
                               IN  RvSipCallLegStateChangeReason         eReason);

/***************************************************************************
 * CallLegCallbackReferNotifyEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg notify state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new notify sub state.
 *            eReason  - The state change reason.
 *          status - Status of the connection attempt.
 *          cseq - The CSeq step of the associated REFER transaction.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackReferNotifyEv(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferNotifyEvents         eState,
                               IN  RvSipCallLegStateChangeReason         eReason,
                               IN  RvInt16                              status,
                               IN  RvInt32                              cseq);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                           AUTHENTICATION CALLBACKS                    */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  Calls to pfnAuthCredentialsFoundEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The sip stack transaction handle
 *          hCallLeg  - CallLeg handle for the transaction
 *          hAuthorization - Handle to the authorization header that was found.
 *          bCredentialsSupported - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackAuthCredentialsFoundEv(
                        IN    RvSipTranscHandle               hTransc,
                        IN    RvSipCallLegHandle              hCallLeg,
                        IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                        IN    RvBool                         bCredentialsSupported);

/***************************************************************************
* CallLegCallbackAuthCompletedEv
* ------------------------------------------------------------------------
* General:  call to pfnAuthCompletedEvHandler.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc      - The sip stack transaction handle
*          hCallLeg     - CallLeg handle for the transaction
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
***************************************************************************/
void RVCALLCONV CallLegCallbackAuthCompletedEv(
                         IN    RvSipTranscHandle        hTransc,
                         IN    RvSipCallLegHandle       hCallLeg,
                         IN    RvBool                  bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * CallLegCallbackTranscOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: calls to pfnOtherURLAddressFoundEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hCallLeg       - CallLeg handle for the transaction
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackTranscOtherURLAddressFoundEv(
                     IN  CallLeg                *pCallLeg,
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved);


/***************************************************************************
 * CallLegCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - The CallLeg handle
 *            hTransc       - The transaction handle
 *            hMsg          - The message that is about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackFinalDestResolvedEv(
                     IN  CallLeg               *pCallLeg,
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipMsgHandle         hMsg);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * CallLegCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - The CallLeg handle
 *            hTransc        - The transaction handle
 *            hAppTransc     - The transaction application handle.
 *            retransNo      - The number of retransmissions of the request
 *                             until now.
 *            ePrevMsgComp   - The type of the previous not responded request
 * Output:    peNextMsgComp  - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSigCompMsgNotRespondedEv(
                         IN  CallLeg                     *pCallLeg,
                         IN  RvSipTranscHandle            hTransc,
                         IN  RvSipAppTranscHandle         hAppTransc,
                         IN  RvInt32                      retransNo,
                         IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                         OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * CallLegCallbackNestedInitialReqRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSecInviteRcvdEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - The CallLeg handle
 *            hMsg          - The handle to the second INVITE message.
 * Output:    pbCreateCallLeg - RV_TRUE will cause the creation of a new call-leg.
 *                            RV_FALSE will cause the rejection of the second INIVTE.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackNestedInitialReqRcvdEv(
                     IN  CallLeg               *pCallLeg,
                     IN  RvSipMsgHandle         hMsg,
                     OUT RvBool                *pbCreateNewCall);


/***************************************************************************
 * CallLegCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - The CallLeg pointer
 *            hConn        - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackNewConnInUseEv(
                     IN  CallLeg*                     pCallLeg,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
* CallLegCallbackCreatedDueToForkingEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegCreatedDueToForkingEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the call-leg manager.
*         hCallLeg    - The new sip stack call-leg handle
* Output:pbTerminated
***************************************************************************/
void RVCALLCONV CallLegCallbackCreatedDueToForkingEv(
                                         IN  CallLegMgr                *pMgr,
                                         IN  RvSipCallLegHandle        hCallLeg,
                                         OUT RvBool                    *pbTerminated);

/***************************************************************************
* CallLegCallbackProvisionalResponseRcvdEv
* ------------------------------------------------------------------------
* General: Call the pfnProvisionalResponseRcvdEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
void RVCALLCONV CallLegCallbackProvisionalResponseRcvdEv(
                                                             IN  CallLeg*          pCallLeg,
                                                             IN  RvSipTranscHandle hTransc,
                                                             IN  CallLegInvite*    pCallInvite,
                                                             IN  RvSipMsgHandle    hMsg);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_CALLBACKS_H*/



