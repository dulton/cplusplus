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
 *                              <SipRegClient.h>
 *
 *  The SipRegClient.h file contains the Internal Api functions of the Register-Client.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef SIP_REG_CLIENT_H
#define SIP_REG_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipRegClientTypes.h"
#include "RvSipTransactionTypes.h"
#include "_SipCommonTypes.h"

/*-----------------------------------------------------------------------*/
/*                          INTERNAL API FUNCTIONS                       */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipRegClientGetActiveTransc
 * ------------------------------------------------------------------------
 * General: Gets the active transaction of this register-client
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient     - Handle to the register-client
 * Output:    phActiveTransc - Handle to the active transaction
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientGetActiveTransc(
                                    IN  RvSipRegClientHandle   hRegClient,
									OUT RvSipTranscHandle     *phActiveTransc);

#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_REG_CLIENT_H */

