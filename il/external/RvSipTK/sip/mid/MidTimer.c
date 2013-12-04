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
 *                              <MidTimer.c>
 *
 * This file contains Timer functionability
 *
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
#include <string.h>

#include "AdsRlist.h"

#include "RvSipMidTypes.h"
#include "MidMgrObject.h"
#include "RvSipMid.h"
#include "_SipCommonTypes.h"
#include "_SipCommonCore.h"
#include "MidSelect.h"
#include "MidTimer.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvBool MidTimerFunc(void *voidMidTimer);

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS  IMPLEMENTATION                    */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * MidTimerRemovePendingTimers
 * ----------------------------------------------------------------------------------
 * General: removes all pending timers from timer queue
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 * Output:  
 ***********************************************************************************/
RvStatus RVCALLCONV MidTimerRemovePendingTimers(IN MidMgr*     pMidMgr)
{
    RvStatus            rv          = RV_OK;
    RLIST_ITEM_HANDLE   listItem    = NULL;
    MidTimer*           pMidTimer   = NULL;
    
    RvMutexLock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);

    /* Check if there were no application timers at all */
    if (NULL == pMidMgr->userTimerPool)
    {
        RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
        return RV_OK;
    }

    RLIST_GetHead(pMidMgr->userTimerPool,pMidMgr->hUserTimers,&listItem);
    while (NULL != listItem)
    {
        pMidTimer = (MidTimer*)listItem;
        MidTimerReset(pMidMgr,pMidTimer);
        listItem = NULL;
        RLIST_GetHead(pMidMgr->userTimerPool,pMidMgr->hUserTimers,&listItem);
    }
    RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    return rv;
}

/************************************************************************************
 * MidTimerSet
 * ----------------------------------------------------------------------------------
 * General: Sets a new timer
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr     -- pointer to the middle layer manager.
 *          miliTimeOut -- experation specified in miliseconds
 *          cb          -- application callback
 *          ctx         -- context to be called when timer expires
 * Output:  pTimer      -- a newly allocated timer
 ***********************************************************************************/
RvStatus RVCALLCONV MidTimerSet(IN MidMgr*                pMidMgr,
                                IN RvUint32               miliTimeOut,
                                IN RvSipMidTimerExpEv     cb,
                                IN void*                  ctx,
                                OUT MidTimer**            pTimer)
{
    RvStatus    rv = RV_OK;
    MidTimer*   pMidTimer = NULL;
    RLIST_ITEM_HANDLE  listItem = NULL;
    
    /* allocate a timer */
    RvMutexLock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    rv = RLIST_InsertTail(pMidMgr->userTimerPool,pMidMgr->hUserTimers ,&listItem);
    RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    if (RV_OK != rv)
    {
        return rv;
    }
    pMidTimer = (MidTimer*)listItem;
    memset(pMidTimer,0,sizeof(MidTimer));
    pMidTimer->cb = cb;
    pMidTimer->ctx = ctx;
    pMidTimer->pMgr = pMidMgr;

    rv = SipCommonCoreRvTimerStart(&pMidTimer->sipTimer,pMidMgr->pSelect,miliTimeOut,MidTimerFunc,(void*)pMidTimer);
    if (RV_OK != rv)
    {
        RvMutexLock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
        RLIST_Remove(pMidMgr->userTimerPool,pMidMgr->hUserTimers ,listItem);
        RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
        return rv;
    }
    *pTimer = pMidTimer;
    return rv;
}

/************************************************************************************
 * MidTimerReset
 * ----------------------------------------------------------------------------------
 * General: releases a timer
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 * Output:  phTimer   -- handle of timer to delete
 ***********************************************************************************/
RvStatus RVCALLCONV MidTimerReset(IN MidMgr*     pMidMgr,
                                  IN MidTimer*   pMidTimer)
{
    RvStatus rv = RV_OK;

    rv = SipCommonCoreRvTimerCancel(&pMidTimer->sipTimer);
    RvMutexLock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    RLIST_Remove(pMidMgr->userTimerPool,pMidMgr->hUserTimers ,(RLIST_ITEM_HANDLE)pMidTimer);
    RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/********************************************************************************************
 * MidTimerFunc
 *	
 * purpose: This is the function that should be called when a timer is triggered.
 * INPUT  : voidMidTimer - the midlle timer casted as void
 * OUTPUT : none
 * RETURN : Return value only affects PERIODIC timers. A return value of RV_TRUE
 *			indicates that the timer should be rescheduled. A value of RV_FALSE
 *			indicates that the timer should not be rescheduled (and is, in effect,
 *			canceled).
 ********************************************************************************************/
static RvBool MidTimerFunc(void *voidMidTimer)
{
    void*               ctx;
    RvSipMidTimerExpEv  cb;
    MidTimer*           pMidTimer = (MidTimer*)voidMidTimer;
    MidMgr*             pMidMgr = pMidTimer->pMgr;
    
    /* first copy the timer data */
    cb = pMidTimer->cb;
    ctx = pMidTimer->ctx;

    /* remove the timer from the timer list */
    RvMutexLock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
    RLIST_Remove(pMidMgr->userTimerPool,pMidMgr->hUserTimers ,(RLIST_ITEM_HANDLE)pMidTimer);
    RvMutexUnlock(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);

    /* call user callback */
    cb(ctx);

    /* dont ever redo timers */
    return RV_FALSE;
}
