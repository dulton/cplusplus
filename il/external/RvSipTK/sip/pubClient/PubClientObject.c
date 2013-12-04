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
 *                              <PubClientObject.c>
 *
 * This file defines the PubClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                  Sep 2006
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipCommonConversions.h"
#include "_SipPubClientMgr.h"
#include "PubClientMgrObject.h"
#include "PubClientObject.h"
#include "RvSipPubClient.h"
#include "PubClientObject.h"
#include "PubClientCallbacks.h"

#include "_SipTranscClient.h"

#include "RvSipOtherHeader.h"
#include "RvSipEventHeader.h"
#include "AdsRlist.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticator.h"
#include "_SipTransport.h"
#include "_SipEventHeader.h"

#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctp.h"
#endif
#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeMgr.h"
#include "_SipSecuritySecObj.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define PUB_CLIENT_STATE_TERMINATED_STR "Terminated"
#define PUB_CLIENT_OBJECT_NAME_STR		"Pub-Client"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar *RVCALLCONV PubClientReasonIntoString(
                                   IN RvSipPubClientStateChangeReason eReason);
#endif /* ##if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


static RvStatus PubClientCopyEventHeader (
										  IN  PubClient				*pPubClient,
										  IN  RvSipEventHeaderHandle hEventHeader);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * PubClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new publish-client object. The publish-client
 *          asumes the idle state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   PubClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - pointer to the publish-clients manager.
 *          pPubClient - pointer to the new publish-client object.
 *          hAppPubClient - The API's handle to the publish-client.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientInitiate(
                                  IN PubClientMgr           *pPubClientMgr,
                                  IN PubClient              *pPubClient,
                                  IN RvSipAppPubClientHandle hAppPubClient)
{
    RvStatus retStatus;
	
	pPubClient->bWasPublished  = RV_FALSE;
    pPubClient->eState          = RVSIP_PUB_CLIENT_STATE_IDLE;
    pPubClient->hAppPubClient   = hAppPubClient;
    pPubClient->pubClientEvHandlers = &(pPubClientMgr->pubClientEvHandlers);
	pPubClient->hSipIfMatchHeader = NULL;
	pPubClient->hEventHeader	  = NULL;
	
	pPubClient->retryAfter		  = 0;
	
	pPubClient->bRemoveInProgress  = RV_FALSE;

	
	/* Get a new memory page for the publish-client object */
    retStatus = RPOOL_GetPage(pPubClientMgr->hGeneralMemPool, 0,
                              &pPubClient->hPage);
	if(retStatus != RV_OK)
    {
        return retStatus;
    }

	pPubClient->hSipIfMatchHeader = NULL;
	pPubClient->hSipIfMatchPage = NULL;

	retStatus = SipTranscClientInitiate(pPubClientMgr->hTranscClientMgr, 
										(SipTranscClientOwnerHandle)pPubClient, 
										RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT, 
										&pPubClient->transcClient);
	
    return retStatus;
}

/***************************************************************************
 * PubClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a publish-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - pointer to the publish-clients manager.
 *          pPubClient - pointer to the publish-client object.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientDestruct(IN PubClient *pPubClient)
{
    PubClientMgr       *pPubClientMgr;
    RvStatus          rv = RV_OK;
    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "PubClientDestruct - Publish Client 0x%p: was destructed",
              pPubClient));

	rv = SipTranscClientDestruct(&pPubClient->transcClient);
	

    /* Remove the publish-client object from the list of publish-clients in
       the publish-clients manager */
    pPubClientMgr = pPubClient->pMgr;
    PubClientMgrLock(pPubClientMgr);
    /* Free the memory page */
    RPOOL_FreePage(pPubClient->pMgr->hGeneralMemPool, pPubClient->hPage);
	if (pPubClient->hSipIfMatchPage != NULL)
	{
		RPOOL_FreePage(pPubClient->pMgr->hGeneralMemPool, pPubClient->hSipIfMatchPage);
		pPubClient->hSipIfMatchPage = NULL;
		pPubClient->hSipIfMatchHeader = NULL;
	}
	

    RLIST_Remove(pPubClient->pMgr->hPubClientListPool,
                 pPubClient->pMgr->hPubClientList,
                 (RLIST_ITEM_HANDLE)pPubClient);
    PubClientMgrUnLock(pPubClientMgr);
    return RV_OK;
}

/***************************************************************************
 * PubClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the publish-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the publish-client.
 *        eReason - The reason for the state change.
 *        pPubClient - The publish-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV PubClientChangeState(
                            IN  RvSipPubClientState             eNewState,
                            IN  RvSipPubClientStateChangeReason eReason,
                            IN  PubClient                       *pPubClient)
{
    if(eNewState == RVSIP_PUB_CLIENT_STATE_TERMINATED)
    {

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientChangeState - Publish-client 0x%p state changed to Terminated, (reason = %s)",
                   pPubClient,
                   PubClientReasonIntoString(eReason)));
    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                 "PubClientChangeState - Publish-client 0x%p state changed: %s->%s, (reason = %s)",
                   pPubClient, PubClientStateIntoString(pPubClient->eState),
                   PubClientStateIntoString(eNewState),
                  PubClientReasonIntoString(eReason)));
    }
    /* Change the publish-client's state */
    pPubClient->eState = eNewState;

    if(pPubClient->eState == RVSIP_PUB_CLIENT_STATE_PUBLISHED)
    {
		pPubClient->bWasPublished = RV_TRUE;
    }

    /* Call the event state changed that was set to the publish-clients
       manager */
    PubClientCallbackStateChangeEv(eNewState, eReason, pPubClient);
}



#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * PubClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to API schema using the transcClient layer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
RvStatus RVCALLCONV PubClientLockAPI(IN  PubClient *pPubClient)
{
	if (pPubClient == NULL)
	{
		return RV_ERROR_NULLPTR;
	}
	
	return SipTranscClientLockAPI(&pPubClient->transcClient);
}


/************************************************************************************
 * PubClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
void RVCALLCONV PubClientUnLockAPI(IN  PubClient *pPubClient)
{
	if (pPubClient == NULL)
	{
		return;
	}
	
	SipTranscClientUnLockAPI(&pPubClient->transcClient);
}

/************************************************************************************
 * PubClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
RvStatus RVCALLCONV PubClientLockMsg(IN  PubClient *pPubClient)
{
	if (pPubClient == NULL)
	{
		return RV_ERROR_NULLPTR;
	}
	
	return SipTranscClientLockMsg(&pPubClient->transcClient);
}


/************************************************************************************
 * PubClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks pub-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
void RVCALLCONV PubClientUnLockMsg(IN  PubClient *pPubClient)
{
	if (pPubClient == NULL)
	{
		return;
	}
	
	SipTranscClientUnLockMsg(&pPubClient->transcClient);
}

#endif

/***************************************************************************
 * PubClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the publish-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV PubClientMgrLock(IN PubClientMgr *pPubClientMgr)
{
    if (NULL == pPubClientMgr)
    {
        return;
    }
    RvMutexLock(&pPubClientMgr->hLock,pPubClientMgr->pLogMgr);
}


/***************************************************************************
 * PubClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the publish-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV PubClientMgrUnLock(IN PubClientMgr *pPubClientMgr)
{
    if (NULL == pPubClientMgr)
    {
        return;
    }
    RvMutexUnlock(&pPubClientMgr->hLock,pPubClientMgr->pLogMgr);
}

/***************************************************************************
 * SipPubClientAddSipIfMatchHeaderIfNeeded
 * ------------------------------------------------------------------------
 * General: This function responsible to add the sip-if-match header known by this pub client.
 *			The sip-if-match is used by the publisher to identify the incoming publish-client.
 * Return Value: - RV_OK - when the sip-if match header was succesfully inserted into the message or
 *				   the sip-if-match is unknown (i.e. pub-client didn't published yet), otherwise rv_error.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: ppubClient - The pub-client that is going to be publishing
 *			 hMsgToSend - The message to be update.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientAddSipIfMatchHeaderIfNeeded(IN PubClient *pPubClient, 
															IN RvSipMsgHandle hMsgToSend)
{
	RvSipHeaderListElemHandle hListElem;
    void                     *hMsgListElem;
	RvStatus				  retStatus;
	
	/*If this pubClient was already published The sip-if-match header should be populated
	  with the sipEtag header received in the incoming response.*/
	if (RV_TRUE == pPubClient->bWasPublished && pPubClient->hSipIfMatchHeader != NULL)
    {
        hListElem = NULL;
		
        retStatus = RvSipMsgPushHeader(
			hMsgToSend, RVSIP_LAST_HEADER,
			(void *)(pPubClient->hSipIfMatchHeader),
			RVSIP_HEADERTYPE_OTHER, &hListElem,
			&hMsgListElem);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
		retStatus = RvSipOtherHeaderCopy(hMsgListElem, pPubClient->hSipIfMatchHeader);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}
    }
	return RV_OK;
}


/***************************************************************************
 * SipPubClientAddEventHeaderIfNeeded
 * ------------------------------------------------------------------------
 * General: This function responsible to add the Event header known by this pub client.
 *			Adding the event header will be only if such header doesn't exist in the message.
 * Return Value: - RV_OK - when the Event header was succesfully inserted into the message or
 *				   the event header already exists , otherwise rv_error.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: ppubClient - The pub-client that is going to be publishing
 *			 hMsgToSend - The message to be update.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientAddEventHeaderIfNeeded(IN PubClient *pPubClient, 
													   IN RvSipMsgHandle hMsgToSend)
{
	RvSipHeaderListElemHandle hListElem;
    void                     *hMsgListElem;
	RvStatus				  retStatus;
	RvSipEventHeaderHandle	  hEventHeader;
	
    hListElem = NULL;
	
	hEventHeader = RvSipMsgGetHeaderByType(hMsgToSend, RVSIP_HEADERTYPE_EVENT, RVSIP_FIRST_HEADER, &hListElem);

	if (hEventHeader != NULL)
	{
		return RV_OK;
	}

    retStatus = RvSipMsgPushHeader(
		hMsgToSend, RVSIP_LAST_HEADER,
		(void *)(pPubClient->hEventHeader),
		RVSIP_HEADERTYPE_EVENT, &hListElem,
		&hMsgListElem);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	return RvSipEventHeaderCopy(hMsgListElem, pPubClient->hEventHeader);
	
}

/***************************************************************************
 * PubClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a PubClient object, Free the resources taken by it and
 *          remove it from the manager pub-client list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - Pointer to the pub-client to terminate.
 *          eReason - termination reason
 ***************************************************************************/
void RVCALLCONV PubClientTerminate(IN PubClient *pPubClient)
{
    if(pPubClient->eState == RVSIP_PUB_CLIENT_STATE_TERMINATED)
    {
        return;
    }
    pPubClient->eState = RVSIP_PUB_CLIENT_STATE_TERMINATED;

	SipTranscClientTerminate(&pPubClient->transcClient);

    return;
}

/***************************************************************************
 * PubClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the PubClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient    - pointer to PubClient
 ***************************************************************************/
void PubClientDetachFromConnection(IN PubClient        *pPubClient)
{
    SipTranscClientDetachFromConnection(&pPubClient->transcClient);
}

/***************************************************************************
 * PubClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the PubClient as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient    - pointer to PubClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus PubClientAttachOnConnection(IN PubClient                        *pPubClient,
									 IN RvSipTransportConnectionHandle  hConn)
{

    RvStatus rv = RV_OK;
    
	if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "PubClientAttachOnConnection - Failed to attach PubClient 0x%p to connection 0x%p",pPubClient,hConn));
        SipTranscClientAttachOnConnection(&pPubClient->transcClient, NULL);
    }
    else
    {
        SipTranscClientAttachOnConnection(&pPubClient->transcClient, hConn);
    }
    return rv;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * PubClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Pub-Client.
 *          As a result of this operation, all messages, sent by this
 *          Pub-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pPubClient - Pointer to the Pub-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus PubClientSetSecObj(IN  PubClient*        pPubClient,
                            IN  RvSipSecObjHandle hSecObj)
{
    RvStatus      rv;

	rv = SipTranscClientSetSecObj(&pPubClient->transcClient, hSecObj);
    
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/******************************************************************************
 * PubClientSetAlertTimerIfNeeded
 * ----------------------------------------------------------------------------
 * General: This method set a timer for the transcClient to invoke when the object is expired.
 * Return Value: 
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pPubClient	- Pointer to the Pub-Client object.
 *          hMsg		- The received message, used to get the expires header.
 *****************************************************************************/
RvStatus PubClientSetAlertTimerIfNeeded(IN PubClient					*pPubClient, 
									IN  RvSipMsgHandle				hMsg)
{

	RvSipHeaderListElemHandle  hListElem;
	RvSipExpiresHeaderHandle   hExpiresHeader;
	RvUint32					   expiresValue;
	RvUint32					   timeToSet;
	RvStatus					rv;


	/*We are setting the timer for successful response before the object changed it state to published
	  Therefore we are checking whether the object is in publishing state and not published state*/
	if (pPubClient->eState != RVSIP_PUB_CLIENT_STATE_PUBLISHING ||
		pPubClient->pMgr->bHandleExpTimer == RV_FALSE)
	{
		return RV_ERROR_ILLEGAL_ACTION;
	}

	hExpiresHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EXPIRES, RVSIP_FIRST_HEADER, &hListElem);
	
	if (hExpiresHeader == NULL)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientSetAlertTimerIfNeeded - missing Expires header on incoming message 0x%p, cannot set alert timer!", hMsg));
		return RV_ERROR_NOT_FOUND;
	}

	rv = RvSipExpiresHeaderGetDeltaSeconds(hExpiresHeader, &expiresValue);
	if (rv != RV_OK)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientSetAlertTimerIfNeeded - Unable to get expires value from 0x%p, cannot set alert timer!", hExpiresHeader));
		return RV_ERROR_NOT_FOUND;
	}

	if (pPubClient->pMgr->pubClientAlertTimeOut >= expiresValue*1000)
	{
		RvLogWarning(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientSetAlertTimerIfNeeded - Publish client 0x%p alertTimeout %d (milli) is greater then expires header value %d (milli), Setting timer to Expires value.", pPubClient, pPubClient->pMgr->pubClientAlertTimeOut, expiresValue*1000));
		timeToSet = expiresValue*1000;
	}
	else
	{
		/*we are multiplying the epiresValue with 1000 to convert it into milliseconds.*/
		timeToSet = expiresValue*1000 - pPubClient->pMgr->pubClientAlertTimeOut;
	}

	return SipTranscClientSetAlertTimer(&pPubClient->transcClient, timeToSet);
}

/***************************************************************************
 * PubClientUpdateSipIfMatchHeader
 * ------------------------------------------------------------------------
 * General: This function will invoked upon response received and will update the 
 *			SIP-If-Match header of the pub client from the SipEtag header exist in this response.
 * Return Value: pPubClient - the publish client to be update.
 *				 hRecievedMsg - the response received
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvStatus PubClientUpdateSipIfMatchHeader(IN PubClient *pPubClient,RvSipMsgHandle hRecievedMsg)
{
	RvSipHeaderListElemHandle  hListElem;
	RvSipOtherHeaderHandle  hSipETag;
	RvStatus				retStatus;
	PubClientMgr			*pPubClientMgr = pPubClient->pMgr;


	RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientUpdateSipIfMatchHeader - Starting to copy SIP-ETag header (0x%p) value to SIP-IF-MATCH header (0x%p)", hRecievedMsg, pPubClient->hSipIfMatchHeader));
	
	hSipETag = RvSipMsgGetHeaderByName(hRecievedMsg, SIP_ETAG_HEADER_NAME, RVSIP_FIRST_HEADER, &hListElem);

	if (hSipETag == NULL)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientUpdateSipIfMatchHeader - missing SIP-ETag on incoming message 0x%p", hRecievedMsg));
		return RV_ERROR_UNKNOWN;
	}

	if (pPubClient->hSipIfMatchPage != NULL)
	{
		RPOOL_FreePage(pPubClientMgr->hGeneralMemPool, pPubClient->hSipIfMatchPage);
		pPubClient->hSipIfMatchHeader = NULL;
		pPubClient->hSipIfMatchPage = NULL;
	}

	/* Get a new memory page for the publish-client object */
    retStatus = RPOOL_GetPage(pPubClientMgr->hGeneralMemPool, 0,
                              &pPubClient->hSipIfMatchPage);
	if(retStatus != RV_OK)
    {
        return retStatus;
    }

	retStatus = RvSipOtherHeaderConstruct(pPubClientMgr->hMsgMgr, 
			pPubClientMgr->hGeneralMemPool,
			pPubClient->hSipIfMatchPage, &pPubClient->hSipIfMatchHeader); 
		if(retStatus != RV_OK)
		{
			RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
				"PubClientUpdateSipIfMatchHeader - Unable to construct sip-if-match header rv=%d", retStatus));
			return retStatus;
		}
	
	retStatus = RvSipOtherHeaderCopy(pPubClient->hSipIfMatchHeader, hSipETag);
	if(retStatus != RV_OK)
    {
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientUpdateSipIfMatchHeader - Unable to copy SIP-ETag header 0x%p to SIP-IF-MATCH header 0x%p rv=%d", hRecievedMsg, pPubClient->hSipIfMatchHeader, retStatus));
        return retStatus;
    }

	retStatus = RvSipOtherHeaderSetName(pPubClient->hSipIfMatchHeader, SIP_IF_MATCH_HEADER_NAME);
	if(retStatus != RV_OK)
    {
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientUpdateSipIfMatchHeader - missing SIP-ETag header on incoming message 0x%p rv=%d", hRecievedMsg, retStatus));
        return retStatus;
    }

	return RV_OK;

}


/***************************************************************************
 * PubClientSetEventHeader
 * ------------------------------------------------------------------------
 * General: Set an Event header in the publish-client object. Before
 *          calling Publish(), the application can use this function to
 *          supply the required Event header to use in the Publish
 *          request. 
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Event header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Event header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hEventHeader - Handle to an Event header to be set to the
 *                           publish-client.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientSetEventHeader (
                                 IN  PubClient				*pPubClient,
                                 IN  RvSipEventHeaderHandle hEventHeader)
{

	if ((NULL == pPubClient) ||
        (NULL == hEventHeader))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "PubClientSetEventHeader - Setting Publish-Client 0x%p Event header",
              pPubClient));

    /* Check if the given header was constructed on the Publish-client
       object's page and if so attach it */
    if((SipEventHeaderGetPool(hEventHeader) ==
                                        pPubClient->pMgr->hGeneralMemPool) &&
       (SipEventHeaderGetPage(hEventHeader) == pPubClient->hPage))
    {
        pPubClient->hEventHeader = hEventHeader;
        return RV_OK;
    }
    

	return PubClientCopyEventHeader(pPubClient, hEventHeader);
	
}

/***************************************************************************
 * PubClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV PubClientStateIntoString(
                                            IN RvSipPubClientState eNewState)
{
    switch (eNewState)
    {
    case RVSIP_PUB_CLIENT_STATE_IDLE:
        return "Idle";
    case RVSIP_PUB_CLIENT_STATE_TERMINATED:
        return PUB_CLIENT_STATE_TERMINATED_STR;
    case RVSIP_PUB_CLIENT_STATE_PUBLISHING:
        return "Publishing";
    case RVSIP_PUB_CLIENT_STATE_REDIRECTED:
        return "Redirected";
    case RVSIP_PUB_CLIENT_STATE_UNAUTHENTICATED:
        return "Unauthenticated";
    case RVSIP_PUB_CLIENT_STATE_PUBLISHED:
        return "Published";
    case RVSIP_PUB_CLIENT_STATE_FAILED:
        return "Failed";
	case RVSIP_PUB_CLIENT_STATE_REMOVING:
		return "Removing";
	case RVSIP_PUB_CLIENT_STATE_REMOVED:
		return "Removed";
    case RVSIP_PUB_CLIENT_STATE_MSG_SEND_FAILURE:
        return "Msg Send Failure";
    default:
        return "Undefined";
    }
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * PubClientReasonIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the reason received as enumeration.
 * Return Value: The reason string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason enumeration.
 ***************************************************************************/
static RvChar *RVCALLCONV PubClientReasonIntoString(
                                   IN RvSipPubClientStateChangeReason eReason)
{
    switch (eReason)
    {
    case RVSIP_PUB_CLIENT_REASON_UNDEFINED:
        return "Undefined";
    case RVSIP_PUB_CLIENT_REASON_USER_REQUEST:
        return "User request";
    case RVSIP_PUB_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD:
        return "Response successful received";
    case RVSIP_PUB_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD:
        return "Response redirection received";
    case RVSIP_PUB_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD:
        return "Response unauthentication received";
    case RVSIP_PUB_CLIENT_REASON_RESPONSE_REMOTE_REJECT_RECVD:
        return "Response reject received";
	case RVSIP_PUB_CLIENT_REASON_RESPONSE_COND_REQ_FAILED:
		return "Conditional request failed";
    case RVSIP_PUB_CLIENT_REASON_TRANSACTION_TIMEOUT:
        return "Transaction timeout";
    case RVSIP_PUB_CLIENT_REASON_NETWORK_ERROR:
        return "Network Error";
    case RVSIP_PUB_CLIENT_REASON_503_RECEIVED:
        return "503 Received";
	case RVSIP_PUB_CLIENT_REASON_MISSING_SIP_ETAG:
		return "Missing SIP-ETag header";
	case RVSIP_PUB_CLIENT_REASON_MISSING_EXPIRES:
		return "Missing Expires header";
    default:
        return "";
    }
}

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/***************************************************************************
 * PubClientCopyEventHeader
 * ------------------------------------------------------------------------
 * General: Coppying the event header from external pool/page to the publish client mem-pool.
 * Return Value: .
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: status - whether the copy worked.
 ***************************************************************************/
static RvStatus PubClientCopyEventHeader (PubClient	*pPubClient,
										  RvSipEventHeaderHandle hEventHeader)
{

	RvStatus retStatus = RV_OK;
	
	if (NULL == pPubClient->hEventHeader)
    {
        /* Construct a new Expires header for the transaction-client object */
        retStatus = RvSipEventHeaderConstruct(
			pPubClient->pMgr->hMsgMgr,
			pPubClient->pMgr->hGeneralMemPool,
			pPubClient->hPage,
			&(pPubClient->hEventHeader));
        if (RV_OK != retStatus)
        {
            RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
				"PubClientCopyEventHeader - Publish client 0x%p: Error setting Event header",
				pPubClient));
            pPubClient->hEventHeader = NULL;
            return retStatus;
        }
    }
    /* Copy the given Expires header to the Publish-client object's Event
	header */
    retStatus = RvSipEventHeaderCopy(pPubClient->hEventHeader, hEventHeader);
    if (RV_OK != retStatus)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientCopyEventHeader - Publish-client 0x%p: Error setting Publish header",
			pPubClient));
        pPubClient->hEventHeader = NULL;
    }
	return retStatus;
}
#endif /* RV_SIP_PRIMITIVES */

