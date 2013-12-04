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
 *                              <SecAgreeStatus.h>
 *
 * This file manages the security-agreement statuses: decides on status occurance,
 * acts upon occuring statuses, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Aug 2007
 *********************************************************************************/

#ifndef SEC_AGREE_STATUS_H
#define SEC_AGREE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
    
/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeStatusConsiderNewStatus
 * ------------------------------------------------------------------------
 * General: Check if a new status occurs and report this status to the 
 *          application.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object.
 *         eProcessing  - The occurring action that might cause the change in
 *                        status
 *         responseCode - The response code of the messgae being processed,
 *                        if relevant.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStatusConsiderNewStatus(IN  SecAgree*           pSecAgree,
												    IN  SecAgreeProcessing  eProcessing,
													IN  RvInt16             responseCode);


#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_STATUS_H*/

