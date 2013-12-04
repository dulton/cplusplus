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
 *                              <TransactionMsgReceived.h>
 *
 *    This file defines the transaction behavior on the receipt of a message.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef TRANSACTION_MSG_RECEIVED_H
#define TRANSACTION_MSG_RECEIVED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"

#ifndef RV_SIP_LIGHT

#include "TransactionObject.h"
#include "RvSipMsg.h"




/***************************************************************************
 * TransactionMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a message. The transaction
 *            will behave according to the state it is in, and to the message
 *            received (request, response...).
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *               RV_ERROR_BADPARAM - The message is not a legal request or
 *                                       response.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the message.
 *          pMsg - The message received.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMsgReceived(IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionMsgReceivedNotifyOthers
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a message to notify other
 *          entities who might be interested:
 *			1. We call MsgReceivedEv to notify the owner of the transaction 
 *             on the receipt of a message
 *          2. We notify the security-agreement layer on the receipt of a message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the message.
 *        pMsg    - The message received.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMsgReceivedNotifyOthers(
											IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionMsgIs1xxLegalRel
 * ------------------------------------------------------------------------
 * General: Checks if a given 1xx message is a reliable provisional response,
 *          and if has all mandatory parameters.
 * Return Value: RV_TRUE - reliable and legal.
 *               RV_FALSE - not.
 * ------------------------------------------------------------------------
 * Input:
 *      hMsg - The reliable 1xx message.
 * Output:
 *      pRseqStep - the step in the rseq header.
 *      pbLegal1xx - is the rel 1xx message legal (contains legal rseq header)
 *                   or not.
 ***************************************************************************/
void RVCALLCONV TransactionMsgIs1xxLegalRel(IN  RvSipMsgHandle hMsg,
                                            OUT RvUint32      *pRseqStep,
                                            OUT RvBool        *pbLegal1xx);

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSACTION_MSG_RECEIVED_H */
