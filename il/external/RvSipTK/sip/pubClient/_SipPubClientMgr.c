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
 *                              <SipPubClientMgr.c>
 *
 *  The SipPubClientMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the publish-client manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Sep 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"
#include "_SipPubClientMgr.h"
#include "PubClientMgrObject.h"
#include "PubClientObject.h"
#include "_SipTranscClient.h"
#include "rvmemory.h"
#include "_SipTransport.h"
#include "PubClientTranscClientEv.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
#include "_SipTranscClientMgr.h"
#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeTypes.h"
#include "_SipSecAgreeMgr.h"
#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus RVCALLCONV PubClientAllocateResources(
                                         IN  PubClientMgr      *pPubClientMgr);
static void RVCALLCONV DestructPubClientList(
                                    IN  PubClientMgr      *pPubClientMgr);

static RvStatus RVCALLCONV CreatePubClientList(
                                    IN  PubClientMgr      *pPubClientMgr);

static void RVCALLCONV PubClientSetTranscClientEvHandlers(
                                         IN PubClient      *pPubClient);

static void RVCALLCONV DestructAllPubClients(IN  PubClientMgr      *pPubClientMgr);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipPubClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new publish-client manager. The publish-client
 *          manager is in charge of all the publish-clients. The manager
 *          holds a list of publish-clients and is used to create new
 *          ones.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the publish-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   publish-client manager.
 *               RV_OK -        publish-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each publish-client for internal
 *                          memory allocations.
 * Input and Output:
 *          maxNumPubClients - Max number of concurrent publish-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack - Handle to the stack manager.
 * Output:     *phPubClientMgr - Handle to the newly created publish-client
 *                            manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrConstruct(
                        INOUT SipPubClientMgrCfg       *pCfg,
                         IN    void*					hStack,
                         OUT   RvSipPubClientMgrHandle  *phPubClientMgr)
{
    PubClientMgr *pPubClientMgr;
    RvStatus     retStatus;
    RvStatus      crv;
    if (NULL == phPubClientMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    if ((NULL == pCfg->hTranscMgr) ||
        (NULL == pCfg->hGeneralPool) ||
		(NULL == pCfg->hTransport))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Allocate a new publish-client manager object */
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(PubClientMgr), pCfg->pLogMgr, (void*)&pPubClientMgr))
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
                  "SipPubClientMgrConstruct - Error trying to construct a new publish-clients manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    *phPubClientMgr = (RvSipPubClientMgrHandle)pPubClientMgr;
	SipCommonCSeqSetStep(0,&pPubClientMgr->cseq); 
    pPubClientMgr->pLogMgr         = pCfg->pLogMgr;
    pPubClientMgr->hGeneralMemPool = pCfg->hGeneralPool;
    pPubClientMgr->hTranscMgr      = pCfg->hTranscMgr;
    pPubClientMgr->pLogSrc           = pCfg->moduleLogId;
    pPubClientMgr->maxNumOfPubClients = pCfg->maxNumOfPubClients;
    pPubClientMgr->seed            = pCfg->seed;
	pPubClientMgr->hTransportMgr   = pCfg->hTransport;
	pPubClientMgr->hMsgMgr			= pCfg->hMsgMgr;
	pPubClientMgr->hTranscClientMgr = pCfg->hTranscClientMgr;

	pPubClientMgr->pubClientAlertTimeOut = pCfg->alertTimeout;

    pPubClientMgr->pubClientEvHandlers.pfnMsgReceivedEvHandler = NULL;
    pPubClientMgr->pubClientEvHandlers.pfnMsgToSendEvHandler = NULL;
    pPubClientMgr->pubClientEvHandlers.pfnStateChangedEvHandler = NULL;
	pPubClientMgr->pubClientEvHandlers.pfnObjExpiredEvHandler = NULL;
	pPubClientMgr->pubClientEvHandlers.pfnFinalDestResolvedEvHandler = NULL;
	pPubClientMgr->pubClientEvHandlers.pfnNewConnInUseEvHandler = NULL;
	pPubClientMgr->pubClientEvHandlers.pfnOtherURLAddressFoundEvHandler = NULL;
#ifdef RV_SIGCOMP_ON
	pPubClientMgr->pubClientEvHandlers.pfnSigCompMsgNotRespondedEvHandler = NULL;
#endif
	
	/*Setting the expTimer flag to false until the application will set a callback for this event.*/
	pPubClientMgr->bHandleExpTimer = RV_FALSE;

    crv = RvMutexConstruct(pPubClientMgr->pLogMgr, &pPubClientMgr->hLock);
    if(crv != RV_OK)
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
                  "SipPubClientMgrConstruct - Failed to allocate a mutex for the pub-client manager (rv=0x%p)",CRV2RV(crv)));
        RvMemoryFree(pPubClientMgr, pPubClientMgr->pLogMgr);
        return CRV2RV(crv);
    }

    /* Allocate all required resources */
    retStatus = PubClientAllocateResources(pPubClientMgr);
    if (RV_OK != retStatus)
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
                  "SipPubClientMgrConstruct - Error trying to construct a new publish-clients manager"));
        RvMemoryFree(pPubClientMgr, pPubClientMgr->pLogMgr);
        *phPubClientMgr = NULL;
        return retStatus;
    }
       
    RvLogInfo(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
              "SipPubClientMgrConstruct - Publish client manager was successfully constructed"));

	RV_UNUSED_ARG(hStack);
    return RV_OK;
}


/***************************************************************************
 * SipPubClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the publish-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The publish-client handle is invalid.
 *                  RV_OK -  publish-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClientMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrDestruct(
                                   IN RvSipPubClientMgrHandle hPubClientMgr)
{
    PubClientMgr *pPubClientMgr;

    if (NULL == hPubClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClientMgr = (PubClientMgr *)hPubClientMgr;
    RvMutexLock(&pPubClientMgr->hLock,pPubClientMgr->pLogMgr);

    /* killing all pub clients that stayed alive */
    DestructAllPubClients(pPubClientMgr);

    /* Free all resources */
    DestructPubClientList(pPubClientMgr);
    if (NULL_PAGE != pPubClientMgr->hMemoryPage)
    {
        RPOOL_FreePage(pPubClientMgr->hGeneralMemPool, pPubClientMgr->hMemoryPage);
    }
    RvLogInfo(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
              "SipPubClientMgrDestruct - publish-client manager was destructed"));
    RvMutexUnlock(&pPubClientMgr->hLock,pPubClientMgr->pLogMgr);
    RvMutexDestruct(&pPubClientMgr->hLock,pPubClientMgr->pLogMgr);
    RvMemoryFree(pPubClientMgr, pPubClientMgr->pLogMgr);
    return RV_OK;
}





/***************************************************************************
 * SipPubClientMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the publish-client module. It includes the publish-client list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the pub-client manager.
 * Output:     pResources - Pointer to a PubClient resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrGetResourcesStatus (
                                 IN  RvSipPubClientMgrHandle   hMgr,
                                 OUT RvSipPubClientResources  *pResources)
{
    RvUint32     allocNumOfLists;
    RvUint32     allocMaxNumOfUserElement;
    RvUint32     currNumOfUsedLists;
    RvUint32     currNumOfUsedUserElement;
    RvUint32     maxUsageInLists;
    RvUint32     maxUsageInUserElement;

    if(hMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /* publish-clients list */
    if (NULL == ((PubClientMgr *)hMgr)->hPubClientListPool)
    {
        pResources->pubClients.currNumOfUsedElements = 0;
        pResources->pubClients.maxUsageOfElements = 0;
        pResources->pubClients.numOfAllocatedElements = 0;
    }
    else
    {
        RLIST_GetResourcesStatus(((PubClientMgr *)hMgr)->hPubClientListPool,
                             &allocNumOfLists, /*allways 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*allways 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);
        pResources->pubClients.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->pubClients.maxUsageOfElements = maxUsageInUserElement;
        pResources->pubClients.numOfAllocatedElements = allocMaxNumOfUserElement;
    }

    return RV_OK;
}

/***************************************************************************
 * SipPubClientMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of pub-clients that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The pub-client manager.
 ***************************************************************************/
void RVCALLCONV SipPubClientMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipPubClientMgrHandle  hMgr)
{
    PubClientMgr *pMgr = (PubClientMgr*)hMgr;

    RLIST_ResetMaxUsageResourcesStatus(pMgr->hPubClientListPool);
}



#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipPubClientMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in pub-client mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the pub-client manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrSetSecAgreeMgrHandle(
                                        IN RvSipPubClientMgrHandle   hPubClientMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr)
{
	PubClientMgr* pMgr = (PubClientMgr*)hPubClientMgr;
	
	/* Set event handlers */
	return SipTranscClientMgrSetSecAgreeMgrHandle(pMgr->hTranscClientMgr, hSecAgreeMgr, RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT);
    
}
#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PubClientAllocateResources
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The publish-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - Pointer to the publish-clients manager.
 *          hTransport - The transport handle.
 ***************************************************************************/
static RvStatus RVCALLCONV PubClientAllocateResources(
                                         IN  PubClientMgr      *pPubClientMgr)
{
    RvStatus rv;

    /* Construct the pool of publish-clients lists. This pool
    will contain one list */
    if (0 != pPubClientMgr->maxNumOfPubClients)
    {
        /* Get a page from the general pool to use later for the Call-Id */
        rv = RPOOL_GetPage(pPubClientMgr->hGeneralMemPool, 0,
                                  &(pPubClientMgr->hMemoryPage));
        if (RV_OK != rv)
        {
            return rv;
        }

        rv = CreatePubClientList(pPubClientMgr);
        if(rv != RV_OK)
        {
            RPOOL_FreePage(pPubClientMgr->hGeneralMemPool,
                           pPubClientMgr->hMemoryPage);
        }
    }
    else
    {
        pPubClientMgr->hMemoryPage =         NULL_PAGE;
        pPubClientMgr->hPubClientListPool =  NULL;
        pPubClientMgr->hPubClientList =      NULL;
    }
    return RV_OK;
}

/***************************************************************************
 * CreatePubClientList
 * ------------------------------------------------------------------------
 * General: Allocated the list of pub-clients managed by the manager
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - Pointer to the publish-clients manager.
 ***************************************************************************/
static RvStatus RVCALLCONV CreatePubClientList(
                                    IN  PubClientMgr      *pPubClientMgr)
{

    RvInt32      i;
    PubClient     *pubClient;
    RvStatus    rv;
	SipTranscClientOwnerUtils pubClientLockUtils;

    memset(&pubClientLockUtils,0,sizeof(SipTranscClientOwnerUtils));
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    pubClientLockUtils.pfnOwnerLock = (OwnerObjLockFunc)PubClientLockAPI;
	pubClientLockUtils.pfnOwnerUnlock = (OwnerObjUnLockFunc)PubClientUnLockAPI;
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

    pPubClientMgr->hPubClientListPool = RLIST_PoolListConstruct(
                                         pPubClientMgr->maxNumOfPubClients,
                                         1, sizeof(PubClient), pPubClientMgr->pLogMgr,
                                         "Pub client pool");
    if (NULL == pPubClientMgr->hPubClientListPool)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    /* Construct the list of publish-clients */
    pPubClientMgr->hPubClientList = RLIST_ListConstruct(
                                       pPubClientMgr->hPubClientListPool);
    if (NULL == pPubClientMgr->hPubClientList)
    {
        RLIST_PoolListDestruct(pPubClientMgr->hPubClientListPool);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* initiate locks of all pubClients */
    for (i=0; i < (RvInt32)pPubClientMgr->maxNumOfPubClients; i++)
    {
        rv = RLIST_InsertTail(pPubClientMgr->hPubClientListPool,
                              pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pubClient);
        pubClient->pMgr = pPubClientMgr;
        
		rv = SipTranscClientConstruct(pPubClientMgr->hTranscClientMgr, 
									&pubClient->transcClient, 
									(SipTranscClientOwnerHandle)pubClient, 
									RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT,
									&pubClientLockUtils,
									&pubClient->tripleLock);
		if (RV_OK != rv)
		{
			RLIST_PoolListDestruct(pPubClientMgr->hPubClientListPool);
			return rv;
		}
		PubClientSetTranscClientEvHandlers(pubClient);
    }

    for (i=0; i < (RvInt32)pPubClientMgr->maxNumOfPubClients; i++)
    {
        RLIST_GetHead(pPubClientMgr->hPubClientListPool,
                         pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pubClient);
        RLIST_Remove(pPubClientMgr->hPubClientListPool,
                     pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE)pubClient);
    }
    return RV_OK;
}

/***************************************************************************
 * DestructAllPubClients
 * ------------------------------------------------------------------------
 * General: Destructs all the pub clients
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - Pointer to the publish-clients manager.
 ***************************************************************************/
static void RVCALLCONV DestructAllPubClients(IN  PubClientMgr      *pPubClientMgr)
{
    PubClient    *pNextToKill = NULL;
    PubClient    *pPub2Kill = NULL;
    if (NULL == pPubClientMgr->hPubClientListPool)
    {
        return;
    }

    RLIST_GetHead(pPubClientMgr->hPubClientListPool,
        pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pPub2Kill);

    while (NULL != pPub2Kill)
    {
        RLIST_GetNext(pPubClientMgr->hPubClientListPool,pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE)pPub2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
		PubClientDestruct(pPub2Kill);
        pPub2Kill = pNextToKill;
    }

}

/***************************************************************************
 * DestructPubClientList
 * ------------------------------------------------------------------------
 * General: Destructs the list of pub-clients
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - Pointer to the publish-clients manager.
 ***************************************************************************/
static void RVCALLCONV DestructPubClientList(
                                    IN  PubClientMgr      *pPubClientMgr)
{

    RvUint32    i;
    PubClient    *pubClient;

    if (NULL == pPubClientMgr->hPubClientListPool)
    {
        return;
    }

    for (i=0; i < pPubClientMgr->maxNumOfPubClients; i++)
    {
        RLIST_GetHead(pPubClientMgr->hPubClientListPool,
            pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pubClient);
        if (pubClient == NULL)
            break;
        RLIST_Remove(pPubClientMgr->hPubClientListPool,
            pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE)pubClient);
    }

    for (i=0; i < pPubClientMgr->maxNumOfPubClients; i++)
    {
        RLIST_InsertTail(pPubClientMgr->hPubClientListPool,
            pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pubClient);
    }

    for (i=0; i < pPubClientMgr->maxNumOfPubClients; i++)
    {
        RLIST_GetHead(pPubClientMgr->hPubClientListPool,
            pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE *)&pubClient);
        SipCommonDestructTripleLock(
            pubClient->tripleLock, pPubClientMgr->pLogMgr);
        RLIST_Remove(pPubClientMgr->hPubClientListPool,
            pPubClientMgr->hPubClientList,(RLIST_ITEM_HANDLE)pubClient);
    }

    RLIST_PoolListDestruct(pPubClientMgr->hPubClientListPool);
    pPubClientMgr->hPubClientListPool = NULL;
}


/***************************************************************************
 * PubClientMgrSetTranscEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the event handlers defined by the publish-client module to
 *          the transaction manager, to be used when ever the transaction
 *          owner is a publish client.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - Pointer to the publish-clients manager.
 ***************************************************************************/
static void RVCALLCONV PubClientSetTranscClientEvHandlers(
                                         IN PubClient      *pPubClient)
{
    SipTranscClientEvHandlers  pubClientEvHandlers;
    RvUint32                   size;

    
    memset(&pubClientEvHandlers,0,sizeof(SipTranscClientEvHandlers));
    pubClientEvHandlers.pfnMsgReceivedEvHandler =
													PubClientTranscClientEvMsgReceived;
    pubClientEvHandlers.pfnMsgToSendEvHandler =
													PubClientTranscClientEvMsgToSend;
    pubClientEvHandlers.pfnStateChangedEvHandler =
													PubClientTranscClientEvStateChanged;
    pubClientEvHandlers.pfnOtherURLAddressFoundEvHandler   =
													PubClientTranscClientEvOtherURLAddressFound;
    pubClientEvHandlers.pfnFinalDestResolvedEvHandler = 
													PubClientTranscClientFinalDestResolvedEv;
    pubClientEvHandlers.pfnNewConnInUseEvHandler = 
													PubClientTranscClientNewConnInUseEv;

	pubClientEvHandlers.pfnObjExpiredEvHandler = 
													PubClientTranscClientObjExpiredEv;
	
	pubClientEvHandlers.pfnObjTerminatedEvHandler = 
													PubClientTranscClientObjTerminated;
#ifdef RV_SIGCOMP_ON
    pubClientEvHandlers.pfnSigCompMsgNotRespondedEvHandler =
                                        PubClientTranscClientSigCompMsgNotRespondedEv;
#endif /* RV_SIGCOMP_ON */

	size = sizeof(SipTranscClientEvHandlers);
	
	SipTranscClientSetEvHandlers(&pPubClient->transcClient,
		&pubClientEvHandlers, size);
	
    return ;
}

#endif /* RV_SIP_PRIMITIVES */





















