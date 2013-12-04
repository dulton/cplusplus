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
 *                              <RvSipRegClient.c>
 *
 * This file defines the RegClient API functions.
 * The API contains two major parts:
 * 1. The register-client manager API: The register-client manager is incharged
 *                                     of all the register-clients. It is used
 *                                     to set the event handlers of the
 *                                     register-client module and to create new
 *                                     register-client.
 * 2. The register-client API: Using the register-client API the user can
 *                             request to Register at a chosen registrar. It
 *                             can redirect a register request and authenticate
 *                             a register request when needed.
 *                             A RegClient is a stateful object and has a set
 *                             of states associated with it.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *    Gilad Govrin                  Sep 2006
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipRegClientMgr.h"
#include "RegClientMgrObject.h"
#include "RegClientObject.h"
#include "RvSipRegClient.h"
#include "RegClientCallbacks.h"

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

static RvStatus RVCALLCONV RegClientGetNewContactHeaderHandle (
                               IN  RegClient                 *pRegClient,
                               OUT RvSipContactHeaderHandle **phContactHeader);

static RvStatus RVCALLCONV RegClientCopyContactHeader (
                           IN  RegClient                *pRegClient,
                           IN  RvSipContactHeaderHandle *phDestContactHeader,
                           IN  RvSipContactHeaderHandle  hSourceHeader);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RvSipRegClientMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all register-client events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the event handler structure.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the register-client manager.
 *            pEvHandlers - Pointer to the application event handler structure
 *            structSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMgrSetEvHandlers(
                                   IN  RvSipRegClientMgrHandle   hMgr,
                                   IN  RvSipRegClientEvHandlers *pEvHandlers,
                                   IN  RvInt32                  structSize)
{
    RegClientMgr       *pRegClientMgr;

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
    pRegClientMgr = (RegClientMgr *)hMgr;
    RegClientMgrLock(pRegClientMgr);
    /* Copy the event handlers struct according to the given size */
    memset(&(pRegClientMgr->regClientEvHandlers), 0,
                                            sizeof(RvSipRegClientEvHandlers));
    memcpy(&(pRegClientMgr->regClientEvHandlers), pEvHandlers, structSize);
	
	if (pRegClientMgr->regClientEvHandlers.pfnObjExpiredEvHandler != NULL)
	{
		pRegClientMgr->bHandleExpTimer = RV_TRUE;
	}

    RegClientMgrUnLock(pRegClientMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipRegClientMgrSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the Call-Id to the register-client manager. The string is be
 *          copied. Note: the Call-Id is a null terminated string.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the Call-Id string.
 *              RV_ERROR_OUTOFRESOURCES - Not enough resources to copy the Call-Id.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the register-client manager.
 *            strCallId - The Call-Id string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMgrSetCallId(
                                   IN  RvSipRegClientMgrHandle   hMgr,
                                   IN  RvChar                  *strCallId)
{
    RegClientMgr *pRegClientMgr;
    RvUint32     callIdSize;
    RvStatus     retStatus;
    RvInt32     offset;

    if (NULL == hMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == strCallId)
    {
        return RV_ERROR_NULLPTR;
    }
    pRegClientMgr = (RegClientMgr *)hMgr;
    callIdSize = (RvUint32)strlen(strCallId);
    RegClientMgrLock(pRegClientMgr);
    if (NULL_PAGE == pRegClientMgr->hMemoryPage)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                   "RvSipRegClientMgrSetCallId - trying to set Call-Id to a manager with zero register-clients"));
        RegClientMgrUnLock(pRegClientMgr);
        return RV_ERROR_UNKNOWN;
    }


	retStatus = RPOOL_AppendFromExternal(pRegClientMgr->hGeneralMemPool,
		                                 pRegClientMgr->hMemoryPage, strCallId, callIdSize+1,
                                         RV_FALSE, &offset,NULL);
    if(RV_OK != retStatus)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                   "RvSipRegClientMgrSetCallId - pRegClientMgr 0x%p Failed in RPOOL_AppendFromExternal (retStatus=%d)",
				   pRegClientMgr,retStatus));
        RegClientMgrUnLock(pRegClientMgr);
        return retStatus;
    }

    /* Get the offset of the new allocated memory */
    pRegClientMgr->strCallId = offset;
#ifdef SIP_DEBUG
    pRegClientMgr->pCallId = RPOOL_GetPtr(pRegClientMgr->hGeneralMemPool,
                                          pRegClientMgr->hMemoryPage,
                                          pRegClientMgr->strCallId);
#endif
    RegClientMgrUnLock(pRegClientMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipRegClientMgrGetCallId
 * ------------------------------------------------------------------------
 * General: Gets the Call-Id of the register-client manager. The Call-Id
 *          string will be copied to strCallId. If strCallId is not
 *          adequate the function returns RV_InsuffitientBuffer.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the Call-Id string.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The given buffer is not large enough.
 *              RV_OK       - Success.
 *              RV_ERROR_UNKNOWN       - There is no Call-Id for this manager.
 *                                 This is an exception.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the register-client manager.
 *            strCallId - The application buffer. The Call-Id string is
 *                      copied to this buffer.
 *            bufferSize - The size of the strCallId buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMgrGetCallId(
                                   IN  RvSipRegClientMgrHandle   hMgr,
                                   IN  RvChar                  *strCallId,
                                   IN  RvUint32                 bufferSize)
{
    RvUint32     callIdSize;
    RegClientMgr *pRegClientMgr;
    RvStatus     retStatus;

    if (NULL == hMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == strCallId)
    {
        return RV_ERROR_NULLPTR;
    }
    pRegClientMgr = (RegClientMgr *)hMgr;
    RegClientMgrLock(pRegClientMgr);
    if ((UNDEFINED == pRegClientMgr->strCallId) ||
        (NULL_PAGE == pRegClientMgr->hMemoryPage))
    {
        RegClientMgrUnLock(pRegClientMgr);
        return RV_ERROR_UNKNOWN;
    }
    callIdSize = RPOOL_Strlen(pRegClientMgr->hGeneralMemPool,
                              pRegClientMgr->hMemoryPage,
                              pRegClientMgr->strCallId) + 1;
    if (bufferSize < callIdSize)
    {
        RegClientMgrUnLock(pRegClientMgr);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    retStatus = RPOOL_CopyToExternal(pRegClientMgr->hGeneralMemPool,
                                     pRegClientMgr->hMemoryPage,
                                     pRegClientMgr->strCallId, strCallId,
                                     callIdSize);
    RegClientMgrUnLock(pRegClientMgr);
    return retStatus;
}


/***************************************************************************
 * RvSipRegClientMgrCreateRegClient
 * ------------------------------------------------------------------------
 * General: Creates a new register-client and replace handles with the
 *          application.  The new register-client assumes the idle state.
 *          In order to register to a registrar you should:
 *          1. Create a new RegClient with this function.
 *          2. Set at least the To and From headers.
 *          3. Call the Register() function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the register-client
 *                                   handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create a new
 *                                   RegClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClientMgr - Handle to the register-client manager
 *            hAppRegClient - Application handle to the newly created register-
 *                          client.
 * Output:     hRegClient -    sip stack handle to the register-client
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMgrCreateRegClient(
                                   IN  RvSipRegClientMgrHandle hRegClientMgr,
                                   IN  RvSipAppRegClientHandle hAppRegClient,
                                   OUT RvSipRegClientHandle   *phRegClient)
{
    RegClientMgr       *pRegClientMgr;
    RvStatus           retStatus;

    if (NULL == hRegClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == phRegClient)
    {
        return RV_ERROR_NULLPTR;
    }
    pRegClientMgr = (RegClientMgr *)hRegClientMgr;
    RegClientMgrLock(pRegClientMgr);
    RvLogDebug(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
            "RvSipRegClientMgrCreateRegClient - Starting to create reg client"));

    /* Insert the new register-client object to the end of the list */
    if (NULL == pRegClientMgr->hRegClientList)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "RvSipRegClientMgrCreateRegClient - Error - Failed to allocate new register-client object"));
        RegClientMgrUnLock(pRegClientMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    retStatus = RLIST_InsertTail(pRegClientMgr->hRegClientListPool,
                                 pRegClientMgr->hRegClientList,
                                 (RLIST_ITEM_HANDLE *)phRegClient);
    if (RV_OK != retStatus)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
                  "RvSipRegClientMgrCreateRegClient - Error - Failed to allocate new register-client object"));
        RegClientMgrUnLock(pRegClientMgr);
        return retStatus;
    }
    /* Initiate the register-client object */
    retStatus = RegClientInitiate(pRegClientMgr, (RegClient *)(*phRegClient),
                                  hAppRegClient);
    if (RV_OK != retStatus)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
              "RvSipRegClientMgrCreateRegClient - Register client 0x%p: Error initiating a register-client object",
              *phRegClient));
        RLIST_Remove(pRegClientMgr->hRegClientListPool,
                     pRegClientMgr->hRegClientList,
                     (RLIST_ITEM_HANDLE)(*phRegClient));
        RegClientMgrUnLock(pRegClientMgr);
        return retStatus;
    }
    RvLogInfo(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
              "RvSipRegClientMgrCreateRegClient - Register client 0x%p: was successfully created",
              *phRegClient));
    RegClientMgrUnLock(pRegClientMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipRegClientMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this reg-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClientMgr   - Handle to the register-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMgrGetStackInstance(
                                   IN   RvSipRegClientMgrHandle   hRegClientMgr,
                                   OUT  void*         *phStackInstance)
{
    RegClientMgr *pRegClientMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == hRegClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClientMgr = (RegClientMgr *)hRegClientMgr;
    return SipTranscClientMgrGetStackInstance(pRegClientMgr->hTranscClientMgr, phStackInstance);
}

/***************************************************************************
 * RvSipRegClientRegister
 * ------------------------------------------------------------------------
 * General: Sends a Register request to the registrar. The Request-Url,To,
 *          From , Expires and Contact headers that were set to the register
 *          client are inserted to the outgoing REGISTER request.
 *          The register-client will asume the Registering state and wait for
 *          a response from the server.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid register-client state for this
 *                                  action, or the To and From header have
 *                                  yet been set to the register-client object,
 *                                  or the registrar Request-Url has not
 *                                  yet been set to the register-client object.
 *               RV_ERROR_OUTOFRESOURCES - register-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientRegister (
                                         IN RvSipRegClientHandle hRegClient)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;
	RPOOL_Ptr		   srcCallIdPage;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientRegister - Register client 0x%p ",pRegClient));

    /* Assert that the current state allows this action */
    if ((RVSIP_REG_CLIENT_STATE_IDLE != pRegClient->eState)
        && (RVSIP_REG_CLIENT_STATE_REDIRECTED != pRegClient->eState)
        && (RVSIP_REG_CLIENT_STATE_REGISTERED != pRegClient->eState)
		&& (RVSIP_REG_CLIENT_STATE_FAILED != pRegClient->eState) 
		&& (RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED != pRegClient->eState))
    {
        RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
	if (pRegClient->bIsDetach == RV_FALSE)
	{
		RvInt32	currentCseq;
		/* if the reg client is not detached from its manager 
		 we need to use the mgr cseq and call-id, lock the manager to get the correct CSeq-Step*/
		RegClientMgrLock(pRegClient->pMgr);
		
		SipCommonCSeqPromoteStep(&pRegClient->pMgr->cseq);
		SipCommonCSeqGetStep(&pRegClient->pMgr->cseq, &currentCseq);
		SipTranscClientSetCSeqStep(&pRegClient->transcClient, currentCseq);
		
		srcCallIdPage.hPage = pRegClient->pMgr->hMemoryPage;
		srcCallIdPage.hPool = pRegClient->pMgr->hGeneralMemPool;
		srcCallIdPage.offset = pRegClient->pMgr->strCallId;

		retStatus = SipTranscClientSetCallIdFromPage(&pRegClient->transcClient, &srcCallIdPage);		
		if (retStatus != RV_OK)
		{
			RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
				"RvSipRegClientRegister - Register client 0x%p: was unable to get call-id from its manager.",
				pRegClient));
			RegClientMgrUnLock(pRegClient->pMgr);
			RegClientUnLockAPI(pRegClient);
			return retStatus;
		}
		RegClientMgrUnLock(pRegClient->pMgr);
	}
    
	
	retStatus = SipTranscClientActivate(&pRegClient->transcClient, SIP_TRANSACTION_METHOD_REGISTER, NULL);

	
	
    if (RV_OK != retStatus)
    {
        
		RegClientUnLockAPI(pRegClient);
        return retStatus;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientRegister - Register client 0x%p: REGISTER request was successfully sent",
               pRegClient));
	RegClientUnLockAPI(pRegClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientMake
 * ------------------------------------------------------------------------
 * General: Set the register client from,to,registrar,contact parameters that
 *          are given as strings to the regClient and sends a Register
 *          request to the registrar. If a null value is supplied as one of
 *          the arguments this parameter will not be set.
 *          The register-client will asume the Registering state and wait for
 *          a response from the server.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid register-client state for this
 *                                  action or there is a missing parameter that
 *                                  is needed for the registration process such as
 *                                  the request URI.
 *               RV_ERROR_OUTOFRESOURCES - register-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 *          strFrom    - the initiator of the registration request.
 *                       for example: "From: sip:user@home.com"
 *          strTo      - The registaring user.
 *                       "To: sip:bob@proxy.com"
 *          strRegistrar - The request URI of the registration request.This
 *                         is the proxy address.
 *                         for example: "sip:proxy.com"
 *          strContact - The location of the registaring user.
 *                       for example: "Contact: sip:bob@work.com"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientMake (
                                         IN RvSipRegClientHandle hRegClient,
                                         IN RvChar*             strFrom,
                                         IN RvChar*             strTo,
                                         IN RvChar*             strRegistrar,
                                         IN RvChar*             strContact)
{
    RvStatus rv;
    RvSipContactHeaderHandle hContact;  /*handle to a Contact header*/
    RegClient                *pRegClient;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientMake - Register client 0x%p: starting to register", pRegClient));


	/*Since the register client is handling the contact header by it self.*/
	rv = SipTranscClientSetActivateDetails(&pRegClient->transcClient, strFrom, strTo, strRegistrar, RV_FALSE);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientMake - Register client 0x%p: Failed to allocate from party header",
            pRegClient));
		RegClientUnLockAPI(pRegClient);
        return rv;
    }

    if(strContact != NULL)
    {
        rv = RvSipRegClientGetNewContactHeaderHandle(hRegClient,&hContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to allocate contact header",
                pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;

        }
        rv = RvSipContactHeaderParse(hContact,strContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to parse to contact - bad syntax",
                 pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;
        }
        rv = RvSipRegClientSetContactHeader(hRegClient,hContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to set contact",
                pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;
        }
    }

    rv = RvSipRegClientRegister(hRegClient);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientMake - Register client 0x%p: Failed to register",
            pRegClient));
		RegClientUnLockAPI(pRegClient);
        return rv;
    }
	RegClientUnLockAPI(pRegClient);
    return RV_OK;

}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV RvSipRegClientPrepare (
                                         IN RvSipRegClientHandle hRegClient,
                                         IN RvChar*             strFrom,
                                         IN RvChar*             strTo,
                                         IN RvChar*             strRegistrar,
                                         IN RvChar*             strContact)
{
    RvStatus rv;
    RvSipContactHeaderHandle hContact;  /*handle to a Contact header*/
    RegClient                *pRegClient;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientMake - Register client 0x%p: starting to register", pRegClient));


	/*Since the register client is handling the contact header by it self.*/
	rv = SipTranscClientSetActivateDetails(&pRegClient->transcClient, strFrom, strTo, strRegistrar, RV_FALSE);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientMake - Register client 0x%p: Failed to allocate from party header",
            pRegClient));
		RegClientUnLockAPI(pRegClient);
        return rv;
    }

    if(strContact != NULL)
    {
        rv = RvSipRegClientGetNewContactHeaderHandle(hRegClient,&hContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to allocate contact header",
                pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;

        }
        rv = RvSipContactHeaderParse(hContact,strContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to parse to contact - bad syntax",
                 pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;
        }
        rv = RvSipRegClientSetContactHeader(hRegClient,hContact);
        if(rv != RV_OK)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                "RvSipRegClientMake - Register client 0x%p: Failed to set contact",
                pRegClient));
			RegClientUnLockAPI(pRegClient);
            return rv;
        }
    }

    rv = TranscClientCreateNewTransaction(&pRegClient->transcClient);
    if(rv<0) {
      RegClientUnLockAPI(pRegClient);
      return rv;
    }

    RegClientUnLockAPI(pRegClient);

    return RV_OK;

}

RVAPI RvStatus RVCALLCONV RvSipRegClientSendRequest(IN RvSipRegClientHandle hRegClient) {

  RegClient          *pRegClient;
  RvStatus           retStatus;
  
  if (NULL == hRegClient) {
    return RV_ERROR_INVALID_HANDLE;
  }
  pRegClient = (RegClient *)hRegClient;
  if (RV_OK != RegClientLockAPI(pRegClient)) {
    return RV_ERROR_INVALID_HANDLE;
  }
  
  RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
					"RvSipRegClientRegister - Register client 0x%p ",pRegClient));
  
  /* Assert that the current state allows this action */
  if ((RVSIP_REG_CLIENT_STATE_IDLE != pRegClient->eState)
      && (RVSIP_REG_CLIENT_STATE_REDIRECTED != pRegClient->eState)
      && (RVSIP_REG_CLIENT_STATE_REGISTERED != pRegClient->eState)
      && (RVSIP_REG_CLIENT_STATE_FAILED != pRegClient->eState) 
      && (RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED != pRegClient->eState)) {
    RegClientUnLockAPI(pRegClient);
    return RV_ERROR_ILLEGAL_ACTION;
  } 
  
  retStatus = TranscClientSendRequest(&pRegClient->transcClient, SIP_TRANSACTION_METHOD_REGISTER, NULL);
  if (RV_OK == retStatus) {
    /* The transaction-client object changes state to Activating */
    TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_ACTIVATING,
			    SIP_TRANSC_CLIENT_REASON_USER_REQUEST,
			    &pRegClient->transcClient, 0);
  } else {
    RegClientUnLockAPI(pRegClient);
    return retStatus;
  }
  
  RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
				       "%s - Register client 0x%p: REGISTER request was successfully sent",
				       __FUNCTION__,
				       pRegClient));
  RegClientUnLockAPI(pRegClient);
  return RV_OK;
}

#endif
//SPIRENT_END


#ifdef SUPPORT_EV_HANDLER_PER_OBJ
/***************************************************************************
 * RvSipRegClientSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all register-client events.
 * Return Value: RV_OK       - Success otherwise ERROR.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient	- Handle to the register-client.
 *            pEvHandlers	- Pointer to the application event handler structure
 *            structSize	- The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetEvHandlers(
                                   IN  RvSipRegClientHandle		hRegClient,
                                   IN  RvSipRegClientEvHandlers *pEvHandlers,
                                   IN  RvUint32                  evHandlerStructSize)
{
    RegClient	       *pRegClient;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pEvHandlers)
    {
        return RV_ERROR_NULLPTR;
    }
    if (0 > evHandlerStructSize)
    {
        return RV_ERROR_BADPARAM;
    }
    pRegClient = (RegClient*)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /* Copy the event handlers struct according to the given size */
    memset(&(pRegClient->objEvHandlers), 0, sizeof(RvSipRegClientEvHandlers));
    memcpy(&(pRegClient->objEvHandlers), pEvHandlers, evHandlerStructSize);
	
	pRegClient->regClientEvHandlers = &pRegClient->objEvHandlers;

    RegClientUnLockAPI(pRegClient);
    return RV_OK;
}
#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/

/***************************************************************************
 * RvSipRegClientDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach the Register-Client owner. After calling this function the user will
 *          stop receiving events for this Register-Client. This function can be called
 *          only when the object is in terminated state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDetachOwner (
                                         IN RvSipRegClientHandle hRegClient)
{
    RegClient *pRegClient;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRegClient->eState != RVSIP_REG_CLIENT_STATE_TERMINATED)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
          "RvSipRegClientDetachOwner - Register client 0x%p is in state %s, and not in state terminated - can't detach owner",
          pRegClient, RegClientStateIntoString(pRegClient->eState)));

		RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    else
    {
        pRegClient->regClientEvHandlers = NULL;
        RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientDetachOwner - Register client 0x%p: was detached from owner",
               pRegClient));
    }

	RegClientUnLockAPI(pRegClient);
    return RV_OK;
}

/*-----------------------------------------------------------------------
       R E G  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipRegClientDNSContinue
 * ------------------------------------------------------------------------
 * General: Prepares the Reg-Client object to a retry to send a request after
 *          the previous attempt failed.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          In order to retry to send the message the application must first
 *          call the RvSipRegClientDNSContinue() function.
 *          Calling this function, clones the failure transaction and
 *          updates the DNS list. (In order to actually re-send the request
 *          to the next IP address use the function RvSipRegClientDNSReSendRequest()).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the Reg-Client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDNSContinue (
                                      IN  RvSipRegClientHandle   hRegClient)
{
    RegClient* pRegClient = (RegClient*)hRegClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = RegClientLockAPI(pRegClient); /*try to lock the reg client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pRegClient->transcClient.hSecAgree ||
        NULL != pRegClient->transcClient.hSecObj)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientDNSContinue - reg-client 0x%p: illegal action for IMS Secure Reg Client",
            pRegClient));
	    RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientDNSContinue - reg-client 0x%p: continue DNS" ,
            pRegClient));

    rv = SipTranscClientDNSContinue(&pRegClient->transcClient);

	RegClientUnLockAPI(pRegClient);
    return rv;
#else
    RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientDNSContinue - reg-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pRegClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipRegClientAuthenticate
 * ------------------------------------------------------------------------
 * General: Re-sends the Register request with authentication information.
 *          This method should be called in the Unauthenticated state
 *          after receiving a 401 or 407 response from the server.
 *          The authorization header will be added to the outgoing request.
 *          The register-client then assumes the Registering state and
 *          waits for a response from the server.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client or
 *                                   the Request-Uri is invalid.
 *               RV_ERROR_BADPARAM - The Request-Uri must be a SIP-Url and
 *                                     must not contain a User name. In case
 *                                     these rules are violated Invalid-
 *                                     Parameter is returned.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid register-client state for this
 *                                  action.
 *               RV_ERROR_OUTOFRESOURCES - register-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientAuthenticate (
                                         IN RvSipRegClientHandle hRegClient)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;
	RPOOL_Ptr			srcCallIdPage;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientAuthenticate - Register client 0x%p ",pRegClient));

    /* Assert that the current state allows this action */
    if (RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED != pRegClient->eState && 
		RVSIP_REG_CLIENT_STATE_REGISTERED != pRegClient->eState)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientAuthenticate - Register client 0x%p - illegal state %s",
            pRegClient, RegClientStateIntoString(pRegClient->eState)));
		RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

	if (pRegClient->bIsDetach == RV_FALSE)
	{
		RvInt32 currentCseq;
		/* if the reg client is not detached from its manager 
		 we need to use the mgr cseq and call-id, lock the manager to get the correct CSeq-Step*/
		RegClientMgrLock(pRegClient->pMgr);

		SipCommonCSeqPromoteStep(&pRegClient->pMgr->cseq);		
		SipCommonCSeqGetStep(&pRegClient->pMgr->cseq, &currentCseq);
		SipTranscClientSetCSeqStep(&pRegClient->transcClient,currentCseq);
		
		srcCallIdPage.hPage = pRegClient->pMgr->hMemoryPage;
		srcCallIdPage.hPool = pRegClient->pMgr->hGeneralMemPool;
		srcCallIdPage.offset = pRegClient->pMgr->strCallId;
		
		retStatus = SipTranscClientSetCallIdFromPage(&pRegClient->transcClient, &srcCallIdPage);
		if (retStatus != RV_OK)
		{
			RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
				"RvSipRegClientAuthenticate - Register client 0x%p: was unable to get call-id from its manager.",
				pRegClient));
			RegClientMgrUnLock(pRegClient->pMgr);
			RegClientUnLockAPI(pRegClient);
			return retStatus;
		}
		
		RegClientMgrUnLock(pRegClient->pMgr);
	}
    
    retStatus = SipTranscClientActivate(&pRegClient->transcClient, SIP_TRANSACTION_METHOD_REGISTER, NULL);

	

    if (RV_OK != retStatus)
    {
		RegClientUnLockAPI(pRegClient);
        return retStatus;
    }
    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientAuthenticate - Register client 0x%p: REGISTER request was successfully sent",
               pRegClient));


	RegClientUnLockAPI(pRegClient);

    return RV_OK;
}
#endif /* #ifdef RV_SIP_AUTH_ON */


/***************************************************************************
 * RvSipRegClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a register-client object. This function destructs the
 *          register-client object. You cannot reference the object after
 *          calling this function.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client is
 *                                   invalid.
 *                 RV_OK - The register-client was successfully terminated
 *                            and distructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientTerminate (
                                         IN RvSipRegClientHandle hRegClient)
{
    RegClient *pRegClient;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pRegClient->eState == RVSIP_REG_CLIENT_STATE_TERMINATED)
    {
		RegClientUnLockAPI(pRegClient);
        return RV_OK;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientTerminate - About to  terminate Register client 0x%p",
               pRegClient));
    RegClientTerminate(pRegClient,RVSIP_REG_CLIENT_REASON_USER_REQUEST);
	RegClientUnLockAPI(pRegClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientUseFirstRouteForRegisterRequest
 * ------------------------------------------------------------------------
 * General: The Application may want to use a preloaded route header when
 *          sending the register message.For this purpose, it should add the
 *          Route headers to the outbound message, and call this function
 *          to indicate the stack to send the request to the address of the
 *          first Route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient    - Handle to the register client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientUseFirstRouteForRegisterRequest (
                                      IN  RvSipRegClientHandle hRegClient)
{
    RegClient *pRegClient;
	RvStatus   rv;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientUseFirstRouteForRegisterRequest - reg-client 0x%p will use preloaded Route header",
              pRegClient));

	rv = SipTranscClientUseFirstRouteForRequest(&pRegClient->transcClient);

	RegClientUnLockAPI(pRegClient);
    return rv;

}

/*-----------------------------------------------------------------------
       R E G  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipRegClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: Stops retries to send a Register request after send failure.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          Calling RvSipRegClientDNSGiveUp() indicates that the application
 *          wishes to give up on this request. Retries to send the request
 *          will stop and the Register client will change its state back to
 *          the previous state.
 *          If this is the initial Register request of the Reg-Client, calling
 *          DNSGiveUp() will terminate the Reg-Client object. If this is a
 *          refresh, the Reg-Client will move back to the REGISTERED state.
 * Return Value: RvStatus;
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the Reg-Client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDNSGiveUp (
                                   IN  RvSipRegClientHandle   hRegClient)
{
    RegClient* pRegClient = (RegClient*)hRegClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = RegClientLockAPI(pRegClient); /*try to lock the reg client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pRegClient->transcClient.hSecAgree ||
        NULL != pRegClient->transcClient.hSecObj)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientDNSGiveUp - reg-client 0x%p: illegal action for IMS Secure Reg Client",
            pRegClient));
	    RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientDNSGiveUp - reg-client 0x%p: giveup DNS" ,
              pRegClient));
    rv = SipTranscClientDNSGiveUp(&pRegClient->transcClient);

	RegClientUnLockAPI(pRegClient);
    return rv;
#else
    RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientDNSGiveUp - reg client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pRegClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipRegClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: Re-sends a register request after the previous attempt failed.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the application can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          In order to re-send the request first call RvSipRegClientDNSContinue().
 *          You should then call RvSipRegClientDNSReSendRequest().
 *          The request will automatically be sent to the next resoulved IP address
 *          in the DNS list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the reg client that sent the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDNSReSendRequest (
                                            IN  RvSipRegClientHandle   hRegClient)
{
    RegClient* pRegClient = (RegClient*)hRegClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = RegClientLockAPI(pRegClient); /*try to lock the reg client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pRegClient->transcClient.hSecAgree ||
        NULL != pRegClient->transcClient.hSecObj)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientDNSReSendRequest - reg-client 0x%p: illegal action for IMS Secure Reg Client",
            pRegClient));
	    RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientDNSReSendRequest - reg client 0x%p: re-send DNS" ,
            pRegClient));

    rv = SipTranscClientDNSReSendRequest(&pRegClient->transcClient, SIP_TRANSACTION_METHOD_REGISTER, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
			"RvSipRegClientDNSReSendRequest: reg-client 0x%p - Failed to resend the request",
			pRegClient));
    }

	RegClientUnLockAPI(pRegClient);
    return rv;
#else
    RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientDNSReSendRequest - reg client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pRegClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipRegClientDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves DNS list object from the Reg-Client current active
 *          transaction.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state you can use RvSipRegClientDNSGetList() to get the
 *          DNS list if you wish to view or change it.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the reg client that sent the request.
 * Output   phDnsList - The DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDNSGetList(
                              IN  RvSipRegClientHandle         hRegClient,
                              OUT RvSipTransportDNSListHandle *phDnsList)
{
    RegClient* pRegClient = (RegClient*)hRegClient;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = RegClientLockAPI(pRegClient); /*try to lock the reg client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientDNSGetList - reg client 0x%p: getting DNS list" ,
              pRegClient));

	rv = SipTranscClientDNSGetList(&pRegClient->transcClient, phDnsList);
    
	RegClientUnLockAPI(pRegClient);
    return rv;
#else
    RV_UNUSED_ARG(phDnsList);
    RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientDNSGetList - reg client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pRegClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipRegClientGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the reg-client page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information
 *         and set it back to the reg-client using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         Party   - you should set these headers back with the RvSipRegClientSetToHeader()
 *                   or RvSipRegClientSetFromHeader() API functions.
 *         Contact - you should set this header back with the RvSipRegClientSetContactHeader()
 *                   API function.
 *         Expires - you should set this header back with the RvSipRegClientSetExpiresHeader()
 *                   API function.
 *         Authorization - you should set this header back with the header to the RvSipRegClientSetInitialAuthorization() 
 *                   API function that is available in the IMS add-on only.
 *         address - you should supply the address to the RvSipRegClientSetRegistrar() API function.
 *
 *          Note - You may use this function only on reg-client initialization stage when the
 *          reg-client is in the IDLE state.
 *          In order to change headers after the initialization stage, you must allocate the 
 *          header on an application page, and then set it with the correct API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the reg-client.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetNewMsgElementHandle (
                                      IN   RvSipRegClientHandle   hRegClient,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phElement)
{
    RvStatus  rv;
    RegClient    *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = RegClientLockAPI(pRegClient); /*try to lock the reg-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	if(eHeaderType == RVSIP_HEADERTYPE_CONTACT)
	{
		/* should be allocated on the reg-client contact page */
		rv = RvSipRegClientGetNewContactHeaderHandle(hRegClient, (RvSipContactHeaderHandle *)phElement);
	}
    else
	{
		rv = SipTranscClientGetNewMsgElementHandle(&pRegClient->transcClient, eHeaderType, eAddrType, phElement);
	}
    if (rv != RV_OK || *phElement == NULL)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientGetNewMsgElementHandle - reg-client 0x%p - Failed to create new header (rv=%d)",
              pRegClient, rv));
		RegClientUnLockAPI(pRegClient);
        return RV_ERROR_OUTOFRESOURCES;
    }
	RegClientUnLockAPI(pRegClient);
    return RV_OK;
}


/***************************************************************************
 * RvSipRegClientDetachFromMgr
 * ------------------------------------------------------------------------
 * General: Detach this register-client from the manager object. This client
 *          will generate its own call-id to be used fro its registrations.
 *          Also this client will manage its own CSeq-Step counter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The Sip Stack handle to the reg-client
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDetachFromMgr(
                                      IN  RvSipRegClientHandle   hRegClient)
{
    RvStatus               rv;
    RegClient              *pRegClient = (RegClient*)hRegClient;

    if(pRegClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RegClientLockAPI(pRegClient); /*try to lock the transc-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pRegClient->eState != RVSIP_REG_CLIENT_STATE_IDLE)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientDetachFromMgr - reg-client 0x%p is not in state Idle", pRegClient));
		RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    pRegClient->bIsDetach = RV_TRUE;

	RegClientUnLockAPI(pRegClient);
    return rv;

}

/***************************************************************************
 * RvSipRegClientSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the reg-client call-id. If the call-id is not set the reg-client
 *          will use the regClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The reg-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The Sip Stack handle to the reg-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetCallId (
                                      IN  RvSipRegClientHandle   hRegClient,
                                      IN  RvChar              *strCallId)
{
    RvStatus               rv;

    RegClient    *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = RegClientLockAPI(pRegClient); /*try to lock the reg-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pRegClient->eState != RVSIP_REG_CLIENT_STATE_IDLE)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetCallId - reg-client 0x%p is not in state Idle", pRegClient));
		RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
	rv = SipTranscClientSetCallId(&pRegClient->transcClient, strCallId);

	pRegClient->bIsDetach = RV_TRUE;

	RegClientUnLockAPI(pRegClient);
    return rv;

}

/***************************************************************************
 * RvSipRegClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the reg-client Call-Id.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the reg-client.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The reg-client does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient   - The Sip Stack handle to the reg-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCallId (
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus               rv;
    RegClient    *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = RegClientLockAPI(pRegClient); /*try to lock the reg-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCallId(&pRegClient->transcClient, bufSize, pstrCallId, actualSize);
    
	RegClientUnLockAPI(pRegClient);
    return rv;


}


/***************************************************************************
 * RvSipRegClientSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the from header associated with the register-client. If the
 *          From header was constructed by the
 *          RvSipRegClientGetNewPartyHeaderHandle function, the From header
 *          is attached to the register-client object.
 *          Otherwise it will be copied by this function. Note
 *          that attempting to alter the from address after Registeration has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid
 *                                   or the From header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 *            hFrom      - Handle to an application constructed an initialized
 *                       from header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetFromHeader (
                                      IN  RvSipRegClientHandle   hRegClient,
                                      IN  RvSipPartyHeaderHandle hFrom)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetFromHeader - Setting Register-Client 0x%p From header",
              pRegClient));

    retStatus = SipTranscClientSetFromHeader(&pRegClient->transcClient, hFrom);
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}


/***************************************************************************
 * RvSipRegClientGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the register-client.
 *          Attempting to alter the from address after Regitration has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 * Output:     phFrom     - Pointer to the register-client From header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetFromHeader (
                                      IN  RvSipRegClientHandle    hRegClient,
                                      OUT RvSipPartyHeaderHandle *phFrom)
{
    RegClient          *pRegClient;
	RvStatus			rv;


    if (NULL == hRegClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetFromHeader(&pRegClient->transcClient, phFrom);
    RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the register-client. If the To
 *          header was constructed by the RvSipRegClientGetNewPartyHeaderHandle
 *          function, the To header will be attached to the register-client
 *          object.
 *          Otherwise the To header is copied by this function. Note
 *          that attempting to alter the To header after Registration has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle or the To
 *                                   header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 *            hTo        - Handle to an application constructed and initialized
 *                       To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetToHeader (
                                      IN  RvSipRegClientHandle   hRegClient,
                                      IN  RvSipPartyHeaderHandle hTo)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetToHeader - Setting Register-Client 0x%p To header",
              pRegClient));

	retStatus = SipTranscClientSetToHeader(&pRegClient->transcClient, hTo);

    RegClientUnLockAPI(pRegClient);
    return retStatus;
}

/***************************************************************************
 * RvSipRegClientGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To header associated with the register-client.
 *          Attempting to alter the To header after Registration has
 *          been requested might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 * Output:     phTo       - Pointer to the register-client To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetToHeader (
                                      IN    RvSipRegClientHandle    hRegClient,
                                      OUT   RvSipPartyHeaderHandle *phTo)
{
    RegClient          *pRegClient;
	RvStatus			rv;


    if (NULL == hRegClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetToHeader(&pRegClient->transcClient, phTo);
    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientSetContactHeader
 * ------------------------------------------------------------------------
 * General: Set a contact header to the register-client object. Before calling
 *          Register() the application should use this function to
 *          supply all the Contact headers the application requires in order
 *          to register.
 *          These Contact headers are inserted to the Register request
 *          message before it is sent to the registrar.
 * Return Value: RV_ERROR_INVALID_HANDLE  -   The register-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Contact header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New contact header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 *            hContactHeader - Handle to a Contact header to be set to the
 *                           register-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetContactHeader (
                                 IN  RvSipRegClientHandle     hRegClient,
                                 IN  RvSipContactHeaderHandle hContactHeader)
{
    RegClient                *pRegClient;
    RvSipContactHeaderHandle *phNewContact;
    RvStatus                 retStatus;

    if ((NULL == hRegClient) ||
        (NULL == hContactHeader))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetContactHeader - Setting Register-Client 0x%p Contact header",
              pRegClient));

    /* Allocate a new Contact header handle in the end of the Contact headers list */
    retStatus = RegClientGetNewContactHeaderHandle(pRegClient, &phNewContact);
    if (RV_OK != retStatus)
    {
        RegClientUnLockAPI(pRegClient);
        return retStatus;
    }
    /* Check if the given header was constructed on the register-client
       object's page and if so attach it */
    if((SipContactHeaderGetPool(hContactHeader) == pRegClient->pMgr->hGeneralMemPool) &&
       (SipContactHeaderGetPage(hContactHeader) == pRegClient->hContactPage))
    {
        *phNewContact = hContactHeader;
        RegClientUnLockAPI(pRegClient);
        return RV_OK;
    }
    /* Copy the given Contact header to the register-client object */
    retStatus = RegClientCopyContactHeader(pRegClient, phNewContact,
                                           hContactHeader);
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}


/***************************************************************************
 * RvSipRegClientGetFirstContactHeader
 * ------------------------------------------------------------------------
 * General: Get the first Contact header of the Contact headers list.
 * Return Value: A pointer to the first Contact header of the list. NULL
 *               there are no Contact headers in the list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient  - Handle to the register-client.
 ***************************************************************************/
RVAPI RvSipContactHeaderHandle *RVCALLCONV RvSipRegClientGetFirstContactHeader(
                          IN  RvSipRegClientHandle                 hRegClient)
{
    RegClient          *pRegClient;
    RLIST_ITEM_HANDLE   hElement;


    if (NULL == hRegClient)
    {
        return NULL;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return NULL;
    }

    if (NULL == pRegClient->hContactList)
    {
        RegClientUnLockAPI(pRegClient);
        return NULL;
    }
    /* Get the first element of the list */
    RLIST_GetHead(NULL, pRegClient->hContactList, &hElement);
    RegClientUnLockAPI(pRegClient);
    return ((RvSipContactHeaderHandle *)hElement);
}


/***************************************************************************
 * RvSipRegClientGetNextContactHeader
 * ------------------------------------------------------------------------
 * General: Get a Contact header from the Contact headers list. The returned
 *          Contact header follows the Contact header indicated by
 *          phPrevContact in the list of Contact headers. For this purpose,
 *          the phPrevContact must be valid- a part of the list of Contact
 *          headers.
 * Return Value: A pointer to the requested Contact header. NULL if doesn't
 *               exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient  - Handle to the register-client.
 *          phPrevContact   - A handle to the previous Contact header.
 ***************************************************************************/
RVAPI RvSipContactHeaderHandle *RVCALLCONV RvSipRegClientGetNextContactHeader(
                        IN  RvSipRegClientHandle                 hRegClient,
                        IN  RvSipContactHeaderHandle            *phPrevContact)
{
    RegClient          *pRegClient;
    RLIST_ITEM_HANDLE   hElement;


    if (NULL == phPrevContact)
    {
        return NULL;
    }
    if (NULL == hRegClient)
    {
        return NULL;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return NULL;
    }

    if (NULL == pRegClient->hContactList)
    {
        RegClientUnLockAPI(pRegClient);
        return NULL;
    }
    /* Get the header following hPrevContact in the list */
    RLIST_GetNext(NULL, pRegClient->hContactList,
                  (RLIST_ITEM_HANDLE)phPrevContact, &hElement);
    RegClientUnLockAPI(pRegClient);
    return ((RvSipContactHeaderHandle *)hElement);
}


/***************************************************************************
 * RvSipRegClientRemoveContactHeader
 * ------------------------------------------------------------------------
 * General: Removes a Contact header from the Contact headers list.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *                 RV_ERROR_NULLPTR     - The supplied Contact header handle is
 *                                   invalid.
 *               RV_OK        - success.
 *               RV_ERROR_NOT_FOUND       - The received Contact header is not in
 *                                   the list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient  - Handle to the register-client.
 *          hContact   - A handle to the Contact header to be removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientRemoveContactHeader (
                        IN  RvSipRegClientHandle                 hRegClient,
                        IN  RvSipContactHeaderHandle             hContact)
{
    RegClient                *pRegClient;
    RvStatus                 retStatus;
    RvUint32                  safeCounter;
    RvUint32                  maxLoops;
    RLIST_ITEM_HANDLE         hElement;

    if (NULL == hContact)
    {
        return RV_ERROR_NULLPTR;
    }
    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientRemoveContactHeader - Removing Contact header 0x%p from register-client 0x%p ",
              pRegClient, hContact));

    if (NULL == pRegClient->hContactList)
    {
        /* The Contacts list is empty */
        RegClientUnLockAPI(pRegClient);
        return RV_ERROR_NOT_FOUND;
    }
    /* Go over the list of Contact headers in the register-client, and
       look for the give Contact header to be removed. */
    safeCounter = 0;
    maxLoops = 10000;
    retStatus = RV_ERROR_NOT_FOUND;
    RLIST_GetHead(NULL, pRegClient->hContactList, &hElement);
    while (NULL != hElement)
    {
        if (*((RvSipContactHeaderHandle *)hElement) == hContact)
        {
            RLIST_Remove(NULL, pRegClient->hContactList, hElement);
            retStatus = RV_OK;
            break;
        }
        RLIST_GetNext(NULL, pRegClient->hContactList, hElement, &hElement);
        if (safeCounter > maxLoops)
        {
            RvLogExcep(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                      "RvSipRegClientRemoveContactHeader - Error in loop - number of rounds is larger than number of Contact headers (reg=0x%p)",
                      pRegClient));
            retStatus = RV_ERROR_UNKNOWN;
            break;
        }
        safeCounter++;
    }
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}

/***************************************************************************
 * RvSipRegClientRemoveAllContactHeaders
 * ------------------------------------------------------------------------
 * General: Removes the whole list of the Contact headers.
 *			The function free the page that holds the contact headers and list.
 *			It is useful when you want to change the whole list. using it
 *			prevent an increase in the reg-client page size.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_OK        - success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient  - Handle to the register-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientRemoveAllContactHeaders (
                        IN  RvSipRegClientHandle                 hRegClient)
{
	RegClient                *pRegClient;
    
    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientRemoveAllContactHeaders - Removing all Contact headers from register-client 0x%p (page=0x%p)",
              pRegClient, pRegClient->hContactPage));

    if (NULL == pRegClient->hContactList)
    {
        /* The Contacts list is empty */
        RegClientUnLockAPI(pRegClient);
        return RV_ERROR_NOT_FOUND;
    }
    
	pRegClient->hContactList = NULL;
	RPOOL_FreePage(pRegClient->pMgr->hGeneralMemPool, pRegClient->hContactPage);

	RPOOL_GetPage(pRegClient->pMgr->hGeneralMemPool, 0, &pRegClient->hContactPage);

	RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientRemoveAllContactHeaders - new page 0x%p was allocated for register-client 0x%p ",
              pRegClient, pRegClient->hContactPage));

    RegClientUnLockAPI(pRegClient);
    return RV_OK;
}
/***************************************************************************
 * RvSipRegClientSetRegistrar
 * ------------------------------------------------------------------------
 * General: Set the SIP-Url of the registrar in the register-client object.
 *          Before calling Register(), your application should use
 *          this function to supply the SIP-Url of the registrar.
 *          The register request will be sent to this SIP-Url.
 *          You can change the registrar's SIP-Url each time you call
 *          Register().
 *          This ability can be used for updating the registrar's SIP-Url
 *          in case of redirections and refreshes.
 *          The registrar address must be a SIP-Url with no user name.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied address handle is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 *            hRequestUri - Handle to the registrar SIP-Url to be set to the
 *                        register-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetRegistrar (
                                 IN  RvSipRegClientHandle  hRegClient,
                                 IN  RvSipAddressHandle    hRequestUri)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;
	RvSipAddressType    eAddrType;
	RvChar             strUser[1];
    RvUint             len;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetRegistrar - Setting Register-Client 0x%p registrar Request-Url",
              pRegClient));
	/* The address spec of the Request-Uri does not have to be a SIP-Url */
    eAddrType = RvSipAddrGetAddrType(hRequestUri);
    if (RVSIP_ADDRTYPE_URL == eAddrType)
    {
        /* The address spec of the Request-Uri must not contain user name for reg-client */
        retStatus = RvSipAddrUrlGetUser(hRequestUri, strUser, 1, &len);
        if (RV_ERROR_NOT_FOUND != retStatus)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
				"RvSipRegClientSetRegistrar - reg-client 0x%p: destination must not contain user name (rv=%d)",
				pRegClient, RV_ERROR_BADPARAM));
			RegClientUnLockAPI(pRegClient);
            return RV_ERROR_BADPARAM;
        }
    }

	retStatus = SipTranscClientSetDestination(&pRegClient->transcClient, hRequestUri);
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}


/***************************************************************************
 * RvSipRegClientGetRegistrar
 * ------------------------------------------------------------------------
 * General: Gets the SIP-Url of the registrar associated with the
 *          register-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The address object was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - Handle to the register-client.
 * Output:     phRequestUri    - Handle to the registrar SIP-Url.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetRegistrar (
                               IN  RvSipRegClientHandle      hRegClient,
                               OUT RvSipAddressHandle       *phRequestUri)
{
    RegClient          *pRegClient;
	RvStatus			rv;

    if (NULL == hRegClient)
    {
        return RV_ERROR_BADPARAM;
    }
    
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetDestination(&pRegClient->transcClient, phRequestUri);
    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the register client. You can
 *          call this function from the RvSipRegClientStateChangedEv call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - Handle to the register-client.
 * Output:     phMsg           - pointer to the received message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetReceivedMsg(
                                            IN  RvSipRegClientHandle  hRegClient,
                                             OUT RvSipMsgHandle          *phMsg)
{
    RvStatus               rv;

    RegClient              *pRegClient = (RegClient *)hRegClient;

    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }


    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetReceivedMsg(&pRegClient->transcClient, phMsg);

    RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the register-client.
 *          You can call this function before you call API functions such as
 *          Register(), Make() and Authenticate().
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - Handle to the register-client.
 * Output:    phMsg           - pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipRegClientGetOutboundMsg(
                                     IN  RvSipRegClientHandle  hRegClient,
                                     OUT RvSipMsgHandle       *phMsg)
{
    RegClient* pRegClient = (RegClient *)hRegClient;
    RvStatus   rv;

    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetOutboundMsg(&pRegClient->transcClient, phMsg);

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientGetOutboundMsg - register-client= 0x%p returned handle= 0x%p",
              pRegClient, *phMsg));

    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the register-client to NULL. If the
 *          register-client is about to send a message it will create its own
 *          message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - Handle to the register-client.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipRegClientResetOutboundMsg(
                                     IN  RvSipRegClientHandle  hRegClient)
{
    RegClient              *pRegClient = (RegClient *)hRegClient;
    RvStatus              rv;

    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientResetOutboundMsg(&pRegClient->transcClient);

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientResetOutboundMsg - register-client= 0x%p resetting outbound message",
              pRegClient));

    RegClientUnLockAPI(pRegClient);
    return RV_OK;
}


/***************************************************************************
 * RvSipRegClientSetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Set an Expires header in the register-client object. Before
 *          calling Register(), the application can use this function to
 *          supply the required Expires header to use in the Register
 *          request. This Expires header is inserted to the Register
 *          request message before it is sent to the registrar.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Expires header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Expires header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client.
 *            hExpiresHeader - Handle to an Expires header to be set to the
 *                           register-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetExpiresHeader (
                                 IN  RvSipRegClientHandle     hRegClient,
                                 IN  RvSipExpiresHeaderHandle hExpiresHeader)
{
    RegClient          *pRegClient;
    RvStatus           retStatus;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetExpiresHeader - Setting Register-Client 0x%p Expires header",
              pRegClient));

	retStatus = SipTranscClientSetExpiresHeader(&pRegClient->transcClient, hExpiresHeader);
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}


/***************************************************************************
 * RvSipRegClientGetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Gets the Expires header associated with the register-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The Expires header was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - Handle to the register-client.
 * Output:     phExpiresHeader - Handle to the register-client's Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetExpiresHeader (
                               IN  RvSipRegClientHandle      hRegClient,
                               OUT RvSipExpiresHeaderHandle *phExpiresHeader)
{
    RegClient          *pRegClient;
	RvStatus			rv;


    if (NULL == hRegClient)
    {
        return RV_ERROR_BADPARAM;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetExpiresHeader(&pRegClient->transcClient, phExpiresHeader);

    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientGetCurrentState
 * ------------------------------------------------------------------------
 * General: Returns the current state of the register-client.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given register-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 * Output:     peState -  The register-client's current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCurrentState (
                                      IN  RvSipRegClientHandle hRegClient,
                                      OUT RvSipRegClientState *peState)
{
    RegClient          *pRegClient;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == peState)
    {
        return RV_ERROR_NULLPTR;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peState = pRegClient->eState;
    RegClientUnLockAPI(pRegClient);
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientSetCSeqStep
 * ------------------------------------------------------------------------
 * General: Sets the CSeq-Step associated with the register-client.
 *          The supplied CSeq must be bigger then zero.
 *          If you don't set the CSeq step the register client will send the
 *          first register request with CSeq=1 and will increase the CSeq for
 *          subsequent requests.
 *          Note1: Most applications do not need to use this function.
 *          The register client manages the CSeq step automatically.
 *          Note2: The CSeq supplied using this function will be used only if
 *          the register client detached from its manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 *           cSeqStep -  The register-client's CSeq-Step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetCSeqStep (
                                      IN  RvSipRegClientHandle hRegClient,
                                      IN  RvUint32            cSeqStep)
{
    RegClient          *pRegClient;
	RvStatus			rv;

    pRegClient = (RegClient *)hRegClient;
    if (NULL == pRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetCSeqStep(&pRegClient->transcClient, cSeqStep);
    RegClientUnLockAPI(pRegClient);

    return rv;
}

/***************************************************************************
 * RvSipRegClientGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Returns the CSeq-Step associated with the register-client.
 *          Note: The CSeq-Step is valid only after a registration request
 *                was successfully executed. Otherwise the CSeq-Step is 0.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given register-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 * Output:     pCSeqStep -  The register-client's CSeq-Step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCSeqStep (
                                      IN  RvSipRegClientHandle hRegClient,
                                      OUT RvUint32            *pCSeqStep)
{
	RvStatus	rv;
    RegClient  *pRegClient;


    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCSeqStep(&pRegClient->transcClient, pCSeqStep);
	
    RegClientUnLockAPI(pRegClient);
    return rv;
}


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * RvSipRegClientSetStateIdle
 * ------------------------------------------------------------------------
 * General: Reset RegClient state to IDLE. This is usefull when each channel 
 *          has multiple clients. The state needs to be reset to start a different client.
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_BadPointer    - Bad pointer was given by the application.
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient - The register-client handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetStateIdle (
                                      IN  RvSipRegClientHandle hRegClient)
{
    RegClient          *pRegClient;
    
    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    pRegClient->eState = RVSIP_REG_CLIENT_STATE_IDLE;

    RegClientUnLockAPI(pRegClient);

    return RV_OK;                                 
}

#if defined(UPDATED_BY_SPIRENT_ABACUS)

void RvSipRegRPoolMemShow( RvSipRegClientMgrHandle hRegClientMgr )
{
   // Display RPool memory usage.
   printf( "***** Reg. General Memory Pool *****\n" );
   RPool_MemShow( ((RegClientMgr*)hRegClientMgr)->hGeneralMemPool );
   printf( "***** Reg. Message Memory Pool *****\n" );
   RPool_MemShow( ((RegClientMgr*)hRegClientMgr)->hMsgMemPool );
}

#endif

/***************************************************************************
 * RvSipRegClientSetCallIdPtr
 * ------------------------------------------------------------------------
 * General: Attach the Call-Id pointer to the Reg-Client. The Call-Id buffer 
 *          should have been allocated from the Reg-Client page.
 *           
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient - The Sip Stack handle to the Reg-Client
 *          strCallIdPtr - Pointer to the application allocated buffer
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetCallIdPtr (
                                      IN  RvSipRegClientHandle   hRegClient,
                                      IN  RvInt32               strCallIdPtr)
{
/*
    RegClient          *pRegClient;

    if (NULL == hRegClient)
        return RV_ERROR_INVALID_HANDLE;

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (0 == strCallIdPtr)
      pRegClient->strCallId = UNDEFINED;
    else
      pRegClient->strCallId = strCallIdPtr;

    RegClientUnLockAPI(pRegClient);
*/
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientGetCallIdPtr 
 * ------------------------------------------------------------------------
 * General:Returns the pointer to Call-Id used by the Reg-Client.
 *         
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient   - The Sip Stack handle to the reg-client.
 *          strCallIdPtr - Pointer to the application allocated buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCallIdPtr (
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvInt32             *strCallIdPtr )
{
/*
    RegClient          *pRegClient;

    if (NULL == hRegClient)
        return RV_ERROR_INVALID_HANDLE;

    if (NULL == strCallIdPtr)
    {
        return RV_ERROR_NULLPTR;
    }

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *strCallIdPtr = pRegClient->strCallId;

    RegClientUnLockAPI(pRegClient);
*/
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientSetFromTagPtr
 * ------------------------------------------------------------------------
 * General: Attach the From Tag pointer to the From header of Reg-Client. The From Tag buffer 
 *          should have already been allocated from the Reg-Client page.
 *           
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient - The Sip Stack handle to the Reg-Client
 *          fromTagPtr - Pointer to the application allocated buffer
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetFromTagPtr (
                                      IN  RvSipRegClientHandle   hRegClient,
                                      IN  RvInt32               fromTagPtr)
{
/*
    RegClient          *pRegClient;
    RvSipPartyHeaderHandle hFrom;
    RvStatus              rv = RV_OK;

    if (NULL == hRegClient)
        return RV_InvalidHandle;

    rv = RvSipRegClientGetFromHeader(hRegClient,&hFrom);
    if (rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (0 == fromTagPtr)
    {
       rv = SipPartyHeaderSetTag(hFrom, NULL, 
                  pRegClient->pMgr->hGeneralMemPool, pRegClient->hPage, UNDEFINED);
    }
    else
    {
       rv = SipPartyHeaderSetTag(hFrom, (RV_CHAR*)fromTagPtr, 
                  pRegClient->pMgr->hGeneralMemPool, pRegClient->hPage, fromTagPtr);
    }

    RegClientUnLockAPI(pRegClient);

    return rv;                                 
*/
    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientGetFromTagPtr
 * ------------------------------------------------------------------------
 * General: Returns the pointer to From Tag used by the Reg-Client's From header.
 *           
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient - The Sip Stack handle to the Reg-Client
 *          fromTagPtr - Pointer to the application allocated buffer
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetFromTagPtr (
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvInt32             *fromTagPtr )
{
    RegClient          *pRegClient;
    RvSipPartyHeaderHandle hFrom;
    RvStatus              rv;

    if (NULL == hRegClient)
        return RV_ERROR_INVALID_HANDLE;

    if (NULL == fromTagPtr)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = RvSipRegClientGetFromHeader(hRegClient,&hFrom);
    if (rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *fromTagPtr = SipPartyHeaderGetTag(hFrom);

    RegClientUnLockAPI(pRegClient);

    return RV_OK;                                 
}

/***************************************************************************
 * RvSipRegClientSetURIFlag
 * ------------------------------------------------------------------------
 * General: Set registrar URI Configuration flag to transaction client object
 *          inside Reg-Client. 
 *           
 * Return Value: RV_InvalidHandle - The given register-client handle is invalid
 *               RV_Success - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hRegClient - The Sip Stack handle to the Reg-Client
 *          uri_flag   - configuration flag for registrar URI
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetCctAndURIFlag (
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvInt32             cctContext,
                            IN  RvInt32             uri_flag )
{
    RegClient          *pRegClient;

    if (NULL == hRegClient)
        return RV_ERROR_INVALID_HANDLE;

    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    SipTranscClientSetCct( &pRegClient->transcClient, cctContext);
    SipTranscClientSetURIFlag( &pRegClient->transcClient, uri_flag);
    
    RegClientUnLockAPI(pRegClient);

    return RV_OK;                                 
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipRegClientGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this reg-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClientMgr   - Handle to the register-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetStackInstance(
                                   IN   RvSipRegClientHandle   hRegClient,
                                   OUT  void                 **phStackInstance)
{
    RegClient    *pRegClient;
	RvStatus	  rv;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pRegClient    = (RegClient *)hRegClient;
	if (RV_OK != RegClientLockAPI(pRegClient))
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	rv = SipTranscClientGetStackInstance(&pRegClient->transcClient, phStackInstance);

    RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the Reg-Client will send outgoing
 *          requests.
 *          The stack can be configured to listen to many local addresses.
 *          Each local address has a transport type (UDP/TCP/TLS) and an address type
 *          (IPv4/IPv6). When the stack sends an outgoing request, the local address
 *          (from where the request is sent) is chosen according to the characteristics
 *          of the remote address. Both the local and remote addresses must have
 *          the same characteristics i.e. the same transport type and address type.
 *          If several configured local addresses
 *          match the remote address characteristics, the first configured address is taken.
 *          You can use RvSipRegClientSetLocalAddress() to force the Reg-Client to choose
 *          a specific local address for a specific transport and address type.
 *          For example, you can force the Reg-Client to use the second configured UDP/IPv4
 *          local address. If the Reg-Client will send a request to an
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
 * Input:     hRegClient     - The register-client handle.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType   - The address type(IPv4 or IPv6).
 *            strLocalIPAddress   - A string with the local address to be set to this reg-client.
 *          localPort      - The local port to be set to this reg-client. If you set
 *                           the local port to 0, you will get a default value of 5060.
 *          if_name        - The local interface name
 *          vdevblock      - the local hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetLocalAddress(
                            IN  RvSipRegClientHandle      hRegClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *strLocalIPAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    )
{
    RvStatus    rv;
    RegClient    *pRegClient = (RegClient*)hRegClient;
    if (pRegClient == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    /*try to lock the transc-client*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetLocalAddress(&pRegClient->transcClient, eTransportType, eAddressType, strLocalIPAddress, localPort
#if defined(UPDATED_BY_SPIRENT)
					    , if_name
					    , vdevblock
#endif
);
	
    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the reg-client will use to send outgoing
 *          requests. This is the address the user set using the
 *          RvSipRegClientSetLocalAddress function. If no address was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient      - The register-client handle
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:    localAddress    - The local address this reg-client is using
 *                            (must be longer than 48).
 *          localPort       - The local port this reg-client is using.
 *          if_name         - The local interface name
 *          vdevblock       - the local hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetLocalAddress(
                            IN  RvSipRegClientHandle      hRegClient,
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
    RegClient                 *pRegClient;
    RvStatus                   rv;

    pRegClient = (RegClient *)hRegClient;


    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientGetLocalAddress - RegClient 0x%p - Getting local %s,%saddress",pRegClient,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType)));


	rv = SipTranscClientGetLocalAddress(&pRegClient->transcClient, eTransportType, eAddressType, localAddress, localPort
#if defined(UPDATED_BY_SPIRENT)
					    , if_name
					    , vdevblock
#endif
    );
    RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the register-client object.
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
 * Input:     hRegClient     - Handle to the register-client.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetOutboundDetails(
                            IN  RvSipRegClientHandle            hRegClient,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg)
{
    RvStatus   rv = RV_OK;
    RegClient  *pRegClient;

    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetOutboundDetails(&pRegClient->transcClient, pOutboundCfg, sizeOfCfg);

	RegClientUnLockAPI(pRegClient);

    return rv;
}

/***************************************************************************
 * RvSipRegClientGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the register-client object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type. If the outbound details were not set to the specific
 *          register-client object, but the outbound proxy was defined to the SIP
 *          Stack on initialization, the SIP Stack parameters will be returned.
 *          If the register-client is not using an outbound address, NULL/UNDEFINED
 *          values are returned.
 *          Note: You must supply a valid consecutive buffer in the
 *                RvSipTransportOutboundProxyCfg structure to get the outbound strings
 *               (host name and IP address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient     - Handle to the register-client.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetOutboundDetails(
                            IN  RvSipRegClientHandle            hRegClient,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg)
{
    RegClient   *pRegClient;
    RvStatus    rv;


    pRegClient = (RegClient*)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetOutboundDetails(&pRegClient->transcClient, sizeOfCfg, pOutboundCfg);
	
	RegClientUnLockAPI(pRegClient);

    return rv;
}

/***************************************************************************
 * RvSipRegClientSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy of the reg-client. All the requests sent
 *          by this reg-client will be sent directly to the reg-client
 *          outbound proxy address (unless the reg-client is using a
 *          Record-Route path).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient        - The register-client handle
 *            strOutboundAddrIp   - The outbound proxy ip to be set to this
 *                              reg-client.
 *          outboundProxyPort - The outbound proxy port to be set to this
 *                              reg-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetOutboundAddress(
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvChar               *strOutboundAddrIp,
                            IN  RvUint16              outboundProxyPort)
{
    RvStatus           rv=RV_OK;
    RegClient            *pRegClient;

    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetOutboundAddress(&pRegClient->transcClient, strOutboundAddrIp, outboundProxyPort);

	RegClientUnLockAPI(pRegClient);
    return rv;
}



/***************************************************************************
 * RvSipRegClientSetOutboundHostName
 * ------------------------------------------------------------------------
 * General:  Sets the outbound proxy host name of the Reg-Client object.
 *           The outbound host name will be resolved each time a Register
 *           request is sent and the Register requests will be sent to the
 *           resolved IP address.
 *           Note: To set a specific IP address use RvSipRegClientSetOutboundAddress().
 *           If you configure a Reg-Client with both an outbound IP address and an
 *           outbound host name, the Reg-Client will ignore the outbound host name
 *           and will use the outbound IP address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient           - Handle to the RegClient
 *            strOutboundHost    - The outbound proxy host to be set to this
 *                               RegClient.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               RegClient. If you set the port to zero it
 *                               will be determined using the DNS procedures.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetOutboundHostName(
                            IN  RvSipRegClientHandle     hRegClient,
                            IN  RvChar                *strOutboundHost,
                            IN  RvUint16              outboundPort)
{
    RvStatus     rv          =RV_OK;
    RegClient     *pRegClient;

    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetOutboundHostName(&pRegClient->transcClient, strOutboundHost, outboundPort);

	RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of outbound proxy the RegClient is using. If an
 *          outbound host was set to the RegClient this host will be
 *          returned. If no outbound host was set to the RegClient
 *          but an outbound host was configured for the stack, the configured
 *          address is returned. '\0' is returned if the RegClient is not using
 *          an outbound host.
 *          NOTE: you must supply a valid consecutive buffer to
 *          get the outboundProxy host name
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hRegClient      - Handle to the RegClient
 * Output:
 *            srtOutboundHostName   -  A buffer with the outbound proxy host name
 *          pOutboundPort - The outbound proxy port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetOutboundHostName(
                            IN   RvSipRegClientHandle    hRegClient,
                            OUT  RvChar              *strOutboundHostName,
                            OUT  RvUint16            *pOutboundPort)
{
    RegClient    *pRegClient;
    RvStatus     rv;
    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetOutboundHostName(&pRegClient->transcClient, strOutboundHostName, pOutboundPort);

	RegClientUnLockAPI(pRegClient);
    return rv;
}




/***************************************************************************
 * RvSipRegClientSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the reg-client. If the request is sent
 *          by an outbound proxy, it will be sent with this transport.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient        - The register-client handle
 *            eTransportType    - The outbound proxy transport to be set to this
 *                              reg-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetOutboundTransport(
                            IN  RvSipRegClientHandle   hRegClient,
                            IN  RvSipTransport         eTransportType)
{
    RvStatus     rv = RV_OK;
    RegClient    *pRegClient;

    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientSetOutboundTransport(&pRegClient->transcClient, eTransportType);

	RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Gets the address of outbound proxy the reg-client is using. If an
 *          outbound address was set to the reg-client this address will be
 *          returned. If no outbound address was set to the reg-client
 *          but an outbound proxy was configured for the stack, the configured
 *          address is returned. '\0' is returned if the reg-client is not using
 *          an outbound proxy.
 *          NOTE: you must supply a valid consecutive buffer of size 48 to
 *          get the outboundProxyIp.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient        - The register-client handle
 * Output:  outboundProxyIp   - A buffer with the outbound proxy IP the reg-client.
 *                              is using(must be longer than 48).
 *          pOutboundProxyPort - The outbound proxy port the reg-client is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetOutboundAddress(
                            IN   RvSipRegClientHandle  hRegClient,
                            OUT  RvChar              *outboundProxyIp,
                            OUT  RvUint16            *pOutboundProxyPort)
{
    RegClient             *pRegClient;
    RvStatus           rv;

    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientGetOutboundAddress(&pRegClient->transcClient, outboundProxyIp, pOutboundProxyPort);

	RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of outbound proxy the reg-client is using. If an
 *          outbound transport was set to the reg-client this transport will be
 *          returned. If no outbound transport was set to the reg-client
 *          but an outbound proxy was configured for the stack, the configured
 *          transport is returned. UNDEFINED is returned if the reg-client is not using
 *          an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient        - The register-client handle
 * Output:  eOutboundProxyTransport    - The outbound proxy tranport type
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetOutboundTransport(
                            IN   RvSipRegClientHandle  hRegClient,
                            OUT  RvSipTransport       *eOutboundProxyTransport)
{
    RegClient   *pRegClient;
    RvStatus   rv;
    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
	rv = SipTranscClientGetOutboundTransport(&pRegClient->transcClient, eOutboundProxyTransport);

	RegClientUnLockAPI(pRegClient);
    return rv;
}


/***************************************************************************
 * RvSipRegClientSetPersistency
 * ------------------------------------------------------------------------
 * General: instructs the Reg-Client on whether to try and use persistent connection
 *          or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRegClient - The Reg-Client handle
 *          bIsPersistent - Determines whether the Reg-Client will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetPersistency(
                           IN RvSipRegClientHandle       hRegClient,
                           IN RvBool                  bIsPersistent)
{
    RvStatus  rv;

    RegClient             *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = RegClientLockAPI(pRegClient); /*try to lock the reg client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientSetPersistency(&pRegClient->transcClient, bIsPersistent);

	RegClientUnLockAPI(pRegClient);
    return rv;
}
/***************************************************************************
 * RvSipRegClientGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns whether the Reg-Client is configured to try and use a
 *          persistent connection or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRegClient - The Reg-Client handle
 * Output:  pbIsPersistent - Determines whether the Reg-Client uses
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipRegClientGetPersistency(
        IN  RvSipRegClientHandle                   hRegClient,
        OUT RvBool                             *pbIsPersistent)
{

    RegClient        *pRegClient;
	RvStatus			rv;


    pRegClient = (RegClient *)hRegClient;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetPersistency(&pRegClient->transcClient, pbIsPersistent);
	
    RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientSetConnection
 * ------------------------------------------------------------------------
 * General: Set a connection to be used by the RegClient. The RegClient will
 *          supply this connection to all the transactions its creates.
 *          If the connection will not fit the transaction local and remote
 *          addresses it will be
 *          replaced. And the Reg-Client will assume the new connection.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransaction - Handle to the transaction
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetConnection(
                   IN  RvSipRegClientHandle                hRegClient,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    RvStatus               rv;
    RegClient        *pRegClient= (RegClient *)hRegClient;
    
	
    if (NULL == pRegClient)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientSetConnection(&pRegClient->transcClient, hConn);

    RegClientUnLockAPI(pRegClient);
    return rv;
}
/***************************************************************************
 * RvSipRegClientGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Reg-Client.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClienthTransc - Handle to the Reg-Client.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetConnection(
                            IN  RvSipRegClientHandle             hRegClient,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    RegClient                 *pRegClient = (RegClient*)hRegClient;
	RvStatus				   rv;

    if (NULL == pRegClient || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phConn = NULL;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = SipTranscClientGetConnection(&pRegClient->transcClient, phConn);

    RegClientUnLockAPI(pRegClient);
    return rv;

}

/***************************************************************************
* RvSipRegClientSetTranscTimers
* ------------------------------------------------------------------------
* General: Sets timeout values for the RegClient's transactions timers.
*          If some of the fields in pTimers are not set (UNDEFINED), this
*          function will calculate it, or take the values from configuration.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    Input: hRegClient - Handle to the RegClient object.
*           pTimers - Pointer to the struct contains all the timeout values.
*           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetTranscTimers(
                            IN  RvSipRegClientHandle  hRegClient,
                            IN  RvSipTimers           *pTimers,
                            IN  RvInt32               sizeOfTimersStruct)
{
    RegClient  *pRegClient = (RegClient*)hRegClient;
    RvStatus   rv;

    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetTranscTimers - pRegClient 0x%p - setting timers",
              pRegClient));

    rv = SipTranscClientSetTranscTimers(&pRegClient->transcClient, pTimers, sizeOfTimersStruct);

    RegClientUnLockAPI(pRegClient);

    return rv;

}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipRegClientSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message,
 *          belonging to the RegClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hRegClient - Handle to the RegClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - Pointer to the struct that contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetSctpMsgSendingParams(
                    IN  RvSipRegClientHandle               hRegClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    RegClient  *pRegClient = (RegClient*)hRegClient;
    
    if(pRegClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientSetSctpMsgSendingParams - reg-client 0x%p - failed to lock the RegClient",
            pRegClient));
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientSetSctpMsgSendingParams(&pRegClient->transcClient, sizeOfParamsStruct, pParams);
    
    RegClientUnLockAPI(pRegClient);
    return rv;
}

/******************************************************************************
 * RvSipRegClientGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message, belonging
 *          to the RegClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hRegClient - Handle to the RegClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetSctpMsgSendingParams(
                    IN  RvSipRegClientHandle               hRegClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    RegClient  *pRegClient = (RegClient*)hRegClient;

    if(pRegClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientGetSctpMsgSendingParams - reg-client 0x%p - failed to lock the RegClient",
            pRegClient));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientGetSctpMsgSendingParams(&pRegClient->transcClient, sizeOfParamsStruct, pParams);

    RegClientUnLockAPI(pRegClient);
    return rv;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipRegClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Reg-Client.
 *          As a result of this operation, all messages, sent by this
 *          Reg-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hRegClient - Handle to the Reg-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetSecObj(
                                        IN  RvSipRegClientHandle    hRegClient,
                                        IN  RvSipSecObjHandle       hSecObj)
{
    RvStatus   rv;
    RegClient* pRegClient = (RegClient*)hRegClient;

    if(NULL == pRegClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientSetSecObj - Reg-Client 0x%p - Set Security Object %p",
        pRegClient, hSecObj));

    /*try to lock the Reg-Client*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientSetSecObj - Reg-Client 0x%p - failed to lock the RegClient",
            pRegClient));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientSetSecObj(&pRegClient->transcClient, hSecObj);

    RegClientUnLockAPI(pRegClient);
    return rv;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipRegClientGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Reg-Client.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hRegClient - Handle to the Reg-Client object.
 *  Output: phSecObj   - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetSecObj(
                                        IN  RvSipRegClientHandle    hRegClient,
                                        OUT RvSipSecObjHandle*      phSecObj)
{
    RvStatus   rv;
    RegClient* pRegClient = (RegClient*)hRegClient;

    if(NULL==pRegClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientGetSecObj - Reg-Client 0x%p - Get Security Object",
        pRegClient));

    /*try to lock the Reg-Client*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientGetSecObj - Reg-Client 0x%p - failed to lock the RegClient",
            pRegClient));
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientGetSecObj(&pRegClient->transcClient, phSecObj);

    RegClientUnLockAPI(pRegClient);
    return rv;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * RvSipRegClientSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the regClient application handle. Usually the application
 *          replaces handles with the stack in the
 *          RvSipRegClientMgrCreateRegClient() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient    - Handle to the regClient.
 *            hAppRegClient - A new application handle to the regClient.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetAppHandle (
                                      IN  RvSipRegClientHandle     hRegClient,
                                      IN  RvSipAppRegClientHandle  hAppRegClient)
{
    RvStatus               rv;
    RegClient   *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the regClient*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientSetAppHandle - Setting regClient 0x%p Application handle 0x%p",
              pRegClient,hAppRegClient));

    pRegClient->hAppRegClient = hAppRegClient;

	RegClientUnLockAPI(pRegClient);

    return RV_OK;
}

/***************************************************************************
 * RvSipRegClientGetAppHandle
 * ------------------------------------------------------------------------
 * General: Gets the regClient application handle. Usually the application
 *          replaces handles with the stack in the
 *          RvSipRegClientMgrCreateRegClient() API function.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient    - Handle to the regClient.
 *            hAppRegClient - A new application handle to the regClient.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetAppHandle (
                                      IN  RvSipRegClientHandle     hRegClient,
                                      IN  RvSipAppRegClientHandle *phAppRegClient)
{
    RvStatus      rv;
    RegClient    *pRegClient = (RegClient*)hRegClient;

    if(pRegClient == NULL || phAppRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the regClient*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phAppRegClient = pRegClient->hAppRegClient;

	RegClientUnLockAPI(pRegClient);

    return rv;
}

/***************************************************************************
 * RvSipRegClientSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the register-client to a SigComp compartment.
 *          The register-client will use this compartment in the message
 *          compression process.
 *          Note The message will be compressed only if the remote URI includes the
 *          comp=sigcomp parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient   - The handle to the register client.
 *            hCompartment - Handle to the SigComp Compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetCompartment(
                            IN RvSipRegClientHandle   hRegClient,
                            IN RvSipCompartmentHandle hCompartment)
{
    RV_UNUSED_ARG(hRegClient);
    RV_UNUSED_ARG(hCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipRegClientGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the SigComp compartment associated with the register-client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hRegClient    - The handle to the register client.
 * Output: phCompartment - The handle to the SigComp compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCompartment(
                            IN  RvSipRegClientHandle    hRegClient,
                            OUT RvSipCompartmentHandle *phCompartment)
{
    *phCompartment = NULL;

    RV_UNUSED_ARG(hRegClient) 

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipRegClientDisableCompression
 * ------------------------------------------------------------------------
 * General:Disables the compression mechanism in a register-client.
 *         This means that even if the message indicates compression,
 *         it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hRegClient    - The handle to the register client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientDisableCompression(
                                      IN  RvSipRegClientHandle hRegClient)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv          = RV_OK;
    RegClient   *pRegClient = (RegClient*)hRegClient;

    if(pRegClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RegClientLockAPI(pRegClient); /* try to lock the regClient */
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTranscClientDisableCompression(&pRegClient->transcClient);

	RegClientUnLockAPI(pRegClient);

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hRegClient);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipRegClientGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: The function retrieve the auth-object that is currently being
 *          processed by the authenticator.
 *          (for application usage in the RvSipAuthenticatorGetSharedSecretEv
 *          callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient          - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetCurrProcessedAuthObj (
                                      IN   RvSipRegClientHandle    hRegClient,
                                      OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    RegClient     *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetCurrProcessedAuthObj(&pRegClient->transcClient, phAuthObj);

	RegClientUnLockAPI(pRegClient);
    return rv;
}
/***************************************************************************
 * RvSipRegClientAuthObjGet
 * ------------------------------------------------------------------------
 * General: The function retrieve auth-objects from the list in the Reg-Client.
 *          you may get the first/last/next object.
 *          in case of getting the next object, please supply the current
 *          object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient        - Handle to the Reg-Client.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientAuthObjGet (
                                      IN   RvSipRegClientHandle     hRegClient,
                                      IN   RvSipListLocation      eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    RegClient     *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the Reg-Client*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientAuthObjGet(&pRegClient->transcClient, eLocation, hRelativeAuthObj, phAuthObj);

	RegClientUnLockAPI(pRegClient);
    return rv;
}

/***************************************************************************
 * RvSipRegClientAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an auth-obj from the list in the Reg-Client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient  - Handle to the Reg-Client.
 *            hAuthObj    - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientAuthObjRemove (
                                      IN   RvSipRegClientHandle   hRegClient,
                                      IN   RvSipAuthObjHandle   hAuthObj)
{
    RvStatus     rv;
    RegClient     *pRegClient = (RegClient*)hRegClient;
    if(pRegClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the Reg-Client*/
    rv = RegClientLockAPI(pRegClient);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	rv = SipTranscClientAuthObjRemove(&pRegClient->transcClient, hAuthObj);

	RegClientUnLockAPI(pRegClient);
    return rv;
}
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipRegClientSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header in the outgoing request.
 *          An initial authorization header is a header that contains
 *          only the client private identity, and not real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The reg-client will set the initial header to the message, only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient     - The register-client handle.
 *            hAuthorization - The Authorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetInitialAuthorization (
                                         IN RvSipRegClientHandle hRegClient,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization)
{
    RegClient          *pRegClient;
    RvStatus            retStatus;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRegClient = (RegClient *)hRegClient;
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
        "RvSipRegClientSetInitialAuthorization - Setting in client 0x%p ",pRegClient));

    /* Assert that the current state allows this action */
    if (RVSIP_REG_CLIENT_STATE_IDLE != pRegClient->eState &&
        RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED != pRegClient->eState)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RvSipRegClientSetInitialAuthorization - Register client 0x%p - illegal state %s",
            pRegClient, RegClientStateIntoString(pRegClient->eState)));
        RegClientUnLockAPI(pRegClient);
        return RV_ERROR_ILLEGAL_ACTION;
    }

	retStatus = SipTranscClientSetInitialAuthorization(&pRegClient->transcClient, hAuthorization);
    
    RegClientUnLockAPI(pRegClient);
    return retStatus;
}

/***************************************************************************
 * RvSipRegClientSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this reg-client. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the reg-client will take upon itself this agreement and the data
 *          it brings. If not, the reg-client will use this security-agreement
 *          object to negotiate an agreement with the remote party.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient   - Handle to the reg-client.
 *          hSecAgree    - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientSetSecAgree(
							IN  RvSipRegClientHandle         hRegClient,
							IN  RvSipSecAgreeHandle          hSecAgree)
{
	RegClient        *pRegClient;
	RvStatus          rv;

    if (hRegClient == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pRegClient = (RegClient *)hRegClient;

	if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* if the reg-client was terminated we do not allow the attachment */
	if (RVSIP_REG_CLIENT_STATE_TERMINATED == pRegClient->eState)
	{
		RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
					"RvSipRegClientSetSecAgree - reg-client 0x%p - Attaching security-agreement 0x%p is forbidden in state Terminated",
					pRegClient, hSecAgree));
		RegClientUnLockAPI(pRegClient);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SipTranscClientSetSecAgree(&pRegClient->transcClient, hSecAgree);

	RegClientUnLockAPI(pRegClient);

	return rv;
}

/***************************************************************************
 * RvSipRegClientGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the reg-client.
 *          The security-agreement object captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient    - Handle to the reg-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetSecAgree(
									IN   RvSipRegClientHandle    hRegClient,
									OUT  RvSipSecAgreeHandle    *phSecAgree)
{
	RegClient          *pRegClient = (RegClient *)hRegClient;
	RvStatus			rv;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    if (RV_OK != RegClientLockAPI(pRegClient))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTranscClientGetSecAgree(&pRegClient->transcClient, phSecAgree);

	RegClientUnLockAPI(pRegClient);

	return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RegClientGetNewContactHeaderHandle
 * ------------------------------------------------------------------------
 * General: Allocated a new contact header handle in the register-client
 *          object'a list of Contact headers.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New contact header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - Handle to the register-client.
 *            phContactHeader - Handle to the new Contact header handle.
 ***************************************************************************/
static RvStatus RVCALLCONV RegClientGetNewContactHeaderHandle (
                                 IN  RegClient                 *pRegClient,
                                 OUT RvSipContactHeaderHandle **phContactHeader)
{
    RvStatus retStatus;

    if (NULL == pRegClient->hContactList)
    {
        /* Construct the Contact headers list if this is the first header to
           be inserted */
        pRegClient->hContactList = RLIST_RPoolListConstruct(
                                       pRegClient->pMgr->hGeneralMemPool,
                                       pRegClient->hContactPage, sizeof(void*),
                                       pRegClient->pMgr->pLogSrc);
        if (NULL == pRegClient->hContactList)
        {
            RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientGetNewContactHeaderHandle - Register client 0x%p: Error setting Contact header",
                  pRegClient));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    /* Allocate a new Contact header handle in the end of the Contact headers list */
    retStatus = RLIST_InsertTail(NULL, pRegClient->hContactList,
                                 (RLIST_ITEM_HANDLE *)phContactHeader);
    if (RV_OK != retStatus)
    {
         RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientGetNewContactHeaderHandle - Register client 0x%p: Error setting Contact header",
                  pRegClient));
    }
    return retStatus;
}


/***************************************************************************
 * RegClientCopyContactHeader
 * ------------------------------------------------------------------------
 * General: Construct the destination Contact header and Copy the source to
 *          it.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Header copied successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phDestContactHeader - Pointer to the destination Contact handle.
 *            hSourceHeader - Handle to the source Contact header.
 ***************************************************************************/
static RvStatus RVCALLCONV RegClientCopyContactHeader (
                           IN  RegClient                *pRegClient,
                           IN  RvSipContactHeaderHandle *phDestContactHeader,
                           IN  RvSipContactHeaderHandle  hSourceHeader)
{
    RvStatus retStatus;

    /* Construct a new Contact header on the place allocated for it in the list */
    retStatus = RvSipContactHeaderConstruct(pRegClient->pMgr->hMsgMgr,
                                            pRegClient->pMgr->hGeneralMemPool,
                                            pRegClient->hContactPage,
                                            phDestContactHeader);
    if (RV_OK != retStatus)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCopyContactHeader - Register client 0x%p: Error setting Contact header",
                  pRegClient));
        RLIST_Remove(NULL, pRegClient->hContactList,
                     (RLIST_ITEM_HANDLE)phDestContactHeader);
        return retStatus;
    }
    /* Copy the given Contact header to the newly constructed Contact header
       of the register-client object */
    retStatus = RvSipContactHeaderCopy(*phDestContactHeader, hSourceHeader);
    if (RV_OK != retStatus)
    {
         RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCopyContactHeader - Register client 0x%p: Error setting Contact header",
                  pRegClient));
        RLIST_Remove(NULL, pRegClient->hContactList,
                     (RLIST_ITEM_HANDLE)phDestContactHeader);
    }
    return retStatus;
}


/*-----------------------------------------------------------------------
       R E G - C L I E N T:  D E P R E C A T E D    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipRegClientGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipRegClientGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - Handle to the register-client object.
 * Output:     phParty - Handle to the newly created party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetNewPartyHeaderHandle (
                                      IN   RvSipRegClientHandle     hRegClient,
                                      OUT  RvSipPartyHeaderHandle  *phParty)
{
    return RvSipRegClientGetNewMsgElementHandle(hRegClient, RVSIP_HEADERTYPE_FROM,
                                            RVSIP_ADDRTYPE_UNDEFINED, (void**)phParty);
}


/***************************************************************************
 * RvSipRegClientGetNewContactHeaderHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipRegClientGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient   - Handle to the register-client.
 * Output:     phContact    - Handle to the newly created Contact header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetNewContactHeaderHandle (
                                    IN   RvSipRegClientHandle      hRegClient,
                                    OUT  RvSipContactHeaderHandle *phContact)
{
	RvStatus  rv;
	RegClient    *pRegClient = (RegClient*)hRegClient;

    if(hRegClient == NULL || phContact == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
	*phContact = NULL;
	
	rv = RegClientLockAPI(pRegClient); /*try to lock the reg-client*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientGetNewContactHeaderHandle - getting new contact handle from reg-client 0x%p",
               pRegClient));

    rv = RvSipContactHeaderConstruct(pRegClient->pMgr->hMsgMgr, pRegClient->pMgr->hGeneralMemPool, 
			pRegClient->hContactPage, phContact) ;
			
    
    if (rv != RV_OK || *phContact == NULL)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RvSipRegClientGetNewContactHeaderHandle - reg-client 0x%p - Failed to create new header (rv=%d)",
              pRegClient, rv));
        
    }
	RegClientUnLockAPI(pRegClient);
		
    return rv;

    
}

/***************************************************************************
 * RvSipRegClientGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipRegClientGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient   - Handle to the register-client.
 *          eAddrType    - The type of address the application wishes to create.
 * Output:     phRequestUri - Handle to the newly created address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetNewAddressHandle (
                                    IN   RvSipRegClientHandle  hRegClient,
                                    IN   RvSipAddressType      eAddrType,
                                    OUT  RvSipAddressHandle    *phRequestUri)
{
    return RvSipRegClientGetNewMsgElementHandle(hRegClient, RVSIP_HEADERTYPE_UNDEFINED,
                                            eAddrType, (void**)phRequestUri);
}


/***************************************************************************
 * RvSipRegClientGetNewExpiresHeaderHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipRegClientGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient   - Handle to the register-client.
 * Output:     phExpires    - Handle to the newly created Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRegClientGetNewExpiresHeaderHandle (
                                    IN   RvSipRegClientHandle      hRegClient,
                                    OUT  RvSipExpiresHeaderHandle *phExpires)
{
    return RvSipRegClientGetNewMsgElementHandle(hRegClient, RVSIP_HEADERTYPE_EXPIRES,
                                            RVSIP_ADDRTYPE_UNDEFINED, (void**)phExpires);
}
#endif /* RV_SIP_PRIMITIVES */






















