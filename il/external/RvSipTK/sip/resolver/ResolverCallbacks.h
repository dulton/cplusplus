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
 *                              <ResolverCallbacks.h>
 *
 * Resolver layer callback function wrappers
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Feb 2005
 *********************************************************************************/

#ifndef RESOLVER_CALLBACKS_H
#define RESOLVER_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "ResolverMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ResolverCallbacksReportData
 * ------------------------------------------------------------------------
 * General: reports data to upper layer 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv     - pointer to the Resolver
 *            bIsError  - do we have an error in the data
 **************************************************************************/
void RVCALLCONV ResolverCallbacksReportData(IN Resolver*  pRslv,
                                   IN RvBool    bIsError);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RESOLVER_CALLBACKS_H*/

