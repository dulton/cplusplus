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
 *                              <CallLegInvite.c>
 *
 *  Handles INIVTE process methods. 
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 March 2005
 *********************************************************************************/
#ifndef CALL_LEG_INVITE_H
#define CALL_LEG_INVITE_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLegTypes.h"
#include "CallLegObject.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
           I N V I T E    O B J   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
* CallLegInviteCreateObj
* ------------------------------------------------------------------------
* General: Creates a new invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
*          bReInvite  - initial invite or re-invite object.
* Output:  phNotify   - Handle to the new created notification object.
*          pRejectCode - in case of invite inside invite (old invite). may be NULL!!!
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCreateObj(IN  CallLeg*        pCallLeg,
                                           IN  RvSipAppCallLegInviteHandle hAppInvite,
                                           IN  RvBool            bReInvite,
                                           IN  RvSipCallLegDirection eDirection,
                                           OUT CallLegInvite*    *ppInviteObj,
                                           OUT RvUint16          *pRejectCode);

/***************************************************************************
* CallLegInviteDestruct
* ------------------------------------------------------------------------
* General: Destructs an invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteDestruct(IN  CallLeg*        pCallLeg,
                                                 IN  CallLegInvite  *pInvite);

/***************************************************************************
* CallLegInviteSetParams
* ------------------------------------------------------------------------
* General: Set parameters in a new invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg  
*          pInvite 
*          hTransc 
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSetParams(IN  CallLeg*        pCallLeg,
                                              IN  CallLegInvite    *pInvite,
                                              IN  RvSipTranscHandle hTransc);

/***************************************************************************
* CallLegInviteTerminate
* ------------------------------------------------------------------------
* General: terminates invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg  - Handle to the call-leg, owns the invite object.
*          pInvite   - Handle to the invite object.
*          eReason   - termination reason.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteTerminate(IN  CallLeg*        pCallLeg,
                                              IN  CallLegInvite  *pInvite,
                                              IN  RvSipCallLegStateChangeReason eReason);

/***************************************************************************
* CallLegInviteFindObjByCseq
* ------------------------------------------------------------------------
* General: finds the invite object with the given cseq..
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
*          cseq
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByCseq(IN   CallLeg*        pCallLeg,
                                               IN   RvInt32         cseq,
                                               IN   RvSipCallLegDirection eDirection,
                                               OUT  CallLegInvite*  *ppInviteObj);

/***************************************************************************
* CallLegInviteFindObjByState
* ------------------------------------------------------------------------
* General: finds the invite object of the initial invite request.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByState(IN  CallLeg*        pCallLeg,
                                                IN  RvSipCallLegModifyState eModifyState,
                                               OUT  CallLegInvite*  *ppInviteObj);

/***************************************************************************
* CallLegInviteFindObjByTransc
* ------------------------------------------------------------------------
* General: finds the invite object of the initial invite request.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByTransc(IN  CallLeg*        pCallLeg,
                                                 IN  RvSipTranscHandle hTransc,
                                                 OUT  CallLegInvite*  *ppInviteObj);

/***************************************************************************
 * CallLegInviteCreateAckMsgAndTrx
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCreateAckMsgAndTrx (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite);

/***************************************************************************
 * CallLegInviteCopyToHeaderFromMsg
 * ------------------------------------------------------------------------
 * General: Copies the to header from a message to the invite object.
 *          To header is copied from INVITE 2xx response and then placed
 *          in the outgoing INVITE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - pointer to the call-leg.
 *          pInvite - pointer to the invite object.
 *          hMsg - Handle to the msg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCopyToHeaderFromMsg (
                                    IN CallLeg*         pCallLeg,
									IN CallLegInvite*   pInvite,
									IN RvSipMsgHandle   hMsg);

/***************************************************************************
 * CallLegInviteSendAck
 * ------------------------------------------------------------------------
 * General: Sends an Ack message to the remote party. First the Call-leg's
 *          request URI is determined and then a Ack is sent using the
 *          transaction's API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The Ack was not sent.
 *               RV_OK - Ack message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 *            hTransc  - The transaction used to send the Ack message.
 *            pInvite  - The invite object sends the ACK.
 *          responseCode - the response code of the response message that trigered the ACK
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSendAck(
                            IN CallLeg                       *pCallLeg,
                            IN RvSipTranscHandle             hTransc,
                            IN CallLegInvite                *pInvite,
                            IN RvUint16                     responseCode);

/***************************************************************************
 * CallLegInviteTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the timer of the invite object, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pInvite - The invite object to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegInviteTimerRelease(IN CallLegInvite *pInvite);

/***************************************************************************
 * CallLegInviteSet2xxTimer
 * ------------------------------------------------------------------------
 * General: Set the 2xx-timer of an invite object.
 *          1. Get T1 and T2 values.
 *          2. If this is the first time - set the longTimeout. (when will 
 *             the call-leg be terminate, if will not receive an ACK)
 *          3. Decide for how long to set the 2xx-timer:
 *             A. If this is the first time - set timer to T1 ms.
 *             B. If not - set timer to min(T2, T1*2^numOfRetransmissions)
 *             C. Check what will come first - the decision we have just made,
 *                or the longTimeout. if longTimeout is sooner, we shall will
 *                set the timer to the time until longTimeout.
 *          4. Set the timer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSet2xxTimer(CallLeg*       pCallLeg,
                                             CallLegInvite* pInvite,
                                             RvBool         bFirstSetting);

/***************************************************************************
* CallLegInviteSetAckTimer
* ------------------------------------------------------------------------
* General: sets the forked Ack Transmitter timer.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - The call-Leg this timer was called for.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSetAckTimer(IN CallLeg*      pCallLeg,
                                             IN CallLegInvite* pInvite);

/***************************************************************************
 * CallLegInviteResetTranscInInviteObj
 * ------------------------------------------------------------------------
 * General: Check if the given transaction is an invite transaction,
 *          and if so, delete the transaction handle in the correct invite object.
 *          Note - When working with old invite behavior, the termination of
 *          an invite transaction means that the invite procedure is finished,
 *          so we can terminate the invite object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteResetTranscInInviteObj(CallLeg*          pCallLeg,
                                                      CallLegInvite*    pInvite,
                                                      RvBool            bTimeout);

/***************************************************************************
 * CallLegInviteDetachInviteTransc
 * ------------------------------------------------------------------------
 * General: The function cuts the relationships between the invite transaction,
 *          and the call-leg. 
 *          The function remove the transaction from the invite object - and so,
 *          from the call-leg invite transactions list.
 *          It also detach the transaction owner - so it won't hold the call-leg 
 *          handle, and reset the pointer to the invite object, in the context memory.
 * Return Value: cseq step
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteDetachInviteTransc(IN  CallLeg        *pCallLeg,
											  IN  CallLegInvite  *pInvite,
											  OUT RvSipTranscHandle hTransc);

/***************************************************************************
 * CallLegInviteAttachOnInviteConnections
 * ------------------------------------------------------------------------
 * General: Attach the invite object as the transaction connections owner.
 *          We are doing it, because the transaction will be terminated in 
 *          a second, and so if the transaction is the only owner, the connections
 *          will be set to the 'not-in-use' connections list. 
 *          2xx retransmissions, however, may still be received on the connections.
 *          If the invite object is an owner too, only when it detaches from
 *          the connection (after invite linger timeout) the connection is set
 *          to the list, and 2xx is not expected to be received on it.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus CallLegInviteAttachOnInviteConnections(IN  CallLeg        *pCallLeg,
                                                IN CallLegInvite     *pInvite,
                                                IN RvSipTranscHandle  hTransc);

/***************************************************************************
 * CallLegInviteTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg of the Transmitter state change.
 *          (ack transmitter, and 2xx transmitter).
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
void RVCALLCONV CallLegInviteTrxStateChangeEv(
                       IN  RvSipTransmitterHandle            hTrx,
                       IN  RvSipAppTransmitterHandle         hAppTrx,
                       IN  RvSipTransmitterState             eState,
                       IN  RvSipTransmitterReason            eReason,
                       IN  RvSipMsgHandle                    hMsg,
                       IN  void*                             pExtraInfo);

/*****************************************************************************
 * CallLegInviteTrxOtherURLAddressFoundEv
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
RvStatus RVCALLCONV CallLegInviteTrxOtherURLAddressFoundEv(
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    pbAddressResolved);

/***************************************************************************
 * CallLegInviteDefineReInviteRejectCode
 * ------------------------------------------------------------------------
 * General: Define if we should reject a re-invite request, because of an
 *          active re-invite process.
 *
 * IMPORTANT: This function should be called ONLY in oldInviteHandling mode
 *
 * Return Value: 0 - no need to reject.
 *               491 or 500 - need to reject.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 ***************************************************************************/
RvUint16 CallLegInviteDefineReInviteRejectCode(CallLeg*         pCallLeg,
                                               RvSipTranscHandle hTransc);

#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_INVITE_H */
