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
 *                              <CallLegDNS.h>
 *
 * This file defines the contains implementation for DNS functions.
 * All functions in this file are for RV_DNS_ENHANCED_FEATURES_SUPPORT compilation
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 August 2002
 *********************************************************************************/

#ifndef CALL_LEG_DNS_H
#define CALL_LEG_DNS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
#include "RvSipCallLegTypes.h"
#include "CallLegTypes.h"
#include "CallLegObject.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegGiveUpDNS
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, delete the sending of the message, and
 *          change state of the state machine back to previous state.
 *          You can use this function for INVITE, BYE, RE-INVITE, CANCEL and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSGiveUp (IN  CallLeg*   pCallLeg);

/***************************************************************************
 * CallLegContinueDNS
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          You can use this function for INVITE, BYE, RE-INVITE, CANCEL and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSContinue (IN  CallLeg* pCallLeg);

/***************************************************************************
 * CallLegDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          You can use this function for INVITE, BYE, RE-INVITE, CANCEL and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSReSendRequest (IN  CallLeg* pCallLeg);

/***************************************************************************
* CallLegGetDNSList
* ------------------------------------------------------------------------
* General: retrives DNS list object from the transaction object.
*
* Return Value: RV_OK or RV_ERROR_BADPARAM
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call leg which is responsible for the message.
* Output   phDnsList - DNS list handle
***************************************************************************/
RvStatus RVCALLCONV CallLegDNSGetList(
                              IN  CallLeg                     *pCallLeg,
                              OUT RvSipTransportDNSListHandle *phDnsList);

/***************************************************************************
 * CallLegTranscGiveUpDNS
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE status of a general
 *          transcation.
 *          Calling to this function, delete the sending of the message, and
 *          terminate it's transaction.
 *          You can use this function if you got MSG_SEND_FAILURE status in
 *          RvSipCallLegTranscResolvedEv event.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 *          hTransc  - Handle to the transaction the request belongs to.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscGiveUp (
                                            IN  CallLeg*   pCallLeg,
                                            IN  RvSipTranscHandle    hTransc);

/***************************************************************************
 * CallLegTranscContinueDNS
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address.
 *          You can use this function if you got MSG_SEND_FAILURE status in
 *          RvSipCallLegTranscResolvedEv event.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 *          hTransc  - Handle to the transaction the general request belongs to.
 * Output:  phNewTransc - Handle to the new transaction - in case of general
 *                      transaction only!!!
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscContinue (
                                      IN  CallLeg*             pCallLeg,
                                      IN  RvSipTranscHandle    hTransc,
                                      OUT RvSipTranscHandle*   phNewTransc);

/***************************************************************************
* CallLegDNSTranscReSendRequest
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the message to the next ip
*          address.
*          You can use this function if you got MSG_SEND_FAILURE status in
*          RvSipCallLegTranscResolvedEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - call leg that sent the request.
*          hTransc  - Handle to the transaction the request belongs to.
***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscReSendRequest (
                                               IN  CallLeg*             pCallLeg,
                                               IN  RvSipTranscHandle    hNewTransc);

#endif /*defined(RV_DNS_ENHANCED_FEATURES_SUPPORT) */
#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_DNS_H*/


