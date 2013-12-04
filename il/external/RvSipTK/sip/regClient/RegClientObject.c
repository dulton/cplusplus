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
 *                              <RegClientObject.c>
 *
 * This file defines the RegClient object, attributes and interface functions.
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
#include "_SipCommonConversions.h"
#include "_SipRegClientMgr.h"
#include "RegClientMgrObject.h"
#include "RegClientObject.h"
#include "RvSipRegClient.h"
#include "RegClientObject.h"
#include "RegClientCallbacks.h"

#include "_SipTranscClient.h"

#include "AdsRlist.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticator.h"
#include "_SipTransport.h"

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
#define REG_CLIENT_STATE_TERMINATED_STR "Terminated"
#define REG_CLIENT_OBJECT_NAME_STR		"Reg-Client"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar *RVCALLCONV RegClientReasonIntoString(
                                   IN RvSipRegClientStateChangeReason eReason);
#endif /* ##if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RegClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new register-client object. The register-client
 *          asumes the idle state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   RegClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - pointer to the register-clients manager.
 *          pRegClient - pointer to the new register-client object.
 *          hAppRegClient - The API's handle to the register-client.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientInitiate(
                                  IN RegClientMgr           *pRegClientMgr,
                                  IN RegClient              *pRegClient,
                                  IN RvSipAppRegClientHandle hAppRegClient)
{
    RvStatus retStatus;
	
	pRegClient->eState          = RVSIP_REG_CLIENT_STATE_IDLE;
    pRegClient->hAppRegClient   = hAppRegClient;
    pRegClient->regClientEvHandlers = &(pRegClientMgr->regClientEvHandlers);
	pRegClient->hContactList    = NULL;
	
	pRegClient->bIsDetach        = RV_FALSE;
    
	
    /* Get a new memory page for the register-client object */
    retStatus = RPOOL_GetPage(pRegClientMgr->hGeneralMemPool, 0,
                              &pRegClient->hContactPage);
    if(retStatus != RV_OK)
    {
        return retStatus;
    }

    retStatus = SipTranscClientInitiate(pRegClientMgr->hTranscClientMgr, 
										(SipTranscClientOwnerHandle)pRegClient, 
										RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT, 
										&pRegClient->transcClient);

    return retStatus;
}

/***************************************************************************
 * RegClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a register-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - pointer to the register-clients manager.
 *          pRegClient - pointer to the register-client object.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientDestruct(IN RegClient *pRegClient)
{
    RegClientMgr       *pRegClientMgr;
    RvStatus          rv = RV_OK;
    if (NULL == pRegClient)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
              "RegClientDestruct - Register Client 0x%p: was destructed",
              pRegClient));

	rv = SipTranscClientDestruct(&pRegClient->transcClient);
	

    /* Remove the register-client object from the list of register-clients in
       the register-clients manager */
    pRegClientMgr = pRegClient->pMgr;
    RegClientMgrLock(pRegClientMgr);
    /* Free the memory page */
    RPOOL_FreePage(pRegClient->pMgr->hGeneralMemPool, pRegClient->hContactPage);

    RLIST_Remove(pRegClient->pMgr->hRegClientListPool,
                 pRegClient->pMgr->hRegClientList,
                 (RLIST_ITEM_HANDLE)pRegClient);
    RegClientMgrUnLock(pRegClientMgr);
    return RV_OK;
}

/***************************************************************************
 * RegClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the register-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the register-client.
 *        eReason - The reason for the state change.
 *        pRegClient - The register-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV RegClientChangeState(
                            IN  RvSipRegClientState             eNewState,
                            IN  RvSipRegClientStateChangeReason eReason,
                            IN  RegClient                       *pRegClient)
{
    if(eNewState == RVSIP_REG_CLIENT_STATE_TERMINATED)
    {

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientChangeState - Register-client 0x%p state changed to Terminated, (reason = %s)",
                   pRegClient,
                   RegClientReasonIntoString(eReason)));
    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                 "RegClientChangeState - Register-client 0x%p state changed: %s->%s, (reason = %s)",
                   pRegClient, RegClientStateIntoString(pRegClient->eState),
                   RegClientStateIntoString(eNewState),
                  RegClientReasonIntoString(eReason)));
    }
    /* Change the register-client's state */
    pRegClient->eState = eNewState;

    /* Call the event state changed that was set to the register-clients
       manager */
    RegClientCallbackStateChangeEv(eNewState, eReason, pRegClient);
}

/***************************************************************************
 * RegClientGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The register-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 *          hPage - The memory page to use.
 * Output:  pstrCallId - Pointer to the call-id offset
 *          ppCallId - Pointer to the call-id pointer offset (NULL when sip-debug)
 ***************************************************************************/
RvStatus RVCALLCONV RegClientGenerateCallId(
                                         IN  RegClientMgr            *pRegClientMgr,
                                         IN  HPAGE                    hPage,
                                         OUT RvInt32                *pstrCallId,
                                         OUT RvChar                **ppCallId)
{
    RvStatus           rv;
    HRPOOL              pool      = pRegClientMgr->hGeneralMemPool;
    RvInt32            offset;
    RvChar             strCallId[SIP_COMMON_ID_SIZE];
    RvInt32            callIdSize;

    if (NULL_PAGE == pRegClientMgr->hMemoryPage)
    {
        pRegClientMgr->strCallId = UNDEFINED;
#ifdef SIP_DEBUG
        pRegClientMgr->pCallId = NULL;
#endif
        return RV_OK;
    }

    rv = SipTransactionGenerateIDStr(pRegClientMgr->hTransportMgr,
                             pRegClientMgr->seed,
                             (void*)pRegClientMgr,
                             pRegClientMgr->pLogSrc,
                             (RvChar*)strCallId);
    if(rv != RV_OK)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
             "RegClientGenerateCallId - Failed"));
    }

    callIdSize = (RvInt32)strlen(strCallId);
    /* allocate a buffer on the register-client manager page that will be
       used for the call-id. */
    rv = RPOOL_AppendFromExternal(pool,hPage,strCallId,callIdSize+1, RV_FALSE, &offset,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pRegClientMgr->pLogSrc,(pRegClientMgr->pLogSrc,
             "RegClientGenerateCallId - regClientMgr 0x%p failed in RPOOL_AppendFromExternal (rv=%d)",
			 pRegClientMgr,rv));
        return rv;
    }
    /* Get the pointer of the new allocated memory */
    *pstrCallId = offset;
#ifdef SIP_DEBUG
    *ppCallId = RPOOL_GetPtr(pool, hPage, offset);
#else
	RV_UNUSED_ARG(ppCallId)
#endif
    return RV_OK;
}


/***************************************************************************
 * RegClientAddContactsToMsg
 * ------------------------------------------------------------------------
 * General: Inserts to the message the list of Contact headers assosiated
 *          with this register-client.
 * Return Value: RV_ERROR_NULLPTR - pTransaction or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pRegClient - The handle to the register-client.
 *          hMsg - The message to insert the Contact headers to.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientAddContactsToMsg(
                                            IN RegClient     *pRegClient,
                                            IN RvSipMsgHandle hMsg)
{
    RvStatus           retStatus;
    RvUint32            safeCounter;
    RvUint32            maxLoops;

    retStatus = RV_OK;
    if (NULL != pRegClient->hContactList)
    {
        /* The Contacts list is not empty */
        RvSipHeaderListElemHandle hPos;
        RLIST_ITEM_HANDLE         hElement;
        void                     *hMsgListElem;

        /* Go over the list of Contact headers in the register-client, and
           insert each of this headers to the message. */
        hPos = NULL;
        safeCounter = 0;
        maxLoops = 10000;
        RLIST_GetHead(NULL, pRegClient->hContactList, &hElement);
        while (NULL != hElement)
        {
            /* set contact header to the message */
            retStatus = RvSipMsgPushHeader(
                                    hMsg, RVSIP_LAST_HEADER,
                                    (void *)(*((RvSipContactHeaderHandle *)hElement)),
                                    RVSIP_HEADERTYPE_CONTACT, &hPos,
                                    (void **)&hMsgListElem);
            if (RV_OK != retStatus)
            {
                RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                          "RegClientAddContactsToMsg - Error trying to insert register-client's (0x%p) Contact header to a message to be sent",
                          pRegClient));
                retStatus = RV_ERROR_OUTOFRESOURCES;
                break;
            }
            RLIST_GetNext(NULL, pRegClient->hContactList, hElement, &hElement);
            if (safeCounter > maxLoops)
            {
                RvLogExcep(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                          "RegClientAddContactsToMsg - Error in loop - number of rounds is larger than number of Contact headers, regClient 0x%p",
                          pRegClient));
                retStatus = RV_ERROR_UNKNOWN;
                break;
            }
            safeCounter++;
        }
    }
    return retStatus;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * RegClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
RvStatus RVCALLCONV RegClientLockAPI(IN  RegClient *pRegClient)
{
	if (pRegClient == NULL)
	{
		return RV_ERROR_NULLPTR;
	}
	return SipTranscClientLockAPI(&pRegClient->transcClient);
}


/************************************************************************************
 * RegClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
void RVCALLCONV RegClientUnLockAPI(IN  RegClient *pRegClient)
{
	if (pRegClient == NULL)
	{
		return;
	}
	SipTranscClientUnLockAPI(&pRegClient->transcClient);
}

/************************************************************************************
 * RegClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
RvStatus RVCALLCONV RegClientLockMsg(IN  RegClient *pRegClient)
{
	if (pRegClient == NULL)
	{
		return RV_ERROR_NULLPTR;
	}
	return SipTranscClientLockMsg(&pRegClient->transcClient);
}


/************************************************************************************
 * RegClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks regclient according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
void RVCALLCONV RegClientUnLockMsg(IN  RegClient *pRegClient)
{
	if (pRegClient == NULL)
	{
		return;
	}
	SipTranscClientUnLockMsg(&pRegClient->transcClient);
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
 * RegClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the register-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV RegClientMgrLock(IN RegClientMgr *pRegClientMgr)
{
    if (NULL == pRegClientMgr)
    {
        return;
    }
    RvMutexLock(&pRegClientMgr->hLock,pRegClientMgr->pLogMgr);
}


/***************************************************************************
 * RegClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the register-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV RegClientMgrUnLock(IN RegClientMgr *pRegClientMgr)
{
    if (NULL == pRegClientMgr)
    {
        return;
    }
    RvMutexUnlock(&pRegClientMgr->hLock,pRegClientMgr->pLogMgr);
}


/***************************************************************************
 * RegClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a RegClient object, Free the resources taken by it and
 *          remove it from the manager reg-client list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - Pointer to the reg-client to terminate.
 *          eReason - termination reason
 ***************************************************************************/
void RVCALLCONV RegClientTerminate(IN RegClient *pRegClient,
                                   RvSipRegClientStateChangeReason eReason)
{
    if(pRegClient->eState == RVSIP_REG_CLIENT_STATE_TERMINATED)
    {
        return;
    }
    pRegClient->eState = RVSIP_REG_CLIENT_STATE_TERMINATED;

	SipTranscClientTerminate(&pRegClient->transcClient);

	RV_UNUSED_ARG(eReason)
    return;
}

/***************************************************************************
 * RegClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the RegClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient    - pointer to RegClient
 ***************************************************************************/
void RegClientDetachFromConnection(IN RegClient        *pRegClient)
{
    SipTranscClientDetachFromConnection(&pRegClient->transcClient);
}

/***************************************************************************
 * RegClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the RegClient as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient    - pointer to RegClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus RegClientAttachOnConnection(IN RegClient                        *pRegClient,
									 IN RvSipTransportConnectionHandle  hConn)
{

    RvStatus rv = RV_OK;
    
	if(rv != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RegClientAttachOnConnection - Failed to attach RegClient 0x%p to connection 0x%p",pRegClient,hConn));
        SipTranscClientAttachOnConnection(&pRegClient->transcClient, NULL);
    }
    else
    {
        SipTranscClientAttachOnConnection(&pRegClient->transcClient, hConn);
    }
    return rv;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RegClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Reg-Client.
 *          As a result of this operation, all messages, sent by this
 *          Reg-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pRegClient - Pointer to the Reg-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus RegClientSetSecObj(IN  RegClient*        pRegClient,
                            IN  RvSipSecObjHandle hSecObj)
{
    RvStatus      rv;

	rv = SipTranscClientSetSecObj(&pRegClient->transcClient, hSecObj);
    
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/


/******************************************************************************
 * RegClientSetAlertTimerIfNeeded
 * ----------------------------------------------------------------------------
 * General: This method set a timer for the transcClient to invoke when the object is expired.
 * Return Value: 
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pRegClient	- Pointer to the Reg-Client object.
 *          hMsg		- The received message, used to get the expires header.
 *****************************************************************************/
void RegClientSetAlertTimerIfNeeded(IN RegClient					*pRegClient, 
									IN  RvSipMsgHandle				hMsg)
{

	RvSipHeaderListElemHandle  hListElem;
	RvSipExpiresHeaderHandle   hExpiresHeader;
	RvUint32				   expiresValue = 0;
	RvInt32 				   timeToSet;
	RvStatus				   rv;
	
	if (pRegClient->eState != RVSIP_REG_CLIENT_STATE_REGISTERING || 
		pRegClient->pMgr->bHandleExpTimer == RV_FALSE)
	{
		return;
	}
	
	
	hExpiresHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EXPIRES, RVSIP_FIRST_HEADER, &hListElem);
	
	if (hExpiresHeader == NULL)
	{
		RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
			"RegClientSetAlertTimerIfNeeded - missing Expires header on incoming message 0x%p, cannot set alert timer!", hMsg));
		return;
	}

	rv = RvSipExpiresHeaderGetDeltaSeconds(hExpiresHeader, &expiresValue);
	if (rv != RV_OK)
	{
		RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
			"RegClientSetAlertTimerIfNeeded - Unable to get expires value from 0x%p, cannot set alert timer!", hExpiresHeader));
		return;
	}

	if (pRegClient->pMgr->regClientAlertTimeOut >= expiresValue*1000)
	{
		RvLogInfo(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
			"RegClientSetAlertTimerIfNeeded - Register-client 0x%p alertTimeout %d (milli) is greater then expires header value %d (milli), Setting timer to Expires value.", pRegClient, pRegClient->pMgr->regClientAlertTimeOut, expiresValue*1000));
		timeToSet = expiresValue*1000;
	}
	else
	{
		/*we are multiplying the epiresValue with 1000 to convert it into milliseconds.*/
		timeToSet = expiresValue*1000 - pRegClient->pMgr->regClientAlertTimeOut;
	}
	

	SipTranscClientSetAlertTimer(&pRegClient->transcClient, timeToSet);
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RegClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV RegClientStateIntoString(
                                            IN RvSipRegClientState eNewState)
{
    switch (eNewState)
    {
    case RVSIP_REG_CLIENT_STATE_IDLE:
        return "Idle";
    case RVSIP_REG_CLIENT_STATE_TERMINATED:
        return REG_CLIENT_STATE_TERMINATED_STR;
    case RVSIP_REG_CLIENT_STATE_REGISTERING:
        return "Registering";
    case RVSIP_REG_CLIENT_STATE_REDIRECTED:
        return "Redirected";
    case RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED:
        return "Unauthenticated";
    case RVSIP_REG_CLIENT_STATE_REGISTERED:
        return "Registered";
    case RVSIP_REG_CLIENT_STATE_FAILED:
        return "Failed";
    case RVSIP_REG_CLIENT_STATE_MSG_SEND_FAILURE:
        return "Msg Send Failure";
    default:
        return "Undefined";
    }
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * RegClientReasonIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the reason received as enumeration.
 * Return Value: The reason string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason enumeration.
 ***************************************************************************/
static RvChar *RVCALLCONV RegClientReasonIntoString(
                                   IN RvSipRegClientStateChangeReason eReason)
{
    switch (eReason)
    {
    case RVSIP_REG_CLIENT_REASON_UNDEFINED:
        return "Undefined";
    case RVSIP_REG_CLIENT_REASON_USER_REQUEST:
        return "User request";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD:
        return "Response successful received";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD:
        return "Response redirection received";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD:
        return "Response unauthentication received";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
        return "Response request failure received";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD:
        return "Response server failure received";
    case RVSIP_REG_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
        return "Response global failure received";
    case RVSIP_REG_CLIENT_REASON_LOCAL_FAILURE:
        return "Local failure";
    case RVSIP_REG_CLIENT_REASON_TRANSACTION_TIMEOUT:
        return "Transaction timeout";
    case RVSIP_REG_CLIENT_REASON_NORMAL_TERMINATION:
        return "Normal termination";
    case RVSIP_REG_CLIENT_REASON_GIVE_UP_DNS:
        return "Give Up DNS";
    case RVSIP_REG_CLIENT_REASON_NETWORK_ERROR:
        return "Network Error";
    case RVSIP_REG_CLIENT_REASON_503_RECEIVED:
        return "503 Received";
    case RVSIP_REG_CLIENT_REASON_CONTINUE_DNS:
        return "Continue DNS";
    default:
        return "";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
#endif /* RV_SIP_PRIMITIVES */











































