
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
 *                              <CompartmentObject.c>
 *
 * This file defines the compartment object, attributes and interface
 * functions. The comparment represents a Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transacions that is identify by a compartment ID when sending request
 * through a compressor entity.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvmutex.h"
#include "_SipCommonCore.h"
#include "CompartmentObject.h"
#include "_SipCommonConversions.h"

#ifdef RV_SIGCOMP_ON
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

static RvStatus RVCALLCONV CompartmentLock(IN Compartment        *pCompartment,
                                           IN CompartmentMgr     *pMgr,
                                           IN SipTripleLock      *tripleLock,
                                           IN RvInt32            identifier);

static void RVCALLCONV CompartmentUnLock(IN  RvLogMgr *pLogMgr,
                                         IN  RvMutex  *hLock);
#else
#define CompartmentLock(a,b,c,d) (RV_OK)
#define CompartmentUnLock(a,b)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CompartmentInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new compartment object. Sets all values to their
 *          initial values.
 * Return Value: RV_ERROR_UNKNOWN - Failed to initialize the compartment.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the new Compartment.
 *            pMgr         - pointer to the manager of this call.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentInitialize(
                            IN  Compartment     *pCompartment,
                            IN  CompartmentMgr  *pMgr)
{
    /* Note: avoid initiation of compartment to all zeros (memset) because hLock */
    /* value should remains unchanged since stack initialization.                */

    pCompartment->pMgr            = pMgr;
    pCompartment->referenceCount  = 0;
    pCompartment->hInternalComp   = NULL;
	pCompartment->hAppCompartment = NULL;
	pCompartment->strAlgoName[0]  = '\0';
	pCompartment->pTripleLock     = &(pCompartment->compTripleLock);
	pCompartment->bIsApproved     = RV_FALSE;
	pCompartment->hPageForUrn     = NULL_PAGE;
	pCompartment->offsetInPage    = 0;
	pCompartment->compType        = RVSIP_COMPARTMENT_TYPE_UNDEFINED;
	pCompartment->bIsHashed       = RV_FALSE;
	pCompartment->bIsInCreatedEv  = RV_FALSE;
	pCompartment->bIsSrc          = RV_FALSE;
	pCompartment->hMsg            = NULL;
	
	    /* Set unique identifier only on initialization success */
    RvRandomGeneratorGetInRange(pCompartment->pMgr->pSeed, RV_INT32_MAX,
        (RvRandom*)&pCompartment->compartmentUniqueIdentifier);

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CompartmentInitialize - Compartment 0x%p initialized successfully", pCompartment));

    return RV_OK;
}

/***************************************************************************
 * CompartmentAttach
 * ------------------------------------------------------------------------
 * General: Attaching to an existing compartment.
 *
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentAttach(IN Compartment *pCompartment,
                                      IN void*        hOwner)
{
    pCompartment->referenceCount += 1;

    RvLogDebug(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentAttach - Compartment 0x%p, owner=%p: count was incremented up to %u ()",
                  pCompartment,hOwner,pCompartment->referenceCount));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hOwner);
#endif

    return RV_OK;
}

/***************************************************************************
 * CompartmentDetach
 * ------------------------------------------------------------------------
 * General: Detaching from an existing compartment. In case that reference
 *          count becomes zero the compartment is terminated automatically,
 *          since the compartment is not used by any of the stack objects or
 *          by the application above.
 *
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentDetach(IN Compartment *pCompartment,
                                      IN void*        hOwner)
{
    RvStatus              rv = RV_OK;

    /* Check if there are any elements that are part of the the compartment */
    /* If the last element wishes to detach from the compartment, then it's */
    /* terminated automatically.                                            */
    if (1 == pCompartment->referenceCount)
    {
        /* Terminating the current compartment */
        rv = CompartmentTerminate(pCompartment);
        if (RV_OK == rv)
        {
            RvLogDebug(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
             "CompartmentDetach - Compartment 0x%p is terminated (hOwner=%p)",
             pCompartment, hOwner));
        }
        else
        {
            RvLogDebug(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
             "CompartmentDetach - Compartment 0x%p failed to be terminated (hOwner=%p)",
             pCompartment, hOwner));
        }

    }
    else if (1 < pCompartment->referenceCount)
    {
        pCompartment->referenceCount -= 1;
        RvLogDebug(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentDetach - Compartment 0x%p, owner=%p: count was decremented to %u",
                  pCompartment,hOwner,pCompartment->referenceCount));
    }
    else
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentDetach - Compartment 0x%p, owner=%p: count is already %u, can't be detached",
                  pCompartment,hOwner,pCompartment->referenceCount));
        return RV_ERROR_UNKNOWN;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hOwner);
#endif

    return rv;
}


/***************************************************************************
 * CompartmentTerminate
 * ------------------------------------------------------------------------
 * General: Destructs a Compartment object, Free the resources taken
 *          by it and remove it from the manager compartments list.
 * Return Value: RV_OK - If the compartment is destructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the terminated  Compartment
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentTerminate(
                           IN Compartment *pCompartment)
{
    RvStatus        rv   = RV_OK;
    CompartmentMgr *pMgr = pCompartment->pMgr;

    pCompartment->referenceCount = 0;

    /* The  Compartment should be removed from the manager compartments list and hash */

    LOCK_MGR(pMgr);
	if (pCompartment->bIsHashed != RV_FALSE)
	{
		HASH_RemoveElement(pMgr->hCompartmentHash, pCompartment->hashElemPtr);
	}
    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList,
        (RLIST_ITEM_HANDLE)pCompartment);
    UNLOCK_MGR(pMgr);

    if (pCompartment->hInternalComp != NULL)
    {
        rv = RvSigCompCloseCompartment(pCompartment->pMgr->hSigCompMgr,
                                       pCompartment->hInternalComp);
    }
    else
    {
        RvLogInfo(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentTerminate - Compartment 0x%p is detached from internal compartment 0x%p",
                  pCompartment,pCompartment->hInternalComp));
    }

    if (rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentTerminate - Compartment 0x%p failed to close internal compartment 0x%p",
                  pCompartment,pCompartment->hInternalComp));
    }
    else
    {
        RvLogDebug(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
                  "CompartmentTerminate - Compartment 0x%p closed internal 0x%p",
                  pCompartment,pCompartment->hInternalComp));
    }

	pCompartment->compartmentUniqueIdentifier = 0;
	pCompartment->bIsApproved = RV_FALSE;
	pCompartment->hInternalComp = NULL;

    return rv;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * CompartmentLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks compartment according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
RvStatus RVCALLCONV CompartmentLockAPI(IN  Compartment*   pCompartment)
{
    RvStatus           retCode;
    SipTripleLock     *tripleLock;
    CompartmentMgr    *pMgr;
    RvInt32            identifier;

    if (pCompartment == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {
        RvMutexLock(&pCompartment->compTripleLock.hLock,pCompartment->pMgr->pLogMgr);
        pMgr       = pCompartment->pMgr;
        tripleLock = pCompartment->pTripleLock;
        identifier = pCompartment->compartmentUniqueIdentifier;
        RvMutexUnlock(&pCompartment->compTripleLock.hLock,pCompartment->pMgr->pLogMgr);


        if (tripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CompartmentLockAPI - Locking compartment 0x%p: Triple Lock 0x%p", pCompartment,
            tripleLock));

        retCode = CompartmentLock(pCompartment,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pCompartment->pTripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CompartmentLockAPI - Locking compartment 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
            pCompartment,tripleLock));
        CompartmentUnLock(pMgr->pLogMgr,&tripleLock->hLock);

    } 

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        CompartmentUnLock(pCompartment->pMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }

    tripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock,pCompartment->pMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }
    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CompartmentLockAPI - Locking compartment 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pCompartment, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}

/************************************************************************************
 * CompartmentUnlockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Compartment according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
void RVCALLCONV CompartmentUnLockAPI(IN  Compartment*   pCompartment)
{
    SipTripleLock        *tripleLock;
    CompartmentMgr       *pMgr;
    RvInt32               lockCnt = 0;

    if (pCompartment == NULL)
    {
        return;
    }

    pMgr = pCompartment->pMgr;
    tripleLock = pCompartment->pTripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock,pCompartment->pMgr->pLogMgr,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CompartmentUnLockAPI - UnLocking compartment 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pCompartment, tripleLock, tripleLock->apiCnt,lockCnt));


    if (tripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                   "CompartmentUnLockAPI - UnLocking call 0x%p: Triple Lock 0x%p, apiCnt=%d",
                   pCompartment, tripleLock, tripleLock->apiCnt));
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,NULL);
    }

    tripleLock->apiCnt--;
    /*reset the thread id in the counter that set it previously*/
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }

    CompartmentUnLock(pCompartment->pMgr->pLogMgr, &tripleLock->hLock);
}

/************************************************************************************
 * CompartmentLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks compartment according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment
***********************************************************************************/
RvStatus RVCALLCONV CompartmentLockMsg(IN  Compartment*   pCompartment)
{
    RvStatus          retCode      = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *tripleLock;
    CompartmentMgr*       pMgr;
    RvInt32           identifier;

    if (pCompartment == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pCompartment->compTripleLock.hLock,pCompartment->pMgr->pLogMgr);
    tripleLock = pCompartment->pTripleLock;
    pMgr       = pCompartment->pMgr;
    identifier = pCompartment->compartmentUniqueIdentifier;
    RvMutexUnlock(&pCompartment->compTripleLock.hLock,pCompartment->pMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "CompartmentLockMsg - Exiting without locking Compartment 0x%p: Triple Lock 0x%p, apiCnt=%d already locked",
                   pCompartment, tripleLock, tripleLock->apiCnt));

        return RV_OK;
    }

    RvMutexLock(&tripleLock->hProcessLock,pCompartment->pMgr->pLogMgr);

    for(;;)
    {
        retCode = CompartmentLock(pCompartment,pMgr, tripleLock,identifier);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pCompartment->pMgr->pLogMgr);
          if (didILockAPI)
          {
            RvSemaphorePost(&tripleLock->hApiLock,NULL);
          }

          return retCode;
        }

        if (tripleLock->apiCnt == 0)
        {
            if (didILockAPI)
            {
              RvSemaphorePost(&tripleLock->hApiLock,NULL);
            }
            break;
        }

        CompartmentUnLock(pCompartment->pMgr->pLogMgr, &tripleLock->hLock);

        retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pCompartment->pMgr->pLogMgr);
          return retCode;
        }

        didILockAPI = RV_TRUE;
    } 

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CompartmentLockMsg - Locking call 0x%p: Triple Lock 0x%p exiting function",
             pCompartment, tripleLock));
    return retCode;
}

/************************************************************************************
 * CompartmentUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks compartment according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
void RVCALLCONV CompartmentUnLockMsg(IN  Compartment*   pCompartment)
{
    SipTripleLock   *tripleLock;
    CompartmentMgr  *pMgr;
    RvThreadId       currThreadId = RvThreadCurrentId();

    if (pCompartment == NULL)
    {
        return;
    }

    pMgr       = pCompartment->pMgr;
    tripleLock = pCompartment->pTripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "CompartmentUnLockMsg - Exiting without unlocking Compartment 0x%p: Triple Lock 0x%p already locked",
                   pCompartment, tripleLock));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CompartmentUnLockMsg - UnLocking Compartment 0x%p: Triple Lock 0x%p", pCompartment,
             tripleLock));

    CompartmentUnLock(pCompartment->pMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock,pCompartment->pMgr->pLogMgr);
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/***************************************************************************
 * CompartmentLock
 * ------------------------------------------------------------------------
 * General: Locks compartment.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to Compartment to be locked
 *            pMgr         - pointer to Compartment manager
 *            hLock        - compartment lock
 *            identifier   - the compartment's unique identifier
 ***************************************************************************/
static RvStatus RVCALLCONV CompartmentLock(IN Compartment        *pCompartment,
                                           IN CompartmentMgr     *pMgr,
                                           IN SipTripleLock      *tripleLock,
                                           IN RvInt32            identifier)
{


    if ((pCompartment == NULL) || (tripleLock == NULL) || (pMgr == NULL))
    {
        return RV_ERROR_BADPARAM;
    }


    RvMutexLock(&tripleLock->hLock,pCompartment->pMgr->pLogMgr); /*lock the compartment */

    if (pCompartment->compartmentUniqueIdentifier != identifier ||
        pCompartment->compartmentUniqueIdentifier == 0)
    {
        /*The Compartment is invalid*/
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CompartmentLock - Compartment 0x%p: Compartment object was destructed", pCompartment));
        RvMutexUnlock(&tripleLock->hLock,pCompartment->pMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}

/***************************************************************************
 * CompartmentUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks a given compartment's lock.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLogMgr  - The log mgr pointer
 *            hLock    - A lock to unlock.
 ***************************************************************************/
static void RVCALLCONV CompartmentUnLock(IN  RvLogMgr *pLogMgr,
                                         IN  RvMutex  *hLock)
{
   RvMutexUnlock(hLock,pLogMgr); /*unlock the compartment*/
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */


#endif /* RV_SIGCOMP_ON */

