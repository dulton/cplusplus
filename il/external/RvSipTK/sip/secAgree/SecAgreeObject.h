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
 *                              <SecAgreeObject.h>
 *
 * This file defines the security-agreement object, attributes and interface functions.
 * The security-agreement object is used to negotiate a security agreement with the
 * remote party, and to maintain this agreement in further requests, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef SEC_AGREE_OBJECT_H
#define SEC_AGREE_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
#include "SecAgreeState.h"

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
 * SecAgreeInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr - Pointer to the Security-Mgr
 *         hAppSecAgree - The application handle for the new security-agreement.
 *         pSecAgree    - Pointer to the new security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeInitialize(
									IN    SecAgreeMgr*					pSecAgreeMgr,
									IN    RvSipAppSecAgreeHandle		hAppSecAgree,
									INOUT SecAgree*			            pSecAgree);

/***************************************************************************
 * SecAgreeTerminate
 * ------------------------------------------------------------------------
 * General: Notifies the security-agreement owners on its termination and sends
 *          the security-agreement to the termination queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - Pointer to the security-agreement to terminate.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeTerminate(
                                 IN SecAgree*      pSecAgree);

/***************************************************************************
 * SecAgreeDestruct
 * ------------------------------------------------------------------------
 * General: Free the security-agreement resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - Pointer to the security-agreement to free.
 ***************************************************************************/
void RVCALLCONV SecAgreeDestruct(IN  SecAgree*     pSecAgree);

/***************************************************************************
 * SecAgreeRemoveOwner
 * ------------------------------------------------------------------------
 * General: Detaches an existing owner from the security-agreement object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree   - Pointer to the security-agreement
 *         pOwnerInfo  - Owner information
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeRemoveOwner(
							 IN  SecAgree*               pSecAgree,
							 IN  SecAgreeOwnerRecord*    pOwnerInfo);

/***************************************************************************
 * SecAgreeCreateHeader
 * ------------------------------------------------------------------------
 * General: Create header on the security-agreement object page
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object
 *         eType        - The type of the header to create
 * Output: phHeader     - The created header
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCreateHeader(
									IN   SecAgree* 	      pSecAgree,
									IN   RvSipHeaderType  eType,
									OUT  void**           phHeader);

/***************************************************************************
 * SecAgreeCopy
 * ------------------------------------------------------------------------
 * General: Copies local information from source security-agreement to 
 *          destination security-agreement. The fields being copied here are:
 *          1. Local security list. 
 *          2. Auth header for server security-agreement only.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pDestSecAgree   - Pointer to the destination security-agreement
 *         pSourceSecAgree - Pointer to the source security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCopy(
							 IN   SecAgree*      pDestSecAgree,
							 IN   SecAgree*      pSourceSecAgree);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SecAgreeSetPortC(
							 IN   SecAgree*      pSecAgree,
							 IN   int portC);
#endif
/* SPIRENT_END */

/***************************************************************************
 * SecAgreeSetRole
 * ------------------------------------------------------------------------
 * General: Sets the given role to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - The security-agreement object
 ***************************************************************************/
void RVCALLCONV SecAgreeSetRole(IN   SecAgree*          pSecAgree,
								IN   RvSipSecAgreeRole  eRole);

/*------------ S E C U R I T Y  O B J E C T  F U N C T I O N S ------------*/

/***************************************************************************
 * SecAgreeObjectUpdateSecObj
 * ------------------------------------------------------------------------
 * General: Decides according to the inner state and the event being processed
 *          what actions to apply to the security object: initiate, activate
 *          or terminate.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree   - Pointer to the security-agreement object
 *          eProcessing - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectUpdateSecObj(
									  IN  SecAgree*                   pSecAgree,
									  IN  SecAgreeProcessing          eProcessing);

/***************************************************************************
 * SecAgreeObjectSupplySecObjToOwners
 * ------------------------------------------------------------------------
 * General: Go over the owners of this security-agreement and supply them
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Handle to the security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSupplySecObjToOwners(
											IN   SecAgree*    pSecAgree);

/*------------------- N E T W O R K   F U N C T I O N S  -----------------*/

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * SecAgreeObjectSetLocalAddr
 * ------------------------------------------------------------------------
 * General: Sets the given local address to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree     - Pointer to the security-agreement object
 *          pLocalAddress - The local address to set
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetLocalAddr(
									  IN   SecAgree*            pSecAgree,
									  IN   RvSipTransportAddr*  pLocalAddress);

/***************************************************************************
 * SecAgreeObjectSetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Sets the given remote address to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree      - Pointer to the security-agreement object
 *          pRemoteAddress - The remote address to set
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetRemoteAddr(
									  IN   SecAgree*            pSecAgree,
									  IN   RvSipTransportAddr*  pRemoteAddress);
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SecAgreeObjectSetSendOptions
 * ------------------------------------------------------------------------
 * General: Sets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp. The send options shall be set
 *          in parallel to setting the remote address via 
 *          RvSipSecAgreeSetRemoteAddr (or deciding that the default remote
 *          address shall be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree    - The security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetSendOptions(
                                     IN  SecAgree                   *pSecAgree,
									 IN  RvSipSecAgreeSendOptions   *pOptions);

/***************************************************************************
 * SecAgreeObjectGetSendOptions
 * ------------------------------------------------------------------------
 * General: Gets the sending options of the security agreement. 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree    - The security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectGetSendOptions(
                                    IN  SecAgree                   *pSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions);
#endif /* RV_SIGCOMP_ON */

/*------------------ L O C K   F U N C T I O N S -----------------*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * SecAgreeLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks the security-agreement according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement object
***********************************************************************************/
RvStatus RVCALLCONV SecAgreeLockAPI(IN  SecAgree*   pSecAgree);

/************************************************************************************
 * SecAgreeUnlockAPI
 * ----------------------------------------------------------------------------------
 * General: Unlocks security-agreement according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree       - pointer to the security-agreement
 ***********************************************************************************/
void RVCALLCONV SecAgreeUnLockAPI(IN  SecAgree*   pSecAgree);

/************************************************************************************
 * SecAgreeLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks security-agreement according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement.
***********************************************************************************/
RvStatus RVCALLCONV SecAgreeLockMsg(IN  SecAgree*   pSecAgree);

/************************************************************************************
 * SecAgreeUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks security-agreement according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement.
***********************************************************************************/
void RVCALLCONV SecAgreeUnLockMsg(IN  SecAgree*   pSecAgree);
#else
#define SecAgreeLockAPI(a) (RV_OK)
#define SecAgreeUnLockAPI(a)
#define SecAgreeLockMsg(a) (RV_OK)
#define SecAgreeUnLockMsg(a)
#endif

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SEC_AGREE_CLIENT_OBJECT_H*/

