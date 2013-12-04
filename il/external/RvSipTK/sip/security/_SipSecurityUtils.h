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
 *                              <_SipSecurityUtils.h>
 *
 * The _SipSecurityUtils.h file defines the utility functions serving user of
 * Security Module
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

#ifndef SIP_SECURITY_UTILS_H
#define SIP_SECURITY_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipSecurityTypes.h"

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            FUNCTIONS HEADERS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SipSecUtilConvertSecObjState2Str
 * ----------------------------------------------------------------------------
 * General: returns string, representing name of the Security Object state.
 *
 * Return Value: state name
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - state to be converted
 *****************************************************************************/
RvChar* SipSecUtilConvertSecObjState2Str(IN RvSipSecObjState eState);

/******************************************************************************
 * SipSecUtilConvertSecObjStateChangeRsn2Str
 * ----------------------------------------------------------------------------
 * General: returns string, representing name of the Security Object state
 *          change reason.
 *
 * Return Value: reason name
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - reason to be converted
 *****************************************************************************/
RvChar* SipSecUtilConvertSecObjStateChangeRsn2Str(
                                    IN RvSipSecObjStateChangeReason eReason);

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIP_SECURITY_UTILS_H*/

