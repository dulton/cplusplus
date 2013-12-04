
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
 *                              <CallLegAuth.h>
 *
 *  Handles Message Events received from the transaction layer.
 *  The events includes message received and message to sent events.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

#ifndef CALL_LEG_AUTH_H
#define CALL_LEG_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef  RV_SIP_AUTH_ON
#ifndef RV_SIP_PRIMITIVES

#include "CallLegObject.h"
#include "RvSipMsg.h"
#include "_SipTransaction.h"


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegAuthAddAuthorizationHeadersToMsg
 * ------------------------------------------------------------------------
 * General: Adds authorization header to outgoing message. The authorization
 *          header was built from the authentication header parameters.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hTransc  - the transaction responsible for the message.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthAddAuthorizationHeadersToMsg(
                                 IN  CallLeg                 *pCallLeg,
                                 IN  RvSipTranscHandle       hTransc,
                                 IN  RvSipMsgHandle          hMsg);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * CallLegAuthSaveAuthenticationHeaderToCallLeg
 * ------------------------------------------------------------------------
 * General: saves the last Authentication header received, inside the CallLeg
 *          structure
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hTransc  - Handle to the transaction.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegAuthSaveAuthenticationHeaderToCallLeg(
                                 IN  CallLeg                 *pCallLeg,
                                 IN  RvSipTranscHandle       hTransc,
                                 IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * CallLegAuthAddAuthorizationHeaderToACK
 * ------------------------------------------------------------------------
 * General: Adds authorization header to outgoing ACK message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	pCallLeg - Pointer to the call-leg.
 *          inviteResponseCode  - the response code the call-leg got, and sends 
 *                      the ACK for.
 *		    hMsg     - Handle to the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthAddAuthorizationHeaderToACK(
								 IN  CallLeg                 *pCallLeg,
                                 IN  RvInt16                inviteResponseCode,
								 IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * CallLegAuthSaveInviteAuthorizationHeaderToCallLeg
 * ------------------------------------------------------------------------
 * General: Saves the Authorization header, inside the CallLeg structure.
 *          If you wish to suuply the WWW authorization header supply it in
 *          hInviteAuthorizHeader and give NULL in hInviteProxyAuthorizHeader.
 *          If you wish to suuply the proxy authorization header supply it in
 *          hInviteProxyAuthorizHeader and give NULL in hInviteAuthorizHeader.
 *          You can also supply both headers here.
 *          current version supports only 2 Authorization headers in a request,
 *          (for UA and for proxy). we save both headers.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hInviteAuthorizHeader - Handle to the WWW authorization header to
 *                                  be set to the call-leg.
 *          hInviteProxyAuthorizHeader - Handle to the proxy authorization header
 *                                       to be set to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthSaveInviteAuthorizationHeaderToCallLeg(
                                 IN  CallLeg                        *pCallLeg,
                                 IN  RvSipMethodType                 eRequestMethod,
                                 IN  RvSipAuthorizationHeaderHandle  hInviteAuthorizHeader,
                                 IN  RvSipAuthorizationHeaderHandle  hInviteProxyAuthorizHeader);

#endif
/* SPIRENT_BEGIN */
/***************************************************************************
 * CallLegAuthHandleUnAuthResponse
 * ------------------------------------------------------------------------
 * General: The function handle 401/407 response message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hTransc  - Handle to the transaction.
 *            statusCode - 401/407.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthHandleUnAuthResponse(
                                IN  CallLeg             *pCallLeg,
                                IN  RvSipTranscHandle   hTransc);

/***************************************************************************
 * CallLegAuthRespondUnauthenticated - server function
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given authentication header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthRespondUnauthenticated(
                                   IN  CallLeg*             pCallLeg,
                                   IN  RvSipTranscHandle    hTransaction,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader,
                                   IN  RvChar              *strRealm,
                                   IN  RvChar              *strDomain,
                                   IN  RvChar              *strNonce,
                                   IN  RvChar              *strOpaque,
                                   IN  RvBool              bStale,
                                   IN  RvSipAuthAlgorithm   eAlgorithm,
                                   IN  RvChar              *strAlgorithm,
                                   IN  RvSipAuthQopOption   eQop,
                                   IN  RvChar              *strQop);



#endif /* #ifndef RV_SIP_PRIMITIVES  */
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef CALL_LEG_AUTH_H */

