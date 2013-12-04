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
 *                              <RegClientTranscEv.h>
 *
 *  Handles state changed events received from the transaction layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Oct 2006
 *********************************************************************************/


#ifndef TRANSC_CLIENT_TRANSC_EV_H
#define TRANSC_CLIENT_TRANSC_EV_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "_SipTranscClientTypes.h"
#include "RvSipMsg.h"
#include "_SipTransaction.h"


/***************************************************************************
 * TranscClientTranscEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in the active transactions.
 *          The Transc-client will change state accordingly.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occurred.
 *            hTransaction - The transaction in which the "event state changed"
 *                         occurred.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV TranscClientTranscEvStateChanged(
                            IN RvSipTranscHandle                   hTransaction,
                            IN RvSipTranscOwnerHandle              hTranscClient,
                            IN RvSipTransactionState               eNewState,
                            IN RvSipTransactionStateChangeReason   eReason);


/***************************************************************************
 * TranscClientTranscEvMsgToSend
 * ------------------------------------------------------------------------
 * General: Called right before the transaction attempts to send a message
 * (not retransmission). 
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message to send" occurred.
 *            hTransaction - The transaction in which the "event message to
 *                         send" occurred.
 *            pMsgToSend -   The message the transaction is going to send. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvMsgToSend(
                                       IN RvSipTranscHandle        hTransaction,
                                       IN RvSipTranscOwnerHandle   hTranscClient,
                                       IN RvSipMsgHandle           hMsgToSend);


/***************************************************************************
 * TranscClientTranscEvMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when the transaction has received a new message (not
 * retransmission). Calls the event message received of the client.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message received" occurred.
 *            pTransaction - The transaction in which the "event message
 *                         received" occurred.
 *            pMsgReceived - The new message the transaction has received. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvMsgReceived (
                                  IN RvSipTranscHandle       hTransaction,
                                  IN RvSipTranscOwnerHandle  hTranscClient,
                                  IN RvSipMsgHandle          hMsgReceived);


/***************************************************************************
 * TranscClientTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the client that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvOtherURLAddressFound(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle     hAddress,
                     OUT RvSipAddressHandle     hSipURLAddress,
                     OUT RvBool               *bAddressResolved);



/***************************************************************************
 * TranscClientTranscFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hTranscClient   - The transc-client handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscFinalDestResolvedEv(
                            IN RvSipTranscHandle       hTransc,
                            IN RvSipTranscOwnerHandle  hTranscClient,
                            IN RvSipMsgHandle          hMsgToSend);


/***************************************************************************
 * TranscClientTranscNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the client that the transc-client is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                         otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - SipTranscClient handle
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscNewConnInUseEv(
                     IN  RvSipTranscHandle              hTransc,
                     IN  RvSipTranscOwnerHandle         hTranscClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TranscClientTranscSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp compressed request
 *          was sent over the transaction and a SigComp timer expired
 *          before receiving any response. This callback is part of a
 *          recovery mechanism and it supplies information about the previous
 *          sent request (that wasn't responded yet). 
 *          NOTE: A RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED value of the out
 *          parameter is referred as a request from the client to block
 *          the sending of additional requests in the current transaction
 *          session.
 *
 * Return Value: RvStatus. If the client returns a value other then
 *               RV_OK no further message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hTranscClient    - The Transc-client handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            client wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hTranscClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_TRANSC_EV_H */
