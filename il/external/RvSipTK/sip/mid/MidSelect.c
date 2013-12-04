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
 *                              <MidSelect.c>
 *
 * This file contains Select functionability
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
#include "rvselect.h"
#include <string.h>

#include "AdsRlist.h"

#include "RvSipMidTypes.h"
#include "MidMgrObject.h"
#include "RvSipMid.h"
#include "_SipCommonTypes.h"
#include "MidSelect.h"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void seliEventsCB(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error);

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS  IMPLEMENTATION                    */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * MidSelectCallOn
 * ----------------------------------------------------------------------------------
 * General: 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 *          fd        -- OS file descriptor
 *          events    -- read/write
 *          pCallBack -- user callback
 *          ctx       -- usr context
 * Output:  
 ***********************************************************************************/
RvStatus RVCALLCONV MidSelectCallOn(IN MidMgr*     pMidMgr,
                                           IN RvInt32              fd,
                                           IN RvSipMidSelectEvent  events,
                                           IN RvSipMidSelectEv     pCallBack,
                                           IN void*                ctx)
{
    RvStatus    rv      = RV_OK;
    RvSocket s;
    RvSelectFd* coreFd;
    SeliUserFd* userFd;
    RvSelectEvents coreEvents = 0;
    RLIST_ITEM_HANDLE  listItem = NULL;

    /* Convert the events to the core's events */
    if (events & RVSIP_MID_SELECT_READ) 
    {
        coreEvents |= RvSelectRead;
    }
    if (events & RVSIP_MID_SELECT_WRITE) 
    {
        coreEvents |= RvSelectWrite;
    }


    /* Look for this fd if we're currently waiting for events on it */
    s = (RvSocket)fd;
    coreFd = RvSelectFindFd(pMidMgr->pSelect, &s);

    if ((coreFd == NULL) && ((RvInt)events != 0) && (pCallBack != NULL))
    {
        /* This is a new fd we're waiting on - add it */

        /* Allocate an FD for the user */
        if (pMidMgr->hUserFds == NULL)
        {
            return RV_ERROR_UNINITIALIZED;
        }

        RvMutexLock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
        rv = RLIST_InsertTail(pMidMgr->userFDPool,pMidMgr->hUserFds ,&listItem);
        RvMutexUnlock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
        if (RV_OK == rv)
        {
            userFd = (SeliUserFd*)listItem;
            memset(userFd,0,sizeof(SeliUserFd));
            userFd->callback = pCallBack;
            userFd->ctx      = ctx;
            rv = RvFdConstruct(&userFd->fd, &s, pMidMgr->pLogMgr);
            if (rv == RV_OK)
            {
#if (RV_SELECT_TYPE == RV_SELECT_KQUEUE && RV_SELECT_KQUEUE_GROUPS == RV_YES)
                RvFdSetGroup(&userFd->fd, RV_SELECT_KQUEUE_GROUP_HIGH);
#endif
                rv = RvSelectAdd(pMidMgr->pSelect, &userFd->fd, coreEvents, seliEventsCB);
                if (rv != RV_OK) 
                {
                    RvFdDestruct(&userFd->fd);
                }
            }
            if (rv != RV_OK)
            {
                RvMutexLock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
                RLIST_Remove(pMidMgr->userFDPool,pMidMgr->hUserFds ,(RLIST_ITEM_HANDLE)userFd);
                RvMutexUnlock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
            }
        }
        else
        {
            return rv; /* No more fd's to spare */
        }
    }
    else if (coreFd != NULL)
    {
        userFd = RV_GET_STRUCT(SeliUserFd, fd, coreFd);

        /* We already have it */
        if (((RvInt)events == 0) || (pCallBack == NULL))
        {
            /* We should remove this fd */
            RvSelectRemove(pMidMgr->pSelect, &userFd->fd);
            RvFdDestruct(&userFd->fd);
            RvMutexLock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
            RLIST_Remove(pMidMgr->userFDPool,pMidMgr->hUserFds ,(RLIST_ITEM_HANDLE)userFd);
            RvMutexUnlock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
        }
        else
        {
            /* We should update this fd */
            rv = RvSelectUpdate(pMidMgr->pSelect, &userFd->fd, coreEvents, seliEventsCB);
            if (rv == RV_OK)
            {
                userFd->callback = pCallBack;
                userFd->ctx      = ctx;
            }
        }
    }

    return rv;
}

/************************************************************************************
 * MidSelectRemovePendingCallOns
 * ----------------------------------------------------------------------------------
 * General: removes all pending registrations from select
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 * Output:  
 ***********************************************************************************/
RvStatus RVCALLCONV MidSelectRemovePendingCallOns(IN MidMgr*     pMidMgr)
{
    SeliUserFd* userFd;
    RLIST_ITEM_HANDLE  listItem = NULL;
    
    RvMutexLock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);

    RLIST_GetHead(pMidMgr->userFDPool,pMidMgr->hUserFds,&listItem);
    while (NULL != listItem)
    {
        userFd = (SeliUserFd*)listItem;
        MidSelectCallOn(pMidMgr,(RvInt32)userFd->fd.fd,(RvSipMidSelectEvent)0,NULL,NULL);
        listItem = NULL;
        RLIST_GetHead(pMidMgr->userFDPool,pMidMgr->hUserFds,&listItem);
    }
    RvMutexUnlock(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
    return RV_OK;
}


/*--------------------------------------------- --------------------------*/
/*                           STATIC FUNCTIONS                            */
/*---------------------------- -------------------------------------------*/
static void seliEventsCB(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error)
{
    SeliUserFd* userFd;
    RvSipMidSelectEv cb;
    RvSocket* s;
    RvSipMidSelectEvent seliEvent = (RvSipMidSelectEvent)0;
    void* ctx;

    userFd = RV_GET_STRUCT(SeliUserFd, fd, fd);

    s = RvFdGetSocket(&userFd->fd);

    if (selectEvent & RvSelectRead) 
    {
        seliEvent = RVSIP_MID_SELECT_READ;
    }
    else if (selectEvent & RvSelectWrite) 
    {
        seliEvent = RVSIP_MID_SELECT_WRITE;
    }
    else
    {
        return;
    }
    cb = userFd->callback;
    ctx = userFd->ctx;
    if (cb != NULL)
    {
        cb((int)(*s), seliEvent, error,ctx);
    }
    RV_UNUSED_ARG(selectEngine);
}
