
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
 *                              <CallLegAuth.c>
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifdef  RV_SIP_AUTH_ON
#ifndef RV_SIP_PRIMITIVES

#include "CallLegAuth.h"
#include "CallLegRefer.h"
#include "CallLegCallbacks.h"
#include "CallLegInvite.h"
#include "_SipMsg.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticator.h"
#include "_SipSubs.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "_SipAuthenticationHeader.h"
#include "AuthenticatorObject.h"
#endif
/* SPIRENT_END */


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void RVCALLCONV GetCallLegObjectInfo(
                            IN  CallLeg                    *pCallLeg,
                            OUT void                      **phObject,
                            OUT RvSipCommonStackObjectType *peObjectType,
                            OUT SipTripleLock             **ppObjTripleLock);

static RvStatus RVCALLCONV AuthNotifyReferGeneratorIfNeeded(
                            IN  CallLeg           *pCallLeg,
                            IN  RvSipTranscHandle  hTransc);

/*-----------------------------------------------------------------------*/
/*                           MODULE  FUNCTIONS                           */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegAuthAddAuthorizationHeadersToMsg
 * ------------------------------------------------------------------------
 * General: Adds authorization header to outgoing message. The authorization
 *          header was built from the authentication header parameters.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Pointer to the call-leg.
 *          hTransc  - the transaction responsible for the message.
 *          hMsg     - Handle to the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthAddAuthorizationHeadersToMsg(
                                 IN  CallLeg                 *pCallLeg,
                                 IN  RvSipTranscHandle       hTransc,
                                 IN  RvSipMsgHandle          hMsg)
{
    RvStatus                        rv;
    RvSipMethodType                 eRequestMethod = RvSipMsgGetRequestMethod(hMsg);
    void*                           hObject; /* handle to the Object, messages
                                             of which has to be authenticated
                                             (CallLeg or Subscription) */
    RvSipCommonStackObjectType      eObjectType; /* Type of the object */
    SipTripleLock                  *pObjTripleLock; /*Triple Lock of the object
                                    It has to be unlocked before code control
                                    passes to the Application */
    CallLegInvite*                  pInvite = NULL;

    if(pCallLeg->bAddAuthInfoToMsg == RV_FALSE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegAuthAddAuthorizationHeadersToMsg - Call-Leg 0x%p - No info will be added, bAddAuthInfoToMsg=%d"
                ,pCallLeg,pCallLeg->bAddAuthInfoToMsg));
        return RV_OK;
    }
    if(RVSIP_METHOD_ACK == eRequestMethod || 
	   RVSIP_METHOD_INVITE == eRequestMethod)
    {
		
		RvSipCSeqHeaderHandle hCSeq = NULL;
		RvInt32 cseqStep;
		hCSeq = RvSipMsgGetCSeqHeader(hMsg);
		SipCSeqHeaderGetInitializedCSeq(hCSeq,&cseqStep);
		rv = CallLegInviteFindObjByCseq(pCallLeg, cseqStep, 
			                            RVSIP_CALL_LEG_DIRECTION_OUTGOING,&pInvite);
		
        if(rv != RV_OK || pInvite == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegAuthAddAuthorizationHeadersToMsg - Call-Leg 0x%p, Transc 0x%p - Failed to get Invite object (rv=%d:%s)",
                pCallLeg, hTransc, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Handle ACK message */
    if(eRequestMethod == RVSIP_METHOD_ACK && NULL != pInvite)
    {
        RvUint16 responseCode;
        /* Get respose code */
        if(NULL == hTransc)
        {/* if there is no transaction, the case must be ACK on 2xx with new
            invite behavior */
            responseCode = 200;
        }
        else
        {
            RvSipTransactionGetResponseCode(hTransc,&responseCode);
        }
        /* Add credentials, if exist, only to ACK for 2xx response */
        if(responseCode >= 200 && responseCode < 300 &&
           NULL != pInvite->hListAuthorizationHeaders)
        {
            rv = SipAuthenticatorInviteMoveAuthorizationListIntoMsg(
                pCallLeg->pMgr->hAuthMgr, hMsg, pInvite->hListAuthorizationHeaders);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegAuthAddAuthorizationHeadersToMsg - Call-Leg 0x%p - Failed to set authorization headers 0x%x into ACK (rv=%d:%s)",
                    pCallLeg, pInvite->hListAuthorizationHeaders, rv,
                    SipCommonStatus2Str(rv)));
                return rv;
            }
            pInvite->hListAuthorizationHeaders = NULL;
        }
        return RV_OK;
    }

    /* Determine type of the Object, messages of which has to be authenticated,
       and get it's handle */
    GetCallLegObjectInfo(pCallLeg,&hObject,&eObjectType,&pObjTripleLock);

    /* Build the Authorization headers in the Msg */
    rv = SipAuthenticatorBuildAuthorizationListInMsg(
        pCallLeg->pMgr->hAuthMgr, hMsg, hObject, eObjectType, pObjTripleLock,
        pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegAuthAddAuthorizationHeadersToMsg - Call-Leg 0x%p - Failed to build authorization headers (rv=%d:%s)",
            pCallLeg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* If the Authorization headers were built in the INVITE, save them into
    the CallLeg's Invite object to be set later into the correspondent ACK. */
    if(eRequestMethod == RVSIP_METHOD_INVITE && NULL != pInvite)
    {
        rv = SipAuthenticatorInviteLoadAuthorizationListFromMsg(
                                        pCallLeg->pMgr->hAuthMgr,hMsg,
										&pInvite->hListAuthorizationHeaders);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegAuthAddAuthorizationHeadersToMsg - Call-Leg 0x%p - Failed to load authorization headers from Msg 0x%p to the List 0x%p (rv=%d:%s)",
                pCallLeg, hMsg, pInvite->hListAuthorizationHeaders, rv,
                SipCommonStatus2Str(rv)));
            return rv;
        }
    } /* ENDOF: if(eRequestMethod == RVSIP_METHOD_INVITE) */

    return RV_OK;
}

/***************************************************************************
 * CallLegAuthHandleUnAuthResponse
 * ------------------------------------------------------------------------
 * General: The function handle 401/407 response message on INVITE or BYE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hTransc  - Handle to the transaction.
 *            statusCode - 401/407.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthHandleUnAuthResponse(
                                IN  CallLeg             *pCallLeg,
                                IN  RvSipTranscHandle   hTransc)
{
    RvStatus rv;
    CallLegInvite* pInvite = NULL;
    SipTransactionMethod eMethod;
    RvBool   bValid = RV_FALSE;

    SipTransactionGetMethod(hTransc, &eMethod);

    if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
    {
        rv = CallLegInviteFindObjByTransc(pCallLeg, hTransc, &pInvite);
        if (rv != RV_OK || NULL == pInvite)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegAuthHandleUnAuthResponse: call-leg 0x%p - Failed to find invite object",
                pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* checking if the parameters inside the authentication header are supported */
    rv = SipAuthenticatorCheckValidityOfAuthList(
        pCallLeg->pMgr->hAuthMgr, pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj, &bValid);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegAuthHandleUnAuthResponse: call-leg 0x%p - Failed to check validity of authentication data 0x%x (rv=%d:%s)",
            pCallLeg, pCallLeg->hListAuthObj, rv, SipCommonStatus2Str(rv)));
    }

    /* the Authentication validity check failed, so we disconnect the call */
    if (RV_FALSE==bValid)
    {
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
        {
            /*make the transaction a stand alone transaction to let is keep
              retransmit ACK without the call-leg existence*/
            rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hTransc);
            if (rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegAuthHandleUnAuthResponse: call-leg 0x%p - Failed to detach from transc 0x%p",
                    pCallLeg, hTransc));
                return rv;
            }
            rv = AuthNotifyReferGeneratorIfNeeded(pCallLeg,hTransc);
            if (rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegAuthHandleUnAuthResponse: call-leg 0x%p - Failed to notify refer generator. transc 0x%p",
                    pCallLeg, hTransc));
                return rv;
            }
        }

        /*Disconnect the call*/
        CallLegDisconnectWithNoBye(
                    pCallLeg, RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS);

        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    }

    pCallLeg->eAuthenticationOriginState = pCallLeg->eState;
    pCallLeg->bInviteReSentInCB = RV_FALSE;

    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_UNAUTHENTICATED,
                                RVSIP_CALL_LEG_REASON_AUTH_NEEDED);
	/*  If application resent the invite synchronically, then
        CallLegInviteDetachInviteTransc was already called in the Connect function,
        inside the callback. Otherwise, we should do it here. */
    if(pCallLeg->bInviteReSentInCB == RV_FALSE && pInvite != NULL)
    {
        CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hTransc);
    }
	SipCommonCSeqReset(&pCallLeg->remoteCSeq); 
    return RV_OK;
}

/***************************************************************************
 * CallLegAuthRespondUnauthenticated - server function
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given authentication header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAuthRespondUnauthenticated(
                                   IN  CallLeg*             pCallLeg,
                                   IN  RvSipTranscHandle    hTransaction,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader,
                                   IN  RvChar              *strRealm,
                                   IN  RvChar              *strDomain,
                                   IN  RvChar              *strNonce,
                                   IN  RvChar              *strOpaque,
                                   IN  RvBool              bStale,
                                   IN  RvSipAuthAlgorithm   eAlgorithm,
                                   IN  RvChar              *strAlgorithm,
                                   IN  RvSipAuthQopOption   eQop,
                                   IN  RvChar              *strQop)
{
    RvStatus    rv;
    RvSipTranscHandle hTransc;
    CallLegInvite*      pInvite = NULL;
    RvSipCallLegState eState = pCallLeg->eState;
    RvSipCallLegModifyState eModifyState = RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED;

    RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
              "CallLegAuthRespondUnauthenticated - CallLeg 0x%p: about to send %d response",
              pCallLeg, responseCode));

    
    hTransc = hTransaction;

    if(hTransaction == NULL || hTransaction == pCallLeg->hActiveTransc) /* this is not a general transc */
    {
        SipTransactionMethod eMethod;
        SipTransactionGetMethod(pCallLeg->hActiveTransc, &eMethod);

        hTransc = pCallLeg->hActiveTransc;
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
        {
            rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
            if(rv != RV_OK || pInvite == NULL)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegAuthRespondUnauthenticated - call-leg 0x%p - Failed to find invite object.",
                    pCallLeg));
                return RV_ERROR_UNKNOWN;
            }
            eModifyState = pInvite->eModifyState;
        }
    }
    if(hTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegAuthRespondUnauthenticated - Failed. Active transaction is NULL for call-leg 0x%p",
               pCallLeg));

        /* for these states, if hTransc is NULL, then the active transaction is NULL,
        and call must be terminated */
        if(eState == RVSIP_CALL_LEG_STATE_OFFERING ||
           eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegAuthRespondUnauthenticated - Call-leg 0x%p - Terminating call.",
            pCallLeg));

            CallLegDisconnectWithNoBye(pCallLeg,
                                       RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);;
            return RV_ERROR_UNKNOWN;
        }
        else
        {
            /* for all other states, the call should not be terminaed. */
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegAuthRespondUnauthenticated - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    /* 1. send the response */
    if(hHeader != NULL)
    {
         rv = RvSipTransactionRespondUnauthenticated(hTransc,
                                                    responseCode,
                                                    strReasonPhrase,
                                                    headerType,
                                                    hHeader);
    }
    else
    {
        rv = RvSipTransactionRespondUnauthenticatedDigest(hTransc, responseCode, strReasonPhrase,
                                                      strRealm, strDomain, strNonce, strOpaque,
                                                      bStale, eAlgorithm, strAlgorithm, eQop, strQop);
    }
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegAuthRespondUnauthenticated -  Call-leg 0x%p - Failed to send %d response (rv=%d)",
                pCallLeg, responseCode,rv));
        /* continue with the function - terminate the call if needed */
    }

    /* 2. update the state machine */
    if(RVSIP_CALL_LEG_STATE_OFFERING == eState ||
       RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD == eModifyState) /* invite or re-inview */
    {
        /* if response fail, we have to terminate the call */
        if (rv != RV_OK)
        {
            /*transaction failed to send respond*/
            CallLegDisconnectWithNoBye(pCallLeg,
                                       RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);;
            return RV_ERROR_UNKNOWN;
        }

        /* update the invite state machine */
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING)
        {
            /* change the call leg state to disconnected*/
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                        RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
        }
        else if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED||
                (pCallLeg->pMgr->bOldInviteHandling == RV_FALSE &&
                 pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED))
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                                     RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                                     RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
        }

        return RV_OK;
    }

    return rv;

}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * GetCallLegObjectInfo
 * ------------------------------------------------------------------------
 * General: Retrieve information about the Call-Leg obj (pointer to the obj,
 *          it's type and a pointer to it's triple lock). The Call-Leg obj
 *          might be simply a CallLeg or Subscription.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - A pointer to the Call-Leg
 * Output:  phObject        - Pointer to the obj handle.
 *          peObjectType    - Pointer to the obj type.
 *          ppObjTripleLock - Pointer to the object triple lock structure.
 ***************************************************************************/
static void RVCALLCONV GetCallLegObjectInfo(
                            IN  CallLeg                    *pCallLeg,
                            OUT void                      **phObject,
                            OUT RvSipCommonStackObjectType *peObjectType,
                            OUT SipTripleLock             **ppObjTripleLock)
{
    if (RV_FALSE == CallLegGetIsHidden(pCallLeg) ||
        (RV_TRUE == CallLegGetIsHidden(pCallLeg) && 
		 RV_TRUE == CallLegGetIsRefer(pCallLeg)))
        /* CallLeg is the object */
    {
        *phObject        = (void*)pCallLeg;
        *peObjectType    = RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG;
        *ppObjTripleLock = pCallLeg->tripleLock;
    }
#ifdef RV_SIP_SUBS_ON
    else /* Subscription is the object. */
    {
       /* If CallLeg was created for Subscription, it is hidden,
        and it has only one handle in the list of Subscription handles. */
        RLIST_ITEM_HANDLE   listElem;
        RLIST_GetHead (NULL, pCallLeg->hSubsList, &listElem);

        *phObject = (void*)(*(RvSipSubsHandle*)listElem);
        *peObjectType = RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION;
        *ppObjTripleLock = SipSubsGetTripleLock(*(RvSipSubsHandle*)listElem);
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * AuthNotifyReferGeneratorIfNeeded
 * ------------------------------------------------------------------------
 * General: Notifes the ReferGenerator about an UnAuth response in case
 *          of REFER CallLeg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - A pointer to the Call-Leg
 *          hTransc    - Handle to transaction that is related to the
 *                       unAuth response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV AuthNotifyReferGeneratorIfNeeded(
                                        IN  CallLeg           *pCallLeg,
                                        IN  RvSipTranscHandle  hTransc)
{
#ifdef RV_SIP_SUBS_ON
    if(pCallLeg->hReferSubsGenerator != NULL)
    {
        RvStatus       rv;
        RvSipMsgHandle hReceivedMsg;

        rv = RvSipTransactionGetReceivedMsg(hTransc, &hReceivedMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AuthNotifyReferGeneratorIfNeeded: call-leg 0x%p - Failed to get received message",
                pCallLeg));
            return rv;
        }

        rv = CallLegReferNotifyReferGenerator(pCallLeg,
                RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNSUPPORTED_AUTH_PARAMS, hReceivedMsg);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AuthNotifyReferGeneratorIfNeeded: call-leg 0x%p - Failed to notify refer generator",
                pCallLeg));
            return rv;
        }
    }
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
	RV_UNUSED_ARG(hTransc)
#endif /* #ifdef RV_SIP_SUBS_ON */
    return RV_OK;
}

#endif /* #ifndef RV_SIP_PRIMITIVES  */

#endif /* #ifdef RV_SIP_AUTH_ON */
