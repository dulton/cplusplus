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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/

/*********************************************************************************
 *                              <SigCompCompartmentHandlerObject.c>
 *
 * Description :
 * this file hold the implementation of the compartment handler manager
 *    Author                         Date
 *    ------                        ------
 *    ellyAmitai                    20031209
 *    Gil Keini                     20040111
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILED                           */
/*-----------------------------------------------------------------------*/
#include "SigCompCompartmentHandlerObject.h"
#include "SigCompMgrObject.h"
#include "SigCompCommon.h"
#include "SigCompCallbacks.h"
#include <string.h> /* for memset & memcpy */

/*-----------------------------------------------------------------------*/
/*                          DEFINES                                      */
/*-----------------------------------------------------------------------*/
#define CPB_MASK (0xC0)
#define DMS_MASK (0x38)
#define SMS_MASK (0x07)

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


static RvStatus FindStateInList(IN  SigCompCompartment *pCompartment,
                                IN  SigCompState       *pState,
                                OUT SigCompCompartmentState **ppListState);

static RvStatus GetLowestPriorityStateItemFromList(
                                 IN  RLIST_POOL_HANDLE       hStateListPool,
                                 IN  RLIST_HANDLE            hStatesList,
                                 IN  RvLogMgr                *pLogMgr,
                                 OUT SigCompCompartmentState **ppState);

static RvStatus FindDuplicateStateInList(IN  SigCompCompartment      *pCompartment,
                                         IN  SigCompState            *pState,
                                         IN  SigCompUdvmStateCompare pfnComparator,
                                         IN  void                    *pCtx,
                                         OUT SigCompCompartmentState **ppListState);
                                         

/*-----------------------------------------------------------------------*/
/*                           MODULE  FUNCTIONS                           */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompCompartmentHandlerConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the compartment handler ,there
*          is one compartment handler per sigComp entity and therefore it is
*          called upon initialization of the sigComp module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RvStatus SigCompCompartmentHandlerConstruct(IN RvSigCompMgrHandle hSigCompMgr)
{
    SigCompMgr         *pSigCompMgr      = (SigCompMgr *)hSigCompMgr;
    RvLogMgr           *pLogMgr          = pSigCompMgr->pLogMgr;
    HRA                compHndlrCompRA   = NULL;
    RLIST_POOL_HANDLE  hStateListPool    = NULL;
    RvUint32           compNum           = pSigCompMgr->maxOpenCompartments;
    SigCompCompartment *pCompartment     = NULL;
    RvStatus           rv;
    RvStatus           crv;
    RvUint32           counter;
    
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource, 
		"SigCompCompartmentHandlerConstruct(mgr=0x%x)",hSigCompMgr));
    if (NULL == hSigCompMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Firstly set the back pointer to SigCompMgr (it's used during destruction)*/
    pSigCompMgr->compartmentHandlerMgr.pSCMgr = pSigCompMgr;

    /* allocate the memory needed for the compartment handler data structure */

    /* allocate an array of compartments */
    compHndlrCompRA = RA_Construct(sizeof(SigCompCompartment),
                                   compNum,
                                   NULL, /* Pointer to a line printing function */
                                   pLogMgr,
                                   "compartments array");
    if (compHndlrCompRA == NULL)
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "SigCompCompartmentHandlerConstruct - failed to allocate an array of compartments"));
        return RV_ERROR_OUTOFRESOURCES;
    }
    /* initialize the compartment handler's compartment array */
    pSigCompMgr->compartmentHandlerMgr.hCompartmentRA = compHndlrCompRA;

    /* create lock for each compartment */
    for(counter = 0; counter < compNum; counter++)
    {
        crv = RA_Alloc(compHndlrCompRA, (RA_Element *)&pCompartment);
        if (RV_OK != crv) 
        {
            /* failed to construct lock in the compartment */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "SigCompCompartmentHandlerConstruct - failed to allocate a compartment (crv=%d)",
                crv));
            return (CCS2SCS(crv));
        }
        /* construct the lock/mutex */
        crv = RvMutexConstruct(pLogMgr, &pCompartment->lock);
        if (RV_OK != crv) 
        {
            /* failed to construct lock in the compartment */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "SigCompCompartmentHandlerConstruct - failed to construct lock in the compartment (crv=%d)",
                crv));
            return (CCS2SCS(crv));
        }
    } /* end of FOR loop to allocate all compartments and construct a lock for each */

    /* deAlloc all compartments after construction locks */
    for(counter = 0; counter < compNum; counter++)
    {
        /* get compartment by order */
        pCompartment = (SigCompCompartment*)RA_GetAnyElement(compHndlrCompRA, counter);
        if (NULL == pCompartment) 
        {
            /* error in getting compartment */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "SigCompCompartmentHandlerConstruct - error in getting compartment after constructions locks"));
            return RV_ERROR_UNKNOWN;
        }
        /* deallocate the compartment (and leave the lock "intact") */
        rv = RA_DeAllocByPtr(compHndlrCompRA, (RA_Element)pCompartment);
        if (rv != RV_OK)
        {
            /* failed to deallocate from RA */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "SigCompCompartmentHandlerConstruct - failed to deallocate from RA (rv=%d)",
                rv));
            return rv;
        }        
    } /* end of FOR loop to dealloc all compartments w/o destructing the locks */

    /* reset usage statistics */
    RA_ResetMaxUsageCounter(compHndlrCompRA);

    /* initialize a pool of lists for all the state lists */
    hStateListPool = RLIST_PoolListConstruct((compNum * pSigCompMgr->maxStatesPerCompartment),
                                              compNum,
                                              sizeof(SigCompCompartmentState),
                                              pLogMgr,
                                              "compartment states list pool");
    if (hStateListPool == NULL)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "SigCompCompartmentHandlerConstruct - failed to initialize state-list pool"));
        /* clear previous allocations (from above) */
        RA_Destruct(pSigCompMgr->compartmentHandlerMgr.hCompartmentRA);
        RA_Destruct(pSigCompMgr->compartmentHandlerMgr.hAlgContextRA);
        pSigCompMgr->compartmentHandlerMgr.hCompartmentRA = NULL;
        pSigCompMgr->compartmentHandlerMgr.hAlgContextRA = NULL;
        return RV_ERROR_UNKNOWN;
    }
    pSigCompMgr->compartmentHandlerMgr.hStateListPool = hStateListPool;

    
    /* construct the lock/mutex of the manager itself */
    crv = RvMutexConstruct(pSigCompMgr->pLogMgr, 
                           &pSigCompMgr->compartmentHandlerMgr.lock);
    if (RV_OK != crv) 
    {
        /* failed to construct lock in the state element */
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "SigCompCompartmentHandlerConstruct - failed to construct lock of the manager (crv=%d)",
            crv));
        SigCompCompartmentHandlerDestruct(&pSigCompMgr->compartmentHandlerMgr);
        return(CCS2SCS(crv));
    }
    pSigCompMgr->compartmentHandlerMgr.pLock = &pSigCompMgr->compartmentHandlerMgr.lock;
    
    pSigCompMgr->compartmentHandlerMgr.maxStateMemoryPerCompartment = pSigCompMgr->stateMemSize;

    RvLogInfo(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource, 
        "SigCompCompartmentHandlerConstruct - object constructed successfully"));
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource, 
		"SigCompCompartmentHandlerConstruct(mgr=0x%x)",hSigCompMgr));
    return RV_OK;
} /* end of SigCompCompartmentHandlerConstruct */

/*************************************************************************
* SigCompCompressorDestruct
* ------------------------------------------------------------------------
* General: destructor of the compartment handler, will be called
*          when destructing the sigComp module
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*************************************************************************/
void SigCompCompartmentHandlerDestruct(IN SigCompCompartmentHandlerMgr *pCHMgr)
{
    SigCompCompartment *pCompartment;
    RvLogMgr           *pLogMgr;
    RvUint32           counter  = 0;
    RvUint32           maxCompartments;
    RvInt32            lockCount;   /* dummy variable */
    RvUint32           elementSize; /* dummy variable */
    RvStatus           rv;
    RvStatus           crv;
    
    if (NULL == pCHMgr || pCHMgr->pSCMgr == NULL)
    {
        return;
    }
    lockCount = 0;

    pLogMgr = pCHMgr->pSCMgr->pLogMgr;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
	if(!pLogMgr || !pCHMgr->pSCMgr->pLogSource)
		return;
#endif
/* SPIRENT_END */

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerDestruct(handlerMgr=0x%x)",pCHMgr));
    if (NULL != pCHMgr->pLock)
    {
        RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    }

    if (NULL != pCHMgr->hCompartmentRA) 
    {
        /* loop to deallocate all non-vacant compartments */
        RA_GetAllocationParams(pCHMgr->hCompartmentRA, &elementSize, &maxCompartments);
        for (counter = 0; counter < maxCompartments; counter++)
        {
            /* get compartment by order */
            pCompartment = (SigCompCompartment*)RA_GetAnyElement(pCHMgr->hCompartmentRA, counter);
            if (NULL == pCompartment) 
            {
                /* error in getting compartment */
                if (NULL != pCHMgr->pLock)
                {
                    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
                }
                return;
            }
            if (!RA_ElemPtrIsVacant(pCHMgr->hCompartmentRA, (RA_Element)pCompartment))
            {
                /* deallocate the compartment (and leave the lock "intact") */
                rv = RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
                if (rv != RV_OK)
                {
                    /* failed to deallocate from RA */
                    if (NULL != pCHMgr->pLock)
                    {
                        RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
                    }
                    return;
                }        
            }
        }
        /* allocate all compartments and destruct the lock of each */
        for (counter = 0; counter < maxCompartments; counter++)
        {
            crv = RA_Alloc(pCHMgr->hCompartmentRA, (RA_Element *)&pCompartment);
            if (RV_OK != crv) 
            {
                /* failed to allocate a compartment */
                if (NULL != pCHMgr->pLock)
                {
                    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
                }
                return;
            }
            /* release all locks */
            crv = RvMutexRelease(&pCompartment->lock, pCHMgr->pSCMgr->pLogMgr, &lockCount);
            if (RV_OK != crv) 
            {
                /* failed to release lock in the compartment */
                if (NULL != pCHMgr->pLock)
                {
                    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
                }
                return;
            }
            /* destruct the lock/mutex */
            crv = RvMutexDestruct(&pCompartment->lock, pLogMgr);
            if (RV_OK != crv) 
            {
                /* failed to destruct lock in the compartment */
                if (NULL != pCHMgr->pLock)
                {
                    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
                }
                return;
            }
        }
    }
    
    if (NULL != pCHMgr->hCompartmentRA)
    {
        RA_Destruct(pCHMgr->hCompartmentRA);
    }
    if (NULL != pCHMgr->hStateListPool)
    {
        RLIST_PoolListDestruct(pCHMgr->hStateListPool);
    }

    if (pCHMgr->pLock != NULL)
    {
        /* release & destruct the lock/mutex of the manager itself */
        crv = RvMutexRelease(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr, &lockCount);
        if (RV_OK != crv) 
        {
            /* failed to release lock of the manager */
            return;
        }
        /* destruct the lock/mutex */
        crv = RvMutexDestruct(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        if (RV_OK != crv) 
        {
            /* failed to destruct lock of the manager */
            return;
        }
        pCHMgr->pLock = NULL;
    }
    
    
    RvLogInfo(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
        "SigCompCompartmentHandlerDestruct - object destructed (pCHMgr=0x%x)",
        pCHMgr));
    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerDestruct(handlerMgr=0x%x)",pCHMgr));
    return;
} /* end of SigCompCompartmentHandlerDestruct */


/*************************************************************************
* SigCompCompartmentHandlerGetResources
* ------------------------------------------------------------------------
* General: retrieves the resource usage of the CompartmentHandler
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pCHMgr - a pointer to the compartment handler
*
* Output:    *RvSigCompCompartmentHandlerResources - a pointer a resource structure
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetResources(
                IN SigCompCompartmentHandlerMgr          *pCHMgr,
                OUT RvSigCompCompartmentHandlerResources *pResources)
{
    RV_Resource       rvResource;
    RvSigCompResource *pSGResource;
    RvStatus          rv = RV_OK;

    if ((NULL == pCHMgr) || (NULL == pResources))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerGetResources(handlerMgr=0x%x)",pCHMgr));
    
    RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);

    /* clear the resource report */
    memset(pResources,0,sizeof(RvSigCompCompartmentHandlerResources));

    /* get the compartment structs RA resource status */
    if (NULL != pCHMgr->hCompartmentRA)
    {
        rv = RA_GetResourcesStatus(pCHMgr->hCompartmentRA,&rvResource);
        pSGResource = &pResources->compratmentsStructRA;
        pSGResource->currNumOfUsedElements = rvResource.numOfUsed;
        pSGResource->maxUsageOfElements = rvResource.maxUsage;
        pSGResource->numOfAllocatedElements = rvResource.maxNumOfElements;
    }

    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerGetResources(handlerMgr=0x%x)",pCHMgr));
    return rv;
} /* end of SigCompCompartmentHandlerGetResources */

/*************************************************************************
* SigCompCompartmentHandlerCreateComprtment
* ------------------------------------------------------------------------
* General: allocates a new compartment from the pool. 
* IMPORTANT: This function locks the output parameter (compartment), thus
*            it must be unlocked by executing 
*            SigCompCompartmentHandlerSaveComprtment.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*
* Output:   **ppCompartment - a pointer to a pointer to the compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerCreateComprtmentEx(
                       IN  SigCompCompartmentHandlerMgr *pCHMgr,
                       IN  RvChar                       *algoName,
                       OUT SigCompCompartment           **ppCompartment)
{
    RvStatus           rv;
    RvStatus           crv; /* common core return value */
    RvLogMgr           *pLogMgr      = pCHMgr->pSCMgr->pLogMgr;
    SigCompCompartment *pCompartment = NULL;
    SigCompMgr *scm = pCHMgr->pSCMgr;
    RvSigCompAlgorithm *algo;

    if ((NULL == pCHMgr) || (NULL == ppCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource, 
		"SigCompCompartmentHandlerCreateComprtmentEx(handlerMgr=0x%x)",pCHMgr));

    if (*ppCompartment != NULL)
    {
        /* pCompartment points to an address -> compartment already exists */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerCreateComprtmentEx - compartment already exists"));
        return(RV_ERROR_BADPARAM);
    }

    RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    /* allocate from compartments RA */
    crv = RA_Alloc(pCHMgr->hCompartmentRA, (RA_Element *)&pCompartment);
    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    if (crv != RV_OK)
    {
        /* failed to allocate from RA */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerCreateComprtmentEx - failed to allocate compartment from RA (crv=%d)",
            crv));
        return(CCS2SCS(crv));
    }
    /* lock the compartment */
    rv = SigCompCommonEntityLock(&pCompartment->lock, 
                                 pLogMgr,
                                 pCHMgr->pSCMgr->pLogSource,
                                 pCHMgr->hCompartmentRA,
                                 (RA_Element)pCompartment);
    if (rv != RV_OK) 
    {
        /* failed to lock compartment */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerCreateComprtmentEx - failed to lock compartment"));
        RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
        return(RV_ERROR_UNKNOWN);
    }

    /* init the compartment values */
    pCompartment->pCHMgr    = pCHMgr; /* hook to the creator */
    pCompartment->curRA     = pCHMgr->hCompartmentRA;
    pCompartment->availableStateMemory = pCHMgr->maxStateMemoryPerCompartment;
    pCompartment->counter = 0;

    pCompartment->bRequestedFeedbackItemReady = RV_FALSE;
    pCompartment->bReturnedFeedbackItemReady = RV_FALSE;
    pCompartment->bSendBytecodeInNextMessage = RV_TRUE;
    pCompartment->bSendLocalCapabilities = RV_TRUE;
    pCompartment->bSendLocalStatesIdList = RV_FALSE;
    
    /* the local EP capabilities may change per compartment on creation and even dynamically */
    pCompartment->localCPB  = scm->cyclePerBit;
    pCompartment->localDMS  = scm->decompMemSize;
    pCompartment->localSMS  = scm->stateMemSize;
    pCompartment->bLocalCapabilitiesHaveChanged = RV_TRUE;

    pCompartment->remoteCPB = MIN_CYCLES_PER_BIT;
    pCompartment->remoteDMS = MIN_DECOMPRESSION_MEMORY_SIZE;
    pCompartment->remoteSMS = MIN_STATE_MEMORY_SIZE;
    pCompartment->remoteSigCompVersion = 1; /* the minimum value allowed */
    pCompartment->bRemoteCapabilitiesHaveChanged = RV_FALSE;
    
    pCompartment->nIncomingMessages = 0;

    memset(pCompartment->requestedFeedbackItem,0,sizeof(pCompartment->requestedFeedbackItem));
    pCompartment->rqFeedbackItemLength = 0;
    memset(pCompartment->returnedFeedbackItem,0,sizeof(pCompartment->returnedFeedbackItem));
    pCompartment->rtFeedbackItemLength = 0;
    memset(pCompartment->remoteIdList,0,sizeof(pCompartment->remoteIdList));

    /* allocate the states array for this compartment (of SigCompCompartmentState elements) */
    RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    pCompartment->hStatesList = RLIST_ListConstruct(pCHMgr->hStateListPool);
    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    if (pCompartment->hStatesList == NULL)
    {
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerCreateComprtmentEx - failure to allocate the states array for this compartment (out of resources)"));
        RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
        RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
        RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        return(RV_ERROR_UNKNOWN);
    }

    /* set compartment's algorithm to point to the first (default) algorithm on list */
    if(algoName == 0 || algoName[0] == 0){
        RLIST_GetHead(pCHMgr->pSCMgr->hAlgorithmListPool,
                      pCHMgr->pSCMgr->hAlgorithmList,
                      (RLIST_ITEM_HANDLE *)&pCompartment->pCompressionAlg);
    } else {
        SigCompAlgorithmFind(scm, algoName, &pCompartment->pCompressionAlg);
    }

    algo = pCompartment->pCompressionAlg;

    if(algo == NULL) 
    {
        /* empty list - user did not map/add at least one compression algorithm */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerCreateComprtmentEx - empty algorithms list"));
        RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        RLIST_ListDestruct(pCHMgr->hStateListPool, pCompartment->hStatesList);
        SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
        RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
        RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        return(RV_ERROR_UNKNOWN);
    }

    /* set the context for the algorithm */
    if(algo->contextSize != 0) 
    {
        /* need to init context */
        rv = SigCompStateHandlerAlgContextDataAlloc(&pCHMgr->pSCMgr->stateHandlerMgr,
                                                    pCompartment->pCompressionAlg->contextSize,
                                                    &pCompartment->algContext.pData,
                                                    &pCompartment->algContext.curRA);
        if (rv != RV_OK) 
        {
            /* failed to allocate data buffer for context */
            RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
                "SigCompCompartmentHandlerCreateComprtmentEx - failure to allocate data buffer for algContext (rv=%d)",
                rv));
            RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
            RLIST_ListDestruct(pCHMgr->hStateListPool, pCompartment->hStatesList);
            SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
            RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
            RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
            return(RV_ERROR_UNKNOWN);
        }
        pCompartment->algContext.dataSize = pCompartment->pCompressionAlg->contextSize;
    }
    else 
    {
        /* no context-memory is needed */
    	pCompartment->algContext.dataSize = 0;
        pCompartment->algContext.curRA    = NULL;
        pCompartment->algContext.pData    = NULL;
    }
    
	RvLogDebug(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
        "SigCompCompartmentHandlerCreateComprtmentEx: Compartment 0x%x was created",
        pCompartment));
    *ppCompartment = pCompartment;
    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerCreateComprtmentEx(handlerMgr=0x%x,comp=0x%x)",pCHMgr,*ppCompartment));
	
	/* Notify the algorithm of a new created compartment */ 
	SigCompCallbackCompartmentCreatedEv(pCHMgr->pSCMgr,pCompartment);

    return(RV_OK);
} /* end of SigCompCompartmentHandlerCreateComprtment */

/*************************************************************************
* SigCompCompartmentHandlerGetComprtment
* ------------------------------------------------------------------------
* General: gets the compartment from its handle and locks it. A call to 
*          this function MUST be followed by a call to the function 
*          SigCompCompartmentHandlerSaveComprtment, which unlocks the 
*          compartment entity.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler Mgr
*           hCompartment - The handle of the compartment
*
* Output:   **ppCompartment - a pointer to the this compartment in memory
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetComprtment(
                IN  SigCompCompartmentHandlerMgr *pCHMgr,
                IN  RvSigCompCompartmentHandle   hCompartment,
                OUT SigCompCompartment           **ppCompartment)
{
    RvLogMgr           *pLogMgr      = pCHMgr->pSCMgr->pLogMgr;
    SigCompCompartment *pCompartment = (SigCompCompartment *) hCompartment;
    RvStatus           rv;
    
    if ((NULL == pCHMgr) || (NULL == hCompartment) || (NULL == ppCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
        "SigCompCompartmentHandlerGetComprtment(handlerMgr=0x%x,hComp=0x%x)",pCHMgr,hCompartment));

    *ppCompartment = pCompartment;
    /* lock the compartment */
    rv = SigCompCommonEntityLock(&pCompartment->lock, 
                                pLogMgr,
                                pCHMgr->pSCMgr->pLogSource,
                                pCHMgr->hCompartmentRA,
                                (RA_Element)pCompartment);
    if (rv != RV_OK) 
    {
        /* failed to lock compartment */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerGetComprtment - failed to lock compartment"));
        return(RV_ERROR_UNKNOWN);
    }

    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerGetComprtment(handlerMgr=0x%x,hComp=0x%x)",pCHMgr,hCompartment));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerGetComprtment */


/*************************************************************************
* SigCompCompartmentHandlerSaveComprtment
* ------------------------------------------------------------------------
* General: saves the given compartment, this function also "unlocks" the compartment
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the session's compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerSaveComprtment(
                           IN SigCompCompartmentHandlerMgr *pCHMgr,
                           IN SigCompCompartment           *pCompartment)

{
    RvLogMgr *pLogMgr;
        
    if ((NULL == pCHMgr) || (NULL == pCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }

    pLogMgr  = pCHMgr->pSCMgr->pLogMgr;

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerSaveComprtment(handlerMgr=0x%x,pComp=0x%x)",pCHMgr,pCompartment));
    
    /* unlock compartment */
    SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);

    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerSaveComprtment(handlerMgr=0x%x,pComp=0x%x)",pCHMgr,pCompartment));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerSaveComprtment */

/*************************************************************************
* SigCompCompartmentHandlerDeleteComprtment
* ------------------------------------------------------------------------
* General: deletes the given compartment, this function is called when the
*          application asks to close a compartment
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           pCompartmentID - The ID of the session's compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerDeleteComprtment(
                                  IN SigCompCompartmentHandlerMgr *pCHMgr,
                                  IN RvSigCompCompartmentHandle   hCompartment)
{
    RvStatus                rv;
    SigCompCompartmentState *pCptState  = NULL;
    SigCompCompartment      *pCompartment;

    if ((NULL == pCHMgr) ||
        (NULL == hCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
        "SigCompCompartmentHandlerDeleteComprtment(handlerMgr=0x%x,comp=0x%x)",pCHMgr,hCompartment));

    /* get and lock the compartment */
    rv = SigCompCompartmentHandlerGetComprtment(pCHMgr, hCompartment, &pCompartment);
    if (rv != RV_OK)
    {
        return(rv);
    }
    
    /* first free its states */
    do {
        RLIST_GetHead(pCompartment->pCHMgr->hStateListPool,
                      pCompartment->hStatesList,
                      (RLIST_ITEM_HANDLE *)&pCptState);
        if (pCptState == NULL)
        {
            /* no state found */
            break;
        }

        /* delete the state from the state handler */
        rv = SigCompStateHandlerDeleteState(&(pCHMgr->pSCMgr->stateHandlerMgr),
                                            pCptState->pState,
                                            RV_TRUE);
        if (rv != RV_OK)
        {
            SigCompCompartmentHandlerSaveComprtment(pCHMgr,pCompartment);
            return(rv);
        }

        /*remove the item from the list*/
        RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
        RLIST_Remove(pCompartment->pCHMgr->hStateListPool,
                     pCompartment->hStatesList,
                     (RLIST_ITEM_HANDLE)pCptState);
        RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    } while(pCptState != NULL);

    /* now release the list itself */
    RLIST_ListDestruct(pCompartment->pCHMgr->hStateListPool, pCompartment->hStatesList);

    /* handle the algContext */
    if (pCompartment->algContext.pData != NULL) 
    {
       	/* Notify the algorithm that the compartment is being destructed */ 
    	SigCompCallbackCompartmentDestructedEv(pCHMgr->pSCMgr,pCompartment);


        /* context was allocated */
        rv = SigCompStateHandlerAlgContextDataDealloc(&pCHMgr->pSCMgr->stateHandlerMgr,
                                                      pCompartment->algContext.pData,
                                                      pCompartment->algContext.curRA);
        if (rv != RV_OK) 
        {
            SigCompCompartmentHandlerSaveComprtment(pCHMgr,pCompartment);
            /* failed to dealloc context data buffer */
            return(rv);
        }
                                                      
        pCompartment->algContext.dataSize = 0;
        pCompartment->algContext.curRA    = NULL;
        pCompartment->algContext.pData    = NULL;
    }  
    SigCompCompartmentHandlerSaveComprtment(pCHMgr,pCompartment);
    
    /* then free the compartment itself */
    RvMutexLock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    rv = RA_DeAllocByPtr(pCHMgr->hCompartmentRA, (RA_Element)pCompartment);
    RvMutexUnlock(pCHMgr->pLock, pCHMgr->pSCMgr->pLogMgr);
    if (rv != RV_OK)
    {
        return(RV_ERROR_UNKNOWN);
    }

    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerDeleteComprtment(handlerMgr=0x%x,comp=0x%x)",pCHMgr,hCompartment));

    return(RV_OK);
} /* end of SigCompCompartmentHandlerDeleteComprtment */


/*************************************************************************
* SigCompCompartmentHandlerGetComprtmentSize
* ------------------------------------------------------------------------
* General: get decompression bytecode
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           hCompartment - The handle to the compartment
*
* Output    *pCompartmentSize - the size of the given state
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetComprtmentSize(
                           IN  SigCompCompartmentHandlerMgr *pCHMgr,
                           IN  RvSigCompCompartmentHandle   hCompartment,
                           OUT RvUint32                     *pCompartmentSize)
{
    RvStatus                rv;
    RvUint32                stateBufSize;
    RvUint32                stateDataSize;
    RLIST_ITEM_HANDLE       curItem       = NULL;
    RLIST_ITEM_HANDLE       nextItem      = NULL;
    SigCompCompartment      *pCompartment = NULL;
    SigCompCompartmentState *pCptState    = NULL;
    
    if ((NULL == pCHMgr)||(NULL == hCompartment)||(NULL == pCompartmentSize))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
        "SigCompCompartmentHandlerGetComprtmentSize(handlerMgr=0x%x,hComp=0x%x)",pCHMgr,hCompartment));

    /* get and lock the compartment */
    rv = SigCompCompartmentHandlerGetComprtment(pCHMgr, hCompartment, &pCompartment);
    if (rv != RV_OK)
    {
        return(rv);
    }

    /* go over the states array and calculate the size of each state */
    RLIST_GetHead(pCHMgr->hStateListPool, 
                  pCompartment->hStatesList, 
                  (RLIST_ITEM_HANDLE *)&pCptState);

    /* zero the variable before calculation */
    *pCompartmentSize = 0;

    while (pCptState != NULL)
    {
        rv = SigCompStateHandlerStateGetSize(&(pCHMgr->pSCMgr->stateHandlerMgr),
                                             pCptState->pState,
                                             &stateBufSize,
                                             &stateDataSize);
        *pCompartmentSize += stateDataSize;

        RLIST_GetNext(pCHMgr->hStateListPool,
                      pCompartment->hStatesList,
                      curItem,
                      &nextItem);
        pCptState = (SigCompCompartmentState *)nextItem;
    }

    /* unlocks the compartment */
    rv = SigCompCompartmentHandlerSaveComprtment(pCHMgr, pCompartment);
    if (RV_OK != rv)
    {
        /* make sure the compartment is unlocked */
        SigCompCommonEntityUnlock(&pCompartment->lock, pCHMgr->pSCMgr->pLogMgr);
        return (rv);
    }
    
    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerGetComprtmentSize(handlerMgr=0x%x,hComp=0x%x)",pCHMgr,hCompartment));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerGetComprtmentSize */

/*************************************************************************
* SigCompCompartmentHandlerAddStateToCompartment
* ------------------------------------------------------------------------
* General: this function adds the given state to the given compartments list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           stateID - The ID of the state
*           statePriority - provided in the END_MESSAGE UDVM command (RFC-3320 section 9.4.9)
*           stateMinAccessLength - same as above parameter
*           stateAddress - same as above parameter
*           stateInstruction - same as above parameter
*           stateDataSize - The size of the content buffer in bytes
*           pfnComparator - state data compare callback function
*           *pComparatorCtx - the above function's context
*
* Output:   **ppStateData - pointer to pointer to the state's content buffer
*************************************************************************/
RvStatus SigCompCompartmentHandlerAddStateToCompartment(
                                   IN  SigCompCompartmentHandlerMgr  *pCHMgr,
                                   IN  SigCompCompartment            *pCompartment,
                                   IN  RvSigCompStateID              stateID,
                                   IN  RvUint16                      statePriority,
                                   IN  RvUint16                      stateMinAccessLength,
                                   IN  RvUint16                      stateAddress,
                                   IN  RvUint16                      stateInstruction,
                                   IN  RvUint16                      stateDataSize,
                                   IN  SigCompUdvmStateCompare       pfnComparator,
                                   IN  void                          *pComparatorCtx,
                                   OUT RvUint8                       **ppStateData)
{
    SigCompState            *pState         = NULL;
    SigCompCompartmentState *pStateToDelete = NULL;
    RvLogMgr                *pLogMgr;
    SigCompCompartmentState *pListStateItem;
    SigCompCompartmentState *pListElement;
    RLIST_ITEM_HANDLE       hElement;
    RvStatus                rv;
    
    if ((NULL == pCHMgr) || (NULL == pCompartment)|| (NULL == ppStateData))
    {
        return(RV_ERROR_NULLPTR);
    }
    if ((stateMinAccessLength <  6) ||
        (stateMinAccessLength > 20))
    {
        return(RV_ERROR_BADPARAM);
    }

    pLogMgr = pCHMgr->pSCMgr->pLogMgr;

    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerAddStateToCompartment(handlerMgr=0x%x,hComp=0x%x,stateID=%d)",
		pCHMgr,pCompartment,stateID));
    
    /* the compartment is locked by a higher level function */
    
    /* create state */
    rv = SigCompStateHandlerCreateState(&pCHMgr->pSCMgr->stateHandlerMgr,
                                        stateDataSize,
                                        &pState);
    if (rv != RV_OK)
    {
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
            "SigCompCompartmentHandlerAddStateToCompartment - failed to create state for compartment 0x%p (rv=%d)",
            pCompartment,rv));
        return(rv);
    }
    RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
        "SigCompCompartmentHandlerAddStateToCompartment - state created (pState=%d stateID=0x%.2X 0x%.2X 0x%.2X stateDataSize=%d)",
        pState,
        stateID[0],
        stateID[1],
        stateID[2],
        stateDataSize));
    /* fill the state's information */
    pState->minimumAccessLength = stateMinAccessLength;
    pState->stateAddress        = stateAddress;
    pState->stateInstruction    = stateInstruction;
    *ppStateData                = pState->pData;
    memcpy(pState->stateID,stateID,sizeof(RvSigCompStateID));

    /* try to find duplicate state in the compartment's list */
    rv = FindDuplicateStateInList(pCompartment, 
                                  pState,
                                  pfnComparator,
                                  pComparatorCtx,
                                  &pListElement);
    if (rv == RV_ERROR_DUPLICATESTATE)
    {
        /* state was already found in the same compartment's list */
        /* thus: */
        
        RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerAddStateToCompartment - found duplicate state in compartment (pState=%d stateID=0x%X 0x%X 0x%X)",
            pState,
            stateID[0],
            stateID[1],
            stateID[2]));
        /* update the retention priority
            according to <draft-ietf-rohc-sigcomp-impl-guide-02> section 5.2 */
        pListElement->stateRetentionPriority = statePriority;
        /* free the temporary allocated state */
        rv = SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, 
                                            pState, 
                                            RV_FALSE);
        if (rv != RV_OK)
        {
            RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
                "SigCompCompartmentHandlerAddStateToCompartment - failed to delete temporary state (rv=%d)",
                rv));
        }
        /* set the data-pointer to NULL */
        *ppStateData = NULL;
        
        RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,"SigCompCompartmentHandlerAddStateToCompartment"));
        return(RV_OK);
    }
    else if(rv != RV_OK)
    {
        /* error in the called function */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
            "SigCompCompartmentHandlerAddStateToCompartment - FindDuplicateStateInList failed (rv=%d)",
            rv));
        return(rv);
    }
    /* no such state has been found in the compartment's state-list */
    RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
        "SigCompCompartmentHandlerAddStateToCompartment - state is not in the compartment (pCompartment=%d pState=%d stateID=0x%X 0x%X 0x%X)",
        pCompartment,
        pState,
        stateID[0],
        stateID[1],
        stateID[2]));
    
    RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
        "SigCompCompartmentHandlerAddStateToCompartment - requiredSpace=%d  availableSpace=%d ",
        stateDataSize+64,
        pCompartment->availableStateMemory));
    /* check for available space in the compartment */
    /* according to RFC-3320 section 6.2, end of second paragraph:
            For the purpose of calculation, each state item
            is considered to cost (state_length + 64) bytes.*/
    while (pCompartment->availableStateMemory < ((RvUint32)(pState->stateDataSize) + 64))
    {
        /* not enough space available */
        /* thus : */

        RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerAddStateToCompartment - freeing space for new state (availableMem=%d requiredMem=%d)",
            pCompartment->availableStateMemory,
            ((RvUint32)(pState->stateDataSize) + 64)));
        /* get lowest priority state */
        rv = GetLowestPriorityStateItemFromList(pCHMgr->hStateListPool,
                                                pCompartment->hStatesList,
                                                pCHMgr->pSCMgr->pLogMgr,
                                                &pStateToDelete);
        if (rv != RV_OK)
        {
            /* ignore irrelevant (in this context) return value */
            /*lint -e{534}*/
            SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, pState, RV_FALSE);            
            *ppStateData = NULL;
            SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
            RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
                "SigCompCompartmentHandlerAddStateToCompartment - failed in GetLowestPriorityStateItemFromList (rv=%d)",
                rv));
            return(rv);
        }

        RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerAddStateToCompartment - deleting state from compartment (pState=%d stateID=0x%X 0x%X 0x%X)",
            pStateToDelete->pState,
            pStateToDelete->pState->stateID[0],
            pStateToDelete->pState->stateID[1],
            pStateToDelete->pState->stateID[2]));
        /* remove state from compartment's list */
        rv = SigCompCompartmentHandlerRemoveStateFromCompartment(pCHMgr,
                                                                 pCompartment,
                                                                 pStateToDelete->pState->stateID,
                                                                 sizeof(pStateToDelete->pState->stateID));
        if (rv != RV_OK)
        {
            RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
                "SigCompCompartmentHandlerAddStateToCompartment - failed to remove state from list (rv=%d)",
                rv));
            /* ignore irrelevant (in this context) return value */
            /*lint -e{534}*/
            SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, pState, RV_FALSE);            
            *ppStateData = NULL;
            return(rv);
        }
        
    } /* end of WHILE loop to free state memory cased on retention priority */

    /* save the state */
    rv = SigCompStateHandlerSaveState(&pCHMgr->pSCMgr->stateHandlerMgr, 
                                      pfnComparator,
                                      pComparatorCtx,
                                      &pState);
    if (rv == RV_ERROR_DUPLICATESTATE) 
    {
        /* returns OK to save memcpy operations, since state already exists */
        *ppStateData = NULL;
        RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerAddStateToCompartment - state already exists, count incremented (count=%d)",
            pState->isUsed));
    }
    else if (rv != RV_OK)
    {
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
            "SigCompCompartmentHandlerAddStateToCompartment - failed to save state (rv=%d)",
            rv));
        /* ignore irrelevant (in this context) return value */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, pState, RV_TRUE);            
        *ppStateData = NULL;
        return(rv);
    }
    
    /* insert state to the list of states correlated to the compartment */
    rv = RLIST_InsertTail(pCHMgr->hStateListPool,
                          pCompartment->hStatesList,
                          &hElement);
    if (rv != RV_OK)
    {
        /* return the state to release the lock */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
            "SigCompCompartmentHandlerAddStateToCompartment - failed to insert state into list (rv=%d)",
            rv));
        /* ignore irrelevant (in this context) return value */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, pState, RV_TRUE);            
        *ppStateData = NULL;
        return(rv);
    }
    
    /* let the element point to the state */
    pListStateItem = (SigCompCompartmentState *)hElement;
    pListStateItem->pState = pState;
    pListStateItem->counter = ++(pCompartment->counter);
    pListStateItem->stateRetentionPriority = statePriority;
    pListStateItem->hStatesList = pCompartment->hStatesList;

    /* update the availableStateMemory */
    pCompartment->availableStateMemory -= (pState->stateDataSize + 64);
    
    RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
        "SigCompCompartmentHandlerAddStateToCompartment - state saved (ID=%d,addr=%d,minAccess=%d)",
		stateID,stateAddress,stateMinAccessLength));

    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerAddStateToCompartment(handlerMgr=0x%x,hComp=0x%x,stateID=%d)",
		pCHMgr,pCompartment,stateID));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerAddStateToCompartment */

/*************************************************************************
* SigCompCompartmentHandlerRemoveStateFromCompartment
* ------------------------------------------------------------------------
* General: this function removes the given state from the given compartments list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - The ID of the compartment
*           pPartialStateID - pointer to the partial (6/9/12/20) ID of the state
*           partialStateIdLength - length of the partial ID, can be one of: 6/9/12/20 bytes
*************************************************************************/
RvStatus SigCompCompartmentHandlerRemoveStateFromCompartment(
                                    IN SigCompCompartmentHandlerMgr *pCHMgr,
                                    IN SigCompCompartment           *pCompartment,
                                    IN RvUint8                      *pPartialStateID,
                                    IN RvUint32                     partialStateIdLength)
{
    RvStatus                rv;
    SigCompCompartmentState *pListElement;
    SigCompState            *pState  = NULL;
    RvLogMgr                *pLogMgr;

    if ((NULL == pCHMgr) || (NULL == pCompartment) ||(NULL == pPartialStateID))
    {
        return(RV_ERROR_NULLPTR);
    }

    pLogMgr = pCHMgr->pSCMgr->pLogMgr;

    RvLogEnter(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerRemoveStateFromCompartment(handlerMgr=0x%x,pComp=0x%x,ID=%d)",
		pCHMgr,pCompartment,*pPartialStateID));
    
    /* lock the compartment */
    rv = SigCompCommonEntityLock(&pCompartment->lock, 
                                pLogMgr,
                                pCHMgr->pSCMgr->pLogSource,
                                pCHMgr->hCompartmentRA,
                                (RA_Element)pCompartment);
    if (rv != RV_OK) 
    {
        /* failed to lock compartment */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerRemoveStateFromCompartment - failed to lock compartment"));
        return(RV_ERROR_UNKNOWN);
    }
    
    
    rv = SigCompStateHandlerGetState(&pCHMgr->pSCMgr->stateHandlerMgr,
                                     pPartialStateID,
                                     partialStateIdLength,
                                     RV_TRUE, /* ignore miminumAccessLength */
                                     &pState);
    if (rv != RV_OK)
    {
        SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
        return(rv);
    }

    /* find state in the list */
    rv = FindStateInList(pCompartment, pState,&pListElement);
    if (rv != RV_OK)
    {
        /* state could not be found in the list */
        SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
        return(rv);
    }

    /* remove list element */
    RLIST_Remove(pCHMgr->hStateListPool,
                 pCompartment->hStatesList,
                 (RLIST_ITEM_HANDLE)pListElement);

    /* reclaim memory */
    pCompartment->availableStateMemory += (pState->stateDataSize + 64);
    
    /* delete state */
    rv = SigCompStateHandlerDeleteState(&pCHMgr->pSCMgr->stateHandlerMgr, pState, RV_TRUE);
    if (rv != RV_OK)
    {
        SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
        return(rv);
    }

    pListElement = NULL;
    RLIST_GetTail(pCHMgr->hStateListPool,
                  pCompartment->hStatesList,
                  (RLIST_ITEM_HANDLE *)&pListElement);
    if (pListElement == NULL) 
    {
        /* the list is empty */
        /* thus, reset counter */
        pCompartment->counter = 0;
    }

    SigCompCommonEntityUnlock(&pCompartment->lock, pLogMgr);
    
    RvLogLeave(pCHMgr->pSCMgr->pLogSource,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerRemoveStateFromCompartment(handlerMgr=0x%x,pComp=0x%x,ID=%d)",
		pCHMgr,pCompartment,*pPartialStateID));
    
    return RV_OK;
} /* end of SigCompCompartmentHandlerRemoveStateFromCompartment */

/*************************************************************************
* SigCompCompartmentHandlerPeerEPCapabilitiesUpdate
* ------------------------------------------------------------------------
* General: this function updates the algorithm structure with the
*          capabilities of the peer endpoint + chooses according to these
*          capabilities the appropriate compressing algorithm
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           remoteEpCapabilities - a byte holding the peer End Point (EP) capabilities:
*                            decompression memory size
*                            state memory size
*                            cycles per bit
*           sigcompVersion - version of the remote EP SigComp
*           *pLocalIdListSize - (in) requested size of list
*
* Output:   *pLocalIdListSize - (out) allocated size of list area (may be different than the requested value)
*           **ppLocalIdList - will contain a pointer to an area to hold the remote EP list of 
*                               locally available states (see RFC-3320 section 9.4.9. on structure of list)
*************************************************************************/
RvStatus SigCompCompartmentHandlerPeerEPCapabilitiesUpdate(
                                  IN    SigCompCompartmentHandlerMgr   *pCHMgr,
                                  IN    SigCompCompartment             *pCompartment,
                                  IN    RvUint8                        remoteEpCapabilities,
                                  IN    RvUint8                        sigcompVersion,
                                  INOUT RvUint32                       *pLocalIdListSize,
                                  OUT   RvUint8                        **ppLocalIdList)
{
    RvUint8  cpb = 0;
    RvUint32 dms = 0;
    RvUint32 sms = 0;
    
    if ((NULL == pCHMgr) ||
        (NULL == pCompartment) ||
        (NULL == pLocalIdListSize) ||
        (NULL == ppLocalIdList))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerPeerEPCapabilitiesUpdate(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    
    RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
        "SigCompCompartmentHandlerPeerEPCapabilitiesUpdate - before - SigCompVer=%d CPB=%d DMS=%d SMS=%d",
        pCompartment->remoteSigCompVersion,
        pCompartment->remoteCPB,
        pCompartment->remoteDMS,
        pCompartment->remoteSMS));

/*
        returned_parameters_location
        from: <draft-ietf-rohc-sigcomp-user-guide-00.txt>
        section 5.4.  Announcing additional resources

          0   1   2   3   4   5   6   7
        +---+---+---+---+---+---+---+---+
        |  cpb  |    dms    |    sms    |
        +---+---+---+---+---+---+---+---+
        |        SigComp_version        |
        +---+---+---+---+---+---+---+---+

        decoding according to: <RFC 3320 Signaling Compression (SigComp)>
        section 3.3.1.  Memory Size and UDVM Cycles
*/
    if (sigcompVersion == 0)
    {
        /* error - 0x01 is the minimal value */
        sigcompVersion = 0x01;
    }
    if (pCompartment->remoteSigCompVersion != sigcompVersion) 
    {
        pCompartment->remoteSigCompVersion = sigcompVersion;
        /* raise the flag to signal the compressor dispatcher */
        pCompartment->bRemoteCapabilitiesHaveChanged = RV_TRUE;
    }
    
    if (0 != remoteEpCapabilities) 
    {
        /* extract information from "compressed" bytes and update values if needed */

        cpb = (RvUint8)  (16 << (0x00000003 & ((remoteEpCapabilities & CPB_MASK) >> 6)));
        dms = (RvUint32) (1024 << (0x00000007 & ((remoteEpCapabilities & DMS_MASK) >> 3)));
        if (dms <= 1024)
        {
            /* error - all EPs implementing SigComp MUST offer DMS of at least 2048 bytes */
            RvLogWarning(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
                "SigCompCompartmentHandlerPeerEPCapabilitiesUpdate - illegal value DMS=%d (fixed to max 2048)",
                dms));
            dms = 2048;
        }
        sms = (RvUint32) (1024 << (remoteEpCapabilities & SMS_MASK));
        if (sms == 1024)
        {
            /* zero (0) value for a stateless EP */
            sms = 0;
        }
        
        if (pCompartment->remoteCPB != cpb) 
        {
            pCompartment->remoteCPB = cpb;
            /* raise the flag to signal the compressor dispatcher */
            pCompartment->bRemoteCapabilitiesHaveChanged = RV_TRUE;
        }
        if (pCompartment->remoteDMS != dms) 
        {
            pCompartment->remoteDMS = dms;
            /* raise the flag to signal the compressor dispatcher */
            pCompartment->bRemoteCapabilitiesHaveChanged = RV_TRUE;
        }
        
        if (pCompartment->remoteSMS != sms) 
        {
            pCompartment->remoteSMS = sms;
            /* raise the flag to signal the compressor dispatcher */
            pCompartment->bRemoteCapabilitiesHaveChanged = RV_TRUE;
        }
    }

    if (pCompartment->bRemoteCapabilitiesHaveChanged == RV_TRUE) 
    {
        RvLogDebug(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerPeerEPCapabilitiesUpdate - after  - SigCompVer=%d CPB=%d DMS=%d SMS=%d",
            pCompartment->remoteSigCompVersion,
            pCompartment->remoteCPB,
            pCompartment->remoteDMS,
            pCompartment->remoteSMS));
    }
    
    /* return the pointers so that the caller (UDVM) can copy the list into the buffer */
    *pLocalIdListSize = sizeof(pCompartment->remoteIdList);
    *ppLocalIdList    = (RvUint8 *)pCompartment->remoteIdList;

    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerPeerEPCapabilitiesUpdate(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerPeerEPCapabilitiesUpdate */

/*************************************************************************
* SigCompCompartmentHandlerClearBytecodeFlag
* ------------------------------------------------------------------------
* General: clears (reset) the resendBytecode flag of the given compartment,
*           This function may be used for recovery of connection with the remote peer
*
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           hCompartment - The handle of the compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerClearBytecodeFlag(
                                 IN SigCompCompartmentHandlerMgr *pCHMgr,
                                 IN RvSigCompCompartmentHandle   hCompartment)
{
    SigCompCompartment *pCompartment = (SigCompCompartment *)hCompartment;
    
    if ((NULL == pCHMgr) ||
        (NULL == hCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerClearBytecodeFlag(handlerMgr=0x%x,hComp=0x%x)",
		pCHMgr,hCompartment));
    

    pCompartment->bSendBytecodeInNextMessage = RV_TRUE;
    
    
    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerClearBytecodeFlag(handlerMgr=0x%x,hComp=0x%x)",
		pCHMgr,hCompartment));

    return(RV_OK);
} /* end of SigCompCompartmentHandlerClearBytecodeFlag */


/*************************************************************************
* SigCompCompartmentHandlerForwardRequestedFeedback
* ------------------------------------------------------------------------
* General: this function handles the forward requested feedback.
*           It is called from the decompressor in response to directions embedded
*           in the incoming message (after the END_MESSAGE command).
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           bKeepStatesInCompartment - reflects the S_bit in the header 
*                                   of the Requested Feedback Data (RFD)
*           bLocalStatesListIsNeeded - reflects the I_bit in the header of the RFD
*           requestedFeedbackItemSize - range = 0..127
*           *pRequestedFeedbackItem - the content of the item to be returned to 
*                                       the remote EP
* Notes:
*  If requestedFeedbackItemSize == 0 semantic is as following:
*     pRequestedFeedbackItem != 0 - retain the previous feedback item
*     otherwise - no feedback item
*************************************************************************/
RvStatus SigCompCompartmentHandlerForwardRequestedFeedback(
                           IN SigCompCompartmentHandlerMgr *pCHMgr,
                           IN SigCompCompartment           *pCompartment,
                           IN RvBool                       bKeepStatesInCompartment,
                           IN RvBool                       bLocalStatesListIsNeeded,
                           IN RvUint16                     requestedFeedbackItemSize,
                           IN RvUint8                      *pRequestedFeedbackItem)
{
    
    if ((NULL == pCHMgr) || (NULL == pCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }
    if (requestedFeedbackItemSize > 127) 
    {
        return(RV_ERROR_BADPARAM);
    }
    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerForwardRequestedFeedback(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    
  
    /* 
    <From RFC-3320 section 9.4.9>
    
      If the S-bit to 1 the remote EP does not wish (or no longer
      wishes) to save state information at the receiving endpoint and also
      does not wish to access state information that it has previously
      saved. If the S-bit is set to 1 then the receiving
      endpoint can reclaim the state memory allocated to the remote
      compressor and set the state_memory_size for the compartment to 0

      If the I-bit is set to 1 the remote EP does not wish (or
      no longer wishes) to access any of the locally available state items
      offered by the receiving endpoint.
    */
    pCompartment->bKeepStatesInCompartment = bKeepStatesInCompartment;
    pCompartment->bSendLocalStatesIdList = bLocalStatesListIsNeeded;
    
    if(requestedFeedbackItemSize != 0 || pRequestedFeedbackItem == 0) {
        pCompartment->rqFeedbackItemLength = requestedFeedbackItemSize; 
    }

    if(requestedFeedbackItemSize) {
        memcpy(pCompartment->requestedFeedbackItem, pRequestedFeedbackItem, requestedFeedbackItemSize);
    }

    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerForwardRequestedFeedback(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    return(RV_OK);
} /* end of SigCompCompartmentHandlerForwardRequestedFeedback */

/*************************************************************************
* SigCompCompartmentHandlerForwardReturnedFeedbackItem
* ------------------------------------------------------------------------
* General: this function handles the forward requested feedback.
*           It is called from the decompressor in response to directions embedded
*           in the incoming message (after the END_MESSAGE command).
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           returnedFeedbackItemSize - range = 0..127
*           *pReturnedFeedbackItem - the content of the item returned from remote EP
*************************************************************************/
RvStatus SigCompCompartmentHandlerForwardReturnedFeedbackItem(
                                  IN SigCompCompartmentHandlerMgr *pCHMgr,
                                  IN SigCompCompartment           *pCompartment,
                                  IN RvUint16                     returnedFeedbackItemSize,
                                  IN RvUint8                      *pReturnedFeedbackItem)
{
    RvStatus rv;


    if ((NULL == pCHMgr) ||
        (NULL == pCompartment) ||
        (NULL == pReturnedFeedbackItem))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerForwardReturnedFeedbackItem(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    
    rv = SigCompCommonEntityLock(&pCompartment->lock, 
                                pCHMgr->pSCMgr->pLogMgr,
                                pCHMgr->pSCMgr->pLogSource,
                                pCHMgr->hCompartmentRA,
                                (RA_Element)pCompartment);
    if (rv != RV_OK) 
    {
        /* failed to lock compartment */
        RvLogError(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource ,
            "SigCompCompartmentHandlerForwardReturnedFeedbackItem - failed to lock compartment"));
        return(RV_ERROR_UNKNOWN);
    }
    
    pCompartment->rtFeedbackItemLength = returnedFeedbackItemSize;
    memcpy(pCompartment->returnedFeedbackItem, pReturnedFeedbackItem, returnedFeedbackItemSize);
    
    SigCompCommonEntityUnlock(&pCompartment->lock, pCHMgr->pSCMgr->pLogMgr);

    RvLogLeave(pCHMgr->pSCMgr->pLogSource ,(pCHMgr->pSCMgr->pLogSource,
		"SigCompCompartmentHandlerForwardReturnedFeedbackItem(handlerMgr=0x%x,pComp=0x%x)",
		pCHMgr,pCompartment));
    return(RV_OK)    ;
} /* end of SigCompCompartmentHandlerForwardReturnedFeedbackItem */



/*************************************************************************
* SigCompCompartmentHandlerGetCompartmentStatus
* ------------------------------------------------------------------------
* General: Call this function to get the compartment's status           
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           returnedFeedbackItemSize - range = 0..127
*           *pReturnedFeedbackItem - the content of the item returned from remote EP
*************************************************************************/
RVAPI RvStatus SigCompCompartmentHandlerGetCompartmentStatus(
                                  IN  RvSigCompMgrHandle         hSCMgr,
                                  IN  RvSigCompCompartmentHandle hCompartment,
                                  OUT RvUint32                   *pNumStatesInCompartment)
{
    SigCompMgr          *pSCMgr = (SigCompMgr *)hSCMgr;
    SigCompCompartment  *pCompartment;
    RvStatus            rv;
    RvUint32            currNumOfUsedUserElement;
    
    if ((NULL == pSCMgr)||
        (NULL == hCompartment)||
        (NULL == pNumStatesInCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }

    rv = SigCompCompartmentHandlerGetComprtment(&pSCMgr->compartmentHandlerMgr, hCompartment, &pCompartment);
    if (rv != RV_OK)
    {
        return(rv);
    }

    *pNumStatesInCompartment = 0;

    if (NULL != pCompartment->hStatesList)
    {
        RLIST_GetNumOfElements(pSCMgr->compartmentHandlerMgr.hStateListPool,
                               pCompartment->hStatesList,
                               &currNumOfUsedUserElement);
        *pNumStatesInCompartment = currNumOfUsedUserElement;
    }
    
    rv = SigCompCompartmentHandlerSaveComprtment(&pSCMgr->compartmentHandlerMgr, pCompartment);
    if (RV_OK != rv)
    {
        SigCompCommonEntityUnlock(&pCompartment->lock, pSCMgr->pLogMgr);
        return (rv);
    }

    return(RV_OK);
}




/***************************************************************************
* FindStateInList
* ------------------------------------------------------------------------
* General: find the element in the states list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pCompartment - pointer to the compartment
*            *pState - pointer to the state to be found
*
* Output:    *phListElement - pointer to the handle of the list element containing the state
***************************************************************************/
static RvStatus FindStateInList(IN  SigCompCompartment      *pCompartment,
                                IN  SigCompState            *pState,
                                OUT SigCompCompartmentState **ppListState)
{
    SigCompCompartmentState *pFirstElement   = NULL;
    SigCompCompartmentState *pCurrentElement = NULL;
    SigCompCompartmentState *pNextElement    = NULL;

    if ((NULL == pCompartment)||
        (NULL == pState)||
        (NULL == ppListState))
    {
        return(RV_ERROR_NULLPTR);
    }

    *ppListState = NULL;

    RLIST_GetHead(pCompartment->pCHMgr->hStateListPool,
                  pCompartment->hStatesList,
                  (RLIST_ITEM_HANDLE *)&pFirstElement);
    if (pFirstElement == NULL)
    {
        /* list is empty */
        return(RV_ERROR_UNKNOWN);
    }

    pCurrentElement = pFirstElement;
    while(pCurrentElement != NULL)
    {
        if(pCurrentElement->pState == pState)
        {
            break;
        }
        RLIST_GetNext(pCompartment->pCHMgr->hStateListPool,
                      pCompartment->hStatesList,
                      (RLIST_ITEM_HANDLE)pCurrentElement,
                      (RLIST_ITEM_HANDLE *)&pNextElement);
        pCurrentElement = pNextElement;
    }

    if(pCurrentElement == NULL)
    {
        /* reached end of list w/o finding the state */
        *ppListState = NULL;
        return(RV_ERROR_UNKNOWN);
    }
    
    *ppListState = pCurrentElement;

    return(RV_OK);
} /* end of FindStateInList */


/***************************************************************************
* FindDuplicateStateInList
* ------------------------------------------------------------------------
* General: find duplicate state in the states list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pCompartment - pointer to the compartment
*            *pState - pointer to the state to be found
*            *pComparatorCtx - comparator function context
*                              pass NULL when using memcmp
*            **ppState - a pointer to the state structure.
*                        if the state already exists, the new state will be deleted
*                        and a pointer to the existing (identical) state will be returned.
*
* Output:    *phListElement - pointer to the handle of the list element containing the state
***************************************************************************/
static RvStatus FindDuplicateStateInList(IN  SigCompCompartment      *pCompartment,
                                         IN  SigCompState            *pState,
                                         IN  SigCompUdvmStateCompare pfnComparator,
                                         IN  void                    *pCtx,
                                         OUT SigCompCompartmentState **ppListState)
{
    SigCompCompartmentState *pFirstElement          = NULL;
    SigCompCompartmentState *pCurrentElement        = NULL;
    SigCompCompartmentState *pNextElement           = NULL;
    RvBool                  bComparatorReturnValue  = RV_FALSE;
    
    if ((NULL == pCompartment)||
        (NULL == pState)||
        (NULL == ppListState))
    {
        return(RV_ERROR_NULLPTR);
    }

    *ppListState = NULL;

    RLIST_GetHead(pCompartment->pCHMgr->hStateListPool,
                  pCompartment->hStatesList,
                  (RLIST_ITEM_HANDLE *)&pFirstElement);
    if (pFirstElement == NULL)
    {
        /* list is empty */
        RvLogDebug(pCompartment->pCHMgr->pSCMgr->pLogSource ,(pCompartment->pCHMgr->pSCMgr->pLogSource ,
            "FindDuplicateStateInList - list is empty"));
        *ppListState = NULL;
        return(RV_OK);
    }

    pCurrentElement = pFirstElement;
    while(pCurrentElement != NULL)
    {
        if((memcmp(pCurrentElement->pState->stateID, pState->stateID, sizeof(RvSigCompStateID))) == 0)
        {
            /* both states have the same SHA1 key (20 bytes long) */
            if (pfnComparator == NULL) 
            {
                /* use memcmp ansi function to compare data */
                if (memcmp(pState->pData,pCurrentElement->pState->pData,pState->stateDataSize) == 0) 
                {
                    bComparatorReturnValue = RV_TRUE;
                }
            }
            else 
            {
                /* use supplied comparator callback function */
                bComparatorReturnValue = pfnComparator(pCtx, 
                                                       pCurrentElement->pState->pData, 
                                                       pCurrentElement->pState->stateDataSize);
            }
            
            if (bComparatorReturnValue == RV_TRUE)
            {
                /* both states hold the same data */
                /* thus: */
                *ppListState = pCurrentElement;
                RvLogDebug(pCompartment->pCHMgr->pSCMgr->pLogSource ,(pCompartment->pCHMgr->pSCMgr->pLogSource ,
                    "FindDuplicateStateInList - duplicate state found (existing pState=%d)",
                    pCurrentElement->pState));
                return(RV_ERROR_DUPLICATESTATE);
            }
            else
            {
                /* states have same SHA1 key but different data - collision */
                *ppListState = NULL;
                RvLogDebug(pCompartment->pCHMgr->pSCMgr->pLogSource ,(pCompartment->pCHMgr->pSCMgr->pLogSource ,
                    "FindDuplicateStateInList - states have same SHA1 key but different data - collision"));
                return(RV_ERROR_UNKNOWN);
            }
        } /* end of IF to compare states */

        RLIST_GetNext(pCompartment->pCHMgr->hStateListPool,
                      pCompartment->hStatesList,
                      (RLIST_ITEM_HANDLE)pCurrentElement,
                      (RLIST_ITEM_HANDLE *)&pNextElement);
        pCurrentElement = pNextElement;
    } /* end of WHILE loop on all elements in list */

    if(pCurrentElement == NULL)
    {
        /* reached end of list w/o finding the state */
        *ppListState = NULL;
        RvLogDebug(pCompartment->pCHMgr->pSCMgr->pLogSource ,(pCompartment->pCHMgr->pSCMgr->pLogSource ,
            "FindDuplicateStateInList - reached end of list without finding the state"));
    }
    
    *ppListState = pCurrentElement;

    return(RV_OK);
} /* end of FindDuplicateStateInList */


/*************************************************************************
* GetLowestPriorityStateItemFromList
* ------------------------------------------------------------------------
* General: Gets the lowest-priority state from a list of states.
*           Chooses the lowest state priority. In case of a tie, the state created
*           first, is chosen.
*           To be used by the compartment handler.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hStateListPool - handle to the stateList pool
*           hStatesList - handle to the stateList
*           *pLogMgr - log manager for logging
*
* Output:   **ppState - the pointer to the lowest priority state will be returned there
*************************************************************************/
static RvStatus GetLowestPriorityStateItemFromList(
                                 IN  RLIST_POOL_HANDLE       hStateListPool,
                                 IN  RLIST_HANDLE            hStatesList,
                                 IN  RvLogMgr                *pLogMgr,
                                 OUT SigCompCompartmentState **ppState)
{
    SigCompCompartmentState *pKeepState;
    SigCompCompartmentState *pTempState;
    RLIST_ITEM_HANDLE       hNextState;
    RLIST_ITEM_HANDLE       hCurrentState;
    RvUint32                compareValueKeep = 0; /* this will be a combination of priority and order of creation */
    RvUint32                compareValueTemp = 0;

    /* argument validity tests */
    if ((NULL == hStateListPool) ||
        (NULL == hStatesList) ||
        (NULL == ppState) )
    {
        return(RV_ERROR_NULLPTR);
    }

    /* get first element from list */
    RLIST_GetHead(hStateListPool, hStatesList, &hCurrentState);
    if (NULL == hCurrentState)
    {
        /* empty list */
        return(RV_ERROR_BADPARAM);
    }
    pKeepState = (SigCompCompartmentState *)hCurrentState;
    pTempState = pKeepState;
    /* combine priority and order of state creation */
    if (pKeepState->stateRetentionPriority == 65535) 
    {
        /* local state */
        compareValueKeep = 0;
    }
    else 
    {
        compareValueKeep = pKeepState->stateRetentionPriority;
        compareValueKeep = (compareValueKeep << 16) | (pKeepState->counter);
    }
    
    /* go through the entire list */
    do
    {
        /* keep the state with lowest priority */
        compareValueTemp = pTempState->stateRetentionPriority;
        compareValueTemp = (compareValueTemp << 16) | (pTempState->counter);
        if (pTempState->stateRetentionPriority == 65535) 
        {
            /* local state */
            compareValueTemp = 0;
        }
        if (compareValueKeep > compareValueTemp)
        {
            pKeepState = pTempState;
            compareValueKeep = pKeepState->stateRetentionPriority;
            compareValueKeep = (compareValueKeep << 16) | (pKeepState->counter);
        }

        RLIST_GetNext(hStateListPool, hStatesList, hCurrentState, &hNextState);
        pTempState = (SigCompCompartmentState *)hNextState;
        hCurrentState = hNextState;
    } while(NULL != hNextState);

    *ppState = pKeepState;

    RV_UNUSED_ARG(pLogMgr);

    return(RV_OK);
} /* end of GetLowestPriorityStateItemFromList */


