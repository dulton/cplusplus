
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
 *                              <CallLegSecAgree.h>
 *
 * In this file the call-leg attaches to a security agreement-object and uses it 
 * to obtain and maintain a security-agreement according to RFC3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

#ifndef CALL_LEG_SEC_AGREE_H
#define CALL_LEG_SEC_AGREE_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipCallLegTypes.h"
#include "CallLegObject.h"
#include "RvSipSecAgreeTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this call-leg. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the call-leg will exploit this 
 *          agreement and the data it brings. If not, the call-leg will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Pointer to the call-leg.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeAttachSecAgree(
								IN  CallLeg                     *pCallLeg,
								IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * CallLegSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this call-leg. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeDetachSecAgree(
										IN  CallLeg            *pCallLeg);

/***************************************************************************
 * CallLegSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the call-leg:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgToSend(
											IN  CallLeg                     *pCallLeg,
											IN  RvSipMsgHandle               hMsg);

/***************************************************************************
 * CallLegSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - Pointer to the call-leg.
 *          hMsg            - Handle to the message
 *          hTrx            - The transmitter that notified destination resolution
 *          hTransc         - The transaction that notified destination resolution
 *                            Notice: only one of hTrx and hTransc is valid.
 ***************************************************************************/
void RVCALLCONV CallLegSecAgreeDestResolved(
								IN  CallLeg                      *pCallLeg,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTransmitterHandle        hTrx,
							    IN  RvSipTranscHandle             hTransc);

/***************************************************************************
 * CallLegSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the call-leg:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgSendFailure(
											IN  CallLeg                     *pCallLeg);

/***************************************************************************
 * CallLegSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the call-leg:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 *          hTransc          - The transaction that received the message.
 *          hMsg             - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgRcvd(
										IN  CallLeg                     *pCallLeg,
										IN  RvSipTranscHandle            hTransc,
										IN  RvSipMsgHandle               hMsg);

/*-----------------------------------------------------------------------*/
/*                          CALLBACKS                                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this call-leg. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the call-leg.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj);

/***************************************************************************
 * CallLegSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this call-leg. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the call-leg.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: ifdef CALL_LEG_SEC_AGREE_H */


