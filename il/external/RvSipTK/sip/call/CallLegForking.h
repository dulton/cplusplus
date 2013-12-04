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
 *                              <CallLegFroking.h>
 *
 * This file defines the callLeg functions related to forking issue.
 * 
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 March 2004
 *********************************************************************************/
#ifndef CALL_LEG_FORKING_H
#define CALL_LEG_FORKING_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifndef RV_SIP_PRIMITIVES
#include "CallLegObject.h"


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegForkingCreateForkedCallLegFromMsg
 * ------------------------------------------------------------------------
 * General: This function handles 1xx/2xx response message that has no call-leg.
 *          1. search for the original call-leg.
 *          2. create a new forked created call-leg.
 *          3. inject the message to the new call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLegMgr -  Handle to the call-leg manager.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingCreateForkedCallLegFromMsg(
                                             IN CallLegMgr*    pMgr,
                                             IN RvSipMsgHandle hMsg,
                                             OUT CallLeg**     ppCallLeg,
                                             OUT RvInt32*        pCurIdentifier,
											 OUT RvBool*		 bNotifyApp);

/***************************************************************************
 * CallLegForkingHandle1xxMsg
 * ------------------------------------------------------------------------
 * General: This function handles forked - 1xx response.
 *          Calling this function causes the following actions to happen:
 *          1. RvSipCallLegMsgRcvdEv is called on the given call-leg.
 *          2. RvSipCallLegChangeStateEv is called if needed, change state
 *             from IDLE to PROCEEDING - if this is the first 1xx on this call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingHandle1xxMsg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    IN RvSipMsgHandle   hResponseMsg);

/***************************************************************************
 * CallLegForkingHandleFirst2xxMsg
 * ------------------------------------------------------------------------
 * General: This function handles first 2xx response, received in a forked call-leg.
 *          Calling this function causes the following actions to happen:
 *          1. RvSipCallLegMsgRcvdEv is called on the given call-leg.
 *          2. Handle the 2xx as a regular first 2xx from the transaction layer.
 *             (update state machine, create and send ACK request, etc.)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingHandleFirst2xxMsg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    IN RvSipMsgHandle   hResponseMsg);

/***************************************************************************
 * CallLegForkingDestructAckMsgParams 
 * ------------------------------------------------------------------------
 * General: This function destructs the ACK request in a call-leg that was created
 *          from a second (or more) response, with no transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg - Handle to the call-leg.
 *			hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegForkingDestructAckMsgParams (
                                    IN CallLeg    *pCallLeg);


/*  T R A N S M I T T E R    E V E N T     H A N D L E R S  */
/***************************************************************************
 * CallLegForkingTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg of the ACK Transmitter state change.
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -     The sip stack Transmitter handle
 *            hAppTrx -     The transaction handle.
 *            eState  -     The new Transmitter state
 *            eReason -     The reason for the state change.
 *            hMsg    -     When the state relates to the outgoing message, the
 *                          message is supplied.
 *            pExtraInfo - specific information for the new state.
 ***************************************************************************/
void RVCALLCONV CallLegForkingTrxStateChangeEv(
       IN  RvSipTransmitterHandle            hTrx,
       IN  RvSipAppTransmitterHandle         hAppTrx,
       IN  RvSipTransmitterState             eState,
       IN  RvSipTransmitterReason            eReason,
       IN  RvSipMsgHandle                    hMsg,
       IN  void*                             pExtraInfo);

/*****************************************************************************
 * CallLegForkingTrxOtherURLAddressFoundEv
 * ---------------------------------------------------------------------------
 * General: implementation to the other url found event handler of the transmitter.
 * Return Value: RvStatus - RvStatus
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx        - the transaction transmitter
 *          hAppTrx     - App handle. in this case always points to transaction
 *          hMsg        - Handle of the message that contains the Other
 *                        URL address.
 *          hAddress    - The other URL address handle to be "converted"
 *                        to SIP URL address .
 *
 * Output:  hSipURLAddress      - Handle to the constructed SIP URL address.
 *          pbAddressResolved   - Indication of a successful/failed address
 *                                resolving.
 *
 ****************************************************************************/
RvStatus RVCALLCONV CallLegForkingTrxOtherURLAddressFoundEv(
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    pbAddressResolved);

/***************************************************************************
 * CallLegForkingTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the call-Leg's ack timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-Leg to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegForkingTimerRelease(IN CallLeg *pCallLeg);


/******************************************************************************
 * CallLegForkingInitCallLeg
 * ----------------------------------------------------------------------------
 * General: Loads data from Original Call-Leg and from Message into forked
 *          Call-Leg. Inserts forked Call-Leg into hash. Gets forked Call-Leg
 *          to RVSIP_CALL_LEG_STATE_PROCEEDING / RVSIP_CALL_LEG_STATE_ACCEPTED
 *          state. Dialog identifiers for the forked Call-Leg are took from
 *          the Message.
 * Return Value:
 *          RV_OK on success; error code, defined in RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pMgr              - Pointer to Call-Leg Manager object.
 *        pCallLeg          - Pointer to the Forked Call-Leg object.
 *        pOriginalCallLeg  - Pointer to the Original Call-Leg object.
 *        hResponseMsg      - Handle to Message, caused forked dialog creation.
 *        bSwitchToAndFromTags - If RV_TRUE, TO and FROM tags from the Message,
 *                            will be switched while being inserted into 
 *                            Forked Call-Leg (Forked Call-Leg can be created
 *                            as a result of response arrival, or as a result
 *                            of new request arrival (e.g. NOTIFY).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegForkingInitCallLeg(
                                    IN  CallLegMgr      *pMgr,
                                    IN  CallLeg         *pCallLeg,
                                    IN  CallLeg         *pOriginalCallLeg,
                                    IN  RvSipMsgHandle  hResponseMsg,
                                    IN  RvBool          bSwitchToAndFromTags);

/***************************************************************************
 * CallLegForkingTerminateOnFailure
 * ------------------------------------------------------------------------
 * General: This function terminates a forked call-leg, in case it do not
 *          have a forked timer yet.
 *          The function is called when handling a forked 1xx fails, and so 
 *          the call remains with no timer...
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
void RVCALLCONV CallLegForkingTerminateOnFailure(CallLeg* pCallLeg);

#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef CALL_LEG_FORKING_H */


