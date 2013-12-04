/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              <MidSelect.h>
 *
 *  middle layer select functionability
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/


#ifndef MID_SELECT_H
#define MID_SELECT_H

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
/* User defined SELI events on file descriptors */
typedef struct
{
    RvSelectFd          fd; /* Actual core fd to use */
    RvSipMidSelectEv    callback; /* Callback to use */
    void*               ctx; /* User context */
} SeliUserFd;

/*-----------------------------------------------------------------------*/
/*                              FUNCTIONS  HEADERS                       */
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
                                           IN void*                ctx);

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
RvStatus RVCALLCONV MidSelectRemovePendingCallOns(IN MidMgr*     pMidMgr);

#ifdef __cplusplus
}
#endif

#endif /*MID_SELECT_H*/
