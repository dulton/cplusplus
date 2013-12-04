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
 *                              <_SipSecAgreeTypes.h>
 *
 *
 * The security-agreement layer of the RADVISION SIP toolkit allows you to negotiatie
 * security ageement between a UA and its first hop.
 * This file defines security agreement types, such as owner type, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2005
 *********************************************************************************/


#ifndef _SIP_SEC_AGREE_TYPES_H
#define _SIP_SEC_AGREE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "RvSipSecurityTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
	
/***************************************************************************
 * SipSecAgreeAttachSecObjEv
 * ----------------------------------
 * General: Notifies the owner that its security-agreement reached the active
 *          state, and supplies the security object that was obtained by the
 *          security-agreement.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - The owner of this security-agreement.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
typedef RvStatus (RVCALLCONV* SipSecAgreeAttachSecObjEv)
										(IN  void*                      hOwner,
										 IN  RvSipSecAgreeHandle        hSecAgree,
										 IN  RvSipSecObjHandle          hSecObj);

/***************************************************************************
 * SipSecAgreeDetachSecAgreeEv
 * ----------------------------------
 * General: Notifies the owner of this security agreement object to detach from
 *          this security agreement and its security object. Usually called when 
 *          the security agreement is terminating
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - The owner of this security-agreement.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV* SipSecAgreeDetachSecAgreeEv)
										(IN  void*                      hOwner,
										 IN  RvSipSecAgreeHandle        hSecAgree);

/* SipSecAgreeEvHandlers
 * --------------------------------------
 * Encapsulates the events that a security-agreement reports internally.
 */
typedef struct
{
	SipSecAgreeAttachSecObjEv				pfnSecAgreeAttachSecObjEvHanlder;
    SipSecAgreeDetachSecAgreeEv             pfnSecAgreeDetachSecAgreeEvHanlder;

} SipSecAgreeEvHandlers;


#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _SIP_SEC_AGREE_TYPES_H */



