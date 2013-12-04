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
 *                              <TranscClientObject.c>
 *
 * This file defines the TranscClient object, attributes and interface functions.
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
#include "_SipCommonConversions.h"
#include "_SipTranscClientMgr.h"
#include "TranscClientMgrObject.h"
#include "TranscClientObject.h"
#include "TranscClientCallbacks.h"
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
#include "TranscClientSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "_SipSecuritySecObj.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define TRANSC_CLIENT_STATE_TERMINATED_STR "Terminated"
#define TRANSC_CLIENT_OBJECT_NAME_STR		"Transc-Client"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar *RVCALLCONV TranscClientReasonIntoString(
													IN SipTranscClientStateChangeReason eReason);

#endif /* ##if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static void TranscClientAlertTimerRelease(SipTranscClient *pTranscClient);

static RvStatus RVCALLCONV TerminateTranscClientEventHandler(IN void      *obj,
                                                             IN RvInt32   reason);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * SipTranscClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new transaction-client object. The transaction-client
 *          asumes the idle state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   TranscClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClientMgr  - pointer to the transaction-clients manager.
 *			  eOwnerType		- The owner type of the client.
 *            pTranscClient		- pointer to the new transaction-client object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientInitiate(
                                  IN SipTranscClientMgrHandle				hTranscClientMgr,
								  IN SipTranscClientOwnerHandle				hOwner,
								  IN RvSipCommonStackObjectType				eOwnerType,
								  IN SipTranscClient						*pTranscClient)

{
	RvStatus retStatus;
	TranscClientMgr			*pTranscClientMgr;

	/*Initiate the event handler of this client for connection events*/
	RvSipTransportConnectionEvHandlers evHandler;
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler = TranscClientConnStateChangedEv;

	pTranscClientMgr = (TranscClientMgr*)hTranscClientMgr;


	
	if (hTranscClientMgr == NULL || pTranscClient == NULL)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientInitiate - Unable to initiate transcClient"));
		return RV_ERROR_INVALID_HANDLE;
	}

    memset(&(pTranscClient->localAddr),0,sizeof(SipTransportObjLocalAddresses));
    SipTransportInitObjectOutboundAddr(pTranscClient->pMgr->hTransportMgr,
                                      &pTranscClient->outboundAddr);
    pTranscClient->bIsPersistent             = pTranscClientMgr->bIsPersistent;
    pTranscClient->hTcpConn       = NULL;
	SipCommonCSeqSetStep(0,&pTranscClient->cseq); 
    pTranscClient->hActiveTransc   = NULL;
    pTranscClient->hRequestUri     = NULL;
    pTranscClient->hExpireHeader   = NULL;
    pTranscClient->hFromHeader     = NULL;
    pTranscClient->hToHeader       = NULL;
    pTranscClient->hReceivedMsg    = NULL;
    pTranscClient->hOutboundMsg    = NULL;
	pTranscClient->eOwnerType = eOwnerType;
	pTranscClient->hOwner	  = hOwner;
	pTranscClient->eState	  = SIP_TRANSC_CLIENT_STATE_IDLE;

	pTranscClient->bWasActivated = RV_FALSE;

	memset(&(pTranscClient->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));
	

	SipCommonCoreRvTimerInit(&pTranscClient->alertTimer);
	pTranscClient->bWasAlerted = RV_FALSE;

	memcpy(&pTranscClient->transportEvHandler, &evHandler, sizeof(RvSipTransportConnectionEvHandlers));
		    
    pTranscClient->strCallId        = UNDEFINED;
	pTranscClient->bUseFirstRouteRequest = RV_FALSE;
#ifdef RV_SIP_AUTH_ON
    pTranscClient->pAuthListInfo = NULL;
    pTranscClient->hListAuthObj  = NULL;
#endif /* #ifdef RV_SIP_AUTH_ON */

#if defined(UPDATED_BY_SPIRENT)
    pTranscClient->cctContext = -1;
    pTranscClient->URI_cfg = 0xff;
#endif

#ifdef SIP_DEBUG
    pTranscClient->pCallId = NULL;
#endif
#ifdef RV_SIGCOMP_ON
    pTranscClient->bSigCompEnabled     = RV_TRUE;
#endif
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpInitializeMsgSendingParams(&pTranscClient->sctpParams);
#endif
#ifdef RV_SIP_IMS_ON    
    pTranscClient->hInitialAuthorization = NULL;
	pTranscClient->hSecAgree             = NULL;
	pTranscClient->hSecObj               = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/
    
    pTranscClient->pTimers = NULL;

    /* Get a new memory page for the transaction-client object */
    retStatus = RPOOL_GetPage(pTranscClientMgr->hGeneralMemPool, 0,
                              &pTranscClient->hPage);
    if(retStatus != RV_OK)
    {
        return retStatus;
    }

	pTranscClient->tripleLock = &pTranscClient->transcClientTripleLock;

    retStatus = SipTransportLocalAddressInitDefaults(pTranscClientMgr->hTransportMgr,
                                                     &pTranscClient->localAddr);
    if (RV_OK != retStatus)
    {
        return retStatus;

    }

    /* Set unique identifier only on initialization success */
	RvRandomGeneratorGetInRange(pTranscClient->pMgr->seed, RV_INT32_MAX,
        (RvRandom*)&pTranscClient->transcClientUniqueIdentifier);

    return retStatus;
}

/***************************************************************************
 * SipTranscClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a transaction-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClientMgr - pointer to the transaction-clients manager.
 *          pTranscClient - pointer to the transaction-client object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDestruct(IN SipTranscClient		*pTranscClient)
{
    TranscClientMgr       *pTranscClientMgr;
    RvStatus          rv = RV_OK;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientDestruct - Transaction Client 0x%p: was destructed",
              pTranscClient));

    pTranscClient->transcClientUniqueIdentifier = 0;

	TranscClientAlertTimerRelease(pTranscClient);
    
#ifdef RV_SIP_AUTH_ON
    if (NULL != pTranscClient->hListAuthObj)
    {
        SipAuthenticatorAuthListResetPasswords(pTranscClient->hListAuthObj);
        SipAuthenticatorAuthListDestruct(pTranscClient->pMgr->hAuthModule, pTranscClient->pAuthListInfo,
                                      pTranscClient->hListAuthObj);
        pTranscClient->hListAuthObj = NULL;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    if(pTranscClient->hTcpConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(pTranscClient->hTcpConn,
            (RvSipTransportConnectionOwnerHandle)pTranscClient);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientDestruct - Failed to detach pTranscClient 0x%p from connection 0x%p",pTranscClient,pTranscClient->hTcpConn));

        }
        pTranscClient->hTcpConn = NULL;
    }

    /* If the application created a message in advance destruct it*/
    if (NULL != pTranscClient->hOutboundMsg)
    {
        RvSipMsgDestruct(pTranscClient->hOutboundMsg);
        pTranscClient->hOutboundMsg = NULL;
    }

#ifdef RV_SIP_IMS_ON 
    /* Detach from Security Object */
    if (NULL != pTranscClient->hSecObj)
    {
        rv = TranscClientSetSecObj(pTranscClient, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
                "SipTranscClientDestruct - TranscClient 0x%p: Failed to unset Security Object %p",
                pTranscClient, pTranscClient->hSecObj));
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */


    /* Remove the transaction-client object from the list of transaction-clients in
       the transaction-clients manager */
    pTranscClientMgr = pTranscClient->pMgr;
    TranscClientMgrLock(pTranscClientMgr);
    /* Free the memory page */
    RPOOL_FreePage(pTranscClient->pMgr->hGeneralMemPool, pTranscClient->hPage);

    TranscClientMgrUnLock(pTranscClientMgr);

    return RV_OK;
}

/***************************************************************************
 * TranscClientGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The transaction-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClientMgr - Pointer to the transaction-clients manager.
 *          hPage - The memory page to use.
 * Output:  pstrCallId - Pointer to the call-id offset
 *          ppCallId - Pointer to the call-id pointer offset (NULL when sip-debug)
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientGenerateCallId(
                                         IN  TranscClientMgr            *pTranscClientMgr,
                                         IN  HPAGE                    hPage,
                                         OUT RvInt32                *pstrCallId,
                                         OUT RvChar                **ppCallId)
{
    RvStatus           rv;
    HRPOOL              pool      = pTranscClientMgr->hGeneralMemPool;
    RvInt32            offset;
    RvChar             strCallId[SIP_COMMON_ID_SIZE];
    RvInt32            callIdSize;

    rv = SipTransactionGenerateIDStr(pTranscClientMgr->hTransportMgr,
                             pTranscClientMgr->seed,
                             (void*)pTranscClientMgr,
                             pTranscClientMgr->pLogSrc,
                             (RvChar*)strCallId);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
             "TranscClientGenerateCallId - Failed"));
    }

    callIdSize = (RvInt32)strlen(strCallId);
    /* allocate a buffer on the transaction-client manager page that will be
       used for the call-id. */
    rv = RPOOL_AppendFromExternal(pool,hPage,strCallId,callIdSize+1, RV_FALSE, &offset,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClientMgr->pLogSrc,(pTranscClientMgr->pLogSrc,
             "TranscClientGenerateCallId - transcClientMgr 0x%p failed in RPOOL_AppendFromExternal (rv=%d)",
			 pTranscClientMgr,rv));
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
 * TranscClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Call-Id of the transc-client object.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was isdufficient.
 *               RV_ERROR_NOT_FOUND           - The transc-client does not contain a call-id
 *												yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient   - The Sip Stack handle to the transc-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientGetCallId (
                            IN  SipTranscClient    *pTranscClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus retStatus;

    *actualSize = 0;
    if(pTranscClient->strCallId == UNDEFINED)
    {
        pstrCallId[0] = '\0';
        return RV_ERROR_NOT_FOUND;
    }
    *actualSize = RPOOL_Strlen(pTranscClient->pMgr->hGeneralMemPool, pTranscClient->hPage,
                               pTranscClient->strCallId);
    if(bufSize < (*actualSize)+1)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    retStatus = RPOOL_CopyToExternal(pTranscClient->pMgr->hGeneralMemPool,
                                     pTranscClient->hPage,
                                     pTranscClient->strCallId,
                                     pstrCallId,
                                     *actualSize+1);
    return retStatus;
}

/***************************************************************************
 * TranscClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a TranscClient object, Free the resources taken by it and
 *          remove it from the manager transc-client list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transcclient.
 ***************************************************************************/
void RVCALLCONV TranscClientTerminate(IN SipTranscClient *pTranscClient,
									  IN SipTranscClientStateChangeReason eReason)
{
    RV_UNUSED_ARG(eReason)

	TranscClientAlertTimerRelease(pTranscClient);
    
	if(pTranscClient->hTcpConn != NULL)
    {
        RvSipTransportConnectionDetachOwner(pTranscClient->hTcpConn,
            (RvSipTransportConnectionOwnerHandle)pTranscClient);
        pTranscClient->hTcpConn = NULL;
    }

    if (NULL != pTranscClient->hActiveTransc)
    {
        /* Terminate the active transaction */
        RvSipTransactionDetachOwner(pTranscClient->hActiveTransc);
        RvSipTransactionTerminate(pTranscClient->hActiveTransc);
        pTranscClient->hActiveTransc = NULL;
    }

#ifdef RV_SIP_IMS_ON 
	/* notify the security-agreement on transc-client's termination */
    TranscClientSecAgreeDetachSecAgree(pTranscClient);
    /* Detach from Security Object */
    if (NULL != pTranscClient->hSecObj)
    {
        RvStatus rv;
        rv = TranscClientSetSecObj(pTranscClient, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
                "TranscClientTerminate - TranscClient 0x%p: Failed to unset Security Object %p",
                pTranscClient, pTranscClient->hSecObj));
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */
	
	/*change the state and insert into the event queue*/
    SipTransportSendTerminationObjectEvent(pTranscClient->pMgr->hTransportMgr,
								(void *)pTranscClient,
                                &(pTranscClient->terminationInfo),
								(RvInt32)eReason,
								TerminateTranscClientEventHandler,
								RV_TRUE,
								TRANSC_CLIENT_STATE_TERMINATED_STR,
								TRANSC_CLIENT_OBJECT_NAME_STR);
    return;
}

/***************************************************************************
 * TranscClientConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn   -   The connection handle
 *          hOwner  -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTranscClient,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{

    SipTranscClient*    pTranscClient    = (SipTranscClient*)hTranscClient;
	RvStatus	  rv = RV_OK;

    if(pTranscClient == NULL)
    {
        return RV_OK;
    }
    RV_UNUSED_ARG(eReason);
    if (SipTranscClientLockMsg(pTranscClient) != RV_OK)
    {
        RvLogWarning(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "TranscClientConnStateChangedEv - Transc-client 0x%p, conn 0x%p - Failed to lock Transc-client",
                   pTranscClient,hConn));
        return RV_ERROR_UNKNOWN;
    }

	switch(eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientConnStateChangedEv - TranscClient 0x%p received a connection closed event",pTranscClient));
        if (pTranscClient->hTcpConn == hConn)
        {
            pTranscClient->hTcpConn = NULL;
        }
        break;
    default:
        break;
    }

    SipTranscClientUnLockMsg(pTranscClient);
    return rv;
}


/***************************************************************************
 * TranscClientSendRequest
 * ------------------------------------------------------------------------
 * General: Request with REGISTER/PUBLISH method on the active transaction of the
 *          transaction-client object.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources request.
 *               RV_ERROR_UNKNOWN - failure sending the request.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The pointer to the transaction-client object.
 *          hRequestUri - The Request-Uri to send the request to.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSendRequest(
                                       IN  SipTranscClient				  *pTranscClient,
									   IN SipTransactionMethod			  eMethod,
									   IN RvChar*							  strMethod)
{
    RvStatus retStatus, cseqRetStatus;
	

    /* If the application created a message in advance use it*/
    if (NULL != pTranscClient->hOutboundMsg)
    {
        retStatus = SipTransactionSetOutboundMsg(pTranscClient->hActiveTransc, pTranscClient->hOutboundMsg);
        if (RV_OK != retStatus)
        {
            RvLogExcep(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                      "TranscClientSendRequest - Failed for transaction-client 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pTranscClient,pTranscClient->hActiveTransc,retStatus));
            return RV_ERROR_UNKNOWN;
        }
		pTranscClient->hOutboundMsg = NULL;        
    }
    
	if(pTranscClient->bUseFirstRouteRequest == RV_TRUE)
	{
		RvSipTransactionSendToFirstRoute(pTranscClient->hActiveTransc, 
			                             RV_TRUE);  
	}
	
	
	/* Request with REGISTER/PUBLISH method on the active transaction */
	switch (eMethod)
	{
	case SIP_TRANSACTION_METHOD_REGISTER:
		retStatus = SipTransactionRequest(pTranscClient->hActiveTransc,
											eMethod,
											pTranscClient->hRequestUri);
		break;
	case SIP_TRANSACTION_METHOD_OTHER:
		retStatus  = RvSipTransactionRequest(pTranscClient->hActiveTransc, strMethod, pTranscClient->hRequestUri);
		break;
	default:
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientSendRequest - Transc-client 0x%p: Unknown transaction method type (%d)",
			pTranscClient,eMethod));
		return RV_ERROR_UNKNOWN;
	}
    
    if (RV_OK != retStatus)
    {
        SipTransactionTerminate(pTranscClient->hActiveTransc);
        pTranscClient->hActiveTransc = NULL;
		cseqRetStatus = SipCommonCSeqReduceStep(&pTranscClient->cseq);
		if (RV_OK != cseqRetStatus) 
		{
			RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSendRequest - Transcister client 0x%p: failed in SipCommonCSeqReduceStep() (rv=%d)",
				pTranscClient,retStatus));		
		}

        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientSendRequest - Transaction client 0x%p: Error requesting on the active transaction",
                  pTranscClient));
        return retStatus;
    }

    return retStatus;
}


/***************************************************************************
 * TranscClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the transc-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the transc-client.
 *        eReason - The reason for the state change.
 *        pTranscClient - The transc-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TranscClientChangeState(
                            IN  SipTranscClientState             eNewState,
                            IN  SipTranscClientStateChangeReason eReason,
                            IN  SipTranscClient                  *pTranscClient,
							IN  RvInt16							 responseCode)
{
    if(eNewState == SIP_TRANSC_CLIENT_STATE_TERMINATED)
    {

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientChangeState - Transc-client 0x%p state changed to Terminated, (reason = %s)",
                   pTranscClient,
                   TranscClientReasonIntoString(eReason)));
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                 "TranscClientChangeState - Transc-client 0x%p state changed: %s->%s, (reason = %s)",
                   pTranscClient, TranscClientStateIntoString(pTranscClient->eState),
                   TranscClientStateIntoString(eNewState),
                  TranscClientReasonIntoString(eReason)));
    }
    /* Change the register-client's state */
    pTranscClient->eState = eNewState;

    if(pTranscClient->eState == SIP_TRANSC_CLIENT_STATE_ACTIVATED)
    {
		pTranscClient->bWasActivated = RV_TRUE;
    }

    /* Call the event state changed that was set to the register-clients
       manager */
    TranscClientCallbackStateChangeEv(eNewState, eReason, pTranscClient, responseCode);
}


/***************************************************************************
 * TranscClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the TranscClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 ***************************************************************************/
void TranscClientDetachFromConnection(IN SipTranscClient        *pTranscClient)
{
    RvStatus rv;
    if(pTranscClient->hTcpConn == NULL)
    {
        return;
    }
    rv = RvSipTransportConnectionDetachOwner(pTranscClient->hTcpConn,
                                            (RvSipTransportConnectionOwnerHandle)pTranscClient);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientDetachFromConnection - Failed to detach TranscClient 0x%p from prev connection 0x%p",pTranscClient,pTranscClient->hTcpConn));

    }
    pTranscClient->hTcpConn = NULL;
}

/***************************************************************************
 * TranscClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the TranscClient as the connection owner
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus TranscClientAttachOnConnection(IN SipTranscClient				 *pTranscClient,
                                 IN RvSipTransportConnectionHandle		  hConn)
{
	RvStatus	rv = RV_OK;

	rv = SipTransportConnectionAttachOwner(hConn,
		(RvSipTransportConnectionOwnerHandle)pTranscClient,
		&pTranscClient->transportEvHandler,sizeof(pTranscClient->transportEvHandler));
	pTranscClient->hTcpConn = hConn;
	
	return rv;

}

/***************************************************************************
 * TranscClientSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the transc-client's transactions timers.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTranscClient - Handle to the transc-client object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus TranscClientSetTimers(IN  SipTranscClient		*pTranscClient,
                            IN  RvSipTimers*			pNewTimers)
{
    RvStatus  rv = RV_OK;
    RvInt32   offset;

    if(NULL == pNewTimers)
    {
        /* reset option */
        pTranscClient->pTimers = NULL;
        return RV_OK;
    }

    /* 1. allocate timers struct on the transcClient page */
    if(pTranscClient->pTimers == NULL)
    {
        pTranscClient->pTimers = (RvSipTimers*)RPOOL_AlignAppend(
                                    pTranscClient->pMgr->hGeneralMemPool,
                                    pTranscClient->hPage,
                                    sizeof(RvSipTimers), &offset);
        if (NULL == pTranscClient->pTimers)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
                "TranscClientSetTimers - Transc-Client 0x%p: Failed to append timers struct",
                pTranscClient));
            return rv;
        }
    }

    /* calculation of default values is done in transaction layer */
    memcpy((void*)pTranscClient->pTimers, (void*)pNewTimers, sizeof(RvSipTimers));


    return rv;
}


/***************************************************************************
* TranscClientAlertTimerHandleTimeout
* ------------------------------------------------------------------------
* General: Called when ever the alert timer expires.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:	timerHandle - The timer that has expired.
*			pContext - The transcClient this timer was called for.
***************************************************************************/
RvBool TranscClientAlertTimerHandleTimeout(IN void  *pContext,
											IN RvTimer *timerInfo)
{
    SipTranscClient   *pTranscClient;

    pTranscClient = (SipTranscClient*)pContext;

	RvLogDebug(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc  ,
            "TranscClientAlertTimerHandleTimeout - transcClient 0x%p: alert timer expired.",
            pTranscClient));

    if (RV_OK != SipTranscClientLockMsg(pTranscClient))
    {
        RvLogError((pTranscClient->pMgr)->pLogSrc ,((pTranscClient->pMgr)->pLogSrc ,
            "TranscClientAlertTimerHandleTimeout - TranscClient 0x%p: failed to lock transcClient",
            pTranscClient));
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pTranscClient->alertTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc  ,
            "TranscClientAlertTimerHandleTimeout - transcClient 0x%p: alert timer expired but was already released. do nothing...",
            pTranscClient));
        
        SipTranscClientUnLockMsg(pTranscClient);
        return RV_FALSE;
    }

    SipCommonCoreRvTimerExpired(&pTranscClient->alertTimer);
	TranscClientAlertTimerRelease(pTranscClient);

    TranscClientCallbackExpirationAlertEv(pTranscClient);
    
    
    
    SipTranscClientUnLockMsg(pTranscClient);

    return RV_FALSE;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TranscClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transc-Client.
 *          As a result of this operation, all messages, sent by this
 *          Transc-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTranscClient - Pointer to the Transc-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TranscClientSetSecObj(	IN  SipTranscClient			*pTranscClient,
								IN  RvSipSecObjHandle		hSecObj)
{
    RvStatus      rv;
    TranscClientMgr* pTranscClientMgr = pTranscClient->pMgr;

    /* If Security Object was already set, detach from it */
    if (NULL != pTranscClient->hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTranscClientMgr->hSecMgr,
                pTranscClient->hSecObj, -1/*delta*/,
                pTranscClient->eOwnerType, (void*)pTranscClient->hOwner);
        if (RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
                "TranscClientSetSecObj - Transc-Client 0x%p: Failed to unset existing Security Object %p",
                pTranscClient, pTranscClient->hSecObj));
            return rv;
        }
        pTranscClient->hSecObj = NULL;
    }

    /* Register the Transc-client to the new Security Object*/
    if (NULL != hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTranscClientMgr->hSecMgr,
                hSecObj, 1/*delta*/, pTranscClient->eOwnerType,
                (void*)pTranscClient->hOwner);
        if (RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
                "TranscClientSetSecObj - Transc-Client 0x%p: Failed to set new Security Object %p",
                pTranscClient, hSecObj));
            return rv;
        }
    }

    pTranscClient->hSecObj = hSecObj;

    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * TranscClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV TranscClientStateIntoString(
                                            IN SipTranscClientState eNewState)
{
    switch (eNewState)
    {
    case SIP_TRANSC_CLIENT_STATE_IDLE:
        return "Idle";
    case SIP_TRANSC_CLIENT_STATE_TERMINATED:
        return TRANSC_CLIENT_STATE_TERMINATED_STR;
    case SIP_TRANSC_CLIENT_STATE_ACTIVATING:
        return "Activating";
    case SIP_TRANSC_CLIENT_STATE_REDIRECTED:
        return "Redirected";
    case SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED:
        return "Unauthenticated";
    case SIP_TRANSC_CLIENT_STATE_ACTIVATED:
        return "Activated";
    case SIP_TRANSC_CLIENT_STATE_FAILED:
        return "Failed";
    case SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE:
        return "Msg Send Failure";
    default:
        return "Undefined";
    }
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

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
static RvChar *RVCALLCONV TranscClientReasonIntoString(
                                   IN SipTranscClientStateChangeReason eReason)
{
    switch (eReason)
    {
    case SIP_TRANSC_CLIENT_REASON_UNDEFINED:
        return "Undefined";
    case SIP_TRANSC_CLIENT_REASON_USER_REQUEST:
        return "User request";
    case SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD:
        return "Response successful received";
    case SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
        return "Response request failure received";
    case SIP_TRANSC_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD:
        return "Response server failure received";
    case SIP_TRANSC_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
        return "Response global failure received";
    case SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE:
        return "Local failure";
    case SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT:
        return "Transaction timeout";
    case SIP_TRANSC_CLIENT_REASON_GIVE_UP_DNS:
        return "Give Up DNS";
	case SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE:
		return "Response Unauthentication received";
	case SIP_TRANSC_CLIENT_REASON_REDIRECT_RESPONSE_RECVD:
		return "Response Redirect received";
    case SIP_TRANSC_CLIENT_REASON_NETWORK_ERROR:
        return "Network Error";
    case SIP_TRANSC_CLIENT_REASON_503_RECEIVED:
        return "503 Received";
    case SIP_TRANSC_CLIENT_REASON_CONTINUE_DNS:
        return "Continue DNS";
    default:
        return "";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


/***************************************************************************
 * TerminateTranscClientEventHandler
 * ------------------------------------------------------------------------
 * General: Terminates a TranscClient object, Free the resources taken by it and
 *          remove it from the manager list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     obj        - pointer to TranscClient to be deleted.
 *            reason    - reason of the termination
 ***************************************************************************/
static RvStatus RVCALLCONV TerminateTranscClientEventHandler(IN void      *obj,
                                                             IN RvInt32  reason)
{
    SipTranscClient            *pTranscClient;
    RvStatus					retCode;

    pTranscClient = (SipTranscClient *)obj;
    if (pTranscClient == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    retCode = SipTranscClientLockMsg(pTranscClient);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TerminateTranscClientEventHandler - TranscClient 0x%p - event is out of the termination queue", pTranscClient));
    
    TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_TERMINATED,
                                 (SipTranscClientStateChangeReason)reason,
                                 pTranscClient, -1);
    TranscClientCallbackTerminatedEv(pTranscClient);
	SipTranscClientUnLockMsg(pTranscClient);
    return retCode;
}

/***************************************************************************
 * TranscClientAlertTimerRelease
 * ------------------------------------------------------------------------
 * General: If the alert timer was started release it.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTranscClient - The pointer to the transcClient object.
 ***************************************************************************/
static void TranscClientAlertTimerRelease(SipTranscClient *pTranscClient)
{
	if (NULL != pTranscClient)
    {
        RvLogDebug((pTranscClient->pMgr)->pLogSrc ,((pTranscClient->pMgr)->pLogSrc ,
            "TranscClientAlertTimerRelease - TranscClient 0x%p: Timer 0x%p was released",
            pTranscClient, &pTranscClient->alertTimer));
		
        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pTranscClient->alertTimer);
    }
}
#endif /* RV_SIP_PRIMITIVES */






















