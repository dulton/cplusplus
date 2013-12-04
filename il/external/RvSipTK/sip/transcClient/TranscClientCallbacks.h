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
 *                              <TranscClientCallbacks.h>
 *
 * This file defines the TranscClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Oct 2006
 *********************************************************************************/

#ifndef TRANSC_CLIENT_CALLBACKS_H
#define TRANSC_CLIENT_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipTranscClientTypes.h"
#include "TranscClientObject.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the transc-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the transc-client.
 *        eReason - The reason for the state change.
 *        pTranscClient - The Transc-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackStateChangeEv(
                            IN  SipTranscClientState             eNewState,
                            IN  SipTranscClientStateChangeReason eReason,
                            IN  SipTranscClient                  *pTranscClient,
							IN  RvInt16							 responseCode);

/***************************************************************************
 * TranscClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient -  Handle to the transc-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackMsgToSendEv(
                                       IN SipTranscClientHandle   hTranscClient,
                                       IN RvSipMsgHandle         hMsgToSend);

/***************************************************************************
 * TranscClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTranscClient   - The transc client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackMsgReceivedEv (
                                       IN SipTranscClientHandle  hTranscClient,
                                  IN RvSipMsgHandle        hMsgReceived);

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
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved);

/***************************************************************************
 * TranscClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg);

/***************************************************************************
 * TranscClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient     - The SipTranscClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackNewConnInUseEv(
                     IN  SipTranscClient*                     pTranscClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TranscClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          client wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hTranscClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * TranscClientCallbackExpirationAlertEv
 * ------------------------------------------------------------------------
 * General: Notify the client about the object expiration in the server.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments: pTranscClient - The pointer of the expired object.
 * Input:     
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackExpirationAlertEv(SipTranscClient *pTranscClient);

/***************************************************************************
 * TranscClientCallbackTerminatedEv
 * ------------------------------------------------------------------------
 * General: Notify the client about the object terminated in the server.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments: pTranscClient - The pointer of the terminated object.
 * Input:     
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackTerminatedEv(SipTranscClient *pTranscClient);

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_CALLBACKS_H */


/*nl for linux */

