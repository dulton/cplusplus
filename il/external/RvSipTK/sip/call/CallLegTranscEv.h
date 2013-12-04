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
 *                              <sipCallLegTranscEv.h>
 *
 *  Handles state changed events received from the transaction layer.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

#ifndef CALL_LEG_TRANSC_EV_H
#define CALL_LEG_TRANSC_EV_H

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
#include "CallLegObject.h"



/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * HandleInviteProvisionalResponseRecvd
 * ------------------------------------------------------------------------
 * General: Handles receipt of a provisional response to previously sent
 *          INVITE request.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - A pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV HandleInviteProvisionalResponseRecvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite);

/***************************************************************************
 * HandleRelProvRespRcvd
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV HandleRelProvRespRcvd (
                      IN    CallLeg           *pCallLeg,
                      IN    RvUint32           rseqResponseNum,
                      IN    RvSipMsgHandle     hMsg);

/***************************************************************************
* CallLegTranscEvHandleInviteFinalResponseRcvd
* ------------------------------------------------------------------------
* General: This function is called when a transaction receives a final
*          response for an Invite request, or the reInvite reqest.
*          This function call two sub functions to handle the two
*          cases.
* Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
*                            state.
*               RV_OK - on success.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - The call-leg that received the transaction event.
*          hTransc - Handle to the related transaction.
*            eStateReason - the reason for the final response.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvHandleInviteFinalResponseRcvd(
                                 IN  CallLeg                           *pCallLeg,
                                 IN  CallLegInvite*                    pInvite,
                                 IN  RvSipTranscHandle                 hTransc,
                                 IN  RvSipTransactionStateChangeReason eStateReason);

/***************************************************************************
 * CallLegTranscEvCancelled
 * ------------------------------------------------------------------------
 * General: Called when a  a cancel request is received on a transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc:      - A pointer to new transaction.
 *          hTranscOwner: - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvCancelled(
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg);


/***************************************************************************
 * CallLegTranscEvRelProvRespRcvdEv
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received. The
 *             callback is called only if the manualPrack flag is set to RV_TRUE.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc:      - A pointer to new transaction.
 *          hTranscOwner: - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvRelProvRespRcvdEv (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    RvUint32                 rseqResponseNum);

/***************************************************************************
 * CallLegTranscEvIgnoreRelProvRespEvHandler
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvIgnoreRelProvRespEvHandler (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    RvUint32                 rseqStep,
                      OUT   RvBool                   *pbIgnore);

/***************************************************************************
 * CallLegTranscEvSupplyCSeqStep
 * ------------------------------------------------------------------------
 * General: Supply the transaction with the next Call-Leg CSeq step.
 *          and the request uri calculated according to contact and
 *          Route list.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The new sip stack transaction handle
 *          hAppTransc  - Application handle for the transaction
 * Output:  pCSeqStep - CSeq step for the transaction
 *          phRequestUri - Request URI for the transaction.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvSupplyTranscParamsEv(
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hAppTransc,
                      OUT   RvInt32                 *pCSeqStep,
                      OUT   RvSipAddressHandle       *phRequestUri);

/***************************************************************************
 * CallLegTranscEvStateChangedHandler
 * ------------------------------------------------------------------------
 * General: Handle a transaction state change.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg this transaction belongs to.
 *          hTransc - Handle to the transaction whos state was changed.
 *            eTranscState - The new state of the transaction
 *            eStateReason - The reason for the change of state.
 ***************************************************************************/

void RVCALLCONV  CallLegTranscEvStateChangedHandler(
                        IN    RvSipTranscHandle                   hTransc,
                        IN  RvSipTranscOwnerHandle              hCallLeg,
                        IN  RvSipTransactionState               eTranscState,
                        IN  RvSipTransactionStateChangeReason   eStateReason);


#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * CallLegTranscEvAuthCredentialsFound
 * ------------------------------------------------------------------------
 * General:  Supply an authorization header, to pass it to the user,
 *           that will continue the authentication procedure, according to
 *           the realm and username parameters in it.
 *           in order to coninue the procedure, user shall use AuthenticateHeader()
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The sip stack transaction handle
 *          hCallLeg  - CallLeg handle for the transaction
 *          hAuthorization - Handle to the authorization header that was found.
 *          bCredentialsSupported - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvAuthCredentialsFound(
                      IN    RvSipTranscHandle               hTransc,
                      IN    RvSipTranscOwnerHandle          hCallLeg,
                      IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                      IN    RvBool                         bCredentialsSupported);

/***************************************************************************
 * CallLegTranscEvAuthCompleted
 * ------------------------------------------------------------------------
 * General:  Notify that authentication procedure is completed.
 *           If it is completed because a correct authorization was found,
 *           bAuthSucceed is RV_TRUE.
 *           If it is completed because there are no more authorization headers
 *           to check, or because user ordered to stop the searching for
 *           correct header, bAuthSucceed is RV_FALSE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The sip stack transaction handle
 *          hCallLeg     - CallLeg handle for the transaction
 *          bAuthSucceed - RV_TRUE if we found correct authorization header,
 *                        RV_FALSE if we did not.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvAuthCompleted(
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    RvBool                  bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * CallLegTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
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
RvStatus RVCALLCONV CallLegTranscEvOtherURLAddressFound(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hCallLeg,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved);


/***************************************************************************
 * CallLegTranscFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *          When this callback is called the application can query the
 *          transaction about the destination address using the
 *          RvSipTransactionGetCurrentDestAddr() API function.
 *          If the application wishes it can update the sent-by part of
 *          the top most via. The application must not update the branch parameter.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hCallLeg     - The call-leg handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscFinalDestResolvedEv(
                            IN RvSipTranscHandle       hTransc,
                             IN RvSipTranscOwnerHandle  hCallLeg,
                            IN RvSipMsgHandle          hMsgToSend);

/***************************************************************************
 * CallLegTranscNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the call-leg transc is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.

 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hCallLeg       - CallLeg handle for the transaction
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscNewConnInUseEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hCallLeg,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * CallLegTranscSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp compressed request
 *          was sent over the transaction and a SigComp timer expired
 *          before receiving any response. This callback is part of a
 *          recovery mechanism and it supplies information about the previous
 *          sent request (that wasn't responded yet). The callback also
 *          gives the application the ability to determine if there will be
 *          additional sent request and what is the type of the next sent
 *          request.
 *          NOTE: A RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED value of the out
 *          parameter is referred as a request from the application to block
 *          the sending of additional requests in the current transaction
 *          session.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK no further message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hCallLeg      - The call-leg handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscSigCompMsgNotRespondedEv(
                        IN  RvSipTranscHandle            hTransc,
                        IN  RvSipTranscOwnerHandle       hCallLeg,
                        IN  RvInt32                      retransNo,
                        IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                        OUT RvSipTransmitterMsgCompType *peNextMsgComp);

#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * CallLegTranscEvInviteResponseNoTranscRcvdEvHandler
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new 1xx/2xx response
 *          for INVITE, and the response is not related to any transaction.
 *          The callback will be called in the following cases:
 *          1. The INVITE request was forked by a forking proxy
 *          to several UAS, and each one of them returns 1xx and 2xx responses.
 *          2. For 2xx retransmissions since the first 2xx terminates the
 *          transaction.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLegMgr
 *          hReceivedMsg - The received 1xx/2xx response.
 *          pRcvdFromAddr - The address that the response was received from
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvInviteResponseNoTranscRcvdEvHandler (
                      IN    void*					hCallLegMgr,
                      IN    RvSipMsgHandle			hReceivedMsg,
                      IN    SipTransportAddress*    pRcvdFromAddr,
                      OUT   RvBool*					pbWasHandled);

/***************************************************************************
 * CallLegTranscEvAckNoTranscEvHandler
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives an ACK request
 *          that does not match any transaction. This will be the case
 *          when the INVITE was responded with 2xx response and was terminated
 *          immediately after that.
 * Return Value: RV_Status - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hAckMsg       - The handle to the received ACK message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvAckNoTranscEvHandler(
                     IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hAckMsg,
                     IN   SipTransportAddress*    pRcvdFromAddr,
                     OUT  RvBool*                 pbWasHandled);

/***************************************************************************
 * CallLegTranscEvInviteTranscMsgSent
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that the invite transaction sent a msg.
 *          1. On first 2xx sending, when working with new-invite handling. 
 *          2. On non-2xx Ack sending, when working with new-invite handling.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvInviteTranscMsgSent (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    SipTransactionMsgSentOption eOption);


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_TRANSC_EV_H */


