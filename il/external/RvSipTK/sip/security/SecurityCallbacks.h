/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <SecurityCallbacks.h>
 *
 * The SecurityCallbacks.h file contains wrappers for application callbacks,
 * called by Security Module.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

#ifndef SECURITY_CALLBACKS_H
#define SECURITY_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipSecurityTypes.h"
#include "SecurityObject.h"

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         SECURITY CALLBACK FUNCTIONS                       */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecurityCallbackSecObjStateChangeEv
 * ----------------------------------------------------------------------------
 * General: Calls the Security Object state change callback.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pSecObj - Pointer to the Security Object
 *        eState  - State
 *        eReason - State change reason
 *****************************************************************************/
void SecurityCallbackSecObjStateChangeEv(
                                IN SecObj*                      pSecObj,
                                IN RvSipSecObjState             eState,
                                IN RvSipSecObjStateChangeReason eReason);

/******************************************************************************
 * SecurityCallbackSecObjStatusEv
 * ----------------------------------------------------------------------------
 * General: Calls the Security Object status callback.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pSecObj - Pointer to the Security Object
 *        eStatus - Status, to be provided through the callback
 *        pInfo   - Auxiliary info
 *****************************************************************************/
void SecurityCallbackSecObjStatusEv(
                                IN SecObj*           pSecObj,
                                IN RvSipSecObjStatus eStatus,
                                IN void*             pInfo);

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SECURITY_CALLBACKS_H*/



