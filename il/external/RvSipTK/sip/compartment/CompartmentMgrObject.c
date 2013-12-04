
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
 *                      <CompartmentMgrObject.c>
 *
 * This file defines the compartment manager object,that holds all the
 * Compartments resources including memory pool, pools of lists, locks and
 * more. Its main functionality is to manage the compartments list, to create new
 * compartments and to associate stack objects to a specific compartment.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvmemory.h"
#include "rvassert.h"
#include "_SipCommonCore.h"
#include "CompartmentObject.h"
#include "RvSipCompartmentTypes.h"
#include "CompartmentMgrObject.h"
#include "CompartmentCallback.h"
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

static RvStatus RVCALLCONV CreateCompartmentList(
                               IN CompartmentMgr *pMgr,
                               IN RvInt32        maxNumOfCompartments);

static RvStatus RVCALLCONV CreateCompartmentHash(
                               IN CompartmentMgr *pMgr,
                               IN RvInt32        maxNumOfCompartments);

static RvStatus RVCALLCONV DestructCompartmentList(
                                    IN CompartmentMgr *pMgr);

static RvStatus RVCALLCONV DestructCompartmentHash(
									IN CompartmentMgr *pMgr);


static RvUint CompHashFunction(IN void *key);

static RvBool CompHashCompare(	IN void *newHashElement,
				                IN void *oldHashElement);

static RvStatus CompartmentSetHashKey (
									IN  CompartmentMgr					*pMgr,
									IN  RPOOL_Ptr						*pUrn,
									IN  RvInt32							urnLen,
									IN  HPAGE							hPageForUrn,
									IN  RvInt32							offsetInPage,
									IN  SipTransportAddress				*pAddr,
									IN  RvSipTransportConnectionHandle	hConn,
									OUT CompartmentHashKeyElement		*pKeyElem);

static RvBool IsConnectionless (SipTransportAddress *pAddr);

static RvStatus CalcCompartmentType (
									IN  RPOOL_Ptr                       *pUrn,
									IN  SipTransportAddress             *pAddr,
									OUT RvSipCompartmentType            *pCompType);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CompartmentMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new Compartment manager. The compartment
 *          manager is in charge of all the compartments. The manager
 *          holds a list of compartments and is used to create new
 *          compartments.
 * Return Value: RV_ERROR_NULLPTR     - The pointer to the compartment mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   Compartment manager.
 *               RV_OK        - Compartment manager was
 *                                   created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     cfg           - A pointer to the configuration structure,
 *                          containing all the required init. information.
 *          hStack        - A handle to the stack instance.
 * Output:  phCompartmentMgr - Pointer to the newly created compartment manager.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrConstruct(
                IN  SipCompartmentMgrCfg      *cfg,
                IN  void                      *hStack,
                OUT RvSipCompartmentMgrHandle *phCompartmentMgr)
{
    RvStatus         crv;
    RvStatus         rv;
    CompartmentMgr  *pMgr;

    *phCompartmentMgr = NULL;

    /*------------------------------------------------------------------------
       allocates the Compartment manager structure and initialize it
    --------------------------------------------------------------------------*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(CompartmentMgr), cfg->pLogMgr, (void*)&pMgr))
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                  "CompartmentMgrConstruct - Failed ,RvMemoryAlloc failed"));
        return RV_ERROR_UNKNOWN;
    }

    /* Initializing the struct members to NULL (0) */
    memset(pMgr, 0, sizeof(CompartmentMgr));

    /* Init Compartment Mgr parameters*/
    pMgr->hStack                     = hStack;
    pMgr->pLogSrc                    = cfg->pLogSrc;
    pMgr->pLogMgr                    = cfg->pLogMgr;
    pMgr->maxNumOfCompartments       = cfg->maxNumOfCompartments;
	pMgr->pSeed                      = cfg->pSeed;

    crv = RvMutexConstruct(cfg->pLogMgr,&(pMgr->lock));
    if (RV_OK != crv)
    {
        RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                  "CompartmentMgrConstruct - Failed ,RvMutexConstruct returned %d",crv));

        RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
        return CRV2RV(crv);
    }

    if(pMgr->maxNumOfCompartments != 0)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		/* initialize HRPOOL */
		pMgr->hCompartmentPool = RPOOL_Construct(RVSIP_COMPARTMENT_MAX_URN_LENGTH, cfg->maxNumOfCompartments*2,
			(RV_LOG_Handle)pMgr->pLogMgr, RV_TRUE, "Compartment Pool" );
#else

		/* initialize HRPOOL */
		pMgr->hCompartmentPool = RPOOL_Construct(RVSIP_COMPARTMENT_MAX_URN_LENGTH, cfg->maxNumOfCompartments,
			(RV_LOG_Handle)pMgr->pLogMgr, RV_TRUE, "Compartment Pool" );
#endif
/* SPIRENT_END */
		if (NULL == pMgr->hCompartmentPool)
		{
            RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                "CompartmentMgrConstruct - Failed to create Compartment pool"));
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
		}

        /*-------------------------------------------------------
           Initialize Compartment list
        ---------------------------------------------------------*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        rv = CreateCompartmentList(pMgr, cfg->maxNumOfCompartments*2);
#else
        rv = CreateCompartmentList(pMgr, cfg->maxNumOfCompartments);
#endif
/* SPIRENT_END */
        if(rv != RV_OK)
        {
            RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                "CompartmentMgrConstruct - Failed to initialize Compartment list (rv=%d)",rv));
			RPOOL_Destruct(pMgr->hCompartmentPool);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* -------------------------------------------
		     Create and initialize compartment hash
          --------------------------------------------*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		rv = CreateCompartmentHash(pMgr, cfg->maxNumOfCompartments*2);
#else
		rv = CreateCompartmentHash(pMgr, cfg->maxNumOfCompartments);
#endif
		if (rv != RV_OK)
		{
            RvLogError(cfg->pLogSrc,(cfg->pLogSrc,
                "CompartmentMgrConstruct - Failed to initialize Compartment hash (rv=%d)",rv));
			DestructCompartmentList(pMgr);
			RPOOL_Destruct(pMgr->hCompartmentPool);
            RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
		}
	}

    RvLogInfo(cfg->pLogSrc,(cfg->pLogSrc,
              "CompartmentMgrConstruct - Compartment manager was constructed successfully, Num of Compartments = %d",
              pMgr->maxNumOfCompartments));

    *phCompartmentMgr = (RvSipCompartmentMgrHandle)pMgr;

    return RV_OK;
}

/***************************************************************************
 * CompartmentMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Compartment manager freeing all resources.
 * Return Value:  RV_OK - Compartment manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCompartmentMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrDestruct(
                            IN RvSipCompartmentMgrHandle hCompartmentMgr)
{
    RvStatus        rv;
    RvLogSource*    pLogSrc;
    CompartmentMgr *pMgr = (CompartmentMgr *)hCompartmentMgr;

    LOCK_MGR(pMgr); /*lock the manager*/
    pLogSrc = pMgr->pLogSrc;

    rv = DestructCompartmentList(pMgr);
    if (RV_OK != rv)
    {
        RvLogError(pLogSrc,(pLogSrc,
              "CompartmentMgrDestruct - Compartment manager failed to destruct the list (rv=%d)",rv));
        UNLOCK_MGR(pMgr); /*unlock the manager*/
        return rv;
    }

	rv = DestructCompartmentHash(pMgr);
    if (RV_OK != rv)
    {
        RvLogError(pLogSrc,(pLogSrc,
              "CompartmentMgrDestruct - Compartment manager failed to destruct the hash (rv=%d)",rv));
        UNLOCK_MGR(pMgr); /*unlock the manager*/
        return rv;
    }

	RPOOL_Destruct(pMgr->hCompartmentPool);

    UNLOCK_MGR(pMgr); /*unlock the manager*/

    RvMutexDestruct(&pMgr->lock,pMgr->pLogMgr); /* Destruct the manager lock */

    /* free Compartment mgr data structure */
    RvMemoryFree((void*)pMgr, pMgr->pLogMgr);

    RvLogInfo(pLogSrc,(pLogSrc,
              "CompartmentMgrDestruct - Compartment manager was destructed successfully"));

    return RV_OK;
}

/***************************************************************************
 * CompartmentMgrCreateCompartment
 * ------------------------------------------------------------------------
 * General: Creates a new Compartment.
 *          This function allocates and initialize the compartment.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create a new compartment. Not
 *                                   enough resources.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr           - Pointer to the compartment manager
 *			  pUrn           - The RPOOL containing the sigcomp-id of the compartment
 *			  pSrcAddr		 - The source address of the message created the compartment
 *			  pDstAddr		 - The destination address of the message created the compartment
 *            bIsIncoming    - The direction of the message, for ip:port key-ing
 *			  hConn			 - The TCP connection identifying the compartment
 *            hMsg           - The message created this compartment
 * Output:    phCompartment  - sip stack handle to the new compartment
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrCreateCompartment(
						IN  CompartmentMgr					*pMgr,
						IN  RPOOL_Ptr						*pUrn,
						IN  SipTransportAddress				*pSrcAddr,
						IN  SipTransportAddress				*pDstAddr,
						IN  RvBool                          bIsIncoming,
						IN  RvSipMsgHandle                  hMsg,
						IN  RvSipTransportConnectionHandle	hConn,
						OUT RvSipCompartmentHandle			*phCompartment)
{
    RvStatus		 		rv;
    Compartment				*pCompartment;
	RvInt32			    	urnLen = 0;
	SipTransportAddress     *pAddr;
	RvBool                  bIsApproved;

    if (NULL == pMgr || NULL == phCompartment)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phCompartment = NULL;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CompartmentMgrCreateCompartment - New Compartment is about to be created"));

    LOCK_MGR(pMgr);

    if(pMgr->hCompartmentListPool == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - Failed. The compartment manager was initialized with 0 compartments"));
        UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
    }

    if(pMgr->hCompartmentHash == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - Failed. The compartment manager was initialized with no hash"));
        UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
    }

    rv = RLIST_InsertTail(pMgr->hCompartmentListPool,pMgr->hCompartmentList,(RLIST_ITEM_HANDLE *)&pCompartment);

    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - Failed to add new compartment to list (rv=%d)", rv));
        UNLOCK_MGR(pMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    rv = CompartmentInitialize(pCompartment,pMgr);
	if (rv != RV_OK)
    {
		RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - Failed to initialize the new compartment (rv=%d)",
                  rv));
	    UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
    }

	if (pUrn != NULL)
	{
		urnLen = RPOOL_Strlen(pUrn->hPool, pUrn->hPage, pUrn->offset);
		rv = RPOOL_GetPage(pMgr->hCompartmentPool, 0, &(pCompartment->hPageForUrn));
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentMgrCreateCompartment - Failed to get new page (rv=%d)", rv));
	        UNLOCK_MGR(pMgr);
			return rv;
		}
		rv = RPOOL_Append(pMgr->hCompartmentPool, pCompartment->hPageForUrn, urnLen+1, RV_FALSE,
				&pCompartment->offsetInPage);
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentMgrCreateCompartment - Failed to append to page (rv=%d)", rv));
			RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	        UNLOCK_MGR(pMgr);
			return rv;
		}
		rv = RPOOL_CopyFrom(pMgr->hCompartmentPool, pCompartment->hPageForUrn, pCompartment->offsetInPage,
					pUrn->hPool, pUrn->hPage, pUrn->offset, urnLen+1);
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentMgrCreateCompartment - Failed to copy sigcomp-id (rv=%d)", rv));
			RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	        UNLOCK_MGR(pMgr);
			return rv;
		}
	}

	RvAddressCopy (&pSrcAddr->addr, &pCompartment->srcAddr.addr);
	pCompartment->srcAddr.eTransport = pSrcAddr->eTransport;
	RvAddressCopy (&pDstAddr->addr, &pCompartment->dstAddr.addr);
	pCompartment->dstAddr.eTransport = pDstAddr->eTransport;
	pCompartment->hConnection = hConn;
	pCompartment->bIsSrc      = bIsIncoming;
	pCompartment->hMsg        = hMsg;

	*phCompartment = (RvSipCompartmentHandle)pCompartment;

	pAddr = (bIsIncoming == RV_TRUE)?pSrcAddr:pDstAddr;

	CalcCompartmentType(pUrn,pAddr,&pCompartment->compType);
	if ((RVSIP_COMPARTMENT_TYPE_CONNECTION == pCompartment->compType) && (hConn == NULL))
	{
		RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
			"CompartmentMgrCreateCompartment - Conpartment 0x%p, connection not valid yet", pCompartment));
	}

    UNLOCK_MGR(pMgr);

	/* ask application to approve compartment creation */
	/* lock the compartment */
	rv = CompartmentLockMsg(pCompartment);
	if (rv != RV_OK)
	{
		RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
			"CompartmentMgrCreateCompartment - failed to lock compartment 0x%p. rv=%d.",
                    pCompartment, rv));
		LOCK_MGR(pMgr);
	    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList, (RLIST_ITEM_HANDLE)pCompartment);
		UNLOCK_MGR(pMgr);
        return rv;
	}

	CompartmentCallbackCreatedEv (pMgr, (RvSipCompartmentHandle)pCompartment, &bIsApproved);
	if (RV_FALSE == bIsApproved)
	{
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - compartment 0x%p was not approved by application",
				  pCompartment));
		CompartmentUnLockMsg(pCompartment);
		LOCK_MGR(pMgr);
		RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList, (RLIST_ITEM_HANDLE)pCompartment);
		UNLOCK_MGR(pMgr);
		return (RV_ERROR_UNKNOWN);
	}
	else
	{
		pCompartment->bIsApproved = RV_TRUE;
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrCreateCompartment - compartment 0x%p approved by application",
				  pCompartment));
		CompartmentUnLockMsg(pCompartment);
	}

	return RV_OK;
}

/***************************************************************************
 * CompartmentMgrFindCompartment
 * ------------------------------------------------------------------------
 * General: Finds a compartment in the hash
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr           - Pointer to the compartment manager
 *            pUrn           - pointer to the RPOOL containing the sigcomp-id of the
 *							   compartment to look for
 *			  pAddr          - IP and port used to look for the compartment, if the sigcomp-id
 *							   does not exist.
 *            hConn          - The TCP connection used to look for the compartment, for TCP connections
 * Output:    phCompartment  - sip stack handle to the new compartment
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrFindCompartment (
   						IN  CompartmentMgr					*pMgr,
						IN  RPOOL_Ptr						*pUrn,
						IN  SipTransportAddress				*pAddr,
						IN  RvSipTransportConnectionHandle	hConn,
						OUT RvSipCompartmentHandle			*phCompartment)
{
    RvStatus					rv;
	CompartmentHashKeyElement	keyElem;
	RvBool						bFound;
	RvInt32						urnLen = 0;
	RvInt32						offsetInPage = 0;
	HPAGE						hTempPage = NULL_PAGE;
	void						*pElement;

    if (NULL == pMgr || NULL == phCompartment)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	*phCompartment = NULL;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CompartmentMgrFindCompartment - looking for compartment"));

	LOCK_MGR(pMgr);

    if(pMgr->hCompartmentListPool == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrFindCompartment - Failed. The compartment manager was initialized with 0 compartments"));
		UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
    }

    if(pMgr->hCompartmentHash == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrFindCompartment - Failed. The compartment manager was initialized with no hash"));
		UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
    }

	if (pUrn != NULL)
	{
		urnLen = RPOOL_Strlen(pUrn->hPool, pUrn->hPage, pUrn->offset);
		rv = RPOOL_GetPage(pMgr->hCompartmentPool, 0, &hTempPage);
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentMgrFindCompartment - Failed to get new page (rv=%d)", rv));
			UNLOCK_MGR(pMgr);
			return rv;
		}
		rv = RPOOL_Append(pMgr->hCompartmentPool, hTempPage, urnLen+1, RV_FALSE,
				&offsetInPage);
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentMgrFindCompartment - Failed to append to page (rv=%d)", rv));
			RPOOL_FreePage(pMgr->hCompartmentPool, hTempPage);
			UNLOCK_MGR(pMgr);
			return rv;
		}
	}

	memset (&keyElem, 0, sizeof(keyElem));
	CalcCompartmentType (pUrn, pAddr, &keyElem.keyType);
	rv = CompartmentSetHashKey(pMgr, pUrn, urnLen, hTempPage, offsetInPage, pAddr, hConn, &keyElem);
	if (rv != RV_OK)
	{
		RPOOL_FreePage(pMgr->hCompartmentPool, hTempPage);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrFindCompartment - Invalid hash key type"));
		UNLOCK_MGR(pMgr);
        return rv;
	}

	bFound = HASH_FindElement(pMgr->hCompartmentHash, &keyElem, CompHashCompare, &pElement);

	/* we can now free the temp hpage we allocated for the search */
	RPOOL_FreePage(pMgr->hCompartmentPool, hTempPage);

	if ((RV_FALSE == bFound) || (NULL == pElement))
	{
		*phCompartment = NULL;
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CompartmentMgrFindCompartment - compartment not found in the hash"));
		UNLOCK_MGR(pMgr);
        return(RV_ERROR_NOT_FOUND);
	}

	rv = HASH_GetUserElement(pMgr->hCompartmentHash, pElement, (void *)phCompartment);

	UNLOCK_MGR(pMgr);

    if(rv != RV_OK)
    {
        *phCompartment = NULL;
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CompartmentMgrFindCompartment - failed to get user part from hash entry"));
        return rv;
	}

	if (RV_FALSE == ((Compartment *)*phCompartment)->bIsApproved)
	{
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrFindCompartment - Compartment 0x%x was found but was not approved by application yet",
				  *phCompartment));
		*phCompartment = NULL;
		return (RV_ERROR_UNINITIALIZED);
	}

	RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
		"CompartmentMgrFindCompartment - found compartment 0x%p", *phCompartment));

	return RV_OK;
}

/***************************************************************************
 * CompartmentMgrDetachFromSigCompModule
 * ------------------------------------------------------------------------
 * General: Release all the relations between SipStack compartments and
 *          SigComp internal compartments.
 *
 * Note:    This function is called upon stack destruction. No lock is being done.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pMgr - Pointer to the comp. manager.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrDetachFromSigCompModule(
                                        IN  CompartmentMgr  *pMgr)
{
    RvInt32      currCompartment;
    Compartment *pCompartment;
    Compartment *pPrevCompartment;

    RLIST_GetHead(pMgr->hCompartmentListPool,
                  pMgr->hCompartmentList,
                  (RLIST_ITEM_HANDLE *)&pCompartment);

    /* Destruct locks of all calls */
    for (currCompartment=0;
         currCompartment < pMgr->maxNumOfCompartments && pCompartment != NULL;
         currCompartment++)
    {
        pCompartment->hInternalComp = NULL;
        pPrevCompartment = pCompartment;
        RLIST_GetNext(pMgr->hCompartmentListPool,
                      pMgr->hCompartmentList,
                      (RLIST_ITEM_HANDLE)pPrevCompartment,
                      (RLIST_ITEM_HANDLE *)&pCompartment);
    }

    return RV_OK;
}

/***************************************************************************
 * CompartmentMgrDetachFromSigCompCompartment
 * ------------------------------------------------------------------------
 * General: Detach SIP compartment from the sigcomp internal compartment.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pMgr - Pointer to the comp. manager.
 *			 pCompartment - Pointer to the SIP compartment
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrDetachFromSigCompCompartment(
									IN CompartmentMgr           *pMgr,
									IN RvSipCompartmentHandle   hCompartment)
{
	RvStatus     rv;
	Compartment *pComp = (Compartment *)hCompartment;

	LOCK_MGR(pMgr);
	rv = RvSigCompCloseCompartment(pMgr->hSigCompMgr,pComp->hInternalComp);
	if (rv != RV_OK)
	{
		RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
		"CompartmentMgrDetachFromSigCompCompartment - Faild detaching compartment 0x%p from internal compartment 0x%p",
		pComp,pComp->hInternalComp));
	}
	else
	{
		pComp->hInternalComp = NULL;
	}
	UNLOCK_MGR(pMgr);
	return (rv);
}

/***************************************************************************
 * CompartmentMgrAddCompartment
 * ------------------------------------------------------------------------
 * General: Adds a compartment to the hash.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pMgr - Pointer to the comp. manager.
 *			 hCompartment - handle to the compartment to add to the hash.
 *           hConnection  - handle to the initialized connection, if valid
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentMgrAddCompartment(
									IN CompartmentMgr			     *pMgr,
									IN RvSipCompartmentHandle         hCompartment,
									IN RvSipTransportConnectionHandle hConn)
{
	CompartmentHashKeyElement keyElem;
	RvStatus                  rv;
	RvBool                    bInserted;
	Compartment              *pCompartment;
	SipTransportAddress      *pAddr;

	if ((pMgr == NULL) || (hCompartment == NULL))
	{
		return (RV_ERROR_INVALID_HANDLE);
	}

	pCompartment = (Compartment *)hCompartment;

	LOCK_MGR(pMgr);

	if (RV_TRUE == pCompartment->bIsHashed)
	{
	    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
			"CompartmentMgrAddCompartment - compartment 0x%p already in hash", pCompartment));
		UNLOCK_MGR(pMgr);
		return (RV_OK);
	}

	keyElem.keyType = pCompartment->compType;

	/* set initialized connection - at this point we can be sure the connection is valid, if applicable */
	pCompartment->hConnection = hConn;
	pAddr = (pCompartment->bIsSrc == RV_TRUE)?&pCompartment->srcAddr:&pCompartment->dstAddr;
	rv = CompartmentSetHashKey (pMgr, NULL, 0, pCompartment->hPageForUrn, pCompartment->offsetInPage,
		pAddr, pCompartment->hConnection, &keyElem);
	if (rv != RV_OK)
	{
		RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList, (RLIST_ITEM_HANDLE)pCompartment);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrAddCompartment - Invalid hash key type"));
        UNLOCK_MGR(pMgr);
        return rv;
	}

	rv = HASH_InsertElement(pMgr->hCompartmentHash, &keyElem, (void *)&pCompartment, RV_FALSE,
							&bInserted, &pCompartment->hashElemPtr, CompHashCompare);
	if (rv != RV_OK)
	{
		RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList, (RLIST_ITEM_HANDLE)pCompartment);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrAddCompartment - Failed to insert element into hash (rv=%d)", rv));
        UNLOCK_MGR(pMgr);
        return rv;
	}

	if (RV_FALSE == bInserted)
	{
		RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
	    RLIST_Remove(pMgr->hCompartmentListPool,pMgr->hCompartmentList, (RLIST_ITEM_HANDLE)pCompartment);
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentMgrAddCompartment - Element was not inserted into hash"));
        UNLOCK_MGR(pMgr);
        return RV_ERROR_UNKNOWN;
	}

	pCompartment->bIsHashed = RV_TRUE;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
             "CompartmentMgrAddCompartment - compartment 0x%p was added successfully", pCompartment));

    UNLOCK_MGR(pMgr);

	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CreateCompartmentHash
 * ------------------------------------------------------------------------
 * General: creates the hash holding compartments managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Compartments list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr                 - Handle to the manager
 *            maxNumOfCompartments - Max number of compartments to allocate
 ***************************************************************************/
static RvStatus RVCALLCONV CreateCompartmentHash(
                               IN CompartmentMgr *pMgr,
                               IN RvInt32         maxNumOfCompartments)
{
	/* create the hash */
	pMgr->hCompartmentHash = HASH_Construct(HASH_DEFAULT_TABLE_SIZE(maxNumOfCompartments),
											maxNumOfCompartments,
											CompHashFunction,
											sizeof(Compartment *),
											sizeof(CompartmentHashKeyElement),
											pMgr->pLogMgr, "CompartmentHash");

	if (NULL == pMgr->hCompartmentHash)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc,
            "CreateCompartmentHash - failed to construct hash"));
        return RV_ERROR_OUTOFRESOURCES;
    }

	return (RV_OK);
}

/***************************************************************************
 * CreateCompartmentList
 * ------------------------------------------------------------------------
 * General: Allocate the list of compartments managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Compartments list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr                 - Handle to the manager
 *            maxNumOfCompartments - Max number of compartments to allocate
 ***************************************************************************/
static RvStatus RVCALLCONV CreateCompartmentList(
                               IN CompartmentMgr *pMgr,
                               IN RvInt32        maxNumOfCompartments)
{
    RvStatus     rv;
    RvInt32      currCompartment;
    Compartment *pCompartment;

    /*allocate a pool with 1 list*/
    pMgr->hCompartmentListPool = RLIST_PoolListConstruct(maxNumOfCompartments,1,
                               sizeof(Compartment),pMgr->pLogMgr, "Compartment List");

    if(pMgr->hCompartmentListPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pMgr->hCompartmentList = RLIST_ListConstruct(pMgr->hCompartmentListPool);

    if(pMgr->hCompartmentList == NULL)
    {
        RLIST_PoolListDestruct(pMgr->hCompartmentListPool);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Initiate locks of all the compartments */
    for (currCompartment=0; currCompartment<maxNumOfCompartments; currCompartment++)
    {
        RLIST_InsertTail(pMgr->hCompartmentListPool,
                         pMgr->hCompartmentList,
                         (RLIST_ITEM_HANDLE *)&pCompartment);

        pCompartment->pMgr = pMgr;
        /* Initialize the lock in the suitable compartment item */
        rv = SipCommonConstructTripleLock(&pCompartment->compTripleLock, pMgr->pLogMgr);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                      "CreateCallLegList - SipCommonConstructTripleLock (rv=%d)",rv));
            DestructCompartmentList(pMgr);
            return rv;
        }
        pCompartment->pTripleLock = NULL;
    }

	/* once the locks are constructed, we can release the compartments from the list.       */
	/* Since we are working over RA, the actual location of the locks will remain the same  */
	/* throughout the lifetime of the stack instance. When creating new compartment object, */
	/* the location of the lock will remain the same.                                       */
	/* The reason we're doing it is because we have to construct all the locks upon         */
	/* initialization, and release them only when the stack is being destructed.            */
    for (currCompartment=0; currCompartment<maxNumOfCompartments; currCompartment++)
    {
        RLIST_GetHead(pMgr->hCompartmentListPool,
                      pMgr->hCompartmentList,(RLIST_ITEM_HANDLE *)&pCompartment);
        RLIST_Remove(pMgr->hCompartmentListPool,
                     pMgr->hCompartmentList,(RLIST_ITEM_HANDLE)pCompartment);
    }

    return RV_OK;
}

/***************************************************************************
 * DestructCompartmentList
 * ------------------------------------------------------------------------
 * General: Destruct the list of compartments managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to destruct list.
 *               RV_OK        - Compartment list was destructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr                 - Handle to the manager
 ***************************************************************************/
static RvStatus RVCALLCONV DestructCompartmentList(
                                    IN CompartmentMgr *pMgr)
{
    RvInt32      compartmentsNum;
    RvInt32      currCompartment;
    Compartment *pCompartment;
    RvStatus     rv = RV_OK;

    /* First create full list of compartments that will be */
    /* handled separately */
    rv = RLIST_GetNumOfElements(pMgr->hCompartmentListPool,pMgr->hCompartmentList,(RvUint32*)&compartmentsNum);
    if (RV_OK != rv)
    {
        compartmentsNum = 0;
    }

    for (currCompartment = compartmentsNum;
         currCompartment < pMgr->maxNumOfCompartments && RV_OK == rv;
         currCompartment++)
    {
        rv = RLIST_InsertTail(pMgr->hCompartmentListPool,
            pMgr->hCompartmentList,
            (RLIST_ITEM_HANDLE *)&pCompartment);
    }

    /* Destruct locks of all calls */
    for (currCompartment=0;
         currCompartment < pMgr->maxNumOfCompartments;
         currCompartment++)
    {
        RLIST_GetHead(pMgr->hCompartmentListPool,
                      pMgr->hCompartmentList,(RLIST_ITEM_HANDLE *)&pCompartment);

        if (NULL != pCompartment)
        {
	        SipCommonDestructTripleLock(&pCompartment->compTripleLock, pMgr->pLogMgr);

			/* free URN pages, if allocated */
			if (currCompartment < compartmentsNum)
			{
				RPOOL_FreePage(pMgr->hCompartmentPool, pCompartment->hPageForUrn);
			}
            RLIST_Remove(pMgr->hCompartmentListPool,
                         pMgr->hCompartmentList,(RLIST_ITEM_HANDLE)pCompartment);
        }
    }

    /* Free the Compartment list and list pool*/
    if(pMgr->hCompartmentListPool)
    {
        RLIST_PoolListDestruct(pMgr->hCompartmentListPool);
		pMgr->hCompartmentListPool = NULL;
    }

    return RV_OK;
}

/***************************************************************************
 * DestructCompartmentHash
 * ------------------------------------------------------------------------
 * General: Destruct the hash of compartments managed by the manager
 * Return Value: RV_OK        - Compartment list was destructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr                 - Handle to the manager
 ***************************************************************************/
static RvStatus RVCALLCONV DestructCompartmentHash(IN CompartmentMgr *pMgr)
{
	HASH_Destruct(pMgr->hCompartmentHash);
	return RV_OK;
}


/***************************************************************************
 * CompHashFunction
 * ------------------------------------------------------------------------
 * General: function used by the hash algorithm to calculate the hash value
 * Return Value: integer representing the hash key value
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   key   - pointer to the key on which the hash value is calculated
 ***************************************************************************/
static RvUint CompHashFunction(IN void *key)
{

    CompartmentHashKeyElement *hashElem = (CompartmentHashKeyElement *)key;
    RvInt32    i,j;
    RvUint     s = 0;
    RvInt32    mod = hashElem->pCompartmentMgr->maxNumOfCompartments;
    RvInt32    keySize;
    RPOOL_Ptr  *pUrn = NULL;
    RvInt32    offset;
    RvInt32    timesToCopy;
    RvInt32    leftOvers;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#define TEMPBUFSZ  50
   RvChar     buffer[TEMPBUFSZ];
#else
   RvChar     buffer[33];
#endif
/* SPIRENT_END */
    static const int base = 1 << (sizeof(char)*8);

	/*
		The functions goes over buffers of 33 bytes, and compute an	aggregated
		hash value.
	*/

    if(hashElem->keyType == RVSIP_COMPARTMENT_TYPE_URN)
    {
		/* compute hash value on the URN */
		pUrn = &(hashElem->urn);
		offset = pUrn->offset;
		keySize = RPOOL_Strlen(pUrn->hPool,	pUrn->hPage, pUrn->offset);

		timesToCopy = keySize/32;
		leftOvers   = keySize%32;

		s = (s*base+hashElem->keyType);

		for(i=0; i<timesToCopy; i++)
		{
			memset(buffer,0,33);
			RPOOL_CopyToExternal(pUrn->hPool, pUrn->hPage, offset, buffer, 32);
			buffer[32] = '\0';
			for (j=0; j<32; j++)
			{
				s = (s*base+buffer[j])%mod;
			}
			offset+=32;
		}
		if(leftOvers > 0)
		{
			memset(buffer,0,33);
			RPOOL_CopyToExternal(pUrn->hPool, pUrn->hPage, offset,
								buffer,	leftOvers);
			buffer[leftOvers] = '\0';
			for (j=0; j<leftOvers; j++)
			{
				s = (s*base+buffer[j])%mod;
			}
		}
	}
	else if (hashElem->keyType == RVSIP_COMPARTMENT_TYPE_ADDRESS)
	{
		SipTransportAddress *pSipAddr = hashElem->pAddress;
		/* compute hash value on the address */
		keySize = sizeof(SipTransportAddress);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		RvAssert (keySize < TEMPBUFSZ);
#else
		RvAssert (keySize <= 32);
#endif
/* SPIRENT_END */
		s = (s*base+hashElem->keyType);
		offset = 0;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		memset(buffer,0,TEMPBUFSZ);
#else
		memset(buffer,0,33);
#endif
/* SPIRENT_END */
		memcpy(buffer + offset, &pSipAddr->addr.addrtype, sizeof(pSipAddr->addr.addrtype));
		offset += sizeof(pSipAddr->addr.addrtype);
		if (RV_ADDRESS_TYPE_IPV4 == pSipAddr->addr.addrtype)
		{
			memcpy(buffer + offset, &pSipAddr->addr.data.ipv4.ip, sizeof(pSipAddr->addr.data.ipv4.ip));
			offset += sizeof(pSipAddr->addr.data.ipv4.ip);
			memcpy(buffer + offset, &pSipAddr->addr.data.ipv4.port, sizeof(pSipAddr->addr.data.ipv4.port));
			offset += sizeof(pSipAddr->addr.data.ipv4.port);
		}
#if(RV_NET_TYPE & RV_NET_IPV6)
		else if (RV_ADDRESS_TYPE_IPV6 == pSipAddr->addr.addrtype)
		{
			memcpy(buffer + offset, pSipAddr->addr.data.ipv6.ip, sizeof(pSipAddr->addr.data.ipv6.ip));
			offset += sizeof(pSipAddr->addr.data.ipv6.ip);
			memcpy(buffer + offset, &pSipAddr->addr.data.ipv6.port, sizeof(pSipAddr->addr.data.ipv6.port));
			offset += sizeof(pSipAddr->addr.data.ipv6.port);
			memcpy(buffer + offset, &pSipAddr->addr.data.ipv6.scopeId, sizeof(pSipAddr->addr.data.ipv6.scopeId));
			offset += sizeof(pSipAddr->addr.data.ipv6.scopeId);
		}
#endif
		buffer[keySize] = '\0';
		for (j=0; j < keySize; j++)
		{
			s = (s*base+buffer[j])%mod;
		}

	}
	else if (hashElem->keyType == RVSIP_COMPARTMENT_TYPE_CONNECTION)
	{
		/* compute hash value on the connection */
		s = (s*base+hashElem->keyType);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		memset(buffer,0,TEMPBUFSZ);
		sprintf(buffer,"%d%p", hashElem->keyType, hashElem->hConnection);
	    for (i = 0; i < TEMPBUFSZ; i++)
		{
			s = (s*base+buffer[i])%mod;
		}
#else
		memset(buffer,0,33);
		sprintf(buffer,"%d%p", hashElem->keyType, hashElem->hConnection);
	    for (i = 0; i < 33; i++)
		{
			s = (s*base+buffer[i])%mod;
		}

#endif
/* SPIRENT_END */
	}
	else
	{
		;
	}
    return s;
}

/***************************************************************************
 * CompHashCompare
 * ------------------------------------------------------------------------
 * General: compare function used by the hash algorithm.
 * Return Value:	RV_TRUE if both entries are identical
 *					RV_FALSE if both entires are not identical
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   newHashElement   - pointer to the first hash entry
 *          oldHashElement   - pointer to the second hash entry
 ***************************************************************************/
static RvBool CompHashCompare(	IN void *newHashElement,
				                IN void *oldHashElement)
{
    CompartmentHashKeyElement  *pOldHashKey   = (CompartmentHashKeyElement*)oldHashElement;
    CompartmentHashKeyElement  *pNewHashKey   = (CompartmentHashKeyElement*)newHashElement;

	if (pOldHashKey->keyType == pNewHashKey->keyType)
	{
		if (pOldHashKey->keyType == RVSIP_COMPARTMENT_TYPE_URN)
		{
			RPOOL_Ptr *pUrn1 = &pOldHashKey->urn;
			RPOOL_Ptr *pUrn2 = &pNewHashKey->urn;

			/* we assume that the urns are already normalized (case-wise), so we can use RPOOL_Strcmp */
			if (RPOOL_Strcmp(pUrn1->hPool, pUrn1->hPage, pUrn1->offset,
							pUrn2->hPool, pUrn2->hPage, pUrn2->offset) != RV_FALSE)
			{
				return (RV_TRUE);
			}
		}
		else if (pOldHashKey->keyType == RVSIP_COMPARTMENT_TYPE_ADDRESS)
		{
			if ((RvAddressCompare(&pOldHashKey->pAddress->addr,&pNewHashKey->pAddress->addr, RV_ADDRESS_FULLADDRESS) != RV_FALSE) &&
				(pOldHashKey->pAddress->eTransport == pNewHashKey->pAddress->eTransport))
			{
				return (RV_TRUE);
			}
		}
		else if (pOldHashKey->keyType == RVSIP_COMPARTMENT_TYPE_CONNECTION)
		{
			if (pOldHashKey->hConnection == pNewHashKey->hConnection)
			{
				return (RV_TRUE);
			}
		}
	}
	return RV_FALSE;
}

/***************************************************************************
 * CompartmentSetHashKey
 * ------------------------------------------------------------------------
 * General: sets the key fields in the hash key structure.
 * Return Value:	RV_OK if both entries are identical
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr	      - pointer to compartment manager
 *          pUrn	      - pointer to the RPOOL_PTR containing the sigcomp-id.
 *                          Should be NULL if sigcomp-id is not valid.
 *          urnLen        - the length of the sigcomp-id
 *          hPageForUrn   - the page that holds the sigcomp-id
 *          offsetInPage  - the offset of the sipcompID in the page (hPageForUrn)
 *          pAddr	      - pointer to IP/PORT in case of UDP
 *          hConn	      - connection handle in case of TCP
 * Output:  pKeyElem	  - the key element holding the information
 ***************************************************************************/
static RvStatus CompartmentSetHashKey (
									IN  CompartmentMgr		*pMgr,
									IN  RPOOL_Ptr			*pUrn,
									IN  RvInt32              urnLen,
									IN  HPAGE                hPageForUrn,
									IN  RvInt32              offsetInPage,
									IN  SipTransportAddress	*pAddr,
									IN  RvSipTransportConnectionHandle	hConn,
									OUT CompartmentHashKeyElement      *pKeyElem)
{
	RvStatus rv;

	pKeyElem->pCompartmentMgr = pMgr;
	switch (pKeyElem->keyType)
	{
		case RVSIP_COMPARTMENT_TYPE_URN:
		{
#ifdef RV_DEBUG
			RvChar debugSigcompId[RVSIP_COMPARTMENT_MAX_URN_LENGTH] = {'\0'};
#endif
			RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"CompartmentSetHashKey - URN based compartment key"));
			pKeyElem->urn.hPool  = pMgr->hCompartmentPool;
			pKeyElem->urn.hPage  = hPageForUrn;
			pKeyElem->urn.offset = offsetInPage;
			if (NULL == pUrn)
			{
				/* not need to copy sigcomp-id */
				return (RV_OK);
			}
			else
			{
				rv = RPOOL_CopyFrom(pMgr->hCompartmentPool, hPageForUrn, pKeyElem->urn.offset,
					pUrn->hPool, pUrn->hPage, pUrn->offset, urnLen+1);
#ifdef RV_DEBUG
				RPOOL_CopyToExternal(pKeyElem->urn.hPool,pKeyElem->urn.hPage,pKeyElem->urn.offset,
					debugSigcompId, urnLen+1);
				RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
					"CompartmentSetHashKey - sigcomp-id is %s", debugSigcompId));
#endif
				return (rv);
			}
		}
		break;

		case RVSIP_COMPARTMENT_TYPE_ADDRESS:
		{
			RvChar		 address[100];
			RvChar		*pAddrStr;
			RvUint16	 port;

			memset (address,0,sizeof(address));
			pAddrStr = RvAddressGetString(&(pAddr->addr), sizeof(address), address);
			port	 = RvAddressGetIpPort(&(pAddr->addr));
			RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"CompartmentSetHashKey - IP-Port based compartment key (%s:%d)",(pAddrStr == NULL)?"0.0.0.0":address,
				port));
			pKeyElem->pAddress = pAddr;
			return RV_OK;
		}
		break;

		case RVSIP_COMPARTMENT_TYPE_CONNECTION:
		{
			RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
			"CompartmentSetHashKey - Connection based compartment key (0x%p)", hConn));
			pKeyElem->hConnection = hConn;
		}
		break;

		default:
			return RV_ERROR_BADPARAM;
		break;
	} /* switch */
	return RV_OK;
}

/***************************************************************************
 * IsConnectionless
 * ------------------------------------------------------------------------
 * General: check what connection type a SipTransportAddress represents
 * Return Value:	RV_TRUE if connection less
 *					RV_FALSE if connection oriented
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pAddr   - pointer to the address object
 ***************************************************************************/
static RvBool IsConnectionless (SipTransportAddress *pAddr)
{
	switch (pAddr->eTransport)
	{
	case RVSIP_TRANSPORT_UDP:
	case RVSIP_TRANSPORT_UNDEFINED:
	case RVSIP_TRANSPORT_OTHER:
		return (RV_TRUE);
		break;
	default:
		return (RV_FALSE);
		break;
	}
}

/***************************************************************************
 * CalcCompartmentType
 * ------------------------------------------------------------------------
 * General: calculate the type of a compartment, based on the parameters for its key.
 *          The rule for determine the type is as follow:
 *          a. If pUrn is not NULL, that the type is URN-based.
 *          b. Otherwise, if the address is not NULL, then:
 *             1. If the transport type is connection oriented, the key is connection-based
 *             2. Else the key is address-based
 *          c. Else, the key is address-based
 *
 * Return Value:	RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pUrn      - holds the sigcomp-id.
 *          pAddr     - holds the address
 * OutPut:  pCompType - holds the resulted compartment type, based on the key.
 ***************************************************************************/
static RvStatus CalcCompartmentType (
									IN  RPOOL_Ptr                       *pUrn,
									IN  SipTransportAddress             *pAddr,
									OUT RvSipCompartmentType            *pCompType)
{
	RvBool conType;

	if (pUrn != NULL)
	{
		*pCompType = RVSIP_COMPARTMENT_TYPE_URN;
	}
	else if (pAddr != NULL)
	{
		conType = IsConnectionless (pAddr);
		if (RV_TRUE == conType)
		{
			*pCompType = RVSIP_COMPARTMENT_TYPE_ADDRESS;
		}
		else
		{
			*pCompType = RVSIP_COMPARTMENT_TYPE_CONNECTION;
		}
	}
	else
	{
		*pCompType = RVSIP_COMPARTMENT_TYPE_CONNECTION;
	}
	return RV_OK;
}

#endif /* RV_SIGCOMP_ON */

