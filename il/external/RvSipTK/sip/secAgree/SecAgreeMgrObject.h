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
 *                              <SecAgreeMgrObject.h>
 *
 * This file defines the security-agrement manager object, attributes and interface functions.
 * The security-agreement manager manager the activity of all security-agreement clients and
 * servers of the SIP stack.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

#ifndef SEC_AGREE_MGR_OBJECT_H
#define SEC_AGREE_MGR_OBJECT_H

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

/***************************************************************************
 * SecAgreeMgrCreateSecAgree
 * ------------------------------------------------------------------------
 * General: Creates a new Security-Agreement object on the Security-Mgr's pool.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr       - Pointer to the security-mgr.
 *         hAppSecAgree       - The application handle for the new security-agreement.
 * Output: phSecAgree         - Pointer to the new security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrCreateSecAgree(
										IN  SecAgreeMgr*                    pSecAgreeMgr,
										IN  RvSipAppSecAgreeHandle          hAppSecAgree,
										OUT RvSipSecAgreeHandle*            phSecAgree);

/***************************************************************************
 * SecAgreeMgrRemoveSecAgree
 * ------------------------------------------------------------------------
 * General: Removes a security-agreement object from the Security-Mgr's pool.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the security-mgr.
 *         pSecAgree       - Pointer to the security-agreement object to remove.
 ***************************************************************************/
void RVCALLCONV SecAgreeMgrRemoveSecAgree(
										IN  SecAgreeMgr*				pSecAgreeMgr,
										IN  SecAgree*			        pSecAgree);

/***************************************************************************
 * SecAgreeMgrGetEvHandlers
 * ------------------------------------------------------------------------
 * General: Gets the event handlers structure of the given owner.
 * Return Value: The event handlers
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeMgr    - Pointer to the Security-Mgr
 *          eOwnerType      - The type of owner to get its event handlers.
 ***************************************************************************/
SipSecAgreeEvHandlers* RVCALLCONV SecAgreeMgrGetEvHandlers(
							IN   SecAgreeMgr                      *pSecAgreeMgr,
							IN   RvSipCommonStackObjectType        eOwnerType);

/***************************************************************************
 * SecAgreeMgrGetDefaultLocalAddr
 * ------------------------------------------------------------------------
 * General: Initializes the local address structure of the security-agreement
 *          with default values
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the security-agreement mgr.
 * Inout:  pSecAgree       - Pointer to the security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrGetDefaultLocalAddr(
								IN    SecAgreeMgr*				 pSecAgreeMgr,
								INOUT SecAgree*                  pSecAgree);

/***************************************************************************
 * SecAgreeMgrInsertSecAgreeOwner
 * ------------------------------------------------------------------------
 * General: Inserts a new owner of this security-agreement to the owner's hash
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr.
 *         pSecAgree        - Pointer to the security-agreement
 *         pOwnerInfo       - Pointer to the owner information record.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrInsertSecAgreeOwner(
										IN  SecAgreeMgr*         pSecAgreeMgr,
										IN  SecAgree*            pSecAgree,
										IN  SecAgreeOwnerRecord* pOwnerInfo);

/***************************************************************************
 * SecAgreeMgrFindSecAgreeOwner
 * ------------------------------------------------------------------------
 * General: Searches for the hash entry of the given owner for this security-
 *          agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr.
 *         pSecAgree        - Pointer to the security-agreement
 *         pOwner           - Pointer to the owner object.
 * Output: ppOwnerInfo      - Pointer to the owner record info.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrFindSecAgreeOwner(
										IN  SecAgreeMgr*          pSecAgreeMgr,
										IN  SecAgree*             pSecAgree,
										IN  void*                 pOwner,
										OUT SecAgreeOwnerRecord** ppOwnerInfo);

/***************************************************************************
 * SecAgreeMgrRemoveOwner
 * ------------------------------------------------------------------------
 * General: Removes an owner given its hash entry
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr
 *         pHashElement     - Pointer to the hash element
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrRemoveOwner(
										IN  SecAgreeMgr*         pSecAgreeMgr,
										IN  void*                pHashElement);
#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SEC_AGREE_MGR_OBJECT_H*/

