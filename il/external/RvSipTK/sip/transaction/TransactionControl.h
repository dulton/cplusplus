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
 *                              <TransactionControl.h>
 *  Handles all tranaction activities initiated by the user such as
 *  request/response/ack/cancel.
 *  call-leg and more.
 *
 *    Author                         Date
 *    ------                        ------
 *   Sarit Mekler                   30/7/01
 *********************************************************************************/


#ifndef TRANSACTION_CONTROL_H
#define TRANSACTION_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "TransactionObject.h"



/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionControlSendRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a request message, according to the
 *          given method with the given address as a Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -   The transaction this request refers to.
 *            pRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 *                           accordingly.
*        bResolveAddr - RV_TRUE if we want the transmitter to resolve the dest addr
*                       from the message, RV_FALSE if the transmitter should use the
*                       address it currently hold.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRequest(
                                    IN  Transaction        *pTransc,
                                    IN  RvSipAddressHandle  hRequestUri,
                                    IN  RvBool              bResolveAddr);

/***************************************************************************
 * TransactionControlSendResponse
 * ------------------------------------------------------------------------
 * General: Creates and sends a response message
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc -   The transaction handle
 *          responseCode - The response code to use in the response
 *          strReasonPhrase - The reason fraze to use in the response.
 *          RvSipTransactionStateChangeReason - The reason to indicate when
 *                                              changing the state.
 *          bIsReliable - RV_TRUE if the response needs to be sent reliably
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendResponse(
                    IN  Transaction*       pTransc,
                    IN  RvUint16           responseCode,
                    IN  RvChar*            strReasonPhrase,
                    IN  RvSipTransactionStateChangeReason eReason,
                    IN  RvBool             bIsReliable);

/***************************************************************************
 * TransactionControlSendAck
 * ------------------------------------------------------------------------
 * General:  Creates and sends an acknowledgment message with the given
 *           address as the Request-Uri.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pTransc - The transaction handle.
 *            pRequestUri -  The Request-Uri to be used in the ACK message. The
 *                           message's Request-Uri will be the address given
 *                         here, and the message will be sent accordingly.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendAck(
                                            IN Transaction         *pTransc,
                                            IN RvSipAddressHandle hRequestUri);


/***************************************************************************
 * TransactionControlSendCancel
 * ------------------------------------------------------------------------
 * General: Cancels the transaction given as a parameter.
 *          calling TransactionControlCancel() will cause A client CANCEL
 *          transaction to be created with transaction key
 *          taken from the transaction that is about to be canceled.
 *          The CANCEL request will then be sent to the remote party.
 *          If the transaction to cancel is an Invite transaction it will
 *          assume the Invite Cancelled state. A general transaction will assume
 *          the general cancelled state.
 *          The newlly created CANCEL transaction will assume the Cancel sent
 *          state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction to cancel.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendCancel (IN     Transaction  *pTransc);

/***************************************************************************
 * TransactionControlGenerateAndSendResponse
 * ------------------------------------------------------------------------
 * General: Creates and sends a response message.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc -   The transaction this request refers to.
 *            responseCode - The response code for the response message
 *            strReasonPhrase - The reason phrase for the response
 *            bIsReliable - RV_TRUE if the response should be sent reliably.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlGenerateAndSendResponse(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint16        responseCode,
                                    IN  RvChar         *strReasonPhrase,
                                    IN  RvBool          bIsReliable);

/***************************************************************************
 * TransactionControlChangeToNextSrvState
 * ------------------------------------------------------------------------
 * General: Call the next server transaction state according to the
 *          trasnaction current state, method response code and
 *          the proxy indication
 * Return Value: next transaction state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        uRespCode - server response code.
 *        bIsReliable - indication whether the response is a reliable response
 *        eReason    - reason of changing the state.
 * output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionControlChangeToNextSrvState(
                                    IN  Transaction*                      pTransc,
                                    IN  RvSipTransactionState             ePrevState,
                                    IN  RvUint16                         uRespCode,
                                    IN  RvBool                           bIsReliable,
                                    IN  RvSipTransactionStateChangeReason eReason);

/***************************************************************************
 * TransactionControlSendRequestMsg
 * ------------------------------------------------------------------------
 * General: Sends a request with a prepared message that was set to the
 *          transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - Pointer to the transaction.
 *          bAddTopVia - indicate wether to add top via header before
 *                       sending the message.
 *    Output:    (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRequestMsg(IN  Transaction    *pTransc,
                                                     IN  RvBool         bAddTopVia);


/***************************************************************************
 * TransactionControlSendRespondMsg
 * ------------------------------------------------------------------------
 * General: Sends a response with a prepared message that was set to the
 *          transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction.
 *            bRemoveTopVia - indicate wether to remove top via header before
 *                          sending the message.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRespondMsg(IN  Transaction  *pTransc,
                                                     IN  RvBool        bRemoveTopVia);

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /*END OF: #ifndef TRANSACTION_CONTROL_H*/
