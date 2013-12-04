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
 *                          <CallLegHighAvailability_3_0.c>
 *
 *  The following functions are for 3.0 version High Availability use. 
 *  It means that user can get all the nessecary parameters of 
 *  a connected call, (which is not in the middle of a refer or re-invite proccess),
 *  and copy it to another list of calls - of another call manager.
 * 
 * IMPORTANT: This file MUSTN'T changed since in order to support backward 
 *            compatability. If new data has to be stored it should be updated 
 *            ONLY in CallLegHighAvailability.h,c files.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 November 2001
 *********************************************************************************/


#ifndef CALL_LEG_HIGH_AVAILABILITY_3_0_H
#define CALL_LEG_HIGH_AVAILABILITY_3_0_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef  RV_SIP_HIGH_AVAL_3_0
#if !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_HIGHAVAL_ON)


#include "RvSipCallLegTypes.h"
#include "_SipTransportTypes.h"
#include "CallLegObject.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegRestoreCall_3_0
 * ------------------------------------------------------------------------
 * General: Restore all call-leg information into a given call-leg. The call-leg
 *          will assume the connceted state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not on state CONNECTED, or the
 *                                 refer state is not REFER_IDLE.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - Handle to the call-leg.
 *            memBuff     - The buffer that stores all the callLeg's parameters.
 ***************************************************************************/
RvStatus CallLegRestoreCall_3_0(IN CallLeg              *pCallLeg,
                                IN void                 *memBuff,
                                IN RvSipCallLegHARestoreMode eHAmode);

#endif /* #if !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_HIGHAVAL_ON) */
#endif /*#ifdef  RV_SIP_HIGH_AVAL_3_0*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_HIGH_AVAILABILITY_3_0_H*/


