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
 *                              <SipPubClient.c>
 *
 *  The SipPubClient.h file contains the Internal Api functions of the Publish-Client.
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
#include "_SipPubClient.h"
#include "PubClientObject.h"
#include "_SipTranscClient.h"

/*-----------------------------------------------------------------------*/
/*                          INTERNAL API FUNCTIONS                       */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipPubClientGetActiveTransc
 * ------------------------------------------------------------------------
 * General: Gets the active transaction of this pub-client
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - Handle to the pub-client
 * Output:    phActiveTransc - Handle to the active transaction
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientGetActiveTransc(
                                    IN  RvSipPubClientHandle   hPubClient,
									OUT RvSipTranscHandle     *phActiveTransc)
{
	PubClient   *pPubClient = (PubClient *)hPubClient;
	RvStatus     rv;
    	
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetActiveTransc(&pPubClient->transcClient, phActiveTransc);
	if (RV_OK != rv)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
					"SipPubClientGetActiveTransc - pub client 0x%p: failed in SipTranscClientGetActiveTransc",
					pPubClient));
	}

	PubClientUnLockAPI(pPubClient);

	return rv;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */
