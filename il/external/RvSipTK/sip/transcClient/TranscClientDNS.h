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
 *                              <TranscClientDNS.h>
 *
 * This file defines the contains implementation for DNS functions.
 * All functions in this file are for RV_DNS_ENHANCED_FEATURES_SUPPORT compilation
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 August 2006
 *********************************************************************************/

#ifndef TRANSC_CLIENT_DNS_H
#define TRANSC_CLIENT_DNS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

#include "_SipTranscClientTypes.h"
#include "TranscClientObject.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is use at MSG_SEND_FAILURE state.
 *          Calling to this function, cancel the sending of the message, and
 *          change state of the state machine back to previous state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc-client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSGiveUp (IN  SipTranscClient	*pTranscClient);

/***************************************************************************
 * TranscClientDNSContinue
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling this function, creates a new transaction that will
 *          be sent to the next IP in dns list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSContinue (IN  SipTranscClient	*pTranscClient);

/***************************************************************************
 * TranscClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          You can use this function for INVITE, BYE, RE-INVITE, and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transc-client is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid transc-client state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc-client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSReSendRequest (IN  SipTranscClient			*pTranscClient,
												  IN    SipTransactionMethod	eMethod,
												  IN  RvChar*					strMethod);

/***************************************************************************
* TranscClientDNSGetList
* ------------------------------------------------------------------------
* General: retrives DNS list object from the transaction object.
*
* Return Value: RV_OK or RV_ERROR_BADPARAM
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTranscClient - call leg which is responsible for the message.
* Output   phDnsList - DNS list handle
***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSGetList(
                              IN  SipTranscClient             *pTranscClient,
                              OUT RvSipTransportDNSListHandle *phDnsList);

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef TRANSC_CLIENT_DNS_H*/


