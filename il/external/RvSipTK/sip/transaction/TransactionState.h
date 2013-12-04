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
 *                              <TransactionState.h>
 *
 *    This file defines one function to each of the transactions states. This
 *    function is called when the state changes. The function will change the
 *    transaction's state, call EvStateChanged callback, and set the timer according
 *  to the new state.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef TRANSACTION_STATE_H
#define TRANSACTION_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RV_SIP_LIGHT

#include "_SipTransactionTypes.h"
#include "TransactionObject.h"


/***************************************************************************
 * TransactionStateSrvGeneralReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 ***************************************************************************/
    RvStatus RVCALLCONV TransactionStateSrvGeneralReqRecvd(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateSrvGeneralFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 ***************************************************************************/
    RvStatus RVCALLCONV TransactionStateSrvGeneralFinalRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateSrvInviteReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
    RvStatus RVCALLCONV TransactionStateSrvInviteReqRecvd(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateSrvInviteFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInviteFinalRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateProxyInvite2xxRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer
 *            according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateProxyInvite2xxRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateSrvInviteReliableProvRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInviteReliableProvRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateUacInviteCancelling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteCancelling(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateUacGeneralCancelling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralCancelling(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);



/***************************************************************************
 * TransactionStateUacGeneralReqMsgSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output: (-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralReqMsgSent(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc);

/***************************************************************************
 * TransactionStateUacCancelReqMsgSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *Output: (-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelReqMsgSent(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateUacGeneralProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralProceeding(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc);
/***************************************************************************
 * TransactionStateUacGeneralFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralFinalResponseRcvd(
                    IN  RvSipTransactionStateChangeReason eReason,
                    IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateUacCancelFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelFinalResponseRcvd(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateUacCancelProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelProceeding(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc);

/***************************************************************************
 * TransactionStateUacInviteCalling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output: (-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteCalling(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateUacInviteProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteProceeding(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateUacInviteFinalRespRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteFinalRespRecvd(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateProxyInvite2xxRespRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer according
 *            to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:V
***************************************************************************/
RvStatus RVCALLCONV TransactionStateProxyInvite2xxRespRcvd(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateUacInviteAckSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteAckSent(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc);

/***************************************************************************
 * TransactionStateServerInviteAckRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateServerInviteAckRcvd(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc);


/***************************************************************************
 * TransactionStateSrvCancelReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *        pCancelledTransc - The transaction to cancel.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvCancelReqRecvd(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc,
                           IN  Transaction                       *pCancelledTransc);


/***************************************************************************
 * TransactionStateServerCancelFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateServerCancelFinalRespSent(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc);



/***************************************************************************
 * TransactionStateUACInviteProceedingTimeOut
 * ------------------------------------------------------------------------
 * General: This function is called when the UAC Invite transaction reached timeout
 *          in the proceeding state.
 *          This function is called only if the flag enableInviteProceedingTimeoutState is TRUE.
 * Return Value: RV_OK - The transaction was terminated successfully.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - the transaction that reached request timeout
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUACInviteProceedingTimeOut(IN Transaction *pTransc);

/***************************************************************************
 * TransactionStateSrvPrackReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *        hPrackMsg - Handle to the PRACK message.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvPrackReqRecvd(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc,
                            IN  RvSipMsgHandle                    hPrackMsg);

/***************************************************************************
 * TransactionStateSrvInvitePrackCompeleted
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInvitePrackCompeleted(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc);

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSACTION_STATE_H */
