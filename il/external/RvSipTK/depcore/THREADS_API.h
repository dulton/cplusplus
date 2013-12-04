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
 *                              <THREADS_API.h>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the Thread module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/
#ifndef _THREADS_API_H
#define _THREADS_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RV_DEF.h"

/*-----------------------------------------------------------------------*/
/*                          THREAD FUNCTIONS                             */
/*-----------------------------------------------------------------------*/

#if defined(RV_DEPRECATED_CORE)
/********************************************************************************************
 * RV_THREAD_GetHandle
 * purpose : Returns the handle of the running thread
 * input   : none
 * output  : none
 * return  : A thread handle
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RVAPI RV_THREAD_Handle RVCALLCONV RV_THREAD_GetHandle(void);

/********************************************************************************************
 * RV_THREAD_GetCurrentId
 * purpose : Returns the OS id of the current thread
 * input   : none
 * output  : none
 * return  : A thread handle
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RVAPI void* RVCALLCONV RV_THREAD_GetCurrentId(void);
#endif /*#if defined(RV_DEPRECATED_CORE) */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _THREADS_API_H */


