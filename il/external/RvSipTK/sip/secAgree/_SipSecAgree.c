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
 *                              <_SipSecAgree.c>
 *
 * This file defines the internal API of the security-agreement object.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
#include "_SipSecAgree.h"
#include "SecAgreeClient.h"
#include "SecAgreeServer.h"
#include "SecAgreeObject.h"
#include "SecAgreeState.h"
#include "SecAgreeUtils.h"
#include "SecAgreeMgrObject.h"
#include "RvSipSecurity.h"
#include "_SipSecuritySecObj.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV IsSecObjReady(IN   RvSipSecMgrHandle   hSecMgr, 
										 IN   RvSipSecObjHandle   hSecObj,
										 OUT  RvBool*             pbIsReady);

/*-----------------------------------------------------------------------*/
/*                         SECURITY AGREEMENT FUNCTIONS                  */
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
							 OUT RvSipSecObjHandle           *phSecObj)
{
	SecAgree*                 pSecAgree     = (SecAgree*)hSecAgree;
	SecAgreeMgr*              pSecAgreeMgr  = pSecAgree->pSecAgreeMgr;
	RLIST_ITEM_HANDLE         hListElem;
	SecAgreeOwnerRecord*      pOwnerInfo;
	RvSipSecObjHandle         hSecObj;
	RvStatus                  rv = RV_OK;

	*phSecObj = NULL;

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}
	
	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "SipSecAgreeAttachOwner - Security-agreement 0x%p attaching owner 0x%p",
			  pSecAgree, pOwner));

	if ((RV_FALSE == SecAgreeStateIsValid(pSecAgree)) ||
		(RV_FALSE == SecAgreeStateCanAttachOwner(pSecAgree)) ||
		(pSecAgree->numOfOwners >= 1 && RV_FALSE == SecAgreeStateCanAttachSeveralOwners(pSecAgree)))
	{
		/* We allow attaching to a security agreement object only in valid state.
		   We allow attaching more than one owner only if it is a steady state */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeAttachOwner - Cannot attach security-agreement=0x%p in state %s (owner=0x%p, num of owners=%d)",
                  pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState), pOwner, pSecAgree->numOfOwners));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Allocate new owner info record structure into the owners list */
	rv = RLIST_InsertTail(pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, &hListElem);
	if(RV_OK != rv || hListElem == NULL)
    {
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeAttachOwner - Failed to insert new owner to security-agreement's 0x%p list (owner=0x%p)",
                  pSecAgree, pOwner));
        SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_OUTOFRESOURCES;
    }
	pOwnerInfo = (SecAgreeOwnerRecord*)hListElem;

	/* Update owner details in the owner record */
	pOwnerInfo->owner.pOwner     = pOwner;
	pOwnerInfo->owner.eOwnerType = eOwnerType;
	pOwnerInfo->pInternalOwner   = pInternalOwner;

	/* Update list element in the owner record */
	pOwnerInfo->hListItem = hListElem;

	/* set event handlers */
	pOwnerInfo->pEvHandlers = SecAgreeMgrGetEvHandlers(pSecAgreeMgr, eOwnerType);

	/* Insert the owner record to the hash of owners in the manager
	   The manager updates the hash element pointer within the record structure */
	rv = SecAgreeMgrInsertSecAgreeOwner(pSecAgreeMgr, pSecAgree, pOwnerInfo);
	if(rv != RV_OK)
    {
		RLIST_Remove(pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, hListElem);
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeAttachOwner - Failed to insert new owner to security-manager's list (security-agreement=0x%p, owner=0x%p, rv=%d)",
                  pSecAgree, pOwner, rv));
        SecAgreeUnLockAPI(pSecAgree);
		return rv;
    }

	/* Increment owners counter */
	pSecAgree->numOfOwners += 1;

	/* Check if the security object is ready for use by the owner */
	hSecObj = pSecAgree->securityData.hSecObj;
	if (hSecObj != NULL && SecAgreeStateIsActive(pSecAgree) == RV_TRUE)
	{
		RvBool  bIsReady;
		rv = IsSecObjReady(pSecAgreeMgr->hSecMgr, hSecObj, &bIsReady);
		if (RV_OK == rv && bIsReady == RV_TRUE)
		{
			*phSecObj = hSecObj;
		}
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);
		
	return rv;
}

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
							 IN  RvSipSecAgreeHandle          hSecAgree,
							 IN  void                        *pOwner)
{
	SecAgree            *pSecAgree    = (SecAgree*)hSecAgree;
	SecAgreeMgr         *pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	SecAgreeOwnerRecord *pOwnerInfo;
	RvStatus             rv;

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}
	
	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "SipSecAgreeDetachOwner - Security-agreement 0x%p detaching from owner 0x%p",
			  pSecAgree, pOwner));

	/* Find the owner in the hash table. The returned record will indicate the list element
	   that holds this owner, therefore searching the list will not be needed */
	rv = SecAgreeMgrFindSecAgreeOwner(pSecAgreeMgr, pSecAgree, pOwner, &pOwnerInfo);
	if (RV_OK != rv && RV_ERROR_NOT_FOUND != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeDetachOwner - Failed to find security-agreement owner in the hash (security-agreement=0x%p, owner=0x%p, rv=%d)",
                  pSecAgree, pOwner, rv));
		SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	if (rv == RV_ERROR_NOT_FOUND)
	{
		/* No detaching required */
		RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SipSecAgreeDetachOwner - Security-agreement 0x%p is not attached owner 0x%p hence not detaching from it",
				pSecAgree, pOwner));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_OK;
	}

	/* remove owner and decrement owners counter */
	rv = SecAgreeRemoveOwner((SecAgree*)hSecAgree, pOwnerInfo);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeDetachOwner - Failed to remove owner 0x%p from security-agreement 0x%p, rv=%d)",
                  pOwner, pSecAgree, rv));
		SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

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
									OUT RvBool                 *pbShouldDetach)
{
	SecAgree* pSecAgree = (SecAgree*)hSecAgree;
	RvStatus  rv;

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}
	
	*pbShouldDetach = RV_FALSE;

	if (SecAgreeStateIsActive(pSecAgree) == RV_FALSE &&
		pSecAgree->securityData.hSecObj == hSecObj)
	{
		/* The security-agreement is not active and the given security object is that of
		   the security-agreement, hence we instruct the owner to detach from the security 
		   object as well */
		*pbShouldDetach = RV_TRUE;
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
                  "SipSecAgreeShouldDetachSecObj - Security-agreement 0x%p: instructing owner to detach from security object=0x%p",
                  pSecAgree, hSecObj));
	}
	else
	{
		if (hSecObj == pSecAgree->securityData.hSecObj)
		{
			RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					  "SipSecAgreeShouldDetachSecObj - Security-agreement 0x%p in state %s: The owner should not detach from security object 0x%p",
					  pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState), hSecObj));
		}
		else
		{
			RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					  "SipSecAgreeShouldDetachSecObj - Security-agreement 0x%p in state %s: The owner has a different security object 0x%p",
					  pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState), hSecObj));
		}
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

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
							 IN  RvSipMsgHandle                hMsg)
{
	SecAgree           *pSecAgree    = (SecAgree*)hSecAgree;
	RvStatus            rv           = RV_OK;

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	if (RVSIP_MSG_REQUEST == RvSipMsgGetMsgType(hMsg))
	{
		if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
		{
			rv = SecAgreeClientHandleMsgToSend(pSecAgree, hSecObj, hMsg);
		}
	}
	else /* response message */
	{
		if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
		{
			rv = SecAgreeServerHandleMsgToSend(pSecAgree, hMsg);
		}
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);
		
	return rv;
}

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
							 IN  RvSipTransmitterHandle        hTrx)
{
	SecAgree           *pSecAgree    = (SecAgree*)hSecAgree;
	RvStatus            rv;
	
	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return;
	}
	
	if (RVSIP_MSG_REQUEST == RvSipMsgGetMsgType(hMsg))
	{
		if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
		{
			rv = SecAgreeClientHandleDestResolved(pSecAgree, hMsg, hTrx);
			if (RV_OK != rv)
			{
				SecAgreeStateInvalidate(pSecAgree, RVSIP_SEC_AGREE_REASON_UNDEFINED);
			}
		}
	}
	/* there is no handling of dest resolved for server security-agreement */

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);
}

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
							 IN  RvSipSecAgreeHandle         hSecAgree)
{
	SecAgree           *pSecAgree    = (SecAgree*)hSecAgree;
	RvStatus            rv           = RV_OK;

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}
	
	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
	{
		rv = SecAgreeClientHandleMsgSendFailure(pSecAgree);
	}
	else if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		rv = SecAgreeServerHandleMsgSendFailure(pSecAgree);
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}


/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * IsSecObjReady
 * ------------------------------------------------------------------------
 * General: Checks that the security object ihas a chosen mechanism
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecMgr    - Handle to the security manager 
 *         hSecObj    - Handle to the security object
 * Output: pbIsReady  - Boolean indicating whether the security object is ready
 ***************************************************************************/
static RvStatus RVCALLCONV IsSecObjReady(IN   RvSipSecMgrHandle   hSecMgr, 
										 IN   RvSipSecObjHandle   hSecObj,
										 OUT  RvBool*             pbIsReady)
{
	RvSipSecurityMechanismType  eSecurityMech;
	RvStatus                    rv;
	
	*pbIsReady = RV_FALSE;

	if (hSecObj == NULL)
	{
		return RV_OK;
	}

	rv = SipSecObjGetMechanism(hSecMgr, hSecObj, &eSecurityMech);
	if (RV_OK != rv)
	{
		return rv;
	}

	if (eSecurityMech == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		return RV_OK;
	}

	*pbIsReady = RV_TRUE;
	
	return RV_OK;
}


#endif /* #ifdef RV_SIP_IMS_ON */

