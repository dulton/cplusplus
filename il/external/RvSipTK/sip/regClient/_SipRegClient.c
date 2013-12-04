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
 *                              <SipRegClient.c>
 *
 *  The SipRegClient.h file contains the Internal Api functions of the Register-Client.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipRegClient.h"
#include "RegClientObject.h"
#include "_SipTranscClient.h"

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
 * Input:     hRegClient     - Handle to the register
 * Output:    phActiveTransc - Handle to the active transaction
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientGetActiveTransc(
                                    IN  RvSipRegClientHandle   hRegClient,
									OUT RvSipTranscHandle     *phActiveTransc)
{
	RegClient   *pRegClient = (RegClient *)hRegClient;
	RvStatus     rv;
    	
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetActiveTransc(&pRegClient->transcClient, phActiveTransc);
	if (RV_OK != rv)
	{
		RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
					"SipRegClientGetActiveTransc - reg client 0x%p: failed in SipTranscClientGetActiveTransc",
					pRegClient));
	}

	RegClientUnLockAPI(pRegClient);

	return rv;
}

#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

