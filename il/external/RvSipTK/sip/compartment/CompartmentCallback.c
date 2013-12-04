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
 *                              <CompartmentCallbacks.c>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofer Goren                   Nov 2007
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/


#include "RV_SIP_DEF.h"
#ifdef RV_SIGCOMP_ON
#include "CompartmentCallback.h"
#include "CompartmentObject.h"
#include "RvSipTransport.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* CompartmentCallbackCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCompartmentCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr			- Handle to the compartment manager.
*        hCompartment   - The new Compartment handle
*        pbIsApproved   - is the new compartment approved to be created
* Output:(-)
***************************************************************************/
void RVCALLCONV CompartmentCallbackCreatedEv(IN  CompartmentMgr			  *pMgr,
											 IN  RvSipCompartmentHandle    hCompartment,
											 OUT RvBool                   *pbIsApproved)
{
    RvInt32							  nestedLock = 0;
    Compartment						 *pCompartment  = (Compartment *)hCompartment;
    RvSipAppCompartmentHandle		  hAppCall   = NULL;
    SipTripleLock					 *pTripleLock;
    RvThreadId						  currThreadId; 
	RvInt32							  threadIdLockCount;
	RvSipTransportAddr				  srcAddr;
	RvSipTransportAddr				  dstAddr;
	RvInt32		                      sigCompIdLen = 0;
	RvSipTransportConnectionHandle	  hConn;
	RvStatus						  rv;
	RvChar                            strAlgoName[RVSIGCOMP_ALGO_NAME];
	RvSipCompartmentCbInfo            compInfo;
    RvSipCompartmentEvHandlers	      *compartmentEvHandlers = &pMgr->compartmentEvHandlers;

    pTripleLock = pCompartment->pTripleLock;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "CompartmentCallbackCreatedEv - compartment mgr notifies the application of a new compartment 0x%p",
        pCompartment));

	memset(&compInfo,0,sizeof(compInfo));
    if(compartmentEvHandlers != NULL && (*compartmentEvHandlers).pfnCompartmentCreatedEvHandler != NULL)
    {        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CompartmentCallbackCreatedEv - compartment 0x%p, Before callback",
                  pCompartment));

		/* prepare sigcomp-id - copy to temporary page */
		if (NULL_PAGE != pCompartment->hPageForUrn)
		{			
			sigCompIdLen = RPOOL_Strlen(pCompartment->pMgr->hCompartmentPool, pCompartment->hPageForUrn,
				pCompartment->offsetInPage);
			rv = RPOOL_GetPage(pMgr->hCompartmentPool, sigCompIdLen, &compInfo.hSigCompIdPage);
			if (rv != RV_OK)
			{
				RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
					"CompartmentCallbackCreatedEv - Failed to get new page (rv=%d)", rv));
				*pbIsApproved = RV_FALSE;
				return;
			}
			compInfo.sigCompIdOffset = 0;
			RPOOL_CopyFrom(pMgr->hCompartmentPool, compInfo.hSigCompIdPage, compInfo.sigCompIdOffset, 
				pMgr->hCompartmentPool, pCompartment->hPageForUrn, pCompartment->offsetInPage, sigCompIdLen);
	  
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT) 
			{
				char urn[128] = {"\0"};
				if(RPOOL_CopyToExternal (pMgr->hCompartmentPool,compInfo.hSigCompIdPage,
					compInfo.sigCompIdOffset, urn, sigCompIdLen) == RV_OK)
				{
					urn[sigCompIdLen] = 0;
					strcpy(compInfo.urn,urn);
				}
			}
#endif
/*SPIRENT_END*/
	 
		}
		/* prepare addresses - convert from SipTransportAddr to RvSipTransportAddr */
		RvSipTransportConvertRvAddress2TransportAddr (&pCompartment->srcAddr.addr, &srcAddr);
		srcAddr.eTransportType = pCompartment->srcAddr.eTransport;
		compInfo.pSrcAddr = &srcAddr;
		RvSipTransportConvertRvAddress2TransportAddr (&pCompartment->dstAddr.addr, &dstAddr);
		dstAddr.eTransportType = pCompartment->dstAddr.eTransport;
		compInfo.pDstAddr = &dstAddr;

		/* -- prepare connection handle -- */
		/* Note - for Initial outgoing requests on connection-oriented transport, */
		/* The hConn handle will be NULL, as it get initialized later when the message */
		/* is just about to be sent */
		hConn = pCompartment->hConnection;

		compInfo.hMsg = pCompartment->hMsg;
		compInfo.bIsIncoming = pCompartment->bIsSrc;

		strcpy (strAlgoName, pCompartment->strAlgoName);
		pCompartment->bIsInCreatedEv = RV_TRUE;

        SipCommonUnlockBeforeCallback(pMgr->pLogMgr,pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*compartmentEvHandlers).pfnCompartmentCreatedEvHandler(
                                        (RvSipCompartmentHandle)pCompartment, &compInfo,
										strAlgoName, &hAppCall, pbIsApproved);

        SipCommonLockAfterCallback(pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

		pCompartment->bIsInCreatedEv  = RV_FALSE;
        pCompartment->hAppCompartment = hAppCall;

		if (strcmp(strAlgoName, pCompartment->strAlgoName) != 0)
		{
			strcpy (pCompartment->strAlgoName, strAlgoName);
			RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
				"CompartmentCallbackCreatedEv - Compression Algorithm changed: %s", pCompartment->strAlgoName));
		}
		
		/* free temporary page, if allocated */
		if (NULL_PAGE != compInfo.hSigCompIdPage)
		{			
			RPOOL_FreePage(pMgr->hCompartmentPool, compInfo.hSigCompIdPage);
		}

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CompartmentCallbackCreatedEv - compartment 0x%p, After callback, phAppCompartment 0x%p",
            pCompartment, hAppCall));        
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CompartmentCallbackCreatedEv - compartment 0x%p, Application did not registered to callback, default phAppCall = 0x%p",
              pCompartment, hAppCall));
		/* default behavior is to APPROVE the creation, so we can be backward compatible with applications */
		/* written on top of old sigcomp (pre-ver 5.0 GA3). */
		*pbIsApproved = RV_TRUE;
    }
}

#endif /* RV_SIGCOMP_ON */
