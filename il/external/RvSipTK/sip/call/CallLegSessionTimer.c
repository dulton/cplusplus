

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
*                              <CallLegSessionTimer.c>
*
*  Handles SESSION TIMER process methods.
*    Author                         Date
*    ------                        ------
*   Michal Mashiach               June 2002
*********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegSession.h"
#include "CallLegSessionTimer.h"
#include "RvSipCallLeg.h"
#include "CallLegObject.h"
#include "CallLegCallbacks.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipCSeqHeader.h"
#include "_SipOtherHeader.h"
#include "_SipTransaction.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipMsg.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define SESSION_TIMER_MSECPERSEC         RvInt32Const(1000)
#define SESSION_TIMER_INTERVAL_DEVIDER   RvInt32Const(3)
#define SESSION_TIMER_INTERVAL_PRECEDER  RvInt32Const(32)
#define INTERVAL_TOO_SMALL_RESPONSE      422

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
static void RVCALLCONV UpdateSessionTimerTimeout(
                                  IN  RvInt32  sessionExpires,
                                  OUT RvInt32 *pTimeout);

static void RVCALLCONV UpdateSessionTimerAlertTimeout(
                                  IN    RvInt32  sessionExpires,
                                  INOUT RvInt32 *pTimeout);

static RvStatus RVCALLCONV GetSessionTimerPointer(
							  IN  CallLeg                         *pCallLeg,
							  IN  RvSipTranscHandle				   hTransc,
							  IN  RvBool						   bInviteTransc,
							  OUT CallLegNegotiationSessionTimer **ppSessionTimer);

static void RVCALLCONV ResetSessionTimerParams(
                              OUT CallLegSessionTimer  *pSessionTimer);

static RvStatus RVCALLCONV CallLegSessionTimerAddMinSEToMsg(
                                IN  CallLeg            *pCallLeg,
                                IN  RvInt32             minSE,
                                IN  RvSipMsgHandle      hMsg);

static RvStatus RVCALLCONV GetSessionExpiresValues(
                       IN  CallLeg                          *pCallLeg,
                       IN  RvSipMsgHandle                    hMsg,
                       OUT RvInt32                          *pSessionExpires,
                       OUT RvSipSessionExpiresRefresherType *peRefresherType);

static RvStatus RVCALLCONV GetMinSEValue(
                                      IN  CallLeg       *pCallLeg,
                                      IN  RvSipMsgHandle hMsg,
                                      OUT RvInt32       *pMinSE);

static RvBool RVCALLCONV IsSessionTimerSupportedByRemote(
                                        IN  RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV HandleAutoRefreshAlert(CallLeg* pCallLeg);

static RvStatus RVCALLCONV HandleManualRefreshAlert(CallLeg* pCallLeg);

static RvStatus RVCALLCONV SetAlertTimeoutAndTimer(CallLeg* pCallLeg);

static RvStatus RVCALLCONV IsInviteOrUpdateTransc(
									IN  CallLeg           *pCallLeg,
									IN  RvSipTranscHandle  hTransc,
									IN  RvSipMsgHandle     hMsg,
									OUT RvBool		      *pbInvite);

static RvStatus RVCALLCONV IsInviteOrUpdateFromMsg(
								   IN  CallLeg           *pCallLeg,
								   IN  RvSipMsgHandle     hMsg,
								   OUT RvBool		    *pbInvite);

static RvStatus RVCALLCONV IsInviteOrUpdateFromTransc(
								  IN  CallLeg           *pCallLeg,
								  IN  RvSipTranscHandle  hTransc,
								  OUT RvBool		    *pbInvite);

static RvStatus RVCALLCONV GetResponseCode(
										   IN  CallLeg           *pCallLeg,
										   IN  RvSipTranscHandle  hTransc,
										   IN  RvSipMsgHandle     hMsg,
										   OUT RvUint16		  *pRespCode); 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSessionTimerRefresh
 * ------------------------------------------------------------------------
 * General: Causes a re-INVITE to be sent in order to refresh the session time.
 *          Can be called only in the connected state when there is no other
 *          pending re-Invite transaction.
 *          The remote-party's response to the re-INVITE will be given
 *          in the EvModifyResultRecvd callback.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to modify.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerRefresh (IN  CallLeg      * pCallLeg,
                                                IN  CallLegInvite* pInvite)
{
    if (pCallLeg->pNegotiationSessionTimer == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    pCallLeg->pNegotiationSessionTimer->bInRefresh = RV_TRUE;

    if(NULL == pInvite)
    {
        return CallLegSeesionCreateAndModify(pCallLeg);
    }
    else
    {
        return CallLegSessionModify(pCallLeg, pInvite);
    }
}

/***************************************************************************
* CallLegSessionTimerGeneralRefresh
* ------------------------------------------------------------------------
* General: Creates a transaction related to the call-leg and sends a
*          Request message with the given method in order to refresh the call.
*          The request will have the To, From and Call-ID of the call-leg and
*          will be sent with a correct CSeq step. It will be record routed if needed.
* Return Value: RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send.
*               RV_OK - Message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - Pointer to the call leg the user wishes to modify.
*          strMethod - A String with the request method.
*          sessionExpires - session time that will attach to this call.
*          minSE - minimum session expires time of this call
*          eRefresher - the refresher preference for this call
* Output:  hTransc - The handle to the newlly created transaction
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerGeneralRefresh (
     IN  CallLeg                                     *pCallLeg,
     IN  RvChar                                      *strMethod,
     IN  RvInt32                                      sessionExpires,
     IN  RvInt32                                      minSE,
     IN  RvSipCallLegSessionTimerRefresherPreference  eRefresher,
     OUT RvSipTranscHandle                           *hTransc)
{
    RvStatus                           rv= RV_OK;
    rv = CallLegSessionRequestRefresh(
        pCallLeg,strMethod,sessionExpires,minSE,eRefresher,hTransc);
    return rv;

}
/***************************************************************************
* CallLegSessionTimerAddSessionExpiresToMsg
* ------------------------------------------------------------------------
* General: Adds a Session-Expires header to outgoing message. The header is taken
*          from the call-leg object.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    sessionExpires - the session expires that will be set in the msg.
*           eRefresherType - the refresher type that will be set in the msg.
*           hMsg           - Handle to the message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerAddSessionExpiresToMsg(
        IN  RvInt32                          sessionExpires,
        IN  RvSipSessionExpiresRefresherType eRefresherType,
        IN  RvSipMsgHandle                   hMsg)
{
    RvStatus                        rv                    = RV_OK;
    RvSipSessionExpiresHeaderHandle  hSessionExpiresHeader = NULL;
    if (sessionExpires < 0)
    {
        return RV_ERROR_BADPARAM;
    }
    if (sessionExpires == 0)
    {
        return RV_OK;
    }

    if(sessionExpires != UNDEFINED)
    {
        rv = RvSipSessionExpiresHeaderConstructInMsg(hMsg, RV_FALSE,
            &hSessionExpiresHeader);
        if (rv != RV_OK)
        {
            return rv;
        }
        rv = RvSipSessionExpiresHeaderSetDeltaSeconds(
            hSessionExpiresHeader,
            sessionExpires);
        if (rv != RV_OK)
        {
            return rv;
        }
        rv = RvSipSessionExpiresHeaderSetRefresherType(
            hSessionExpiresHeader,
            eRefresherType);
    }


    return rv;
}

/***************************************************************************
* CallLegSessionTimerHandleSendMsg
* ------------------------------------------------------------------------
* General:Handle session timer for INVITE and UPDATE out-going calls.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pCallLeg - Pointer to the call-leg.
*           hTransc  - The transaction that received this message.
*           hMsg     - Handle to the message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleSendMsg(
                           IN  CallLeg                *pCallLeg,
						   IN  RvSipTranscHandle	   hTransc,
                           IN  RvSipMsgHandle          hMsg)
{
    RvStatus                        rv                    = RV_OK;
    RvUint16                        responseCode          = RvSipMsgGetStatusCode(hMsg);
    RvSipMsgType                    eMsgType              = RvSipMsgGetMsgType(hMsg);
    RvSipCallLegState               eState                = RVSIP_CALL_LEG_STATE_UNDEFINED;
    CallLegNegotiationSessionTimer *pSessionTimer         = NULL;
	RvBool							bInviteTransc; 
	
	rv = IsInviteOrUpdateTransc(pCallLeg,hTransc,hMsg,&bInviteTransc);
	if (rv != RV_OK) 
	{
        return RV_OK;
	}

    rv = RvSipCallLegGetCurrentState((RvSipCallLegHandle)pCallLeg,&eState);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleSendMsg - Call-Leg 0x%p - failed in RvSipCallLegGetCurrentState"
            ,pCallLeg));
        return rv;
    }

    if (pCallLeg->pSessionTimer == NULL || pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleSendMsg - Call-Leg 0x%p - Session Timer parameters doesn't appear in the call leg"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    rv = GetSessionTimerPointer(pCallLeg,hTransc,bInviteTransc,&pSessionTimer);
	/* Only INVITE requests/responses are taken care (no ACKs,which MUSTN'T include SE params) */
    if (bInviteTransc == RV_TRUE)
    {
        if (eState == RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            if (pSessionTimer->bInRefresh == RV_FALSE)
            {
                return RV_OK;

            }
        }
        else
        {
            /* INVITE refresher type */
            pSessionTimer->bInRefresh = RV_TRUE;
        }
    }
    else 
    {
        if(pSessionTimer == NULL || pSessionTimer->bInRefresh == RV_FALSE)
        {
            return RV_OK;
        }
    }
    
    /*if the message is Request message transaction*/
    if(eMsgType == RVSIP_MSG_REQUEST)
    {
        /*Add the session timer params only in INVITE transaction*/
        if (eState == RVSIP_CALL_LEG_STATE_CONNECTED)
		{
			/* re-INVITE or UPDATE refresher type */
			if (pSessionTimer->eRefresherPreference ==
				RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)
			{
				pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
			}
			else if (pSessionTimer->eRefresherPreference ==
				RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL)
			{
				pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
			}
			else
			{
				pSessionTimer->eRefresherType =
					(pCallLeg->pSessionTimer->eCurrentRefresher==RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)?
RVSIP_SESSION_EXPIRES_REFRESHER_UAS:RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
			}
		}
		else
		{
			/* INVITE refresher type */
			if (pSessionTimer->eRefresherPreference
				== RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL)
			{
				pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
			}
			else if (pSessionTimer->eRefresherPreference
				== RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)
			{
				pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
			}
		}
		rv = CallLegSessionTimerAddParamsToMsg(pCallLeg,
			pSessionTimer->sessionExpires,
			pSessionTimer->minSE,
			pSessionTimer->eRefresherType,
			hMsg);
		if(rv != RV_OK)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSessionTimerHandleSendMsg - Call-Leg 0x%p - Failed to add session timer params to the msg"
				,pCallLeg));
			return rv;
		}

	}

    /*if the message is 2xx response */
    if(eMsgType == RVSIP_MSG_RESPONSE &&
       responseCode >= 200 && responseCode<300 )
    {
        if (pSessionTimer->eRefresherPreference == RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL &&
            pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
        {
            pCallLeg->pSessionTimer->eNegotiationReason =
                RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT;
        }
        else if (pSessionTimer->eRefresherPreference == RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE &&
                 pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAS)
        {
            pCallLeg->pSessionTimer->eNegotiationReason =
                RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT;
        }
        else
        {
            pCallLeg->pSessionTimer->eNegotiationReason =
                RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_UNDEFINED;
        }

        if (RVSIP_SESSION_EXPIRES_REFRESHER_NONE == pSessionTimer->eRefresherType)
        {
            if (pSessionTimer->eRefresherPreference ==
                RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)
            {
                pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
            }
            else
            {
                pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
            }
        }

        if (pSessionTimer->sessionExpires > 0 &&
            pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
        {
            rv = SipMsgAddNewOtherHeaderToMsg(hMsg,"Require","timer",NULL);
            if (RV_OK != rv)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleSendMsg - Call Leg 0x%p , Failed to add Require header to message 0x%p"
                    ,pCallLeg,hMsg));
                return rv;
            }
         }

        rv = CallLegSessionTimerAddParamsToMsg(pCallLeg,
                                               pSessionTimer->sessionExpires,
                                               pSessionTimer->minSE,
                                               pSessionTimer->eRefresherType,
                                               hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleSendMsg - Call-Leg 0x%p - Failed to add session timer params to the msg"
                ,pCallLeg));
            return rv;
        }

    }
    /*if the message is 422 response */
    if(eMsgType     == RVSIP_MSG_RESPONSE &&
       responseCode == INTERVAL_TOO_SMALL_RESPONSE)
    {
        pSessionTimer->minSE = pCallLeg->pSessionTimer->minSE;
        /* Add the MinSE header to the msg*/
        rv = CallLegSessionTimerAddMinSEToMsg(
            pCallLeg,pSessionTimer->minSE,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleSendMsg - Call-Leg 0x%p - Failed to add Min-SE header to the msg"
                ,pCallLeg));
            return rv;
        }

    }


    return rv;

}

/***************************************************************************
* CallLegSessionTimerHandleRequestRcvd
* ------------------------------------------------------------------------
* General:Handle session timer INVITE request received.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg.
*            hMsg     - Handle to the message.
*           hTransc  - The transaction that received this message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleRequestRcvd(
                    IN  CallLeg                *pCallLeg,
                    IN  RvSipTranscHandle       hTransc,
					IN  RvSipMsgHandle			hMsg,
                    OUT RvBool                 *pbIsRejected)

{
    RvStatus                         rv                    = RV_OK;
    RvInt32                          msgSessionExpires     = UNDEFINED;
    RvInt32                          msgMinSE              = UNDEFINED;
    RvSipSessionExpiresRefresherType eMsgRefresherType     = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
    RvBool                           bDoReject             = RV_FALSE;
    CallLegNegotiationSessionTimer  *pSessionTimer         = NULL;
	RvBool							 bInviteTransc;
    RvBool                           bTimerSupported;

	rv = IsInviteOrUpdateTransc(pCallLeg,hTransc,hMsg,&bInviteTransc);
	if (rv != RV_OK) 
	{	
        return RV_OK;
	}

    if (pCallLeg->pSessionTimer == NULL ||pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleRequestRcvd - Call-Leg 0x%p - Session Timer parameters doesn't appear in the call leg"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
    rv = GetSessionTimerPointer(pCallLeg,hTransc,bInviteTransc,&pSessionTimer);
    if (pSessionTimer == NULL)
    {
        if (bInviteTransc == RV_FALSE) /* Update Transaction */
        {
            pSessionTimer =
                (CallLegNegotiationSessionTimer *)SipTransactionAllocateSessionTimerMemory(
                hTransc,
                sizeof(CallLegNegotiationSessionTimer));

            /* Initialize the structure in the default values from the call mgr*/
            CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr, RV_TRUE, pSessionTimer);
            pSessionTimer->bInRefresh = RV_FALSE;

        }
        else
        {
            return RV_OK;
        }
    }
    
    rv = GetSessionExpiresValues(pCallLeg,hMsg,&msgSessionExpires,&eMsgRefresherType);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleRequestRcvd - Call-leg 0x%p - Failed in GetSessionExpiresValues for transc 0x%p (rv=%d)",
            pCallLeg,hTransc,rv));
        return rv;
    }

    /* Bug Fix */
    /* According to chapter 9 in RFC 4028,the Min-SE should be referred even */
    /* when no Session-Expires header appears within the incoming message -  */
    /* in case that the remote side supports the mechanism */
    rv = GetMinSEValue(pCallLeg,hMsg,&msgMinSE);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleRequestRcvd - Call-leg 0x%p - Failed in GetMinSEValue for transc 0x%p (rv=%d)",
            pCallLeg,hTransc,rv));
        return rv;
    }

    /* Copy the received MinSE to the call ONLY if the remote Min-SE is */
    /* bigger than the local MinSE, since we don't want to allow the    */
    /* remote-side to reduce the local Min-SE */
    if (msgMinSE > pSessionTimer->minSE) {
        pSessionTimer->minSE = msgMinSE;
    }
    
    if (msgSessionExpires > 0)
    {
        pSessionTimer->bInRefresh = RV_TRUE;
    }
    else
    {
		/* Check whether the other side supports session timer */
		bTimerSupported = IsSessionTimerSupportedByRemote(hMsg);
        /* The request doesn't contain sessionExpires header				*/
        /* If: 1. There is no Session-Expires header in the re-INVITE or in an UPDATE  */
		/*  AND																*/
		/* 2. There is a 'timer' phrase within the remote Supported header  */
		/* We will not treat this message as a refresh						*/
        if (bTimerSupported == RV_TRUE &&
			(bInviteTransc == RV_FALSE ||
            (bInviteTransc == RV_TRUE && pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)))
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleRequestRcvd - Call-Leg 0x%p - transc 0x%p does not contain Session-Expires header - session timer is disabled"
                ,pCallLeg,hTransc));
            pSessionTimer->bInRefresh = RV_FALSE;
            return RV_OK;
        }
        else /* if (bInviteTransc == RV_TRUE). Fix: The bInRefresh must become RV_TRUE anyway */
        {
            pSessionTimer->bInRefresh = RV_TRUE;
        }

        if (pSessionTimer->sessionExpires > 0 )
        {
            if (pSessionTimer->eRefresherPreference ==
                    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)
            {
                if (bTimerSupported == RV_TRUE)
                {
                    pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
                }
                else
                {
                    pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
                }
            }
            else
            {
                pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
            }
        }
        /* The message doesn't include Session-Expires */
        /* thus no validity checks should take place   */
        return RV_OK;
    }

    /* If the msg SE is smaller than the minSE of the call */
    if(msgSessionExpires < pCallLeg->pSessionTimer->minSE)
    {
        /* reject the msg*/
        bDoReject = RV_TRUE;
    }
    /*If the msg SE value is smaller than the value of SE in the call*/
    else if (pSessionTimer->sessionExpires > 0 &&
        msgSessionExpires < pSessionTimer->sessionExpires )
    {
        if(pSessionTimer->minSE > 0 &&
           msgSessionExpires >= pSessionTimer->minSE)
        {
            /* Update The session-Expires value in the call*/
            pSessionTimer->sessionExpires = msgSessionExpires;
        }
        else if(pSessionTimer->minSE > 0 &&
                msgSessionExpires < pSessionTimer->minSE)
        {
            /* reject the msg*/
            bDoReject = RV_TRUE;
        }
        else if (pSessionTimer->minSE <= 0)
        {
            /* Update The session-Expires value in the call*/
            pSessionTimer->sessionExpires = msgSessionExpires;
        }
    }
	/* Bug Fix - from 3.1 */
    /* According to RFC4028 (chapter 9): "The UAS response MAY reduce its value  
       but MUST NOT set it to a duration lower than the value in the Min-SE header 
       field in the request, if it is present; otherwise the UAS MAY reduce its value 
       but MUST NOT set it to a duration lower than 90 seconds." If the MinSE is 
       missing within the message, it will be considered as 90 seconds. */
    else if (msgSessionExpires > pSessionTimer->sessionExpires &&
             pSessionTimer->sessionExpires < msgMinSE)
    { 
        pSessionTimer->sessionExpires = msgMinSE;
    }

    /* Check the refresher type of the msg */
    switch(eMsgRefresherType)
    {
        case RVSIP_SESSION_EXPIRES_REFRESHER_NONE:
           if (pSessionTimer->eRefresherPreference
                  ==RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE)
           {
               pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
           }
           else if(pSessionTimer->eRefresherPreference
                  ==RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL)
           {
                pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
           }
           else
           {
               pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
           }
           break;
       case RVSIP_SESSION_EXPIRES_REFRESHER_UAC:
           /* if the INVITE msg contain refresher parameter it must copy
              to the response*/
          pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
          break;
       case RVSIP_SESSION_EXPIRES_REFRESHER_UAS:
           /* if the INVITE msg contain refresher parameter it must copy
              to the response*/
          pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
          break;
    }
     /* If the INVITE msg the Session-Expires value is larger than the value of minSE
        or the minSE of the msg ismaller than the minSE of the call*/
    if (msgSessionExpires != UNDEFINED && bDoReject==RV_TRUE)
    {
        if (bInviteTransc == RV_TRUE &&
            pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            rv = CallLegSessionReject(pCallLeg,INTERVAL_TOO_SMALL_RESPONSE);
        }
        else
        {
            /* Reject the call with 422 response code */
            rv = RvSipTransactionRespond(hTransc,INTERVAL_TOO_SMALL_RESPONSE,NULL);

        }
        if(rv != RV_OK)
        {
            /* Transaction failed to send respond */
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleRequestRcvd - Call-Leg 0x%p - Failed to send %d response",
                pCallLeg,INTERVAL_TOO_SMALL_RESPONSE));
            CallLegDisconnectWithNoBye(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        }

        *pbIsRejected  = RV_TRUE;
        CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr, RV_TRUE, pSessionTimer);
    }

    return rv;
}


/***************************************************************************
* CallLegSessionTimerHandleFinalResponseSend
* ------------------------------------------------------------------------
* General:handle the session timer in final response send
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg.
* hTransc  - handle to the transaction.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleFinalResponseSend(
                                       IN  CallLeg           *pCallLeg,
									   IN  RvSipTranscHandle  hTransc,
                                       IN  RvSipMsgHandle     hMsg)
{
    RvUint16                        responseCode;
	CallLegNegotiationSessionTimer *pSessionTimer  = NULL;
	RvBool							bInviteTransc;
	RvStatus						rv; 

	rv = IsInviteOrUpdateTransc(pCallLeg,hTransc,hMsg,&bInviteTransc);
	if (rv != RV_OK) 
	{
        return RV_OK;
	}

	/* Retrieve the response code */
	rv = GetResponseCode(pCallLeg,hTransc,hMsg,&responseCode);
	if (rv != RV_OK) 
	{
		return rv;
	}

    if (pCallLeg->pSessionTimer == NULL ||pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleFinalResponseSend - Call-Leg 0x%p - Session Timer parameters doesn't appear in the call leg"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
    GetSessionTimerPointer(pCallLeg,hTransc,bInviteTransc,&pSessionTimer);
    if (pSessionTimer == NULL || pSessionTimer->bInRefresh== RV_FALSE)
    {
        return RV_OK;
    }

	
    /*if the message is 2xx response */
    if(responseCode >= 200 && responseCode<300)
    {
        /* Copy the negotiation session timer params to the call session timer params*/
        pCallLeg->pSessionTimer->sessionExpires   = pSessionTimer->sessionExpires;
        pCallLeg->pSessionTimer->eRefresherType   = pSessionTimer->eRefresherType;
        if (RVSIP_SESSION_EXPIRES_REFRESHER_UAS == pSessionTimer->eRefresherType)
        {
            pCallLeg->pSessionTimer->eCurrentRefresher =
                RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
        }
        else if (RVSIP_SESSION_EXPIRES_REFRESHER_UAC == pSessionTimer->eRefresherType)
        {
            pCallLeg->pSessionTimer->eCurrentRefresher =
                RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
        }
        pCallLeg->pSessionTimer->defaultAlertTime = pCallLeg->pSessionTimer->sessionExpires/2;

        if (pCallLeg->pSessionTimer->alertTime != UNDEFINED &&
            pCallLeg->pSessionTimer->alertTime >= pCallLeg->pSessionTimer->sessionExpires)
        {
            pCallLeg->pSessionTimer->alertTime = pCallLeg->pSessionTimer->defaultAlertTime;
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleFinalResponseSend - Call-Leg 0x%p - alert Time was larger than the session expires val (was changed)"
                ,pCallLeg));
        }

    }

    /* clean the negotiation params except the minSE which */
    /* will be reset in CallLegSessionTimerHandleSendMsg   */
    CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr,RV_FALSE,pSessionTimer);

    return RV_OK;
}

/***************************************************************************
* CallLegSessionTimerHandleFinalResponseRcvd
* ------------------------------------------------------------------------
* General:handle the session timer in INVITE final response received
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pCallLeg - Pointer to the call-leg.
            hMsg     - handle to the received response
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleFinalResponseRcvd(
                                          IN  CallLeg           *pCallLeg,
										  IN  RvSipTranscHandle  hTransc,
                                          IN  RvSipMsgHandle	 hMsg)
{
    RvStatus                         rv                    = RV_OK;
    RvSipSessionExpiresHeaderHandle  hSessionExpiresHeader = NULL;
    RvSipMinSEHeaderHandle           hMinSEHeader          = NULL;
    RvInt32                          msgMinSE              = UNDEFINED;
    RvInt32                          msgSessionExpires     = UNDEFINED;
    RvSipSessionExpiresRefresherType eMsgRefresherType     = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
    RvSipHeaderListElemHandle        hListElem             = NULL;
    CallLegNegotiationSessionTimer  *pSessionTimer         = NULL;
    RvSipCallLegState                eState                = RVSIP_CALL_LEG_STATE_UNDEFINED;
	RvBool							 bInviteTransc;
	RvUint16                         responseCode;

    
	rv = IsInviteOrUpdateTransc(pCallLeg,hTransc,hMsg,&bInviteTransc);
	if (rv != RV_OK) 
	{
		return RV_OK;
	}
	
	rv = GetResponseCode(pCallLeg,hTransc,hMsg,&responseCode); 
	if (rv != RV_OK) 
	{
		return rv;
	}
	
    if (pCallLeg->pSessionTimer == NULL || pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Session Timer parameters doesn't appear in the call leg"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
    rv = RvSipCallLegGetCurrentState((RvSipCallLegHandle)pCallLeg,&eState);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - fail in RvSipCallLegGetCurrentState"
            ,pCallLeg));
        return rv;
    }

    rv = GetSessionTimerPointer(pCallLeg,hTransc,bInviteTransc,&pSessionTimer);
    if (rv != RV_OK || pSessionTimer == NULL)
    {
        /* There might be a response with no Session-Expires. This indicates that */
        /* the remote side wishes to disable the Session-Timer mechanism          */
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Current Session Timer parameters doesn't appear in the call leg",
            pCallLeg));
        return RV_OK;
    }

    /*if the message is 422 response */
    if(responseCode == INTERVAL_TOO_SMALL_RESPONSE)
    {
        /* manual mode notify the application */
        rv = CallLegCallbackSessionTimerNotificationEv(
            (RvSipCallLegHandle)pCallLeg,
            RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_422_RECEIVED);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Received %d and failed in CallLegCallbackSessionTimerNotificationEv",
                pCallLeg,INTERVAL_TOO_SMALL_RESPONSE));
            return rv;
        }
        /* Indicate that in the next request a Min-SE header should be added */
        pCallLeg->pSessionTimer->bAddOnceMinSE = RV_TRUE;
    }

    /*if the message is 2xx response */
    if(responseCode >= 200 && responseCode<300)
    {
        /* get the Session-Expires value from the msg*/
        hSessionExpiresHeader = (RvSipSessionExpiresHeaderHandle)RvSipMsgGetHeaderByType(
            hMsg, RVSIP_HEADERTYPE_SESSION_EXPIRES,
            RVSIP_FIRST_HEADER, &hListElem);

        if (hSessionExpiresHeader== NULL)
        {
            /* In case of INVITE and in re-INVITE or UPDATE that was intent to be refresh*/
            if (eState != RVSIP_CALL_LEG_STATE_CONNECTED ||
                pSessionTimer->bInRefresh == RV_TRUE)
            {
                /* load the negotiation reason*/
                pCallLeg->pSessionTimer->eNegotiationReason =
                    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED;
                /* copy to values from the msg to the call*/
                pCallLeg->pSessionTimer->sessionExpires = pSessionTimer->sessionExpires;
                pCallLeg->pSessionTimer->eRefresherType   = pSessionTimer->eRefresherType;
                if (RVSIP_SESSION_EXPIRES_REFRESHER_UAS == pSessionTimer->eRefresherType)
                {
                    pCallLeg->pSessionTimer->eCurrentRefresher =
                        RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
                }
                else if (RVSIP_SESSION_EXPIRES_REFRESHER_UAC == pSessionTimer->eRefresherType)
                {
                    pCallLeg->pSessionTimer->eCurrentRefresher =
                        RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
                }
                pCallLeg->pSessionTimer->defaultAlertTime = pSessionTimer->sessionExpires/2;

                if (pCallLeg->pSessionTimer->alertTime != UNDEFINED &&
                    pCallLeg->pSessionTimer->alertTime >= pSessionTimer->sessionExpires)
                {
                    pCallLeg->pSessionTimer->alertTime = pCallLeg->pSessionTimer->defaultAlertTime;
                    RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - alert Time was larger than the session expires val (was changed)"
                        ,pCallLeg));
                }
            }
        }
        else
        {
            pCallLeg->pSessionTimer->eNegotiationReason =
                RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_UNDEFINED;
        }
        if (NULL != hSessionExpiresHeader)
        {
            pSessionTimer->bInRefresh= RV_TRUE;
            rv = RvSipSessionExpiresHeaderGetDeltaSeconds(
                hSessionExpiresHeader,
                &msgSessionExpires);
            if((rv != RV_OK) || (msgSessionExpires==UNDEFINED))
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Failed in RvSipSessionExpiresHeaderGetDeltaSeconds"
                    ,pCallLeg));
                return rv;
            }
            rv = RvSipSessionExpiresHeaderGetRefresherType(
                hSessionExpiresHeader,
                &eMsgRefresherType);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Failed in RvSipSessionExpiresHeaderGetRefresherType"
                    ,pCallLeg));
                return rv;
            }
            hListElem = NULL;
            /* get the MinSE value from the msg*/
            hMinSEHeader = (RvSipMinSEHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_MINSE,
                RVSIP_FIRST_HEADER, &hListElem);
            
            if (NULL != hMinSEHeader)
            {
                rv = RvSipMinSEHeaderGetDeltaSeconds(
                    hMinSEHeader,
                    &msgMinSE);
                if(rv != RV_OK||msgMinSE == UNDEFINED)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - Failed in RvSipMinSEHeaderGetDeltaSeconds"
                        ,pCallLeg));
                    return rv;
                }
            }
            /* copy to values from the msg to the call*/
            if (msgSessionExpires >= pCallLeg->pSessionTimer->minSE)
            {
                pCallLeg->pSessionTimer->sessionExpires = msgSessionExpires;
            }
            else
            {
                pCallLeg->pSessionTimer->sessionExpires = pSessionTimer->sessionExpires;
                RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleFinalResponseRcvd - Call-leg 0x%p - received SE (%d) smaller than local MinSE (%d)- ignoring it",
                    pCallLeg,msgSessionExpires,pCallLeg->pSessionTimer->minSE));
            }
            /* The Min-SE header should be the internal Min-SE, */
            /* otherwise the remote side will be able to        */
            /* reduce the Session-Expires below the original    */
            /* (configured) Min-SE */
            pCallLeg->pSessionTimer->eRefresherType   = eMsgRefresherType;
            if (RVSIP_SESSION_EXPIRES_REFRESHER_UAS == eMsgRefresherType)
            {
                pCallLeg->pSessionTimer->eCurrentRefresher =
                    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
            }
            else if (RVSIP_SESSION_EXPIRES_REFRESHER_UAC == eMsgRefresherType)
            {
                pCallLeg->pSessionTimer->eCurrentRefresher =
                    RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
            }
            
            pCallLeg->pSessionTimer->defaultAlertTime =
                pCallLeg->pSessionTimer->sessionExpires/2;
            
            if (pCallLeg->pSessionTimer->alertTime != UNDEFINED &&
                pCallLeg->pSessionTimer->alertTime >= pCallLeg->pSessionTimer->sessionExpires)
            {
                pCallLeg->pSessionTimer->alertTime = pCallLeg->pSessionTimer->defaultAlertTime;
                RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleFinalResponseRcvd - Call-Leg 0x%p - alert Time was larger than the session expires val (was changed)"
                    ,pCallLeg));
            }
            
            /* If the refresher prefrence doesn't match to the refresher type of the call*/
            if (pSessionTimer->eRefresherPreference == RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL &&
                pCallLeg->pSessionTimer->eRefresherType != RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
            {
                pCallLeg->pSessionTimer->eNegotiationReason =
                    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT;
            }
            else if (pSessionTimer->eRefresherPreference == RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE &&
                pCallLeg->pSessionTimer->eRefresherType != RVSIP_SESSION_EXPIRES_REFRESHER_UAS)
            {
                pCallLeg->pSessionTimer->eNegotiationReason =
                    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT;
            }
            else
            {
                pCallLeg->pSessionTimer->eNegotiationReason =
                    RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_UNDEFINED;
            }
        }
    }

	/* Only in case of NON-3xx,NON-401 and NON-407 responses the SessionTimer params */
	/* are reset. The reason for it is that the 3xx, 401 and 407 responses are final */
	/* but don't cause to the immediate termination of the Call-Leg, thus the session*/
	/* timer data shouldn't be overwrriten yet */
	if ((responseCode < 300 || responseCode >= 400) &&
		responseCode != 401 &&
		responseCode != 407)
	{
		/* Clean the session timer params of the modify call*/
		CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr, RV_FALSE, pSessionTimer);
	}

    return rv;
}

/***************************************************************************
* CallLegSessionTimerHandleTimers
* ------------------------------------------------------------------------
* General:handle the session timers
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg..
*           bMsgReceived  - is the msg received or sent
*           hTransc  - The transaction (UPDATE/INVITE).
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleTimers(
                                           IN  CallLeg           *pCallLeg,
                                           IN  RvSipTranscHandle  hTransc,
										   IN  RvSipMsgHandle	  hMsg,
                                           IN  RvBool             bMsgRecieved)
{
    RvStatus                        rv                          = RV_OK;
    RvInt32                         timeOut                     = 0;
    RvBool                          bHandleSessionTimer         = RV_TRUE;
    RvTimerFuncEx                   timerFunc                   = NULL;
    CallLegNegotiationSessionTimer *pSessionTimer               = NULL;
	RvBool							bInviteTransc;

	rv = IsInviteOrUpdateTransc(pCallLeg,hTransc,hMsg,&bInviteTransc); 
	if (rv != RV_OK) 
	{
		return RV_OK;
	}
    
    /* First, release the current timer, before setting the next one 
       (if the session-timer is still enabled) */ 
    CallLegSessionTimerReleaseTimer(pCallLeg);

    rv = GetSessionTimerPointer(pCallLeg,hTransc,bInviteTransc,&pSessionTimer);
    if (pSessionTimer == NULL || pSessionTimer->bInRefresh == RV_FALSE)
    {
        return RV_OK;
    }
    pSessionTimer->bInRefresh = RV_FALSE;

    /* In case of response with no Session-Expires header to */
    /* a request that included this header. */
    if (pCallLeg->pSessionTimer->sessionExpires <= 0)
    {
        return RV_OK;
    }

    /* If the response doesn't contain session expires -
    the remote party doesn't support*/
    if (pCallLeg->pSessionTimer->eNegotiationReason ==
         RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED)
    {

        /* Ask the application whether to execute session timer or not*/
        rv = CallLegCallbackSessionTimerNegotiationFaultEv(
                        (RvSipCallLegHandle)pCallLeg,
                        RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED,
                        &bHandleSessionTimer);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerHandleTimers - Call Leg 0x%p , Failed in CallLegCallbackSessionTimerNegotiationFaultEv \
                                reason - dest not supported"
                ,pCallLeg));
            return rv;
        }
    }
    /* If the refresher preference rejected*/
     else if (pCallLeg->pSessionTimer->eNegotiationReason ==
              RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT)
     {
         /* Ask the application whether to execute session timer or not*/
         rv = CallLegCallbackSessionTimerNegotiationFaultEv(
                             (RvSipCallLegHandle)pCallLeg,
                             RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT,
                             &bHandleSessionTimer);
         if (RV_OK != rv)
         {
             RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSessionTimerHandleTimers - Call Leg 0x%p , Failed in CallLegCallbackSessionTimerNegotiationFaultEv \
                                 reason = refresher preference rejected"
                 ,pCallLeg));
             return rv;
         }
     }


    /* If the application want to operate session timer*/
     if (pCallLeg->pSessionTimer->eNegotiationReason ==
         RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_DEST_NOT_SUPPORTED)

     {
        if(bHandleSessionTimer == RV_TRUE && bMsgRecieved == RV_FALSE)
        {
            /* The local party will be the refresher*/
            pCallLeg->pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
        }
        else if(bHandleSessionTimer == RV_TRUE && bMsgRecieved == RV_TRUE)
        {
            /* The local party will be the refresher*/
            pCallLeg->pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;

        }
        /* If the application doesn't want to operate session timer */
        else if (bHandleSessionTimer == RV_FALSE)
        {
            /*clean the session timer parameters in the call*/
            ResetSessionTimerParams(pCallLeg->pSessionTimer);
        }
     }
     if (pCallLeg->pSessionTimer->eNegotiationReason ==
         RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_REFRESHER_PREFERENCE_REJECT )
     {
         if (bHandleSessionTimer == RV_TRUE  &&
             pCallLeg->pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_NONE)
         {
             if(bMsgRecieved == RV_FALSE)
             {
                 /* The local party will be the refresher*/
                 pCallLeg->pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAS;
             }
             else if(bMsgRecieved == RV_TRUE)
             {
                 /* The local party will be the refresher*/
                 pCallLeg->pSessionTimer->eRefresherType = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
                 
             }
         }
         
         /* If the application doesn't want to operate session timer */
         else if (bHandleSessionTimer == RV_FALSE)
         {
             /*clean the session timer parameters in the call*/
             ResetSessionTimerParams(pCallLeg->pSessionTimer);
         }
     }
     if (pCallLeg->pSessionTimer->defaultAlertTime <= 0)
     {
         if (pCallLeg->pSessionTimer->sessionExpires != UNDEFINED)
         {
             pCallLeg->pSessionTimer->defaultAlertTime = pCallLeg->pSessionTimer->sessionExpires/2;
         }
     }

    /* Final Response is been sent */
    if(bMsgRecieved        == RV_FALSE               &&
       bHandleSessionTimer == RV_TRUE                &&
       pCallLeg->pSessionTimer->defaultAlertTime > 0 &&
       pCallLeg->pSessionTimer->sessionExpires   > 0)
    {
        /* If the SE is defined, in case of re-INVITE release all other timers*/
        /* CallLegSessionTimerReleaseTimer(pCallLeg); - it is done in the beginning of function */ 

        if (pCallLeg->pSessionTimer->eRefresherType != RVSIP_SESSION_EXPIRES_REFRESHER_NONE)
        {
            /*If the refresher for this call is the local party */
            if (pCallLeg->pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAS)
            {
                /* Set the timer to the alert time value*/
                if (pCallLeg->pSessionTimer->alertTime > 0 &&
                    pCallLeg->pSessionTimer->alertTime < pCallLeg->pSessionTimer->sessionExpires)
                {
                    timeOut = (pCallLeg->pSessionTimer->alertTime)*SESSION_TIMER_MSECPERSEC;
                }
                else
                {
                    timeOut = (pCallLeg->pSessionTimer->defaultAlertTime)*SESSION_TIMER_MSECPERSEC;
                }
                timerFunc = CallLegSessionTimerRefreshAlertTimeout;
            }
            else if (pCallLeg->pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
            {
                UpdateSessionTimerTimeout(pCallLeg->pSessionTimer->sessionExpires, &timeOut);
                if (timeOut <= 0)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegSessionTimerHandleTimers - Call-Leg 0x%p - Failed to set the session expires timer (the value is <= 0)"
                        ,pCallLeg));
                    return rv;
                }

                timerFunc = CallLegSessionTimerSessionExpiresTimeout;

            }
           RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleTimers - Call-Leg 0x%p - Setting timer to %d"
                     ,pCallLeg,timeOut));

            /* Set the timer */
            rv = SipCommonCoreRvTimerStartEx(
                        &pCallLeg->pSessionTimer->hSessionTimerTimer,
                        pCallLeg->pMgr->pSelect,
                        timeOut,
                        timerFunc,
                        pCallLeg);
            if (rv != RV_OK)
            {
                RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "CallLegSessionTimerHandleTimers - Call-Leg  0x%p: No timer is available",
                    pCallLeg));
                return RV_ERROR_OUTOFRESOURCES;
            }
            else
            {
                RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "CallLegSessionTimerHandleTimers - Call-Leg  0x%p: timer was set to %u milliseconds",
                    pCallLeg, timeOut));
            }

        }
    }
    /* Final Response is been receievd */
    if(bMsgRecieved        == RV_TRUE                &&
       bHandleSessionTimer == RV_TRUE                &&
       pCallLeg->pSessionTimer->defaultAlertTime > 0 &&
       pCallLeg->pSessionTimer->sessionExpires   > 0)
    {
        /* If the SE is defined, in case of re-INVITE release all other timers*/
        /* CallLegSessionTimerReleaseTimer(pCallLeg); - it is done in the beginning of function */ 

        if (pCallLeg->pSessionTimer->eRefresherType != RVSIP_SESSION_EXPIRES_REFRESHER_NONE)
        {
            /*If the refresher for this call is tha local party */
            if (pCallLeg->pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
            {
                /* Set the timer to the alert time value*/
                if (pCallLeg->pSessionTimer->alertTime > 0 &&
                    pCallLeg->pSessionTimer->alertTime < pCallLeg->pSessionTimer->sessionExpires)
                {
                    timeOut = (pCallLeg->pSessionTimer->alertTime)*SESSION_TIMER_MSECPERSEC;
                }
                else
                {
                    timeOut = (pCallLeg->pSessionTimer->defaultAlertTime)*SESSION_TIMER_MSECPERSEC;
                }

                timerFunc = CallLegSessionTimerRefreshAlertTimeout;
            }
            else if (pCallLeg->pSessionTimer->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAS)
            {
                /* if the remote party is the refresher set the timer
                   the (sessionExpires - 32)*/
                UpdateSessionTimerTimeout(pCallLeg->pSessionTimer->sessionExpires,
                                          &timeOut);
                if (timeOut <= 0)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegSessionTimerHandleTimers - Call-Leg 0x%p - Failed to set the session expires timer (the value <= 0)"
                        ,pCallLeg));
                    return rv;
                }
                timerFunc = CallLegSessionTimerSessionExpiresTimeout;

            }
            /* Set the timer*/
           RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerHandleTimers - Call-Leg 0x%p - Setting timer to %d"
                     ,pCallLeg,timeOut));

            rv = SipCommonCoreRvTimerStartEx(
                        &pCallLeg->pSessionTimer->hSessionTimerTimer,
                        pCallLeg->pMgr->pSelect,
                        timeOut,
                        timerFunc,
                        pCallLeg);
            if (rv != RV_OK)
            {
                RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "CallLegSessionTimerHandleTimers - Call-Leg  0x%p: No timer is available",
                    pCallLeg));
                return RV_ERROR_OUTOFRESOURCES;
            }
            else
            {
                RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "CallLegSessionTimerHandleTimers - Call-Leg  0x%p: timer was set to %u milliseconds",
                    pCallLeg,timeOut));
            }

        }
    }

    return rv;
}


/***************************************************************************
* CallLegSessionTimerRefreshAlertTimeout
* ------------------------------------------------------------------------
* General: Called when ever a the alert time to refresh the call is expires.
*          The stack in session timer auto mode will be sent a re-INVITE
*          which will refresh the call,in manual mode the call-back
*          RvSipCallLegSessionTimerRefreshAlertEv will notify the application
*          that the alert time of the session is been expires.
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: timerHandle - The timer that has expired.
*          pContext - The call-Leg this timer was called for.
***************************************************************************/
RvBool CallLegSessionTimerRefreshAlertTimeout(IN void *pContext,
                                              IN RvTimer *timerInfo)
{
    CallLeg             *pCallLeg  = NULL;
    RvStatus             rv         = RV_OK;

    pCallLeg = (CallLeg *)pContext;
    if (RV_OK != CallLegLockMsg(pCallLeg))
    {
        CallLegSessionTimerReleaseTimer(pCallLeg);
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pCallLeg->pSessionTimer->hSessionTimerTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
            "CallLegSessionTimerRefreshAlertTimeout - Call-Leg 0x%p: alert timer expired but was already released. do nothing...",
            pCallLeg));

        CallLegUnLockMsg(pCallLeg);
        return RV_FALSE;
    }
    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pCallLeg->pSessionTimer->hSessionTimerTimer);
    CallLegSessionTimerReleaseTimer(pCallLeg);
    RvLogDebug((pCallLeg->pMgr)->pLogSrc ,((pCallLeg->pMgr)->pLogSrc ,
              "CallLegSessionTimerRefreshAlertTimeout - Call-Leg 0x%p: alert timer expired", pCallLeg));

    switch (pCallLeg->eState)
    {
    case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
        if (pCallLeg->pMgr->manualSessionTimer == RV_FALSE &&
            pCallLeg->pMgr->bOldInviteHandling == RV_TRUE) /* auto Session-Timer mode is disable in new invite */
        {
            rv = HandleAutoRefreshAlert(pCallLeg);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionTimerRefreshAlertTimeout - Call-Leg 0x%p - Failed in HandleAutoRefreshAlert"
                    ,pCallLeg));
                CallLegUnLockMsg(pCallLeg);
                return RV_FALSE;
            }
        }
        else
        {
            /* manual mode notify the application */
            rv = HandleManualRefreshAlert(pCallLeg);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerRefreshAlertTimeout - Call-Leg 0x%p - Failed in HandleManualRefreshAlert"
                ,pCallLeg));
                CallLegUnLockMsg(pCallLeg);
                return RV_FALSE;
            }
        }
        rv = SetAlertTimeoutAndTimer(pCallLeg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerRefreshAlertTimeout - Call-Leg 0x%p - Failed in SetAlertTimeoutAndTimer"
                ,pCallLeg));
            CallLegUnLockMsg(pCallLeg);
            return RV_FALSE;
        }
        break;
    default:
        break;
    }
    CallLegUnLockMsg(pCallLeg);
    return RV_FALSE;
}


/***************************************************************************
 * CallLegSessionTimerSessionExpiresTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever a the session time timer is expires.
 *          The stack in session timer auto mode will be sent a BYE
 *          if no other re-INVITE was received ,in manual mode the call-back
 *          pfnSessionTimerNotificationEvHandler with reason
 *          RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES
 *          will notify the application that the session time of the session
 *          is been expires.
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The call-Leg this timer was called for.
 ***************************************************************************/
RvBool CallLegSessionTimerSessionExpiresTimeout(IN void *pContext,
                                                IN RvTimer *timerInfo)
{
    CallLeg             *pCallLeg  = NULL;
    RvStatus             rv         = RV_OK;

    pCallLeg = (CallLeg *)pContext;
    if (RV_OK != CallLegLockMsg(pCallLeg))
    {
        CallLegSessionTimerReleaseTimer(pCallLeg);
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pCallLeg->pSessionTimer->hSessionTimerTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
            "CallLegSessionTimerSessionExpiresTimeout - Call-Leg 0x%p: session timer expired but was already released. do nothing...",
            pCallLeg));

        CallLegUnLockMsg(pCallLeg);
        return RV_FALSE;
    }

    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pCallLeg->pSessionTimer->hSessionTimerTimer);
    CallLegSessionTimerReleaseTimer(pCallLeg);
    RvLogDebug((pCallLeg->pMgr)->pLogSrc ,((pCallLeg->pMgr)->pLogSrc ,
              "CallLegSessionTimerSessionExpiresTimeout - Call-Leg 0x%p: session timer expired", pCallLeg));

    switch (pCallLeg->eState)
    {
    case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
    {
        /* In auto mode*/
        if (pCallLeg->pMgr->manualSessionTimer == RV_FALSE &&
            pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
        {
            /* Send BYE to disconnect the call*/
            rv = CallLegSessionDisconnect(pCallLeg);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerSessionExpiresTimeout - Call-Leg 0x%p - Failed in CallLegSessionDisconnect"
                ,pCallLeg));
                CallLegUnLockMsg(pCallLeg);
                return RV_FALSE;
            }
        }
        else
        {
            /* manual mode notify the application */
            rv = CallLegCallbackSessionTimerNotificationEv(
                            (RvSipCallLegHandle)pCallLeg,
                            RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerSessionExpiresTimeout - Call-Leg 0x%p - Failed in CallLegCallbackSessionTimerNotificationEv"
                ,pCallLeg));
                CallLegUnLockMsg(pCallLeg);
                return RV_FALSE;
            }
        }
        break;
    }
    default:
        break;
    }
    ResetSessionTimerParams(pCallLeg->pSessionTimer);
    CallLegUnLockMsg(pCallLeg);

    return RV_FALSE;
}

/***************************************************************************
 * CallLegSessionTimerReleaseTimer
 * ------------------------------------------------------------------------
 * General: Release the call-Leg's timer.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-Leg to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegSessionTimerReleaseTimer(IN CallLeg *pCallLeg)
{
    if (NULL    != pCallLeg &&
        NULL    != pCallLeg->pSessionTimer &&
        RV_TRUE == SipCommonCoreRvTimerStarted(&pCallLeg->pSessionTimer->hSessionTimerTimer))
    {
        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pCallLeg->pSessionTimer->hSessionTimerTimer);
        RvLogDebug((pCallLeg->pMgr)->pLogSrc ,((pCallLeg->pMgr)->pLogSrc ,
                  "CallLegSessionTimerReleaseTimer - Call-Leg 0x%p: Timer was released",
                  pCallLeg));
    }
}

/***************************************************************************
 * CallLegSessionTimerAllocate
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the session timer object attach to
 *          the  call leg.
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg   - Handle to the call-leg.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV CallLegSessionTimerAllocate(
                                IN  CallLeg      *pCallLeg,
                                IN  RvInt32     size)
{
    RvInt32            offset = 0;
    if (pCallLeg ==NULL)
    {
        return NULL;
    }
    pCallLeg->pSessionTimer = (CallLegSessionTimer *)RPOOL_AlignAppend(pCallLeg->pMgr->hGeneralPool,
                                                pCallLeg->hPage,
                                                size, &offset);
    return pCallLeg->pSessionTimer;
}

/***************************************************************************
 * CallLegSessionTimerAllocateNegotiationSessionTimer
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the temp session timer object attach to
 *          the  call leg.
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg   - Handle to the call-leg.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV CallLegSessionTimerAllocateNegotiationSessionTimer(
                                IN  CallLeg      *pCallLeg,
                                IN  RvInt32     size)
{
    RvInt32            offset = 0;
    if (pCallLeg ==NULL)
    {
        return NULL;
    }
    pCallLeg->pNegotiationSessionTimer = (CallLegNegotiationSessionTimer *)RPOOL_AlignAppend(pCallLeg->pMgr->hGeneralPool,
                                                           pCallLeg->hPage,
                                                           size, &offset);
    return pCallLeg->pNegotiationSessionTimer;
}


/***************************************************************************
 * CallLegSessionTimerInitialize
 * ------------------------------------------------------------------------
 * General: Initialize the session timer object in the call leg
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize due to a
 *                                   resources problem.
 *               RV_OK - initialize generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the new call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerInitialize(IN  CallLeg          *pCallLeg)
{
    RvStatus  rv    = RV_OK;

    /* Allocate space for the session timer structure*/
    pCallLeg->pSessionTimer = (CallLegSessionTimer *)CallLegSessionTimerAllocate(
        pCallLeg,
        sizeof(CallLegSessionTimer));
    if (pCallLeg->pSessionTimer == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
     /* Allocate space for the temp session timer structure*/
    pCallLeg->pNegotiationSessionTimer = (CallLegNegotiationSessionTimer *)
      CallLegSessionTimerAllocateNegotiationSessionTimer(pCallLeg,
                             sizeof(CallLegNegotiationSessionTimer));
    if (pCallLeg->pNegotiationSessionTimer == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    CallLegSessionTimerSetMgrDefaults(
            pCallLeg->pMgr,RV_TRUE,pCallLeg->pNegotiationSessionTimer);
    pCallLeg->pNegotiationSessionTimer->bInRefresh = RV_FALSE;
    ResetSessionTimerParams(pCallLeg->pSessionTimer);
    /* Set the 'constant' values in the CallLeg */
    pCallLeg->pSessionTimer->bAddReqMinSE = pCallLeg->pMgr->bAddReqMinSE;
    pCallLeg->pSessionTimer->minSE        = pCallLeg->pMgr->MinSE;

    SipCommonCoreRvTimerInit(&pCallLeg->pSessionTimer->hSessionTimerTimer);
    return rv;
}

/***************************************************************************
* CallLegSessionTimerAddParamsToMsg
* ------------------------------------------------------------------------
* General: Copy the session timer parameters from the call to the msg
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - pointer to the new call-leg
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerAddParamsToMsg(
               IN  CallLeg                          *pCallLeg,
               IN  RvInt32                           sessionExpires,
               IN  RvInt32                           minSE,
               IN  RvSipSessionExpiresRefresherType  eRefresherType,
               IN  RvSipMsgHandle                    hMsg)
{
    RvStatus rv = RV_OK;

    if (sessionExpires > 0)
    {
        /* Add the Session-Expires header to the msg*/
        rv = CallLegSessionTimerAddSessionExpiresToMsg(
                        sessionExpires,eRefresherType,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionTimerAddParamsToMsg - Call-Leg 0x%p - Failed to add Session-Expires header to the msg"
                ,pCallLeg));
            return rv;
        }
    }

    /* Add the MinSE header to the msg even if it's zero or the Sesssion Expires */
    /* is zero. Otherwise the remote side might assume that local side Min-SE is */
    /* the default value (90). */
    rv = CallLegSessionTimerAddMinSEToMsg(pCallLeg,minSE,hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionTimerAddParamsToMsg - Call-Leg 0x%p - Failed to add Min-SE header to the msg"
            ,pCallLeg));
        return rv;
    }

    return rv;
}

/***************************************************************************
* CallLegSessionTimerSetMgrDefaults
* ------------------------------------------------------------------------
* General: Assigns the default values, which are taken from the CallLeg Mgr
*          structure into the session parameters.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input :  pCallLegMgr   - Pointer to the CallLeg Mgr.
*          bUpdateMinSE  - Indicates if the MinSE is updated as well.
* Output:  pSessionTimer - Pointer to the set session timer structure.
***************************************************************************/
void RVCALLCONV CallLegSessionTimerSetMgrDefaults(
                       IN  CallLegMgr                     *pCallLegMgr,
                       IN  RvBool                          bUpdateMinSE,
                       OUT CallLegNegotiationSessionTimer *pSessionTimer)
{
    if (bUpdateMinSE == RV_TRUE)
    {
        pSessionTimer->minSE = pCallLegMgr->MinSE;
    }
    pSessionTimer->sessionExpires       = pCallLegMgr->sessionExpires;
    pSessionTimer->eRefresherType       = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
    pSessionTimer->eRefresherPreference = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* UpdateSessionTimerTimeout
* ------------------------------------------------------------------------
* General: Update the session timer timeout according to sessionExpires
*          value.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  sessionExpires - Session Expires interval value in seconds
* Output: pMSecTimeout   - The suitable timeout interval in milliseconds
***************************************************************************/
static void RVCALLCONV UpdateSessionTimerTimeout(
                                  IN  RvInt32  sessionExpires,
                                  OUT RvInt32 *pMSecTimeout)
{
    /* Find min(SESSION_TIMER_INTERVAL_PRECEDER,                */
    /*          sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER)  */
    if (SESSION_TIMER_INTERVAL_PRECEDER >
        sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER)
    {
        *pMSecTimeout = sessionExpires - sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER;
    }
    else
    {
        *pMSecTimeout = sessionExpires - SESSION_TIMER_INTERVAL_PRECEDER;
    }

    /* Convert the Timeout to millisec units */
    *pMSecTimeout *= SESSION_TIMER_MSECPERSEC;
}

/***************************************************************************
* UpdateSessionTimerAlertTimeout
* ------------------------------------------------------------------------
* General: Update the session timer ALERT timeout according to
*          sessionExpires value.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  sessionExpires - Session Expires interval value in seconds
* Output: pMSecTimeout   - The suitable timeout interval in milliseconds
***************************************************************************/
static void RVCALLCONV UpdateSessionTimerAlertTimeout(
                                  IN    RvInt32  sessionExpires,
                                  INOUT RvInt32 *pTimeout)
{
    if (!(SESSION_TIMER_INTERVAL_PRECEDER >= *pTimeout &&
          sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER >= *pTimeout))
    {
        if (SESSION_TIMER_INTERVAL_PRECEDER >
            sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER)
        {
            *pTimeout = (*pTimeout - sessionExpires/SESSION_TIMER_INTERVAL_DEVIDER);
        }
        else
        {
            *pTimeout = (*pTimeout - SESSION_TIMER_INTERVAL_PRECEDER);
        }
    }

    *pTimeout *= SESSION_TIMER_MSECPERSEC;
}

/***************************************************************************
* GetSessionTimerPointer
* ------------------------------------------------------------------------
* General: Retrieves the suitable pointer of the CallLeg Session Timer
*          data structure.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg         - Pointer to the CallLeg.
*		  hTransc		   - Handle to transaction
*         bInviteTransc    - Indication if the transc msg is an UPDATE 
*							 request/response
* Output: ppSessionExpires - Pointer to the Session Expires struct
***************************************************************************/
static RvStatus RVCALLCONV GetSessionTimerPointer(
                            IN  CallLeg                         *pCallLeg,
							IN  RvSipTranscHandle				 hTransc,
							IN  RvBool							 bInviteTransc,
                            OUT CallLegNegotiationSessionTimer **ppSessionTimer)
{
    *ppSessionTimer = NULL;

    if (bInviteTransc == RV_TRUE)
    {
        *ppSessionTimer = (CallLegNegotiationSessionTimer *)pCallLeg->pNegotiationSessionTimer;
    }
    else 
    {
        *ppSessionTimer =
            (CallLegNegotiationSessionTimer*)SipTransactionGetSessionTimerMemory(hTransc);
    }

    return RV_OK;
}

/***************************************************************************
* ResetSessionTimerParams
* ------------------------------------------------------------------------
* General: Resets the session timer parameters of a CallLeg
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pSessionTimer - Pointer to the session timer parameters struct
*                         in a CallLeg.
* Output: -
***************************************************************************/
static void RVCALLCONV ResetSessionTimerParams(
                              OUT CallLegSessionTimer  *pSessionTimer)
{
    pSessionTimer->eRefresherType     = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
    pSessionTimer->sessionExpires     = UNDEFINED;
    pSessionTimer->minSE              = UNDEFINED;
    pSessionTimer->defaultAlertTime   = UNDEFINED;
    pSessionTimer->alertTime          = UNDEFINED;
    pSessionTimer->eCurrentRefresher  = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_NONE;
    pSessionTimer->bAddReqMinSE       = RV_FALSE;
    pSessionTimer->bAddOnceMinSE      = RV_FALSE;
    pSessionTimer->eNegotiationReason =
        RVSIP_CALL_LEG_SESSION_TIMER_NEGOTIATION_REASON_UNDEFINED;
}

/***************************************************************************
* CallLegSessionTimerAddMinSEToMsg
* ------------------------------------------------------------------------
* General: Adds a Min-SE header to outgoing message. The header is taken
*          from the call-leg object.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg - Pointer to the CallLeg.
*         minSE    - The minSE value that will be insert to the msg.
*         hMsg     - Handle to the message.
***************************************************************************/
static RvStatus RVCALLCONV CallLegSessionTimerAddMinSEToMsg(
                                IN  CallLeg            *pCallLeg,
                                IN  RvInt32             minSE,
                                IN  RvSipMsgHandle      hMsg)
{
    RvStatus                rv            = RV_OK;
    RvSipMinSEHeaderHandle  hMinSEHeader  = NULL;
    CallLegSessionTimer    *pSessionTimer = pCallLeg->pSessionTimer;
    RvSipMsgType            eMsgType      = RvSipMsgGetMsgType(hMsg);
    RvUint16                responseCode  = RvSipMsgGetStatusCode(hMsg);

    /* If the message is a non 422 response the Min-SE shouldn't be added */
     if(eMsgType     == RVSIP_MSG_RESPONSE &&
        responseCode != INTERVAL_TOO_SMALL_RESPONSE)
     {
         return RV_OK;
     }

    /* Check if the MinSE should be added - in case that its' value  */
    /* differs the default or in case that 422 response was received */
    if (eMsgType                     == RVSIP_MSG_REQUEST        &&
        minSE                        == RVSIP_CALL_LEG_MIN_MINSE &&
        pSessionTimer->bAddReqMinSE  == RV_FALSE                 &&
        pSessionTimer->bAddOnceMinSE == RV_FALSE)
    {
        return RV_OK;
    }


    rv = RvSipMinSEHeaderConstructInMsg(hMsg, RV_FALSE,&hMinSEHeader);
    if (rv != RV_OK)
    {
        return rv;
    }
    rv = RvSipMinSEHeaderSetDeltaSeconds(hMinSEHeader,minSE);

    pSessionTimer->bAddOnceMinSE = RV_FALSE;

    return rv;
}

/***************************************************************************
* GetSessionExpiresValues
* ------------------------------------------------------------------------
* General: Retrieves the Session-Expires header values.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  hMsg             - Handle to the message.
* Output: pSessionExpires  - The retrieved session expires value.
*         peRefresherType  - The retrieved refresher type.
***************************************************************************/
static RvStatus RVCALLCONV GetSessionExpiresValues(
                       IN  CallLeg                          *pCallLeg,
                       IN  RvSipMsgHandle                    hMsg,
                       OUT RvInt32                          *pSessionExpires,
                       OUT RvSipSessionExpiresRefresherType *peRefresherType)
{
    RvSipSessionExpiresHeaderHandle  hSessionExpiresHeader = NULL;
    RvSipHeaderListElemHandle        hListElem             = NULL;
    RvStatus                         rv;

    *pSessionExpires = 0;

    /* get the Session-Expires value from the msg*/
    hSessionExpiresHeader = (RvSipSessionExpiresHeaderHandle)RvSipMsgGetHeaderByType(
        hMsg, RVSIP_HEADERTYPE_SESSION_EXPIRES,
        RVSIP_FIRST_HEADER, &hListElem);

    if (NULL == hSessionExpiresHeader)
    {
        return RV_OK;
    }

    rv = RvSipSessionExpiresHeaderGetDeltaSeconds(
                                hSessionExpiresHeader,
                                pSessionExpires);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GetSessionExpiresValues - pCallLeg=0x%p hMsg=0x%p Failed in RvSipSessionExpiresHeaderGetDeltaSeconds",
            pCallLeg,hMsg));
        return rv;
    }
    if (*pSessionExpires <= 0)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GetSessionExpiresValues - pCallLeg=0x%p hMsg=0x%p contains illegal Session-Expires (%d)",
            pCallLeg,hMsg,*pSessionExpires));
        return RV_ERROR_UNKNOWN;
    }

    rv = RvSipSessionExpiresHeaderGetRefresherType(
                                hSessionExpiresHeader,
                                peRefresherType);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GetSessionExpiresValues - pCallLeg 0x%p hMsg 0x%p - Failed in RvSipSessionExpiresHeaderGetRefresherType",
            pCallLeg,hMsg));
        return rv;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

    return RV_OK;
}

/***************************************************************************
* GetMinSEValue
* ------------------------------------------------------------------------
* General: Retrieves the Min-SE header values.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  hMsg    - Handle to the message.
* Output: pMinSE  - The retrieved Min-SE value.
***************************************************************************/
static RvStatus RVCALLCONV GetMinSEValue(
                                      IN  CallLeg       *pCallLeg,
                                      IN  RvSipMsgHandle hMsg,
                                      OUT RvInt32       *pMinSE)
{
    RvSipHeaderListElemHandle hListElem    = NULL;
    RvSipMinSEHeaderHandle    hMinSEHeader = NULL;
    RvStatus                  rv;

    *pMinSE   = UNDEFINED;
    hListElem = NULL;

    /* get the MinSE value from the msg*/
    hMinSEHeader = (RvSipMinSEHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_MINSE,RVSIP_FIRST_HEADER, &hListElem);

    if (NULL != hMinSEHeader)
    {
        rv = RvSipMinSEHeaderGetDeltaSeconds(hMinSEHeader,pMinSE);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GetMinSEValue - pCallLeg 0x%p hMsg 0x%p - Failed in RvSipMinSEHeaderGetDeltaSeconds",
                pCallLeg,hMsg));
            return rv;
        }
        if (*pMinSE < 0)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GetMinSEValue - pCallLeg 0x%p hMsg 0x%p - Min-SE header contains illegal value (%d)",
                pCallLeg,hMsg,*pMinSE));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        /* If the MinSE header doesn't exist in the */
        /* message it's value is the default (90)   */
        *pMinSE = RVSIP_CALL_LEG_MIN_MINSE;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

    return RV_OK;
}

/***************************************************************************
* IsSessionTimerSupportedByRemote
* ------------------------------------------------------------------------
* General: Checks if the Session-Timer feature is supported remotely.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  hMsg - Handle to the message.
***************************************************************************/
static RvBool RVCALLCONV IsSessionTimerSupportedByRemote(
                                        IN  RvSipMsgHandle hMsg)
{

    RvBool bHeaderFound = RV_FALSE;

    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"Supported","timer",NULL);

    if(bHeaderFound == RV_TRUE)
    {
        return RV_TRUE;
    }

    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"k","timer",NULL);

    if(bHeaderFound == RV_TRUE)
    {
        return RV_TRUE;
    }
    return RV_FALSE;
}

/***************************************************************************
* HandleAutoRefreshAlert
* ------------------------------------------------------------------------
* General: Handle the automatic mode of session-timer alert timeout:
*          Send a re-INVITE to refresh the call
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: pCallLeg - The call-Leg this timer was called for.
***************************************************************************/
static RvStatus RVCALLCONV HandleAutoRefreshAlert(CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;

    rv = RvSipCallLegSessionTimerSetPreferenceParams(
                                                (RvSipCallLegHandle)pCallLeg,
                                                pCallLeg->pSessionTimer->sessionExpires,
                                                pCallLeg->pSessionTimer->minSE,
                                                RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleAutoRefreshAlert - Call-Leg 0x%p - Failed in RvSipCallLegSessionTimerSetPreferenceParams"
            ,pCallLeg));
        return rv;
    }
    rv = RvSipCallLegSessionTimerRefresh((RvSipCallLegHandle)pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleAutoRefreshAlert - Call-Leg 0x%p - Failed to refresh the call"
            ,pCallLeg));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
* HandleManualRefreshAlert
* ------------------------------------------------------------------------
* General: Handle the manual mode of session-timer alert timeout:
*          Notifies application with RvSipCallLegSessionTimerRefreshAlertEv
*          that the alert time of the session is been expires.
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: pCallLeg - The call-Leg this timer was called for.
***************************************************************************/
static RvStatus RVCALLCONV HandleManualRefreshAlert(CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;

    rv = CallLegCallbackSessionTimerRefreshAlertEv((RvSipCallLegHandle)pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleManualRefreshAlert - Call-Leg 0x%p - Failed in CallLegCallbackSessionTimerRefreshAlertEv"
            ,pCallLeg));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
* SetAlertTimeoutAndTimer
* ------------------------------------------------------------------------
* General: Decide of the timeout value for the alert timer, and set the timer.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: pCallLeg - The call-Leg this timer was called for.
***************************************************************************/
static RvStatus RVCALLCONV SetAlertTimeoutAndTimer(CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;
    RvInt32  timeOut = 0;

    if (pCallLeg->pSessionTimer->defaultAlertTime > 0  &&
        pCallLeg->pSessionTimer->sessionExpires   > 0)
    {
        if (pCallLeg->pSessionTimer->alertTime > 0 &&
            pCallLeg->pSessionTimer->alertTime < pCallLeg->pSessionTimer->sessionExpires)
        {
            timeOut = (pCallLeg->pSessionTimer->sessionExpires) - (pCallLeg->pSessionTimer->alertTime);
        }
        else
        {
            timeOut = (pCallLeg->pSessionTimer->sessionExpires) - (pCallLeg->pSessionTimer->defaultAlertTime);
        }

        UpdateSessionTimerAlertTimeout(pCallLeg->pSessionTimer->sessionExpires, &timeOut);
    }
    else if (pCallLeg->pSessionTimer->sessionExpires > 0)
    {
        timeOut = pCallLeg->pSessionTimer->sessionExpires/2;
        UpdateSessionTimerAlertTimeout(pCallLeg->pSessionTimer->sessionExpires, &timeOut);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetAlertTimeoutAndTimer - Call-Leg 0x%p - failed to set the Session Expires timer - AlertTime and SessionExpires <=0"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    if (timeOut <= 0)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetAlertTimeoutAndTimer - Call-Leg 0x%p - Failed to set the session expires timer (the value is too small)"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    /* Set the timer*/
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "SetAlertTimeoutAndTimer - Call-Leg 0x%p - Setting timer to %d"
        ,pCallLeg,timeOut));

    rv = SipCommonCoreRvTimerStartEx( &pCallLeg->pSessionTimer->hSessionTimerTimer,
                                    pCallLeg->pMgr->pSelect,
                                    timeOut,
                                    CallLegSessionTimerSessionExpiresTimeout,
                                    pCallLeg);
    if (rv != RV_OK)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "SetAlertTimeoutAndTimer - Call-Leg  0x%p: No timer is available",
            pCallLeg));
        return rv;
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetAlertTimeoutAndTimer - Call-Leg 0x%p - Set the session Expires timer to %d milisecond"
            ,pCallLeg,timeOut));
    }
    return RV_OK;
}

/***************************************************************************
* IsInviteOrUpdateTransc
* ------------------------------------------------------------------------
* General: Checks if the given message method is INVITE/UPDATE. If the 
*		   message method is none of the mention above then an ERROR
*		   code is returned. There are 2 possiblities for checking 
*		   the INVITE/UPDATE methods: 
*		   1. By the Msg handle (This option is preferable, since by the 
*			  Transaction handle we cannot distinguish between ACK and
*			  INVITE messages).
*		   2. By the Transaction handle. 
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg     - The message related call-leg
*		  hTransc      - The transaction handle
*		  hMsg		   - The message which the method is retrieved from
* Output: pbInvite     - Indication if the found method type is UPDATE
***************************************************************************/
static RvStatus RVCALLCONV IsInviteOrUpdateTransc(
									IN  CallLeg           *pCallLeg,
									IN  RvSipTranscHandle  hTransc,
									IN  RvSipMsgHandle     hMsg,
									OUT RvBool		      *pbInvite)
{
	
	/* First case: When the Msg handle is not NULL the required details */
	/* can be retrieved from it */ 
	if (hMsg != NULL) 
	{
		return IsInviteOrUpdateFromMsg(pCallLeg,hMsg,pbInvite);
	}
	/* Second case: When the Transc handle is not NULL the required details */
	/* can be retrieved from it */ 
	else if (hTransc != NULL) 
	{
		return IsInviteOrUpdateFromTransc(pCallLeg,hTransc,pbInvite);
	}
	/* Third caes: When both of the handles are NULL */
	else 
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"IsInviteOrUpdateTransc - Call-Leg 0x%p - the Transc and Msg are both NULL",
            pCallLeg));
		return RV_ERROR_INVALID_HANDLE;
	}
	
}

/***************************************************************************
* IsInviteOrUpdateFromMsg
* ------------------------------------------------------------------------
* General: Checks if the given message method is INVITE/UPDATE based on
*		   the Msg handle details. If the 
*		   message method is none of the mention above then an ERROR
*		   code is returned.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg     - The message related call-leg
*		  hTransc      - The transaction handle
* Output: pbInvite     - Indication if the found method type is UPDATE
***************************************************************************/
static RvStatus RVCALLCONV IsInviteOrUpdateFromMsg(
									  IN  CallLeg           *pCallLeg,
									  IN  RvSipMsgHandle     hMsg,
									  OUT RvBool		    *pbInvite)
{
	RvSipMsgType	eMsgType      = RvSipMsgGetMsgType(hMsg);
	RvChar	        strMethod[10] = "";
	RvSipMethodType eMsgMethod;
	RvUint			methodActualLen;

	switch (eMsgType) 
	{
	case RVSIP_MSG_REQUEST:
		eMsgMethod = RvSipMsgGetRequestMethod(hMsg);
		if (eMsgMethod == RVSIP_METHOD_OTHER) 
		{
			RvSipMsgGetStrRequestMethod(
				hMsg,strMethod,sizeof(strMethod),&methodActualLen);
		}
		break;
	case RVSIP_MSG_RESPONSE:
		{
			RvSipCSeqHeaderHandle hCSeq = RvSipMsgGetCSeqHeader(hMsg);
			if (hCSeq == NULL)
			{
				RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"IsInviteOrUpdateFromMsg - Call-Leg 0x%p - the given message 0x%p doesn't include a CSeq header"
					,pCallLeg,hMsg));
				return RV_ERROR_BADPARAM;
			}
			eMsgMethod = RvSipCSeqHeaderGetMethodType(hCSeq);
			if (eMsgMethod == RVSIP_METHOD_OTHER) 
			{
				RvSipCSeqHeaderGetStrMethodType(
					hCSeq,strMethod,sizeof(strMethod),&methodActualLen);
			}
		}
		break;
	default:
		eMsgMethod = RVSIP_METHOD_UNDEFINED; 
	}
	
	if (eMsgMethod == RVSIP_METHOD_INVITE) 
	{
		*pbInvite = RV_TRUE;
	}
	else if (eMsgMethod == RVSIP_METHOD_OTHER && 
		SipCommonStricmp(strMethod,CALL_LEG_UPDATE_METHOD_STR) == RV_TRUE) 
	{
		*pbInvite = RV_FALSE;
	}
	else
	{
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"IsInviteOrUpdateFromMsg - Call Leg 0x%p, Msg 0x%p is not INVITE/UPDATE"
			,pCallLeg,hMsg));
		/* The message method is NOT INVITE/UPDATE thus the sessionTimer cannot be handled */
		return RV_ERROR_BADPARAM;
	}
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

	return RV_OK;
}

/***************************************************************************
* IsInviteOrUpdateFromTransc
* ------------------------------------------------------------------------
* General: Checks if the given message method is INVITE/UPDATE based on
*		   the Transaction handle details. If the 
*		   message method is none of the mention above then an ERROR
*		   code is returned.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg     - The message related call-leg
*		  hTransc      - The transaction handle
* Output: pbInvite     - Indication if the found method type is UPDATE
***************************************************************************/
static RvStatus RVCALLCONV IsInviteOrUpdateFromTransc(
									  IN  CallLeg           *pCallLeg,
									  IN  RvSipTranscHandle  hTransc,
									  OUT RvBool		    *pbInvite)
{
	SipTransactionMethod eTranscMethod;
		
	SipTransactionGetMethod(hTransc,&eTranscMethod);
	switch (eTranscMethod) 
	{
	case SIP_TRANSACTION_METHOD_INVITE:
		*pbInvite = RV_TRUE;
		break;
	case SIP_TRANSACTION_METHOD_OTHER:
		if (CallLegIsUpdateTransc(hTransc) == RV_TRUE) 
		{
			*pbInvite = RV_FALSE;
		}
		else 
		{
			RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"IsInviteOrUpdateFromTransc - Call Leg 0x%p, Transc 0x%p is not INVITE/UPDATE"
				,pCallLeg,hTransc));
			return RV_ERROR_BADPARAM;
		}
		break;
	default:
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"IsInviteOrUpdateFromTransc - Call Leg 0x%p, Transc 0x%p is not INVITE/UPDATE"
			,pCallLeg,hTransc));
		return RV_ERROR_BADPARAM;
	}
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

	return RV_OK;
}

/***************************************************************************
* GetResponseCode
* ------------------------------------------------------------------------
* General: Retrieve the response code:
*		   1. By the Msg handle.
*		   2. By the Transaction handle. 
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:  pCallLeg     - The message related call-leg
*		  hTransc      - The transaction handle
*		  hMsg		   - The message which the method is retrieved from
* Output: pRespCode    - The retrieved response code
***************************************************************************/
static RvStatus RVCALLCONV GetResponseCode(
									IN  CallLeg           *pCallLeg,
									IN  RvSipTranscHandle  hTransc,
									IN  RvSipMsgHandle     hMsg,
									OUT RvUint16		  *pRespCode)
{
	if (hTransc != NULL) 
	{
		SipTransactionGetResponseCode(hTransc,pRespCode);
	}
	else if (hMsg != NULL) 
	{
		*pRespCode = RvSipMsgGetStatusCode(hMsg);
	}
	else
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GetResponseCode - Call-Leg 0x%p - cannot retrieve the RespCode the both hMsg and hTrans are NULL"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

	return RV_OK;
}

#endif /* ifndef RV_SIP_PRIMITIVES */

