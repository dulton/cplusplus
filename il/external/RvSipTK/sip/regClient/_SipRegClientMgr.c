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
 *                              <SipRegClientMgr.c>
 *
 *  The SipRegClientMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the register-client manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *    Gilad Govrin                 Sep 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"
#include "_SipRegClientMgr.h"
#include "RegClientMgrObject.h"
#include "RegClientObject.h"
#include "_SipTranscClient.h"
#include "rvmemory.h"
#include "_SipTransport.h"
#include "RegClientTranscClientEv.h"
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


static RvStatus RVCALLCONV RegClientAllocateResources(
                                         IN  RegClientMgr      *pRegClientMgr);
static void RVCALLCONV DestructRegClientList(
                                    IN  RegClientMgr      *pRegClientMgr);

static RvStatus RVCALLCONV CreateRegClientList(
                                    IN  RegClientMgr      *pRegClientMgr);

static void RVCALLCONV RegClientSetTranscClientEvHandlers(
                                         IN RegClient      *pRegClient);

static void RVCALLCONV DestructAllRegClients(IN  RegClientMgr      *pRegClientMgr);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipRegClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new register-client manager. The register-client
 *          manager is encharged of all the register-clients. The manager
 *          holds a list of register-clients and is used to create new
 *          regClients.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the register-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   register-client manager.
 *               RV_OK -        register-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each register-client for internal
 *                          memory allocations.
 * Input and Output:
 *          maxNumRegClients - Max number of concurrent register-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack - Handle to the stack manager.
 * Output:     *phRegClientMgr - Handle to the newly created register-client
 *                            manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrConstruct(
                        INOUT SipRegClientMgrCfg       *pCfg,
                         IN    void*					hStack,
                         OUT   RvSipRegClientMgrHandle  *phRegClientMgr)
{
    RegClientMgr *pRegClientMgr;
    RvStatus     retStatus;
    RvStatus      crv;
    if (NULL == phRegClientMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    if ((NULL == pCfg->hTranscMgr) ||
        (NULL == pCfg->hGeneralPool) ||
		(NULL == pCfg->hTransport))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Allocate a new register-client manager object */
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RegClientMgr), pCfg->pLogMgr, (void*)&pRegClientMgr))
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "SipRegClientMgrConstruct - Error trying to construct a new register-clients manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    *phRegClientMgr = (RvSipRegClientMgrHandle)pRegClientMgr;
	SipCommonCSeqSetStep(0,&pRegClientMgr->cseq); 
    pRegClientMgr->pLogMgr         = pCfg->pLogMgr;
    pRegClientMgr->hGeneralMemPool = pCfg->hGeneralPool;
    pRegClientMgr->hTranscMgr      = pCfg->hTranscMgr;
    pRegClientMgr->pLogSrc           = pCfg->moduleLogId;
    pRegClientMgr->maxNumOfRegClients = pCfg->maxNumOfRegClients;
    pRegClientMgr->seed            = pCfg->seed;
	pRegClientMgr->hTransportMgr   = pCfg->hTransport;
	pRegClientMgr->hMsgMgr			= pCfg->hMsgMgr;
	pRegClientMgr->hTranscClientMgr = pCfg->hTranscClientMgr;
	
	pRegClientMgr->regClientAlertTimeOut = pCfg->alertTimeout;


    pRegClientMgr->regClientEvHandlers.pfnMsgReceivedEvHandler = NULL;
    pRegClientMgr->regClientEvHandlers.pfnMsgToSendEvHandler = NULL;
    pRegClientMgr->regClientEvHandlers.pfnStateChangedEvHandler = NULL;
	pRegClientMgr->regClientEvHandlers.pfnObjExpiredEvHandler = NULL;
	pRegClientMgr->regClientEvHandlers.pfnFinalDestResolvedEvHandler = NULL;
	pRegClientMgr->regClientEvHandlers.pfnNewConnInUseEvHandler = NULL;
	pRegClientMgr->regClientEvHandlers.pfnOtherURLAddressFoundEvHandler = NULL;

#ifdef RV_SIGCOMP_ON
	pRegClientMgr->regClientEvHandlers.pfnSigCompMsgNotRespondedEvHandler = NULL;
#endif
	
	/*Setting the expTimer flag to false until the application will set a callback for this event.*/
	pRegClientMgr->bHandleExpTimer = RV_FALSE;

    crv = RvMutexConstruct(pRegClientMgr->pLogMgr, &pRegClientMgr->hLock);
    if(crv != RV_OK)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "SipRegClientMgrConstruct - Failed to allocate a mutex for the reg-client manager (rv=0x%p)",CRV2RV(crv)));
        RvMemoryFree(pRegClientMgr, pRegClientMgr->pLogMgr);
        return CRV2RV(crv);
    }

    /* Allocate all required resources */
    retStatus = RegClientAllocateResources(pRegClientMgr);
    if (RV_OK != retStatus)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "SipRegClientMgrConstruct - Error trying to construct a new register-clients manager"));
        RvMemoryFree(pRegClientMgr, pRegClientMgr->pLogMgr);
        *phRegClientMgr = NULL;
        return retStatus;
    }
    /* Generate a unique global Call-Id */
#ifdef SIP_DEBUG
    retStatus = RegClientGenerateCallId(pRegClientMgr, pRegClientMgr->hMemoryPage,
                                        &(pRegClientMgr->strCallId), &(pRegClientMgr->pCallId));
#else
    retStatus = RegClientGenerateCallId(pRegClientMgr, pRegClientMgr->hMemoryPage,
                                        &(pRegClientMgr->strCallId), NULL);
#endif
    if (RV_OK != retStatus)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "SipRegClientMgrConstruct - Error trying to construct a new register-clients manager"));
        SipRegClientMgrDestruct(*phRegClientMgr);
        *phRegClientMgr = NULL;
        return retStatus;
    }

    RvLogInfo(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
              "SipRegClientMgrConstruct - Register client manager was successfully constructed"));

	RV_UNUSED_ARG(hStack);
    return RV_OK;
}


/***************************************************************************
 * SipRegClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the register-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The register-client handle is invalid.
 *                  RV_OK -  register-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClientMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrDestruct(
                                   IN RvSipRegClientMgrHandle hRegClientMgr)
{
    RegClientMgr *pRegClientMgr;

    if (NULL == hRegClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClientMgr = (RegClientMgr *)hRegClientMgr;
    RvMutexLock(&pRegClientMgr->hLock,pRegClientMgr->pLogMgr);

    /* killing all reg clients that stayed alive */
    DestructAllRegClients(pRegClientMgr);

    /* Free all resources */
    DestructRegClientList(pRegClientMgr);
    if (NULL_PAGE != pRegClientMgr->hMemoryPage)
    {
        RPOOL_FreePage(pRegClientMgr->hGeneralMemPool, pRegClientMgr->hMemoryPage);
    }
    RvLogInfo(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
              "SipRegClientMgrDestruct - Register-client manager was destructed"));
    RvMutexUnlock(&pRegClientMgr->hLock,pRegClientMgr->pLogMgr);
    RvMutexDestruct(&pRegClientMgr->hLock,pRegClientMgr->pLogMgr);
    RvMemoryFree(pRegClientMgr, pRegClientMgr->pLogMgr);
    return RV_OK;
}





/***************************************************************************
 * SipRegClientMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the register-client module. It includes the register-client list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the reg-client manager.
 * Output:     pResources - Pointer to a RegClient resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrGetResourcesStatus (
                                 IN  RvSipRegClientMgrHandle   hMgr,
                                 OUT RvSipRegClientResources  *pResources)
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
    /* register-clients list */
    if (NULL == ((RegClientMgr *)hMgr)->hRegClientListPool)
    {
        pResources->regClients.currNumOfUsedElements = 0;
        pResources->regClients.maxUsageOfElements = 0;
        pResources->regClients.numOfAllocatedElements = 0;
    }
    else
    {
        RLIST_GetResourcesStatus(((RegClientMgr *)hMgr)->hRegClientListPool,
                             &allocNumOfLists, /*allways 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*allways 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);
        pResources->regClients.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->regClients.maxUsageOfElements = maxUsageInUserElement;
        pResources->regClients.numOfAllocatedElements = allocMaxNumOfUserElement;
    }

    return RV_OK;
}

/***************************************************************************
 * SipRegClientMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of reg-clients that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The reg-client manager.
 ***************************************************************************/
void RVCALLCONV SipRegClientMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipRegClientMgrHandle  hMgr)
{
    RegClientMgr *pMgr = (RegClientMgr*)hMgr;

    RLIST_ResetMaxUsageResourcesStatus(pMgr->hRegClientListPool);
}



#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipRegClientMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in reg-client mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the reg-client manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrSetSecAgreeMgrHandle(
                                        IN RvSipRegClientMgrHandle   hRegClientMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr)
{
    RegClientMgr* pMgr = (RegClientMgr*)hRegClientMgr;

	/* Set event handlers */
	return SipTranscClientMgrSetSecAgreeMgrHandle(pMgr->hTranscClientMgr, hSecAgreeMgr, RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT);
}
#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RegClientAllocateResources
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The register-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 *          hTransport - The transport handle.
 ***************************************************************************/
static RvStatus RVCALLCONV RegClientAllocateResources(
                                         IN  RegClientMgr      *pRegClientMgr)
{
    RvStatus rv;

    /* Construct the pool of register-clients lists. This pool
    will contain one list */
    if (0 != pRegClientMgr->maxNumOfRegClients)
    {
        /* Get a page from the general pool to use later for the Call-Id */
        rv = RPOOL_GetPage(pRegClientMgr->hGeneralMemPool, 0,
                                  &(pRegClientMgr->hMemoryPage));
        if (RV_OK != rv)
        {
            return rv;
        }

        rv = CreateRegClientList(pRegClientMgr);
        if(rv != RV_OK)
        {
            RPOOL_FreePage(pRegClientMgr->hGeneralMemPool,
                           pRegClientMgr->hMemoryPage);
        }
    }
    else
    {
        pRegClientMgr->hMemoryPage =         NULL_PAGE;
        pRegClientMgr->hRegClientListPool =  NULL;
        pRegClientMgr->hRegClientList =      NULL;
    }
    return RV_OK;
}

/***************************************************************************
 * CreateRegClientList
 * ------------------------------------------------------------------------
 * General: Allocated the list of reg-clients managed by the manager
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
static RvStatus RVCALLCONV CreateRegClientList(
                                    IN  RegClientMgr      *pRegClientMgr)
{

    RvInt32      i;
    RegClient     *regClient;
    RvStatus    rv;
	SipTranscClientOwnerUtils regClientLockUtils;
    RvUint32                   size = sizeof(SipTranscClientOwnerUtils);

    memset(&regClientLockUtils,0,size);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    regClientLockUtils.pfnOwnerLock = (OwnerObjLockFunc)RegClientLockAPI;
	regClientLockUtils.pfnOwnerUnlock = (OwnerObjUnLockFunc)RegClientUnLockAPI;
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

    pRegClientMgr->hRegClientListPool = RLIST_PoolListConstruct(
                                         pRegClientMgr->maxNumOfRegClients,
                                         1, sizeof(RegClient), pRegClientMgr->pLogMgr,
                                         "Reg client pool");
    if (NULL == pRegClientMgr->hRegClientListPool)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    /* Construct the list of register-clients */
    pRegClientMgr->hRegClientList = RLIST_ListConstruct(
                                       pRegClientMgr->hRegClientListPool);
    if (NULL == pRegClientMgr->hRegClientList)
    {
        RLIST_PoolListDestruct(pRegClientMgr->hRegClientListPool);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* initiate locks of all regClients */
    for (i=0; i < (RvInt32)pRegClientMgr->maxNumOfRegClients; i++)
    {
        rv = RLIST_InsertTail(pRegClientMgr->hRegClientListPool,
                              pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&regClient);
        regClient->pMgr = pRegClientMgr;
        
		rv = SipTranscClientConstruct(pRegClientMgr->hTranscClientMgr, 
										&regClient->transcClient, 
										(SipTranscClientOwnerHandle)regClient, 
										RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT,
										&regClientLockUtils,
										&regClient->tripleLock);
		if (RV_OK != rv)
		{
			RLIST_PoolListDestruct(pRegClientMgr->hRegClientListPool);
			return rv;
		}
		RegClientSetTranscClientEvHandlers(regClient);

    }

    for (i=0; i < (RvInt32)pRegClientMgr->maxNumOfRegClients; i++)
    {
        RLIST_GetHead(pRegClientMgr->hRegClientListPool,
                         pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&regClient);
        RLIST_Remove(pRegClientMgr->hRegClientListPool,
                     pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE)regClient);
    }
    return RV_OK;
}

/***************************************************************************
 * DestructAllRegClients
 * ------------------------------------------------------------------------
 * General: Destructs all the reg clients
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
static void RVCALLCONV DestructAllRegClients(IN  RegClientMgr      *pRegClientMgr)
{
    RegClient    *pNextToKill = NULL;
    RegClient    *pReg2Kill = NULL;
    if (NULL == pRegClientMgr->hRegClientListPool)
    {
        return;
    }

    RLIST_GetHead(pRegClientMgr->hRegClientListPool,
        pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&pReg2Kill);

    while (NULL != pReg2Kill)
    {
        RLIST_GetNext(pRegClientMgr->hRegClientListPool,pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE)pReg2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
		RegClientDestruct(pReg2Kill);
        pReg2Kill = pNextToKill;
    }

}

/***************************************************************************
 * DestructRegClientList
 * ------------------------------------------------------------------------
 * General: Destructs the list of reg-clients
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
static void RVCALLCONV DestructRegClientList(
                                    IN  RegClientMgr      *pRegClientMgr)
{

    RvUint32    i;
    RegClient    *regClient;

    if (NULL == pRegClientMgr->hRegClientListPool)
    {
        return;
    }

    for (i=0; i < pRegClientMgr->maxNumOfRegClients; i++)
    {
        RLIST_GetHead(pRegClientMgr->hRegClientListPool,
            pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&regClient);
        if (regClient == NULL)
            break;
        RLIST_Remove(pRegClientMgr->hRegClientListPool,
            pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE)regClient);
    }

    for (i=0; i < pRegClientMgr->maxNumOfRegClients; i++)
    {
        RLIST_InsertTail(pRegClientMgr->hRegClientListPool,
            pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&regClient);
    }

    for (i=0; i < pRegClientMgr->maxNumOfRegClients; i++)
    {
        RLIST_GetHead(pRegClientMgr->hRegClientListPool,
            pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE *)&regClient);

        SipCommonDestructTripleLock(
                    regClient->tripleLock, pRegClientMgr->pLogMgr);
        
        RLIST_Remove(pRegClientMgr->hRegClientListPool,
            pRegClientMgr->hRegClientList,(RLIST_ITEM_HANDLE)regClient);
    }

    RLIST_PoolListDestruct(pRegClientMgr->hRegClientListPool);
    pRegClientMgr->hRegClientListPool = NULL;
}


/***************************************************************************
 * RegClientMgrSetTranscEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the event handlers defined by the register-client module to
 *          the transaction manager, to be used when ever the transaction
 *          owner is a register client.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
static void RVCALLCONV RegClientSetTranscClientEvHandlers(
                                         IN RegClient      *pRegClient)
{
    SipTranscClientEvHandlers  regClientEvHandlers;
	
    RvUint32                   size;

    /* The implementations of all three events are set to the transaction
       manager */
    memset(&regClientEvHandlers,0,sizeof(SipTranscClientEvHandlers));
    regClientEvHandlers.pfnMsgReceivedEvHandler =
                                        RegClientTranscClientEvMsgReceived;
    regClientEvHandlers.pfnMsgToSendEvHandler =
                                        RegClientTranscClientEvMsgToSend;
    regClientEvHandlers.pfnStateChangedEvHandler =
                                        RegClientTranscClientEvStateChanged;
    regClientEvHandlers.pfnOtherURLAddressFoundEvHandler   =
                                        RegClientTranscClientEvOtherURLAddressFound;
    regClientEvHandlers.pfnFinalDestResolvedEvHandler      = 
										RegClientTranscClientFinalDestResolvedEv;
    regClientEvHandlers.pfnNewConnInUseEvHandler  = 
										RegClientTranscClientNewConnInUseEv;
	
	regClientEvHandlers.pfnObjExpiredEvHandler = 
										RegClientTranscClientObjExpiredEv;

	regClientEvHandlers.pfnObjTerminatedEvHandler = 
										RegClientTranscClientObjTerminated;
#ifdef RV_SIGCOMP_ON
    regClientEvHandlers.pfnSigCompMsgNotRespondedEvHandler =
                                        RegClientTranscClientSigCompMsgNotRespondedEv;
#endif /* RV_SIGCOMP_ON */

    size = sizeof(SipTranscClientEvHandlers);
	
	SipTranscClientSetEvHandlers(&pRegClient->transcClient,
												&regClientEvHandlers, size);
}

#endif /* RV_SIP_PRIMITIVES */





















