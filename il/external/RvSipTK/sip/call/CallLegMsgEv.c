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
 *                              <CallLegMsgEv.c>
 *
 *  Handles Message Events received from the transaction layer.
 *  The events includes message received and message to sent events.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                          */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegMsgEv.h"
#include "CallLegObject.h"
#include "CallLegCallbacks.h"
#include "CallLegRouteList.h"
#include "CallLegMsgLoader.h"
#include "CallLegAuth.h"
#include "CallLegRefer.h"
#include "CallLegSubs.h"
#include "CallLegSessionTimer.h"
#include "CallLegInvite.h"
#include "_SipCommonUtils.h"
#include "RvSipContactHeader.h"
#include "RvSipMsg.h"
#include "RvSipOtherHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipCSeqHeader.h"
#include "_SipSubs.h"
#ifdef RV_SIP_IMS_ON
#include "CallLegSecAgree.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvBool RVCALLCONV IsContactExistInMsg(IN  RvSipMsgHandle hMsg);

/*-------------------------------------------------------------------------
      H A N D L E     M E S S A G E      E V E N T S
 --------------------------------------------------------------------------*/

static RvStatus RVCALLCONV CommonMsgToSend(
                       IN  RvSipTranscHandle       hTransc,
                       IN  CallLeg                *pCallLeg,
                       IN  RvSipMsgHandle          hMsg);

static void RVCALLCONV LoadFromInviteRequestRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);

static void RVCALLCONV LoadFromInviteResponseRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);

static RvStatus RVCALLCONV AddContactHeaderToOutgoingMsgIfNeeded(
                        IN  CallLeg                 *pCallLeg,
                        IN    RvSipTranscHandle       hTransc,
                        IN  RvSipMsgHandle          hMsg);

static RvStatus RVCALLCONV AddContactHeaderToMsg (
                                   IN  CallLeg                 *pCallLeg,
                                   IN  RvSipMsgHandle          hMsg);

#ifdef RV_SIP_SUBS_ON
static void RVCALLCONV LoadFromSubsRcvdMsg(
                                    IN CallLeg             *pCallLeg,
                                    IN RvSipTranscHandle    hTransc,
                                    IN RvSipMsgHandle       hMsg,
                                    IN RvSipMsgType         eMsgType,
                                    IN SipTransactionMethod eMethod);

static void RVCALLCONV NotifyReferGeneratorIfNeeded(
                                IN CallLeg             *pCallLeg,
                                IN RvSipMsgHandle       hMsg,
                                IN RvSipMsgType         eMsgType,
                                IN SipTransactionMethod eMethod);
#endif /* #ifdef RV_SIP_SUBS_ON */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegMsgEvMsgToSendHandler
 * ------------------------------------------------------------------------
 * General: Handles a message that is about to be sent and notify the
 *          application.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call leg the transaction belongs to.
 *          hTransc  - The transaction that is about to send this message.
 *            hMsg     - Handle to the message ready to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsgEvMsgToSendHandler(
                        IN  RvSipTranscHandle       hTransc,
                        IN  RvSipTranscOwnerHandle  hCallLeg,
                        IN  RvSipMsgHandle          hMsg)

{
    CallLeg                *pCallLeg      = (CallLeg*)hCallLeg;
    RvStatus              rv;
    RvBool                bAppTransc     = SipTransactionIsAppInitiator(hTransc);
    SipTransactionMethod   eMethod;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegMsgEvMsgToSendHandler - Call-Leg 0x%p - Is about to send a message"
                  ,pCallLeg));

    SipTransactionGetMethod(hTransc,&eMethod);
    
    /*add route list to outgoing messages*/
    rv = CallLegRouteListAddToMessage(pCallLeg,hTransc,hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegMsgEvMsgToSendHandler - Call-Leg 0x%p - Failed to add route list to message - message will not be sent"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
	
#ifdef RV_SIP_IMS_ON
	/* Add security information to outgoing messages */
	rv = CallLegSecAgreeMsgToSend(pCallLeg, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"CallLegMsgEvMsgToSendHandler - Call-Leg 0x%p - Failed to add security information to message - message will not be sent"
					,pCallLeg));
        return rv;
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    rv = CommonMsgToSend(hTransc, pCallLeg, hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegMsgEvMsgToSendHandler - Call-Leg 0x%p - Failed in CommonMsgToSend (rv=%d)"
            ,pCallLeg,rv));
        return rv;
        
    }

    if(bAppTransc == RV_FALSE) /*handle stack transactions*/
    {
        /* If this is a subscribe or notify request, it should be inform with subscription
        msgToSendEv. */
        if(SipTransactionGetSubsInfo(hTransc) != NULL)
        {
            rv = SipSubsMsgToSendEvHandling((RvSipCallLegHandle)hCallLeg, hTransc, hMsg, eMethod);
            return rv;
        }
    }
    /* If the stack suppprt session timer, the session timer parameters need to be handles*/
    if (pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
    {
        rv = CallLegSessionTimerHandleSendMsg(pCallLeg,hTransc,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegMsgEvMsgToSendHandler - Call-Leg 0x%p - Failed in CallLegSessionTimerHandleSendMsg"
                ,pCallLeg));
            return rv;
        }
    }

    rv = CallLegCallbackMsgToSendEv((RvSipCallLegHandle)pCallLeg,hMsg);
    return rv;
}

/***************************************************************************
 * CommonMsgToSend
 * ------------------------------------------------------------------------
 * General: The function holds the common code for all message-to-send handling.
 *          1. Add contact header
 *          2. Add route list
 *          3. Add Authorization header
 *          4. Call MsgToSend callback.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call leg the transaction belongs to.
 *            hTransc  - The transaction that is about to send this message.
 *            hMsg     - Handle to the message ready to be sent.
 ***************************************************************************/
static RvStatus RVCALLCONV CommonMsgToSend(
                        IN  RvSipTranscHandle       hTransc,
                        IN  CallLeg                *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)

{
    RvStatus        rv;
#ifdef RV_SIP_AUTH_ON
    RvSipMsgType    eMessageType   = RvSipMsgGetMsgType(hMsg);
#endif
    /*add contact header to the outgoing message if the message require contact
      according to the standard*/
    rv= AddContactHeaderToOutgoingMsgIfNeeded(pCallLeg,hTransc, hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CommonMsgToSend - Call-Leg 0x%p - Failed to add contact - message will not be sent"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

#ifdef RV_SIP_AUTH_ON
    /* add authorization header to the msg if needed*/
    if (RVSIP_MSG_REQUEST == eMessageType)
    {
        rv = CallLegAuthAddAuthorizationHeadersToMsg(pCallLeg, hTransc, hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CommonMsgToSend - Call-Leg 0x%p - Failed to add authorization header to message - message will not be sent (rv=%d:%s)",
                pCallLeg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
#ifdef RV_SIP_IMS_ON
        /* Add initial Authorization header to all requests except ACK */
        if(NULL != pCallLeg->hInitialAuthorization &&
            RvSipMsgGetRequestMethod(hMsg) != RVSIP_METHOD_ACK)
        {
            RvSipHeaderListElemHandle hElem = NULL;
            /* insert the empty authorization header to a request message, 
               only if no other authorization header was set:
               1. no auth obj
               2. no authorization header in message (global header in advance...)*/
            if(NULL == pCallLeg->hListAuthObj &&
               NULL == RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_AUTHORIZATION, RVSIP_FIRST_HEADER, &hElem))
            {
                RvSipHeaderListElemHandle hListElem = NULL;
                void                     *hMsgListElem;

                rv = RvSipMsgPushHeader( hMsg, RVSIP_LAST_HEADER, (void *)(pCallLeg->hInitialAuthorization),
                                         RVSIP_HEADERTYPE_AUTHORIZATION, &hListElem, &hMsgListElem);
                if (RV_OK != rv)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CommonMsgToSend - CallLeg 0x%p - Failed to insert the empty authorization header (rv=%d:%s)",
                        pCallLeg, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
            }
        }
#endif /*#ifdef RV_SIP_IMS_ON*/
        
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    return rv;
}
/***************************************************************************
* CallLegMsgEvMsgRcvdHandler
* ------------------------------------------------------------------------
* General: Handle a message received event. Take needed
*          parameters from the message and notify the application.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg - The call leg the transaction belongs to.
*        hTransc  - The transaction that received this message.
*         hMsg     - Handle to the message that was received.
***************************************************************************/
RvStatus RVCALLCONV CallLegMsgEvMsgRcvdHandler(
                                                IN  RvSipTranscHandle       hTransc,
                                                IN  RvSipTranscOwnerHandle  hCallLeg,
                                                IN  RvSipMsgHandle          hMsg)

{
    CallLeg                         *pCallLeg  = (CallLeg*)hCallLeg;
    RvSipMsgType                    eMsgType   = RvSipMsgGetMsgType(hMsg);
    RvInt16                         statusCode = RvSipMsgGetStatusCode(hMsg);
    RvStatus                        rv         = RV_OK;
    SipTransactionMethod            eMethod;
    RvSipMethodType                 eMsgMethod = RvSipMsgGetRequestMethod(hMsg);
	SipTransportAddress*            pRcvdFromAddr;
#ifdef RV_SIP_SUBS_ON
    RvBool                          bAppTransc = SipTransactionIsAppInitiator(hTransc);
#endif /* #ifdef RV_SIP_SUBS_ON */


    SipTransactionGetMethod(hTransc,&eMethod);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegMsgEvMsgRcvdHandler - Call-Leg 0x%p (htrans=0x%p,hMsg=0x%p) - received a message",
                  pCallLeg,hTransc,hMsg));
	
	/* Save received from address to call-leg */
	pRcvdFromAddr = SipTransactionGetReceivedFromAddress(hTransc);
	CallLegSaveReceivedFromAddr(pCallLeg, pRcvdFromAddr);
#ifdef RV_SIP_IMS_ON
	/* Temporarily store the transaction in the call leg (until event state changed is called) */
	pCallLeg->hMsgRcvdTransc = hTransc;
#endif /* RV_SIP_IMS_ON */

    /* If this is a subscribe or notify request, it should be inform with subscription
    msgRcvdEv. */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        rv = SipSubsMsgRcvdEvHandling((RvSipCallLegHandle)hCallLeg, hTransc,
                                             hMsg, eMethod);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegMsgEvMsgRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - SipSubsMsgRcvdEvHandling returned %d",
                  pCallLeg,hMsg,rv));
            return rv;
        }
    }
    else
    {
        rv = CallLegCallbackMsgReceivedEv((RvSipCallLegHandle)pCallLeg,hMsg);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegMsgEvMsgRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - status %d returned from application",
                  pCallLeg,hMsg,rv));
            return rv;
        }
    }

    /*this is an invite transaction - the message can be INVITE, OK or ACK*/
    if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
    {
        if(eMsgType == RVSIP_MSG_REQUEST)
        {
            if(eMsgMethod == RVSIP_METHOD_INVITE)
            {
                LoadFromInviteRequestRcvdMsg(pCallLeg,hMsg);
            }
        }
        if(eMsgType == RVSIP_MSG_RESPONSE)
        {
            LoadFromInviteResponseRcvdMsg(pCallLeg,hMsg);
        }
    }

#ifdef RV_SIP_AUTH_ON
    /* Load the authentication headers from the message (if needed) */
    if (eMsgType == RVSIP_MSG_RESPONSE && (401==statusCode || 407==statusCode))
    {
        rv = SipAuthenticatorUpdateAuthObjListFromMsg(
                                                pCallLeg->pMgr->hAuthMgr, hMsg, 
                                                pCallLeg->hPage, pCallLeg,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
                                                (ObjLockFunc)CallLegLockAPI,(ObjUnLockFunc) CallLegUnLockAPI,
#else
                                                NULL, NULL,
#endif
                                                &pCallLeg->pAuthListInfo, &pCallLeg->hListAuthObj);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegMsgEvMsgRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - failed to load Authentication data (rv=%d:%s)",
                pCallLeg, hMsg, rv, SipCommonStatus2Str(rv)));
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    /*check if it is a provisional response (1xx)*/
    if ((eMsgType == RVSIP_MSG_RESPONSE) && (statusCode < 200) &&
		SipTransactionGetSubsInfo(hTransc) == NULL)
    {
        RvSipTranscHandle       hTranscForCb = hTransc;
        CallLegInvite          *pInvite = NULL;


        /*check if it is an INVITE or RE-INVITE or general transaction*/
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE) 
        {
            /* case: INVITE*/
            if ((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING) ||
                (pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING )) 
            {  
                /*hTransc & hInvite are NULL*/
                hTranscForCb = NULL;
            }
            else  /*case:it's a ReInvite */
            {  
                /*find CInvite object (hActiveTransc = NULL)*/
                rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
                if(rv != RV_OK || pInvite == NULL)
                {
                    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegMsgEvMsgRcvdHandler - Call-leg 0x%p - Failed to find invite obj for transc 0x%p (rv=%d)",
                        pCallLeg,hTransc,rv));
                }
            }
        }
        /*make sure it is not an old invite*/
        if (pCallLeg->pMgr->bOldInviteHandling == RV_TRUE) 
        {
            pInvite = NULL;
        }
        /*notify the application of a provisional response received event*/
        CallLegCallbackProvisionalResponseRcvdEv( 
            pCallLeg,
            hTranscForCb,
            pInvite,
            hMsg);
    }

    
#ifdef RV_SIP_SUBS_ON
    /* Get refer and notify information from message */
    if(bAppTransc == RV_FALSE &&
       (eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethod == SIP_TRANSACTION_METHOD_REFER     ||
        eMethod == SIP_TRANSACTION_METHOD_NOTIFY))
    {
        LoadFromSubsRcvdMsg(pCallLeg,hTransc,hMsg,eMsgType,eMethod);
    }
    NotifyReferGeneratorIfNeeded(pCallLeg,hMsg,eMsgType,eMethod);
#endif /* #ifdef RV_SIP_SUBS_ON */
	
#ifdef RV_SIP_IMS_ON 
	rv = CallLegSecAgreeMsgRcvd(pCallLeg, hTransc, hMsg);
	if (RV_OK != rv)
	{
		/* The call-leg was terminated within the security-agreement layer */
		RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					 "CallLegMsgEvMsgRcvdHandler - Call-Leg 0x%p terminated. Msg 0x%p will not be handled",
					 pCallLeg, hMsg));
		return rv;
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    return RV_OK;
}

/***************************************************************************
* CallLegMsgForkedInviteResponseRcvdHandler
* ------------------------------------------------------------------------
* General: Handle a message received event. when the call-leg was created with
*          no transaction (two responses were received after forking)
*          The function handles only response on INVITE messages.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg - The call leg the transaction belongs to.
*        hMsg     - Handle to the message that was received.
***************************************************************************/
RvStatus RVCALLCONV CallLegMsgForkedInviteResponseRcvdHandler(
                                    IN  CallLeg*        pCallLeg,
                                    IN  RvSipMsgHandle  hMsg)

{
    RvStatus     rv         = RV_OK;
    RvSipMsgType eMsgType   = RvSipMsgGetMsgType(hMsg);
    RvInt16      statusCode = RvSipMsgGetStatusCode(hMsg);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
          "CallLegMsgForkedInviteResponseRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - received a message",
          pCallLeg,hMsg));

    rv = CallLegCallbackMsgReceivedEv((RvSipCallLegHandle)pCallLeg,hMsg);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
          "CallLegMsgForkedInviteResponseRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - status %d returned from application",
          pCallLeg,hMsg,rv));
        return rv;
    }

    LoadFromInviteResponseRcvdMsg(pCallLeg,hMsg);

#ifdef RV_SIP_AUTH_ON
    /* Load the authentication headers from the message (if needed) */
    if (RVSIP_MSG_RESPONSE==eMsgType && (401==statusCode || 407==statusCode))
    {
        rv = SipAuthenticatorUpdateAuthObjListFromMsg(
                                                pCallLeg->pMgr->hAuthMgr, hMsg,
                                                pCallLeg->hPage, pCallLeg,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
                                                (ObjLockFunc)CallLegLockAPI,(ObjUnLockFunc)CallLegUnLockAPI,
#else
                                                NULL, NULL,
#endif
                                                &pCallLeg->pAuthListInfo, &pCallLeg->hListAuthObj);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegMsgForkedInviteResponseRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - failed to load Authentication data (rv=%d:%s)",
                pCallLeg, hMsg, rv, SipCommonStatus2Str(rv)));
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    /* notify the application of a provisional response received event*/
    if (statusCode < 200) 
    {
        /* hTransc does not exist in forked INVITE. */
        /*notify the application of a provisional response received event*/
        CallLegCallbackProvisionalResponseRcvdEv( 
             pCallLeg,
             NULL,
             NULL,
             hMsg);
    
    }  

#ifdef RV_SIP_SUBS_ON
    /* Get refer and notify information from message */
    NotifyReferGeneratorIfNeeded(pCallLeg,hMsg,eMsgType,SIP_TRANSACTION_METHOD_INVITE);
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_IMS_ON 
	rv = CallLegSecAgreeMsgRcvd(pCallLeg, NULL, hMsg);
	if (RV_OK != rv)
	{
		/* The call-leg was terminated within the security-agreement layer */
		RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			 "CallLegMsgForkedInviteResponseRcvdHandler - Call-Leg 0x%p terminated. Msg 0x%p will not be handled",
			 pCallLeg, hMsg));
		return rv;
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    return RV_OK;
}

/***************************************************************************
 * CallLegMsg2xxAckMsgToSendHandler
 * ------------------------------------------------------------------------
 * General: Handle a message to send event. when the call-leg was created with
 *          no transaction (two responses were received after forking)
 *          The function handles only response on INVITE messages.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The call leg the ACK message belongs to.
 *          inviteResponseCode - the response code the call-leg got, and sends
 *                     the ACK for.
 *          hMsg     - Handle to the message ready to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsg2xxAckMsgToSendHandler(
                        IN  CallLeg                *pCallLeg,
                        IN  RvUint16               inviteResponseCode,
                        IN  RvSipMsgHandle          hMsg)

{
    RvStatus              rv;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
          "CallLegMsg2xxAckMsgToSendHandler - Call-Leg 0x%p - Is about to send a message"
          ,pCallLeg));

    /*add route list to outgoing messages*/
    rv = CallLegRouteListAddTo2xxAckMessage(pCallLeg, inviteResponseCode, hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegMsg2xxAckMsgToSendHandler - Call-Leg 0x%p - Failed to add route list to message - message will not be sent"
            ,pCallLeg));
        return rv;
    }

    rv = CommonMsgToSend(NULL, pCallLeg, hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegMsg2xxAckMsgToSendHandler - Call-Leg 0x%p - Failed in CommonMsgToSend (rv=%d)"
            ,pCallLeg,rv));
        return rv;
        
    }
    rv = CallLegCallbackMsgToSendEv((RvSipCallLegHandle)pCallLeg,hMsg);
    return rv;
}

/***************************************************************************
 * CallLegMsgAckMsgRcvdHandler
 * ------------------------------------------------------------------------
 * General: Handle an ACK message Received event. 
 *          (since ACK is not related to a transaction, we must call the
 *          msg-rcvd-ev callback from here).
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - The call leg the ACK message belongs to.
 *          hMsg     - Handle to the received ACK message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMsgAckMsgRcvdHandler(
                        IN  CallLeg                *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)

{
    RvStatus              rv;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
          "CallLegMsgAckMsgRcvdHandler - Call-Leg 0x%p - received an ACK message"
          ,pCallLeg));

    rv = CallLegCallbackMsgReceivedEv((RvSipCallLegHandle)pCallLeg,hMsg);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegMsgAckMsgRcvdHandler - Call-Leg 0x%p (hMsg=0x%p) - status %d returned from application",
            pCallLeg,hMsg,rv));
        return rv;
    }

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * LoadFromInviteRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters from incoming invite request message, for
 *          example the contact address from an initial Invite.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static void RVCALLCONV LoadFromInviteRequestRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)
{
    /*-----------------------------------------------------------
      initial INVITE in call
     ------------------------------------------------------------*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE &&
       NULL	== CallLegGetActiveReferSubs(pCallLeg))
    {
        CallLegMsgLoaderLoadDialogInfo(pCallLeg,hMsg);

        return;
    }
    /*-----------------------------------------------------------
      re-INVITE in call - change the contact if needed.
     ------------------------------------------------------------*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED &&
       NULL	== CallLegGetActiveReferSubs(pCallLeg))
    {
        if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE) /* old invite */
        {
            CallLegInvite* pActiveInvite = NULL;
            CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pActiveInvite);
            if(pActiveInvite == NULL) /* no active re-invite exists */
            {
                CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
            }
            return;
        }
        else /* new invite */
        {
            if(pCallLeg->hActiveTransc == NULL)/* no active re-invite exists */
            {
                CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
            }
            return;
        }
    }
}


/***************************************************************************
 * LoadFromInviteResponseRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters from incoming Invite response message,
 *          For example in a 3xx response the contact address should be
 *          loaded.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static void RVCALLCONV LoadFromInviteResponseRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)
{
    RvStatus rv = RV_OK;
    CallLegInvite* pInvite = NULL;
    RvUint16 responseCode = RvSipMsgGetStatusCode(hMsg);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /*SPC: Update the Route List when receiving 1xx (not 100) response with Record-Route*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING) &&
        responseCode>100 && responseCode < 200)
    {
        /*build a new route list on the CallLeg page*/
        rv = CallLegRouteListInitialize(pCallLeg, hMsg);
#else
    /*initialize the route list when receiving 1xx (not 100) response*/
    /*the list will be destructed when 2xx is received*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING) &&
        responseCode>100 && responseCode < 200 &&
        pCallLeg->hRouteList == NULL)
    {
        /*build a route list only if there is no route list yet*/
        rv = CallLegRouteListProvInitialize(pCallLeg,hMsg);
#endif
/* SPIRENT_END */        
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - Failed build route list from provisional response",pCallLeg));
        }
        else
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - Route list was built from provisional response",pCallLeg));

        }
    }

    if(pCallLeg->hActiveTransc != NULL)
    {
        SipTransactionMethod eMethod;
        SipTransactionGetMethod(pCallLeg->hActiveTransc, &eMethod);
        /* if we already sent BYE, we might get response for a re-invite, but the active transaction is 
           the BYE transaction. */
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
        {
            rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
        }
    }
    else if(pCallLeg->bOriginalCall == RV_FALSE)
    {
        /* this is a forked call-leg and active transaction is not available -
           search for the initial invite object */
        rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED, &pInvite);
    }
    else
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - active transc is NULL on an original call-leg",
             pCallLeg));
        rv = RV_ERROR_NOT_FOUND;
    }
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - CallLegInviteFindObjByTransc retrieved error (rv=%d)",
             pCallLeg, rv));
    }

    /*for 3xx response, load contact address*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING) &&
       responseCode>=300 && responseCode<400)
    {
        CallLegMsgLoaderLoadContactToRedirectAddress(pCallLeg,hMsg);
    }
    else
    {
        /*the following conditions handles this problem:
        1xx is received with contact header - change the call-leg remote contact.
        401 is received without contact header - the next invite will be sent to
        the 1xx contact header, and not to the original remote contact as it should.
        solution: after 1xx - save the original contact and the 1xx contact.
        when final response is received - set the remote contact to be the original
        contact (in case of non 2xx) or the message contact (in case of 2xx),
        and free the spare contact page */
        RvBool bContactExistInMsg = IsContactExistInMsg(hMsg);

        if(responseCode >= 100 && responseCode < 200 && bContactExistInMsg == RV_TRUE)/*1xx*/
        {
            /* 1. save the call-leg original remote contact - only for the first 1xx!*/
            if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING || 
                (pInvite != NULL && pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT)) ||
               ((pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING ||
                 (pInvite != NULL && pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING)) 
               && pCallLeg->hOriginalContactPage == NULL))
            {
                if(pCallLeg->hOriginalContactPage != NULL)
                {
                    RPOOL_FreePage(pCallLeg->pMgr->hElementPool, pCallLeg->hOriginalContactPage);
                    pCallLeg->hOriginalContactPage = NULL;
                    pCallLeg->hOriginalContact = NULL;
                }
                pCallLeg->hOriginalContactPage = pCallLeg->hRemoteContactPage;
                pCallLeg->hOriginalContact = pCallLeg->hRemoteContact;
                pCallLeg->hRemoteContactPage = NULL;
                pCallLeg->hRemoteContact = NULL;
            }
            /* 2. save the remote contact from 1xx in call-leg object */
            CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
        }
        else if(responseCode >= 200 && responseCode < 300 && bContactExistInMsg == RV_TRUE)/*2xx*/
        {
            /*1. free the original remote contact */
            pCallLeg->hOriginalContact = NULL;
            if(pCallLeg->hOriginalContactPage != NULL)
            {
                RPOOL_FreePage(pCallLeg->pMgr->hElementPool, pCallLeg->hOriginalContactPage);
                pCallLeg->hOriginalContactPage = NULL;
            }
            /* 2. save the remote contact from 1xx in call-leg object */
            CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
        }
        else /*3xx-6xx*/
        {
            /* set the remote contact to the original remote contact */
           if(pCallLeg->hOriginalContactPage != NULL)
           {
               if(pCallLeg->hRemoteContactPage != NULL)
               {
                    RPOOL_FreePage(pCallLeg->pMgr->hElementPool,
                                   pCallLeg->hRemoteContactPage);
               }
               pCallLeg->hRemoteContactPage = pCallLeg->hOriginalContactPage;
               pCallLeg->hRemoteContact = pCallLeg->hOriginalContact;
               pCallLeg->hOriginalContact = NULL;
               pCallLeg->hOriginalContactPage = NULL;
           }
           /* else - there was no 1xx response that could have change the remote contact,
              nothing to do */
        }
    }
    /*initialize the route list for an initial Invite 200 response.
      this happens only once. The route list will not change in responses to
      re-Invite*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLING) &&
       responseCode>=200 && responseCode<300)
    {
        rv = CallLegRouteListInitialize(pCallLeg,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
        }
        else
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "LoadFromInviteResponseRcvdMsg - Call-Leg 0x%p - Route list was built from 2xx response",pCallLeg));
        }

    }
}



/***************************************************************************
 * AddLocalContactToMsg
 * ------------------------------------------------------------------------
 * General: Adds a contact header to outgoing message. The contact is taken
 *          from the call-leg object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddContactHeaderToMsg(
                                 IN  CallLeg                 *pCallLeg,
                                 IN  RvSipMsgHandle          hMsg)
  {

    RvSipAddressHandle hLocalContactAddress = NULL;
    RvSipContactHeaderHandle hContactHeader = NULL;
    RvStatus rv;
    /*take the contact address from the call-leg*/
    rv = CallLegGetLocalContactAddress(pCallLeg,&hLocalContactAddress);
    if(rv != RV_OK && hLocalContactAddress == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddContactHeaderToMsg - Call-Leg 0x%p - Failed for msg 0x%p, no contact available"
                  ,pCallLeg,hMsg));
        return RV_ERROR_UNKNOWN;
    }
    /*construct contact in message*/
    rv = RvSipContactHeaderConstructInMsg(hMsg, RV_FALSE, &hContactHeader);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddContactHeaderToMsg - Call-Leg 0x%p - Failed to construct contact in msg 0x%p, (rv=%d)"
                  ,pCallLeg,hMsg,rv));
        return RV_ERROR_UNKNOWN;

    }
    /*set address spec in contact*/
    rv = RvSipContactHeaderSetAddrSpec(hContactHeader,hLocalContactAddress);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddContactHeaderToMsg - Call-Leg 0x%p - Failed to set address spec in contact for msg 0x%p, (rv=%d)"
                     ,pCallLeg,hMsg,rv));
        return RV_ERROR_UNKNOWN;
    }
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddContactHeaderToMsg - Call Leg 0x%p ,Contact header was added to msg 0x%p successfully"
                     ,pCallLeg,hMsg));
    return RV_OK;
}

/***************************************************************************
 * AddContactHeaderToOutgoingMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: checks whether to add a contact header to the outgoing message
 *          and add it it needed.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg object.
 *          hTransc  - Handle to the transaction.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddContactHeaderToOutgoingMsgIfNeeded(
                        IN  CallLeg                 *pCallLeg,
                        IN    RvSipTranscHandle       hTransc,
                        IN  RvSipMsgHandle            hMsg)
{

    RvStatus             rv          = RV_OK;
    RvSipMsgType         eMsgType    = RvSipMsgGetMsgType(hMsg);
    RvBool               bAddContact = RV_FALSE;
    SipTransactionMethod eMethod;

    if(NULL == hTransc)
    {
        eMethod = SIP_TRANSACTION_METHOD_INVITE;
    }
    else
    {
        SipTransactionGetMethod(hTransc,&eMethod);
    }

    if (eMsgType == RVSIP_MSG_REQUEST)
    {
        switch (eMethod)
        {
        case SIP_TRANSACTION_METHOD_INVITE:
        case SIP_TRANSACTION_METHOD_OTHER:
#ifdef RV_SIP_SUBS_ON
		case SIP_TRANSACTION_METHOD_REFER:
        case SIP_TRANSACTION_METHOD_SUBSCRIBE:
        case SIP_TRANSACTION_METHOD_NOTIFY:
        
#endif /* #ifdef RV_SIP_SUBS_ON */
            bAddContact = RV_TRUE;
            break;
        default:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "AddContactHeaderToOutgoingMsgIfNeeded - Call-Leg 0x%p - unexpected method %d for Request message"
                       ,pCallLeg, eMethod));
            return RV_OK;
        }
    }
    else if (eMsgType == RVSIP_MSG_RESPONSE)
    {
        RvInt16 statusCode = RvSipMsgGetStatusCode(hMsg);
        switch (eMethod)
        {
        case SIP_TRANSACTION_METHOD_INVITE:
        case SIP_TRANSACTION_METHOD_OTHER:
#ifdef RV_SIP_SUBS_ON
        case SIP_TRANSACTION_METHOD_SUBSCRIBE:
        case SIP_TRANSACTION_METHOD_NOTIFY:
#endif /* #ifdef RV_SIP_SUBS_ON */
            /*add the contact to 1xx,2xx,3xx and 485 reponses*/
            if(statusCode < 400 || statusCode == 485)
            {
                bAddContact = RV_TRUE;
            }
            break;
#ifdef RV_SIP_SUBS_ON
        case SIP_TRANSACTION_METHOD_REFER:
            if (statusCode > 200)
            {
                bAddContact = RV_TRUE;
            }
            break;
#endif /*#ifdef RV_SIP_SUBS_ON*/
        case SIP_TRANSACTION_METHOD_PRACK:
            if ((statusCode >= 300 && statusCode < 400) || statusCode == 485)
            {
                bAddContact = RV_TRUE;
            }
        default:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "AddContactHeaderToOutgoingMsgIfNeeded - Call-Leg 0x%p - unexpected method %d for Response message"
                       ,pCallLeg, eMethod));
            return RV_OK;
        }
    }

    if (bAddContact == RV_FALSE)
    {
        return RV_OK;
    }

    rv = AddContactHeaderToMsg(pCallLeg,hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "AddContactHeaderToOutgoingMsgIfNeeded - Call-Leg 0x%p - Failed to add contact - message will not be sent"
            ,pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;

}

/***************************************************************************
 * IsContactExistInMsg
 * ------------------------------------------------------------------------
 * General: checks if a legal contact header exists in th egiven message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg     - Handle to the message.
 ***************************************************************************/
static RvBool RVCALLCONV IsContactExistInMsg(IN  RvSipMsgHandle hMsg)
{

    RvSipHeaderListElemHandle   hPos = NULL;
    RvSipContactHeaderHandle    hContact;
    RvSipAddressHandle          hContactAddress;

    /*get the contact header from the message*/
    hContact = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType(
                                             hMsg,
                                             RVSIP_HEADERTYPE_CONTACT,
                                             RVSIP_FIRST_HEADER, &hPos);
    if(hContact == NULL)
    {
        return RV_FALSE;
    }
    /*get the address spec from the contact header and set it in the call-leg*/
    hContactAddress = RvSipContactHeaderGetAddrSpec(hContact);
    if(hContactAddress == NULL)
    {
        return RV_FALSE;
    }

    return RV_TRUE;
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * LoadFromSubsRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load all the data from a received message, which is related
 *          to SUBSCRIBE/REFER.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Pointer to the CallLeg, which received the message.
 *          hMsg     - Handle to the message.
 *          eMsgType - The received message type (REQUEST/RESPONSE).
 *          eMethod  - The received message method.
 ***************************************************************************/
static void RVCALLCONV LoadFromSubsRcvdMsg(
                                        IN CallLeg             *pCallLeg,
                                        IN RvSipTranscHandle    hTransc,
                                        IN RvSipMsgHandle       hMsg,
                                        IN RvSipMsgType         eMsgType,
                                        IN SipTransactionMethod eMethod)
{
    switch (eMethod)
    {
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
    case SIP_TRANSACTION_METHOD_REFER:
        (eMsgType == RVSIP_MSG_REQUEST) ?
            CallLegSubsLoadFromRequestRcvdMsg(pCallLeg,hMsg) :
            CallLegSubsLoadFromResponseRcvdMsg(pCallLeg,hMsg);
        break;
    case SIP_TRANSACTION_METHOD_NOTIFY:
        (eMsgType == RVSIP_MSG_REQUEST) ?
            CallLegSubsLoadFromNotifyRequestRcvdMsg(pCallLeg, hTransc, hMsg) :
            CallLegSubsLoadFromNotifyResponseRcvdMsg(pCallLeg, hMsg);
            break;
    default:
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "LoadFromSubsRcvdMsg - Call-Leg 0x%p (hMsg=0x%p) - method is not SUBSCRIBE/REFER/NOTIFY",
            pCallLeg,hMsg));
        return;
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * NotifyReferGeneratorIfNeeded
 * ------------------------------------------------------------------------
 * General: Notify the refer generator (if exists) about the call-leg
 *          received a final response.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - pointer to the call leg.
 *          hMsg     - Handle to received message.
 *          eMsgType - The received message type (REQUEST/RESPONSE).
 *          eMethod  - The received message method.
 * Output:  (-)
 ***************************************************************************/
static void RVCALLCONV NotifyReferGeneratorIfNeeded(
                                    IN CallLeg             *pCallLeg,
                                    IN RvSipMsgHandle       hMsg,
                                    IN RvSipMsgType         eMsgType,
                                    IN SipTransactionMethod eMethod)
{
    if(eMsgType == RVSIP_MSG_RESPONSE &&
      (eMethod  == SIP_TRANSACTION_METHOD_INVITE))
    {
        RvInt16 responseCode = RvSipMsgGetStatusCode(hMsg);
        if(responseCode != 401 && responseCode != 407 &&
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
           responseCode != 503 &&
#endif
           (responseCode < 300 || responseCode >= 400))
        {
            /* we inform the refer generator of a response, only in cases that this
               is a 'final response', in terms it won't cause a second attempt to
               connect the call */
            CallLegReferNotifyReferGenerator(pCallLeg,
                RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNDEFINED, hMsg);
        }
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /*RV_SIP_PRIMITIVES */

