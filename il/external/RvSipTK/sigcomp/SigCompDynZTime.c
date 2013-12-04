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
 *                              <SigCompDynZTime.c>
 *
 * This file defines dynamic compression utility functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Sasha G                      July 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "SigCompDynZTime.h"
#include "rvtime.h"
#include "rvtimestamp.h"

#ifndef RV_Z_FAKETIME
#define RV_Z_FAKETIME 0
#endif

#if RV_Z_FAKETIME

#if __GNUC__

#warning "You're compiling with faked notion of time for debugging purposes. To change unset RV_Z_FAKETIME"

#else

#pragma message(RvReminder "You're compiling with faked notion of time for debugging purposes. To change unset RV_SIGCOMP_FAKETIME")

#endif


static RvSigCompZTimeSec gsTime = 0;

/*************************************************************************
* SigCompDynZTimeSecSet
* ------------------------------------------------------------------------
* General: Set the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
void SigCompDynZTimeSecSet(RvSigCompZTimeSec time)
{
    if(gsTime < time) {
        gsTime = time;
    }
}

/*************************************************************************
* SigCompDynZTimeSecGet
* ------------------------------------------------------------------------
* General: Retrieve the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RvSigCompZTimeSec SigCompDynZTimeSecGet()
{
    return gsTime;
}


#else /* #if RV_Z_FAKETIME */

/*************************************************************************
* SigCompDynZTimeSecSet
* ------------------------------------------------------------------------
* General: Set the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
void SigCompDynZTimeSecSet(RvSigCompZTimeSec t)
{
    (void)t;
}


/*************************************************************************
* SigCompDynZTimeSecGet
* ------------------------------------------------------------------------
* General: Retrieve the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RvSigCompZTimeSec SigCompDynZTimeSecGet()
{
    return (RvSigCompZTimeSec)RvInt64ToRvUint32(
                RvInt64Div(RvTimestampGet(NULL), RV_TIME64_NSECPERSEC));
}

#endif /* #if RV_Z_FAKETIME */
