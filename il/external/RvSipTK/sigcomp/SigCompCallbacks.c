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
 *                              <SigCompCallbacks.c>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  May 2006
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"
#include "SigCompMgrObject.h"
#include "SigCompCallbacks.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#define UNLOCK_BEFORE_CALLBACK 0
#if UNLOCK_BEFORE_CALLBACK

static void SigCompUnlockBeforeCallback(IN SigCompMgr *pSigCompMgr);
static void SigCompReturnFromCallback(IN SigCompMgr *pSigCompMgr); 

#else /* #if UNLOCK_BEFORE_CALLBACK */ 

#define SigCompUnlockBeforeCallback(pSigCompMgr) 
#define SigCompReturnFromCallback(pSigCompMgr) 

#endif /* #if UNLOCK_BEFORE_CALLBACK */ 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SigCompCallbackCompressMsgEv
* ------------------------------------------------------------------------
* General: Call the pfnCompressionFunc callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pSigCompMgr      - Pointer to the SigComp manager.
*        pCompartment     - Pointer to the created compartment
*		 pCompressionInfo - Pointer to compression information structure 
*							(within the compartment context)
*        pMsgInfo         - Pointer to the message information structure
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV SigCompCallbackCompressMsgEv(
					IN    SigCompMgr               *pSigCompMgr,
					IN    SigCompCompartment	   *pCompartment,
					IN    RvSigCompCompressionInfo *pCompressionInfo,
					IN    RvSigCompMessageInfo     *pMsgInfo)
{
	RvStatus		    rv         = RV_OK;
 	RvSigCompAlgorithm *pAlgorithm = pCompartment->pCompressionAlg; 
			
	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackCompressMsgEv - Asks the application to compress a msg with algorithm %s (compartment=0x%p)",
		pAlgorithm->algorithmName,pCompartment));
		
	if (pAlgorithm->pfnCompressionFunc != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompressMsgEv: Compartment 0x%p, Algorithm %s, Before callback",
			pCompartment,pAlgorithm->algorithmName));
		
		SigCompUnlockBeforeCallback(pSigCompMgr);
		
		rv = pAlgorithm->pfnCompressionFunc(
						(RvSigCompMgrHandle)pSigCompMgr,
						(RvSigCompCompartmentHandle)pCompartment,
						pCompressionInfo,
						pMsgInfo,
						pAlgorithm,
						pCompartment->algContext.pData,
						0);
		
		SigCompReturnFromCallback(pSigCompMgr);
		
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompressMsgEv: Compartment 0x%p, Algorithm %s, After callback",
			pCompartment,pAlgorithm->algorithmName));
	}
	else
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompressMsgEv: Compartment 0x%p, Algorithm %s, Application did not registered to callback",
			pCompartment,pAlgorithm->algorithmName));

		return RV_ERROR_NOTSUPPORTED;
    }	

	return rv;
}

/***************************************************************************
* SigCompCallbackAlgorithmInitializedEv
* ------------------------------------------------------------------------
* General: Call the pfnAlgorithmInitializedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - Pointer to the SigComp manager.
*        pAlgorithm  - Pointer to the initialized algorithm data structure 
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackAlgorithmInitializedEv(
								IN SigCompMgr		  *pSigCompMgr,
								IN RvSigCompAlgorithm *pAlgorithm)
{
	RvSigCompAlgoEvHandlers *pAlgoEvHandlers; 

	if (pAlgorithm == NULL) 
	{
		RvLogExcep(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
			"SigCompCallbackAlgorithmInitializedEv - Got NULL algorithm pointer"));

        return;  /* Cannot move on if the algorithm equals NULL */ 
	}

	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackAlgorithmInitializedEv - Notifies the application of a newly initialized algo %s (0x%p)",
		pAlgorithm->algorithmName,pAlgorithm));
	
	pAlgoEvHandlers = &(pAlgorithm->evHandlers); 

	if (pAlgoEvHandlers->pfnAlgorithmInitializedEvHandler != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmInitializedEv: Algorithm %s, Before callback",
			pAlgorithm->algorithmName));

		SigCompUnlockBeforeCallback(pSigCompMgr);

		(*pAlgoEvHandlers).pfnAlgorithmInitializedEvHandler(
							(RvSigCompMgrHandle)pSigCompMgr,pAlgorithm);
		
		SigCompReturnFromCallback(pSigCompMgr);

		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmInitializedEv: Algorithm %s, After callback",
			pAlgorithm->algorithmName));
	}
	else
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmInitializedEv: Algorithm 0x%p, Application did not registered to callback",
			pAlgorithm));
    }
	
}

/***************************************************************************
* SigCompCallbackAlgorithmEndedEv
* ------------------------------------------------------------------------
* General: Call the pfnAlgorithmEndedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - Pointer to the SigComp manager.
*        pAlgorithm  - Pointer to the initialized algorithm data structure 
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackAlgorithmEndedEv(
								IN SigCompMgr		  *pSigCompMgr,
								IN RvSigCompAlgorithm *pAlgorithm)
{
	RvSigCompAlgoEvHandlers *pAlgoEvHandlers; 

	if (pAlgorithm == NULL) 
	{
		RvLogExcep(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
			"SigCompCallbackAlgorithmEndedEv - Got NULL algorithm pointer"));

        return;  /* Cannot move on if the algorithm equals NULL */ 
	}

	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackAlgorithmEndedEv - Notifies the application of an ended algo %s (0x%p)",
		pAlgorithm->algorithmName,pAlgorithm));
	
	pAlgoEvHandlers = &(pAlgorithm->evHandlers); 

	if (pAlgoEvHandlers->pfnAlgorithmEndedEvHandler != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmEndedEv: Algorithm %s, Before callback",pAlgorithm->algorithmName));

		SigCompUnlockBeforeCallback(pSigCompMgr);

		(*pAlgoEvHandlers).pfnAlgorithmEndedEvHandler(
							(RvSigCompMgrHandle)pSigCompMgr,pAlgorithm);
		
		SigCompReturnFromCallback(pSigCompMgr);

		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmEndedEv: Algorithm %s, After callback",pAlgorithm->algorithmName));
	}
	else
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackAlgorithmEndedEv: Algorithm %s, Application did not registered to callback",
			pAlgorithm->algorithmName));
    }	
}

/***************************************************************************
* SigCompCallbackCompartmentCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Pointer to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentCreatedEv(
								IN SigCompMgr		    *pSigCompMgr,
								IN SigCompCompartment   *pCompartment)
{
	RvSigCompAlgorithm      *pAlgorithm      = pCompartment->pCompressionAlg; 
	RvSigCompAlgoEvHandlers *pAlgoEvHandlers = &(pAlgorithm->evHandlers);

	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackCompartmentCreatedEv - Notifies the application of a newly created compartment 0x%p (algorithm %s)",
		pCompartment,pAlgorithm->algorithmName));
	
	if (pAlgoEvHandlers->pfnCompartmentCreatedEvHandler != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentCreatedEv: Compartment 0x%p, Before callback",pCompartment));

		SigCompUnlockBeforeCallback(pSigCompMgr);

		(*pAlgoEvHandlers).pfnCompartmentCreatedEvHandler(
									(RvSigCompMgrHandle)pSigCompMgr,
									(RvSigCompCompartmentHandle)pCompartment,
									pAlgorithm,
									pCompartment->algContext.pData);
		
		SigCompReturnFromCallback(pSigCompMgr);

		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentCreatedEv: Compartment 0x%p, After callback",pCompartment));
	}
	else
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentCreatedEv: Compartment 0x%p, Application did not registered to callback",
			pCompartment));
    }	
}

/***************************************************************************
* SigCompCallbackCompartmentDestructedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentDestructedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Handle to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentDestructedEv(
								IN SigCompMgr		    *pSigCompMgr,
								IN SigCompCompartment   *pCompartment)
{
	RvSigCompAlgorithm      *pAlgorithm      = pCompartment->pCompressionAlg; 
	RvSigCompAlgoEvHandlers *pAlgoEvHandlers = &(pAlgorithm->evHandlers);

	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackCompartmentDestructedEv - Notifies the application of a terminated compartment 0x%p (algorithm %s)",
		pCompartment,pAlgorithm->algorithmName));
	
	if (pAlgoEvHandlers->pfnCompartmentDestructedEvHandler != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentDestructedEv: Compartment 0x%p, Before callback",pCompartment));

		SigCompUnlockBeforeCallback(pSigCompMgr);

		(*pAlgoEvHandlers).pfnCompartmentDestructedEvHandler(
									(RvSigCompMgrHandle)pSigCompMgr,
									(RvSigCompCompartmentHandle)pCompartment,
									pAlgorithm,
									pCompartment->algContext.pData);
		
		SigCompReturnFromCallback(pSigCompMgr);

		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentDestructedEv: Compartment 0x%p, After callback",pCompartment));
	}
	else
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentDestructedEv: Compartment 0x%p, Application did not registered to callback",
			pCompartment));
    }	
}

/***************************************************************************
* SigCompCallbackCompartmentOnPeerMsgEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentOnPeerMsgEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Handle to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentOnPeerMsgEv(
								IN SigCompMgr		        *pSigCompMgr,
								IN SigCompCompartment       *pCompartment,
								IN RvSigCompCompressionInfo *pCompInfo)
{
	RvSigCompAlgorithm      *pAlgorithm      = pCompartment->pCompressionAlg; 
	RvSigCompAlgoEvHandlers *pAlgoEvHandlers = &(pAlgorithm->evHandlers);

	RvLogDebug(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
		"SigCompCallbackCompartmentOnPeerMsgEv - Notifies the application of a compressed msg, related to compartment 0x%p (algorithm %s)",
		pCompartment,pAlgorithm->algorithmName));
	
	if (pAlgoEvHandlers->pfnCompartmentOnPeerMsgEvHandler != NULL)
	{
		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentOnPeerMsgEv: Compartment 0x%p, Before callback",pCompartment));

		SigCompUnlockBeforeCallback(pSigCompMgr);

		(*pAlgoEvHandlers).pfnCompartmentOnPeerMsgEvHandler(
									(RvSigCompMgrHandle)pSigCompMgr,
									(RvSigCompCompartmentHandle)pCompartment,
									pAlgorithm,
									pCompInfo,
									pCompartment->algContext.pData);
		
		SigCompReturnFromCallback(pSigCompMgr);

		RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentOnPeerMsgEv: Compartment 0x%p, After callback",pCompartment));
	}
	else
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"SigCompCallbackCompartmentOnPeerMsgEv: Compartment 0x%p, Application did not registered to callback",
			pCompartment));
    }	
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if UNLOCK_BEFORE_CALLBACK
/***************************************************************************
* SigCompUnlockBeforeCallback
* ------------------------------------------------------------------------
* General:Prepare a handle for use in a callback to the app.
*         This function will make sure the handle is unlocked 
* Return Value: void.
* ------------------------------------------------------------------------
* Arguments:
* input: pSigCompMgr - Pointer to the SigComp manager
***************************************************************************/
static void SigCompUnlockBeforeCallback(IN SigCompMgr *pSigCompMgr)
{
	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
}

/***************************************************************************
* SigCompReturnFromCallback
* ------------------------------------------------------------------------
* General:- This function will ensure that the element is locked again
*           with the specified number of times when the server returns from
*           callback.
*           Since we are locking back the object lock, the function restore
*           the tripleLock->objLockThreadId.
* ------------------------------------------------------------------------
* Arguments:
* input: pSigCompMgr - Pointer to the SigComp manager
***************************************************************************/
static void SigCompReturnFromCallback(IN SigCompMgr *pSigCompMgr)
{
	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
}
#endif /* #if UNLOCK_BEFORE_CALLBACK */ 


