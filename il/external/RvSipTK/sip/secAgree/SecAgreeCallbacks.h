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
 *                              <SecAgreeCallbacks.h>
 *
 * This file handles all callback calls.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

#ifndef SEC_AGREE_CALLBACKS_H
#define SEC_AGREE_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
	
/*----------------- SECURITY - AGREEMENT EVENT HANDLERS ---------------------*/

/***************************************************************************
 * SecAgreeCallbacksStateChangedEvHandler
 * ------------------------------------------------------------------------
 * General: Handles event state changed called from the event queue: calls
 *          the event state changed callback to notify the application on
 *          the change in state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pObj    	- A pointer to the security-agreement object.
 *         state    - An integer combination indicating the new state
 *         reason   - An integer combination indicating the new reason
 **************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksStateChangedEvHandler(
													IN void*    pObj,
                                                    IN RvInt32  state,
                                                    IN RvInt32  reason,
													IN RvInt32  objUniqueIdentifier);

/***************************************************************************
 * SecAgreeCallbacksStateTerminated
 * ------------------------------------------------------------------------
 * General: Handles the TERMINATED event state .
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pObj    	- A pointer to the security-agreement object.
 *         reason   - An integer combination indicating the new reason
 **************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksStateTerminated(
													IN void*    pObj,
                                                    IN RvInt32  reason);

/*---------------------- SECURITY OBJECT EVENT HANDLERS -----------------------*/

/******************************************************************************
 * SecAgreeCallbacksSecObjStateChangedEvHandler
 * ----------------------------------------------------------------------------
 * General: Handles security object state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - Handle of the Security Object
 *          hAppSecObj - Application handle of the Security Object
 *          eState     - New state of the Security Object
 *          eReason    - Reason for the state change
 *****************************************************************************/
void RVCALLCONV SecAgreeCallbacksSecObjStateChangedEvHandler(
                            IN  RvSipSecObjHandle            hSecObj,
                            IN  RvSipAppSecObjHandle         hAppSecObj,
                            IN  RvSipSecObjState             eState,
                            IN  RvSipSecObjStateChangeReason eReason);

/******************************************************************************
 * SecAgreeCallbacksSecObjStatusEvHandler
 * ----------------------------------------------------------------------------
 * General: Handles security object status notification
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - Handle of the Security Object
 *          hAppSecObj - Application handle of the Security Object
 *          eStatus    - Status of the Security Object
 *          pInfo      - Auxiliary information. Not in use.
 *****************************************************************************/
void RVCALLCONV SecAgreeCallbacksSecObjStatusEvHandler(
                            IN  RvSipSecObjHandle       hSecObj,
                            IN  RvSipAppSecObjHandle    hAppSecObj,
                            IN  RvSipSecObjStatus       eStatus,
                            IN  void*                   pInfo);

/*----------------- SECURITY - AGREEMENT  CALLBACKS ---------------------*/

/***************************************************************************
 * SecAgreeCallbacksRequiredEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnEvSecAgreeRequired callback.
 * Return Value: RvStatus returned from callback
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr       - Pointer to the security-agreement manager
 *         pOwner             - The sip object that will become the owner of this
 *                              security-agreement object. This sip object received the
 *                              message that caused the invoking of this event.
 *         pOwnerLock         - The triple lock of the owner, to open before callback
 *         hMsg               - The received message requestion for a security-agreement
 *         pCurrSecAgree      - The current security-agreement object of this owner, that is either
 *                              in a state that cannot handle negotiation (invalid, terminated,
 *                              etc) or is about to become INVALID because of negotiation data
 *                              arriving in the message (see eReason to determine the
 *                              type of data); hence requiring a new security-agreement object
 *                              to be supplied. NULL if there is no security-agreement object to
 *                              this owner.
 *         eRole              - The role of the required security-agreement object (client
 *                              or server)
 * Output: ppSecAgree         - The new security-agreement object to use.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksRequiredEv(
									IN  SecAgreeMgr*							pSecAgreeMgr,
									IN  RvSipSecAgreeOwner*						pOwner,
									IN  SipTripleLock*							pOwnerLock,
									IN  RvSipMsgHandle                          hMsg,
									IN  SecAgree*						        pCurrSecAgree,
									IN  RvSipSecAgreeRole				        eRole,
									OUT SecAgree**					            ppNewSecAgree);

/***************************************************************************
 * SecAgreeCallbacksDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnEvSecAgreeDetachSecAgrees callback.
 * Return Value: RvStatus returned from callback
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSecAgreeObj - The security-agreement to detach from
 *        hOwner       - Handle to the owner that should detach from this security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksDetachSecAgreeEv(
										  IN  SecAgree*                  pSecAgree,
										  IN  SecAgreeOwnerRecord*       pSecAgreeOwner);

/***************************************************************************
 * SecAgreeCallbacksAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnEvSecAgreeSctiveSecObj callback.
 * Return Value: RvStatus returned from callback
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSecAgree      - The security-agreement object
 *        pOwner       - Pointer to the owner that should be notified
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksAttachSecObjEv(
											IN  SecAgree*                  pSecAgree,
										    IN  SecAgreeOwnerRecord*        pOwner);

/***************************************************************************
 * SecAgreeCallbacksStatusEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnEvSecAgreeStatus callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree	- A pointer to the security-agreement object.
 *         eStatus		- The new security-agreement status.
 **************************************************************************/
void RVCALLCONV SecAgreeCallbacksStatusEv(
                                   IN  SecAgree*               pSecAgree,
                                   IN  RvSipSecAgreeStatus     eStatus,
								   IN  void*				   pInfo);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * SecAgreeCallbacksReplaceSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Calls the Calls the pfnEvSecAgreeReplaceSecAgree callback.
 * Return Value: RvStatus returned from callback
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr       - Pointer to the security-agreement manager
 *         pOwner             - The sip object that will become the owner of this
 *                              security-agreement object. This sip object received the
 *                              message that caused the invoking of this event.
 *         pOwnerLock         - The triple lock of the owner, to open before callback
 *         hMsg               - The received message requestion for a security-agreement
 *         pCurrSecAgree      - The current security-agreement object of this owner, that is either
 *                              in a state that cannot handle negotiation (invalid, terminated,
 *                              etc) or is about to become INVALID because of negotiation data
 *                              arriving in the message (see eReason to determine the
 *                              type of data); hence requiring a new security-agreement object
 *                              to be supplied. NULL if there is no security-agreement object to
 *                              this owner.
 *         eRole              - The role of the required security-agreement object (client
 *                              or server)
 * Output: bReplace           - RV_TRUE is to do the replacement; RV_FALSE if otherwise.
 *         ppNewSecAgree      - The new security-agreement object to use.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCallbacksReplaceSecAgreeEv(
									IN  SecAgreeMgr*							pSecAgreeMgr,
									IN  RvSipSecAgreeOwner*					pOwner,
									IN  SipTripleLock*						pOwnerLock,
									IN  RvSipMsgHandle                  hMsg,
									IN  SecAgree*						      pCurrSecAgree,
									IN  RvSipSecAgreeRole				   eRole,
                           OUT RvBool*                         bReplace,
									OUT SecAgree**					         ppNewSecAgree);

#endif
/* SPIRENT_END */


#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_CALLBACKS_H*/

