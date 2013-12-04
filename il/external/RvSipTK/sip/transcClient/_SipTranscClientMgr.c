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
 *                              <SipTranscClientMgr.c>
 *
 *  The SipTranscClientMgr.c file contains Internal Api functions Including the
 *  construct and destruct of the transc-client manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"
#include "_SipTranscClientMgr.h"
#include "TranscClientMgrObject.h"
#include "TranscClientObject.h"
#include "TranscClientTranscEv.h"
#include "rvmemory.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeTypes.h"
#include "_SipSecAgreeMgr.h"
#include "TranscClientSecAgree.h"
#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus RVCALLCONV TranscClientAllocateResources(
                                         IN  TranscClientMgr      *pTranscClientMgr);

static RvStatus RVCALLCONV TranscClientMgrSetTranscEvHandlers(
															  IN TranscClientMgr      *pTranscClientMgr);
#ifdef RV_SIP_IMS_ON
static void RVCALLCONV SetSecAgreeEvHandlers(
											 IN RvSipSecAgreeMgrHandle   hSecAgreeMgr,
											 IN RvSipCommonStackObjectType eOwnerType);
#endif /*RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipTranscClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new transc-client manager. The transc-client
 *          manager is encharged of all the register-clients and publish-clients. 
 *			The manager holds a list of register-clients and publish-clients and is used to create new
 *          regClients and pubClients.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the transc-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   transc-client manager.
 *               RV_OK -        transc-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each transc-client for internal
 *                          memory allocations.
 *          hAuthModule   - A handle to the authentication module
 * Input and Output:
 *          maxNumTranscClients - Max number of concurrent register-clients and publish-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack - Handle to the stack manager.
 * Output:     *phTranscClientMgr - Handle to the newly created transc-client
 *                            manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrConstruct(
                        INOUT SipTranscClientMgrCfg       *pCfg,
                         IN    void*          hStack,
                         OUT   SipTranscClientMgrHandle  *phTranscClientMgr)
{
    TranscClientMgr *pTranscClientMgr;
    RvStatus     retStatus;
    RvStatus      crv;
    if (NULL == phTranscClientMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    if ((NULL == pCfg->hTranscMgr) ||
        (NULL == pCfg->hGeneralPool) ||
        (NULL == pCfg->hTransport))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Allocate a new transaction-client manager object */
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(TranscClientMgr), pCfg->pLogMgr, (void*)&pTranscClientMgr))
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
                  "SipTranscClientMgrConstruct - Error trying to construct a new Transc-clients manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    *phTranscClientMgr = (SipTranscClientMgrHandle)pTranscClientMgr;
    pTranscClientMgr->pLogMgr         = pCfg->pLogMgr;
    pTranscClientMgr->hGeneralMemPool = pCfg->hGeneralPool;
    pTranscClientMgr->hMsgMemPool     = pCfg->hMessagePool;
    pTranscClientMgr->hTranscMgr      = pCfg->hTranscMgr;
    pTranscClientMgr->hMsgMgr         = pCfg->hMsgMgr;
    pTranscClientMgr->pLogSrc           = pCfg->moduleLogId;
    pTranscClientMgr->seed            = pCfg->seed;
    pTranscClientMgr->hTransportMgr   = pCfg->hTransport;
    pTranscClientMgr->hStack          = hStack;
#ifdef RV_SIP_AUTH_ON
    pTranscClientMgr->hAuthModule     = pCfg->hAuthModule;
#endif /* #ifdef RV_SIP_AUTH_ON */
    pTranscClientMgr->bIsPersistent   = pCfg->bIsPersistent;
#ifdef RV_SIP_IMS_ON
    pTranscClientMgr->hSecMgr         = pCfg->hSecMgr;
	pTranscClientMgr->hSecAgreeMgr	  = pCfg->hSecAgreeMgr;
#endif /* #ifdef RV_SIP_IMS_ON */

	RvSelectGetThreadEngine(pTranscClientMgr->pLogMgr, &pTranscClientMgr->pSelect);

    crv = RvMutexConstruct(pTranscClientMgr->pLogMgr, &pTranscClientMgr->hLock);
    if(crv != RV_OK)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
                  "SipTranscClientMgrConstruct - Failed to allocate a mutex for the transc-client manager (rv=0x%p)",CRV2RV(crv)));
        RvMemoryFree(pTranscClientMgr, pTranscClientMgr->pLogMgr);
        return CRV2RV(crv);
    }

    /* Allocate all required resources */
    retStatus = TranscClientAllocateResources(pTranscClientMgr);
    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
                  "SipTranscClientMgrConstruct - Error trying to construct a new transaction-clients manager"));
        RvMemoryFree(pTranscClientMgr, pTranscClientMgr->pLogMgr);
        *phTranscClientMgr = NULL;
        return retStatus;
    }

    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
                  "SipTranscClientMgrConstruct - Error trying to construct a new transc-clients manager"));
        SipTranscClientMgrDestruct(*phTranscClientMgr);
        *phTranscClientMgr = NULL;
        return retStatus;
    }
	
	retStatus = TranscClientMgrSetTranscEvHandlers(pTranscClientMgr);
	if (RV_OK != retStatus)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
			"SipTranscClientMgrConstruct - Error trying to construct a new transaction-clients manager"));
        SipTranscClientMgrDestruct(*phTranscClientMgr);
        *phTranscClientMgr = NULL;
        return retStatus;
    }

    RvLogInfo(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
              "SipTranscClientMgrConstruct - Transc client manager was successfully constructed"));
    return RV_OK;
}


/***************************************************************************
 * SipTranscClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the transaction-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The transaction-client handle is invalid.
 *                  RV_OK -  transaction-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClientMgr - Handle to the manager to destruct.
 *			eOwnerType		 - The owner that asked to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrDestruct(
                                   IN SipTranscClientMgrHandle hTranscClientMgr)
{
    TranscClientMgr *pTranscClientMgr;

    if (NULL == hTranscClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTranscClientMgr = (TranscClientMgr *)hTranscClientMgr;
    RvMutexLock(&pTranscClientMgr->hLock,pTranscClientMgr->pLogMgr);

    if (NULL_PAGE != pTranscClientMgr->hMemoryPage)
    {
        RPOOL_FreePage(pTranscClientMgr->hGeneralMemPool, pTranscClientMgr->hMemoryPage);
    }
    RvLogInfo(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
              "SipTranscClientMgrDestruct - Transaction-client manager was destructed"));
    RvMutexUnlock(&pTranscClientMgr->hLock,pTranscClientMgr->pLogMgr);
    RvMutexDestruct(&pTranscClientMgr->hLock,pTranscClientMgr->pLogMgr);
    RvMemoryFree(pTranscClientMgr, pTranscClientMgr->pLogMgr);
    return RV_OK;
}


/***************************************************************************
 * SipTranscClientConstruct
 * ------------------------------------------------------------------------
 * General: Populate all relevalt memebers of this transaction-client object.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The transaction-client handle is invalid.
 *                  RV_OK -  transaction-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClientMgr - Handle to the manager to destruct.
 *			eOwnerType		 - The owner that asked to destruct.
 ***************************************************************************/
RV_Status SipTranscClientConstruct(SipTranscClientMgrHandle	 hTranscClientMgr, 
							  SipTranscClient				 *pTranscClient, 
							  SipTranscClientOwnerHandle	 hOwner,
							  RvSipCommonStackObjectType	 eOwnerType,
							  SipTranscClientOwnerUtils		 *pOwnerLockUtils,
							  SipTripleLock					 **tripleLock)
{
	RvStatus				rv				 = RV_OK;
	TranscClientMgr			*pTranscClientMgr = (TranscClientMgr*)hTranscClientMgr;
	pTranscClient->pMgr						 = pTranscClientMgr;
	pTranscClient->hOwner					 = hOwner;
	pTranscClient->eOwnerType				 = eOwnerType;

	memset(&pTranscClient->ownerLockUtils, 0, sizeof(SipTranscClientOwnerUtils));
	memcpy(&pTranscClient->ownerLockUtils, pOwnerLockUtils, sizeof(SipTranscClientOwnerUtils));
	
	rv = SipCommonConstructTripleLock(
		&pTranscClient->transcClientTripleLock, pTranscClient->pMgr->pLogMgr);
	if (rv != RV_OK)
	{
		RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
			"SipTranscClientConstruct - SipCommonConstructTripleLock failed (rv=0x%p)", rv));
	}
	*tripleLock = &pTranscClient->transcClientTripleLock;
	pTranscClient->tripleLock = NULL;
	return rv;
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
RvStatus RVCALLCONV SipTranscClientMgrSetSecAgreeMgrHandle(
                                        IN SipTranscClientMgrHandle   hTranscClientMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr,
										IN RvSipCommonStackObjectType eOwnerType)
{
    TranscClientMgr* pMgr = (TranscClientMgr*)hTranscClientMgr;

	/* Update handle */
    pMgr->hSecAgreeMgr = hSecAgreeMgr;

	SetSecAgreeEvHandlers(hSecAgreeMgr, eOwnerType);

    return RV_OK;
}

#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientAllocateResources
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The transaction-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClientMgr - Pointer to the register-clients manager.
 *          hTransport - The transport handle.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscClientAllocateResources(
                                         IN  TranscClientMgr      *pTranscClientMgr)
{
    RvStatus rv;

    /* Get a page from the general pool to use later for the Call-Id */
    rv = RPOOL_GetPage(pTranscClientMgr->hGeneralMemPool, 0,
                              &(pTranscClientMgr->hMemoryPage));
    return rv;
}


/***************************************************************************
* TranscClientMgrSetTranscEvHandlers
* ------------------------------------------------------------------------
* General: Set the event handlers defined by the register-client module to
*          the transaction manager, to be used when ever the transaction
*          owner is a transaction client.
* Return Value: -.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pTranscClientMgr - Pointer to the transaction-clients manager.
***************************************************************************/
static RvStatus RVCALLCONV TranscClientMgrSetTranscEvHandlers(
														   IN TranscClientMgr      *pTranscClientMgr)
{
    SipTransactionMgrEvHandlers mgrEvHandlers;
    RvSipTransactionEvHandlers  transcClientEvHandlers;
    RvUint32                   size;
    RvStatus                   retStatus;
	
    /* No event handlers are defined for the manager since there are no
	server registrars, only register-client */
    mgrEvHandlers.pfnEvTransactionCreated = NULL;
    mgrEvHandlers.pMgr = NULL;
    /* The implementations of all three events are set to the transaction
	manager */
    memset(&transcClientEvHandlers,0,sizeof(RvSipTransactionEvHandlers));
    transcClientEvHandlers.pfnEvMsgReceived =
								TranscClientTranscEvMsgReceived;
    transcClientEvHandlers.pfnEvMsgToSend =
								TranscClientTranscEvMsgToSend;
    transcClientEvHandlers.pfnEvStateChanged =
								TranscClientTranscEvStateChanged;
    transcClientEvHandlers.pfnEvOtherURLAddressFound   =
								TranscClientTranscEvOtherURLAddressFound;
    transcClientEvHandlers.pfnEvFinalDestResolved      = 
								TranscClientTranscFinalDestResolvedEv;
    transcClientEvHandlers.pfnEvNewConnInUse  = TranscClientTranscNewConnInUseEv;
#ifdef RV_SIGCOMP_ON
    transcClientEvHandlers.pfnEvSigCompMsgNotResponded =
								TranscClientTranscSigCompMsgNotRespondedEv;
#endif /* RV_SIGCOMP_ON */
	
    size = sizeof(RvSipTransactionEvHandlers);
    retStatus = SipTransactionMgrSetEvHandlers(
		pTranscClientMgr->hTranscMgr, SIP_TRANSACTION_OWNER_TRANSC_CLIENT,
		&mgrEvHandlers, sizeof(mgrEvHandlers), &transcClientEvHandlers, size);
    return retStatus;
}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SetSecAgreeEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the security-agreement events handlers to the security-agreement 
 *          module.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
static void RVCALLCONV SetSecAgreeEvHandlers(
                               IN RvSipSecAgreeMgrHandle   hSecAgreeMgr,
							   IN RvSipCommonStackObjectType eOwnerType)
{
	SipSecAgreeEvHandlers   secAgreeEvHandlers;

    memset(&secAgreeEvHandlers,0,sizeof(SipSecAgreeEvHandlers));
	secAgreeEvHandlers.pfnSecAgreeAttachSecObjEvHanlder = TranscClientSecAgreeAttachSecObjEv;
    secAgreeEvHandlers.pfnSecAgreeDetachSecAgreeEvHanlder = TranscClientSecAgreeDetachSecAgreeEv;
	
    SipSecAgreeMgrSetEvHandlers(
                             hSecAgreeMgr,
                             &secAgreeEvHandlers,
							 eOwnerType);
}
#endif /* RV_SIP_IMS_ON */


#endif /* RV_SIP_PRIMITIVES */










































