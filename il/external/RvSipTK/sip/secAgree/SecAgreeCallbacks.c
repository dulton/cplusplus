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
 *                              <SecAgreeCallbacks.c>
 *
 * This file handles all callback calls.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Feb 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/


#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON
#include "SecAgreeCallbacks.h"
#include "SecAgreeObject.h"
#include "SecAgreeUtils.h"
#include "SecAgreeMgrObject.h"
#include "SecAgreeState.h"
#include "SecAgreeServer.h"
#include "_SipSecuritySecObj.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static void RVCALLCONV SecAgreeCallbacksStateChangedEv(
                                   IN  SecAgree*               pSecAgree,
                                   IN  RvSipSecAgreeState      eState,
                                   IN  RvSipSecAgreeReason     eReason);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
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
													IN RvInt32  objUniqueIdentifier)
{
	SecAgree			*pSecAgree = (SecAgree*)pObj;
	RvSipSecAgreeState   eState    = (RvSipSecAgreeState)state;
	RvSipSecAgreeReason  eReason   = (RvSipSecAgreeReason)reason;
	RvStatus             rv;

	/* lock the security-agreement object */
	rv = SecAgreeLockMsg(pSecAgree);
    if (rv != RV_OK)
    {
        return rv;
    }

    if(objUniqueIdentifier != pSecAgree->secAgreeUniqueIdentifier)
    {
		/* The security-agreement was reallocated before this event
		   came out of the event queue, therefore we do not handle it */
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			  "SecAgreeCallbacksStateChangedEvHandler - Security-agreement 0x%p: different identifier. Ignore the event.",
			  pSecAgree));
		SecAgreeUnLockMsg(pSecAgree);
		return RV_OK;
	
    }
	if (eState != RVSIP_SEC_AGREE_STATE_TERMINATED &&
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_TERMINATED)
	{
		/* The security-agreement object was terminated before this state changed notification
		   came out of the event queue, therefore we do not call event state callback */
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "SecAgreeCallbacksStateChangedEvHandler - Security-agreement 0x%p was terminated. The application will not be notified on state %s",
				  pSecAgree, SecAgreeUtilsStateEnum2String(eState)));
		SecAgreeUnLockMsg(pSecAgree);
		return RV_OK;
	}

	/* Call the application event state changed callback */
	SecAgreeCallbacksStateChangedEv(pSecAgree, eState, eReason);


	/* unlock the security-agreement object */
	SecAgreeUnLockMsg(pSecAgree);

	return RV_OK;
}


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
                          IN RvInt32  reason)

{
	SecAgree			*pSecAgree = (SecAgree*)pObj;
	RvSipSecAgreeReason  eReason   = (RvSipSecAgreeReason)reason;
	RvStatus             rv;

	/* lock the security-agreement object */
	rv = SecAgreeLockMsg(pSecAgree);
    if (rv != RV_OK)
    {
        return rv;
    }

	/* Call the application event state changed callback */
	SecAgreeCallbacksStateChangedEv(pSecAgree, RVSIP_SEC_AGREE_STATE_TERMINATED, eReason);

	/* Destruct the security-agreement object */
	SecAgreeDestruct(pSecAgree);
	
	/* unlock the security-agreement object */
	SecAgreeUnLockMsg(pSecAgree);

	return RV_OK;
}

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
                            IN  RvSipSecObjStateChangeReason eReason)
{
	SecAgree*     pSecAgree = (SecAgree*)hAppSecObj;
	SecAgreeMgr*  pSecAgreeMgr;
	RvStatus      rv;

	/* lock the security-agreement object */
	rv = SecAgreeLockMsg(pSecAgree);
    if (rv != RV_OK)
    {
        return;
    }

	pSecAgreeMgr = pSecAgree->pSecAgreeMgr;

	switch (eState) 
	{
	case RVSIP_SECOBJ_STATE_CLOSING:
    case RVSIP_SECOBJ_STATE_CLOSED:
    case RVSIP_SECOBJ_STATE_TERMINATED:
		{
			if (pSecAgree->securityData.hSecObj == NULL)
			{
				/* The security-agreement is no longer interested in the closing indication */
				break;
			}
			/* The security object becomes invalid. Check the state of the security-agreement 
			   to decide whether to invalidate it */
			if (SecAgreeStateIsValid(pSecAgree) == RV_TRUE)
			{
				/* invalidating the security-agreement */
				RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 		  "SecAgreeCallbacksSecObjStateChangedEvHandler - Security-agreement=0x%p: Security-object=0x%p becomes unusable, security-agreement is invalidated", 
						  pSecAgree, hSecObj));
				SecAgreeStateInvalidate(pSecAgree, RVSIP_SEC_AGREE_REASON_SECURITY_OBJ_EXPIRED);
			}
			break;
		}
	default:
		{
			RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					 	"SecAgreeCallbacksSecObjStateChangedEvHandler - Security-agreement=0x%p: Nothing to do in security-object=0x%p state change callback", 
						pSecAgree, hSecObj));
			break;
		}
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockMsg(pSecAgree); 

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hSecObj);
#endif
	RV_UNUSED_ARG(eReason);
}

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
                            IN  void*                   pInfo)
{
	SecAgree*     pSecAgree = (SecAgree*)hAppSecObj;
	SecAgreeMgr*  pSecAgreeMgr;
	RvStatus      rv;

	/* lock the security-agreement object */
	rv = SecAgreeLockMsg(pSecAgree);
    if (rv != RV_OK)
    {
        return;
    }

	pSecAgreeMgr = pSecAgree->pSecAgreeMgr;


	switch (eStatus) 
	{
	case RVSIP_SECOBJ_STATUS_IPSEC_LIFETIME_EXPIRED:
		{
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					  "SecAgreeCallbacksSecObjStatusEvHandler - Security-agreement=0x%p: reporting status %s to the application (security-object=0x%p)", 
				      pSecAgree, SecAgreeUtilsStatusEnum2String(RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED), hSecObj));
		}
		SecAgreeCallbacksStatusEv(pSecAgree, RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED, pInfo);
		break;
	default:
		{
			RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					 	"SecAgreeCallbacksSecObjStatusEvHandler - Security-agreement=0x%p: Nothing to do in security-object=0x%p status callback", 
						pSecAgree, hSecObj));
			break;
		}
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockMsg(pSecAgree); 

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hSecObj);
#endif
}


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
									OUT SecAgree**					            ppNewSecAgree)
{
    RvInt32                nestedLock        = 0;
    RvThreadId             currThreadId      = 0;
	RvInt32                threadIdLockCount = 0;
	RvStatus               rv = RV_OK;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeCallbacksRequiredEv - requesting the application to supply security-agreement for owner 0x%d",
				pOwner->pOwner));

	if(pSecAgreeMgr->appMgrEvHandlers.pfnSecAgreeMgrSecAgreeRequiredEvHandler != NULL)
    {
        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                   "SecAgreeCallbacksRequiredEv: Owner 0x%p, Before callback",
				   pOwner->pOwner));

        /* pCurrSecAgree is not lock therefore there is no need to unlock it */

		if (NULL != pOwnerLock)
		{
			/* unlock the owner before calling application callback */
			SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr, pSecAgreeMgr->pLogSrc, pOwnerLock, &nestedLock, &currThreadId, &threadIdLockCount);
		}

        /* call application callback */
		rv = pSecAgreeMgr->appMgrEvHandlers.pfnSecAgreeMgrSecAgreeRequiredEvHandler(
												(RvSipSecAgreeMgrHandle)pSecAgreeMgr,
												pOwner,
												hMsg,
												(RvSipSecAgreeHandle)pCurrSecAgree,
												eRole,
												(RvSipSecAgreeHandle*)ppNewSecAgree);

		if (NULL != pOwnerLock)
		{
			/* lock the owner after returning from application callback */
			SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pOwnerLock, nestedLock, currThreadId, threadIdLockCount);
		}

        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksRequiredEv: Owner 0x%p, After callback. New Security-agreement=0x%p",
					pOwner->pOwner, *ppNewSecAgree));
    }
	else
	{
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksRequiredEv: Application did not register to callback"));
	}

	return rv;
}

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
										  IN  SecAgreeOwnerRecord*       pSecAgreeOwner)
{
    SecAgreeMgr*             pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvInt32                  nestedLock   = 0;
    SipTripleLock           *pTripleLock  = pSecAgree->tripleLock;
    RvThreadId               currThreadId;
	RvInt32                  threadIdLockCount;
	SipSecAgreeEvHandlers	*pEvHandlers  = pSecAgreeOwner->pEvHandlers;
	void*                    hOwner       = pSecAgreeOwner->pInternalOwner;
	RvStatus                 rv = RV_OK;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeCallbacksDetachSecAgreeEv - Security-agreement 0x%p notifies owner 0x%p to detach from it",
				pSecAgree, hOwner));

    if(pEvHandlers != NULL && (*pEvHandlers).pfnSecAgreeDetachSecAgreeEvHanlder != NULL)
    {
        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SecAgreeCallbacksDetachSecAgreeEv: Security-agreement 0x%p, Before callback", pSecAgree));

        SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr, pSecAgreeMgr->pLogSrc, pTripleLock, &nestedLock, &currThreadId, &threadIdLockCount);

        rv = (*pEvHandlers).pfnSecAgreeDetachSecAgreeEvHanlder(hOwner, (RvSipSecAgreeHandle)pSecAgree);

        SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pTripleLock, nestedLock, currThreadId, threadIdLockCount);

        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksDetachSecAgreeEv: Security-agreement 0x%p, After callback", pSecAgree));
    }
	else
	{
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksDetachSecAgreeEv: Security-agreement 0x%p,Application did not register to callback", pSecAgree));
	}

	return rv;
}

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
											IN  SecAgreeOwnerRecord*        pOwner)
{
	SecAgreeMgr*             pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvInt32                  nestedLock   = 0;
    SipTripleLock           *pTripleLock  = pSecAgree->tripleLock;
    RvThreadId               currThreadId;
	RvInt32                  threadIdLockCount;
	RvSipSecObjHandle        hSecObj      = pSecAgree->securityData.hSecObj;
	SipSecAgreeEvHandlers	*pEvHandlers  = pOwner->pEvHandlers;
	void*                    hOwner       = pOwner->pInternalOwner;
	RvStatus                 rv           = RV_OK;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeCallbacksAttachSecObjEv - Security-agreement 0x%p notifies owner 0x%p to attach to security-object 0x%p",
				pSecAgree, hOwner, pSecAgree->securityData.hSecObj));

    if(pEvHandlers != NULL && (*pEvHandlers).pfnSecAgreeAttachSecObjEvHanlder != NULL)
    {
        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SecAgreeCallbacksAttachSecObjEv: Security-agreement 0x%p, Before callback", pSecAgree));

        SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr, pSecAgreeMgr->pLogSrc, pTripleLock, &nestedLock, &currThreadId, &threadIdLockCount);

        rv = (*pEvHandlers).pfnSecAgreeAttachSecObjEvHanlder(hOwner, 
                                                             (RvSipSecAgreeHandle)pSecAgree, 
                                                             hSecObj);

		SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pTripleLock, nestedLock, currThreadId, threadIdLockCount);

        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksAttachSecObjEv: Security-agreement 0x%p, After callback", pSecAgree));
    }
	else
	{
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksAttachSecObjEv: Security-agreement 0x%p,Application did not register to callback", pSecAgree));
	}

	return rv;
}

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
								   IN  void*				   pInfo)
{
	SecAgreeMgr*             pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvInt32                  nestedLock   = 0;
    SipTripleLock           *pTripleLock  = pSecAgree->tripleLock;
    RvThreadId               currThreadId;
	RvInt32                  threadIdLockCount;
    RvSipAppSecAgreeHandle   hAppOwner = pSecAgree->hAppOwner;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeCallbacksStatusEv - Notifying the application on new status of security-agreement 0x%p",
				pSecAgree));

    if(pSecAgreeMgr->appEvHandlers.pfnSecAgreeStatusEvHandler != NULL)
    {
        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                   "SecAgreeCallbacksStatusEv: Security-agreement 0x%p, Before callback",
				   pSecAgree));

        SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr,pSecAgreeMgr->pLogSrc, pTripleLock, &nestedLock, &currThreadId, &threadIdLockCount);

        pSecAgreeMgr->appEvHandlers.pfnSecAgreeStatusEvHandler(
												(RvSipSecAgreeHandle)pSecAgree,
												hAppOwner,
												eStatus,
												pInfo);

        SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pTripleLock, nestedLock, currThreadId, threadIdLockCount);

        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksStatusEv: Security-agreement 0x%p, After callback.",
					pSecAgree));
    }
	else
	{
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksStatusEv: Application did not register to callback"));
	}
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeCallbacksStateChangedEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnEvSecAgreeStateChanged callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree	- A pointer to the security-agreement object.
 *         eState		- The new security-agreement state.
 *         eReason		- The reason for the state change (if informative).
 **************************************************************************/
static void RVCALLCONV SecAgreeCallbacksStateChangedEv(
                                   IN  SecAgree*               pSecAgree,
                                   IN  RvSipSecAgreeState      eState,
                                   IN  RvSipSecAgreeReason     eReason)
{
	SecAgreeMgr*             pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvInt32                  nestedLock   = 0;
    SipTripleLock           *pTripleLock  = pSecAgree->tripleLock;
    RvThreadId               currThreadId;
	RvInt32                  threadIdLockCount;
    RvSipAppSecAgreeHandle hAppOwner = pSecAgree->hAppOwner;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeCallbacksStateChangedEv - Notifying the application on state change for security-agreement 0x%p",
				pSecAgree));

    if(pSecAgreeMgr->appEvHandlers.pfnSecAgreeStateChangedEvHandler != NULL)
    {
        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                   "SecAgreeCallbacksStateChangedEv: Security-agreement 0x%p, Before callback",
				   pSecAgree));

        SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr,pSecAgreeMgr->pLogSrc, pTripleLock, &nestedLock, &currThreadId, &threadIdLockCount);

        pSecAgreeMgr->appEvHandlers.pfnSecAgreeStateChangedEvHandler(
												(RvSipSecAgreeHandle)pSecAgree,
												hAppOwner,
												eState,
												eReason);

        SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pTripleLock, nestedLock, currThreadId, threadIdLockCount);

        RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksStateChangedEv: Security-agreement 0x%p, After callback.",
					pSecAgree));
    }
	else
	{
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeCallbacksStateChangedEv: Application did not register to callback"));
	}
}

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
									OUT SecAgree**					         ppNewSecAgree)
{
   RvInt32                nestedLock        = 0;
   RvThreadId             currThreadId      = 0;
   RvInt32                threadIdLockCount = 0;
   RvStatus               rv = RV_OK;

	RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 "SecAgreeCallbacksReplaceSecAgreeEv - requesting the application to supply a replacement security-agreement for the existing sec-agree=%p, owner 0x%p",
				 pCurrSecAgree, pOwner->pOwner));

   // sanity checks
   if(pSecAgreeMgr == NULL || bReplace == NULL || ppNewSecAgree == NULL)
   {
      return RV_ERROR_NULLPTR;
   }

   // default decision is yes
   *bReplace      = RV_TRUE;
   *ppNewSecAgree = NULL;

   if(pSecAgreeMgr->appMgrEvHandlers.pfnSecAgreeMgrReplaceSecAgreeEvHandler != NULL)
   {
      RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
          "SecAgreeCallbacksReplaceSecAgreeEv: Owner 0x%p, Before callback",
          pOwner->pOwner));

      /* pCurrSecAgree is not lock therefore there is no need to unlock it */
      if (NULL != pOwnerLock)
      {
	      /* unlock the owner before calling application callback */
	      SipCommonUnlockBeforeCallback(pSecAgreeMgr->pLogMgr, pSecAgreeMgr->pLogSrc, pOwnerLock, &nestedLock, &currThreadId, &threadIdLockCount);
      }

        /* call application callback */
      rv = pSecAgreeMgr->appMgrEvHandlers.pfnSecAgreeMgrReplaceSecAgreeEvHandler(
										      (RvSipSecAgreeMgrHandle)pSecAgreeMgr,
										      pOwner,
										      hMsg,
										      (RvSipSecAgreeHandle)pCurrSecAgree,
										      eRole,
                                    bReplace,
										      (RvSipSecAgreeHandle*)ppNewSecAgree);

      if (NULL != pOwnerLock)
      {
	      /* lock the owner after returning from application callback */
	      SipCommonLockAfterCallback(pSecAgreeMgr->pLogMgr, pOwnerLock, nestedLock, currThreadId, threadIdLockCount);
      }

      RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			      "SecAgreeCallbacksReplaceSecAgreeEv: Owner 0x%p, After callback. bReplace=%d, New Security-agreement=0x%p",
			      pOwner->pOwner, *bReplace, *ppNewSecAgree));
   }
   else
   {
      RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			      "SecAgreeCallbacksReplaceSecAgreeEv: Application did not register to callback  --> use default bReplace=RV_TRUE"));
   }

	return rv;
}

#endif
/* SPIRENT_END */

#endif /*#ifdef RV_SIP_IMS_ON*/

