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
 *                              <SELI_API.c>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the SELI module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"
#include "SELI_API.h"
#include "depcoreInternals.h"
#include "rvmemory.h"
#include "rvselect.h"
#include "rvstdio.h"
#if defined(RV_DEPRECATED_CORE)

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                   STATIC AND GLOBAL PARAMS                            */
/*-----------------------------------------------------------------------*/
/* the log mgr enables the use to set a log mgr to the Wrapper before it was initiated
*/
RvLogMgr* g_pLogMgr    = NULL;


/* the g_pSeliMgr hold data that is relevent to all instances that 
   make use of the select engine.*/
static WSeliMgr* g_pSeliMgr   = NULL;

/* enabled the user to set a user data size befor the wrapper was initiated
*/
static RvUint32  g_fdUserSize = sizeof(void*);

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
typedef enum
{
    SELI_INIT_PHASE_ALL,
    SELI_INIT_PHASE_HRA,
    SELI_INIT_PHASE_MUTEXES,
    SELI_INIT_PHASE_MEM_ALLOCATED
}SeliInitPhase;

typedef struct
{
    RvSelectFd fd;
    RV_SELI_EVENT_HANDLER pcb;
}WUserFd;

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void seli_unallocate(SeliInitPhase ePhase);

static RvStatus RVCALLCONV seli_allocate(void);

static void UserFdSelectCB(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error);

/*-----------------------------------------------------------------------*/
/*                            SELI FUNCTIONS                             */
/*-----------------------------------------------------------------------*/
/********************************************************************************************
 * SELI_SetLog
 * purpose : Set a log handle to the select wrapper.
 *           
 * input   : logHandle - log handle to set. (the log can be retrieved from one of the 
 *           toolkits
 * output  : none
 * return  : RV_Success          - In case the routine has succeded.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_SetLog (IN RV_LOG_Handle logHandle)
{
    g_pLogMgr = (RvLogMgr*)logHandle;
    return RV_OK;
}

/********************************************************************************************
 * SELI_SetUserDataSize
 * purpose : Set the size of user data to associate with each file descriptor.
 *           
 * input   : hSeli      - Handle to SELI instance
 *           size       - Size of user data per file descriptor in bytes
 * output  : none
 * return  : RV_Success          - In case the routine has succeded.
 *           RV_OutOfResources   - Allocation problem
 *           RV_Failure          - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_SetUserDataSize (IN RV_SELI_Handle hSeli,
                                                 IN RvUint32       size)
{
	RV_UNUSED_ARG(hSeli)
    g_fdUserSize = size;
    return RV_OK;
}


/********************************************************************************************
 * SELI_CallOnThread
 * purpose : Register an event on a file descriptor in a specific thread.
 *           To unregister events, this function should be called with event=RV_SELI_EvNone
 *           Registered sockets will be re-registered by this function.
 * input   : hSeli          - SELI application handle to use
 *           fdHandle       - File descriptor handle to register on
 *           event          - Event to register for file descriptor
 *           eventHandler   - Callback function to call on event
 *           threadId       - Thread where event will be generated
 * output  : userData       - Pointer to the user data for this file descriptor
 *                            If set to NULL, it is ignored
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_CallOnThread(IN  RV_SELI_Handle         hSeli,
			                                 IN  RV_SELI_FdHandle       fdHandle,
				                             IN  RV_SELI_Events         event,
						                     IN  RV_SELI_EVENT_HANDLER  eventHandler,
							                 IN  RV_THREAD_Handle       threadId,
									         OUT RV_SELI_USER_DATA*     userData)
{
	RV_UNUSED_ARG(hSeli)
    return SELI_CallOn((RV_SELI_Handle)threadId,fdHandle,event,eventHandler,userData);
}

/********************************************************************************************
 * SELI_CallOn
 * purpose : Register an event on a file descriptor.
 *           To unregister events, this function should be called with event=RV_SELI_EvNone
 *           Registered sockets will be re-registered by this function.
 * input   : hSeli          - SELI application handle to use
 *           fdHandle       - File descriptor handle to register on
 *           event          - Event to register for file descriptor
 *           eventHandler   - Callback function to call on event
 * output  : userData       - Pointer to the user data for this file descriptor
 *                            If set to NULL, it is ignored
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_CallOn (IN  RV_SELI_Handle         hSeli,
			                            IN  RV_SELI_FdHandle       fdHandle,
				                        IN  RV_SELI_Events         event,
						                IN  RV_SELI_EVENT_HANDLER  eventHandler,
							            OUT RV_SELI_USER_DATA*     userData)
{
    RvSelectEngine* pSelect = (RvSelectEngine*)hSeli;
    RvStatus        rv      = RV_OK;
    RvSocket        s;
    RvSelectEvents  coreEvents      = (RvUint16)0;
    RvSelectFd*     coreFd          = NULL;
    WUserFd*        userFd          = NULL;

    if (NULL == pSelect)
    {
        return RV_ERROR_NULLPTR;
    }
    if (0 > (RvInt32)fdHandle)
    {
        return RV_ERROR_BADPARAM;
    }

    /* making sure we are allocated */
    rv = seli_allocate();
    if (RV_OK != rv)
    {
        return rv;
    }
    s = (RvSocket)fdHandle;

    /* Convert the events to the core's events */
    if (event & RV_SELI_EvRead)
    {
        coreEvents |= RvSelectRead;
    }
    if (event & RV_SELI_EvWrite)
    {
        coreEvents |= RvSelectWrite;
    }
    
    RvMutexLock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
    /* Look for this fd if we're currently waiting for events on it */
    coreFd = RvSelectFindFd(pSelect, &s);
    if ((coreFd == NULL) && ((RvInt)event != 0) && (eventHandler != NULL))
    {
        /* This is a new fd we're waiting on - add it */

        /* Allocate an FD for the user */
        if (NULL == g_pSeliMgr->hSelectFds)
        {
            RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
            return RV_ERROR_ILLEGAL_ACTION;
        }
        rv = RA_Alloc(g_pSeliMgr->hSelectFds, (RA_Element*)&userFd);
        if (RV_OK == rv)
        {
            memset(userFd,0,sizeof(userFd));
            userFd->pcb = eventHandler;
            
            rv = RvFdConstruct(&userFd->fd, &s, g_pLogMgr);
            if (rv == RV_OK)
            {
                rv = RvSelectAdd(pSelect, &userFd->fd, coreEvents, UserFdSelectCB);
                if (rv != RV_OK)
                {
                    RvFdDestruct(&userFd->fd);
                }
                if (userData)
                {
                    *userData = (RV_SELI_USER_DATA)(userFd + sizeof(WUserFd));
                }
            }
            if (rv != RV_OK)
            {
                RA_DeAlloc(g_pSeliMgr->hSelectFds, (RA_Element)userFd);
                RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
                return RvErrorGetCode(rv);
            }
        }
        else
        {
            RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
            return rv; /* No more fd's to spare */
        }
    }
    else if (coreFd != NULL)
    {
        userFd = RV_GET_STRUCT(WUserFd, fd, coreFd);

        /* We already have it */
        if (((RvInt)event == 0) || (eventHandler == NULL))
        {
            /* We should remove this fd */
            RvSelectRemove(pSelect, &userFd->fd);
            RvFdDestruct(&userFd->fd);
            RA_DeAlloc(g_pSeliMgr->hSelectFds, (RA_Element)userFd);
        }
        else
        {
            /* We should update this fd */
            rv = RvSelectUpdate(pSelect, &userFd->fd, coreEvents, UserFdSelectCB);
            if (rv == RV_OK)
            {
                userFd->pcb = eventHandler;
                if (userData)
                {
                    *userData = (RV_SELI_USER_DATA)(userFd + sizeof(WUserFd));
                }
            }
        }
    }
    RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
    return RV_OK;
}

/********************************************************************************************
 * SELI_GetUserData
 * purpose : Returns the user's data pointer for the specific file descriptor
 * input   : fdHandle       - File descriptor handle to use
 * output  : none
 * return  : User data pointer
 *           NULL on failure or when the file descriptor is not used
 ********************************************************************************************/
RVAPI RV_SELI_USER_DATA RVCALLCONV SELI_GetUserData (IN RV_SELI_FdHandle      fdHandle)
{
    RvSocket        s       = (RvSocket)fdHandle;
    RvStatus        rv      = RV_OK;
    WUserFd*        pFd     = NULL;         
    RvInt32         loc     = 0;
	  RvInt32 tempLoc = 0;
    rv = seli_allocate();
    if (RV_OK != rv)
    {
        return NULL;
    }
    RvMutexLock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);

    rv = RA_GetFirstUsedElement(g_pSeliMgr->hSelectFds,loc,(RA_Element*)&pFd,&loc);
    {
        if (RV_OK != rv || NULL == pFd)
        {
            RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
            return NULL;
        }
    }
    for (;;)
    {
        if (pFd->fd.fd == s)
        {
            break;
        }
		tempLoc = ++loc;
        rv = RA_GetFirstUsedElement(g_pSeliMgr->hSelectFds,tempLoc,(RA_Element*)&pFd,&loc);
        if (RV_OK != rv || NULL == pFd)
        {
            RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
            return NULL;
        }
    }
    RvMutexUnlock(&g_pSeliMgr->lock,g_pSeliMgr->pLogMgr);
    return (RV_SELI_USER_DATA)(pFd + sizeof(WUserFd));
}

/********************************************************************************************
 * SELI_Select
 * purpose : Checks for events on all registered file descriptors and execute any callback
 *           functions necessary.
 *           This function also calls any timeout callbacks pending in the TIMER.
 *           SELI_Select can be called only from the thread that is the thread that initiated 
 *           the select engine
 * input   : none
 * output  : none
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_Select(void)
{
   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

    RvSelectEngine* selectEngine=NULL;
    RvStatus crv=0;

    crv = RvSelectGetThreadEngine(NULL, &selectEngine);
    if (crv != RV_OK)
    {
        return crv;
    }
    if (selectEngine == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    crv = RvSelectWaitAndBlock(selectEngine, 0);

    return crv;

#else
    RvSelectEngine *pEngine;
    RvSelectGetThreadEngine(NULL,&pEngine);
    if (RV_OK == RvSelectWaitAndBlock(pEngine,RV_UINT64_MAX))
        return RV_OK;
    else
        return RV_ERROR_UNKNOWN;
#endif
   //SPIRENT_END
}

/********************************************************************************************
 * SELI_End
 * purpose : Ends the seli module
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_End(void)
{
    seli_unallocate(SELI_INIT_PHASE_ALL);
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                            STATIC FUNCTIONS                           */
/*-----------------------------------------------------------------------*/
/*************************************************************************
 * seli_unallocate
 * purpose : destructs parts of the seli, sepending on the construction 
 *           phase, acumelating the releasing phases
 * input   : logHandle  - Handle to a log instance
 * output  : none
 * return  : -.
 *************************************************************************/
static void seli_unallocate(SeliInitPhase ePhase)
{
    switch(ePhase)
    {
    case SELI_INIT_PHASE_ALL:
        g_pSeliMgr->initCalls--;
    case SELI_INIT_PHASE_HRA:
        if (NULL != g_pSeliMgr->hSelectFds)
        {
            RA_Destruct(g_pSeliMgr->hSelectFds);
        }
    case SELI_INIT_PHASE_MUTEXES:
        RvMutexDestruct(&g_pSeliMgr->lock,g_pLogMgr);
    case SELI_INIT_PHASE_MEM_ALLOCATED:
        if (NULL != g_pSeliMgr)
        {
            if (0 == g_pSeliMgr->initCalls)
            {
                RvMemoryFree(g_pSeliMgr,g_pLogMgr);
                g_pSeliMgr = NULL;
                RvCBaseEnd();
            }
        }
    default:
        break;
    }
}

/*************************************************************************
 * seli_allocate
 * purpose : Allocate an instance of SELI interface.
 *           Each application that want to use SELI Wrapper, shuold call 
 *           this function once
 *        *) on first call will:
 *           1. Initiate core services
 *           2. Allocate the seli manager and init params.
 *           3. initiate the log 
 * input   : -
 * output  : none
 * return  : RvStatus
 *************************************************************************/
static RvStatus RVCALLCONV seli_allocate(void)
{
    RvStatus    rv  = RV_OK;
    

    /* allocating the manager of the select module */
    if (NULL == g_pSeliMgr)
    {
        if (RV_OK != RvCBaseInit())
        {
            return RV_ERROR_UNKNOWN;
        }
        rv = RvMemoryAlloc(NULL,sizeof(WSeliMgr),g_pLogMgr,(void**)&g_pSeliMgr);
        if (RV_OK != rv)
        {
            return RvErrorGetCode(rv);
        }
        memset(g_pSeliMgr,0,sizeof(WSeliMgr));
        
        /* 2. create locks */
        rv = RvMutexConstruct(g_pLogMgr,&g_pSeliMgr->lock);
        if (RV_OK != rv) 
        {
            seli_unallocate(SELI_INIT_PHASE_MEM_ALLOCATED);
            return RvErrorGetCode(rv);
        }
        
        /* 3. create an RA from the file descriptors */
        g_pSeliMgr->hSelectFds = RA_Construct(sizeof(WUserFd)+g_fdUserSize,
                                            WSELI_MAX_USER_FD,
                                            NULL,
                                            g_pLogMgr,
                                            "WSELI");
        if (NULL == g_pSeliMgr->hSelectFds)
        {
            seli_unallocate(SELI_INIT_PHASE_MUTEXES);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* 4. register the log */
        if (NULL != g_pLogMgr)
        {
            rv = RvLogSourceConstruct(g_pLogMgr,&g_pSeliMgr->logSrcMem,"WSELI","Seli Wrappers");
            if (RV_OK != rv)
            {
                seli_unallocate(SELI_INIT_PHASE_HRA);
                return RvErrorGetCode(rv);
            }
            g_pSeliMgr->pLogSrc = &g_pSeliMgr->logSrcMem;
        }
        RvLogDebug(g_pSeliMgr->pLogSrc,(g_pSeliMgr->pLogSrc,
                  "seli_allocate - Seli wrapper manager initiated succesfully (0x%x)", g_pSeliMgr));
    }
    g_pSeliMgr->initCalls++;
    return RV_OK;
}

/***************************************************************************
* UserFdSelectCB
* ------------------------------------------------------------------------
* General: converts the core callback to a callback the application can understand
* Return Value: (-)
* ------------------------------------------------------------------------
 * Input:   selectEngine - select engine which we registered on
 *          fd      - file descriptor coresponding with registered socket
 *          event   - The type of the event
 *          error   - indicates whether an error occured in the li level
 * Output:  (-)
***************************************************************************/
static void UserFdSelectCB(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error)
{
    WUserFd*                    userFd;
    RvSocket*                   s;
    RV_SELI_Events              eventToUser = (RV_SELI_Events)0;
    RV_SELI_Events              proccesed = (RV_SELI_Events)0;
    void*                       ctx = NULL;
    RV_SELI_EVENT_HANDLER       cb;
    
    RV_UNUSED_ARG(selectEngine);
    userFd = RV_GET_STRUCT(WUserFd, fd, fd);

    s = RvFdGetSocket(&userFd->fd);

    /* For determinig the "processed events" - each event will contain the events that 
       might have been called before it and not the event that were actually called 
       before it. this is done to maintain backwards compatibility */
    if (selectEvent & RvSelectRead)
    {
        eventToUser |= RV_SELI_EvRead;
    }
    else if (selectEvent & RvSelectWrite)
    {
        eventToUser |= RV_SELI_EvWrite;
        proccesed = RV_SELI_EvRead;
    }
    else
    {
        return;
    }
    
    ctx = userFd+sizeof(WUserFd);
    cb = userFd->pcb;

    if (cb != NULL)
    {
        cb((int)(*s), eventToUser,proccesed,ctx, error);
    }    
}

#endif /*defined(RV_DEPRECATED_CORE)*/

    
