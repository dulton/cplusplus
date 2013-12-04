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
 *                              <CallLegHighAvailability.c>
 *
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/

#ifndef CALL_LEG_HIGH_AVAILABILITY_H
#define CALL_LEG_HIGH_AVAILABILITY_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_HIGHAVAL_ON

#include "RvSipCallLegTypes.h"
#include "_SipTransportTypes.h"
#include "CallLegObject.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegGetCallStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of 
 *          connected call. 
 *          (The size of buffer should be supply in 
 *          RvSipCallLegStoreConnectedCall()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR     - hCallleg or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - Handle to the call-leg.
 *          bStandAlone  - Indicates if the CallLeg is independent.
 *                         This parameter should be RV_FALSE if
 *                         a subscription object retrieves its CallLeg 
 *                         storage size, in order to avoid infinity recursion.
 * Output:  pLen     - size of buffer. will be UNDEFINED (-1) in case of failure.
 ***************************************************************************/
RvStatus CallLegGetCallStorageSize(IN  RvSipCallLegHandle hCallLeg,
                                   IN  RvBool             bStandAlone,
                                   OUT RvInt32           *pLen);

/***************************************************************************
 * CallLegStoreCall
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call leg.
 *          In order to store call-leg information the call leg must be in the
 *          connceted state.
 * Return Value:RV_ERROR_INVALID_HANDLE    - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR       - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION    - If the state is not on state CONNECTED, or the
 *                                    refer state is not REFER_IDLE.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES   - Resource problems.
 *              RV_OK          - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCallLeg    - Handle to the call-leg .
 *        maxBuffLen  - The length of the given buffer.
 *        bStandAlone - Indicates if the CallLeg is independent .
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg 
 *                      storage size, in order to avoid infinity recursion.
 * Output: memBuff    - The buffer that will store the CallLeg's parameters.
 *         pStoredLen - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus CallLegStoreCall(IN  RvSipCallLegHandle hCallLeg,
                          IN  RvBool             bStandAlone,
                          IN  RvUint32           maxBuffLen,
                          OUT void              *memBuff,
                          OUT RvInt32           *pStoredLen);

/***************************************************************************
 * CallLegRestoreCall
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
 * Input: memBuff     - The buffer that stores all the callLeg's parameters.
 *        buffLen     - The buffer size
 *        bStandAlone - Indicates if the CallLeg is independent.
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg 
 *                      storage size, in order to avoid infinity recursion.
 * InOut: pCallLeg    - Pointer to the call-leg.
 ***************************************************************************/
RvStatus CallLegRestoreCall(IN    void     *memBuff,
                            IN    RvUint32  buffLen,
                            IN    RvBool    bStandAlone,
                            IN    RvSipCallLegHARestoreMode eHAmode,
                            OUT   CallLeg  *pCallLeg);

#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 
#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_HIGH_AVAILABILITY_H*/


