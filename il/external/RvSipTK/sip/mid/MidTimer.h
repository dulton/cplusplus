/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              <MidTimer.h>
 *
 *  middle layer timer functionability
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/


#ifndef MID_TIMER_H
#define MID_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
    
#include "rvselect.h"
#include "rvmutex.h"
    
#include "AdsRlist.h"
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
typedef struct  
{
    SipTimer sipTimer;
    void*    ctx;
    RvSipMidTimerExpEv cb;
    MidMgr* pMgr;
}MidTimer;
/*-----------------------------------------------------------------------*/
/*                              FUNCTIONS  HEADERS                       */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * MidTimerSet
 * ----------------------------------------------------------------------------------
 * General: Sets a new timer
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr     -- pointer to the middle layer manager.
            miliTimeOut -- experation specified in miliseconds
            cb          -- application callback
            ctx         -- context to be called when timer expires
 * Output:  pTimer      -- a newly allocated timer
 ***********************************************************************************/
RvStatus RVCALLCONV MidTimerSet(IN MidMgr*               pMidMgr,
                                IN RvUint32              miliTimeOut,
                                IN RvSipMidTimerExpEv    cb,
                                IN void*                 ctx,
                                OUT MidTimer**           pTimer);

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
                                  IN MidTimer*   pMidTimer);

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
RvStatus RVCALLCONV MidTimerRemovePendingTimers(IN MidMgr*     pMidMgr);

#ifdef __cplusplus
}
#endif

#endif /*MID_TIMER_H*/
