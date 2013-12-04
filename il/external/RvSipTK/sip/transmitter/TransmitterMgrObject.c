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
 *                              <TransmitterMgrObject.c>
 *   This file includes the functionality of the Transmitter manager object.
 *   The Transmitter manager is responsible for creating new Transmitter.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  January 2004
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "TransmitterMgrObject.h"
#include "TransmitterObject.h"
#include "rvmemory.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipTransport.h"
#ifdef RV_SIGCOMP_ON
#include "RvSipCompartment.h"
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV TransmittersListConstruct(
                                     IN TransmitterMgr*   pTrxMgr);
static void RVCALLCONV TransmittersListDestruct(
                                     IN TransmitterMgr*   pTrxMgr);

static void RVCALLCONV DestructAllTransmitters(IN TransmitterMgr*   pTrxMgr);


/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER MGR FUNCTIONS                         */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransmitterMgrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs the Transmitter manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg        - The module configuration.
 *          pStack      - A handle to the stack manager.
 * Output:  *ppTrxMgr   - Pointer to the newly created Transmitter manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrConstruct(
                            IN  SipTransmitterMgrCfg*         pCfg,
                            IN  void*                         pStack,
                            OUT TransmitterMgr**              ppTrxMgr)
{
    RvStatus           rv               = RV_OK;
    RvStatus           crv              = RV_OK;
    TransmitterMgr*    pTrxMgr          = NULL;
    RvSelectEngine*    pSelect          = NULL;

    *ppTrxMgr = NULL;
    
    /*allocate the transmitter manager*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(TransmitterMgr), pCfg->pLogMgr, (void*)&pTrxMgr))
    {
         RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                   "TransmitterMgrConstruct - Failed to allocate memory for the transmitter manager"));
         return RV_ERROR_UNKNOWN;
    }
    memset(pTrxMgr,0,sizeof(TransmitterMgr));

    /*set information from the configuration structure*/
    pTrxMgr->pLogMgr             = pCfg->pLogMgr;
    pTrxMgr->pLogSrc             = pCfg->pLogSrc;
    pTrxMgr->hTransportMgr       = pCfg->hTransportMgr;
    pTrxMgr->hMsgMgr             = pCfg->hMsgMgr;
    pTrxMgr->bUseRportParamInVia = pCfg->bUseRportParamInVia;
    pTrxMgr->hRslvMgr            = pCfg->hRslvMgr;
#ifdef RV_SIP_IMS_ON
    pTrxMgr->hSecMgr             = pCfg->hSecMgr;
#endif
#ifdef RV_SIGCOMP_ON
    pTrxMgr->hCompartmentMgr= pCfg->hCompartmentMgr;
#endif
    pTrxMgr->bIsPersistent  = pCfg->bIsPersistent;
    pTrxMgr->seed           = pCfg->seed;
    pTrxMgr->maxNumOfTrx    = pCfg->maxNumOfTrx;
    pTrxMgr->hGeneralPool   = pCfg->hGeneralPool;
    pTrxMgr->hMessagePool   = pCfg->hMessagePool;
    pTrxMgr->hElementPool   = pCfg->hElementPool;
    pTrxMgr->pStack         = pStack;
    pTrxMgr->maxMessages    = pCfg->maxMessages;
    pTrxMgr->bResolveTelUrls= pCfg->bResolveTelUrls;
    
    /* allocate mutex*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RvMutex), pTrxMgr->pLogMgr, (void*)&pTrxMgr->pMutex))
    {
         RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "TransmitterMgrConstruct - Failed to allocate mutex for the transmitter manager"));
         return RV_ERROR_UNKNOWN;
    }

    crv = RvMutexConstruct(pTrxMgr->pLogMgr, pTrxMgr->pMutex);
    if(crv != RV_OK)
    {
         RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "TransmitterMgrConstruct - Failed to construct mutex for the transmitter manager"));
         RvMemoryFree((void*)pTrxMgr->pMutex,pTrxMgr->pLogMgr);
         pTrxMgr->pMutex = NULL;
         return CRV2RV(crv);
    }

    rv = RvSelectGetThreadEngine(pTrxMgr->pLogMgr,&pSelect);
    if (rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "TransmitterMgrConstruct - Failed to obtain select engine"));
        return CRV2RV(rv);
    }

    /* construct transmitters resources */
    rv = TransmittersListConstruct(pTrxMgr);
    if(rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "TransmitterMgrConstruct - Failed to construct transmitters list"));
        return rv;
    }

    *ppTrxMgr = pTrxMgr;

    return RV_OK;
}

/***************************************************************************
 * TransmitterMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the Transmitter manager and free all its resources.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrxMgr     - The Transmitter manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrDestruct(
                            IN TransmitterMgr*   pTrxMgr)
{
    RvLogMgr*   pLogMgr = NULL;
    /*noting to free*/
    if(pTrxMgr == NULL)
    {
        return RV_OK;
    }

    pLogMgr = pTrxMgr->pLogMgr;
    if(pTrxMgr->pMutex != NULL)
    {
        RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
    }

    DestructAllTransmitters(pTrxMgr);

    TransmittersListDestruct(pTrxMgr);

    if(pTrxMgr->pMutex != NULL)
    {
        RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
        RvMutexDestruct(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
        RvMemoryFree((void*)pTrxMgr->pMutex,pTrxMgr->pLogMgr);
    }

    RvMemoryFree((void*)pTrxMgr,pLogMgr);

    return RV_OK;
}



/***************************************************************************
 * TransmitterMgrCreateTransmitter
 * ------------------------------------------------------------------------
 * General: Creates a new transmitter
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr   - Handle to the Transmitter manager
 *            bIsAppTrx - indicates whether the transmitter was created
 *                        by the application or by the stack.
 *            hAppTrx   - Application handle to the transmitter.
 *            hPool     - A pool for this transmitter
 *            hPage     - A memory page to be used by this transmitter. If NULL
 *                        is supplied the transmitter will allocate a new page.
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 * Output:     phTrx    - sip stack handle to the new Transmitter
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrCreateTransmitter(
                   IN     TransmitterMgr            *pTrxMgr,
                   IN     RvBool                    bIsAppTrx,
                   IN     RvSipAppTransmitterHandle hAppTrx,
                   IN     HRPOOL                    hPool,
                   IN     HPAGE                     hPage,
                   IN     SipTripleLock*            pTripleLock,
                   OUT    RvSipTransmitterHandle    *phTrx)
{
    Transmitter*            pTrx        = NULL;
    RLIST_ITEM_HANDLE       hListItem   = NULL;
    RvStatus                rv          = RV_OK;

    *phTrx  = NULL;
    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*lock the transmitter mgr*/

    RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
               "TransmitterMgrCreateTransmitter - Creating a new transmitter..."));

    /*allocate a new transmitter object*/
    rv = RLIST_InsertTail(pTrxMgr->hTrxListPool,
                          pTrxMgr->hTrxList,
                          &hListItem);

    if(rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "TransmitterMgrCreateTransmitter - Failed to create a new transmitter (rv=%d)",rv));
        RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
        return rv;
    }

    pTrx = (Transmitter*)hListItem;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if (pTrx->pTrxMgr == 0)
    {
        RvStatus rvInit = RV_OK;
        pTrx->pTrxMgr = pTrxMgr; 
        rvInit = SipCommonConstructTripleLock(
                                    &pTrx->trxTripleLock, pTrxMgr->pLogMgr);
        if(rvInit != RV_OK)
        {
            RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                       "TransmitterMgrCreateTransmitter - Failed to initialize the tripleLock (rv=%d)",rvInit));
            pTrx->pTripleLock = NULL;
            RLIST_Remove(pTrxMgr->hTrxListPool,pTrxMgr->hTrxList,
                         (RLIST_ITEM_HANDLE)pTrx);
            RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
            return rvInit;
        }
    }

#endif
/* SPIRENT_END */  
    rv = TransmitterInitialize(pTrx,bIsAppTrx,hAppTrx,hPool,hPage,pTripleLock);
    if(rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "TransmitterMgrCreateTransmitter - Failed to initialize the transmitter (rv=%d)",rv));
        RLIST_Remove(pTrxMgr->hTrxListPool,pTrxMgr->hTrxList,
                     (RLIST_ITEM_HANDLE)pTrx);
        RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
        return rv;
    }

    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*unlock the manager*/

    *phTrx = (RvSipTransmitterHandle)pTrx;
    RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
               "TransmitterMgrCreateTransmitter - Transmitter 0x%p was created successfully, pTripleLock=0x%p",
               pTrx,pTrx->pTripleLock));

    return RV_OK;
}

/***************************************************************************
 * TransmitterMgrRemoveTransmitter
 * ------------------------------------------------------------------------
 * General: removes a transmitter object from the transmitters list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr   - Handle to the Transmitter manager
 *            hTrx      - Handle to the Transmitter
 ***************************************************************************/
void RVCALLCONV TransmitterMgrRemoveTransmitter(
                                IN TransmitterMgr*        pTrxMgr,
                                IN RvSipTransmitterHandle hTrx)
{
    Transmitter*       pTrx        = (Transmitter*)hTrx;

    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*lock the transmitter manager*/

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
               "TransmitterMgrRemoveTransmitter - removing transmitter 0x%p from list",pTrx));

    RLIST_Remove(pTrxMgr->hTrxListPool,pTrxMgr->hTrxList,
                     (RLIST_ITEM_HANDLE)pTrx);
    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*unlock the manager*/
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransmittersListConstruct
 * ------------------------------------------------------------------------
 * General: Allocated the list of transmitters and initialize their locks.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrxMgr   - Pointer to the transmitter manager
 ***************************************************************************/
static RvStatus RVCALLCONV TransmittersListConstruct(
                                          IN TransmitterMgr*   pTrxMgr)
{
    Transmitter*    pTrx = NULL;
    RvStatus        rv   = RV_OK;

    /*allocate a pool with 1 list*/
    pTrxMgr->hTrxListPool = RLIST_PoolListConstruct(pTrxMgr->maxNumOfTrx,
                                                    1,
                                                    sizeof(Transmitter),
                                                    pTrxMgr->pLogMgr ,
                                                    "Transmitters List");

    if(pTrxMgr->hTrxListPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pTrxMgr->hTrxList = RLIST_ListConstruct(pTrxMgr->hTrxListPool);

    if(pTrxMgr->hTrxList == NULL)
    {
        TransmittersListDestruct(pTrxMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }


    RLIST_PoolGetFirstElem(pTrxMgr->hTrxListPool,
                           (RLIST_ITEM_HANDLE*)&pTrx);
    while (NULL != pTrx)
    {
        RvStatus rvInit = RV_OK;
        pTrx->pTrxMgr = pTrxMgr;
        rvInit = SipCommonConstructTripleLock(
                                    &pTrx->trxTripleLock, pTrxMgr->pLogMgr);
        pTrx->pTripleLock = NULL;
        RLIST_PoolGetNextElem(pTrxMgr->hTrxListPool,
                              (RLIST_ITEM_HANDLE)pTrx,
                              (RLIST_ITEM_HANDLE*)&pTrx);
        if(rvInit != RV_OK)
        {
            rv = rvInit;
        }
    }

    if(rv != RV_OK)
    {
        TransmittersListDestruct(pTrxMgr);
    }
    return RV_OK;
}

/***************************************************************************
 * DestructAllTransmitters
 * ------------------------------------------------------------------------
 * General: Destructs the list of transmitters and free their locks.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrxMgr   - Pointer to the transmitter manager
 ***************************************************************************/
static void RVCALLCONV DestructAllTransmitters(IN TransmitterMgr*   pTrxMgr)
{
    Transmitter    *pNextToKill = NULL;
    Transmitter    *pTrx2Kill = NULL;

    if(pTrxMgr->hTrxListPool == NULL)
    {
        return;
    }


    RLIST_GetHead(pTrxMgr->hTrxListPool,pTrxMgr->hTrxList,(RLIST_ITEM_HANDLE *)&pTrx2Kill);

    while (NULL != pTrx2Kill)
    {
        RLIST_GetNext(pTrxMgr->hTrxListPool,pTrxMgr->hTrxList,(RLIST_ITEM_HANDLE)pTrx2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
        TransmitterDestruct(pTrx2Kill);
        pTrx2Kill = pNextToKill;
    }
}

/***************************************************************************
 * TransmittersListDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the list of transmitters and free their locks.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrxMgr   - Pointer to the transmitter manager
 ***************************************************************************/
static void RVCALLCONV TransmittersListDestruct(IN TransmitterMgr*   pTrxMgr)
{
    Transmitter       *pTrx;

    if(pTrxMgr->hTrxListPool == NULL)
    {
        return;
    }

    if(pTrxMgr->hTrxList == NULL)
    {
        RLIST_PoolListDestruct(pTrxMgr->hTrxListPool);
        pTrxMgr->hTrxListPool = NULL;
        return;
    }

    /*free triple locks.*/
    RLIST_PoolGetFirstElem(pTrxMgr->hTrxListPool,
                           (RLIST_ITEM_HANDLE*)&pTrx);
    while (NULL != pTrx)
    {
        pTrx->pTrxMgr = pTrxMgr;
        SipCommonDestructTripleLock(&pTrx->trxTripleLock, pTrxMgr->pLogMgr);
        RLIST_PoolGetNextElem(pTrxMgr->hTrxListPool,
                              (RLIST_ITEM_HANDLE)pTrx,
                              (RLIST_ITEM_HANDLE*)&pTrx);
    }
    /*free the list pool*/
    RLIST_PoolListDestruct(pTrxMgr->hTrxListPool);
    pTrxMgr->hTrxListPool = NULL;
    pTrxMgr->hTrxList     = NULL;
}

