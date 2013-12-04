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
 *                              <RvSipPubClient.c>
 *
 * This file defines the PubClient API functions.
 * The API contains two major parts:
 * 1. The publish-client manager API: The publish-client manager is incharged
 *                                     of all the publish-clients. It is used
 *                                     to set the event handlers of the
 *                                     publish-client module and to create new
 *                                     publish-client.
 * 2. The publish-client API: Using the publish-client API the user can
 *                             request to PUBLISH at a chosen publisher. It
 *                             can redirect a publish request and authenticate
 *                             a publish request when needed.
 *                             A PubClient is a stateful object and has a set
 *                             of states associated with it.
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
#include "_SipPubClientMgr.h"
#include "PubClientMgrObject.h"
#include "PubClientObject.h"
#include "RvSipPubClient.h"
#include "PubClientCallbacks.h"

#include "_SipTranscClient.h"

#include "AdsRlist.h"
#include "_SipPartyHeader.h"
#include "_SipContactHeader.h"
#include "_SipExpiresHeader.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include "_SipTransport.h"
#include "_SipCommonCSeq.h"
#include "RvSipCompartmentTypes.h"

#include "_SipCommonConversions.h"
#include "_SipHeader.h"

#ifdef RV_SIP_IMS_ON
#include "_SipAuthorizationHeader.h"
#include "RvSipTransaction.h"
#endif /*#ifdef RV_SIP_IMS_ON*/


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RvSipPubClientMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all publish-client events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the event handler structure.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the publish-client manager.
 *            pEvHandlers - Pointer to the application event handler structure
 *            structSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrSetEvHandlers(
                                   IN  RvSipPubClientMgrHandle   hMgr,
                                   IN  RvSipPubClientEvHandlers *pEvHandlers,
                                   IN  RvInt32                  structSize)
{
    PubClientMgr       *pPubClientMgr;

    if (NULL == hMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pEvHandlers)
    {
        return RV_ERROR_NULLPTR;
    }
    if (0 > structSize)
    {
        return RV_ERROR_BADPARAM;
    }
    pPubClientMgr = (PubClientMgr *)hMgr;
    PubClientMgrLock(pPubClientMgr);
    /* Copy the event handlers struct according to the given size */
    memset(&(pPubClientMgr->pubClientEvHandlers), 0,
                                            sizeof(RvSipPubClientEvHandlers));
    memcpy(&(pPubClientMgr->pubClientEvHandlers), pEvHandlers, structSize);
	
	if (pPubClientMgr->pubClientEvHandlers.pfnObjExpiredEvHandler != NULL)
	{
		pPubClientMgr->bHandleExpTimer = RV_TRUE;
	}

    PubClientMgrUnLock(pPubClientMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipPubClientMgrCreatePubClient
 * ------------------------------------------------------------------------
 * General: Creates a new publish-client and replace handles with the
 *          application.  The new publish-client assumes the idle state.
 *          In order to publish to a publisher you should:
 *          1. Create a new PubClient with this function.
 *          2. Set at least the To and From headers.
 *          3. Call the Publish() function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the publish-client
 *                                   handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create a new
 *                                   PubClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClientMgr - Handle to the publish-client manager
 *            hAppPubClient - Application handle to the newly created publish-
 *                          client.
 * Output:     hPubClient -    sip stack handle to the publish-client
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrCreatePubClient(
                                   IN  RvSipPubClientMgrHandle hPubClientMgr,
                                   IN  RvSipAppPubClientHandle hAppPubClient,
                                   OUT RvSipPubClientHandle   *phPubClient)
{
    PubClientMgr       *pPubClientMgr;
    RvStatus           retStatus;

    if (NULL == hPubClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == phPubClient)
    {
        return RV_ERROR_NULLPTR;
    }
    pPubClientMgr = (PubClientMgr *)hPubClientMgr;
    PubClientMgrLock(pPubClientMgr);
    RvLogDebug(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
            "RvSipPubClientMgrCreatePubClient - Starting to create pub client"));

    /* Insert the new publish-client object to the end of the list */
    if (NULL == pPubClientMgr->hPubClientList)
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
                  "RvSipPubClientMgrCreatePubClient - Error - Failed to allocate new publish-client object"));
        PubClientMgrUnLock(pPubClientMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    retStatus = RLIST_InsertTail(pPubClientMgr->hPubClientListPool,
                                 pPubClientMgr->hPubClientList,
                                 (RLIST_ITEM_HANDLE *)phPubClient);
    if (RV_OK != retStatus)
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
                  "RvSipPubClientMgrCreatePubClient - Error - Failed to allocate new publish-client object"));
        PubClientMgrUnLock(pPubClientMgr);
        return retStatus;
    }
    /* Initiate the publish-client object */
    retStatus = PubClientInitiate(pPubClientMgr, (PubClient *)(*phPubClient),
                                  hAppPubClient);
    if (RV_OK != retStatus)
    {
        RvLogError(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
              "RvSipPubClientMgrCreatePubClient - Publish client 0x%p: Error initiating a publish-client object",
              *phPubClient));
        RLIST_Remove(pPubClientMgr->hPubClientListPool,
                     pPubClientMgr->hPubClientList,
                     (RLIST_ITEM_HANDLE)(*phPubClient));
        PubClientMgrUnLock(pPubClientMgr);
        return retStatus;
    }
    RvLogInfo(pPubClientMgr->pLogSrc,(pPubClientMgr->pLogSrc,
              "RvSipPubClientMgrCreatePubClient - Publish client 0x%p: was successfully created",
              *phPubClient));
    PubClientMgrUnLock(pPubClientMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipPubClientMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this pub-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClientMgr   - Handle to the publish-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrGetStackInstance(
                                   IN   RvSipPubClientMgrHandle   hPubClientMgr,
                                   OUT  void*         *phStackInstance)
{
    PubClientMgr *pPubClientMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == hPubClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClientMgr = (PubClientMgr *)hPubClientMgr;
    return SipTranscClientMgrGetStackInstance(pPubClientMgr->hTranscClientMgr, phStackInstance);
}

/***************************************************************************
 * RvSipPubClientPublish
 * ------------------------------------------------------------------------
 * General: Sends a PUBLISH request to the publisher. The Request-Url,To,
 *          From , Expires and Contact headers that were set to the publish
 *          client are inserted to the outgoing PUBLISH request.
 *          The publish-client will assumes the Publishing state and wait for
 *          a response from the server.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the publish-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid publish-client state for this
 *                                  action, or the To and From header have
 *                                  yet been set to the publish-client object,
 *                                  or the publisher Request-Url has not
 *                                  yet been set to the publish-client object.
 *               RV_ERROR_OUTOFRESOURCES - publish-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientPublish (
                                         IN RvSipPubClientHandle hPubClient)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;
	RvUint32		   expVal;



    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientPublish - publish client 0x%p ",pPubClient));

    /* Assert that the current state allows this action */
    if ((RVSIP_PUB_CLIENT_STATE_IDLE != pPubClient->eState) &&
         (RVSIP_PUB_CLIENT_STATE_REDIRECTED != pPubClient->eState) &&
         (RVSIP_PUB_CLIENT_STATE_PUBLISHED != pPubClient->eState) && 
		 (RVSIP_PUB_CLIENT_STATE_FAILED	!= pPubClient->eState &&
		  RVSIP_PUB_CLIENT_STATE_UNAUTHENTICATED != pPubClient->eState))
    {
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
	
	/*Cancelling any previous timer since we are sending new publish request.*/
	SipTranscClientSetAlertTimer(&pPubClient->transcClient, 0);

	if (RVSIP_PUB_CLIENT_STATE_IDLE == pPubClient->eState && 
		pPubClient->hEventHeader == NULL)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
	            "RvSipPubClientPublish - Unable to publish object 0x%p before setting up Event header to this object.",pPubClient));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;

	}

	/*Using the publish API when the expires value was set to 0 is invalid*/
	retStatus = SipTranscClientGetExpiresValue(&pPubClient->transcClient, &expVal);
	if (retStatus == RV_OK && expVal == 0)
    {
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"RvSipPubClientPublish - Unable to publish object 0x%p when expires value is equals to 0 (zero).",pPubClient));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

	retStatus = SipTranscClientActivate(&pPubClient->transcClient, SIP_TRANSACTION_METHOD_OTHER, "PUBLISH");
    if (RV_OK != retStatus)
    {
		PubClientUnLockAPI(pPubClient);
        return retStatus;
    }

	/*Since the message was successfully sent we need to reset the retry after interval*/
	pPubClient->retryAfter = 0;

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientPublish - Publish client 0x%p: PUBLISH request was successfully sent",
               pPubClient));
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientRemove
 * ------------------------------------------------------------------------
 * General: Sends a PUBLISH request to the publisher. The Request-Url,To,
 *          From , Expires and Contact headers that were set to the publish
 *          client are inserted to the outgoing PUBLISH request.
 *          The publish-client will assumes the Removing state and wait for
 *          a response from the server.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the publish-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid publish-client state for this
 *                                  action, or the To and From header have
 *                                  yet been set to the publish-client object,
 *                                  or the publisher Request-Url has not
 *                                  yet been set to the publish-client object.
 *               RV_ERROR_OUTOFRESOURCES - publish-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientRemove (
                                         IN RvSipPubClientHandle hPubClient)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;
	RvSipExpiresHeaderHandle hExpires;
	RvUint32				lastExpiresValue = 0;



    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientRemove() - publish client 0x%p ",pPubClient));

    /* Remove is allowed only when the pub-client is in state PUBLISHED*/
    if (RVSIP_PUB_CLIENT_STATE_PUBLISHED != pPubClient->eState &&
		RVSIP_PUB_CLIENT_STATE_FAILED	!= pPubClient->eState)
    {
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientRemove() - Remove is not allowed in state %d of object 0x%p ",pPubClient->eState, pPubClient));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

	pPubClient->bRemoveInProgress = RV_TRUE;

	/*Cancelling any previous timer since we are sending new publish request.*/
	SipTranscClientSetAlertTimer(&pPubClient->transcClient, 0);

 	retStatus = SipTranscClientGetExpiresHeader(&pPubClient->transcClient, &hExpires);
	if (retStatus != RV_OK)
	{
		pPubClient->bRemoveInProgress = RV_FALSE;
		PubClientUnLockAPI(pPubClient);
		return retStatus;
	}

	/*If the expires header not found we need to create a new one since the remove action
	  must send an expires header with the value 0*/
	if (hExpires == NULL)
	{
		retStatus = SipTranscClientGetNewMsgElementHandle(&pPubClient->transcClient, RVSIP_HEADERTYPE_EXPIRES, RVSIP_ADDRTYPE_UNDEFINED , (void*)&hExpires);
		if(retStatus != RV_OK)
		{
			RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
				"RvSipPubClientRemove - Publish client 0x%p: Failed to create new expires header.",
				pPubClient));
			pPubClient->bRemoveInProgress = RV_FALSE;
			PubClientUnLockAPI(pPubClient);
			return retStatus;
		}
		retStatus = SipTranscClientSetExpiresHeader(&pPubClient->transcClient, hExpires);
		if (retStatus != RV_OK)
		{
			RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
				"RvSipPubClientRemove - Publish client 0x%p: Failed to set expires in transc-client.",
				pPubClient));
			pPubClient->bRemoveInProgress = RV_FALSE;
			PubClientUnLockAPI(pPubClient);
			return retStatus;
		}
	}
	else 
	{
		/*If there was an expires header we are keeping its last value in case of an error*/
		RvSipExpiresHeaderGetDeltaSeconds(hExpires, &lastExpiresValue);
	}

	

	retStatus = RvSipExpiresHeaderSetDeltaSeconds(hExpires, 0);
	if (retStatus != RV_OK)
	{
		pPubClient->bRemoveInProgress = RV_FALSE;
		PubClientUnLockAPI(pPubClient);
		return retStatus;
	}

	retStatus = SipTranscClientActivate(&pPubClient->transcClient, SIP_TRANSACTION_METHOD_OTHER, "PUBLISH");
    if (RV_OK != retStatus)
    {
		pPubClient->bRemoveInProgress = RV_FALSE;
		if (lastExpiresValue == 0)
		{
			SipTranscClientSetExpiresHeader(&pPubClient->transcClient, NULL);
		}
		else
		{
			RvSipExpiresHeaderSetDeltaSeconds(hExpires, lastExpiresValue);
		}
		PubClientUnLockAPI(pPubClient);
        return retStatus;
    }

	/*Since the message was successfully sent we need to reset the retry after interval*/
	pPubClient->retryAfter = 0;

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientRemove - Publish client 0x%p: PUBLISH request was successfully sent, for removing",
               pPubClient));
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}


/***************************************************************************
 * RvSipPubClientInit
 * ------------------------------------------------------------------------
 * General: Set the publish client from,to,request-uri parameters that
 *          are given as strings to the pubClient. 
 *			If a null value is supplied as one of the arguments this parameter 
 *			will not be set. After calling this function the application can call the publish
 *			method without further details.
 *          The publish-client will stay in the IDLE state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the publish-client is
 *                                   invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid publish-client state for this
 *                                  action or there is a missing parameter that
 *                                  is needed for the publication process such as
 *                                  the request URI.
 *               RV_ERROR_OUTOFRESOURCES - publish-client failed to create 
 *										   and set one of the fields
 *               RV_ERROR_UNKNOWN - An error occurred while trying create or set field.
 *               RV_OK - The init succeeded.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClient - The publish-client handle.
 *          strFrom    - the initiator of the publication request.
 *                       for example: "sip:user@home.com"
 *          strTo      - The publishing user.
 *                       "sip:bob@proxy.com"
 *          strPublisher - The request URI of the publication request.This
 *                         is the proxy address.
 *                         for example: "sip:proxy.com"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientInit (
                                         IN RvSipPubClientHandle hPubClient,
                                         IN RvChar*             strFrom,
                                         IN RvChar*             strTo,
                                         IN RvChar*             strPublisher)
{
    RvStatus rv;
    PubClient                *pPubClient;

	if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientInit - Publish client 0x%p: starting to initiate", pPubClient));


	/*Since the publish client is handling the contact header by it self we should send NULL as the contact header*/
	rv = SipTranscClientSetActivateDetails(&pPubClient->transcClient, strFrom, strTo, strPublisher, RV_TRUE);
    if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientInit - Publish client 0x%p: Failed to allocate from party header",
            pPubClient));
		PubClientUnLockAPI(pPubClient);
        return rv;
    }

	RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientInit - Publish client 0x%p: initiated successfully.", pPubClient));
        
	PubClientUnLockAPI(pPubClient);
    return RV_OK;

}

/***************************************************************************
 * RvSipPubClientDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach the Publish-Client owner. After calling this function the user will
 *          stop receiving events for this Publish-Client. This function can be called
 *          only when the object is in terminated state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDetachOwner (
                                         IN RvSipPubClientHandle hPubClient)
{
    PubClient *pPubClient;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pPubClient->eState != RVSIP_PUB_CLIENT_STATE_TERMINATED)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
          "RvSipPubClientDetachOwner - Publish client 0x%p is in state %s, and not in state terminated - can't detach owner",
          pPubClient, PubClientStateIntoString(pPubClient->eState)));

		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    else
    {
        pPubClient->pubClientEvHandlers = NULL;
        RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientDetachOwner - Publish client 0x%p: was detached from owner",
               pPubClient));
    }

	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a publish-client object. This function destructs the
 *          publish-client object. You cannot reference the object after
 *          calling this function.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the publish-client is
 *                                   invalid.
 *                 RV_OK - The publish-client was successfully terminated
 *                            and distructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientTerminate (
                                         IN RvSipPubClientHandle hPubClient)
{
    PubClient *pPubClient;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pPubClient->eState == RVSIP_PUB_CLIENT_STATE_TERMINATED)
    {
		PubClientUnLockAPI(pPubClient);
        return RV_OK;
    }

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientTerminate - About to  terminate Publish client 0x%p",
               pPubClient));
    PubClientTerminate(pPubClient);
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientUseFirstRouteForPublishRequest
 * ------------------------------------------------------------------------
 * General: The Application may want to use a preloaded route header when
 *          sending the publish message.For this purpose, it should add the
 *          Route headers to the outbound message, and call this function
 *          to indicate the stack to send the request to the address of the
 *          first Route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient    - Handle to the publish client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientUseFirstRouteForPublishRequest (
                                      IN  RvSipPubClientHandle hPubClient)
{
    PubClient *pPubClient;
	RvStatus   rv;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientUseFirstRouteForPublishRequest - pub-client 0x%p will use preloaded Route header",
              pPubClient));

	rv = SipTranscClientUseFirstRouteForRequest(&pPubClient->transcClient);

	PubClientUnLockAPI(pPubClient);
    return rv;

}




/*-----------------------------------------------------------------------
       P U B  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPubClientDNSContinue
 * ------------------------------------------------------------------------
 * General: Prepares the Pub-Client object to a retry to send a request after
 *          the previous attempt failed.
 *          When a Publish request fails due to timeout, network error or
 *          503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          In order to retry to send the message the application must first
 *          call the RvSipPubClientDNSContinue() function.
 *          Calling this function, clones the failure transaction and
 *          updates the DNS list. (In order to actually re-send the request
 *          to the next IP address use the function RvSipPubClientDNSReSendRequest()).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the Pub-Client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSContinue (
                                      IN  RvSipPubClientHandle   hPubClient)
{
    PubClient* pPubClient = (PubClient*)hPubClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = PubClientLockAPI(pPubClient); /*try to lock the pub client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pPubClient->transcClient.hSecAgree ||
        NULL != pPubClient->transcClient.hSecObj)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientDNSContinue - pub client 0x%p: illegal action for IMS Secure Pub Client",
            pPubClient));
	    PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientDNSContinue - pub-client 0x%p: continue DNS" ,
            pPubClient));

    rv = SipTranscClientDNSContinue(&pPubClient->transcClient);

	PubClientUnLockAPI(pPubClient);
    return rv;
#else
    RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientDNSContinue - pub-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pPubClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipPubClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: Stops retries to send a Publish request after send failure.
 *          When a Publish request fails due to timeout, network error or
 *          503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          Calling RvSipPubClientDNSGiveUp() indicates that the application
 *          wishes to give up on this request. Retries to send the request
 *          will stop and the Publish client will change its state back to
 *          the previous state.
 *          If this is the initial Publish request of the Pub-Client, calling
 *          DNSGiveUp() will change the publish-client object state to FAILED. If this is a
 *          refresh, the Pub-Client will move back to the PUBLISHED state.
 * Return Value: RvStatus;
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the Pub-Client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSGiveUp (
                                   IN  RvSipPubClientHandle   hPubClient)
{
    PubClient* pPubClient = (PubClient*)hPubClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = PubClientLockAPI(pPubClient); /*try to lock the pub client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pPubClient->transcClient.hSecAgree ||
        NULL != pPubClient->transcClient.hSecObj)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientDNSGiveUp - pub client 0x%p: illegal action for IMS Secure Pub Client",
            pPubClient));
	    PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientDNSGiveUp - pub-client 0x%p: giveup DNS" ,
              pPubClient));
    rv = SipTranscClientDNSGiveUp(&pPubClient->transcClient);

	PubClientUnLockAPI(pPubClient);
    return rv;
#else
    RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientDNSGiveUp - pub client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pPubClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipPubClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: Re-sends a publish request after the previous attempt failed.
 *          When a Publish request fails due to timeout, network error or
 *          503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          In order to re-send the request first call RvSipPubClientDNSContinue().
 *          You should then call RvSipPubClientDNSReSendRequest().
 *          The request will automatically be sent to the next resoulved IP address
 *          in the DNS list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the pub client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSReSendRequest (
                                            IN  RvSipPubClientHandle   hPubClient)
{
    PubClient* pPubClient = (PubClient*)hPubClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = PubClientLockAPI(pPubClient); /*try to lock the pub client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pPubClient->transcClient.hSecAgree ||
        NULL != pPubClient->transcClient.hSecObj)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientDNSReSendRequest - pub client 0x%p: illegal action for IMS Secure Pub Client",
            pPubClient));
	    PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientDNSReSendRequest - pub client 0x%p: re-send DNS" ,
            pPubClient));

    rv = SipTranscClientDNSReSendRequest(&pPubClient->transcClient, 
											SIP_TRANSACTION_METHOD_OTHER, 
											"PUBLISH");
	PubClientUnLockAPI(pPubClient);
    return rv;
#else
    RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientDNSReSendRequest - pub client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pPubClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipPubClientDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves DNS list object from the Pub-Client current active
 *          transaction.
 *          When a Publish request fails due to timeout, network error or
 *          503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state you can use RvSipPubClientDNSGetList() to get the
 *          DNS list if you wish to view or change it.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the pub client that sent the request.
 * Output   phDnsList - The DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSGetList(
                              IN  RvSipPubClientHandle         hPubClient,
                              OUT RvSipTransportDNSListHandle *phDnsList)
{
    PubClient* pPubClient = (PubClient*)hPubClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = PubClientLockAPI(pPubClient); /*try to lock the pub client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientDNSGetList - pub client 0x%p: getting DNS list" ,
              pPubClient));

    rv = SipTranscClientDNSGetList(&pPubClient->transcClient,phDnsList);

	PubClientUnLockAPI(pPubClient);
    return rv;
#else
    RV_UNUSED_ARG(phDnsList);
    RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientDNSGetList - pub client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pPubClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipPubClientGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the pub-client page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information
 *         and set it back to the pub-client using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         Party   - you should set these headers back with the RvSipPubClientSetToHeader()
 *                   or RvSipPubClientSetFromHeader() API functions.
 *         Expires - you should set this header back with the RvSipPubClientSetExpiresHeader()
 *                   API function.
 *         Authorization - you should set this header back with the header to the RvSipPubClientSetInitialAuthorization() 
 *                   API function that is available in the IMS add-on only.
 *         address - you should supply the address to the RvSipPubClientSetRequestURI() API function.
 *
 *          Note - You may use this function only on pub-client initialization stage when the
 *          pub-client is in the IDLE state.
 *          In order to change headers after the initialization stage, you must allocate the 
 *          header on an application page, and then set it with the correct API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the pub-client.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetNewMsgElementHandle (
                                      IN   RvSipPubClientHandle   hPubClient,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader)
{
    RvStatus  rv;
    PubClient    *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = PubClientLockAPI(pPubClient); /*try to lock the pub-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    

	/*Event header is something relevant to the pub-client only and not all client so the event header will stay
	on the mem pool of the pub-client and not transc-client*/
	if (eHeaderType == RVSIP_HEADERTYPE_EVENT)
	{
		rv = SipHeaderConstruct(pPubClient->pMgr->hMsgMgr, eHeaderType,pPubClient->pMgr->hGeneralMemPool,
				pPubClient->hPage, phHeader);
	}
	else
	{
		rv = SipTranscClientGetNewMsgElementHandle(&pPubClient->transcClient, eHeaderType, eAddrType, phHeader);
	}
	

    if (rv != RV_OK || *phHeader == NULL)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientGetNewMsgElementHandle - pub-client 0x%p - Failed to create new header (rv=%d)",
              pPubClient, rv));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_OUTOFRESOURCES;
    }
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}


/***************************************************************************
 * RvSipPubClientSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the pub-client call-id. If the call-id is not set the pub-client
 *          will use the pubClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The pub-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The Sip Stack handle to the pub-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetCallId (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvChar              *strCallId)
{
    RvStatus               rv;

    PubClient    *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = PubClientLockAPI(pPubClient); /*try to lock the pub-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pPubClient->eState != RVSIP_PUB_CLIENT_STATE_IDLE)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetCallId - pub-client 0x%p is not in state Idle", pPubClient));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
	rv = SipTranscClientSetCallId(&pPubClient->transcClient, strCallId);

	PubClientUnLockAPI(pPubClient);
    return rv;

}

/***************************************************************************
 * RvSipPubClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the pub-client Call-Id.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the pub-client.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The pub-client does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient   - The Sip Stack handle to the pub-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCallId (
                            IN  RvSipPubClientHandle   hPubClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus               rv;
    PubClient    *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = PubClientLockAPI(pPubClient); /*try to lock the pub-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCallId(&pPubClient->transcClient, bufSize, pstrCallId, actualSize);
    
	PubClientUnLockAPI(pPubClient);
    return rv;


}


/***************************************************************************
 * RvSipPubClientSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the from header associated with the publish-client. If the
 *          From header was constructed by the
 *          RvSipPubClientGetNewPartyHeaderHandle function, the From header
 *          is attached to the publish-client object.
 *          Otherwise it will be copied by this function. Note
 *          that attempting to alter the from address after Publish has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid
 *                                   or the From header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hFrom      - Handle to an application constructed an initialized
 *                       from header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetFromHeader (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvSipPartyHeaderHandle hFrom)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetFromHeader - Setting Publish-Client 0x%p From header",
              pPubClient));

    retStatus = SipTranscClientSetFromHeader(&pPubClient->transcClient, hFrom);
	PubClientUnLockAPI(pPubClient);
    return retStatus;
}


/***************************************************************************
 * RvSipPubClientGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the publish-client.
 *          Attempting to alter the from address after Pubitration has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 * Output:     phFrom     - Pointer to the publish-client From header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetFromHeader (
                                      IN  RvSipPubClientHandle    hPubClient,
                                      OUT RvSipPartyHeaderHandle *phFrom)
{
    PubClient          *pPubClient;
	RvStatus			rv;


    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetFromHeader(&pPubClient->transcClient, phFrom);
	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the publish-client. If the To
 *          header was constructed by the RvSipPubClientGetNewMsgElementHandle
 *          function, the To header will be attached to the publish-client
 *          object.
 *          Otherwise the To header is copied by this function. Note
 *          that attempting to alter the To header after publication has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle or the To
 *                                   header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hTo        - Handle to an application constructed and initialized
 *                       To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetToHeader (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvSipPartyHeaderHandle hTo)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetToHeader - Setting Publish-Client 0x%p To header",
              pPubClient));

	retStatus = SipTranscClientSetToHeader(&pPubClient->transcClient, hTo);

	PubClientUnLockAPI(pPubClient);
    return retStatus;
}

/***************************************************************************
 * RvSipPubClientGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To header associated with the publish-client.
 *          Attempting to alter the To header after publication has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 * Output:     phTo       - Pointer to the publish-client To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetToHeader (
                                      IN    RvSipPubClientHandle    hPubClient,
                                      OUT   RvSipPartyHeaderHandle *phTo)
{
    PubClient          *pPubClient;
	RvStatus			rv;


    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetToHeader(&pPubClient->transcClient, phTo);
	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientGetRetryAfterInterval
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the Retry-after interval found in the last rejected response.
 *	If the retry-after is not valid, meaning that it was not appear in the last response,
 *	the return value is 0.
 *
 * Arguments:
 * Input:     hPubClient			- Handle to the publish-client.
 * Output:    pRetryAfterInterval	- The publishClient retryAfter interval.
 * 
 * Return Value: RV_OK  - If the interval value returned successfully.
 *               else error code is return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetRetryAfterInterval (
                                      IN    RvSipPubClientHandle    hPubClient,
                                      OUT   RvUint32				*pRetryAfterInterval)
{
	PubClient          *pPubClient;
		
    if (NULL == hPubClient)
    {
        return RV_ERROR_NULLPTR;
    }
    
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_UNKNOWN;
    }

	*pRetryAfterInterval = pPubClient->retryAfter;

	PubClientUnLockAPI(pPubClient);

	return RV_OK;


}

/***************************************************************************
 * SipPubClientGetSipIfMatchHeader
 * ------------------------------------------------------------------------
 * General: Returns the SIP-IF-MATCH header associated with the pub-client.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the pub-client.
 * Output:     phSipIfMatch       - Pointer to the transaction-client To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSipIfMatchHeader (
                                      IN    RvSipPubClientHandle	hPubClient,
                                      OUT   RvSipOtherHeaderHandle  *phSipIfMatch)
{
	PubClient          *pPubClient;

    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == phSipIfMatch)
    {
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_NULLPTR;
    }
	
	*phSipIfMatch = pPubClient->hSipIfMatchHeader;
	
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientSetRequestURI
 * ------------------------------------------------------------------------
 * General: Set the SIP-Url of the publisher in the publish-client object.
 *          Before calling Publish(), your application should use
 *          this function to supply the SIP-Url of the publisher.
 *          The publish request will be sent to this SIP-Url.
 *          You can change the publisher's SIP-Url each time you call
 *          Publish().
 *          This ability can be used for updating the publisher's SIP-Url
 *          in case of redirections and refreshes.
 *          The publisher address must be a SIP-Url with no user name.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied address handle is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hRequestUri - Handle to the publisher SIP-Url to be set to the
 *                        publish-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetRequestURI (
                                 IN  RvSipPubClientHandle  hPubClient,
                                 IN  RvSipAddressHandle    hRequestUri)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetRequestURI - Setting Publish-Client 0x%p publisher Request-Url",
              pPubClient));
	retStatus = SipTranscClientSetDestination(&pPubClient->transcClient, hRequestUri);
	PubClientUnLockAPI(pPubClient);
    return retStatus;
}


/***************************************************************************
 * RvSipPubClientGetRequestURI
 * ------------------------------------------------------------------------
 * General: Gets the SIP-Url of the publisher associated with the
 *          publish-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The address object was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phRequestUri    - Handle to the publisher SIP-Url.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetRequestURI (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipAddressHandle       *phRequestUri)
{
    PubClient          *pPubClient;
	RvStatus			rv;

    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetDestination(&pPubClient->transcClient, phRequestUri);
	PubClientUnLockAPI(pPubClient);
    return rv;
}


/***************************************************************************
 * RvSipPubClientGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the publish client. You can
 *          call this function from the RvSipPubClientStateChangedEv call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phMsg           - pointer to the received message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetReceivedMsg(
                                            IN  RvSipPubClientHandle  hPubClient,
                                             OUT RvSipMsgHandle          *phMsg)
{
    RvStatus               rv;

    PubClient              *pPubClient = (PubClient *)hPubClient;

    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }


    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetReceivedMsg(&pPubClient->transcClient, phMsg);

	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the publish-client.
 *          You can call this function before you call API functions such as
 *          Publish(), Make() and Authenticate().
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:    phMsg           - pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientGetOutboundMsg(
                                     IN  RvSipPubClientHandle  hPubClient,
                                     OUT RvSipMsgHandle       *phMsg)
{
    PubClient* pPubClient = (PubClient *)hPubClient;
    RvStatus   rv;

    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetOutboundMsg(&pPubClient->transcClient, phMsg);

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientGetOutboundMsg - publish-client= 0x%p returned handle= 0x%p",
              pPubClient, *phMsg));

	PubClientUnLockAPI(pPubClient);
    return rv;
}


/***************************************************************************
 * RvSipPubClientResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the publish-client to NULL. If the
 *          publish-client is about to send a message it will create its own
 *          message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientResetOutboundMsg(
                                     IN  RvSipPubClientHandle  hPubClient)
{
    PubClient              *pPubClient = (PubClient *)hPubClient;
    RvStatus              rv;

    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientResetOutboundMsg(&pPubClient->transcClient);

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientResetOutboundMsg - publish-client= 0x%p resetting outbound message",
              pPubClient));

	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}


/***************************************************************************
 * RvSipPubClientSetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Set an Expires header in the publish-client object. Before
 *          calling Publish(), the application can use this function to
 *          supply the required Expires header to use in the Publish
 *          request. This Expires header is inserted to the Publish
 *          request message before it is sent to the publisher.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Expires header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Expires header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hExpiresHeader - Handle to an Expires header to be set to the
 *                           publish-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetExpiresHeader (
                                 IN  RvSipPubClientHandle     hPubClient,
                                 IN  RvSipExpiresHeaderHandle hExpiresHeader)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetExpiresHeader - Setting Publish-Client 0x%p Expires header",
              pPubClient));

	retStatus = SipTranscClientSetExpiresHeader(&pPubClient->transcClient, hExpiresHeader);
	PubClientUnLockAPI(pPubClient);
    return retStatus;
}


/***************************************************************************
 * RvSipPubClientGetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Gets the Expires header associated with the publish-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The Expires header was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phExpiresHeader - Handle to the publish-client's Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetExpiresHeader (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipExpiresHeaderHandle *phExpiresHeader)
{
    PubClient          *pPubClient;
	RvStatus			rv;


    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetExpiresHeader(&pPubClient->transcClient, phExpiresHeader);

	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientGetEventHeader
 * ------------------------------------------------------------------------
 * General: Gets the Event header associated with the publish-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The Event header was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phEventHeader - Handle to the publish-client's Event header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetEventHeader (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipEventHeaderHandle *phEventHeader)
{
    PubClient          *pPubClient;

    if (NULL == hPubClient)
    {
        return RV_ERROR_BADPARAM;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == phEventHeader)
    {
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_NULLPTR;
    }
    *phEventHeader = pPubClient->hEventHeader;

	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientSetEventHeader
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
RVAPI RvStatus RVCALLCONV RvSipPubClientSetEventHeader (
                                 IN  RvSipPubClientHandle     hPubClient,
                                 IN  RvSipEventHeaderHandle hEventHeader)
{
    PubClient          *pPubClient;
    RvStatus           retStatus;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetEventHeader - Setting Publish-Client 0x%p Event header",
              pPubClient));

	retStatus = PubClientSetEventHeader(pPubClient, hEventHeader);

	PubClientUnLockAPI(pPubClient);
    return retStatus;
}

/***************************************************************************
 * RvSipPubClientGetCurrentState
 * ------------------------------------------------------------------------
 * General: Returns the current state of the publish-client.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given publish-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 * Output:     peState -  The publish-client's current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCurrentState (
                                      IN  RvSipPubClientHandle hPubClient,
                                      OUT RvSipPubClientState *peState)
{
    PubClient          *pPubClient;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == peState)
    {
        return RV_ERROR_NULLPTR;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peState = pPubClient->eState;
	PubClientUnLockAPI(pPubClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Returns the CSeq-Step associated with the publish-client.
 *          Note: The CSeq-Step is valid only after a publication request
 *                was successfully executed. Otherwise the CSeq-Step is 0.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given publish-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 * Output:     pCSeqStep -  The publish-client's CSeq-Step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCSeqStep (
                                      IN  RvSipPubClientHandle hPubClient,
                                      OUT RvUint32            *pCSeqStep)
{
	RvStatus	rv;
    PubClient  *pPubClient;


    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCSeqStep(&pPubClient->transcClient, pCSeqStep);
	
	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this pub-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClientMgr   - Handle to the publish-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetStackInstance(
                                   IN   RvSipPubClientHandle   hPubClient,
                                   OUT  void                 **phStackInstance)
{
    PubClient    *pPubClient;
	RvStatus	  rv;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pPubClient    = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
		
	rv = SipTranscClientGetStackInstance(&pPubClient->transcClient, phStackInstance);

    PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the Pub-Client will send outgoing
 *          requests.
 *          The stack can be configured to listen to many local addresses.
 *          Each local address has a transport type (UDP/TCP/TLS) and an address type
 *          (IPv4/IPv6). When the stack sends an outgoing request, the local address
 *          (from where the request is sent) is chosen according to the characteristics
 *          of the remote address. Both the local and remote addresses must have
 *          the same characteristics i.e. the same transport type and address type.
 *          If several configured local addresses
 *          match the remote address characteristics, the first configured address is taken.
 *          You can use RvSipPubClientSetLocalAddress() to force the Pub-Client to choose
 *          a specific local address for a specific transport and address type.
 *          For example, you can force the Pub-Client to use the second configured UDP/IPv4
 *          local address. If the Pub-Client will send a request to an
 *          UDP/IPv4 remote address it will use the local address that you set instead
 *          of the default first local address.
 *
 *          Note: The localAddress string you provide this function must match exactly
 *          to the local address that was inserted in the configuration structure in the
 *          initialization of the sip stack.
 *          If you configured the stack to listen to a 0.0.0.0 local address you must use
 *          the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - The publish-client handle.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType   - The address type(IPv4 or IPv6).
 *            strLocalIPAddress   - A string with the local address to be set to this pub-client.
 *          localPort      - The local port to be set to this pub-client. If you set
 *                           the local port to 0, you will get a default value of 5060.
 *          if_name - local interface name
 *          vdevblock - hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetLocalAddress(
                            IN  RvSipPubClientHandle      hPubClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *strLocalIPAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                 *if_name
			    , IN  RvUint16               vdevblock
#endif
			    )
{
    RvStatus    rv;
    PubClient    *pPubClient = (PubClient*)hPubClient;
    if (pPubClient == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    /*try to lock the pub-client*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetLocalAddress(&pPubClient->transcClient, eTransportType, eAddressType, strLocalIPAddress, localPort
#if defined(UPDATED_BY_SPIRENT)
					    ,if_name
					    ,vdevblock
#endif
					    );
	
	PubClientUnLockAPI(pPubClient);
    return rv;
}


/***************************************************************************
 * RvSipPubClientGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the pub-client will use to send outgoing
 *          requests. This is the address the user set using the
 *          RvSipPubClientSetLocalAddress function. If no address was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - The publish-client handle
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:    localAddress    - The local address this pub-client is using
 *                            (must be longer than 48).
 *          localPort       - The local port this pub-client is using.
 * if_name - local interface name
 * vdevblock - hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetLocalAddress(
                            IN  RvSipPubClientHandle      hPubClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *localAddress,
                            OUT  RvUint16                *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    )
{
    PubClient                 *pPubClient;
    RvStatus                   rv;

    pPubClient = (PubClient *)hPubClient;


    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientGetLocalAddress - PubClient 0x%p - Getting local %s,%saddress",pPubClient,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType)));


	rv = SipTranscClientGetLocalAddress(&pPubClient->transcClient, eTransportType, eAddressType, localAddress, localPort
#if defined(UPDATED_BY_SPIRENT)
					    ,if_name
					    ,vdevblock
#endif
					    );
	PubClientUnLockAPI(pPubClient);
    return rv;
}


/***************************************************************************
 * RvSipPubClientSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the publish-client object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameters such as the IP address or host name,
 *          transport, port and compression type.
 *          Requests sent by this object will use the outbound details
 *          specifications as a remote address.
 *          Note: If you specify both the IP address and a host name in the
 *          configuration structure, either of them will be set but the IP
 *          address will be used. If you do not specify port or transport,
 *          both will be determined according to the DNS
 *          procedures specified in RFC 3263.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - Handle to the publish-client.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetOutboundDetails(
                            IN  RvSipPubClientHandle            hPubClient,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg)
{
    RvStatus   rv = RV_OK;
    PubClient  *pPubClient;

    pPubClient = (PubClient *)hPubClient;

    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetOutboundDetails(&pPubClient->transcClient, pOutboundCfg, sizeOfCfg);

	PubClientUnLockAPI(pPubClient);

    return rv;
}

/***************************************************************************
 * RvSipPubClientGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the publish-client object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type. If the outbound details were not set to the specific
 *          publish-client object, but the outbound proxy was defined to the SIP
 *          Stack on initialization, the SIP Stack parameters will be returned.
 *          If the publish-client is not using an outbound address, NULL/UNDEFINED
 *          values are returned.
 *          Note: You must supply a valid consecutive buffer in the
 *                RvSipTransportOutboundProxyCfg structure to get the outbound strings
 *               (host name and IP address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - Handle to the publish-client.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetOutboundDetails(
                            IN  RvSipPubClientHandle            hPubClient,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg)
{
    PubClient   *pPubClient;
    RvStatus    rv;


    pPubClient = (PubClient*)hPubClient;

    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetOutboundDetails(&pPubClient->transcClient, sizeOfCfg, pOutboundCfg);
	
	PubClientUnLockAPI(pPubClient);

    return rv;
}

/***************************************************************************
 * RvSipPubClientSetPersistency
 * ------------------------------------------------------------------------
 * General: instructs the Pub-Client on whether to try and use persistent connection
 *          or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hPubClient - The Pub-Client handle
 *          bIsPersistent - Determines whether the Pub-Client will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetPersistency(
                           IN RvSipPubClientHandle       hPubClient,
                           IN RvBool                  bIsPersistent)
{
    RvStatus  rv;

    PubClient             *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = PubClientLockAPI(pPubClient); /*try to lock the pub client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientSetPersistency(&pPubClient->transcClient, bIsPersistent);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
/***************************************************************************
 * RvSipPubClientGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns whether the Pub-Client is configured to try and use a
 *          persistent connection or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hPubClient - The Pub-Client handle
 * Output:  pbIsPersistent - Determines whether the Pub-Client uses
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientGetPersistency(
        IN  RvSipPubClientHandle                   hPubClient,
        OUT RvBool                             *pbIsPersistent)
{

    PubClient        *pPubClient;
	RvStatus			rv;


    pPubClient = (PubClient *)hPubClient;

    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetPersistency(&pPubClient->transcClient, pbIsPersistent);
	
	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientSetConnection
 * ------------------------------------------------------------------------
 * General: Set a connection to be used by the PubClient. The PubClient will
 *          supply this connection to all the transactions its creates.
 *          If the connection will not fit the transaction local and remote
 *          addresses it will be
 *          replaced. And the Pub-Client will assume the new connection.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransaction - Handle to the transaction
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetConnection(
                   IN  RvSipPubClientHandle                hPubClient,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    RvStatus               rv;
    PubClient        *pPubClient= (PubClient *)hPubClient;
    
	
    if (NULL == pPubClient)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientSetConnection(&pPubClient->transcClient, hConn);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
/***************************************************************************
 * RvSipPubClientGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Pub-Client.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClienthTransc - Handle to the Pub-Client.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetConnection(
                            IN  RvSipPubClientHandle             hPubClient,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    PubClient                 *pPubClient = (PubClient*)hPubClient;
	RvStatus				   rv;

    if (NULL == pPubClient || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phConn = NULL;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = SipTranscClientGetConnection(&pPubClient->transcClient, phConn);

	PubClientUnLockAPI(pPubClient);
    return rv;

}

/***************************************************************************
* RvSipPubClientSetTranscTimers
* ------------------------------------------------------------------------
* General: Sets timeout values for the PubClient's transactions timers.
*          If some of the fields in pTimers are not set (UNDEFINED), this
*          function will calculate it, or take the values from configuration.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    Input: hPubClient - Handle to the PubClient object.
*           pTimers - Pointer to the struct contains all the timeout values.
*           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetTranscTimers(
                            IN  RvSipPubClientHandle  hPubClient,
                            IN  RvSipTimers           *pTimers,
                            IN  RvInt32               sizeOfTimersStruct)
{
    PubClient  *pPubClient = (PubClient*)hPubClient;
    RvStatus   rv;

    if (NULL == pPubClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetTranscTimers - pPubClient 0x%p - setting timers",
              pPubClient));

    rv = SipTranscClientSetTranscTimers(&pPubClient->transcClient, pTimers, sizeOfTimersStruct);

	PubClientUnLockAPI(pPubClient);

    return rv;

}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipPubClientSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message,
 *          belonging to the PubClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hPubClient - Handle to the PubClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - Pointer to the struct that contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSctpMsgSendingParams(
                    IN  RvSipPubClientHandle               hPubClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    PubClient  *pPubClient = (PubClient*)hPubClient;
    
    if(pPubClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientSetSctpMsgSendingParams - pub-client 0x%p - failed to lock the PubClient",
            pPubClient));
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetSctpMsgSendingParams(&pPubClient->transcClient, sizeOfParamsStruct, pParams);
    
	PubClientUnLockAPI(pPubClient);
    return rv;
}

/******************************************************************************
 * RvSipPubClientGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message, belonging
 *          to the PubClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hPubClient - Handle to the PubClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSctpMsgSendingParams(
                    IN  RvSipPubClientHandle               hPubClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    PubClient  *pPubClient = (PubClient*)hPubClient;

    if(pPubClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientGetSctpMsgSendingParams - pub-client 0x%p - failed to lock the PubClient",
            pPubClient));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientGetSctpMsgSendingParams(&pPubClient->transcClient, sizeOfParamsStruct, pParams);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipPubClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Pub-Client.
 *          As a result of this operation, all messages, sent by this
 *          Pub-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hPubClient - Handle to the Pub-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSecObj(
                                        IN  RvSipPubClientHandle    hPubClient,
                                        IN  RvSipSecObjHandle       hSecObj)
{
    RvStatus   rv;
    PubClient* pPubClient = (PubClient*)hPubClient;

    if(NULL == pPubClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientSetSecObj - Pub-Client 0x%p - Set Security Object %p",
        pPubClient, hSecObj));

    /*try to lock the Pub-Client*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientSetSecObj - Pub-Client 0x%p - failed to lock the PubClient",
            pPubClient));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientSetSecObj(&pPubClient->transcClient, hSecObj);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipPubClientGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Pub-Client.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hPubClient - Handle to the Pub-Client object.
 *  Output: phSecObj   - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSecObj(
                                        IN  RvSipPubClientHandle    hPubClient,
                                        OUT RvSipSecObjHandle*      phSecObj)
{
    RvStatus   rv;
    PubClient* pPubClient = (PubClient*)hPubClient;

    if(NULL==pPubClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientGetSecObj - Pub-Client 0x%p - Get Security Object",
        pPubClient));

    /*try to lock the Pub-Client*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientGetSecObj - Pub-Client 0x%p - failed to lock the PubClient",
            pPubClient));
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetSecObj(&pPubClient->transcClient, phSecObj);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * RvSipPubClientSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the pubClient application handle. Usually the application
 *          replaces handles with the stack in the
 *          RvSipPubClientMgrCreatePubClient() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient    - Handle to the pubClient.
 *            hAppPubClient - A new application handle to the pubClient.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetAppHandle (
                                      IN  RvSipPubClientHandle     hPubClient,
                                      IN  RvSipAppPubClientHandle  hAppPubClient)
{
    RvStatus               rv;
    PubClient   *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the pubClient*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
              "RvSipPubClientSetAppHandle - Setting pubClient 0x%p Application handle 0x%p",
              pPubClient,hAppPubClient));

    pPubClient->hAppPubClient = hAppPubClient;

	PubClientUnLockAPI(pPubClient);

    return RV_OK;
}

/***************************************************************************
 * RvSipPubClientGetAppHandle
 * ------------------------------------------------------------------------
 * General: Gets the pubClient application handle. Usually the application
 *          replaces handles with the stack in the
 *          RvSipPubClientMgrCreatePubClient() API function.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient    - Handle to the pubClient.
 *            hAppPubClient - A new application handle to the pubClient.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetAppHandle (
                                      IN  RvSipPubClientHandle     hPubClient,
                                      IN  RvSipAppPubClientHandle *phAppPubClient)
{
    RvStatus      rv;
    PubClient    *pPubClient = (PubClient*)hPubClient;

    if(pPubClient == NULL || phAppPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the pubClient*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phAppPubClient = pPubClient->hAppPubClient;

	PubClientUnLockAPI(pPubClient);

    return rv;
}

/***************************************************************************
 * RvSipPubClientSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the publish-client to a SigComp compartment.
 *          The publish-client will use this compartment in the message
 *          compression process.
 *          Note The message will be compressed only if the remote URI includes the
 *          comp=sigcomp parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient   - The handle to the publish client.
 *            hCompartment - Handle to the SigComp Compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetCompartment(
                            IN RvSipPubClientHandle   hPubClient,
                            IN RvSipCompartmentHandle hCompartment)
{
    RV_UNUSED_ARG(hPubClient);
    RV_UNUSED_ARG(hCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipPubClientGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the SigComp compartment associated with the publish-client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hPubClient    - The handle to the publish client.
 * Output: phCompartment - The handle to the SigComp compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCompartment(
                            IN  RvSipPubClientHandle    hPubClient,
                            OUT RvSipCompartmentHandle *phCompartment)
{
    *phCompartment = NULL;

    RV_UNUSED_ARG(hPubClient) 

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipPubClientDisableCompression
 * ------------------------------------------------------------------------
 * General:Disables the compression mechanism in a publish-client.
 *         This means that even if the message indicates compression,
 *         it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hPubClient    - The handle to the publish client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDisableCompression(
                                      IN  RvSipPubClientHandle hPubClient)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv          = RV_OK;
    PubClient   *pPubClient = (PubClient*)hPubClient;

    if(pPubClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = PubClientLockAPI(pPubClient); /* try to lock the pubClient */
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientDisableCompression(&pPubClient->transcClient);

	PubClientUnLockAPI(pPubClient);

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hPubClient);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

#ifdef RV_SIP_AUTH_ON


/***************************************************************************
 * RvSipPubClientGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: The function retrieve the auth-object that is currently being
 *          processed by the authenticator.
 *          (for application usage in the RvSipAuthenticatorGetSharedSecretEv
 *          callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient          - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCurrProcessedAuthObj (
                                      IN   RvSipPubClientHandle    hPubClient,
                                      OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    PubClient     *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCurrProcessedAuthObj(&pPubClient->transcClient, phAuthObj);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
/***************************************************************************
 * RvSipPubClientAuthObjGet
 * ------------------------------------------------------------------------
 * General: The function retrieve auth-objects from the list in the Pub-Client.
 *          you may get the first/last/next object.
 *          in case of getting the next object, please supply the current
 *          object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient        - Handle to the Pub-Client.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientAuthObjGet (
                                      IN   RvSipPubClientHandle     hPubClient,
                                      IN   RvSipListLocation      eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    PubClient     *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the Pub-Client*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientAuthObjGet(&pPubClient->transcClient, eLocation, hRelativeAuthObj, phAuthObj);

	PubClientUnLockAPI(pPubClient);
    return rv;
}

/***************************************************************************
 * RvSipPubClientAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an auth-obj from the list in the Pub-Client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient  - Handle to the Pub-Client.
 *            hAuthObj    - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientAuthObjRemove (
                                      IN   RvSipPubClientHandle   hPubClient,
                                      IN   RvSipAuthObjHandle   hAuthObj)
{
    RvStatus     rv;
    PubClient     *pPubClient = (PubClient*)hPubClient;
    if(pPubClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the Pub-Client*/
    rv = PubClientLockAPI(pPubClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientAuthObjRemove(&pPubClient->transcClient, hAuthObj);

	PubClientUnLockAPI(pPubClient);
    return rv;
}
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipPubClientSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header in the outgoing request.
 *          An initial authorization header is a header that contains
 *          only the client private identity, and not real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The pub-client will set the initial header to the message, only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - The publish-client handle.
 *            hAuthorization - The Authorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetInitialAuthorization (
                                         IN RvSipPubClientHandle hPubClient,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization)
{
    PubClient          *pPubClient;
    RvStatus            retStatus;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPubClient = (PubClient *)hPubClient;
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
        "RvSipPubClientSetInitialAuthorization - Setting in client 0x%p ",pPubClient));

    /* Assert that the current state allows this action */
    if (RVSIP_PUB_CLIENT_STATE_IDLE != pPubClient->eState &&
        RVSIP_PUB_CLIENT_STATE_UNAUTHENTICATED != pPubClient->eState)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "RvSipPubClientSetInitialAuthorization - Publish client 0x%p - illegal state %s",
            pPubClient, PubClientStateIntoString(pPubClient->eState)));
		PubClientUnLockAPI(pPubClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

	retStatus = SipTranscClientSetInitialAuthorization(&pPubClient->transcClient, hAuthorization);
    
	PubClientUnLockAPI(pPubClient);
    return retStatus;
}

/***************************************************************************
 * RvSipPubClientSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this pub-client. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the pub-client will exploit this agreement and the data
 *          it brings. If not, the pub-client will use this security-agreement
 *          object to negotiate an agreement with the remote party.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClient   - Handle to the pub-client.
 *          hSecAgree    - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSecAgree(
							IN  RvSipPubClientHandle         hPubClient,
							IN  RvSipSecAgreeHandle          hSecAgree)
{
	PubClient        *pPubClient;
	RvStatus          rv;

    if (hPubClient == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pPubClient = (PubClient *)hPubClient;

	if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* if the pub-client was terminated we do not allow the attachment */
	if (RVSIP_PUB_CLIENT_STATE_TERMINATED == pPubClient->eState)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
					"RvSipPubClientSetSecAgree - pub-client 0x%p - Attaching security-agreement 0x%p is forbidden in state Terminated",
					pPubClient, hSecAgree));
		PubClientUnLockAPI(pPubClient);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SipTranscClientSetSecAgree(&pPubClient->transcClient, hSecAgree);

	PubClientUnLockAPI(pPubClient);

	return rv;
}

/***************************************************************************
 * RvSipPubClientGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the pub-client.
 *          The security-agreement object captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClient    - Handle to the pub-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSecAgree(
									IN   RvSipPubClientHandle    hPubClient,
									OUT  RvSipSecAgreeHandle    *phSecAgree)
{
	PubClient          *pPubClient = (PubClient *)hPubClient;
	RvStatus			rv;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    if (RV_OK != PubClientLockAPI(pPubClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetSecAgree(&pPubClient->transcClient, phSecAgree);

	PubClientUnLockAPI(pPubClient);

	return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */
#endif /* RV_SIP_PRIMITIVES */






















