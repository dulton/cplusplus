/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              <MigMgrObject.h>
 *
 *  This file provides headers for the mid layer of the sip stack. this layer is the 
 *  applicaion way to interact wth core. build application timers, register on file 
 *  descriptor event, etc.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/


#ifndef MID_MGR_OBJECT_H
#define MID_MGR_OBJECT_H

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
    RvBool              bIsDestructing;
    RvLogMgr*           pLogMgr;
    RLIST_POOL_HANDLE   userFDPool;
    RLIST_HANDLE        hUserFds;
    RLIST_POOL_HANDLE   userTimerPool;
    RLIST_HANDLE        hUserTimers;
    RvSelectEngine*     pSelect;
    RvMutex*            pSelectMutex;
    RvMutex*            pTimerMutex;
    RvInt32             maxTimers;
}MidMgr;


/*-----------------------------------------------------------------------*/
/*                              FUNCTIONS  HEADERS                       */
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
                                         IN    RvSipMidCfg* pMidCfg);


/************************************************************************************
 * MidMgrFreeResources
 * ----------------------------------------------------------------------------------
 * General: frees timer lists and select FD lists.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   - Pointer to the middle layer manager
 * Output:  -
 ***********************************************************************************/
void RVCALLCONV MidMgrFreeResources(IN    MidMgr*      pMidMgr);

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
void RVCALLCONV MidMgrRemovePendingRegistrations(IN    MidMgr*      pMidMgr);

#ifdef __cplusplus
}
#endif

#endif /*MID_MGR_OBJECT_H*/
