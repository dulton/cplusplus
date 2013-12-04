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
 *                              <depcoreInternals.h>
 *
 *  file holds internal structures needed for wrappers
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/
#ifndef _DEPCORE_INTERNALSI_H
#define _DEPCORE_INTERNALSI_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"
#include "AdsRa.h"
#include "rvmutex.h"
#include "rvselect.h"
#include "rvccore.h"
#include "TIMER_API.h"

#if defined(RV_DEPRECATED_CORE)
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* WSeliMgr
 * ----------------------------------------------------------------------
 * The select wrapper manager. Holds configuration parameters and data relevant 
 * to all instances of select
 * initCalls        -- Number of times SELI_Init was called for the instance
 * hSelectFds       -- The select file descritor array.
 * lock             -- A lock for the seli module use
 */
typedef struct {
    RvUint          initCalls;
    HRA             hSelectFds;
    RvMutex         lock;
    RvLogMgr*       pLogMgr;
    RvLogSource     logSrcMem;
    RvLogSource*    pLogSrc;
} WSeliMgr;

/* WTimerMgr
 * ----------------------------------------------------------------------
 * The timer wrapper manager. Holds configuration parameters and data relevant 
 * to all instances of timer
 * initCalls        -- Number of times SELI_Init was called for the instance
 * lock             -- A lock for the seli module use
 */
typedef struct {
    RvUint          initCalls;
    RvMutex*        plock;
    RvLogSource     logSrc;
    RV_TIMER_Resource wtmgrres;
} WTimerMgr;

/* WTimerInstance
 * ----------------------------------------------------------------------
 * The timer wrapper for one application. data for each timer instance 
 * that an application may use
 */
typedef struct {
    HRA             timerNodes;
    RvMutex*        plock;
    RvSelectEngine* pSelect;
    RV_Resource     wtRes;
} WTimerInstance;

/* WTimerCell
 * ----------------------------------------------------------------------
 * The timer wrapper for one application. data for each timer instance 
 * that an application may use
 */
typedef struct {
    RvTimer         timer;
    RvBool          bIsOn;
    void*           ctx;
    WTimerInstance* pWt;
    RV_TIMER_EVENTHANDLER handler;
} WTimerCell;

/*-----------------------------------------------------------------------*/
/*                            SELI FUNCTIONS                             */
/*-----------------------------------------------------------------------*/

#endif /*defined(RV_DEPRECATED_CORE)*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _DEPCORE_INTERNALSI_H */


