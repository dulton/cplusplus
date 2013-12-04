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
 *                              <TransactionTimer.h>
 *
 *  This file defines the behavior of a transaction object when it's timer expires.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/



#ifndef TRANSACTION_TIMER_H
#define TRANSACTION_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT
#include "TransactionObject.h"


/***************************************************************************
 * TransactionTimerRetransmit
 * ------------------------------------------------------------------------
 * General: Called when a retransmission timer expires.
 *          If the number of retransmissions sent is smaller than the number
 *            of retransmissions that are supposed to be sent the last message
 *          is retransmited, and the retransmission
 *            timer is set according to the next retransmission round.
 *            If the number of retransmission sent matches the number of
 *            retransmissions that were supposed to be sent the tansaction will
 *            be terminated.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to retransmit from.
 *          maxRetransmissions - The number of retransmissions supposed to be
 *                                 sent by this transaction according to state.
 *                             If maxRetransmissions is equal to -1 then there is no
 *                             limitation on the retransmissions number.
 *        enableTerminateTransaction - enables/disables transaction termination
 ***************************************************************************/
void RVCALLCONV TransactionTimerRetransmit(
                                 IN Transaction *pTransc,
                                 IN RvInt8     maxRetransmissions,
                                 IN RvBool        enableTerminateTransaction);


/***************************************************************************
 * TransactionTimerHandleMainTimerTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever the main transaction timer expires.
 *          If this is a retransmission    timer call the sipTransactionRetransmit function.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The transaction this timer was called for.
 ***************************************************************************/
RvBool TransactionTimerHandleMainTimerTimeout(IN void  *pContext,
                                              IN RvTimer *timerInfo);


/***************************************************************************
 * TransactionTimerMainTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the transaction's timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV TransactionTimerMainTimerRelease(IN Transaction *pTransc);

/***************************************************************************
 * TransactionTimerMainTimerStart
 * ------------------------------------------------------------------------
 * General: Starts the transaction's main timer.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to start the timer from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTimerMainTimerStart(IN Transaction *pTransc,
                                                   IN RvUint32     timerInterval,
                                                   IN RvChar*     functionName);
/***************************************************************************
 * TransactionTimerSetTimers
 * ------------------------------------------------------------------------
 * General:
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc   - the transaction handle
 *            msgType   - the type of message that was sent
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTimerSetTimers(
                                   IN  Transaction*           pTransc,
                                   IN  SipTransportMsgType    msgType);

/***************************************************************************
 * TransactionTimerReleaseAllTimers
 * ------------------------------------------------------------------------
 * General: Release both timers of the transactions. (generalRequestTimeoutTimer
 *          and the interval timer).
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV TransactionTimerReleaseAllTimers(IN Transaction *pTransc);


/***************************************************************************
 * TransactionTimerHandleRequestLongTimerTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever the request long timer expires.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The handle to the timer that has expired.
 *          pContext - The transaction this timer was called for.
 ***************************************************************************/
RvBool TransactionTimerHandleRequestLongTimerTimeout(
                                                     IN void    *pContext,
                                                     IN RvTimer *timerInfo);


#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSACTION_TIMER_H */
