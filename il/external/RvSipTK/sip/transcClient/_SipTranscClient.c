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
 *                              <SipTranscClient.c>
 *
 * This file defines the TranscClient API functions, which is the platform of all
 * known client transaction such as Register client and publisher clients and future
 * clients.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                  Aug 2006
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "TranscClientMgrObject.h"
#include "TranscClientObject.h"
#include "_SipTranscClient.h"
#include "AdsRlist.h"
#include "_SipPartyHeader.h"
#include "_SipExpiresHeader.h"
#include "_SipContactHeader.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include "_SipMsg.h"
#include "_SipCommonCSeq.h"
#include "_SipCommonCore.h"
#include "RvSipContactHeader.h"
#include "_SipTransport.h"
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
#include "TranscClientDNS.h"
#endif

#include "_SipCommonConversions.h"
#include "_SipHeader.h"

#ifdef RV_SIP_IMS_ON
#include "_SipAuthorizationHeader.h"
#include "TranscClientSecAgree.h"
#include "RvSipTransaction.h"
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus RVCALLCONV TranscClientCopyToHeader (
                                      IN  SipTranscClient             *pTranscClient,
                                      IN  RvSipPartyHeaderHandle hTo);

static RvStatus RVCALLCONV TranscClientCopyFromHeader (
                                      IN  SipTranscClient             *pTranscClient,
                                      IN  RvSipPartyHeaderHandle hFrom);

static RvStatus RVCALLCONV TranscClientCopyRequestUri (
                                      IN  SipTranscClient               *pTranscClient,
                                      IN  RvSipAddressHandle       hRequestUri);

static RvStatus RVCALLCONV TranscClientCopyExpiresHeader (
                                      IN  SipTranscClient               *pTranscClient,
                                      IN  RvSipExpiresHeaderHandle hExpires);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus TranscClientLock(IN SipTranscClient *pTranscClient,
							  IN TranscClientMgr    *pMgr,
							  IN SipTripleLock		*tripleLock,
							  IN RvInt32			identifier );

static void RVCALLCONV TranscClientUnLock(	IN RvLogMgr *pLogMgr,
											IN RvMutex  *hLock);
#else
#define TranscClientLock(a,b,c,d) (RV_OK)
#define TranscClientUnLock(a,b)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipTranscClientSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the event handlers defined by the client module to
 *          the transcClient.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
void RVCALLCONV SipTranscClientSetEvHandlers( IN SipTranscClient *pTranscClient,
                                         IN SipTranscClientEvHandlers  *pTranscClientEvHandler,
										 IN RvInt32						size)
{
	RV_UNUSED_ARG(pTranscClient|| pTranscClientEvHandler|| size)
	memset(&(pTranscClient->transcClientEvHandlers), 0, size);
	memcpy(&(pTranscClient->transcClientEvHandlers), pTranscClientEvHandler,size);
}

/***************************************************************************
 * SipTranscClientActivate
 * ------------------------------------------------------------------------
 * General: Sends a client request to the Destination. The Request-Url,To,
 *          From , Expires and Contact headers that were set to the transaction-
 *          client are inserted to the outgoing REGISTER/PUBLISH request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid transc-client state for this
 *                                  action, or the To and From header have
 *                                  yet been set to the transc-client object,
 *                                  or the destination Request-Url has not
 *                                  yet been set to the transc-client object.
 *               RV_ERROR_OUTOFRESOURCES - transc-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The transaction-client pointer.
 *			  eTranscMethod - The method to be used by the transaction.
 *			  strMethod		- In case the transcMethod is OTHER method the caller should send the string of the method. 
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientActivate (
                                         IN SipTranscClient			  *pTranscClient,
										 IN SipTransactionMethod	  eTranscMethod,
										 IN RvChar*					  strMethod)
{
    RvStatus				retStatus;

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientActivate - Activating the client 0x%p ",pTranscClient));

    
	retStatus = TranscClientCreateNewTransaction(pTranscClient);
	if (RV_OK != retStatus)
    {
		return retStatus;
    }
    /* Request with REGISTER/PUBLISH method on the active transaction */
	retStatus = TranscClientSendRequest(pTranscClient, eTranscMethod, strMethod);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientActivate - Activating client 0x%p: request was successfully sent",
               pTranscClient));

	
	/* The transaction-client object changes state to Activating */
    TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_ACTIVATING,
							SIP_TRANSC_CLIENT_REASON_USER_REQUEST,
							pTranscClient, 0);
    

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transc-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTarnscClientMgr   - Handle to the transc-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrGetStackInstance(
                                   IN   SipTranscClientMgrHandle   hTranscClientMgr,
                                   OUT  void*         *phStackInstance)
{
    TranscClientMgr *pTranscClientMgr;

    *phStackInstance = NULL;

    if (NULL == hTranscClientMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTranscClientMgr = (TranscClientMgr *)hTranscClientMgr;
    TranscClientMgrLock(pTranscClientMgr);
    *phStackInstance = pTranscClientMgr->hStack;
    TranscClientMgrUnLock(pTranscClientMgr);
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetAlertTimer
 * ------------------------------------------------------------------------
 * General: Setting an alert timer for timeToSet.
 *			calling this function with timeToSet equals to 0 will cancel the previous
 *			timer without starting new interval
 *			
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient		- The pointer to the transcClient.
 * Output:    TimeToSet			- The interval to set the alert timer.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetAlertTimer(SipTranscClient *pTranscClient, RvUint32 timeToSet)
{

	RvStatus rv;
	
	SipCommonCoreRvTimerCancel(&pTranscClient->alertTimer);

	if (timeToSet == 0)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
            "SipTranscClientSetAlertTimer - transcClient 0x%p - timer 0x%p was canceled",
            pTranscClient, &pTranscClient->alertTimer));
		return RV_OK;
	}

	rv = SipCommonCoreRvTimerStartEx(&pTranscClient->alertTimer,
		pTranscClient->pMgr->pSelect,
		timeToSet,
		TranscClientAlertTimerHandleTimeout,
		pTranscClient);
	
    if (rv != RV_OK)
    {
        RvLogExcep(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
            "SipTranscClientSetAlertTimer - transcClient 0x%p - No timer is available",
            pTranscClient));
        return RV_ERROR_OUTOFRESOURCES;
    }
    else
    {
        RvLogInfo(pTranscClient->pMgr->pLogSrc ,(pTranscClient->pMgr->pLogSrc ,
            "SipTranscClientSetAlertTimer - transcClient 0x%p - timer 0x%p was set to %u milliseconds",
            pTranscClient, &pTranscClient->alertTimer, timeToSet));
    }
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetActivateDetails
 * ------------------------------------------------------------------------
 * General: Set the transaction client from,to,destination and contact parameters that
 *          are given as strings to the transcClient. If a null value is supplied as one of
 *          the arguments this parameter will not be set.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transaction-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid transaction-client state for this
 *                                  action or there is a missing parameter that
 *                                  is needed for the process such as the request URI.
 *               RV_ERROR_OUTOFRESOURCES - transaction-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 *          strFrom    - the initiator of the request.
 *                       for example: "From: sip:user@home.com"
 *          strTo      - The processing user.
 *          strRegistrar - The request URI of the transc-client request.This
 *                         is the proxy address.
 *			bValues		 - Boolean indicates whether the strings are the values of the header
 *						   sip:user@home.com or the header including its prefix "From: sip:user@home.com"
 *          
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetActivateDetails (
                                         IN SipTranscClient			*pTranscClient,
                                         IN RvChar*					strFrom,
                                         IN RvChar*					strTo,
                                         IN RvChar*					strDestination,
										 IN RvBool					bValues)
{
    RvStatus rv = RV_OK;
    RvSipPartyHeaderHandle   hFrom;			/*handle to the From header*/
    RvSipPartyHeaderHandle   hTo;			/*handle to the To header*/
    RvSipAddressHandle       hDestination;	/*handle to the registrar address*/

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientSetActivateDetails - Transaction client 0x%p: starting the process", pTranscClient));

	if (strFrom != NULL)
	{
		rv = SipTranscClientGetNewMsgElementHandle(pTranscClient, RVSIP_HEADERTYPE_FROM,
											RVSIP_ADDRTYPE_UNDEFINED, (void**)&hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transcaction-client 0x%p: Failed to allocate from party header",
                pTranscClient));
            return rv;

        }
		if (bValues == RV_TRUE)
		{
			rv = RvSipPartyHeaderParseValue(hFrom, RV_FALSE, strFrom);
		}
		else
		{
			rv = RvSipPartyHeaderParse(hFrom,RV_FALSE,strFrom);
		}
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transaction-client 0x%p: Failed to parse from header - bad syntax",
                pTranscClient));
            return rv;
        }
        rv = SipTranscClientSetFromHeader(pTranscClient,hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transaction-client 0x%p: Failed to set from header",
                pTranscClient));
            return rv;
        }
    }

    if(strTo != NULL)
    {
		rv = SipTranscClientGetNewMsgElementHandle(pTranscClient, RVSIP_HEADERTYPE_TO,
												RVSIP_ADDRTYPE_UNDEFINED, (void**)&hTo);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transc-client 0x%p: Failed to allocate to party header",
                pTranscClient));
            return rv;

        }
		if (bValues == RV_TRUE)
		{
			rv = RvSipPartyHeaderParseValue(hTo, RV_TRUE, strTo);
		}
		else
		{
			rv = RvSipPartyHeaderParse(hTo,RV_TRUE,strTo);
		}
        
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transc-client 0x%p: Failed to parse to header - bad syntax",
                pTranscClient));
            return rv;
        }
        rv = SipTranscClientSetToHeader(pTranscClient,hTo);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transaction-client 0x%p: Failed to set To header",
                pTranscClient));
            return rv;
        }
    }

    if(strDestination != NULL)
    {
        RvSipAddressType eStrAddr = RVSIP_ADDRTYPE_UNDEFINED;

        /* get the address type from the registrar string */
		eStrAddr = SipAddrGetAddressTypeFromString(pTranscClient->pMgr->hMsgMgr, strDestination);
		rv = SipTranscClientGetNewMsgElementHandle(pTranscClient, RVSIP_HEADERTYPE_UNDEFINED,
												eStrAddr, (void**)&hDestination);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transactionclient 0x%p: Failed to allocate registrar address",
                pTranscClient));
            return rv;

        }
        rv = RvSipAddrParse(hDestination,strDestination);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transaction-client 0x%p: Failed to parse to destination - bad syntax",
                pTranscClient));
            return rv;
        }
        rv = SipTranscClientSetDestination(pTranscClient,hDestination);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetActivateDetails - Transc-client 0x%p: Failed to set Destination",
                pTranscClient));
            return rv;
        }
    }

	return RV_OK;

}

/***************************************************************************
 * SipTranscClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a transc-client object. This function destructs the
 *          transc-client object. You cannot reference the object after
 *          calling this function.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transaction-client is
 *                                   invalid.
 *                 RV_OK - The transaction-client was successfully terminated
 *                            and distructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientTerminate (
                                         IN SipTranscClient	 *pTranscClient)
{
	if (pTranscClient->eState == SIP_TRANSC_CLIENT_STATE_TERMINATED)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientTerminate - Transaction-client 0x%p, already terminated!",
			pTranscClient));
		return RV_OK;
	}
    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientTerminate - About to  terminate Transaction-client 0x%p",
               pTranscClient));
    TranscClientTerminate(pTranscClient, SIP_TRANSC_CLIENT_REASON_USER_REQUEST);
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientUseFirstRouteForRequest
 * ------------------------------------------------------------------------
 * General: The owner may want to use a preloaded route header when
 *          sending the register/publish message.For this purpose, it should add the
 *          Route headers to the outbound message, and call this function
 *          to indicate the stack to send the request to the address of the
 *          first Route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient    - Handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientUseFirstRouteForRequest (
                                      IN  SipTranscClient	*pTranscClient)
{
   	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientUseFirstRouteForRequest - transc-client 0x%p will use preloaded Route header",
              pTranscClient));

    pTranscClient->bUseFirstRouteRequest = RV_TRUE;

    return RV_OK;

}

/*-----------------------------------------------------------------------
       TRANSC  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * SipTranscClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: Stops retries to send a Register/publish request after send failure.
 *          When a Register/publish request fails due to timeout, network error or
 *          503 response the Transc-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the owner can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          Calling SipTranscClientDNSGiveUp() indicates that the application
 *          wishes to give up on this request. Retries to send the request
 *          will stop and the Transc-client will change its state back to
 *          the previous state.
 *          If this is the initial request of the Transc-Client, calling
 *          DNSGiveUp() will terminate the transc-Client object. If this is a
 *          refresh, the Transc-Client will move back to the ACCEPTED state.
 * Return Value: RvStatus;
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSGiveUp (
                                   IN  SipTranscClient   *pTranscClient)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientDNSGiveUp - Transc-client 0x%p: giveup DNS" ,
              pTranscClient));
    
	rv = TranscClientDNSGiveUp(pTranscClient);

    return rv;
#else
    RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientDNSGiveUp - transc-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pTranscClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * SipTranscClientDNSContinue
 * ------------------------------------------------------------------------
 * General: Prepares the Transc-Client object to a retry to send a request after
 *          the previous attempt failed.
 *          When a request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSContinue (
                                      IN  SipTranscClient	   *pTranscClient)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientDNSContinue - transc-client 0x%p: continue DNS" ,
            pTranscClient));

    rv = TranscClientDNSContinue(pTranscClient);

    return rv;
#else
    RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientDNSContinue - transc-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pTranscClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * SipTranscClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: Re-sends a request after the previous attempt failed.
 *          When a Register/publish request fails due to timeout, network error or
 *          503 response the Transc-Client Object moves to the MSG_SEND_FAILURE state.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSReSendRequest (
                                            IN    SipTranscClient		 *pTranscClient,
											IN    SipTransactionMethod	  eMethod,
											IN	  RvChar*				  strMethod)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientDNSReSendRequest - transc-client 0x%p: re-send DNS" ,
            pTranscClient));

    rv = TranscClientDNSReSendRequest(pTranscClient, eMethod, strMethod);
    return rv;
#else
	RV_UNUSED_ARG(eMethod)
	RV_UNUSED_ARG(strMethod)
		
    RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientDNSReSendRequest - transc-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pTranscClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * SipTranscClientDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves DNS list object from the Transc-Client current active
 *          transaction.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state you can use SipTranscClientDNSGetList() to get the
 *          DNS list if you wish to view or change it.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client that sent the request.
 * Output     phDnsList     - The DNS list handle
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSGetList(
                              IN  SipTranscClient				*pTranscClient,
                              OUT RvSipTransportDNSListHandle	*phDnsList)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientDNSGetList - transc-client 0x%p: getting DNS list" ,
              pTranscClient));

    rv = TranscClientDNSGetList(pTranscClient, phDnsList);

    return rv;
#else
    RV_UNUSED_ARG(phDnsList);
    RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientDNSGetList - transc-client 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pTranscClient));
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * SipTranscClientGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the transc-client page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information
 *         and set it back to the transc-client using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         Party   - you should set these headers back with the SipTranscClientSetToHeader()
 *                   or SipTranscClientSetFromHeader() API functions.
 *         Contact - you should set this header back with the SipTranscClientSetContactHeader()
 *                   API function.
 *         Expires - you should set this header back with the SipTranscClientSetExpiresHeader()
 *                   API function.
 *         Authorization - you should set this header back with the header to the SipTranscClientSetInitialAuthorization() 
 *                   API function that is available in the IMS add-on only.
 *         address - you should supply the address to the SipTranscClientSetDestination() API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetNewMsgElementHandle (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader)
{
    RvStatus  rv;
    if(pTranscClient == NULL || phHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
	*phHeader = NULL;
	if(eHeaderType != RVSIP_HEADERTYPE_TO &&
        eHeaderType != RVSIP_HEADERTYPE_FROM &&
#ifdef RV_SIP_IMS_ON
        eHeaderType != RVSIP_HEADERTYPE_AUTHORIZATION &&
#endif
        eHeaderType != RVSIP_HEADERTYPE_CONTACT &&
        eHeaderType != RVSIP_HEADERTYPE_EXPIRES &&
        (eHeaderType == RVSIP_HEADERTYPE_UNDEFINED &&
         eAddrType == RVSIP_ADDRTYPE_UNDEFINED))
    {
        /* None of the supported headers, and not an address */
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientGetNewMsgElementHandle - transc-client 0x%p - unsupported action for header %s address %s",
            pTranscClient, SipHeaderName2Str(eHeaderType), SipAddressType2Str(eAddrType)));
        return RV_ERROR_ILLEGAL_ACTION;

    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientGetNewMsgElementHandle - getting new header-%s || addr-%s handle from transc-client 0x%p",
              SipHeaderName2Str(eHeaderType), SipAddressType2Str(eAddrType) , pTranscClient));

    if(eHeaderType != RVSIP_HEADERTYPE_UNDEFINED)
    {
        rv = SipHeaderConstruct(pTranscClient->pMgr->hMsgMgr, eHeaderType,pTranscClient->pMgr->hGeneralMemPool,
                                pTranscClient->hPage, phHeader);
    }
    else /*address*/
    {
        rv = RvSipAddrConstruct(pTranscClient->pMgr->hMsgMgr, pTranscClient->pMgr->hGeneralMemPool,
                                pTranscClient->hPage, eAddrType, (RvSipAddressHandle*)phHeader);
    }

    if (rv != RV_OK || *phHeader == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientGetNewMsgElementHandle - transc-client 0x%p - Failed to create new header (rv=%d)",
              pTranscClient, rv));
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}


/***************************************************************************
 * SipTranscClientSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the Transc-client call-id. If the call-id is not set the Transc-client
 *          will use the TranscClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The Transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The Sip Stack handle to the Transc-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCallId (
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RvChar				*strCallId)
{
    RvStatus               rv;
    RvInt32                offset;
    RPOOL_ITEM_HANDLE       ptr;

    if(strcmp(strCallId,"") == 0)
    {
        return RV_ERROR_BADPARAM;
    }
    
    rv = RPOOL_AppendFromExternal(pTranscClient->pMgr->hGeneralMemPool, pTranscClient->hPage,
                                  strCallId, (RvInt)strlen(strCallId)+1, RV_FALSE,
                                  &offset, &ptr);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetCallId - transc-client 0x%p Failed (rv=%d)", pTranscClient,rv));
        return rv;
    }

    pTranscClient->strCallId = offset;
#ifdef SIP_DEBUG
    pTranscClient->pCallId = RPOOL_GetPtr(pTranscClient->pMgr->hGeneralMemPool, pTranscClient->hPage,
                                       pTranscClient->strCallId);
#endif

#if defined(UPDATED_BY_SPIRENT)
    if(pTranscClient->hActiveTransc) {
      RvSipTransactionSetCallId(pTranscClient->hActiveTransc,strCallId);
    }
#endif

    return rv;
}


/***************************************************************************
 * SipTranscClientSetCallIdFromPage
 * ------------------------------------------------------------------------
 * General: Sets the Transc-client call-id. If the call-id is not set the Transc-client
 *          will use the TranscClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The Transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The Sip Stack handle to the Transc-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCallIdFromPage(
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RPOOL_Ptr				*srcCallIdPage)
{
    RvStatus               rv;
    RPOOL_Ptr			   destCallId;

	destCallId.hPage = pTranscClient->hPage;
	destCallId.hPool = pTranscClient->pMgr->hGeneralMemPool;
	destCallId.offset= UNDEFINED;

	/*First we are checking whether the last call id that was setted is the same as the new one.
	  If so we don't need to copy it again.*/
	if (RPOOL_Strcmp(srcCallIdPage->hPool, srcCallIdPage->hPage, srcCallIdPage->offset,
				 destCallId.hPool, destCallId.hPage, pTranscClient->strCallId))
	{
		/*Both call-ids are the same we don't need to copy it again.*/
		return RV_OK;
	}
	
    rv = SipCommonCopyStrFromPageToPage(srcCallIdPage, &destCallId);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetCallIdFromPage - transc-client 0x%p Failed to set call-id from page (rv=%d)", pTranscClient,rv));
        return rv;
    }

    pTranscClient->strCallId = destCallId.offset;
#ifdef SIP_DEBUG
    pTranscClient->pCallId = RPOOL_GetPtr(pTranscClient->pMgr->hGeneralMemPool, pTranscClient->hPage,
                                       pTranscClient->strCallId);
#endif
    return rv;
}



/***************************************************************************
 * SipTranscClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Transc-client Call-Id.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the Transc-client.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The Transc-client does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient   - The Sip Stack handle to the Transc-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCallId (
                            IN  SipTranscClient		*pTranscClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus               rv;
    if(pTranscClient == NULL || pstrCallId == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TranscClientGetCallId(pTranscClient,bufSize,pstrCallId,actualSize);

    return rv;


}


/***************************************************************************
 * SipTranscClientSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the from header associated with the transaction-client. If the
 *          From header was constructed by the
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid
 *                                   or the From header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hFrom      - Handle to an application constructed an initialized
 *                       from header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetFromHeader (
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RvSipPartyHeaderHandle hFrom)
{
    RvStatus           retStatus;


    if ((NULL == pTranscClient) ||
        (NULL == hFrom))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetFromHeader - Setting Transaction-Client 0x%p From header",
              pTranscClient));

    /* Check if the given header was constructed on the transaction-client
       object's page and if so attach it */
    if((SipPartyHeaderGetPool(hFrom) == pTranscClient->pMgr->hGeneralMemPool) &&
       (SipPartyHeaderGetPage(hFrom) == pTranscClient->hPage))
    {
        pTranscClient->hFromHeader = hFrom;
        return RV_OK;
    }
    retStatus = TranscClientCopyFromHeader(pTranscClient, hFrom);
    return retStatus;
}


/***************************************************************************
 * SipTranscClientGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the transaction-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 * Output:     phFrom     - Pointer to the transaction-client From header handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetFromHeader (
                                      IN  SipTranscClient			*pTranscClient,
                                      OUT RvSipPartyHeaderHandle	*phFrom)
{
    if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == phFrom)
    {
        return RV_ERROR_NULLPTR;
    }
    *phFrom = pTranscClient->hFromHeader;
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the transaction-client. If the To
 *          header was constructed by the SipTranscClientGetNewPartyHeaderHandle
 *          function, the To header will be attached to the transaction-client
 *          object.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle or the To
 *                                   header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hTo        - Handle to an application constructed and initialized
 *                       To header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetToHeader (
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RvSipPartyHeaderHandle hTo)
{
    RvStatus           retStatus;


    if ((NULL == pTranscClient) ||
        (NULL == hTo))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetToHeader - Setting transc-Client 0x%p To header",
              pTranscClient));

    /* Check if the given header was constructed on the transaction-client
       object's page and if so attach it */
    if((SipPartyHeaderGetPool(hTo) == pTranscClient->pMgr->hGeneralMemPool) &&
       (SipPartyHeaderGetPage(hTo) == pTranscClient->hPage))
    {
        pTranscClient->hToHeader = hTo;
        return RV_OK;
    }
    /* Copy the given To header to the transaction-client object */
    retStatus = TranscClientCopyToHeader(pTranscClient, hTo);
    return retStatus;
}

/***************************************************************************
 * SipTranscClientGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To header associated with the transc-client.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 * Output:     phTo       - Pointer to the transaction-client To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV SipTranscClientGetToHeader (
                                      IN    SipTranscClient			*pTranscClient,
                                      OUT   RvSipPartyHeaderHandle  *phTo)
{
    if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == phTo)
    {
        return RV_ERROR_NULLPTR;
    }
    *phTo = pTranscClient->hToHeader;

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetDestination
 * ------------------------------------------------------------------------
 * General: Set the SIP-Url of the destination in the transc-client object.
 *          The transaction request will be sent to this SIP-Url.
 *          This ability can be used for updating the destination's SIP-Url
 *          in case of redirections and refreshes.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_BADPARAM - The supplied address handle is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hRequestUri - Handle to the destination SIP-Url to be set to the
 *                        transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetDestination (
                                 IN  SipTranscClient		*pTranscClient,
                                 IN  RvSipAddressHandle		hRequestUri)
{
    RvStatus           retStatus;

    if ((NULL == pTranscClient) ||
        (NULL == hRequestUri))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetDestination - Setting Transaction-Client 0x%p Destination Request-Url",
              pTranscClient));

    /* Check if the given header was constructed on the transaction-client
       object's page and if so attach it */
    if((SipAddrGetPool(hRequestUri) == pTranscClient->pMgr->hGeneralMemPool) &&
       (SipAddrGetPage(hRequestUri) == pTranscClient->hPage))
    {
        pTranscClient->hRequestUri = hRequestUri;
        return RV_OK;
    }
    retStatus = TranscClientCopyRequestUri(pTranscClient, hRequestUri);
    return retStatus;
}


/***************************************************************************
 * SipTranscClientGetDestination
 * ------------------------------------------------------------------------
 * General: Gets the SIP-Url of the destination associated with the
 *          transc-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the owner.
 *               RV_OK        - The address object was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phRequestUri    - Handle to the destination SIP-Url.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetDestination (
                               IN  SipTranscClient			*pTranscClient,
                               OUT RvSipAddressHandle       *phRequestUri)
{
    if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == phRequestUri)
    {
        return RV_ERROR_NULLPTR;
    }

    *phRequestUri = pTranscClient->hRequestUri;
    return RV_OK;
}


/***************************************************************************
 * SipTranscClientGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the transc client. You can
 *          call this function from the SipTranscClientStateChangedEv call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phMsg           - pointer to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetReceivedMsg(
                                            IN  SipTranscClient			*pTranscClient,
                                             OUT RvSipMsgHandle         *phMsg)
{
    if(pTranscClient == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phMsg = NULL;

    *phMsg = pTranscClient->hReceivedMsg;

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the transc-client.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transc-client.
 * Output:     phMsg           -  pointer to the message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundMsg(
                                     IN  SipTranscClient			*pTranscClient,
                                     OUT RvSipMsgHandle				*phMsg)
{
    RvSipMsgHandle         hOutboundMsg = NULL;
    RvStatus              rv;

    if(pTranscClient == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phMsg = NULL;

    if (NULL != pTranscClient->hOutboundMsg)
    {
        hOutboundMsg = pTranscClient->hOutboundMsg;
    }
    else
    {
        rv = RvSipMsgConstruct(pTranscClient->pMgr->hMsgMgr,
                               pTranscClient->pMgr->hMsgMemPool,
                               &hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                       "SipTranscClientGetOutboundMsg - transaction-client= 0x%p error in constructing message",
                      pTranscClient));
            return rv;
        }
        pTranscClient->hOutboundMsg = hOutboundMsg;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientGetOutboundMsg - transaction-client= 0x%p returned handle= 0x%p",
              pTranscClient,hOutboundMsg));

    *phMsg = hOutboundMsg;

    return RV_OK;
}


/***************************************************************************
 * SipTranscClientResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the transaction-client to NULL. If the
 *          transaction-client is about to send a message it will create its own
 *          message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientResetOutboundMsg(
                                     IN  SipTranscClient		*pTranscClient)
{
    if(pTranscClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (NULL != pTranscClient->hOutboundMsg)
    {
        RvSipMsgDestruct(pTranscClient->hOutboundMsg);
    }
    pTranscClient->hOutboundMsg = NULL;

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientResetOutboundMsg - transaction-client= 0x%p resetting outbound message",
              pTranscClient));

    return RV_OK;
}


/***************************************************************************
 * SipTranscClientSetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Set an Expires header in the transaction-client object. 
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Expires header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Expires header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hExpiresHeader - Handle to an Expires header to be set to the
 *                           transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetExpiresHeader (
                                 IN  SipTranscClient			*pTranscClient,
                                 IN  RvSipExpiresHeaderHandle	hExpiresHeader)
{
    RvStatus           retStatus;


    if (NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/*If this fuction was triggered with NULL it means that we need to reset the previous expires header.*/
	if (hExpiresHeader == NULL)
	{
		RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientSetExpiresHeader - Resetting Transaction-Client 0x%p Expires header",
			pTranscClient));
		pTranscClient->hExpireHeader = NULL;
		return RV_OK;
	}
    
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetExpiresHeader - Setting Transaction-Client 0x%p Expires header",
              pTranscClient));

    /* Check if the given header was constructed on the Transaction-client
       object's page and if so attach it */
    if((SipExpiresHeaderGetPool(hExpiresHeader) ==
                                        pTranscClient->pMgr->hGeneralMemPool) &&
       (SipExpiresHeaderGetPage(hExpiresHeader) == pTranscClient->hPage))
    {
        pTranscClient->hExpireHeader = hExpiresHeader;
        return RV_OK;
    }
    retStatus = TranscClientCopyExpiresHeader(pTranscClient, hExpiresHeader);
    return retStatus;
}


/***************************************************************************
 * SipTranscClientGetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Gets the Expires header associated with the transaction-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The Expires header was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phExpiresHeader - Handle to the transaction-client's Expires header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetExpiresHeader (
                               IN  SipTranscClient			*pTranscClient,
                               OUT RvSipExpiresHeaderHandle *phExpiresHeader)
{


    if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == phExpiresHeader)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *phExpiresHeader = pTranscClient->hExpireHeader;
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientGetExpiresVal
 * ------------------------------------------------------------------------
 * General: Gets the Expires value associated with the transaction-client expires header.
 * Return Value: RV_ERROR_NOT_FOUND   - The transc-client expires header was set yet.
 *               RV_OK				  - The expires value was found.
 *				 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     pExpVal			 - The value of the transc-client expires header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetExpiresValue (
                               IN  SipTranscClient			*pTranscClient,
                               OUT RvUint32					*pExpVal)
{
	RvStatus	rv;
	if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pTranscClient->hExpireHeader)
    {
        return RV_ERROR_NOT_FOUND;
    }

    rv = RvSipExpiresHeaderGetDeltaSeconds(pTranscClient->hExpireHeader, pExpVal);
	if(RV_OK != rv)
    {
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientGetExpiresValue - TranscClient 0x%p - Unable to get expires value from expHeader 0x%p", pTranscClient,
			pTranscClient->hExpireHeader));
		return rv;
    }
    return RV_OK;
}
							   


/***************************************************************************
 * SipTranscClientGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transc-client
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transc-client object.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetStackInstance(
                                   IN   SipTranscClient	*pTranscClient,
                                   OUT  void*			*phStackInstance)
{
    TranscClientMgr *pTranscClientMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    pTranscClientMgr = pTranscClient->pMgr;
    TranscClientMgrLock(pTranscClientMgr);
    *phStackInstance = pTranscClientMgr->hStack;
    TranscClientMgrUnLock(pTranscClientMgr);
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the Transc-Client will send outgoing
 *          requests.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The transc-client handle.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType   - The address type(IPv4 or IPv6).
 *            strLocalIPAddress   - A string with the local address to be set to this Transc-client.
 *          localPort      - The local port to be set to this Transc-client. If you set
 *                           the local port to 0, you will get a default value of 5060.
 *          if_name - the local interface name
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetLocalAddress(
                            IN  SipTranscClient			 *pTranscClient,
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
    RvSipTransportLocalAddrHandle hLocalAddr = NULL;
    if (pTranscClient == NULL || strLocalIPAddress==NULL || localPort==(RvUint16)UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    if((eTransportType != RVSIP_TRANSPORT_TCP &&
#if (RV_TLS_TYPE != RV_TLS_NONE)
        eTransportType != RVSIP_TRANSPORT_TLS &&
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        eTransportType != RVSIP_TRANSPORT_SCTP &&
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        eTransportType != RVSIP_TRANSPORT_UDP) ||
       (eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP &&
        eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP6))
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetLocalAddress - TranscClient 0x%p - Setting %s %s local address %s:%d",
               pTranscClient,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType),
               (strLocalIPAddress==NULL)?"0":strLocalIPAddress,localPort));

    rv = SipTransportLocalAddressGetHandleByAddress(pTranscClient->pMgr->hTransportMgr,
                                                    eTransportType,
                                                    eAddressType,
                                                    strLocalIPAddress,
                                                    localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name, 
						    vdevblock,
#endif
                                                    &hLocalAddr);

    if(hLocalAddr == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }
    if(rv != RV_OK)
    {
        return rv;
    }


    SipTransportLocalAddrSetHandleInStructure(pTranscClient->pMgr->hTransportMgr,
                                              &pTranscClient->localAddr,
                                              hLocalAddr,RV_FALSE);
    return rv;
}


/***************************************************************************
 * SipTranscClientGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address the Transc-Client will use to send outgoing
 *          requests to a destination with the supplied
 *          transport type and address type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - The transaction-client handle
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:    strLocalIPAddress    - The local address the Transc-Client will use
 *                            for the supplied transport and address types.
 *                            (The buffer must be longer than 48 bytes).
 *          pLocalPort       - The local port the Transc-Client will use.
 *          if_name - the local interface name
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetLocalAddress(
                            IN  SipTranscClient				*pTranscClient,
                            IN  RvSipTransport				eTransportType,
                            IN  RvSipTransportAddressType	eAddressType,
                            OUT  RvChar						*localAddress,
                            OUT  RvUint16					*localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                   *if_name
			    , OUT  RvUint16                 *vdevblock
#endif
			    )
{
    RvStatus                   rv;

    if (NULL == pTranscClient || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientGetLocalAddress - TranscClient 0x%p - Getting local %s,%saddress",pTranscClient,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType)));


    rv = SipTransportGetLocalAddressByType(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->localAddr,
                                           eTransportType,
                                           eAddressType,
                                           localAddress,
                                           localPort
#if defined(UPDATED_BY_SPIRENT)
					   ,if_name
					   ,vdevblock
#endif
					   );

    if(RV_OK != rv)
    {
       RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetLocalAddress - TranscClient 0x%p - Local %s %s address not found", pTranscClient,
                   SipCommonConvertEnumToStrTransportType(eTransportType),
                   SipCommonConvertEnumToStrTransportAddressType(eAddressType)));
    }

    return rv;
}


/***************************************************************************
 * SipTranscClientSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the transc-client object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameters such as the IP address or host name,
 *          transport, port and compression type.
 *          Requests sent by this object will use the outbound details
 *          specifications as a remote address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transaction-client.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundDetails(
                            IN  SipTranscClient					*pTranscClient,
                            IN  RvSipTransportOutboundProxyCfg	*pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg)
{
    RvStatus   rv = RV_OK;

    RV_UNUSED_ARG(sizeOfCfg);

    if (NULL == pTranscClient || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }


    /* Setting the Host Name of the outbound proxy BEFORE the ip address  */
    /* in order to assure that is the IP address is set it is going to be */
    /* even if the host name is set too (due to internal preference)      */
    if (pOutboundCfg->strHostName            != NULL &&
        strcmp(pOutboundCfg->strHostName,"") != 0)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientSetOutboundDetails - Transc-Client 0x%p - Setting an outbound host name %s:%d ",
            pTranscClient, (pOutboundCfg->strHostName==NULL)?"NULL":pOutboundCfg->strHostName,pOutboundCfg->port));

        /*we set all info on the size an it will be copied to the Transaction on success*/
        rv = SipTransportSetObjectOutboundHost(pTranscClient->pMgr->hTransportMgr,
                                               &pTranscClient->outboundAddr,
                                               pOutboundCfg->strHostName,
                                               (RvUint16)pOutboundCfg->port,
                                               pTranscClient->pMgr->hGeneralMemPool,
                                               pTranscClient->hPage);
        if(RV_OK != rv)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetOutboundDetails - Transc-Client 0x%p Failed (rv=%d)", pTranscClient,rv));
            return rv;
        }
    }

	/*Setting the transport type is done here in order to use it when undefined port is set*/
	pTranscClient->outboundAddr.ipAddress.eTransport = pOutboundCfg->eTransport;

    /* Setting the IP Address of the outbound proxy */
    if(pOutboundCfg->strIpAddress                            != NULL &&
       strcmp(pOutboundCfg->strIpAddress,"")                 != 0    &&
       strcmp(pOutboundCfg->strIpAddress,IPV4_LOCAL_ADDRESS) != 0    &&
       memcmp(pOutboundCfg->strIpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) != 0)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientSetOutboundDetails - Transc-Client 0x%p - Setting an outbound address %s:%d ",
            pTranscClient, (pOutboundCfg->strIpAddress==NULL)?"0":pOutboundCfg->strIpAddress,pOutboundCfg->port));

        rv = SipTransportSetObjectOutboundAddr(pTranscClient->pMgr->hTransportMgr,
                                               &pTranscClient->outboundAddr,
                                               pOutboundCfg->strIpAddress,
                                               pOutboundCfg->port);
        if (RV_OK != rv)
        {
            RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetOutboundDetails - Failed to set outbound address for Transc-Client 0x%p (rv=%d)",
                pTranscClient,rv));
            return rv;
        }
    }

     /* Setting the Transport type of the outbound proxy */
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetOutboundDetails - Transc-Client 0x%p - Setting outbound transport to %s",
               pTranscClient, SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));
    

#ifdef RV_SIGCOMP_ON
    /* Setting the Compression type of the outbound proxy */
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetOutboundDetails - Transc-Client 0x%p - Setting outbound compression to %s",
               pTranscClient, SipTransportGetCompTypeName(pOutboundCfg->eCompression)));
    pTranscClient->outboundAddr.eCompType = pOutboundCfg->eCompression;


	/* setting the sigcomp-id to the outbound proxy */
    if (pOutboundCfg->strSigcompId            != NULL &&
        strcmp(pOutboundCfg->strSigcompId,"") != 0)
	{
	    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
		          "SipTranscClientSetOutboundDetails - Trans-client 0x%p - Setting outbound sigcomp-id to %s",
				  pTranscClient, (pOutboundCfg->strSigcompId==NULL)?"0":pOutboundCfg->strSigcompId));

		rv = SipTransportSetObjectOutboundSigcompId(pTranscClient->pMgr->hTransportMgr,
													&pTranscClient->outboundAddr,
													pOutboundCfg->strSigcompId,
													pTranscClient->pMgr->hGeneralMemPool,
													pTranscClient->hPage);
		if (RV_OK != rv)
		{
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetOutboundDetails - Failed to set outbound sigcomp-id for Transc-Clinet 0x%p (rv=%d)",
                pTranscClient,rv));
            return rv;
		}
	}
#endif /* RV_SIGCOMP_ON */

    return rv;
}

/***************************************************************************
 * SipTranscClientGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the transaction-client object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type. If the outbound details were not set to the specific
 *          transaction-client object, but the outbound proxy was defined to the SIP
 *          Stack on initialization, the SIP Stack parameters will be returned.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transaction-client.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to outbound proxy configuration structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundDetails(
                            IN  SipTranscClient					*pTranscClient,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg	*pOutboundCfg)
{
    RvStatus    rv;
    RvUint16    port;

    pOutboundCfg->port = UNDEFINED;

    RV_UNUSED_ARG(sizeOfCfg);

    if (NULL == pTranscClient || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Retrieving the outbound host name */
    rv = SipTransportGetObjectOutboundHost(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           pOutboundCfg->strHostName,
                                           &port);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                 "SipTranscClientGetOutboundDetails - Failed for TranscClient 0x%p (rv=%d)", pTranscClient,rv));
        return rv;
    }
    pOutboundCfg->port = ((RvUint16)UNDEFINED==port || port==0) ? UNDEFINED:port;

    /* Retrieving the outbound ip address */
    rv = SipTransportGetObjectOutboundAddr(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           pOutboundCfg->strIpAddress,
                                           &port);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "SipTranscClientGetOutboundDetails - Failed to get outbound address of TranscClient 0x%p,(rv=%d)",
                   pTranscClient, rv));
        return rv;
    }
    /* If port was not updated after SipTransportGetObjectOutboundHost(),
       update it now -         after SipTransportGetObjectOutboundAddr() */
    if (UNDEFINED == pOutboundCfg->port)
    {
        pOutboundCfg->port = ((RvUint16)UNDEFINED==port) ? UNDEFINED:port;
    }

    /* Retrieving the outbound transport type */
    rv = SipTransportGetObjectOutboundTransportType(pTranscClient->pMgr->hTransportMgr,
                                                    &pTranscClient->outboundAddr,
                                                    &pOutboundCfg->eTransport);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundDetails - Failed to get transport for TranscClient 0x%p (rv=%d)", pTranscClient,rv));
        return rv;
    }

#ifdef RV_SIGCOMP_ON
    /* Retrieving the outbound compression type */
    rv = SipTransportGetObjectOutboundCompressionType(pTranscClient->pMgr->hTransportMgr,
                                                      &pTranscClient->outboundAddr,
                                                      &pOutboundCfg->eCompression);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundDetails - Failed to get compression for TranscClient 0x%p (rv=%d)", pTranscClient,rv));
        return rv;
    }

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
             "SipTranscClientGetOutboundDetails - outbound details of TranscClient 0x%p: comp=%s",
              pTranscClient,SipTransportGetCompTypeName(pOutboundCfg->eCompression)));

	/* retrieving the outbound sigcomp-id */
	rv = SipTransportGetObjectOutboundSigcompId(pTranscClient->pMgr->hTransportMgr,
												&pTranscClient->outboundAddr,
												pOutboundCfg->strSigcompId);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundDetails - Failed to get sigcomp-id for TranscClient 0x%p (rv=%d)",
				  pTranscClient,rv));
        return rv;
    }


#endif /* RV_SIGCOMP_ON */

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
             "SipTranscClientGetOutboundDetails - outbound details of TranscClient 0x%p: address=%s, host=%s, port=%d, trans=%s",
              pTranscClient, pOutboundCfg->strIpAddress,pOutboundCfg->strHostName, pOutboundCfg->port,
              SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy IP address of the Transc-Client object. All clients
 *          requests sent by this Transc-Client will be sent directly to the Transc-Client
 *          outbound IP address.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transaction-client handle
 *            strOutboundAddrIp   - The outbound IP to be set to this
 *                              transc-client.
 *          outboundProxyPort - The outbound port to be set to this
 *                              Transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundAddress(
                            IN  SipTranscClient		 *pTranscClient,
                            IN  RvChar               *strOutboundAddrIp,
                            IN  RvUint16              outboundProxyPort)
{
    RvStatus           rv=RV_OK;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if(strOutboundAddrIp == NULL || strcmp(strOutboundAddrIp,"") == 0 ||
       strcmp(strOutboundAddrIp,IPV4_LOCAL_ADDRESS) == 0 ||
       memcmp(strOutboundAddrIp, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) == 0)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientSetOutboundAddress - Failed, ip address  not supplied"));
        return RV_ERROR_BADPARAM;
    }


      RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetOutboundAddress - Transc-Client 0x%p - Setting an outbound address %s:%d ",
              pTranscClient, (strOutboundAddrIp==NULL)?"0":strOutboundAddrIp,outboundProxyPort));

    rv = SipTransportSetObjectOutboundAddr(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           strOutboundAddrIp,(RvInt32)outboundProxyPort);

    if (RV_OK != rv)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                    "SipTranscClientSetOutboundAddress - Failed to set outbound address for Transc-Client 0x%p (rv=%d)",
                   pTranscClient,rv));
        return rv;
    }
    return rv;
}

/***************************************************************************
 * SipTranscClientSetCSeqStep
 * ------------------------------------------------------------------------
 * General: Sets the CSeq-Step associated with the transaction-client.
 *          The supplied CSeq must be bigger then zero.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 *           cSeqStep -  The transc-client's CSeq-Step.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCSeqStep (
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RvInt32				cSeqStep)
{
    if (NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(cSeqStep == 0)
    {
        return RV_ERROR_BADPARAM;
    }


	SipCommonCSeqSetStep(cSeqStep-1,&pTranscClient->cseq);

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Returns the CSeq-Step assosiated with the transc-client.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE - The given transaction-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 * Output:     pCSeqStep -  The transaction-client's CSeq-Step.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCSeqStep (
                                      IN  SipTranscClient		*pTranscClient,
                                      OUT RvUint32				*pCSeqStep)
{
	RvStatus	rv;
	
    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }
    if (NULL == pCSeqStep)
    {
        return RV_ERROR_NULLPTR;
    }
	rv = SipCommonCSeqGetStep(&pTranscClient->cseq,(RvInt32*)pCSeqStep);
    if (rv != RV_OK)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientGetCSeqStep - transcClient 0x%p failed to retrieve CSeq",pTranscClient));
	}
    return rv;
}

/***************************************************************************
 * SipTranscClientSetOutboundHostName
 * ------------------------------------------------------------------------
 * General:  Sets the outbound proxy host name of the Transc-Client object.
 *           The outbound host name will be resolved each time a 
 *           request is sent and the clients requests will be sent to the
 *           resolved IP address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient           - Handle to the TanscClient
 *            strOutboundHost    - The outbound proxy host to be set to this
 *                               TranscClient.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               TranscClient. If you set the port to zero it
 *                               will be determined using the DNS procedures.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundHostName(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvChar					*strOutboundHost,
                            IN  RvUint16				outboundPort)
{
    RvStatus     rv          =RV_OK;
    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    if (strOutboundHost == NULL ||
        strcmp(strOutboundHost,"") == 0)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientSetOutboundHostName - Failed, host name was not supplied"));
        return RV_ERROR_BADPARAM;
    }

      RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetOutboundHostName - Transc Client 0x%p - Setting an outbound host name %s:%d ",
              pTranscClient, (strOutboundHost==NULL)?"NULL":strOutboundHost,outboundPort));

    /*we set all info on the size an it will be copied to the Transc Client on success*/
    rv = SipTransportSetObjectOutboundHost(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           strOutboundHost,
                                           outboundPort,
                                           pTranscClient->pMgr->hGeneralMemPool,
                                           pTranscClient->hPage);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientSetOutboundHostName - Transc Client 0x%p Failed (rv=%d)", pTranscClient,rv));
        return rv;
    }
    return rv;
}

/***************************************************************************
 * SipTranscClientGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy the TranscClient is using. If an
 *          outbound host was set to the TranscClient this host will be
 *          returned. If no outbound host was set to the TranscClient
 *          but an outbound host was configured for the stack, the configured
 *          address is returned. '\0' is returned if the TranscClient is not using
 *          an outbound host.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTranscClient      - Handle to the TranscClient
 * Output:
 *            srtOutboundHostName   -  A buffer with the outbound proxy host name
 *          pOutboundPort - The outbound proxy port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundHostName(
                            IN   SipTranscClient		*pTranscClient,
                            OUT  RvChar					*strOutboundHostName,
                            OUT  RvUint16				*pOutboundPort)
{
    RvStatus     rv;
    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    
    if (strOutboundHostName == NULL || pOutboundPort == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundHostName - Failed, NULL pointer supplied"));

        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetObjectOutboundHost(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           strOutboundHostName,
                                           pOutboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                 "SipTranscClientGetOutboundHostName - Failed for TranscClient 0x%p (rv=%d)", pTranscClient,rv));
    }

    return RV_OK;
}

#if defined(UPDATED_BY_SPIRENT)

RvStatus RVCALLCONV SipTranscClientSetCct(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvInt32    			cctContext) {
  RvStatus     rv = RV_OK;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }


	RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			  "SipTranscClientSetURIFlag - Transc-client 0x%p - Setting cctContext to %d",
			  pTranscClient,cctContext));

    pTranscClient->cctContext = cctContext;

    return rv;
}

/***************************************************************************
 * SipTranscClientSetURIFlag
 * ------------------------------------------------------------------------
 * General: Sets the Req-URI configuration flag of the Transc-Client 
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The Transc-Client handle
 *            uri_flag          - URI configuration flag to be set to this
 *                              Transc-Client
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetURIFlag(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvUint32    			uri_flag)
{
    RvStatus     rv = RV_OK;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }


	RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			  "SipTranscClientSetURIFlag - Transc-client 0x%p - Setting URI flag to %d",
			  pTranscClient,uri_flag));

    pTranscClient->URI_cfg = uri_flag;

    return rv;
}

#endif


/***************************************************************************
 * SipTranscClientSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the Transc-Client outbound proxy.
 *          This transport will be used for the outbound proxy that you set
 *          using the SipTranscClientSetOutboundAddress() function or the
 *          SipTranscClientSetOutboundHostName() function.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The Transc-Client handle
 *            eTransportType    - The outbound proxy transport to be set to this
 *                              Transc-Client
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundTransport(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvSipTransport			eTransportType)
{
    RvStatus     rv = RV_OK;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }


	RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			  "SipTranscClientSetOutboundTransport - Transc-client 0x%p - Setting outbound transport to %s",
			  pTranscClient,SipCommonConvertEnumToStrTransportType(eTransportType)));

    pTranscClient->outboundAddr.ipAddress.eTransport = eTransportType;

    return rv;
}

/***************************************************************************
 * SipTranscClientGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Gets the address of outbound proxy the Transc-Client is using. If an
 *          outbound address was set to the Transc-Client this address will be
 *          returned. If no outbound address was set to the transc-client
 *          but an outbound proxy was configured for the stack, the configured
 *          address is returned. '\0' is returned if the transc-client is not using
 *          an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transction-client handle
 * Output:  outboundProxyIp   - A buffer with the outbound proxy IP the transc-client.
 *                              is using(must be longer than 48).
 *          pOutboundProxyPort - The outbound proxy port the transc-client is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundAddress(
                            IN   SipTranscClient		*pTranscClient,
                            OUT  RvChar					*outboundProxyIp,
                            OUT  RvUint16				*pOutboundProxyPort)
{
    RvStatus           rv;

    if (NULL == pTranscClient || outboundProxyIp == NULL || pOutboundProxyPort== NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    
    if(outboundProxyIp == NULL || pOutboundProxyPort == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundAddress - Failed, NULL pointer supplied"));
        return RV_ERROR_BADPARAM;
    }

    rv = SipTransportGetObjectOutboundAddr(pTranscClient->pMgr->hTransportMgr,
                                           &pTranscClient->outboundAddr,
                                           outboundProxyIp,
                                           pOutboundProxyPort);
    if(rv != RV_OK)
    {
          RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                       "SipTranscClientGetOutboundAddress - Failed to get outbound address of Transc-Client 0x%p,(rv=%d)",
                     pTranscClient, rv));
          return rv;
    }
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
             "SipTranscClientGetOutboundAddress - outbound address of Transc-Client 0x%p: %s:%d",
              pTranscClient, outboundProxyIp, *pOutboundProxyPort));

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of outbound proxy the Transc-Client is using. If an
 *          outbound transport was set to the transc-client this transport will be
 *          returned. If no outbound transport was set to the transc-client
 *          but an outbound proxy was configured for the stack, the configured
 *          transport is returned. RVSIP_TRANSPORT_UNDEFINED is returned if the
 *          transc-client is not using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transaction-client handle
 * Output:  eOutboundProxyTransport    - The outbound proxy transport type
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundTransport(
                            IN   SipTranscClient		*pTranscClient,
                            OUT  RvSipTransport			*eOutboundProxyTransport)
{
    RvStatus   rv;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    
    if(eOutboundProxyTransport == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundTransport - Failed, eOutboundProxyTransport is NULL"));
        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetObjectOutboundTransportType(pTranscClient->pMgr->hTransportMgr,
                                                    &pTranscClient->outboundAddr,
                                                    eOutboundProxyTransport);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "SipTranscClientGetOutboundTransport - Failed for TranscClient 0x%p (rv=%d)", pTranscClient,rv));
        return rv;
    }

    return RV_OK;
}


/***************************************************************************
 * SipTranscClientSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the Transc-Client persistency definition on run time.
 *          The function receives a boolean value that indicates whether the
 *          application wishes this Transc-Client to be persistent or not.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscClient - The Transc-Client handle
 *          bIsPersistent - Determines the Transc-Client persistency definition.
 *                          RV_TRUE = persistent, RV_FALSE otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetPersistency(
                           IN SipTranscClient			*pTranscClient,
                           IN RvBool					bIsPersistent)
{
    RvStatus  rv;

    if(pTranscClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(pTranscClient->bIsPersistent == RV_TRUE && bIsPersistent == RV_FALSE &&
        pTranscClient->hTcpConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(pTranscClient->hTcpConn,
                                            (RvSipTransportConnectionOwnerHandle)pTranscClient);
           if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                 "SipTranscClientSetPersistency - Failed to detach pTranscClient 0x%p from prev connection 0x%p",pTranscClient,pTranscClient->hTcpConn));
            return rv;
        }
        pTranscClient->hTcpConn = NULL;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetPersistency - Setting TranscClient 0x%p persistency to %d ",
              pTranscClient,bIsPersistent));

    pTranscClient->bIsPersistent = bIsPersistent;
    return RV_OK;
}
/***************************************************************************
 * SipTranscClientGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the Transc-Client persistency definition.
 *          RV_TRUE indicates that the Transc-Client is persistent, RV_FALSE
 *          otherwise.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscClient - The Transc-Client handle
 * Output:  pbIsPersistent - The Transc-Client persistency definition.
 *                           RV_TRUE indicates that the Transc-Client is persistent,
 *                           RV_FALSE otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetPersistency(
        IN  SipTranscClient							*pTranscClient,
        OUT RvBool									*pbIsPersistent)
{

	*pbIsPersistent = RV_FALSE;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    *pbIsPersistent = pTranscClient->bIsPersistent;

    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the Transc-Client transactions.
 *          The Transc-Client object holds this connection in its internal database.
 *          Whenever the Transc-Client creates a new transaction it supplies
 *          the transaction with given connection.
 *          The transaction will use the connection only if it fits the transaction's
 *          local and remote addresses.
 *          Otherwise the transaction will either locate a suitable connection in the hash
 *          or create a new connection. The Transc-Client object will be informed that
 *          the supplied connection did not fit and that a different connection was used and
 *          will update its database.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client
 *          hConn - Handle to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetConnection(
                   IN  SipTranscClient						*pTranscClient,
                   IN  RvSipTransportConnectionHandle		hConn)
{
    RvStatus               rv;
    if (NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetConnection - TranscClient 0x%p - Setting connection 0x%p", pTranscClient,hConn));

    if(pTranscClient->bIsPersistent == RV_FALSE)
    {
         RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "SipTranscClientSetConnection - TranscClient 0x%p - Cannot set connection when the Transc-Client is not persistent", pTranscClient));
         return RV_ERROR_ILLEGAL_ACTION;
    }

    if(hConn != NULL)
    {
        /*first try to attach the TranscClient to the connection*/
        rv = SipTransportConnectionAttachOwner(hConn,
                               (RvSipTransportConnectionOwnerHandle)pTranscClient,
                               &pTranscClient->transportEvHandler,sizeof(pTranscClient->transportEvHandler));

        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetConnection - Failed to attach TranscClient 0x%p to connection 0x%p",pTranscClient,hConn));
            return rv;
        }
    }
    /*detach from the transaction current owner if there is one and set the new connection
      handle*/
    if(pTranscClient->hTcpConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(pTranscClient->hTcpConn,
                                            (RvSipTransportConnectionOwnerHandle)pTranscClient);
           if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetConnection - Failed to detach Transc-Client 0x%p from prev connection 0x%p",pTranscClient,pTranscClient->hTcpConn));
        }
    }
    pTranscClient->hTcpConn = hConn;

    return RV_OK;
}
/***************************************************************************
 * SipTranscClientGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Transc-Client.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetConnection(
                            IN  SipTranscClient					*pTranscClient,
                           OUT  RvSipTransportConnectionHandle  *phConn)
{
    if (NULL == pTranscClient || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phConn = NULL;

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientGetConnection - pTranscClient 0x%p - Getting connection 0x%p",pTranscClient,pTranscClient->hTcpConn));

    *phConn = pTranscClient->hTcpConn;

    return RV_OK;

}

/***************************************************************************
* SipTranscClientSetTranscTimers
* ------------------------------------------------------------------------
* General: Sets timeout values for the TranscClient's transactions timers.
*          If some of the fields in pTimers are not set (UNDEFINED), this
*          function will calculate it, or take the values from configuration.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    Input: hTranscClient - Handle to the TranscClient object.
*           pTimers - Pointer to the struct contains all the timeout values.
*           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetTranscTimers(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvSipTimers				*pTimers,
                            IN  RvInt32					sizeOfTimersStruct)
{
    RvStatus   rv;

    if (NULL == pTranscClient)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientSetTranscTimers - pTranscClient 0x%p - setting timers",
              pTranscClient));

    rv = TranscClientSetTimers(pTranscClient,pTimers);

    RV_UNUSED_ARG(sizeOfTimersStruct);
    return rv;

}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * SipTranscClientSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message,
 *          belonging to the TranscClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTranscClient - Handle to the TranscClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - Pointer to the struct that contains all the parameters.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSctpMsgSendingParams(
                    IN  SipTranscClient						*pTranscClient,
                    IN  RvUint32							sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams	*pParams)
{
    RvStatus rv;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pTranscClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(&params,pParams,minStructSize);

    memcpy(&pTranscClient->sctpParams, pParams, sizeof(RvSipTransportSctpMsgSendingParams));

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientSetSctpMsgSendingParams - Transc-client 0x%p - SCTP params(stream=%d, flag=0x%x) were set",
        pTranscClient, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    return RV_OK;
}

/******************************************************************************
 * SipTranscClientGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message, belonging
 *          to the TranscClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTranscClient - Handle to the TranscClient object.
 *           sizeOfStruct - Size of the SipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSctpMsgSendingParams(
                    IN  SipTranscClient						*pTranscClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pTranscClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(pParams, &pTranscClient->sctpParams, sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(pParams,&params,minStructSize);

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientGetSctpMsgSendingParams - Transc-client 0x%p - SCTP params(stream=%d, flag=0x%x) were get",
        pTranscClient, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    return RV_OK;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON

/******************************************************************************
 * SipTranscClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the TRansc-Client.
 *          As a result of this operation, all messages, sent by this
 *          Transc-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTranscClient - Handle to the Transc-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSecObj(
                                        IN  SipTranscClient			*pTranscClient,
                                        IN  RvSipSecObjHandle       hSecObj)
{
    RvStatus   rv;

    if(NULL == pTranscClient)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientSetSecObj - Transc-Client 0x%p - Set Security Object %p",
        pTranscClient, hSecObj));

	if (pTranscClient->hSecAgree != NULL)
	{
		/* We do not allow setting security-object when there is a security-agreement attached to this Transc-client */
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
					"SipTranscClientSetSecObj - Transc-client 0x%p - Setting security-object (0x%p) is illegal when the Transc-client has a security-agreement (security-agreement=0x%p)",
					pTranscClient, hSecObj, pTranscClient->hSecAgree));
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = TranscClientSetSecObj(pTranscClient, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientSetSecObj - Transc-Client 0x%p - failed to set Security Object %p(rv=%d:%s)",
            pTranscClient, hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SipTranscClientGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Transc-Client.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTranscClient - Handle to the Transc-Client object.
 *  Output: phSecObj   - Handle to the Security Object.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSecObj(
                                        IN  SipTranscClient			*pTranscClient,
                                        OUT RvSipSecObjHandle*      phSecObj)
{

    if(NULL==pTranscClient || NULL==phSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientGetSecObj - Transc-Client 0x%p - Get Security Object",
        pTranscClient));

    *phSecObj = pTranscClient->hSecObj;

    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/


/***************************************************************************
 * SipTranscClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the TranscClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 ***************************************************************************/
void SipTranscClientDetachFromConnection(IN SipTranscClient	*pTranscClient)
{
	TranscClientDetachFromConnection(pTranscClient);
}

/***************************************************************************
 * SipTranscClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the TranscClient as the connection owner
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 *            hConn         - The connection handle
 ***************************************************************************/
void SipTranscClientAttachOnConnection(IN SipTranscClient				*pTranscClient,
                                 IN RvSipTransportConnectionHandle		  hConn)
{
	TranscClientAttachOnConnection(pTranscClient, hConn);
}

/***************************************************************************
 * SipTranscClientSetTransportEvHandle
 * ------------------------------------------------------------------------
 * General: Sets the transport ev handler that will be in use for this client.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            evHandler		- The transport ev handlere of this client.
 ***************************************************************************/
void SipTranscClientSetTransportEvHandle(SipTranscClient					*pTranscClient, 
										 RvSipTransportConnectionEvHandlers evHandler)
{
	pTranscClient->transportEvHandler = evHandler;	
}


/***************************************************************************
 * SipTranscClientDisableCompression
 * ------------------------------------------------------------------------
 * General:Disables the compression mechanism in a transaction-client.
 *         This means that even if the message indicates compression,
 *         it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTranscClient    - The handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDisableCompression(
                                      IN  SipTranscClient		*pTranscClient)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv          = RV_OK;
    if(pTranscClient == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "SipTranscClientDisableCompression - disabling the compression in TranscClient 0x%p",
              pTranscClient));

    pTranscClient->bSigCompEnabled = RV_FALSE;

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(pTranscClient);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * SipTranscClientGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: The function retrieve the auth-object that is currently being
 *          processed by the authenticator.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCurrProcessedAuthObj (
                                      IN   SipTranscClient		*pTranscClient,
                                      OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv = RV_OK;
    if(pTranscClient == NULL || phAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(pTranscClient->pAuthListInfo == NULL || pTranscClient->hListAuthObj == NULL)
    {
        *phAuthObj = NULL;
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "SipTranscClientGetCurrProcessedAuthObj - call-leg 0x%p - pAuthListInfo=0x%p, hListAuthObj=0x%p ",
            pTranscClient, pTranscClient->pAuthListInfo, pTranscClient->hListAuthObj ));
        return RV_ERROR_NOT_FOUND;
    }

    *phAuthObj = pTranscClient->pAuthListInfo->hCurrProcessedAuthObj;
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientGetCurrProcessedAuthObj - call-leg 0x%p - currAuthObj=0x%p ",
        pTranscClient, *phAuthObj));

    return rv;
}
/***************************************************************************
 * SipTranscClientAuthObjGet
 * ------------------------------------------------------------------------
 * General: The function retrieve auth-objects from the list in the Transc-Client.
 *          you may get the first/last/next object.
 *          in case of getting the next object, please supply the current
 *          object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - Handle to the Transc-Client.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientAuthObjGet (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipListLocation     eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    if(pTranscClient == NULL || phAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(pTranscClient->pAuthListInfo == NULL || pTranscClient->hListAuthObj == NULL)
    {
        *phAuthObj = NULL;
        return RV_ERROR_NOT_FOUND;
    }

    rv = SipAuthenticatorAuthListGetObj(pTranscClient->pMgr->hAuthModule, pTranscClient->hListAuthObj,
                                       eLocation, hRelativeAuthObj, phAuthObj);
    return rv;
}

/***************************************************************************
 * SipTranscClientAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an auth-obj from the list in the Transc-Client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient  - Handle to the Transc-Client.
 *            hAuthObj - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientAuthObjRemove (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipAuthObjHandle   hAuthObj)
{
    RvStatus     rv;
    if(pTranscClient == NULL || hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(pTranscClient->pAuthListInfo == NULL || pTranscClient->hListAuthObj == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }

    rv = SipAuthenticatorAuthListRemoveObj(pTranscClient->pMgr->hAuthModule,
                                           pTranscClient->hListAuthObj, hAuthObj);
    return rv;
}
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTranscClientSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header in the outgoing request.
 *          An initial authorization header is a header that contains
 *          only the client private identity, and not real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The transc-client will set the initial header to the message, only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The transaction-client handle.
 *            hAuthorization - The Authorization header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetInitialAuthorization (
                                         IN SipTranscClient					*pTranscClient,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization)
{
    RvStatus            retStatus;

    if (NULL == pTranscClient || NULL == hAuthorization)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
        "SipTranscClientSetInitialAuthorization - Setting in client 0x%p ",pTranscClient));

    /* Check if the given header was constructed on the transaction-client
       object's page and if so attach it */
    if((SipAuthorizationHeaderGetPool(hAuthorization) == pTranscClient->pMgr->hGeneralMemPool) &&
       (SipAuthorizationHeaderGetPage(hAuthorization) == pTranscClient->hPage))
    {
        pTranscClient->hInitialAuthorization = hAuthorization;
    }
    else /* need to copy the given header to the Transc=client page */
    {
        if(pTranscClient->hInitialAuthorization == NULL)
        {
            retStatus = RvSipAuthorizationHeaderConstruct(pTranscClient->pMgr->hMsgMgr,
                                                        pTranscClient->pMgr->hGeneralMemPool,
                                                        pTranscClient->hPage,
                                                        &pTranscClient->hInitialAuthorization);
            if(retStatus != RV_OK)
            {
                RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                    "SipTranscClientSetInitialAuthorization - Transaction-client 0x%p - Failed to construct header (retStatus=%d)",
                    pTranscClient, retStatus));
                return retStatus;
            }
        }
        retStatus = RvSipAuthorizationHeaderCopy(pTranscClient->hInitialAuthorization, hAuthorization);
        if(retStatus != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "SipTranscClientSetInitialAuthorization - Transaction-client 0x%p - Failed to copy header (retStatus=%d)",
                pTranscClient, retStatus));
            return retStatus;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * SipTranscClientSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this transc-client. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the transc-client will exploit this agreement and the data
 *          it brings. If not, the transc-client will use this security-agreement
 *          object to negotiate an agreement with the remote party.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient   - Handle to the transc-client.
 *          hSecAgree    - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSecAgree(
							IN  SipTranscClient				*pTranscClient,
							IN  RvSipSecAgreeHandle          hSecAgree)
{
	RvStatus          rv;

    if (pTranscClient == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	rv = TranscClientSecAgreeAttachSecAgree(pTranscClient, hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
					"SipTranscClientSetSecAgree - Transc-client 0x%p - Failed to attach security-agreement=0x%p (retStatus=%d)",
					pTranscClient, hSecAgree, rv));
	}

	return rv;
}

/***************************************************************************
 * SipTranscClientSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Handle detach secAgree from this transcClient.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeDetachSecAgree(SipTranscClient *pTranscClient)
{
	return TranscClientSecAgreeDetachSecAgree(pTranscClient);

}

/***************************************************************************
 * SipTranscClientSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Handle detach secAgree event from secAgree.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeDetachSecAgreeEv(SipTranscClient *pTranscClient, RvSipSecAgreeHandle hSecAgree)
{
	if (pTranscClient->hSecAgree == hSecAgree)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientSecAgreeDetachSecAgreeEv - transc-client 0x%p - detaching from security-agreement 0x%p",
			pTranscClient, pTranscClient->hSecAgree));
		pTranscClient->hSecAgree = NULL;
		TranscClientDetachFromSecObjIfNeeded(pTranscClient, hSecAgree);
	}
	else
	{
		/* Trying to detach a security-agreement object that is not the security-agreement of this reg-client */
		RvLogWarning(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"SipTranscClientSecAgreeDetachSecAgreeEv - Called to detach transc-client 0x%p from security-agreement 0x%p, but the transc-client's security-agreement is 0x%p",
			pTranscClient, hSecAgree, pTranscClient->hSecAgree));
	}
	return RV_OK;
	
}

/***************************************************************************
 * SipTranscClientSecAgreeAttachSecObj
 * ------------------------------------------------------------------------
 * General: Attaches the secObj to the secAgree if needed.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeAttachSecObj(SipTranscClient *pTranscClient, RvSipSecAgreeHandle hSecAgree, RvSipSecObjHandle hSecObj)
{
	RvStatus rv = RV_OK;
	if (pTranscClient->hSecAgree == hSecAgree)
	{	
		rv = TranscClientAttachToSecObjIfNeeded(pTranscClient, hSecObj);
	}
	return rv;
}
/***************************************************************************
 * SipTranscClientGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the transc-client.
 *          The security-agreement object captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSecAgree(
									IN   SipTranscClient		*pTranscClient,
									OUT  RvSipSecAgreeHandle    *phSecAgree)
{

    if (NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (NULL == phSecAgree)
	{
		return RV_ERROR_BADPARAM;
	}

	*phSecAgree = pTranscClient->hSecAgree;

	return RV_OK;
}

/***************************************************************************
 * SipTranscClientGetActiveTransc
 * ------------------------------------------------------------------------
 * General: Gets the active transaction of the transc-client.
 *          The active transaction is the transaction in progress of
 *          the transc-client.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 * Output:  phActiveTransc   - Handle to the active transaction
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetActiveTransc(
									IN   SipTranscClient	  *pTranscClient,
									OUT  RvSipTranscHandle    *phActiveTransc)
{
	if (NULL == pTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (NULL == phActiveTransc)
	{
		return RV_ERROR_BADPARAM;
	}

	*phActiveTransc = pTranscClient->hActiveTransc;

	return RV_OK;
}

#endif /* #ifdef RV_SIP_IMS_ON */

/***************************************************************************
 * TranscClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the transc-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV TranscClientMgrLock(IN TranscClientMgr *pTranscClientMgr)
{
    if (NULL == pTranscClientMgr)
    {
        return;
    }
    RvMutexLock(&pTranscClientMgr->hLock,pTranscClientMgr->pLogMgr);
}


/***************************************************************************
 * TranscClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the transc-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV TranscClientMgrUnLock(IN TranscClientMgr *pTranscClientMgr)
{
    if (NULL == pTranscClientMgr)
    {
        return;
    }
    RvMutexUnlock(&pTranscClientMgr->hLock,pTranscClientMgr->pLogMgr);
}

/***************************************************************************
 * TranscClientCreateNewTransaction
 * ------------------------------------------------------------------------
 * General: Creates a new transaction for the Transaction-client according to the
 *          To and From headers of the Transaction-client object and according to
 *          the Call-Id and CSeq-Step of the transaction-clients manager.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create a new
 *                                   transaction.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The pointer to the transaction-client object.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCreateNewTransaction(
                                            IN  SipTranscClient* pTranscClient)
{
    SipTransactionKey transcKey;
    RvStatus         retStatus,cseqRetStatus;
	
    /*detach from the previous transaction*/
    if(pTranscClient->hActiveTransc != NULL)
    {
        retStatus = RvSipTransactionDetachOwner(pTranscClient->hActiveTransc);
        if (RV_OK != retStatus)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                       "TranscClientCreateNewTransaction - transaction-client 0x%p: Failed to detach from transaction 0x%p - terminating transaction",
                      pTranscClient,pTranscClient->hActiveTransc));
            RvSipTransactionTerminate(pTranscClient->hActiveTransc);
        }
        pTranscClient->hActiveTransc = NULL;
    }
    memset(&transcKey,0,sizeof(transcKey));
    retStatus = SipCommonCSeqPromoteStep(&pTranscClient->cseq);
	if (RV_OK != retStatus)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
		"TranscClientCreateNewTransaction - Transc-client 0x%p: Failed in SipCommonCSeqPromoteStep() (rv=%d)",
		pTranscClient,retStatus));
		return retStatus;
	}

    if (pTranscClient->strCallId == UNDEFINED)
    {
#ifdef SIP_DEBUG
		retStatus = TranscClientGenerateCallId(pTranscClient->pMgr, pTranscClient->hPage,
                                           &(pTranscClient->strCallId), &(pTranscClient->pCallId));
#else
        retStatus = TranscClientGenerateCallId(pTranscClient->pMgr, pTranscClient->hPage,
                                                &(pTranscClient->strCallId), NULL);
#endif
        if (RV_OK != retStatus)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                      "TranscClientCreateNewTransaction - Transaction-client 0x%p: Error generating call-id",
                      pTranscClient));
            return retStatus;
        }
    }
    /* Create a new transaction key */
	SipCommonCSeqGetStep(&pTranscClient->cseq,&transcKey.cseqStep);
    transcKey.hFromHeader = pTranscClient->hFromHeader;
    transcKey.hToHeader = pTranscClient->hToHeader;
    if(pTranscClient->strCallId != UNDEFINED)
    {
        transcKey.strCallId.offset = pTranscClient->strCallId;
        transcKey.strCallId.hPage = pTranscClient->hPage;
        transcKey.strCallId.hPool = pTranscClient->pMgr->hGeneralMemPool;
    }
    
    transcKey.localAddr = pTranscClient->localAddr;

   retStatus = SipTransactionCreate(
                            pTranscClient->pMgr->hTranscMgr , &transcKey,
                            (void*)pTranscClient, SIP_TRANSACTION_OWNER_TRANSC_CLIENT,
                            pTranscClient->tripleLock,
                            &(pTranscClient->hActiveTransc));

    if (RV_OK != retStatus)
    {
        pTranscClient->hActiveTransc = NULL;
        cseqRetStatus = SipCommonCSeqReduceStep(&pTranscClient->cseq);
		if (RV_OK != cseqRetStatus)
		{
			RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientCreateNewTransaction - Transaction-client 0x%p: failed in SipCommonCSeqReduceStep() (rv=%d)",
				pTranscClient,retStatus));
		}
        
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCreateNewTransaction - Transaction-client 0x%p: Error creating a new transaction",
                  pTranscClient));
        return retStatus;
    }

    /* set the Transc-client outbound proxy to the transaction */
    retStatus = SipTransactionSetOutboundAddress(pTranscClient->hActiveTransc, &pTranscClient->outboundAddr);
    if(retStatus != RV_OK)
    {
        SipTransactionTerminate(pTranscClient->hActiveTransc);
        return retStatus;  /*error message will be printed in the function*/
    }

    if(pTranscClient->pTimers != NULL)
    {
        retStatus = SipTransactionSetTimers(pTranscClient->hActiveTransc, pTranscClient->pTimers);
        if(retStatus != RV_OK)
        {
            SipTransactionTerminate(pTranscClient->hActiveTransc);
            return retStatus;  /*error message will be printed in the function*/
        }
    }

    if(pTranscClient->bIsPersistent == RV_TRUE)
    {
        retStatus = RvSipTransactionSetPersistency(pTranscClient->hActiveTransc,RV_TRUE);
        if(retStatus != RV_OK)
        {
            SipTransactionTerminate(pTranscClient->hActiveTransc);
            return retStatus;  /*error message will be printed in the function*/
        }
    }
    if(pTranscClient->hTcpConn != NULL)
    {
        retStatus = RvSipTransactionSetConnection(pTranscClient->hActiveTransc,pTranscClient->hTcpConn);
        if(retStatus != RV_OK)
        {
            SipTransactionTerminate(pTranscClient->hActiveTransc);
            return retStatus;  /*error message will be printed in the function*/
        }
    }


#ifdef RV_SIGCOMP_ON
    if (pTranscClient->bSigCompEnabled == RV_FALSE)
    {
        RvSipTransactionDisableCompression(pTranscClient->hActiveTransc);
    }
#endif /* RV_SIGCOMP_ON */

#if (RV_NET_TYPE & RV_NET_SCTP)
    /* set the Transc-client SCTP message sending parameters to the transaction */
    SipTransactionSetSctpMsgSendingParams(pTranscClient->hActiveTransc,
                                          &pTranscClient->sctpParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
    /* Set Security Object to be used  while sending messages */
    if (NULL != pTranscClient->hSecObj)
    {
        retStatus = RvSipTransactionSetSecObj(pTranscClient->hActiveTransc,
                                              pTranscClient->hSecObj);
        if (RV_OK != retStatus)
        {
            SipTransactionTerminate(pTranscClient->hActiveTransc);
            return retStatus;  /*error message will be printed in the function*/
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

#if defined(UPDATED_BY_SPIRENT)
    RvSipTransactionSetCct(pTranscClient->hActiveTransc,pTranscClient->cctContext);
    RvSipTransactionSetURIFlag(pTranscClient->hActiveTransc,pTranscClient->URI_cfg);
#endif

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
             "TranscClientCreateNewTransaction - Transaction-client 0x%p created new transc 0x%p",
                 pTranscClient,pTranscClient->hActiveTransc));

    return retStatus;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * SipTranscClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks transc-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transcclient.
***********************************************************************************/
RvStatus RVCALLCONV SipTranscClientLockAPI(IN  SipTranscClient *pTranscClient)
{
    RvStatus         retCode;
    SipTripleLock   *tripleLock;
    TranscClientMgr    *pMgr;
    RvInt32          identifier;

    if (pTranscClient == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {

        RvMutexLock(&(pTranscClient->tripleLock)->hLock,pTranscClient->pMgr->pLogMgr);
        pMgr = pTranscClient->pMgr;
        tripleLock = pTranscClient->tripleLock;
        identifier = pTranscClient->transcClientUniqueIdentifier;
        RvMutexUnlock(&(pTranscClient->tripleLock)->hLock,pMgr->pLogMgr);

        if ((pMgr == NULL) || (tripleLock == NULL))
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipTranscClientLockAPI - Locking TranscClient 0x%p: Triple Lock 0x%p", pTranscClient,
                  tripleLock));

        retCode = TranscClientLock(pTranscClient,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pTranscClient->tripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "SipTranscClientLockAPI - Locking TranscClient 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
            pTranscClient,tripleLock));
        TranscClientUnLock(pMgr->pLogMgr,&tripleLock->hLock);

    }

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        TranscClientUnLock(pTranscClient->pMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }
    tripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock,pTranscClient->pMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SipTranscClientLockAPI - Locking TranscClient 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pTranscClient, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * SipTranscClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: UnLocks transc-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
void RVCALLCONV SipTranscClientUnLockAPI(IN  SipTranscClient *pTranscClient)
{
    SipTripleLock        *tripleLock;
    TranscClientMgr        *pMgr;
    RvInt32               lockCnt = 0;

    if (pTranscClient == NULL)
    {
        return;
    }

    pMgr = pTranscClient->pMgr;
    tripleLock = pTranscClient->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock,pTranscClient->pMgr->pLogMgr,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SipTranscClientUnLockAPI - UnLocking TranscClient 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pTranscClient, tripleLock, tripleLock->apiCnt,lockCnt));

    if (tripleLock->apiCnt == 0)
    {
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,NULL);
    }

    tripleLock->apiCnt--;
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }
    TranscClientUnLock(pTranscClient->pMgr->pLogMgr, &tripleLock->hLock);
}


/************************************************************************************
 * SipTrasncClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks transc-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
RvStatus RVCALLCONV SipTranscClientLockMsg(IN  SipTranscClient *pTranscClient)
{
    RvStatus         retCode      = RV_OK;
    RvBool           didILockAPI  = RV_FALSE;
    RvThreadId       currThreadId = RvThreadCurrentId();
    SipTripleLock   *tripleLock;
    TranscClientMgr    *pMgr;
    RvInt32          identifier;

    if (pTranscClient == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&(pTranscClient->tripleLock)->hLock,pTranscClient->pMgr->pLogMgr);
    pMgr       = pTranscClient->pMgr;
    tripleLock = pTranscClient->tripleLock;
    identifier = pTranscClient->transcClientUniqueIdentifier;
    RvMutexUnlock(&(pTranscClient->tripleLock)->hLock,pMgr->pLogMgr);

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "SipTranscClientLockMsg - Exiting without locking TranscClient 0x%p: Triple Lock 0x%p already locked",
                   pTranscClient, tripleLock));

        return RV_OK;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SipTranscClientLockMsg - Locking TranscClient 0x%p: Triple Lock 0x%p",
             pTranscClient, tripleLock));


    RvMutexLock(&tripleLock->hProcessLock,pMgr->pLogMgr);

    for(;;)
    {
        retCode = TranscClientLock(pTranscClient,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
          if (didILockAPI)
          {
            RvSemaphorePost(&tripleLock->hApiLock,NULL);
          }
          return retCode;
        }
        if (tripleLock->apiCnt == 0)
        {

            if (didILockAPI)
            {
                RvSemaphorePost(&tripleLock->hApiLock,NULL);
            }

            break;
        }

        TranscClientUnLock(pTranscClient->pMgr->pLogMgr, &tripleLock->hLock);

        retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
          return retCode;
        }

        didILockAPI = RV_TRUE;
    } 

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SipTranscClientLockMsg - Locking TranscClient 0x%p: Triple Lock 0x%p exiting function",
             pTranscClient, tripleLock));

    return retCode;
}


/************************************************************************************
 * SipTranscClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks transcclient according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
void RVCALLCONV SipTranscClientUnLockMsg(IN  SipTranscClient *pTranscClient)
{
    SipTripleLock    *tripleLock;
    TranscClientMgr     *pMgr;
    RvThreadId        currThreadId = RvThreadCurrentId();

    if (pTranscClient == NULL)
    {
        return;
    }

    pMgr = pTranscClient->pMgr;
    tripleLock = pTranscClient->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "SipTranscClientUnLockMsg - Exiting without unlocking TranscClient 0x%p: Triple Lock 0x%p already locked",
                  pTranscClient, tripleLock));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "SipTranscClientUnLockMsg - UnLocking TranscClient 0x%p: Triple Lock 0x%p",
             pTranscClient, tripleLock));

/*
    tripleLock->objLockThreadId = 0;
*/
    TranscClientUnLock(pTranscClient->pMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
}

#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TranscClientCopyToHeader
 * ------------------------------------------------------------------------
 * General: Copies the given To header to the transaction-client object's To
 *          header.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - Pointer to the transaction-client.
 *            hTo        - Handle to an application constructed To header.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscClientCopyToHeader (
                                      IN  SipTranscClient             *pTranscClient,
                                      IN  RvSipPartyHeaderHandle hTo)
{
    RvStatus retStatus;

    retStatus = SipPartyHeaderConstructAndCopy(pTranscClient->pMgr->hMsgMgr,
                                        pTranscClient->pMgr->hGeneralMemPool,
                                        pTranscClient->hPage,
                                        hTo,
                                        RVSIP_HEADERTYPE_TO,
                                        &(pTranscClient->hToHeader));
    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCopyToHeader - Transaction-client 0x%p: Error setting To header",
            pTranscClient));
        pTranscClient->hToHeader = NULL;
    }

    return retStatus;
}



/***************************************************************************
 * TranscClientCopyFromHeader
 * ------------------------------------------------------------------------
 * General: Copies the given From header to the Transaction-client object's From
 *          header.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - Pointer to the transaction-client.
 *            hFrom        - Handle to an application constructed From header.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscClientCopyFromHeader (
                                      IN  SipTranscClient             *pTranscClient,
                                      IN  RvSipPartyHeaderHandle hFrom)
{
    RvStatus retStatus;

    retStatus = SipPartyHeaderConstructAndCopy(pTranscClient->pMgr->hMsgMgr,
                                                pTranscClient->pMgr->hGeneralMemPool,
                                                pTranscClient->hPage,
                                                hFrom,
                                                RVSIP_HEADERTYPE_FROM,
                                                &(pTranscClient->hFromHeader));
    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCopyFromHeader - Transaction client 0x%p: Error setting To header",
            pTranscClient));
        pTranscClient->hFromHeader = NULL;
    }
    return retStatus;
}


/***************************************************************************
 * TranscClientCopyRequestUri
 * ------------------------------------------------------------------------
 * General: Copies the given address object to the transaction-client object's
 *          Request-Uri.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient  - Pointer to the Transaction-client.
 *            hRequestUri - Handle to an application constructed address object.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscClientCopyRequestUri (
                                      IN  SipTranscClient               *pTranscClient,
                                      IN  RvSipAddressHandle       hRequestUri)
{
    RvStatus        retStatus;
    RvSipAddressType eAddrCurType = RVSIP_ADDRTYPE_UNDEFINED;
    RvSipAddressType eAddrNewType = RVSIP_ADDRTYPE_UNDEFINED;

    if (NULL != pTranscClient->hRequestUri)
    {
        /* getting the address type of the current requestUri */
        eAddrCurType = RvSipAddrGetAddrType(pTranscClient->hRequestUri);
    }
    /* getting the address type of the new requestUri */
    eAddrNewType = RvSipAddrGetAddrType(hRequestUri);
    if (NULL == pTranscClient->hRequestUri || eAddrNewType != eAddrCurType)
    {
        /* Construct a new address object for the transaction-client object */
        if (eAddrNewType != RVSIP_ADDRTYPE_UNDEFINED)
        {
            retStatus = RvSipAddrConstruct(pTranscClient->pMgr->hMsgMgr,
                                           pTranscClient->pMgr->hGeneralMemPool,
                                           pTranscClient->hPage, eAddrNewType,
                                           &(pTranscClient->hRequestUri));
        }
        else
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCopyRequestUri - Transaction client 0x%p: Error unknown type of address of request-uri",
                  pTranscClient));
            return RV_ERROR_INVALID_HANDLE;
        }

        if (RV_OK != retStatus)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCopyRequestUri - Transc-client 0x%p: Error setting destination Request-Url",
                  pTranscClient));
            pTranscClient->hRequestUri = NULL;
            return retStatus;
        }
    }
    /* Copy the given address-object to the transaction-client object's
       Request-Uri field */
    retStatus = RvSipAddrCopy(pTranscClient->hRequestUri, hRequestUri);
    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCopyRequestUri - Transaction client 0x%p: Error setting destination Request-Url",
                  pTranscClient));
        pTranscClient->hRequestUri = NULL;
    }
    return retStatus;
}


/***************************************************************************
 * TranscClientCopyExpiresHeader
 * ------------------------------------------------------------------------
 * General: Copies the given Expires header to the transaction-client object's
 *          Expires header.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - Pointer to the transaction-client.
 *            hExpires   - Handle to an application constructed Expires header.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscClientCopyExpiresHeader (
                                      IN  SipTranscClient               *pTranscClient,
                                      IN  RvSipExpiresHeaderHandle hExpires)
{
    RvStatus retStatus;

    if (NULL == pTranscClient->hExpireHeader)
    {
        /* Construct a new Expires header for the transaction-client object */
        retStatus = RvSipExpiresHeaderConstruct(
                                            pTranscClient->pMgr->hMsgMgr,
                                            pTranscClient->pMgr->hGeneralMemPool,
                                            pTranscClient->hPage,
                                            &(pTranscClient->hExpireHeader));
        if (RV_OK != retStatus)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCopyExpiresHeader - Transaction client 0x%p: Error setting Expires header",
                  pTranscClient));
            pTranscClient->hExpireHeader = NULL;
            return retStatus;
        }
    }
    /* Copy the given Expires header to the transaction-client object's Expires
       header */
    retStatus = RvSipExpiresHeaderCopy(pTranscClient->hExpireHeader, hExpires);
    if (RV_OK != retStatus)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCopyExpiresHeader - Transaction-client 0x%p: Error setting Expires header",
                  pTranscClient));
        pTranscClient->hExpireHeader = NULL;
    }
    return retStatus;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * TranscClientLock
 * ------------------------------------------------------------------------
 * General: If an owner lock exists locks the owner's lock. else locks the
 *          TranscClient's lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pTransaction - The transaction to lock.
 *            pMgr         - pointer to transaction manager
 *            tripleLock     - triple lock of the transaction
 ***************************************************************************/
static RvStatus TranscClientLock(IN SipTranscClient        *pTranscClient,
                                     IN TranscClientMgr    *pMgr,
                                     IN SipTripleLock    *tripleLock,
                                     IN RvInt32        identifier)
{

    if ((pTranscClient == NULL) || (pMgr == NULL) || (tripleLock == NULL))
    {
        return RV_ERROR_NULLPTR;
    }

    RvMutexLock(&tripleLock->hLock,pMgr->pLogMgr); /*lock the TranscClient*/

    if (pTranscClient->transcClientUniqueIdentifier != identifier ||
        pTranscClient->transcClientUniqueIdentifier == 0)
    {
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TranscClientLock - TranscClient 0x%p: object was destructed",
            pTranscClient));
        RvMutexUnlock(&tripleLock->hLock, pTranscClient->pMgr->pLogMgr);
        return RV_ERROR_INVALID_HANDLE;
    }

    return RV_OK;
}

/***************************************************************************
 * TranscClientUnLock
 * ------------------------------------------------------------------------
 * General: un-lock the given lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pLogMgr    - The log mgr pointer.
 *           hLock      - The lock to un-lock.
 ***************************************************************************/
static void RVCALLCONV TranscClientUnLock(IN RvLogMgr *pLogMgr,
                                       IN RvMutex  *hLock)
{
    if (NULL != hLock)
    {
        RvMutexUnlock(hLock,pLogMgr);
    }
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

#endif /* RV_SIP_PRIMITIVES */

