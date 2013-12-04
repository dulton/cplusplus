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
 *                              <PubClientCallbacks.h>
 *
 * This file defines the PubClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/

#ifndef PUB_CLIENT_CALLBACKS_H
#define PUB_CLIENT_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipPubClientTypes.h"
#include "PubClientObject.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PubClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the publish-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the publish-client.
 *        eReason - The reason for the state change.
 *        pPubClient - The publish-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV PubClientCallbackStateChangeEv(
                            IN  RvSipPubClientState             eNewState,
                            IN  RvSipPubClientStateChangeReason eReason,
                            IN  PubClient                       *pPubClient);

/***************************************************************************
 * PubClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient -  Handle to the pub-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackMsgToSendEv(
                                       IN RvSipPubClientHandle   hPubClient,
                                       IN RvSipMsgHandle         hMsgToSend);

/***************************************************************************
 * PubClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPubClient   - The pub client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackMsgReceivedEv (
                                       IN RvSipPubClientHandle  hPubClient,
                                  IN RvSipMsgHandle        hMsgReceived);

/***************************************************************************
 * PubClientTranscEvOtherURLAddressFound
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
RvStatus RVCALLCONV PubClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  SipTranscClientOwnerHandle hPubClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved);

/***************************************************************************
 * PubClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hPubClient,
                     IN  RvSipMsgHandle         hMsg);

/***************************************************************************
 * PubClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient     - The PubClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackNewConnInUseEv(
                     IN  PubClient*                     pPubClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


/***************************************************************************
 * PubClientCallbackObjExpired
 * ------------------------------------------------------------------------
 * General: calls to pfnObjExpireEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient     - The PubClient pointer
 ***************************************************************************/
void RVCALLCONV PubClientCallbackObjExpired(
                     IN  PubClient*                     pPubClient);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * PubClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hPubClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef PUB_CLIENT_CALLBACKS_H */


/*nl for linux */

