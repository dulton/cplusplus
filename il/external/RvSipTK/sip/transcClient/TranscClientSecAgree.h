
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
 *                              <TranscClientSecAgree.h>
 *
 * In this file the transc-client attaches to a security agreement-object and uses it 
 * to obtain and maintain a security-agreement according to RFC3329.
 *
 *    Author                         Date
 *    ------                        ------
 *	  Gilad Govrin					Aug 2006
 *********************************************************************************/

#ifndef TRANSC_CLIENT_SEC_AGREE_H
#define TRANSC_CLIENT_SEC_AGREE_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_ON
#include "_SipTranscClientTypes.h"
#include "TranscClientObject.h"
#include "RvSipSecAgreeTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this transc-client. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the transc-client will exploit this 
 *          agreement and the data it brings. If not, the transc-client will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient   - Pointer to the transc-client.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeAttachSecAgree(
								IN  SipTranscClient              *pTranscClient,
								IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * TranscClientSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this transc-client. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient         - Pointer to the transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeDetachSecAgree(
										IN  SipTranscClient            *pTranscClient);

/***************************************************************************
 * TranscClientSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the transc-client:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient         - Pointer to the Transc-client.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgToSend(
											IN  SipTranscClient             *pTranscClient,
											IN  RvSipMsgHandle               hMsg);

/***************************************************************************
 * TranscClientSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient      - Pointer to the Transc-client.
 *          hMsg            - Handle to the message
 *          hTransc         - The transaction that notified destination resolution
 *                            Notice: only one of hTrx and hTransc is valid.
 ***************************************************************************/
void RVCALLCONV TranscClientSecAgreeDestResolved(
								IN  SipTranscClient              *pTranscClient,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTranscHandle             hTransc);

/***************************************************************************
 * TranscClientSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the transaction-client:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient       - Pointer to the Transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgSendFailure(
											IN  SipTranscClient    *pTranscClient);

/***************************************************************************
 * TranscClientSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the Transc-client:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient    - Pointer to the Transc-client.
 *          hMsg          - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgRcvd(
										IN  SipTranscClient             *pTranscClient,
										IN  RvSipMsgHandle               hMsg);


void RVCALLCONV TranscClientDetachFromSecObjIfNeeded(
													 IN  SipTranscClient      *pTranscClient,
													 IN  RvSipSecAgreeHandle   hSecAgree);

RvStatus RVCALLCONV TranscClientAttachToSecObjIfNeeded(IN  SipTranscClient    *pTranscClient,
													   IN  RvSipSecObjHandle   hSecObj);

/*-----------------------------------------------------------------------*/
/*                          CALLBACKS                                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this Transc-client. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the Transc-client.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj);

/***************************************************************************
 * TranscClientSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this Transc-client. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the Transc-client.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: ifdef TRANSC_CLIENT_SEC_AGREE_H */


