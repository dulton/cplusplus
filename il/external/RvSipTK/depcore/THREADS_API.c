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
 *                              <THREADS_API.c>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the Thread module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"
#include "rvlog.h"
#include "rvselect.h"

#if defined(RV_DEPRECATED_CORE)

/*-----------------------------------------------------------------------*/
/*                   STATIC AND GLOBAL PARAMS                            */
/*-----------------------------------------------------------------------*/
/* the log mgr enables the use to set a log mgr to the Wrapper before it was initiated
*/

extern RvLogMgr* g_pLogMgr;

/*-----------------------------------------------------------------------*/
/*                          THREAD FUNCTIONS                             */
/*-----------------------------------------------------------------------*/
/********************************************************************************************
 * RV_THREAD_GetHandle
 * purpose : Returns the handle of the running thread
 * input   : none
 * output  : none
 * return  : A thread handle
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RVAPI RV_THREAD_Handle RVCALLCONV RV_THREAD_GetHandle(void)
{
    RvSelectEngine* pSelect;
    if (RV_OK == RvSelectGetThreadEngine(g_pLogMgr,&pSelect))
        return (RV_THREAD_Handle)pSelect;
    else
        return NULL;
}

/********************************************************************************************
 * RV_THREAD_GetCurrentId
 * purpose : Returns the OS id of the current thread
 * input   : none
 * output  : none
 * return  : A thread handle
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RVAPI void* RVCALLCONV RV_THREAD_GetCurrentId(void)
{
    return (void*)RvThreadCurrentId();
}

#endif /*#if defined(RV_DEPRECATED_CORE)*/

