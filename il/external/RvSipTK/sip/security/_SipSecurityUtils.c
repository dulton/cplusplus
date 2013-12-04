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
 *                              <_SipSecurityUtils.c>
 *
 * The _SipSecurityUtils.c file implements the utility functions serving user of
 * Security Module
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecurityTypes.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTIONS PROTOTYPES                      */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                              */
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
RvChar* SipSecUtilConvertSecObjState2Str(IN RvSipSecObjState eState)
{
    switch(eState)
    {
    case RVSIP_SECOBJ_STATE_UNDEFINED:      return "UNDEFINED";
    case RVSIP_SECOBJ_STATE_IDLE:           return "IDLE";
    case RVSIP_SECOBJ_STATE_INITIATED:      return "INITIATED";
    case RVSIP_SECOBJ_STATE_ACTIVE:         return "ACTIVE";
    case RVSIP_SECOBJ_STATE_CLOSING:        return "CLOSING";
    case RVSIP_SECOBJ_STATE_CLOSED:         return "CLOSED";
    case RVSIP_SECOBJ_STATE_TERMINATED:     return "TERMINATED";
    }
    return "";
}

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
                                    IN RvSipSecObjStateChangeReason eReason)
{
    switch(eReason)
    {
    case RVSIP_SECOBJ_REASON_UNDEFINED:     return "UNDEFINED";
    }
    return "";
}

#endif /*#ifdef RV_SIP_IMS_ON*/

