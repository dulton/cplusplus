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
 *                              <CompartmentCallbacks.h>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofer Goren                    Nov 2007
 *********************************************************************************/

#ifndef COMPARTMENT_CALLBACKS_H
#define COMPARTMENT_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIGCOMP_ON
#include "CompartmentMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* CompartmentCallbackCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr			- Handle to the compartment manager.
*        hCompartment   - The new Compartment handle
*        pbIsApproved   - is the new compartment approved to be created
* Output:(-)
***************************************************************************/
void RVCALLCONV CompartmentCallbackCreatedEv(IN  CompartmentMgr			  *pMgr,
											 IN  RvSipCompartmentHandle    hCompartment,
											 OUT RvBool                   *pbIsApproved);


#endif /* RV_SIGCOMP_ON */

#ifdef __cplusplus
}
#endif


#endif /* COMPARTMENT_CALLBACKS_H */



