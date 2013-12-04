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
 *                              <SigCompCallbacks.c>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  May 2006
 *********************************************************************************/

#ifndef SIGCOMP_CALLBACKS_H
#define SIGCOMP_CALLBACKS_H

#if defined(__cplusplus)
extern "C" {
#endif
	
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"
#include "SigCompMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SigCompCallbackCompressMsgEv
* ------------------------------------------------------------------------
* General: Call the pfnCompressionFunc callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pSigCompMgr      - Pointer to the SigComp manager.
*        pCompartment     - Pointer to the created compartment
*		 pCompressionInfo - Pointer to compression information structure 
*							(within the compartment context)
*        pMsgInfo         - Pointer to the message information structure
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV SigCompCallbackCompressMsgEv(
					IN    SigCompMgr               *pSigCompMgr,
					IN    SigCompCompartment	   *pCompartment,
					IN    RvSigCompCompressionInfo *pCompressionInfo,
					IN    RvSigCompMessageInfo     *pMsgInfo); 

/***************************************************************************
* SigCompCallbackAlgorithmInitializedEv
* ------------------------------------------------------------------------
* General: Call the pfnAlgorithmInitializedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - Handle to the SigComp manager.
*        pAlgorithm  - Pointer to the initialized algorithm data structure 
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackAlgorithmInitializedEv(
								IN SigCompMgr		  *pSigCompMgr,
								IN RvSigCompAlgorithm *pAlgorithm); 

/***************************************************************************
* SigCompCallbackAlgorithmEndedEv
* ------------------------------------------------------------------------
* General: Call the pfnAlgorithmEndedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - Handle to the SigComp manager.
*        pAlgorithm  - Pointer to the initialized algorithm data structure 
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackAlgorithmEndedEv(
								IN SigCompMgr		  *pSigCompMgr,
								IN RvSigCompAlgorithm *pAlgorithm); 

/***************************************************************************
* SigCompCallbackCompartmentCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Handle to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentCreatedEv(
								IN SigCompMgr		    *pSigCompMgr,
								IN SigCompCompartment   *pCompartment); 

/***************************************************************************
* SigCompCallbackCompartmentDestructedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentDestructedEv callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Handle to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentDestructedEv(
								IN SigCompMgr		    *pSigCompMgr,
								IN SigCompCompartment   *pCompartment); 

/***************************************************************************
* SigCompCallbackCompartmentOnPeerMsgEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentOnPeerMsgEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr   - Handle to the SigComp manager.
*        pCompartment  - Pointer to the created compartment
* Output:(-)
***************************************************************************/
void RVCALLCONV SigCompCallbackCompartmentOnPeerMsgEv(
								IN SigCompMgr		        *pSigCompMgr,
								IN SigCompCompartment       *pCompartment,
								IN RvSigCompCompressionInfo *pCompInfo); 

#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_CALLBACKS_H */

