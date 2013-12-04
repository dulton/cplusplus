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
 *                              <RvSipMid.c>
 *
 * This file contains all API functions relevant to intrer acting with core lever 
 * functionality, including select poll and timers
 *
 * select/poll 
 * ------------------------
 * Enables an application to register event for select loop or poll queries.
 * Enables an application to perform a select loop or a poll loop
 *
 * timers
 * -----------------
 * Enables an application to set and release timers.
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvccore.h"

#include "RvSipMidTypes.h"
#include "MidMgrObject.h"
#include "RvSipMid.h"
#include "_SipCommonTypes.h"
#include "MidSelect.h"
#include "MidTimer.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS  IMPLEMENTATION                    */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * MidMgrAllocateResources
 * ----------------------------------------------------------------------------------
 * General: Allocates timer lists and select FD lists.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   - Pointer to the middle layer manager.
 *          pMidCfg   - The size of the configuration structure.
 * Output:  -
 ***********************************************************************************/
RvStatus RVCALLCONV MidMgrAllocateResources(
                                         IN    MidMgr*      pMidMgr,
                                         IN    RvSipMidCfg* pMidCfg)
{
    RvStatus    rv      = RV_OK;
    
    if (0 < pMidCfg->maxUserTimers)
    {
        /* allocating timer list pool */
        pMidMgr->userTimerPool = RLIST_PoolListConstruct(pMidCfg->maxUserTimers,
                                                         1,sizeof(MidTimer),pMidMgr->pLogMgr,"User Timer");
        if (NULL == pMidMgr->userTimerPool)
        {
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* allocating timer list */
        pMidMgr->hUserTimers = RLIST_ListConstruct(pMidMgr->userTimerPool);
    
        if (NULL == pMidMgr->hUserTimers)
        {
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    if (0 < pMidCfg->maxUserFd)
    {
        pMidMgr->userFDPool = RLIST_PoolListConstruct(pMidCfg->maxUserFd,
                                                      1,sizeof(SeliUserFd),pMidMgr->pLogMgr,"User Fd");
        if (NULL == pMidMgr->userFDPool)
        {
            MidMgrFreeResources(pMidMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* allocating timer list */
        pMidMgr->hUserFds = RLIST_ListConstruct(pMidMgr->userFDPool);
    
        if (NULL == pMidMgr->hUserFds)
        {
            MidMgrFreeResources(pMidMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }

        pMidMgr->maxTimers = pMidCfg->maxUserTimers;
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    rv = RvSelectConstruct(4100,pMidCfg->maxUserTimers,pMidMgr->pLogMgr,&pMidMgr->pSelect);
#else
    rv = RvSelectConstruct(2048,pMidCfg->maxUserTimers,pMidMgr->pLogMgr,&pMidMgr->pSelect);
#endif
    //SPIRENT_END
    if (RV_OK != rv)
    {
        MidMgrFreeResources(pMidMgr);
        return rv;
    }
    return RV_OK;
}


/************************************************************************************
 * MidMgrFreeResources
 * ----------------------------------------------------------------------------------
 * General: frees timer lists and select FD lists.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   - Pointer to the middle layer manager.
 * Output:  -
 ***********************************************************************************/
void RVCALLCONV MidMgrFreeResources(IN    MidMgr*      pMidMgr)
{
    if (NULL != pMidMgr->userTimerPool)
    {
        RLIST_PoolListDestruct(pMidMgr->userTimerPool);
        pMidMgr->userTimerPool = NULL;
    }

    if (NULL != pMidMgr->userFDPool)
    {
        RLIST_PoolListDestruct(pMidMgr->userFDPool);
        pMidMgr->userFDPool = NULL;
    }
    if (NULL != pMidMgr->pSelect)
    {
        RvSelectDestruct(pMidMgr->pSelect, pMidMgr->maxTimers);
        pMidMgr->pSelect=NULL;
    }
}

/************************************************************************************
 * MidMgrRemovePendingRegistrations
 * ----------------------------------------------------------------------------------
 * General: removes all timers wating on select engine.
 *          removes all select registrations
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   - Pointer to the middle layer manager.
 * Output:  -
 ***********************************************************************************/
void RVCALLCONV MidMgrRemovePendingRegistrations(IN    MidMgr*      pMidMgr)
{
    /* 1. Remove all pending timers */
    if (NULL != pMidMgr->hUserTimers)
    {
        MidTimerRemovePendingTimers(pMidMgr);
    }
    /* 2. Remove all select registrations*/
    if (NULL != pMidMgr->hUserFds)
    {
        MidSelectRemovePendingCallOns(pMidMgr);
    }
}

