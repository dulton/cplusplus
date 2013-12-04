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
 *                              <CallLegReplaces.h>
 *
 *  Handles REFER process methods. Such as:
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

#ifndef CALL_LEG_REPLACES_H
#define CALL_LEG_REPLACES_H

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
/*-----------------------------------------------------------------------*/
/*                           REPLACES FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegReplacesAddToMsg
 * ------------------------------------------------------------------------
 * General: Sets Replaces header in an outgoing INVITE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg that sends the INVITE message.
 *          hMsg     - Handle to the message to add the Replaces header to.
 *          eMethod  - The method of the message. If the method is not INVITE then
 *                     the Replaces will not be added to the message.
 ***************************************************************************/
RvStatus CallLegReplacesAddToMsg(IN  CallLeg                *pCallLeg,
                                 IN  RvSipMsgHandle          hMsg,
                                 IN  SipTransactionMethod    eMethod);


/***************************************************************************
 * CallLegReplacesGetMatchedCall
 * ------------------------------------------------------------------------
 * General: This function is called when the call leg is in the OFFERING state.
 *          This function searches for the call leg that has the same Call-ID, to tag and
 *          from tag as the Replaces header in the original call leg. If a matched call
 *          leg is found then this call leg is returned as the function output, otherwise this
 *          pointer will be NULL.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg                - Pointer to the call leg that received an INVITE with Replaces header.
 *          hReplacesHeader         - Handle to the Replaces header.
 * Output:  hMatchedCallLeg         - Pointer to the handle to the call leg witht he same
 *                                    identifiers as the Replaces header. If no call leg was found
 *                                    this parameter will be NULL.
 *          *peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog doesn't fit.
 *                     application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces.
 ***************************************************************************/
RvStatus CallLegReplacesGetMatchedCall(
                            IN  CallLeg                   *pCallLeg,
                            OUT RvSipCallLegHandle        *hMatchedCallLeg,
                            OUT RvSipCallLegReplacesReason *peReason);

/***************************************************************************
 * CallLegReplacesLoadReplacesHeaderToCallLeg
 * ------------------------------------------------------------------------
 * General: Loads the Replaces header from the INVITE request to the Call leg. If the
 *          INVITE has more then one Replaces header then the message is not legal
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - Pointer to the call leg that received the INVITE.
 *          hMsg            - Handle to the message to load the Replaces header from.
 *          pblegalReplaces - If the INVITE has more then one replaces header then
 *                            the message is not legal, and this parameter will be false.
 ***************************************************************************/
RvStatus CallLegReplacesLoadReplacesHeaderToCallLeg(IN  CallLeg          *pCallLeg,
                                                    IN  RvSipMsgHandle    hMsg,
                                                    OUT RvBool          *pblegalReplaces);

/***************************************************************************
 * CallLegReplacesPrepareReplacesHeaderFromCallLeg
 * ------------------------------------------------------------------------
 * General: This function prepares a Replaces header from the Call-ID, from-tag and
 *          to-tag of a Call-Leg
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - The call leg to make the Replaces header from.
 *          hReplacesHeader - Handle to a Constructed Replaces header.
 * Output:  hReplacesHeader - The Replaces header with hCallLeg identifiers
 ***************************************************************************/
RvStatus CallLegReplacesPrepareReplacesHeaderFromCallLeg(IN       CallLeg                    *pCallLeg,
                                                         INOUT    RvSipReplacesHeaderHandle   hReplacesHeader);

/***************************************************************************
 * CallLegReplacesCompareReplacesToCallLeg
 * ------------------------------------------------------------------------
 * General: This function compares a Call leg to a Replaces header. The Call leg and Replaces
 *          header are equal if the Call-ID, from-tag and to-tag are equal.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - The call leg to compare with the Replaces header.
 *          hReplacesHeader - Handle to a Replaces header.
 * Output:  pbIsEqual       - The result of the comparison. RV_TRUE if the Call-leg and
 *                            Replaces header are equal, RV_FALSE - otherwise.
 ***************************************************************************/
RvStatus CallLegReplacesCompareReplacesToCallLeg(IN  CallLeg                  *pCallLeg,
                                                 IN  RvSipReplacesHeaderHandle hReplacesHeader,
                                                 OUT RvBool                  *pbIsEqual);

#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_REPLACES_H */

