
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
 *                              <TranscClientDNS.c>
 *
 * This file contains implementation for DNS functions.
 * All functions in this file are for RV_DNS_ENHANCED_FEATURES_SUPPORT compilation
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 August 2006
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

#include "TranscClientDNS.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TranscClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is use at MSG_SEND_FAILURE state.
 *          Calling to this function, cancel the sending of the message, and
 *          change state of the state machine back to previous state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc-client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSGiveUp (IN  SipTranscClient	*pTranscClient)
{
    RvStatus rv;
    
	if(pTranscClient->eState != SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE)
    {
        /* error - no msg was failed */
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientDNSGiveUp: transc-client 0x%p - no failed message. transc-client state - %s",
			pTranscClient, TranscClientStateIntoString(pTranscClient->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

	/* 1. terminate active transaction */
    rv = RvSipTransactionTerminate(pTranscClient->hActiveTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientDNSGiveUp: transc-client 0x%p - Failed to terminate transaction 0x%p.",
            pTranscClient, pTranscClient->hActiveTransc));
        return rv;
    }

    pTranscClient->hActiveTransc = NULL;

	if(pTranscClient->bWasActivated == RV_TRUE)
    {
        /* 2. Change regClient state back to registered */
        TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_ACTIVATED,
			SIP_TRANSC_CLIENT_REASON_GIVE_UP_DNS,
			pTranscClient, 0);
    }
    else
    {
		/*Since the transc-client was not activated we are moving the object to failed state*/
		TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_FAILED, SIP_TRANSC_CLIENT_REASON_GIVE_UP_DNS,
								pTranscClient, 0);
    }

    return RV_OK;
}

/***************************************************************************
 * TranscClientDNSContinue
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling this function, creates a new transaction that will
 *          be sent to the next IP in dns list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSContinue (IN  SipTranscClient	*pTranscClient)
{
    RvSipTranscHandle hNewTransc = NULL;
    RvStatus rv;

	if(pTranscClient->eState != SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientDNSContinue: Transc-client 0x%p - no failed message. transc-client state - %s",
			pTranscClient, TranscClientStateIntoString(pTranscClient->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pTranscClient->hActiveTransc == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientDNSContinue: transc-client 0x%p - Failed, no active transaction",pTranscClient));
        return RV_ERROR_UNKNOWN;
    }

    /* 1. continue DNS */
    rv = SipTransactionDNSContinue(pTranscClient->hActiveTransc, (RvSipTranscOwnerHandle)pTranscClient, RV_FALSE, &hNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientDNSContinue: transc-client 0x%p - failed to continue DNS for tansaction 0x%p",
            pTranscClient, pTranscClient->hActiveTransc));
        return rv;
    }
    if(SipTransactionDetachOwner(pTranscClient->hActiveTransc) != RV_OK)
    {
        RvSipTransactionTerminate(pTranscClient->hActiveTransc);
    }
    pTranscClient->hActiveTransc = hNewTransc;
    return RV_OK;
}

/***************************************************************************
 * TranscClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          You can use this function for INVITE, BYE, RE-INVITE, and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transc-client is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid transc-client state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - transc-client which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSReSendRequest (IN  SipTranscClient			*pTranscClient,
												  IN  SipTransactionMethod	eMethod,
												  IN  RvChar*				strMethod)
{
    RvStatus rv;
    
	if(pTranscClient->eState != SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientDNSReSendRequest: transc-client 0x%p - no failed message. transc-client state - %s",
			pTranscClient, TranscClientStateIntoString(pTranscClient->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pTranscClient->hActiveTransc == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientDNSReSendRequest: transc-client 0x%p - Failed, Active transaction is NULL",
                pTranscClient));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = TranscClientSendRequest(pTranscClient, eMethod, strMethod);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientDNSReSendRequest: transc-client 0x%p - Failed to resend the request",
                pTranscClient));
        return rv;
    }

	TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_ACTIVATING, 
							SIP_TRANSC_CLIENT_REASON_CONTINUE_DNS,
							pTranscClient, 0);
    return RV_OK;
}

/***************************************************************************
* TranscClientDNSGetList
* ------------------------------------------------------------------------
* General: retrives DNS list object from the transaction object.
*
* Return Value: RV_OK or RV_ERROR_BADPARAM
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTranscClient - call leg which is responsible for the message.
* Output   phDnsList - DNS list handle
***************************************************************************/
RvStatus RVCALLCONV TranscClientDNSGetList(
                              IN  SipTranscClient             *pTranscClient,
                              OUT RvSipTransportDNSListHandle *phDnsList)
{
    RvStatus         rv;

	if(pTranscClient->eState != SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientDNSGetList: transc-client 0x%p - no failed message. transc-client state - %s",
			pTranscClient, TranscClientStateIntoString(pTranscClient->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pTranscClient->hActiveTransc == NULL)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientDNSGetList: transc-client 0x%p - Failed, Active transaction is NULL",
                pTranscClient));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RvSipTransactionDNSGetList(pTranscClient->hActiveTransc, phDnsList);
    if(rv != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientDNSGetList: transc-client 0x%p - failed to get DNS list of tansaction 0x%p",
            pTranscClient, pTranscClient->hActiveTransc));
        return rv;
    }
    return RV_OK;
}

#endif/*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
#endif/*#ifndef RV_SIP_PRIMITIVES */






















