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
 *                              <SecAgreeStatus.c>
 *
 * This file manages the security-agreement statuses: decides on status occurance,
 * acts upon occuring statuses, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Aug 2007
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeState.h"
#include "SecAgreeStatus.h"
#include "SecAgreeCallbacks.h"
#include "SecAgreeUtils.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void RVCALLCONV SecAgreeStatusOccur(
									IN  SecAgree*            pSecAgree,
							 		IN  RvSipSecAgreeStatus  eStatus);

/*-----------------------------------------------------------------------*/
/*                         SEC AGREE STATUS FUNCTIONS                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeStatusConsiderNewStatus
 * ------------------------------------------------------------------------
 * General: Check if a new status occurs and report this status to the 
 *          application.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object.
 *         eProcessing  - The occurring action that might cause the change in
 *                        status
 *         responseCode - The response code of the message being processed,
 *                        if relevant.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStatusConsiderNewStatus(IN  SecAgree*           pSecAgree,
												    IN  SecAgreeProcessing  eProcessing,
													IN  RvInt16             responseCode)
{
	if (RV_FALSE == SecAgreeUtilsIsGmInterfaceSupport(pSecAgree))
	{
		return RV_OK;
	}

	if (eProcessing == SEC_AGREE_PROCESSING_IPSEC_ACTIVATION)
	{
		if (pSecAgree->eSecAssocType != RVSIP_SEC_AGREE_SEC_ASSOC_UNDEFINED)
		{
			/* This is the activation of ipsec and the creation of security association.
			   The security association cannot have a type yet */
			RvLogExcep(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeStatusConsiderNewStatus - security-agreement=0x%p: has security association type %d before activating ipsec",
					   pSecAgree, pSecAgree->eSecAssocType));
			return RV_ERROR_UNKNOWN;
		}
		/* According to ES-283.003, the new security-associations are labeled temporary until 
		   the next successful transaction */
		SecAgreeStatusOccur(pSecAgree, RVSIP_SEC_AGREE_STATUS_IPSEC_TEMPORARY_ASSOCIATION);
	}

	if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD && 
		pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT &&
		SecAgreeStateIsActive(pSecAgree) == RV_TRUE)
	{
		if (pSecAgree->eSecAssocType == RVSIP_SEC_AGREE_SEC_ASSOC_TEMPORARY &&
			responseCode == 200)
		{
			/* According to ES-283.003 this is the time when the security association status
			   changes from temporary to established */
			SecAgreeStatusOccur(pSecAgree, RVSIP_SEC_AGREE_STATUS_IPSEC_ESTABLISHED_ASSOCIATION);
		}
	}

	if (eProcessing == SEC_AGREE_PROCESSING_MSG_TO_SEND && 
		pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER &&
		SecAgreeStateIsActive(pSecAgree) == RV_TRUE)
	{
		if (pSecAgree->eSecAssocType == RVSIP_SEC_AGREE_SEC_ASSOC_TEMPORARY &&
			pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP &&
			responseCode == 200)
		{
			/* According to ES-283.003 this is the time when the security association status
			   changes from temporary to established */
			SecAgreeStatusOccur(pSecAgree, RVSIP_SEC_AGREE_STATUS_IPSEC_ESTABLISHED_ASSOCIATION);
		}
	}

	return RV_OK;
}


/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeStatusOccur
 * ------------------------------------------------------------------------
 * General: Modifies security-agreement object on the occurrence of a new
 *          status
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree   - Pointer to the security-agreement object.
 *         eStatus     - The occurring status
 ***************************************************************************/
static void RVCALLCONV SecAgreeStatusOccur(
									IN  SecAgree*            pSecAgree,
							 		IN  RvSipSecAgreeStatus  eStatus)
{
	if (eStatus == RVSIP_SEC_AGREE_STATUS_IPSEC_TEMPORARY_ASSOCIATION)
	{
		pSecAgree->eSecAssocType = RVSIP_SEC_AGREE_SEC_ASSOC_TEMPORARY;
	}
	else if (eStatus == RVSIP_SEC_AGREE_STATUS_IPSEC_ESTABLISHED_ASSOCIATION) 
	{
		pSecAgree->eSecAssocType = RVSIP_SEC_AGREE_SEC_ASSOC_ESTABLISHED;
	}	

	SecAgreeCallbacksStatusEv(pSecAgree, eStatus, NULL /* pInfo */);
}
 

#endif /* #ifdef RV_SIP_IMS_ON */

