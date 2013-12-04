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
 *                      <SigCompStateHandlerObject.c>
 *
 * This file defines the SigComp state handler object,that holds all the
 * SigComp states resources including memory pool, pools of lists, locks and
 * more. Its main functionality is to manage the state list, to create new
 * states.
 * The file also contains Internal API functions of the SigComp State Handler Object.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Gil Keini                    20040114
 *********************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "SigCompStateHandlerObject.h"
#include "SigCompMgrObject.h"
#include "SigCompCommon.h"
#include <string.h> /* for memset & memcpy */
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/
const RvSigCompStateID nullStateID = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
/*-----------------------------------------------------------------------*/
/*                           HELPER FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/* HASH FUNCTION CALL BACKS */
static RvUint32 SigCompStateHashFunction(void *key);

static RvBool SigCompStateHashCompare(IN void *newHashElement,
                                      IN void *oldHashElement);


/*-----------------------------------------------------------------------*/
/*               SIGCOMP STATE HANDLER INTERNAL API                      */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompStateHandlerConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the state handler ,there
*          is one state handler per sigComp entity and therefore it is
*          called upon initialization of the sigComp module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSigCompMgr - A handle to the SigCompMgr context structure
*************************************************************************/
RvStatus  SigCompStateHandlerConstruct(IN RvSigCompMgrHandle hSigCompMgr)
{
    RvUint32     hashTableSize;
    RvUint32     numOfUserElements;
    RvStatus     crv; /* used for core return value */
    /* cast the handle to a pointer to SigCompMgr struct */
    SigCompMgr   *pSCMgr   = (SigCompMgr *) hSigCompMgr;

    if (NULL == pSCMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogEnter(pSCMgr->pLogSource ,(pSCMgr->pLogSource, 
		"SigCompStateHandlerConstruct(mgr=0x%x)",hSigCompMgr));
    
    /* initialize the stateHandlerMgr structure elements */
    /* and allocate memory pools */
    pSCMgr->stateHandlerMgr.pSCMgr        = pSCMgr;
    pSCMgr->stateHandlerMgr.pLogMgr       = pSCMgr->pLogMgr;
    pSCMgr->stateHandlerMgr.pLogSrc       = pSCMgr->pLogSource;
    pSCMgr->stateHandlerMgr.smallBufSize  = pSCMgr->smallBufPoolSize;
    pSCMgr->stateHandlerMgr.smallBufAmount= pSCMgr->smallPoolAmount;
    pSCMgr->stateHandlerMgr.smallBufRA    = RA_Construct(pSCMgr->smallBufPoolSize,
                                                       pSCMgr->smallPoolAmount,
                                                       NULL,
                                                       pSCMgr->pLogMgr,
                                                       "small buffers RA");
    if (NULL == pSCMgr->stateHandlerMgr.smallBufRA)
    {
        RvLogError(pSCMgr->pLogSource ,(pSCMgr->pLogSource ,
            "SigCompStateHandlerConstruct - failed to allocate smallBufRA"));
        /* memory pool allocation failed */
        return RV_ERROR_OUTOFRESOURCES;
    }

    pSCMgr->stateHandlerMgr.mediumBufSize   = pSCMgr->mediumBufPoolSize;
    pSCMgr->stateHandlerMgr.mediumBufAmount = pSCMgr->mediumPoolAmount;
    pSCMgr->stateHandlerMgr.mediumBufRA     = RA_Construct(pSCMgr->mediumBufPoolSize,
                                                         pSCMgr->mediumPoolAmount,
                                                         NULL,
                                                         pSCMgr->pLogMgr,
                                                         "medium buffers RA");
    if (NULL == pSCMgr->stateHandlerMgr.mediumBufRA)
    {
        RvLogError(pSCMgr->pLogSource ,(pSCMgr->pLogSource ,
            "SigCompStateHandlerConstruct - failed to allocate mediumBufRA"));
        /* memory pool allocation failed */
        RA_Destruct(pSCMgr->stateHandlerMgr.smallBufRA);
        pSCMgr->stateHandlerMgr.smallBufRA = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

    pSCMgr->stateHandlerMgr.largeBufSize = pSCMgr->largeBufPoolSize;
    pSCMgr->stateHandlerMgr.largeBufAmount = pSCMgr->largePoolAmount;
    pSCMgr->stateHandlerMgr.largeBufRA   = RA_Construct(pSCMgr->largeBufPoolSize,
                                                      pSCMgr->largePoolAmount,
                                                      NULL,
                                                      pSCMgr->pLogMgr,
                                                      "large buffers RA");
    if (NULL == pSCMgr->stateHandlerMgr.largeBufRA)
    {
        RvLogError(pSCMgr->pLogSource ,(pSCMgr->pLogSource ,
            "SigCompStateHandlerConstruct - failed to allocate largeBufRA"));
        /* memory pool allocation failed */
        RA_Destruct(pSCMgr->stateHandlerMgr.smallBufRA);
        RA_Destruct(pSCMgr->stateHandlerMgr.mediumBufRA);
        pSCMgr->stateHandlerMgr.smallBufRA  = NULL;
        pSCMgr->stateHandlerMgr.mediumBufRA = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* initialize the hash-table (key = 6 Bytes (MSB) from the stateID) */
    numOfUserElements = pSCMgr->smallPoolAmount +
                        pSCMgr->mediumPoolAmount +
                        pSCMgr->largePoolAmount; /* max number of states */
    hashTableSize = numOfUserElements;
    pSCMgr->stateHandlerMgr.hHashTable =  HASH_Construct(hashTableSize,
                                                       numOfUserElements,
                                                       SigCompStateHashFunction,
                                                       sizeof(SigCompState *),
                                                       sizeof(SigCompMinimalStateID),
                                                       pSCMgr->pLogMgr,
                                                       "SigCompStateHandler statesDB");
    if (NULL == pSCMgr->stateHandlerMgr.hHashTable)
    {
        RvLogError(pSCMgr->pLogSource ,(pSCMgr->pLogSource ,
            "SigCompStateHandlerConstruct - failed to allocate Hash table"));
        /* Hash table allocation failed */
        RA_Destruct(pSCMgr->stateHandlerMgr.smallBufRA);
        RA_Destruct(pSCMgr->stateHandlerMgr.mediumBufRA);
        RA_Destruct(pSCMgr->stateHandlerMgr.largeBufRA);
        pSCMgr->stateHandlerMgr.smallBufRA  = NULL;
        pSCMgr->stateHandlerMgr.mediumBufRA = NULL;
        pSCMgr->stateHandlerMgr.largeBufRA  = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* initialize an RA of state structures */
    pSCMgr->stateHandlerMgr.statesRA = RA_Construct(sizeof(SigCompState),
                                                  numOfUserElements,
                                                  NULL,
                                                  pSCMgr->pLogMgr,
                                                  "state structures RA");
    if (NULL == pSCMgr->stateHandlerMgr.statesRA)
    {
        RvLogError(pSCMgr->pLogSource ,(pSCMgr->pLogSource ,
            "SigCompStateHandlerConstruct - failed to allocate statesRA"));
        /* memory pool allocation failed */
        RA_Destruct(pSCMgr->stateHandlerMgr.smallBufRA);
        RA_Destruct(pSCMgr->stateHandlerMgr.mediumBufRA);
        RA_Destruct(pSCMgr->stateHandlerMgr.largeBufRA);
        HASH_Destruct(pSCMgr->stateHandlerMgr.hHashTable);
        pSCMgr->stateHandlerMgr.smallBufRA  = NULL;
        pSCMgr->stateHandlerMgr.mediumBufRA = NULL;
        pSCMgr->stateHandlerMgr.largeBufRA  = NULL;
        pSCMgr->stateHandlerMgr.hHashTable  = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* construct the lock/mutex of the manager itself */
    crv = RvMutexConstruct(pSCMgr->pLogMgr, &pSCMgr->stateHandlerMgr.lock);
    if (RV_OK != crv) 
    {
        /* failed to construct lock in the state element */
        RvLogError(pSCMgr->pLogSource,(pSCMgr->pLogSource,
            "SigCompStateHandlerConstruct - failed to construct lock of the manager (crv=%d)",
            crv));
        SigCompStateHandlerDestruct(&pSCMgr->stateHandlerMgr);
        return RV_ERROR_UNKNOWN;
    }
    pSCMgr->stateHandlerMgr.pLock = &pSCMgr->stateHandlerMgr.lock;
    
    /* reset usage statistics */
    RA_ResetMaxUsageCounter(pSCMgr->stateHandlerMgr.statesRA);

    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompStateHandlerConstruct - object constructed successfully (pSHMgr=0x%X)",
        &pSCMgr->stateHandlerMgr));
    RvLogLeave(pSCMgr->pLogSource,(pSCMgr->pLogSource,
		"SigCompStateHandlerConstruct(mgr=0x%x)",hSigCompMgr));
    return RV_OK;
} /* end of SigCompStateHandlerConstruct */



/*************************************************************************
* SigCompStateHandlerDestruct
* ------------------------------------------------------------------------
* General: destructor of the state handler, will be called
*
*          when destructing the sigComp module
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*************************************************************************/
void  SigCompStateHandlerDestruct(IN SigCompStateHandlerMgr *pSHMgr)
{
    RvUint32     maxStates;
    RvUint32     elementSize; /* dummy variable */
    RvUint32     counter;
    RvStatus     rv;
    RvStatus     crv;         /* to be used for core return value */
    RvInt32      lockCount;
    RA_Element   raElement;
    
    lockCount = 0;
    if (NULL == pSHMgr)
    {
        return;
    }

    if (pSHMgr->pLogSrc != NULL) 
    {
        RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
			"SigCompStateHandlerDestruct(mgr=0x%x)",pSHMgr));
    }
    if (NULL != pSHMgr->pLock)
    {
        RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);
    }

    if (NULL != pSHMgr->statesRA) 
    {
        RA_GetAllocationParams(pSHMgr->statesRA, &elementSize, &maxStates);
        /* loop to deallocate all non-vacant states from RA */
        for (counter = 0; counter < maxStates; counter++)
        {
            /* get state element by order */
            raElement = RA_GetAnyElement(pSHMgr->statesRA, counter);
            if (NULL == raElement) 
            {
                /* error in getting state */
                if (NULL != pSHMgr->pLock)
                {
                    RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
                }
                return;
            }
            if (!RA_ElemPtrIsVacant(pSHMgr->statesRA, raElement))
            {
                /* deallocate the state (and leave the lock "intact") */
                rv = RA_DeAllocByPtr(pSHMgr->statesRA, raElement);
                if (rv != RV_OK)
                {
                    /* failed to deallocate from RA */
                    if (NULL != pSHMgr->pLock)
                    {
                        RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
                    }
                    return;
                }        
            }
        }
    }
    
    /* free the hash table */
    if (NULL != pSHMgr->hHashTable)
    {
        HASH_Destruct(pSHMgr->hHashTable);
    }
    /* free state memory pools */
    if (NULL != pSHMgr->smallBufRA)
    {
        RA_Destruct(pSHMgr->smallBufRA);
    }
    if (NULL != pSHMgr->mediumBufRA)
    {
        RA_Destruct(pSHMgr->mediumBufRA);
    }
    if (NULL != pSHMgr->largeBufRA)
    {
        RA_Destruct(pSHMgr->largeBufRA);
    }
    if (NULL != pSHMgr->statesRA)
    {
        RA_Destruct(pSHMgr->statesRA);
    }

    /* release & destruct the lock/mutex of the manager itself */
    if (NULL != pSHMgr->pLock)
    {
        crv = RvMutexRelease(pSHMgr->pLock, pSHMgr->pLogMgr, &lockCount);
        if (RV_OK != crv) 
        {
            /* failed to release lock of the manager */
            return;
        }
        /* destruct the lock/mutex */
        crv = RvMutexDestruct(pSHMgr->pLock, pSHMgr->pLogMgr);
        if (RV_OK != crv) 
        {
            /* failed to destruct lock of the manager */
            return;
        }
        pSHMgr->pLock = NULL;
    }

    if (pSHMgr->pLogSrc != NULL) 
    {
        RvLogInfo(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
            "SigCompStateHandlerDestruct - object destructed (pSHMger=0x%X)",
            pSHMgr));
        RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
			"SigCompStateHandlerDestruct(mgr=0x%x)",pSHMgr));
    }
    
    return;
} /* end of SigCompStateHandlerDestruct */

/*************************************************************************
* SigCompStateHandlerGetResources
* ------------------------------------------------------------------------
* General: retrieves the resource usage of the StateHandler
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pSHMgr - a pointer to the state handler
*
* Output:    *pResources - a pointer a resource structure
*************************************************************************/
RvStatus SigCompStateHandlerGetResources(IN  SigCompStateHandlerMgr         *pSHMgr,
                                         OUT RvSigCompStateHandlerResources *pResources)
{
    RV_HASH_Resource  hashResources;
    RV_Resource       rvResource;
    RvSigCompResource *pSGResource;
    

    if ((NULL == pSHMgr) ||
        (NULL == pResources))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);
   
    RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerGetResources(mgr=0x%x)",pSHMgr));
	
    /* clear the resource report */
    memset(pResources,0,sizeof(RvSigCompStateHandlerResources));

    /* get the hash table resource status */
    if (NULL != pSHMgr->hHashTable)
    {
        HASH_GetResourcesStatus(pSHMgr->hHashTable,&hashResources);
        pSGResource = &pResources->hashTableKeys;
        pSGResource->currNumOfUsedElements = hashResources.keys.numOfUsed;
        pSGResource->maxUsageOfElements = hashResources.keys.maxUsage;
        pSGResource->numOfAllocatedElements = hashResources.keys.maxNumOfElements;
        pSGResource = &pResources->hashTableElements;
        pSGResource->currNumOfUsedElements = hashResources.elements.numOfUsed;
        pSGResource->maxUsageOfElements = hashResources.elements.maxUsage;
        pSGResource->numOfAllocatedElements = hashResources.elements.maxNumOfElements;
    }

    /* get the state structs RA resource status */
    if (NULL != pSHMgr->statesRA)
    {
        RA_GetResourcesStatus(pSHMgr->statesRA,&rvResource);
        pSGResource = &pResources->stateStructsRA;
        pSGResource->currNumOfUsedElements = rvResource.numOfUsed;
        pSGResource->maxUsageOfElements = rvResource.maxUsage;
        pSGResource->numOfAllocatedElements = rvResource.maxNumOfElements;
    }

    /* get the small buffers RA resource status */
    if (NULL != pSHMgr->smallBufRA)
    {
        RA_GetResourcesStatus(pSHMgr->smallBufRA,&rvResource);
        pSGResource = &pResources->smallPool;
        pSGResource->currNumOfUsedElements = rvResource.numOfUsed;
        pSGResource->maxUsageOfElements = rvResource.maxUsage;
        pSGResource->numOfAllocatedElements = rvResource.maxNumOfElements;
    }

    /* get the medium buffers RA resource status */
    if (NULL != pSHMgr->mediumBufRA)
    {
        RA_GetResourcesStatus(pSHMgr->mediumBufRA,&rvResource);
        pSGResource = &pResources->mediumPool;
        pSGResource->currNumOfUsedElements = rvResource.numOfUsed;
        pSGResource->maxUsageOfElements = rvResource.maxUsage;
        pSGResource->numOfAllocatedElements = rvResource.maxNumOfElements;
    }

    /* get the large buffers RA resource status */
    if (NULL != pSHMgr->largeBufRA)
    {
        RA_GetResourcesStatus(pSHMgr->largeBufRA,&rvResource);
        pSGResource = &pResources->largePool;
        pSGResource->currNumOfUsedElements = rvResource.numOfUsed;
        pSGResource->maxUsageOfElements = rvResource.maxUsage;
        pSGResource->numOfAllocatedElements = rvResource.maxNumOfElements;
    }

    RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
    
    RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerGetResources(mgr=0x%x)",pSHMgr));
    return(RV_OK);
} /* end of SigCompStateHandlerGetResources */

/*************************************************************************
* SigCompStateHandlerCreateState
* ------------------------------------------------------------------------
* General: creates a new state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*            requestedSize - the requested size of the state in bytes
*
* Output:   **pState - a pointer to the this (empty) state in memory
*************************************************************************/
RvStatus  SigCompStateHandlerCreateState(IN  SigCompStateHandlerMgr *pSHMgr,
                                         IN  RvUint32               requestedSize,
                                         OUT SigCompState           **ppState)
{
    RvStatus     crv;
    HRA          raPool;
    RvUint32     bufSize;
    RvInt32      freeElements;
    RvUint8      *pDataBuffer  = NULL;
    SigCompState *pState      = NULL;
    
    if ((NULL == pSHMgr) ||
        (NULL == ppState))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerCreateState(mgr=0x%x)",pSHMgr));

    /* lock before modifying resources */
    RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);

    /* check if a state can be allocated */
    freeElements = RA_NumOfFreeElements(pSHMgr->statesRA);
    if (freeElements == 0)
    {
        RvLogError(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
            "SigCompStateHandlerCreateState - no free states left to allocate"));
        /* no free states left to allocate */
        RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
        return(RV_ERROR_OUTOFRESOURCES);
    }

    /* find out from which RA pool to allocate state-data-buffer, according to size and availability */
    if ((requestedSize <= pSHMgr->smallBufSize) &&
        ((freeElements = RA_NumOfFreeElements(pSHMgr->smallBufRA)) > 0))
    {
        /* allocate from pool with small buffers */
        raPool = pSHMgr->smallBufRA;
        bufSize = pSHMgr->smallBufSize;
    }
    else if ((requestedSize <= pSHMgr->mediumBufSize) &&
             ((freeElements = RA_NumOfFreeElements(pSHMgr->mediumBufRA)) > 0))
    {
        /* allocate from pool with medium buffers */
        raPool = pSHMgr->mediumBufRA;
        bufSize = pSHMgr->mediumBufSize;
    }
    else if ((requestedSize <= pSHMgr->largeBufSize) &&
             ((freeElements = RA_NumOfFreeElements(pSHMgr->largeBufRA)) > 0))
    {
        /* allocate from pool with large buffers */
        raPool = pSHMgr->largeBufRA;
        bufSize = pSHMgr->largeBufSize;
    }
    else
    {
        if (requestedSize > pSHMgr->largeBufSize)
        {
         /*
        requested size is too large for current pools
        it is recommended to check the relevant configuration parameters
        */
           RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerCreateState - requested size is too large for current pools (size=%d)",
                requestedSize));
           RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
           return(RV_ERROR_BADPARAM);
        }
        else
        {
            /* out of resources */
            RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerCreateState - out of resources, request %d",requestedSize));
            RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
            return(RV_ERROR_OUTOFRESOURCES);
        }
    } /* end of block to determine the data-buffer allocation pool */

    /* allocate a state structure from the pool */
    crv = RA_Alloc(pSHMgr->statesRA,(RA_Element *)&pState);
    if (RV_OK != crv)
    {
        RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
		/* failed to allocate a state */
        RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerCreateState - failed to allocate a state (crv=%d)",
            crv));
		return(CCS2SCS(crv));
    }
    /* allocate a buffer for the state data from the selected (above) pool */
    crv = RA_Alloc(raPool,(RA_Element *)&pDataBuffer);
    RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
    if (RV_OK != crv)
    {
        /* failed to allocate a data buffer */
        RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerCreateState - failed to allocate a data buffer (crv=%d)",
            crv));
        RA_DeAlloc(pSHMgr->statesRA,(RA_Element)pState);
        return(CCS2SCS(crv));
    }

    /* fill the state's variables */
    pState->dataBufRA           = raPool;
    pState->statesRA            = pSHMgr->statesRA;
    pState->stateBufSize        = bufSize;
    pState->stateDataSize       = (RvUint16) requestedSize;
    pState->pData               = pDataBuffer;
    pState->retentionPriority   = 0;
    pState->isUsed              = 0;
    pState->stateAddress        = 0;
    pState->stateInstruction    = 0;
    pState->minimumAccessLength = 6;
    memset(pState->stateID,0,sizeof(RvSigCompStateID));

    *ppState = pState;

    RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerCreateState(mgr=0x%x)",pSHMgr));
    return(RV_OK);
} /* end of SigCompStateHandlerCreateState */

/*************************************************************************
* SigCompStateHandlerCreateStateForDictionary
* ------------------------------------------------------------------------
* General: creates a new state for a dictionary.
*           The difference from the above "normal" creteState function
*           is that the dictionary-state data pointer is to an
*           "external" memory which is handled by the application
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*            requestedSize - the requested size of the state in bytes
*
* Output:   **pState - a pointer to the this (empty) state in memory
*************************************************************************/
RvStatus  SigCompStateHandlerCreateStateForDictionary(
                                         IN  SigCompStateHandlerMgr *pSHMgr,
                                         IN  RvUint32               requestedSize,
                                         OUT SigCompState           **ppState)
{
    RvStatus     crv;
    RvInt32      freeElements;
    SigCompState *pState      = NULL;
    
    if ((NULL == pSHMgr) ||
        (NULL == ppState))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerCreateStateForDictionary(mgr=0x%x)",pSHMgr));

    /* lock before modifying resources */
    RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);

    /* check if a state can be allocated */
    freeElements = RA_NumOfFreeElements(pSHMgr->statesRA);
    if (freeElements == 0)
    {
        RvLogError(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
            "SigCompStateHandlerCreateStateForDictionary - no free states left to allocate"));
        /* no free states left to allocate */
        RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
        return(RV_ERROR_OUTOFRESOURCES);
    }

    /* allocate a state from the pool */
    crv = RA_Alloc(pSHMgr->statesRA,(RA_Element *)&pState);
    RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
    if (RV_OK != crv)
    {
        /* failed to allocate a state */
        RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerCreateStateForDictionary - failed to allocate a state (crv=%d)",
            crv));
        return(CCS2SCS(crv));
    }

    /* fill the state's variables */
    pState->dataBufRA			= NULL;
    pState->statesRA			= pSHMgr->statesRA;
    pState->stateBufSize		= requestedSize;
    pState->stateDataSize		= (RvUint16) requestedSize;
    pState->pData				= NULL;
    pState->retentionPriority	= 0;
    pState->isUsed				= 0;
    pState->stateAddress		= 0;
    pState->stateInstruction	= 0;
    pState->minimumAccessLength = 6;
    memset(pState->stateID,0,sizeof(RvSigCompStateID));

    *ppState = pState;

    RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerCreateStateForDictionary(mgr=0x%x)",pSHMgr));
    return(RV_OK);
} /* end of SigCompStateHandlerCreateStateForDictionary */


/*************************************************************************
* SigCompStateHandlerSaveState
* ------------------------------------------------------------------------
* General: saves the given state.
*           Note the usage of pState !
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pfnComparator - comparator function for UDVM cyclic buffer
*                            pass NULL to use memcmp (ansi function)
*           *pComparatorCtx - comparator function context
*                            pass NULL when using memcmp
*           **ppState - a pointer to the state structure.
*                       if the state already exists, the new state will be deleted
*                       and a pointer to the existing (identical) state will be returned.
*
* Output:   **ppState - a pointer to the state structure
*************************************************************************/
RvStatus SigCompStateHandlerSaveState(IN    SigCompStateHandlerMgr  *pSHMgr,
                                      IN    SigCompUdvmStateCompare pfnComparator,
                                      IN    void                    *pCtx,
                                      INOUT SigCompState            **ppState)
{
    RvStatus              rv;
    RvUint32              memcmpReturnValue;
    SigCompMinimalStateID partialID;
    RvBool                bComparatorReturnValue = RV_FALSE;
    RvBool                elementWasInserted     = RV_FALSE;
    RvBool                elementWasFound        = RV_FALSE;
    void                  *elementPtr            = NULL;
    SigCompState          *pExistingState        = NULL;
    SigCompState          *pState;

    if ((NULL == pSHMgr) || (NULL == ppState))
    {
        return(RV_ERROR_NULLPTR);
    }
    pState = *ppState;
    /* compare stateID (SHA1) */
    memcmpReturnValue = memcmp(nullStateID, pState->stateID,sizeof(RvSigCompStateID));
    if (0 == memcmpReturnValue )
    {
        /* StateID has not been set */
        return(RV_ERROR_BADPARAM);
    }
    memcpy(partialID, pState->stateID,sizeof(partialID));

    /* try to find a match to the partialID */
    elementWasFound = HASH_FindElement(pSHMgr->hHashTable,
                                       partialID,
                                       SigCompStateHashCompare,
                                       &elementPtr);
    if (elementWasFound)
    {
        /* get the found state */
        rv = HASH_GetUserElement(pSHMgr->hHashTable,elementPtr,&pExistingState);
        if (RV_OK != rv)
        {
            /* failed to insert element */
            return(rv);
        }
        /* check if this both states hold the same data */
        if (memcmp(pState->stateID,pExistingState->stateID,sizeof(RvSigCompStateID)) == 0)
        {
            /* both states have the same SHA1 key (20 bytes long) */
            if (pfnComparator == NULL) 
            {
                /* use memcmp ansi function to compare data */
                if (memcmp(pState->pData,pExistingState->pData,pState->stateDataSize) == 0) 
                {
                    bComparatorReturnValue = RV_TRUE;
                }
            }
            else 
            {
            	/* use supplied comparator callback function */
                bComparatorReturnValue = pfnComparator(pCtx, 
                                                       pExistingState->pData, 
                                                       pExistingState->stateDataSize);
            }
            
            if (bComparatorReturnValue == RV_TRUE)
            {
                /* both states hold the same data */
                /* thus: */

                /* delete the "temporary" state, since the content already exists */
                RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);
                /* delete state data */
                rv = RA_DeAlloc(pState->dataBufRA,(RA_Element)pState->pData);
                RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
                if (rv != RV_OK)
                {
                    /* failed to deAlloc the data buffer */
                    return(rv);
                }
                RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);

                /* delete the state object itself */
                rv = RA_DeAlloc(pState->statesRA,(RA_Element)pState);
                RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
                if (rv != RV_OK)
                {
                    /* failed to deAlloc the state */
                    return(rv);
                }
                *ppState = pExistingState;

                /* increment number of "users" */
                pExistingState->isUsed++;

                return(RV_ERROR_DUPLICATESTATE);
            }
            else
            {
                /* states have same SHA1 key but different data - collision */
                return(RV_ERROR_UNKNOWN);
            }
        }
        else
        {
            /* SHA1 (partialID) collision */
            return(RV_ERROR_UNKNOWN);
        }

    }
    else
    {
        RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);

		RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerSaveState - The state 0x%x%x%x is stored",partialID[0],partialID[1],partialID[2]));
        /* no such element, thus must insert into table */
        rv = HASH_InsertElement(pSHMgr->hHashTable,
                                partialID,
                                &pState,
                                RV_FALSE,
                                &elementWasInserted,
                                &elementPtr,
                                SigCompStateHashCompare);

        RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
        if (RV_OK != rv)
        {
            /* failed to insert element */
            return(rv);
        }
        if (elementWasInserted == RV_FALSE)
        {
            /* failed to insert element */
            return(RV_ERROR_UNKNOWN);
        }
        /* increment number of "users" */
        pState->isUsed++;
    }

    return(RV_OK);
} /* end of SigCompStateHandlerSaveState */


/*************************************************************************
* SigCompStateHandlerDeleteState
* ------------------------------------------------------------------------
* General: deletes the given state, this function is called when the
*          application asks to close a compartment, per each state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pState - The pointer to the state
*           bIsHashed - has this state been hashed / used
*************************************************************************/
RvStatus SigCompStateHandlerDeleteState(IN SigCompStateHandlerMgr *pSHMgr,
                                        IN SigCompState           *pState,
                                        IN RvBool                 bIsHashed)
{
    RvStatus              rv;
    SigCompMinimalStateID partialID;
    RvBool                elementWasFound    = RV_FALSE;
    void                  *elementPtr        = NULL;
    

    if ((NULL == pSHMgr) ||
        (NULL == pState))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
        "SigCompStateHandlerDeleteState - trying to delete a state (pState=%d stateID=0x%X 0x%X 0x%X)",
        pState,
        pState->stateID[0],
        pState->stateID[1],
        pState->stateID[2]));
    
    if (pState->isUsed > 0) 
    {
        /* decrement number of "users" of this state */
        pState->isUsed--;
        RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerDeleteState - state is used by %d compartments (pState=%d)",
            pState->isUsed,
            pState));
    }
    else 
    {
    	/* deleting a state that is not associated to any compartment */
    }
    
    if (pState->isUsed == 0)
    {
        RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerDeleteState - state is not used by any compartment"));
        if (pState->retentionPriority != 65535)
        {
            if (bIsHashed == RV_TRUE)
            {
                /* create a temporary partialID for the HASH */
                memcpy(partialID, pState->stateID,sizeof(partialID));
                /* try to find a match to the partialID */
                elementWasFound = HASH_FindElement(pSHMgr->hHashTable,
                                                    partialID,
                                                    SigCompStateHashCompare,
                                                    &elementPtr);
                if (!elementWasFound)
                {
                    /* error - state should have been in the HASH table */
                    RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                        "SigCompStateHandlerDeleteState - state not found in hash table"));
                    return(RV_ERROR_UNKNOWN);
                }
                RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                    "SigCompStateHandlerDeleteState - state found in hash table"));
                
                RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);
                /* remove from HASH table */
                rv = HASH_RemoveElement(pSHMgr->hHashTable,
                    elementPtr);
                RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
                if (RV_OK != rv)
                {
                    /* failed to remove element from HASH table */
                    RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                        "SigCompStateHandlerDeleteState - failed to remove element from hash table (rv=%d)",
                        rv));
                    return(rv);
                }
                RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                    "SigCompStateHandlerDeleteState - state removed from hash table"));
            }
            else 
            {
                /* state has not been "used" */
                RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                    "SigCompStateHandlerDeleteState - state has not been used"));
            }

            RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);
            /* delete state data */
            rv = RA_DeAlloc(pState->dataBufRA,(RA_Element)pState->pData);
            RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
            if (rv != RV_OK)
            {
                /* failed to deAlloc the data buffer */
                RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                    "SigCompStateHandlerDeleteState - failed to deAlloc the data buffer (rv=%d)",
                    rv));
                return(rv);
            }
            RvMutexLock(pSHMgr->pLock, pSHMgr->pLogMgr);

            /* delete the state object itself */
            rv = RA_DeAlloc(pState->statesRA,(RA_Element)pState);
            RvMutexUnlock(pSHMgr->pLock, pSHMgr->pLogMgr);
            if (rv != RV_OK)
            {
                /* failed to deAlloc the state */
                RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                    "SigCompStateHandlerDeleteState - failed to deAlloc the state object itself (rv=%d)",
                    rv));
                return(rv);
            }
        }
        else
        {
            /* local state (such as a static dictionary) -> keep in memory */
        }
    }

    return(RV_OK);
} /* end of SigCompStateHandlerDeleteState */

/*************************************************************************
* SigCompStateHandlerGetState
* ------------------------------------------------------------------------
* General: gets the given state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pPartialID - pointer to the ID of the state, can be a partial ID (6/9/12 bytes)
*           partialIdLength - length in bytes of partial ID (6/9/12/20 bytes)
*           bIgnoreAccessLength - TRUE if the GetState operation must ignore
*                                   the state's minimumAccessLength
*
* Output:   **ppState - a pointer to pointer to the state
*************************************************************************/
RvStatus  SigCompStateHandlerGetState(IN  SigCompStateHandlerMgr *pSHMgr,
                                      IN  RvUint8                *pPartialID,
                                      IN  RvUint32               partialIdLength,
                                      IN  RvBool                 bIgnoreAccessLength,
                                      OUT SigCompState           **ppState)
{
    RvStatus                crv;
    RvInt32                 memCompareResult;
    SigCompMinimalStateID   partialId6B;
    RvBool                  elementWasFound     = RV_FALSE;
    void                    *elementPtr         = NULL;
    SigCompState            *pExistingState     = NULL;

    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pPartialID) ||
        (NULL == ppState))
    {
        return(RV_ERROR_NULLPTR);
    }

    if ((partialIdLength <  6) ||
        (partialIdLength > 20))
    {
        return(RV_ERROR_BADPARAM);
    }

    RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
        "SigCompStateHandlerGetState - trying to get state (statePartialID=0x%X 0x%X 0x%X partialIdLength=%d)",
        pPartialID[0],
        pPartialID[1],
        pPartialID[2],
        partialIdLength));
    
    /* copy the 6 MSBs */
    memcpy(partialId6B,pPartialID,sizeof(partialId6B));
    /* try to find a match to the partialID */
    elementWasFound = HASH_FindElement(pSHMgr->hHashTable,
                                       partialId6B,
                                       SigCompStateHashCompare,
                                       &elementPtr);
    if (elementWasFound)
    {
        /* get the found state */
        crv = HASH_GetUserElement(pSHMgr->hHashTable,elementPtr,&pExistingState);
        if (RV_OK != crv)
        {
            /* failed to get element */
            *ppState = NULL;
            RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerGetState - failed to get state from hash table (crv=%d)",
                crv));
            return(CCS2SCS(crv));
        }
        
        RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerGetState - state retrieved from hash table (pState=%d stateID=0x%X 0x%X0 0x%X dataSize=%d)",
            pExistingState,
            pExistingState->stateID[0],
            pExistingState->stateID[1],
            pExistingState->stateID[2],
            pExistingState->stateDataSize));

        /* check if this is the intended state */
        memCompareResult = memcmp(pPartialID, pExistingState->stateID, partialIdLength);
        if((memCompareResult == 0)&&
            (partialIdLength >= pExistingState->minimumAccessLength))
        {
            /* both states have the same partial SHA1 key */
            *ppState = pExistingState;
        }
        else if((memCompareResult == 0)&&
                (bIgnoreAccessLength == RV_TRUE)) 
        {
            /* both states have the same partial SHA1 key */
            *ppState = pExistingState;
        }
        else
        {
            /* states have different partial SHA1 key - 6 MSBs collision */
            *ppState = NULL;
            RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerGetState - state partial key (6MSBs) collision"));
            return(RV_ERROR_UNKNOWN);
        }
    }
    else
    {
        /* no such state, with partialID */
        *ppState = NULL;
        RvLogDebug(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerGetState - state with requested partial key (6MSBs) not found"));
        return(RV_ERROR_UNKNOWN);
    }

    return(RV_OK);
} /* end of SigCompStateHandlerGetState */


/*************************************************************************
* SigCompStateHandlerStateSetParameters
* ------------------------------------------------------------------------
* General: set state priority
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           statePriority - state retention priority [0..65535]
*           stateMinAccessLength - minimum access length for the stateID (6..20)
*           stateAddress - 
*           stateInstruction - 
*           *pState - pointer to the state for which the priority will be modified
*
* Output:   *pState -
*************************************************************************/
RvStatus  SigCompStateHandlerStateSetParameters(
                             IN    SigCompStateHandlerMgr *pSHMgr,
                             IN    RvUint16               statePriority,
                             IN    RvUint16               stateMinAccessLength,
                             IN    RvUint16               stateAddress,
                             IN    RvUint16               stateInstruction,
                             INOUT SigCompState           *pState)
{
    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) )
    {
        return(RV_ERROR_NULLPTR);
    }

    pState->retentionPriority   = statePriority;
    pState->minimumAccessLength = stateMinAccessLength;
    pState->stateAddress        = stateAddress;
    pState->stateInstruction    = stateInstruction;

    return(RV_OK);
} /* end of SigCompStateHandlerStateSetParameters */


/*************************************************************************
* SigCompStateHandlerStateSetPriority
* ------------------------------------------------------------------------
* General: set state priority
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           priority - state retention priority [0..65535]
*           *pState - pointer to the state for which the priority will be modified
*
* Output:   *pState -
*************************************************************************/
RvStatus  SigCompStateHandlerStateSetPriority(IN    SigCompStateHandlerMgr *pSHMgr,
                                              IN    RvUint32               priority,
                                              INOUT SigCompState           *pState)
{
    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) )
    {
        return(RV_ERROR_NULLPTR);
    }

    pState->retentionPriority = (RvUint16) (priority & RV_UINT16_MAX);

    return(RV_OK);
} /* end of SigCompStateHandlerStateSetPriority */

/*************************************************************************
* SigCompStateHandlerStateGetPriority
* ------------------------------------------------------------------------
* General: set state priority
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pState - pointer to the state
*
* Output:   *pPriority - pointer to a variable to be filled with
*                        the state retention priority [0..65535]
*************************************************************************/
RvStatus  SigCompStateHandlerStateGetPriority(IN  SigCompStateHandlerMgr *pSHMgr,
                                              OUT RvUint32               *pPriority,
                                              IN  SigCompState           *pState)
{
    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pPriority) ||
        (NULL == pState) )
    {
        return(RV_ERROR_NULLPTR);
    }

    *pPriority = pState->retentionPriority & RV_UINT16_MAX;

    return(RV_OK);
} /* end of SigCompStateHandlerStateGetPriority */

/*************************************************************************
* SigCompStateHandlerStateSetID
* ------------------------------------------------------------------------
* General: sets the state ID, according to RFC-3320 section 9.4.9.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pState - pointer to the state
*
* Output    *pState - stateID field updated
*************************************************************************/
RvStatus SigCompStateHandlerStateSetID(IN    SigCompStateHandlerMgr *pSHMgr,
                                       INOUT SigCompState           *pState)
{
    RvSigCompSHA1Context shaContext;
    RvSigCompShaStatus	 srv;
    RvUint8				 data8;

    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState))
    {
        return(RV_ERROR_NULLPTR);
    }

    srv = RvSigCompSHA1Reset(&shaContext);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) ((0xFF00 & (pState->stateDataSize)) >> 8);
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) (0x00FF & (pState->stateDataSize));
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) ((0xFF00 & (pState->stateAddress)) >> 8);
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) ((0x00FF & (pState->stateAddress)));
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) ((0xFF00 & (pState->stateInstruction)) >> 8);
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) (0x00FF & (pState->stateInstruction));
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) ((0xFF00 & (pState->minimumAccessLength)) >> 8);
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    data8 = (RvUint8) (0x00FF & (pState->minimumAccessLength));
    srv = RvSigCompSHA1Input(&shaContext,&data8,1);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    srv = RvSigCompSHA1Input(&shaContext,pState->pData, pState->stateDataSize);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    srv = RvSigCompSHA1Result(&shaContext,pState->stateID);
    if (RVSIGCOMP_SHA_SUCCESS != srv) 
    {
        return(RV_ERROR_UNKNOWN);
    }
    
    return(RV_OK);
} /* end of SigCompStateHandlerStateSetID */

/*************************************************************************
* SigCompStateHandlerStateGetID
* ------------------------------------------------------------------------
* General: get state ID
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*
* Output:   *pStateID - pointer to a state ID variable (of type RvSigCompStateID)
*************************************************************************/
RvStatus SigCompStateHandlerStateGetID(IN  SigCompStateHandlerMgr *pSHMgr,
                                       IN  SigCompState           *pState, 
                                       OUT RvSigCompStateID       *pStateID)
{
    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) ||
        (NULL == pStateID) )
    {
        return(RV_ERROR_NULLPTR);
    }

    memcpy(pStateID, pState->stateID, sizeof(RvSigCompStateID));

    return(RV_OK);
} /* end of SigCompStateHandlerStateGetID */

/*************************************************************************
* SigCompStateHandlerStateGetSize
* ------------------------------------------------------------------------
* General: get state size
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*
* Output:   *pStateBufSize - return the allocated buffer size
*           *pStateDataSize - return the actual data size
*************************************************************************/
RvStatus SigCompStateHandlerStateGetSize(IN  SigCompStateHandlerMgr *pSHMgr,
                                         IN  SigCompState           *pState, 
                                         OUT RvUint32               *pStateBufSize,
                                         OUT RvUint32               *pStateDataSize)
{
    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) ||
        (NULL == pStateBufSize) ||
        (NULL == pStateDataSize) )
    {
        return(RV_ERROR_NULLPTR);
    }

    *pStateBufSize = pState->stateBufSize;
    *pStateDataSize = pState->stateDataSize;

    return(RV_OK);
} /* end of SigCompStateHandlerStateGetSize */

/*************************************************************************
* SigCompStateHandlerStateReadData
* ------------------------------------------------------------------------
* General: Reads the raw data from the state
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*           startLocation - byte location to start reading from
*           dataLength - number of bytes to read
* Output:   *pData - pointer allocated by the caller.
*                    Will be filled with the state's data
*************************************************************************/
RvStatus SigCompStateHandlerStateReadData(IN  SigCompStateHandlerMgr *pSHMgr,
                                          IN  SigCompState           *pState, 
                                          IN  RvUint32               startLocation, 
                                          IN  RvUint32               dataLength,
                                          OUT RvUint8                *pData)
{
    RvUint8  *startPosition;

    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) ||
        (NULL == pData) )
    {
        return(RV_ERROR_NULLPTR);
    }

    if ((startLocation + dataLength) > pState->stateDataSize)
    {
        /* trying to read past the data in the state memory */
        return(RV_ERROR_BADPARAM);
    }

    /* copy data from state into buffer */
    startPosition = pState->pData + startLocation;
    memcpy(pData, startPosition, dataLength);

    return(RV_OK);
} /* end of SigCompStateHandlerStateReadData */

/*************************************************************************
* SigCompStateHandlerStateWriteData
* ------------------------------------------------------------------------
* General: Writes the raw data into the state
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*           startLocation - byte location to start writing to
*           dataLength - number of bytes to write
*           *pData - raw data
* Output:
*************************************************************************/
RvStatus SigCompStateHandlerStateWriteData(IN SigCompStateHandlerMgr *pSHMgr,
                                           IN SigCompState           *pState, 
                                           IN RvUint32               startLocation, 
                                           IN RvUint32               dataLength,
                                           IN RvUint8                *pData)
{
    RvUint8  *startPosition;

    /* argument validity tests */
    if ((NULL == pSHMgr) ||
        (NULL == pState) ||
        (NULL == pData) )
    {
        return(RV_ERROR_NULLPTR);
    }
    if ((startLocation + dataLength) > pState->stateDataSize)
    {
        /* trying to write past the data buffer in the state memory */
        return(RV_ERROR_BADPARAM);
    }

    /* copy data from buffer into the state */
    startPosition = pState->pData + startLocation;
    memcpy(startPosition, pData, dataLength);

    return(RV_OK);
} /* end of SigCompStateHandlerStateWriteData */




/*--------------------HASH FUNCTION CALLBACKS----------------------*/
/***************************************************************************
* HashFunction
* ------------------------------------------------------------------------
* General: call back function for calculating the hash value from the hash key.
* Return Value: The value generated from the key.
* ------------------------------------------------------------------------
* Arguments:
* Input:     key  - The hash key (of type CallLegMgrHashKey)
***************************************************************************/
static RvUint32 SigCompStateHashFunction(void *key)
{
    /* the states are hashed into the table according to the 6 MSBs of their SHA-1
        the key is thus 6 bytes long */
    if (NULL == key)
    {
        return(0);
    }

    /* use 42 as the "seed" */
    return(SigCompFastHash((RvUint8 *)key,SIGCOMP_MINIMAL_STATE_ID_LENGTH,42));
} /* end of SigCompStateHashFunction */

/***************************************************************************
* CallHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same - the compare
*          is to a regular call (not a replaces comparison).
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashElement  - a new hash element
*           oldHashElement  - an element from the hash
***************************************************************************/
static RvBool SigCompStateHashCompare(IN void *newHashElement,
                                      IN void *oldHashElement)
{
    RvInt32 rv;

    if ((NULL == newHashElement) ||
        (NULL == oldHashElement))
    {
        return(RV_FALSE);
    }

    rv = memcmp(newHashElement,oldHashElement,sizeof(SigCompMinimalStateID));
    if (0 == rv)
    {
        /* buffers are identical */
        return(RV_TRUE);
    }

    return(RV_FALSE);
} /* end of SigCompStateHashCompare */


/*************************************************************************
* SigCompStateHandlerAlgContextDataAlloc
* ------------------------------------------------------------------------
* General: allocates an area for the algorithm-context data
*           Should be called once per compartment/algorithm
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a pointer to the state handler
*            requestedSize - the requested size in bytes
* Output:   **ppContextData - a pointer to pointer of the context's data
*           *pCurRA - the RA from which the buffer was allocated
*************************************************************************/
RvStatus SigCompStateHandlerAlgContextDataAlloc(
                            IN  SigCompStateHandlerMgr *pSHMgr,
                            IN  RvUint32               requestedSize,
                            OUT RvUint8                **ppContextData,
                            OUT HRA                    *pCurRA)
{
    RvStatus     crv;
    HRA          raPool;
    RvUint8      *pDataBuffer = NULL;
    RvUint32     freeElements = 0;
    
    if ((NULL == pSHMgr) ||
        (NULL == ppContextData))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerAlgContextDataAlloc(mgr=0x%x)",pSHMgr));


    /* find out from which RA pool to allocate the buffer, according to size and availability */
    if ((requestedSize <= pSHMgr->smallBufSize) &&
        ((freeElements = RA_NumOfFreeElements(pSHMgr->smallBufRA)) > 0))
    {
        /* allocate from pool with small buffers */
        raPool = pSHMgr->smallBufRA;
    }
    else if ((requestedSize <= pSHMgr->mediumBufSize) &&
             ((freeElements = RA_NumOfFreeElements(pSHMgr->mediumBufRA)) > 0))
    {
        /* allocate from pool with medium buffers */
        raPool = pSHMgr->mediumBufRA;
    }
    else if ((requestedSize <= pSHMgr->largeBufSize) &&
             ((freeElements = RA_NumOfFreeElements(pSHMgr->largeBufRA)) > 0))
    {
        /* allocate from pool with large buffers */
        raPool = pSHMgr->largeBufRA;
    }
    else
    {
        if (requestedSize > pSHMgr->largeBufSize)
        {
         /*
        requested size is too large for current pools
        it is recommended to check the relevant configuration parameters
        */
           RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerAlgContextDataAlloc - requested size is too large for current pools (size=%d)",
                requestedSize));
            return(RV_ERROR_BADPARAM);
        }
        else
        {
            /* out of resources */
            RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
                "SigCompStateHandlerAlgContextDataAlloc - out of resources"));
                return(RV_ERROR_OUTOFRESOURCES);
        }
    }

    /* allocate a buffer for the context data from the pool */
    crv = RA_Alloc(raPool,(RA_Element *)&pDataBuffer);
    if (RV_OK != crv)
    {
        /* failed to allocate a data buffer */
        RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerAlgContextDataAlloc - failed to allocate a data buffer (crv=%d)",
            crv));
        return(CCS2SCS(crv));
    }

    memset(pDataBuffer, 0, requestedSize);
    *ppContextData = pDataBuffer;
    *pCurRA        = raPool;

    RV_UNUSED_ARG(freeElements);

    RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerAlgContextDataAlloc(mgr=0x%x)",pSHMgr));
    return(RV_OK);
} /* end of SigCompStateHandlerAlgContextDataAlloc */


/*************************************************************************
* SigCompStateHandlerAlgContextDataDealloc
* ------------------------------------------------------------------------
* General: deallocates the algorithm-context data buffer, this function is called when the
*          application asks to close a compartment, or when changing algorithms
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a pointer to the state handler
*           *pContext - a pointer to the context
*           curRA - the RA from which the buffer was allocated
*************************************************************************/
RvStatus SigCompStateHandlerAlgContextDataDealloc(
                            IN SigCompStateHandlerMgr *pSHMgr,
                            IN RvUint8                *pContextData,
                            IN HRA                    curRA)
{
    RvStatus rv;

    if ((NULL == pSHMgr) ||
        (NULL == pContextData))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerAlgContextDataDealloc(mgr=0x%x)",pSHMgr));
    
    /* deAllocate the buffer for the context data from the pool */
    rv = RA_DeAlloc(curRA,(RA_Element)pContextData);
    if (RV_OK != rv)
    {
        /* failed to allocate a data buffer */
        RvLogError(pSHMgr->pLogSrc ,(pSHMgr->pLogSrc ,
            "SigCompStateHandlerAlgContextDataDealloc - failed to deallocate the data buffer (rv=%d)",
            rv));
        return(rv);
    }
    
    RvLogLeave(pSHMgr->pLogSrc,(pSHMgr->pLogSrc,
		"SigCompStateHandlerAlgContextDataDealloc(mgr=0x%x)",pSHMgr));
    return(RV_OK);
} /* end of SigCompStateHandlerAlgContextDataDealloc */





#if defined(__cplusplus)
}
#endif



