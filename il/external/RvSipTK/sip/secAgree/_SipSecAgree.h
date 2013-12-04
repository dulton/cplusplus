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
 *                              <_SipSecAgree.h>
 *
 * This file defines the internal API of the security-agreement object.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef _SIP_SEC_AGREE_H
#define _SIP_SEC_AGREE_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "_SipSecAgreeTypes.h"
#include "RvSipMsg.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipTransactionTypes.h"
#include "_SipTransportTypes.h"
#include "_SipCommonTypes.h"
#include "RvSipSecurity.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
	
/***************************************************************************
 * SipSecAgreeAttachOwner
 * ------------------------------------------------------------------------
 * General: Attaches a new owner to the security-agreement object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree	  - Handle to the security-agreement
 *         pOwner         - Pointer to the owner to attach to this security-agreement
 *         eOwnerType     - The type of the supplied owner
 *         pInternalOwner - Pointer to the internal owner. This owner will be supplied
 *                          in internal callbacks (to the transaction, call-leg, reg-client...).
 *                          If this is transaction or call-leg, pInternalOwner and pOwner are
 *                          equivalent. If this is reg-client or pub-client, pOwner is
 *                          the reg or pub, and pInternalOwner is the transc-client they are
 *                          using.
 * Output: phSecObj       - Returns the security object of this security-agreement,
 *                          for the owner to use in further requests. The security
 *                          object is returned only if it is ready for use.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeAttachOwner(
							 IN  RvSipSecAgreeHandle          hSecAgree,
							 IN  void                        *pOwner,
							 IN  RvSipCommonStackObjectType   eOwnerType,
							 IN  void*                        pInternalOwner,
							 OUT RvSipSecObjHandle           *phSecObj);

/***************************************************************************
 * SipSecAgreeDetachOwner
 * ------------------------------------------------------------------------
 * General: Detaches an existing owner from the security-agreement object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree    - Pointer to the security-agreement
 *         pOwner       - Pointer to the owner to attach to this security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeDetachOwner(
							 IN  RvSipSecAgreeHandle	      hSecAgree,
							 IN  void                        *pOwner);

/***************************************************************************
 * SipSecAgreeShouldDetachSecObj
 * ------------------------------------------------------------------------
 * General: Used by the owner to determine whether to detach from its security
 *          object while detaching from the security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree      - Pointer to the security-agreement
 *         hSecObj        - Pointer to the security object
 * Output: pbShouldDetach - Boolean indication whether to detach from the 
 *                          security object
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeShouldDetachSecObj(
									IN  RvSipSecAgreeHandle     hSecAgree,
									IN  RvSipSecObjHandle       hSecObj,
									OUT RvBool                 *pbShouldDetach);

/***************************************************************************
 * SipSecAgreeHandleMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message about to be sent: adds security information
 *          and updates the security-agreement object accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         hSecObj         - Handle to the security object of the SIP object
 *                           sending the message
 *         hMsg            - Handle to the message to send
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeHandleMsgToSend(
							 IN  RvSipSecAgreeHandle           hSecAgree,
							 IN  RvSipSecObjHandle             hSecObj,
							 IN  RvSipMsgHandle                hMsg);

/***************************************************************************
 * SipSecAgreeHandleDestResolved
 * ------------------------------------------------------------------------
 * General: Performs last state changes and message changes based on the
 *          transport type that is obtained via destination resolution.
 *          For clients: If this is TLS on initial state, we abort negotiation
 *          attempt by removing all negotiation info from the message, and changing
 *          state to NO_AGREEMENT_TLS. If this is not TLS we change state to
 *          CLIENT_LOCAL_LIST_SENT.
 * Return Value: (-) we do not stop the message from being sent
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Pointer to the security-agreement
 *         hMsg            - Handle to the message
 *         hTrx            - The transmitter that notified destination resolution
 ***************************************************************************/
void RVCALLCONV SipSecAgreeHandleDestResolved(
							 IN  RvSipSecAgreeHandle           hSecAgree,
							 IN  RvSipMsgHandle                hMsg,
							 IN  RvSipTransmitterHandle        hTrx);

/***************************************************************************
 * SipSecAgreeHandleMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Invalidates a security-agreement whenever a message send failure
 *          occurs.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         eMsgType        - Indication whether this is a request message or a
 *                           response message
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeHandleMsgSendFailure(
							 IN  RvSipSecAgreeHandle         hSecAgree);


#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _SIP_SEC_AGREE_H*/

