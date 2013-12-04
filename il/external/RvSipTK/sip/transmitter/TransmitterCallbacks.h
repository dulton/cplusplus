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
 *                              <TransmitterCallbacks.h>
 *
 * This file contains all callbacks called from the Transmitter layer
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos               January 2004
 *********************************************************************************/

#ifndef TRANSMITTER_CALLBACKS_H
#define TRANSMITTER_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitter.h"
#include "TransmitterObject.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER CALLBACK FUNCTIONS                    */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransmitterCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Calls the state change callback of the transmitter
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx      - The transmitter pointer
 *        eNewState - The new state of the transmitter.
 *        eReason   - The reason for the state change.
 *        hMsg      - A message related to this state
 *        pExtraInfo- Extra info related to this state.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TransmitterCallbackStateChangeEv(
                            IN  Transmitter                       *pTrx,
                            IN  RvSipTransmitterState             eNewState,
                            IN  RvSipTransmitterReason            eReason,
                            IN  RvSipMsgHandle                    hMsg,
                            IN  void*                             pExtraInfo);


/***************************************************************************
 * TransmitterCallbackOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: calls to pfnOtherURLAddressFoundEv
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTrx           - The sip stack Transmitter handle
 *        hTransc        - The transaction handle
 *        hAppTransc     - The application handle for this transaction.
 *        hMsg           - The message that includes the other URL address.
 *        hAddress       - Handle to unsupport address to be converted.
 * Output:hSipURLAddress - Handle to the SIP URL address - this is an empty
 *                         address object that the application should fill.
 *        bAddressResolved-Indication wether the SIP URL address was filled.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterCallbackOtherURLAddressFoundEv(
                     IN  Transmitter*               pTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    bAddressResolved);

/************************************************************************************
 * TransmitterCallbacksResolveApplyRegExpCb
 * ---------------------------------------------------------------------------
 * General: asks the application to resolve a regular expression
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input: pTrx - resolver
 *        strRegexp      - a regular expression from NAPTR query.
 *        strString      - the string to apply the regexp on
 *        matchSize      - the size of the pMatchArray
 *        eFlags         - flags to take in account when applying the 
 *                         regular expression
 * Output:pMatches       - The eFlags should be filled with results of applying 
 *                         the regexp on the string 
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterCallbacksResolveApplyRegExpCb(
                            IN Transmitter* pTrx,
                             IN  RvChar*   strRegExp,
                             IN  RvChar*   strString,
                             IN RvInt32     matchSize,
                             IN  RvSipTransmitterRegExpFlag eFlags,
                             OUT RvSipTransmitterRegExpMatch* pMatches);
/***************************************************************************
* TransmitterCallbackMgrResolveAddressEv
* ------------------------------------------------------------------------
* General: calls to pfnMgrResolveAddressEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hTrx           - The sip stack Transmitter handle
*        hostName       - The hostname string that should be resolved.
*        pAddrToResolve - A structure containing all the information of
*                         the address that should be resolved.
* Output:pAddrToResolve - A structure containing all the information of
*                         the resolved address.
*        pbAddressResolved - Indication whether the pResolvedAddress was filled.
*        pbConfinueResolution - indication to the stack how to proceed (one
*                          of three options that were described above).
***************************************************************************/
RvStatus RVCALLCONV TransmitterCallbackMgrResolveAddressEv(
                                           IN  Transmitter*               pTrx,
                                           IN  RvChar*                    hostName,
                                           INOUT  RvSipTransportAddr      *pAddrToResolve,
                                           OUT RvBool                     *pbAddressResolved,
                                           OUT RvBool                     *pbConfinueResolution);
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef TRANSMITTER_CALLBACKS_H*/



