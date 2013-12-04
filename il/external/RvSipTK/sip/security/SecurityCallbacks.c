/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <SecurityCallbacks.c>
 *
 * The SecurityCallbacks.c file contains wrappers for application callbacks,
 * called by Security Module.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          February 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecurityCallbacks.h"
#include "_SipSecurityUtils.h"
#include "_SipCommonUtils.h"

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         SECURITY CALLBACK FUNCTIONS                       */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecurityCallbackSecObjStateChangeEv
 * ----------------------------------------------------------------------------
 * General: Calls the Security Object state change callback.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pSecObj - Pointer to the Security Object
 *        eState  - State
 *        eReason - State change reason
 *****************************************************************************/
void SecurityCallbackSecObjStateChangeEv(
                                    IN SecObj*                      pSecObj,
                                    IN RvSipSecObjState             eState,
                                    IN RvSipSecObjStateChangeReason eReason)
{
    RvSipSecEvHandlers* pAppSecEvHandlers = &pSecObj->pMgr->appEvHandlers;
    RvSipSecEvHandlers* pOwnerSecEvHandlers = &pSecObj->pMgr->ownerEvHandlers;
    SecMgr*             pSecMgr = pSecObj->pMgr;
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
	SipTripleLock*      pTripleLock = pSecObj->pTripleLock;
    RvThreadId          currThreadId;
    RvInt32             nestedLockCount;
    RvInt32             threadIdLockCount;
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

    /* Call internal callback only if there is internal owner */
    if (NULL != pOwnerSecEvHandlers->pfnSecObjStateChangedEvHandler &&
        NULL != pSecObj->pSecAgree)
    {
        RvSipAppSecObjHandle hSecAgree = (RvSipAppSecObjHandle)pSecObj->pSecAgree;

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStateChangeEv: SecObj %p - Before callback",
            pSecObj));

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        nestedLockCount = 0;
        SipCommonUnlockBeforeCallback(pSecMgr->pLogMgr,pSecMgr->pLogSrc,
            pTripleLock,&nestedLockCount,&currThreadId,&threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        pOwnerSecEvHandlers->pfnSecObjStateChangedEvHandler(
            (RvSipSecObjHandle)pSecObj, hSecAgree, eState, eReason);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        SipCommonLockAfterCallback(pSecMgr->pLogMgr, pTripleLock, 
            nestedLockCount, currThreadId, threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStateChangeEv: SecObj %p - After callback",
            pSecObj));
    }

    /* If state of the Security Object was changed while code was as a result of the
    owner's callback call, don't propagate it to the application: just return*/
    if (eState != pSecObj->eState)
    {
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                   "SecurityCallbackSecObjStateChangeEv: SecObj %p - state was changed when application had being handled callback: %s -> %s",
                   pSecObj, SipSecUtilConvertSecObjState2Str(eState),
                   SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return;
    }

    /* Call application callback, only if there is Application Owner */
    if (NULL != pAppSecEvHandlers->pfnSecObjStateChangedEvHandler &&
        NULL != pSecObj->hAppSecObj)
    {
        RvSipAppSecObjHandle hAppSecObj = pSecObj->hAppSecObj;

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStateChangeEv: SecObj %p - Before callback",
            pSecObj));

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        nestedLockCount = 0;
        SipCommonUnlockBeforeCallback(pSecMgr->pLogMgr,pSecMgr->pLogSrc,
            pTripleLock,&nestedLockCount,&currThreadId,&threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        pAppSecEvHandlers->pfnSecObjStateChangedEvHandler(
            (RvSipSecObjHandle)pSecObj, hAppSecObj, eState, eReason);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        SipCommonLockAfterCallback(pSecMgr->pLogMgr, pTripleLock, 
            nestedLockCount, currThreadId, threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStateChangeEv: SecObj %p - After callback",
            pSecObj));
    }
    else
    {
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStateChangeEv: SecObj %p - Application didn't register callback",
            pSecObj));
    }
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecMgr);
#endif

}

/******************************************************************************
 * SecurityCallbackSecObjStatusEv
 * ----------------------------------------------------------------------------
 * General: Calls the Security Object status callback.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pSecObj - Pointer to the Security Object
 *        eStatus - Status, to be provided through the callback
 *        pInfo   - Auxiliary info
 *****************************************************************************/
void SecurityCallbackSecObjStatusEv(
                                IN SecObj*           pSecObj,
                                IN RvSipSecObjStatus eStatus,
                                IN void*             pInfo)
{
    RvSipSecEvHandlers* pAppSecEvHandlers = &pSecObj->pMgr->appEvHandlers;
    RvSipSecEvHandlers* pOwnerSecEvHandlers = &pSecObj->pMgr->ownerEvHandlers;
    SecMgr*             pSecMgr = pSecObj->pMgr;
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
    SipTripleLock*      pTripleLock = pSecObj->pTripleLock;
    RvThreadId          currThreadId;
    RvInt32             nestedLockCount;
    RvInt32             threadIdLockCount;
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

    /* Call internal callback only if there is internal owner */
    if (NULL != pOwnerSecEvHandlers->pfnSecObjStatusEvHandler &&
        NULL != pSecObj->pSecAgree)
    {
        RvSipAppSecObjHandle hSecAgree = (RvSipAppSecObjHandle)pSecObj->pSecAgree;

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStatusEv: SecObj %p - Before callback",
            pSecObj));

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        nestedLockCount = 0;
        SipCommonUnlockBeforeCallback(pSecMgr->pLogMgr,pSecMgr->pLogSrc,
            pTripleLock,&nestedLockCount,&currThreadId,&threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        pOwnerSecEvHandlers->pfnSecObjStatusEvHandler(
            (RvSipSecObjHandle)pSecObj, hSecAgree, eStatus, pInfo);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        SipCommonLockAfterCallback(pSecMgr->pLogMgr, pTripleLock, 
            nestedLockCount, currThreadId, threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStatusEv: SecObj %p - After callback",
            pSecObj));
    }

    /* Call application callback only if there is Application Owner */
    if (NULL != pAppSecEvHandlers->pfnSecObjStatusEvHandler &&
        NULL != pSecObj->hAppSecObj)
    {
        RvSipAppSecObjHandle hAppSecObj = pSecObj->hAppSecObj;

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStatusEv: SecObj %p - Before callback",
            pSecObj));

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        nestedLockCount = 0;
        SipCommonUnlockBeforeCallback(pSecMgr->pLogMgr,pSecMgr->pLogSrc,
            pTripleLock,&nestedLockCount,&currThreadId,&threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        pAppSecEvHandlers->pfnSecObjStatusEvHandler(
            (RvSipSecObjHandle)pSecObj, hAppSecObj, eStatus, pInfo);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)    
        SipCommonLockAfterCallback(pSecMgr->pLogMgr, pTripleLock, 
            nestedLockCount, currThreadId, threadIdLockCount);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStatusEv: SecObj %p - After callback",
            pSecObj));
    }
    else
    {
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecurityCallbackSecObjStatusEv: SecObj %p - Application didn't register callback",
            pSecObj));
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecMgr);
#endif

}

#endif /*#ifdef RV_SIP_IMS_ON*/

