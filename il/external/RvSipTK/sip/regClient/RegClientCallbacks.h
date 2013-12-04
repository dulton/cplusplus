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
 *                              <RegClientCallbacks.h>
 *
 * This file defines the RegClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *********************************************************************************/

#ifndef REG_CLIENT_CALLBACKS_H
#define REG_CLIENT_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipRegClientTypes.h"
#include "RegClientObject.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RegClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the register-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the register-client.
 *        eReason - The reason for the state change.
 *        pRegClient - The register-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV RegClientCallbackStateChangeEv(
                            IN  RvSipRegClientState             eNewState,
                            IN  RvSipRegClientStateChangeReason eReason,
                            IN  RegClient                       *pRegClient);

/***************************************************************************
 * RegClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient -  Handle to the reg-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackMsgToSendEv(
                                       IN RvSipRegClientHandle   hRegClient,
                                       IN RvSipMsgHandle         hMsgToSend);

/***************************************************************************
 * RegClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hRegClient   - The reg client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackMsgReceivedEv (
                                       IN RvSipRegClientHandle  hRegClient,
                                  IN RvSipMsgHandle        hMsgReceived);

/***************************************************************************
 * RegClientTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  SipTranscClientOwnerHandle hRegClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved);

/***************************************************************************
 * RegClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  SipTranscClientOwnerHandle hRegClient,
                     IN  RvSipMsgHandle         hMsg);

/***************************************************************************
 * RegClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient     - The RegClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackNewConnInUseEv(
                     IN  RegClient*                     pRegClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


/***************************************************************************
 * RegClientCallbackObjExpired
 * ------------------------------------------------------------------------
 * General: calls to pfnObjExpireEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient     - The RegClient pointer
 ***************************************************************************/
void RVCALLCONV RegClientCallbackObjExpired(
                     IN  RegClient*                     pRegClient);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RegClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hRegClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_CALLBACKS_H */


/*nl for linux */

