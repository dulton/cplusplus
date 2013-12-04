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
 *                              <sipCallLegMsgEv.h>
 *
 *  Handles Message Events received from the transaction layer.
 *  The events includes message received and message to sent events.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

#ifndef CALL_LEG_MSG_EV_H
#define CALL_LEG_MSG_EV_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipTransaction.h"
#include "CallLegObject.h"
#include "RvSipMsg.h"



/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegMsgEvMsgToSendHandler
 * ------------------------------------------------------------------------
 * General: Handles a message that is about to be sent and notify the
 *          application.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call leg the transaction belongs to.
 *          hTransc  - The transaction that is about to send this message.
 *            hMsg     - Handle to the message ready to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsgEvMsgToSendHandler(
                        IN  RvSipTranscHandle       hTransc,
                        IN  RvSipTranscOwnerHandle  hCallLeg,
                        IN  RvSipMsgHandle          hMsg);




/***************************************************************************
 * CallLegMsgEvMsgRcvdHandler
 * ------------------------------------------------------------------------
 * General: Handle a message received event. Take needed
 *          parameters from the message and notify the application.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call leg the transaction belongs to.
 *          hTransc  - The transaction that received this message.
 *            hMsg     - Handle to the message that was received.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsgEvMsgRcvdHandler(
                                    IN  RvSipTranscHandle       hTransc,
                                    IN  RvSipTranscOwnerHandle  hCallLeg,
                                    IN  RvSipMsgHandle          hMsg);






/***************************************************************************
* CallLegMsgForkedInviteResponseRcvdHandler
* ------------------------------------------------------------------------
* General: Handle a message received event. when the call-leg was created with 
*          no transaction (two responses were received after forking)
*          The function handles only response on INVITE messages.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg - The call leg the transaction belongs to.
*		 hMsg     - Handle to the message that was received.    
***************************************************************************/
RvStatus RVCALLCONV CallLegMsgForkedInviteResponseRcvdHandler(
									IN  CallLeg*        pCallLeg,
									IN  RvSipMsgHandle  hMsg);

/***************************************************************************
 * CallLegMsg2xxAckMsgToSendHandler
 * ------------------------------------------------------------------------
 * General: Handle a message to send event. when the call-leg was created with 
 *          no transaction (two responses were received after forking)
 *          The function handles only response on INVITE messages.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg - The call leg the ACK message belongs to.
 *          inviteResponseCode - the response code the call-leg got, and sends 
 *                     the ACK for.
 *          hMsg     - Handle to the message ready to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsg2xxAckMsgToSendHandler(
					    IN  CallLeg                *pCallLeg,
                        IN  RvUint16               inviteResponseCode,
						IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * CallLegMsgAckMsgRcvdHandler
 * ------------------------------------------------------------------------
 * General: Handle an ACK message Received event. 
 *          (since ACK is not related to a transaction, we must call the
 *          msg-rcvd-ev callback from here).
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The call leg the ACK message belongs to.
 *          hMsg     - Handle to the received ACK message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsgAckMsgRcvdHandler(
                        IN  CallLeg                *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_MSG_EV_H */

