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
 *                              <PubClientTranscClientEv.h>
 *
 *  Handles state changed events received from the transaction layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef PUB_CLIENT_TRANSC_EV_H
#define PUB_CLIENT_TRANSC_EV_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipPubClientTypes.h"
#include "_SipTranscClientTypes.h" 
#include "RvSipTransactionTypes.h"
#include "RvSipMsg.h"
#include "_SipTransaction.h"


/***************************************************************************
 * PubClientTranscClientClientEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in the active transactions.
 *          The publish-client will react and change state accordingly.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occured.
 *            hTransaction - The transaction in which the "event state changed"
 *                         occured.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV PubClientTranscClientEvStateChanged(
								  IN  SipTranscClientOwnerHandle       hTranscClientOwner,
								  IN  SipTranscClientState			 eNewState,
								  IN  SipTranscClientStateChangeReason eReason,
								  IN  RvInt16							 responseCode);


/***************************************************************************
 * PubClientTranscClientEvMsgToSend
 * ------------------------------------------------------------------------
 * General: Called right before the transaction attempts to send a message
 * (not retransmission). Adds the list of Contact headers and the Expires
 * header that were set to the publish-client to the message
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message to send" occured.
 *            hTransaction - The transaction in which the "event message to
 *                         send" occured.
 *            pMsgToSend -   The message the transaction is going to send. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientEvMsgToSend(
								   IN  SipTranscClientOwnerHandle	hTranscClientOwner,
								   IN  RvSipMsgHandle				hMsg);


/***************************************************************************
 * PubClientTranscClientEvMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when the transaction has received a new message (not
 * retransmission). Calls the event message received of the application.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message received" occured.
 *            pTransaction - The transaction in which the "event message
 *                         received" occured.
 *            pMsgReceived - The new message the transaction has received. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientEvMsgReceived (
									  IN  SipTranscClientOwnerHandle	hTranscClientOwner,
									  IN  RvSipMsgHandle				hMsg);

/***************************************************************************
 * PubClientMsgReceivedEvHandler
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler call- back
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPubClient   - The pub client in which the "event message received" occured.
 *          pMsgReceived - The new message the transaction has received. After
 *                       the callback returns the message object can not be
 *                        =used anymore (by the "owner" of this callback).
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV PubClientMsgReceivedEvHandler (
                                       IN RvSipTranscOwnerHandle  hPubClient,
                                  IN RvSipMsgHandle          hMsgReceived);

/***************************************************************************
 * PubClientTranscClientEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientEvOtherURLAddressFound(
                     IN  RvSipTranscHandle			hTransc,
                     IN  SipTranscClientOwnerHandle hTranscClientOwner,
                     IN  RvSipMsgHandle				hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool                *bAddressResolved);



/***************************************************************************
 * PubClientTranscClientFinalDestResolvedEv
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
 *            hPubClient   - The pub-client handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientFinalDestResolvedEv(
                            IN RvSipTranscHandle			hTransc,
                            IN SipTranscClientOwnerHandle	hPubClient,
                            IN RvSipMsgHandle				hMsgToSend);


/***************************************************************************
 * PubClientTranscClientNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the pub-client is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                         otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - PubClient handle
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientNewConnInUseEv(
                     IN  SipTranscClientOwnerHandle     hPubClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


/***************************************************************************
 * PubClientTranscClientObjExpiredEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the pub-client is about to expire
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - PubClient handle
 ***************************************************************************/
void RVCALLCONV PubClientTranscClientObjExpiredEv(
                     IN  SipTranscClientOwnerHandle         hPubClient);

/***************************************************************************
 * PubClientTranscClientObjTerminated
 * ------------------------------------------------------------------------
 * General: Handle transcClient Termianted.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - PubClient handle
 ***************************************************************************/
void RVCALLCONV PubClientTranscClientObjTerminated(
                     IN  SipTranscClientOwnerHandle         hPubClient);
#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * PubClientTranscClientSigCompMsgNotRespondedEv
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
 *               RV_OK no futher message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hPubClient    - The pub-client handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  SipTranscClientOwnerHandle       hPubClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef PUB_CLIENT_TRANSC_EV_H */
