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
 *                              <_SipSubs.c>
 *
 * This file contains implementation for the subscription internal API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "SubsObject.h"
#include "SubsMgrObject.h"
#include "SubsNotify.h"
#include "_SipSubs.h"
#include "SubsRefer.h"
#include "SubsCallbacks.h"
#include "SubsHighAvailability.h"
#include "RvSipEventHeader.h"
#include "_SipEventHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipCallLeg.h"
#include "_SipCallLeg.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV SubsHandleSubscribeRequest(Subscription*     pSubs,
                                                       RvSipTranscHandle hTransc,
                                                       SipTransactionMethod eMethod,
                                                       RvSipMsgHandle    hMsg);

static RvStatus RVCALLCONV SubsNotifyHandleRequest(IN  Notification*     pNotify,
                                                    IN  RvSipTranscHandle hTransc,
                                                    IN  RvSipMsgHandle    hMsg);

static RvStatus RVCALLCONV SubsHandleSubscribeResponse(
                                            IN  Subscription*  pSubs,
                                            IN  RvSipMsgHandle hMsg,
                                            IN  RvUint16       responseCode,
                                            IN  RvBool        bRefer);

static RvStatus RVCALLCONV SubsNotifyHandleResponse(IN  Notification*     pNotify,
                                                    IN  RvSipMsgHandle    hMsg,
                                                    IN  RvSipTranscHandle hNotifyTransc,
                                                     IN  RvInt16          responseCode);

static RvStatus RVCALLCONV SubsHandleSubscribeFinalResponseSent(
                                  IN Subscription*                     pSubs,
                                  IN RvSipTranscHandle                 hTransc,
                                  IN RvSipTransactionStateChangeReason eStateReason);

static RvStatus RVCALLCONV SubsNotifyHandleFinalResponseSent(
                                  IN  Notification*                     pNotify,
                                  IN  RvSipTranscHandle                 hTransc,
                                  IN RvSipTransactionStateChangeReason  eStateReason);

static RvStatus RVCALLCONV SubsTranscTerminatedEvHandling(
                            IN  RvSipCallLegHandle  hCallLeg,
                            IN    RvSipTranscHandle   hTransc,
                            IN  SipTransactionMethod eMethodType,
                            IN  RvSipTransactionStateChangeReason eStateReason,
                            IN  void*               pObj,
                            OUT RvBool             *bWasMsgHandled);

static RvStatus RVCALLCONV SubsHandleSubsTranscTerminated(
                               IN    RvSipTranscHandle   hTransc,
                               IN  RvSipSubsStateChangeReason eReason,
                               IN  void*               pObj);

static RvStatus RVCALLCONV SubsHandleNotifyTranscTerminated(
                                             IN    RvSipTranscHandle   hTransc,
                                             IN  RvSipSubsNotifyReason eReason,
                                             IN  void*               pObj);
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static void RVCALLCONV SubsHandleSubscribeMsgSendFailure(
                              IN  Subscription*                     pSubs,
                              IN  RvSipTranscHandle                 hTransc,
                              IN  RvSipTransactionStateChangeReason eStateReason);

static void RVCALLCONV SubsHandleNotifyMsgSendFailure(
                              IN  Notification*                      pNotify,
                              IN  RvSipTranscHandle                 hTransc,
                              IN  RvSipTransactionStateChangeReason eStateReason);

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
#endif /* #ifdef RV_SIP_SUBS_ON  - static functions */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SipSubsSetSubsType
* ------------------------------------------------------------------------
* General: The function sets the subscription type. (This function
*          is used by high-availability feature).
* Return Value: RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
*        eType - Type of subscription.
***************************************************************************/
RvStatus RVCALLCONV SipSubsSetSubsType(IN RvSipSubsHandle       hSubs,
                                       IN RvSipSubscriptionType eType)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;

    pSubs->eSubsType = eType;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hSubs);
	RV_UNUSED_ARG(eType);
	
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsSetSubsCurrentState
* ------------------------------------------------------------------------
* General: The function sets the subscription current state. (This function
*          is used by high-availability feature).
* Return Value: RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
*        eState - State of subscription.
***************************************************************************/
RvStatus RVCALLCONV SipSubsSetSubsCurrentState(RvSipSubsHandle hSubs,
                                               RvSipSubsState  eState)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;

    pSubs->eState = eState;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hSubs);
	RV_UNUSED_ARG(eState);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsGetTransc
* ------------------------------------------------------------------------
* General: The function gets the subscription transaction handle
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipSubsGetTransc(RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;

    return pSubs->hActiveTransc;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hSubs);

    return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsNotifyGetTransc
* ------------------------------------------------------------------------
* General: The function gets the notification transaction handle
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvSipTranscHandle RVCALLCONV SipSubsNotifyGetTransc(RvSipNotifyHandle hNotify)
{
#ifdef RV_SIP_SUBS_ON
    Notification* pNotify = (Notification*)hNotify;

    return pNotify->hTransc;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hNotify); 

    return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsReferGetFinalStatus
* ------------------------------------------------------------------------
* General: The function gets the refer final status value,
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvUint16 RVCALLCONV SipSubsReferGetFinalStatus(RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;

    return pSubs->pReferInfo->referFinalStatus;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hSubs);

    return (RvUint16)UNDEFINED;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsReferGetFinalStatus
* ------------------------------------------------------------------------
* General: The function gets the refer final status value,
* Return Value: hTransc.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - Handle to the subscription
***************************************************************************/
RvStatus RVCALLCONV SipSubsReferGetEventId(IN  RvSipSubsHandle hSubs,
                                           OUT RvInt32*        pEventId)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;
    RvChar       strEventId[15];
    RvUint       len;
    RvInt32      eventId;

    if(pSubs->pReferInfo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    if(RvSipEventHeaderGetEventId(pSubs->hEvent, strEventId, 15, &len) != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SipSubsReferGetEventId: subs 0x%p - Failed to get the event id", pSubs));
        return RV_ERROR_NOT_FOUND;
    }
    sscanf(strEventId, "%d", &eventId);
    *pEventId = eventId;
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    *pEventId = UNDEFINED;
	RV_UNUSED_ARG(hSubs);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SipSubsGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of state of a given subscription.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs -
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetStateNameOfSubs (IN  RvSipSubsHandle  hSubs)
{
#ifdef RV_SIP_SUBS_ON
    if(NULL == hSubs)
    {
        return "Undefined (Null)";
    }
    return SubsGetStateName(((Subscription*)hSubs)->eState);
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hSubs);

    return "ERROR (Not Supported)";
#endif /* #ifdef RV_SIP_SUBS_ON */
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/***************************************************************************
 * SipSubsSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Set the subscription outbound msg.
 *          The function is used from call-leg layer for refer subscription,
 *          when we know that no one else knows about this subscription.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - The subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsSetOutboundMsg(RvSipSubsHandle hSubs,
                                          RvSipMsgHandle  hMsg)
{
#ifdef RV_SIP_SUBS_ON
    Subscription* pSubs = (Subscription*)hSubs;

    if(pSubs->hOutboundMsg != NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    pSubs->hOutboundMsg = hMsg;
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(hMsg);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}
/***************************************************************************
* SipSubsHandleTranscChangeState
* ------------------------------------------------------------------------
* General: Handle a transaction state change.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:hTransc      - Handle to the transaction whos state was changed.
*       hCallLeg     - The call-leg this transaction belongs to.
*       eTranscState - The new state of the transaction
*       eStateReason - The reason for the change of state.
*       eMethod      - SUBSCRIBE or NOTIFY
*       pObj         - Pointer to the subscription or notification object,
*                      saved inside the transaction.
* Output:bWasHandled - Was the state handled by this function or not.
***************************************************************************/
RvStatus RVCALLCONV SipSubsHandleTranscChangeState(
                    IN  RvSipTranscHandle                 hTransc,
                    IN  RvSipCallLegHandle                hCallLeg,
                    IN  RvSipTransactionState             eTranscState,
                    IN  RvSipTransactionStateChangeReason eStateReason,
                    IN  SipTransactionMethod              eMethod,
                    IN  void*                             pObj,
                    OUT RvBool*                           bWasHandled)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus      rv       = RV_OK;
    RvSipMsgHandle hMsg    = NULL;
    Subscription*  pSubs   = NULL;
    Notification*  pNotify = NULL;
    RvLogSource* pLogSrc;
    RvUint16      statusCode;


    *bWasHandled = RV_TRUE;

    if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
       eMethod == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)pObj;
        pLogSrc = pSubs->pMgr->pLogSrc;
    }
    else if(eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
    {
        pNotify = (Notification*)pObj;
        pLogSrc = pNotify->pSubs->pMgr->pLogSrc;
    }
    else
    {
        *bWasHandled = RV_FALSE;
        return RV_ERROR_BADPARAM;
    }

    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "SipSubsHandleTranscChangeState - Call-leg 0x%p subs 0x%p - Failed to get received msg from transaction 0x%p",
            hCallLeg, pSubs, hTransc));
        return rv;
    }

    switch(eTranscState)
    {
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
        /* SUBSCRIBE or NOTIFY request was received */
        if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
           eMethod == SIP_TRANSACTION_METHOD_REFER)
        {
            rv = SubsHandleSubscribeRequest(pSubs, hTransc, eMethod, hMsg);
        }
        else if (eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
        {
            rv = SubsNotifyHandleRequest(pNotify, hTransc, hMsg);
        }
        break;
    case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
        /* SUBSCRIBE or NOTIFY final response was sent.
        here wa will handle only responses that were sent by the transaction layer,
        and have to change the subscription state too.
        (e.g. 487 response because of a cancel request )*/
        if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
           eMethod == SIP_TRANSACTION_METHOD_REFER)
        {
            rv = SubsHandleSubscribeFinalResponseSent(pSubs, hTransc, eStateReason);
        }
        else if (eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
        {
            rv = SubsNotifyHandleFinalResponseSent(pNotify, hTransc, eStateReason);
        }
        break;
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
        /* SUBSCRIBE or NOTIFY response was received */

        RvSipTransactionGetResponseCode(hTransc,&statusCode);

        if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE)
        {
            rv = SubsHandleSubscribeResponse(pSubs, hMsg, statusCode, RV_FALSE);
        }
        else if(eMethod == SIP_TRANSACTION_METHOD_REFER )
        {
            rv = SubsHandleSubscribeResponse(pSubs, hMsg, statusCode, RV_TRUE);
        }
        else if (eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
        {
            rv = SubsNotifyHandleResponse(pNotify, hMsg, hTransc, statusCode);
        }
        break;
    case RVSIP_TRANSC_STATE_TERMINATED:
        rv = SubsTranscTerminatedEvHandling(hCallLeg,hTransc, eMethod, eStateReason, pObj, bWasHandled);
        break;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
           eMethod == SIP_TRANSACTION_METHOD_REFER)
        {
            SubsHandleSubscribeMsgSendFailure(pSubs, hTransc, eStateReason);
        }
        else if (eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
        {
            SubsHandleNotifyMsgSendFailure(pNotify,hTransc, eStateReason);
        }
        break;
#endif
    default:
        break;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    *bWasHandled = RV_FALSE;
	RV_UNUSED_ARG(hTransc)
	RV_UNUSED_ARG(hCallLeg)
	RV_UNUSED_ARG(eTranscState)
	RV_UNUSED_ARG(eStateReason)
	RV_UNUSED_ARG(eMethod)
	RV_UNUSED_ARG(pObj)
		
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsMsgRcvdEvHandling
* ------------------------------------------------------------------------
* General: Handle a message received event, for a SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to any subscription,
*          bWasMsgHandled is set to false, so call-leg continue in handling the
*          message.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          hMsg - the received msg.
*          eMsgType - Notify or Subscribe.
* Output:  bWasMsgHandled - Was the message handled by subscription events or not.
***************************************************************************/
RvStatus RVCALLCONV SipSubsMsgRcvdEvHandling(
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipMsgHandle       hMsg,
                            IN  SipTransactionMethod eMethodType)
{
#ifdef RV_SIP_SUBS_ON
    void*                phObj;
    Subscription*        pSubs;
    RvSipNotifyHandle    hNotify = NULL;
    RvSipAppNotifyHandle hAppNotify = NULL;
    RvStatus            rv = RV_OK;
    RvSipMsgType         eMsgType   = RvSipMsgGetMsgType(hMsg);

    RV_UNUSED_ARG(hCallLeg);
    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs = ((Notification*)phObj)->pSubs;
        hNotify = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    rv = SubsCallbackMsgRcvdEv(pSubs, hNotify, hAppNotify, hMsg);

    /* if this is a response on SUBSCIBE/NOTIFY, and the subscription was created
    as a result of REFER request with method=SUBSCIRBE/REFER, we need to inform
    the refer subscription if this response */
    if(NULL == hNotify && eMsgType == RVSIP_MSG_RESPONSE)
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
            if(NULL != pSubs->hReferSubsGenerator)
            {
                /* notify the refer subscription generator about the response */
                SipSubsReferGeneratorCallNotifyReady(
                    pSubs->hReferSubsGenerator,
                    RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNDEFINED, hMsg);
            }

        }
    }
    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(eMethodType);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsMsgToSendEvHandling
* ------------------------------------------------------------------------
* General: Handle a message to send event, for a SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to any subscription,
*          bWasMsgHandled is set to false, so call-leg continue in handling the
*          message.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          hMsg - the received msg.
*          eMethodType - Notify or Subscribe.
* Output:  bWasMsgHandled - Was the message handled by subscription events or not.
***************************************************************************/
RvStatus RVCALLCONV SipSubsMsgToSendEvHandling(
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipMsgHandle       hMsg,
                            IN  SipTransactionMethod eMethodType)
{
#ifdef RV_SIP_SUBS_ON
    void*         phObj;
    Subscription* pSubs;
    RvSipNotifyHandle hNotify = NULL;
    RvSipAppNotifyHandle hAppNotify = NULL;
    RvStatus     rv = RV_OK;

    RV_UNUSED_ARG(hCallLeg);
    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs = ((Notification*)phObj)->pSubs;
        hNotify = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SipSubsMsgToSendEvHandling - subs 0x%p - a msg is about to be sent", pSubs));

    rv = SubsCallbackMsgToSendEv(pSubs, hNotify, hAppNotify, hMsg);
    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(eMethodType);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
* SipSubsNotifyHandleAuthCredentialsFound
* ------------------------------------------------------------------------
* General: Supply an authorization header, to pass it to the user, for a
*          SUBSCRIBE or NOTIFY message.
*          If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc - handle to the transaction.
*          eMsgType - Notify or Subscribe.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
* Output:  bWasMsgHandled - Was the message handled by subscription events or not.
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleAuthCredentialsFound(
                              IN  RvSipTranscHandle               hTransc,
                              IN  SipTransactionMethod            eMethodType,
                              IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                              IN  RvBool                          bCredentialsSupported)
{
#ifdef RV_SIP_SUBS_ON
    void*         phObj;
    Subscription* pSubs;
    RvSipNotifyHandle hNotify = NULL;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        hNotify = (RvSipNotifyHandle)phObj;
        pSubs = ((Notification*)phObj)->pSubs;
    }

    SubsCallbackAuthCredentialsFoundEv(pSubs, hNotify, hAuthorization, bCredentialsSupported);
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(hAuthorization);
    RV_UNUSED_ARG(bCredentialsSupported);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsNotifyHandleAuthCompleted
* ------------------------------------------------------------------------
* General: Notify that authentication procedure is completed, for a SUBSCRIBE or
*          NOTIFY message. If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc - handle to the transaction.
*          eMsgType - Notify or Subscribe.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleAuthCompleted(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvBool               bAuthSucceed)
{
#ifdef RV_SIP_SUBS_ON
    void*         phObj;
    Subscription* pSubs;
    RvSipNotifyHandle hNotify = NULL;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        hNotify = (RvSipNotifyHandle)phObj;
        pSubs = ((Notification*)phObj)->pSubs;
    }
    SubsCallbackAuthCompletedEv(pSubs, hNotify, bAuthSucceed);
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(bAuthSucceed);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
* SipSubsNotifyHandleOtherURLAddressFound
* ------------------------------------------------------------------------
* General: Notify that authentication procedure is completed, for a SUBSCRIBE or
*          NOTIFY message. If this is a Notify message, that does not relate to
*          any subscription, bWasMsgHandled is set to false,
*          so call-leg continue in handling the callback.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc           - handle to the transaction.
*           eMethodType    - The transaction method (Notify or Subscribe.)
*            hMsg           - The message that includes the other URL address.
*            hAddress       - Handle to unsupport address to be converted.
* Output:    hSipURLAddress - Handle to the known SIP URL address.
*            bAddressResolved-Indication of a successful/failed address
*                             resolving.
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleOtherURLAddressFound(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipMsgHandle       hMsg,
                              IN  RvSipAddressHandle   hAddress,
                              OUT RvSipAddressHandle   hSipURLAddress,
                              OUT RvBool               *bAddressResolved)
{
#ifdef RV_SIP_SUBS_ON
    void                *phObj;
    Subscription        *pSubs;
    RvSipNotifyHandle     hNotify    = NULL;
    RvSipAppNotifyHandle hAppNotify = NULL;
    RvStatus             rv            = RV_OK;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs       = ((Notification*)phObj)->pSubs;
        hNotify    = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    rv = SubsCallbackOtherURLAddressFoundEv(pSubs, hNotify, hAppNotify, hMsg,
                                            hAddress, hSipURLAddress, bAddressResolved);

    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(hAddress);
    RV_UNUSED_ARG(hSipURLAddress);

    *bAddressResolved = RV_FALSE;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsNotifyHandleFinalDestResolved
* ------------------------------------------------------------------------
* General:  Indicates that the transaction is about to send
*           a message after the destination address was resolved.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc           - handle to the transaction.
*           eMethodType    - The transaction method (Notify or Subscribe.)
*            hMsg           - The message to be sent
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleFinalDestResolved(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipMsgHandle       hMsg)
{
#ifdef RV_SIP_SUBS_ON
    void                *phObj;
    Subscription        *pSubs;
    RvSipNotifyHandle     hNotify    = NULL;
    RvSipAppNotifyHandle hAppNotify = NULL;
    RvStatus             rv            = RV_OK;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs       = ((Notification*)phObj)->pSubs;
        hNotify    = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    rv = SubsCallbackFinalDestResolvedEv(pSubs, hNotify, hAppNotify, hTransc, hMsg);

    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(hMsg);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsNotifyHandleNewConnInUse
 * ------------------------------------------------------------------------
 * General:  Indicates that the transaction is using a new connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc      - handle to the transaction.
 *           eMethodType  - The transaction method (Notify or Subscribe.)
 *           hConn         - The connection handle,
 *            bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleNewConnInUse(
                              IN  RvSipTranscHandle    hTransc,
                              IN  SipTransactionMethod eMethodType,
                              IN  RvSipTransportConnectionHandle hConn,
                              IN  RvBool               bNewConnCreated)

{
#ifdef RV_SIP_SUBS_ON
    void                 *phObj;
    Subscription         *pSubs;
    RvSipNotifyHandle     hNotify    = NULL;
    RvSipAppNotifyHandle  hAppNotify = NULL;
    RvStatus              rv            = RV_OK;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs       = ((Notification*)phObj)->pSubs;
        hNotify    = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    rv = SubsCallbackNewConnInUseEv(pSubs, hNotify, hAppNotify, hConn,bNewConnCreated);

    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(hConn);
    RV_UNUSED_ARG(bNewConnCreated);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}


#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SipSubsNotifyHandleSigCompMsgNotResponded
* ------------------------------------------------------------------------
* General:  Indicates that SigComp request was sent with no response
*           received back. The application should determine what action
*           has to be taken.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc         - Handle to the transaction.
*          hAppTransc      - The transaction application handle.
*          eMethodType     - The transaction method (Notify or Subscribe.)
*          retransNo       - The number of retransmissions of the request
*                            until now.
*          ePrevMsgComp    - The type of the previous not responded request
* Output:  peNextMsgComp   - The type of the next request to retransmit (
*                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
*                            application wishes to stop sending requests).
***************************************************************************/
RvStatus RVCALLCONV SipSubsNotifyHandleSigCompMsgNotResponded(
                              IN  RvSipTranscHandle            hTransc,
                              IN  RvSipAppTranscHandle         hAppTransc,
                              IN  SipTransactionMethod         eMethodType,
                              IN  RvInt32                      retransNo,
                              IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                              OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
#ifdef RV_SIP_SUBS_ON
    void                *phObj;
    Subscription        *pSubs;
    RvSipNotifyHandle     hNotify    = NULL;
    RvSipAppNotifyHandle hAppNotify = NULL;
    RvStatus             rv            = RV_OK;

    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)phObj;
    }
    else
    {
        pSubs       = ((Notification*)phObj)->pSubs;
        hNotify    = (RvSipNotifyHandle)phObj;
        hAppNotify = ((Notification*)phObj)->hAppNotify;
    }

    rv = SubsCallbackSigCompMsgNotRespondedEv(
                                          pSubs,
                                          hNotify,
                                          hAppNotify,
                                          hTransc,
                                          hAppTransc,
                                          retransNo,
                                          ePrevMsgComp,
                                          peNextMsgComp);

    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(hAppTransc);
    RV_UNUSED_ARG(eMethodType);
    RV_UNUSED_ARG(retransNo);
    RV_UNUSED_ARG(ePrevMsgComp);

    *peNextMsgComp = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
* SipSubsTerminate
* ------------------------------------------------------------------------
* General: Terminate a subscription, without sending any message.
*          If there are notification objects, it will be terminated first, and
*          when the last notification is free, the subscription is terminated.
*          if there are no notification object, the subscription is terminated at once.
*          subscription termination: subs state is changed to TERMINATED,
*          and subscription is destructed.
*          The function is used from call-leg layer, when user calls to RvSipCallLegTerminate.
*          subscription functions should use internal SubsTerminate function.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Cannot send response due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send the response message.
*               RV_OK - response message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs       - Pointer to the subscription the user wishes to subscribe.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SipSubsTerminate (IN  RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription      *pSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pSubs = (Subscription *)hSubs;
	

	if (pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
	{
		RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
			"SipSubsTerminate - Subs 0x%p was already in state Terminated.",
			hSubs));
		return RV_OK;
	}
    SubsTerminate(pSubs, RVSIP_SUBS_REASON_DIALOG_TERMINATION);

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SipSubsTerminate - Subs 0x%p was terminated",
        hSubs));
    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsGetListItemHandle
* ------------------------------------------------------------------------
* General: return the list item handle of the subscription in call-leg
*          subscriptions list.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RLIST_ITEM_HANDLE SipSubsGetListItemHandle(IN RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription *pSubs = (Subscription *)hSubs;

    if(hSubs == NULL)
    {
        return NULL;
    }

    return pSubs->hItemInCallLegList;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsIsEventHeaderExist
* ------------------------------------------------------------------------
* General: Define wether this subscription conains an event header or not.
*          (if not, it is the same as having 'event:PINT').
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvBool SipSubsIsEventHeaderExist(IN RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription *pSubs = (Subscription *)hSubs;

    if(hSubs == NULL)
    {
        return RV_TRUE;
    }

    return pSubs->bEventHeaderExist;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return RV_FALSE;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsInformOfRejectedRequest
* ------------------------------------------------------------------------
* General: If the call-leg layer rejected a subscription request, because
*          of a dialog mistake (such as wrong cseq) it will inform the
*          subscription of the rejection, with this function.
*          in this function, subscription updates it's state-machine.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvStatus SipSubsInformOfRejectedRequestByDialog(IN void*             pSubsInfo,
                                                IN RvSipTranscHandle hTransc)
{
#ifdef RV_SIP_SUBS_ON
    Subscription *pSubs;
    Notification *pNotify;
    SipTransactionMethod eMethod;
    RvStatus rv = RV_OK;

    SipTransactionGetMethod(hTransc,&eMethod);

    if(eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
       eMethod == SIP_TRANSACTION_METHOD_REFER)
    {
        pSubs = (Subscription*)pSubsInfo;
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SipSubsInformOfRejectedRequestByDialog: subs 0x%p - transc 0x%p was rejected by dialog layer.",
             pSubsInfo, hTransc));
        if(pSubs->eState == RVSIP_SUBS_STATE_IDLE)
        {
            /* the dialog rejected an initial subscribe request */
            /* we need to detach the transaction, otherwise the subscription termination
               will also terminate the transaction, since it is in it's transcList */
            SubsDetachTranscAndRemoveFromList(pSubs, hTransc);
            SubsTerminate(pSubs, RVSIP_SUBS_REASON_DIALOG_TERMINATION);
        }
        return RV_OK;
    }
    else if(eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
    {
        pNotify = (Notification*)pSubsInfo;
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SipSubsInformOfRejectedRequestByDialog: notify 0x%p - transc 0x%p was rejected by dialog layer.",
             pSubsInfo, hTransc));
        if(pNotify->eStatus == RVSIP_SUBS_NOTIFY_STATUS_IDLE)
        {
            if(hTransc == pNotify->hTransc)
            {
                rv = SipTransactionDetachOwner(hTransc);
                if(rv != RV_OK)
                {
                    RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                        "SipSubsInformOfRejectedRequestByDialog: subs 0x%p notify 0x%p - failed to detach transc 0x%p. response message might not be sent on TCP",
                        pNotify->pSubs,pNotify, hTransc));
                }
                pNotify->hTransc = NULL;
            }
            SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR);
        }
        return RV_OK;
    }
    else
    {
        return RV_ERROR_BADPARAM;
    }
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(pSubsInfo);
    RV_UNUSED_ARG(hTransc);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsGetSubscriptionStateOfNotifyObj
* ------------------------------------------------------------------------
* General: returns the state of subscription related to this notification object.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification.
***************************************************************************/
RvSipSubsState SipSubsGetSubscriptionStateOfNotifyObj(IN RvSipNotifyHandle hNotify)
{
#ifdef RV_SIP_SUBS_ON
    Notification *pNotify;

    pNotify = (Notification*)hNotify;

    return pNotify->pSubs->eState;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hNotify);

    return RVSIP_SUBS_STATE_UNDEFINED;
#endif /* #ifdef RV_SIP_SUBS_ON */
}


/***************************************************************************
* SipSubsGetCurrState
* ------------------------------------------------------------------------
* General: returns the state of subscription
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification.
***************************************************************************/
RvSipSubsState SipSubsGetCurrState(IN RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    return ((Subscription*)hSubs)->eState;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return RVSIP_SUBS_STATE_UNDEFINED;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/*-----------------------------------------------------------------------*/
/*                           REFER                                       */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SipSubsReferGeneratorCallNotifyReady
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - reason for the notify ready.
*          hResponseMsg - Handle to the received response message.
****************************************************************************/
RvStatus RVCALLCONV SipSubsReferGeneratorCallNotifyReady(
                                   IN  RvSipSubsHandle                 hSubs,
                                   IN  RvSipSubsReferNotifyReadyReason eReason,
                                   IN  RvSipMsgHandle                  hResponseMsg)
{
#ifdef RV_SIP_SUBS_ON
    return SubsReferGeneratorCallNotifyReady(hSubs, eReason, hResponseMsg);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(eReason);
    RV_UNUSED_ARG(hResponseMsg);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipSubsIsFirstReferSubs
* ------------------------------------------------------------------------
* General: Define wether this subscription is the first refer subscription
*          created on this dialog.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
RvBool SipSubsIsFirstReferSubs(IN RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    Subscription *pSubs = (Subscription *)hSubs;

    if(NULL == hSubs || NULL == pSubs->pReferInfo)
    {
        return RV_FALSE;
    }

    return pSubs->pReferInfo->bFirstReferSubsInDialog;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return RV_FALSE;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsReferInit
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with mandatory parameters:
 *          refer-to and referred-by headers of the refer subscription.
 *          If the subscription was not created inside of a call-leg you must
 *          call RvSipSubsDialogInitStr() before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request.
 *          You should call RvSipSubsRefer() in order to send a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - Handle to the subscription the user wishes to initialize.
 *          strReferTo    - String of the Refer-To header value.
 *                          for Example: "A<sip:176.56.23.4:4444;method=INVITE>"
 *          strReferredBy - String of the Referred-By header value.
 *                          for Example: "<sip:176.56.23.4:4444>"
 *          strReplaces  - The Replaces header to be set in the Refer-To header of
 *                         the REFER request. The Replaces header string doesn't
 *                           contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                           the subscription object.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsReferInit(
                         IN  RvSipSubsHandle             hSubs,
                         IN  RvSipReferToHeaderHandle    hReferToHeader,
                         IN  RvSipReferredByHeaderHandle hReferredByHeader,
                         IN  RvChar*                    strReferTo,
                         IN  RvChar*                    strReferredBy,
                         IN  RvChar*                    strReplaces)
{
#ifdef RV_SIP_SUBS_ON
    return SubsReferInit(hSubs, hReferToHeader, hReferredByHeader,
                          strReferTo, strReferredBy, strReplaces);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(hReferToHeader);
    RV_UNUSED_ARG(hReferredByHeader);
    RV_UNUSED_ARG(strReferTo);
    RV_UNUSED_ARG(strReferredBy);
	RV_UNUSED_ARG(strReplaces);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/******************************************************************************
* SipSubsGetTripleLock
* -----------------------------------------------------------------------------
* General: enable access to the Subscription triple lock from another modules
* -----------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
***************************************************************************/
SipTripleLock* RVCALLCONV SipSubsGetTripleLock(IN RvSipSubsHandle hSubs)
{
#ifdef RV_SIP_SUBS_ON
    return ((Subscription*)hSubs)->tripleLock;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);

    return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_HIGHAVAL_ON
/***************************************************************************
 * SipSubsGetActiveSubsStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of active subs.
 *          (The size of buffer, that should be supply in RvSipSubsStoreActiveSubs()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE in case
 *                           that a CallLeg object retrieves its subscription
 *                           storage size, in order to avoid infinity recursion.
 * Output:  pLen           - size of buffer. will be -1 in case of failure.
 ***************************************************************************/
RvStatus SipSubsGetActiveSubsStorageSize(IN  RvSipSubsHandle hSubs,
                                         IN  RvBool          bStandAlone,
                                         OUT RvInt32        *pLen)
{
#ifdef RV_SIP_SUBS_ON
    return SubsGetActiveSubsStorageSize(hSubs,bStandAlone,pLen);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(bStandAlone);
    *pLen = UNDEFINED;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsStoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Gets all the important parameters of an already active subs.
 *          The subs must be in state active.
 *          User must give an empty page that will be fullfill with the information.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE if
 *                           a CallLeg object asks for its subscription
 *                           storage, in order to avoid infinity recursion.
 *          maxBuffLen     - The length of the given buffer.
 * Output:  memBuff        - The buffer that will store the Subs' parameters.
 *          pStoredLen     - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus SipSubsStoreActiveSubs(IN  RvSipSubsHandle  hSubs,
                                IN  RvBool           bStandAlone,
                                IN  RvUint32         maxBuffLen,
                                OUT void            *memBuff,
                                OUT RvInt32         *pStoredLen)
{
#ifdef RV_SIP_SUBS_ON
    Subscription *pSubs = (Subscription *)hSubs;

    /* Storing a Refer Subscription is impossible */
    if (pSubs->pReferInfo != NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SipSubsStoreActiveSubs - Subscription 0x%p is a REFER Subs ,thus it cannot be stored",
                pSubs));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    return SubsStoreActiveSubs(hSubs,bStandAlone,maxBuffLen,memBuff,(RvUint32*)pStoredLen);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(bStandAlone);
    RV_UNUSED_ARG(maxBuffLen);
    RV_UNUSED_ARG(memBuff);
    *pStoredLen = UNDEFINED;

    return RV_ERROR_ILLEGAL_ACTION;

#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsRestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Create a new Subs on state Active, and fill all it's parameters,
 *          which are strored on the given page. (the page was updated in the
 *          SubsStoreActiveSubs function).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff        - The buffer that stores all the Subs' parameters.
 *        buffLen        - The buffer length.
 *        bStandAlone    - Indicates if the Subs is independent.
 *                         This parameter should be RV_FALSE if
 *                         a CallLeg object asks for its subscription
 *                         restoration, in order to avoid infinity recursion.
 * Input/Output:
 *        pSubs   - Pointer to the restored subscription.
 ***************************************************************************/
RvStatus SipSubsRestoreActiveSubs(IN    void            *memBuff,
                                  IN    RvUint32         buffLen,
                                  IN    RvBool           bStandAlone,
                                  INOUT RvSipSubsHandle  hSubs)
{
#ifdef RV_SIP_SUBS_ON
    return SubsRestoreActiveSubs(memBuff,buffLen,bStandAlone,(Subscription *)hSubs);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(memBuff);
    RV_UNUSED_ARG(buffLen);
    RV_UNUSED_ARG(bStandAlone);
    RV_UNUSED_ARG(hSubs);

    return RV_ERROR_ILLEGAL_ACTION;

#endif /* #ifdef RV_SIP_SUBS_ON */
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 

/***************************************************************************
* SipSubsRespondProvisional
* ------------------------------------------------------------------------
* General:  Send 1xx on subscribe request (initial subscribe, refresh
*           or unsubscribe).
*           Can be used in the RVSIP_SUBS_STATE_SUBS_RCVD, RVSIP_SUBS_STATE_REFRESH_RCVD
*           or RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD states.
*           Note - According to RFC 3265, sending provisional response on
*           SUBSCRIBE request is not allowed.
*           According to RFC 3515, only 100 response on REFER request is allowed.
* Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
*               RV_ERROR_BADPARAM          - The status code is invalid.
*               RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
*               RV_ERROR_UNKNOWN           - Failed to reject the subscription. (failed
*                                            while trying to send the reject response).
*               RV_OK                      - 1xx was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs      - Handle to the subscription the user wishes to send 1xx for.
*           statusCode - The 1xx value.
*           strReasonPhrase - Reason phrase to be set in the sent message.
*                        (May be NULL, to set a default reason phrase.)
***************************************************************************/
RvStatus SipSubsRespondProvisional(IN  RvSipSubsHandle  hSubs,
                                   IN  RvUint16         statusCode,
                                   IN  RvChar*          strReasonPhrase)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus      rv;
    Subscription *pSubs = (Subscription*)hSubs;

    if(statusCode >= 200 || statusCode < 100)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SipSubsRespondProvisional - subscription 0x%p: Illegal Action with statusCode %d",
            pSubs,statusCode));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SipSubsRespondProvisional - sending %d for subscription 0x%p",statusCode, pSubs));

    /* function can be called only on subs state subs_rcvd, refresh_rcvd, unsubscribe_rcvd */

    if((pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_REFRESH_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SipSubsRespondProvisional - Failed for subscription 0x%p: Illegal Action in subs state %s",
            pSubs,SubsGetStateName(pSubs->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsSendProvisional(pSubs,statusCode, strReasonPhrase);

    return rv;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(statusCode);
    RV_UNUSED_ARG(strReasonPhrase);

    return RV_ERROR_ILLEGAL_ACTION;

#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * SipSubsGetSubscriptionFromTransc
 * ------------------------------------------------------------------------
 * General: Get the subscription object from a transaction
 * Return Value: the subscription object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc     - handle to the transaction.
 ***************************************************************************/
RvSipSubsHandle RVCALLCONV SipSubsGetSubscriptionFromTransc(
										IN  RvSipTranscHandle    hTransc)
{
#ifdef RV_SIP_SUBS_ON
    void*                phObj;
    Subscription*        pSubs;
	SipTransactionMethod eMethodType;

	/* Get the method from the transaction */
	SipTransactionGetMethod(hTransc,&eMethodType);
    
	/* Get the subscription info from the transaction */
    phObj = SipTransactionGetSubsInfo(hTransc);

    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
		/* The subscription info of the transaction is a subscrioption object */
        pSubs = (Subscription*)phObj;
    }
    else
    {
		/* The subscription info of the transaction is a notification object */
        pSubs = ((Notification*)phObj)->pSubs;
    }

	return (RvSipSubsHandle)pSubs;
	
#else /* RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hTransc);
	return NULL;
#endif /* RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsHandleSubscribeRequest
* ------------------------------------------------------------------------
* General: The function receives an incoming subscribe request, and update
*          state-machine as needed.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription.
*          hTransc - Handle to the subscribe transaction.
*          hMsg   - Handle to the subscribe request message.
***************************************************************************/
static RvStatus RVCALLCONV SubsHandleSubscribeRequest(Subscription*        pSubs,
                                                      RvSipTranscHandle    hTransc,
                                                      SipTransactionMethod eMethod,
                                                      RvSipMsgHandle       hMsg)
{
    RvStatus rv = RV_OK;
    RvSipCallLegState eCallState;
    RvUint16 responseCode = 0;

    RvSipCallLegGetCurrentState(pSubs->hCallLeg, &eCallState);

    if(eCallState == RVSIP_CALL_LEG_STATE_DISCONNECTING)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsHandleSubscribeRequest - subs 0x%p - Responding with 400 to a SUBSCRIBE/REFER message received in state DISCONNECTING",
            pSubs));

        pSubs->hActiveTransc = hTransc;
        rv = SubsReject(pSubs, 400, NULL);
        pSubs->hActiveTransc = NULL;
        return rv;

    }

    if(SIP_TRANSACTION_METHOD_REFER == eMethod)
    {
        /* check refer request validity */
       rv = SubsReferCheckReferRequestValidity(pSubs,hMsg,&responseCode);
       if(rv != RV_OK)
       {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeRequest - subs 0x%p - SubsReferCheckReferRequestValidity failed (rv=%d)",
                pSubs, rv));
            return rv;
       }
       if(responseCode > 0)
       {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeRequest - subs 0x%p - illegal REFER request. reject with %d",
                pSubs, responseCode));
            RvSipTransactionRespond(hTransc,responseCode,NULL);

            /* need to detach the transaction ,otherwise it will be terminated with the
               subscription, and the respond won't be sent */
            SubsUpdateStateAfterReject(pSubs,RVSIP_SUBS_REASON_ILLEGAL_REFER_MSG);
            
            return RV_OK;
       }
    }

    /* initial SUBSCRIBE request with refer event should be rejected with 403 */
    if(SIP_TRANSACTION_METHOD_SUBSCRIBE == eMethod &&
       RVSIP_SUBS_STATE_IDLE == pSubs->eState)
    {
        RvSipEventHeaderHandle hEvent;
        RvSipHeaderListElemHandle hListElem;

        hEvent = (RvSipEventHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                           RVSIP_HEADERTYPE_EVENT, RVSIP_FIRST_HEADER, &hListElem);
        if(hEvent != NULL)
        {
            if(SipEventHeaderIsReferEventPackage(hEvent) == RV_TRUE)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsHandleSubscribeRequest - subscription 0x%p - initial SUBSCRIBE with event refer. reject with 403",
                    pSubs));

                rv = RvSipTransactionRespond(hTransc,403,NULL);

                /* need to detach the transaction ,otherwise it will be terminated with the
                   subscription, and the respond won't be sent */
                SubsDetachTranscAndRemoveFromList(pSubs, hTransc);
                pSubs->hActiveTransc = NULL;

                SubsTerminate(pSubs, RVSIP_SUBS_REASON_ILLEGAL_REFER_MSG);
                return RV_OK;
            }
        }
    }

    switch (pSubs->eState)
    {
      /* the following states are a subscriber's states, and not notifier's
        therefor it is a bug situation for all of them */
        case RVSIP_SUBS_STATE_SUBS_SENT:
        case RVSIP_SUBS_STATE_REDIRECTED:
        case RVSIP_SUBS_STATE_UNAUTHENTICATED:
        case RVSIP_SUBS_STATE_2XX_RCVD:
        case RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD:
        case RVSIP_SUBS_STATE_REFRESHING:
        case RVSIP_SUBS_STATE_UNSUBSCRIBING:
        case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
        case RVSIP_SUBS_STATE_UNDEFINED :
            RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeRequest -  Subs 0x%p - Illegal subscribe request at (subscriber) state %s",
                pSubs, SubsGetStateName(pSubs->eState)));
            return RV_ERROR_UNKNOWN;
        default:
            break;
    }


    return HandleSubsRequest(pSubs, hTransc, hMsg);
}

/***************************************************************************
* SubsHandleSubscribeResponse
* ------------------------------------------------------------------------
* General: The function receives a response for a subscribe request, and update
*          state-machine as needed.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs        - Handle to the subscription
*          hMsg         - Handle to the response message.
*          responseCode - The response code received on the subscribe request.
*          bRefer       = TRUE if this is response on REFER request.
***************************************************************************/
static RvStatus RVCALLCONV SubsHandleSubscribeResponse(
                                            IN  Subscription*  pSubs,
                                            IN  RvSipMsgHandle hMsg,
                                            IN  RvUint16       responseCode,
                                            IN  RvBool        bRefer)
{
    RvBool bAuthValid = RV_TRUE;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsHandleSubscribeResponse: Subscription 0x%p - %d response was received on state %s. ",
        pSubs, responseCode, SubsGetStateName(pSubs->eState)));
    
#ifdef RV_SIP_AUTH_ON
    if(401 == responseCode || 407 == responseCode)
    {
        RvStatus rv;
 
        /* check credential validity. if invalid - terminate the subscription ??? */
        rv = SipCallLegAuthenticatorCheckValidityOfAuthObj(pSubs->hCallLeg, &bAuthValid);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeResponse: Subscription 0x%p - Failed to check validation of challenge in %d response. terminate subs!!! ",
                pSubs, responseCode));
            return SubsTerminate(pSubs, RVSIP_SUBS_REASON_LOCAL_FAILURE);
        }
        if(bAuthValid == RV_FALSE)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeResponse: Subscription 0x%p - invalid challenge in %d response.",
                pSubs, responseCode));        
        }
    }
#endif /*RV_SIP_AUTH_ON*/

    /* in case of 3xx-6xx save the retry-after value if exists */
    if(responseCode >= 300 && responseCode < 700)
    {
        SipCommonGetRetryAfterFromMsg(hMsg, &pSubs->retryAfter);
    }

    switch (pSubs->eState)
    {
        case RVSIP_SUBS_STATE_SUBS_SENT:
            return SubsHandleResponseOnInitialSubscribe(pSubs, hMsg, responseCode, bRefer, bAuthValid);

        case RVSIP_SUBS_STATE_REFRESHING:
            return SubsHandleResponseOnRefresh(pSubs, hMsg, responseCode, bAuthValid);

        case RVSIP_SUBS_STATE_UNSUBSCRIBING:
            return SubsHandleResponseOnUnsubscribe(pSubs, responseCode, bAuthValid);

        case RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD:
            return SubsHandleResponseOnNotifyBefore2xx(pSubs, hMsg, responseCode, bRefer);

        case RVSIP_SUBS_STATE_TERMINATED:
            break;

        case RVSIP_SUBS_STATE_ACTIVE:
        case RVSIP_SUBS_STATE_IDLE:
        case RVSIP_SUBS_STATE_SUBS_RCVD:
        case RVSIP_SUBS_STATE_PENDING:
        case RVSIP_SUBS_STATE_ACTIVATED:
        case RVSIP_SUBS_STATE_REFRESH_RCVD:
        case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
        case RVSIP_SUBS_STATE_TERMINATING:

        case RVSIP_SUBS_STATE_REDIRECTED:
        case RVSIP_SUBS_STATE_UNAUTHENTICATED:
        case RVSIP_SUBS_STATE_2XX_RCVD:
        case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
        case RVSIP_SUBS_STATE_UNDEFINED :
        default:
            RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeResponse -  Subs 0x%p - Illegal subscribe response at (subscriber) state %s",
                pSubs, SubsGetStateName(pSubs->eState)));
            return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}


/***************************************************************************
* SubsHandleSubscribeFinalResponseSent
* ------------------------------------------------------------------------
* General:  The function update the subscription state, when a response was
*           sent by the transaction layer.
*           for example, a CANCEL request was received for this SUBSCRIBE request,
*           and transaction layer respond with 487 on the subscribe.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription.
*          hTransc - Handle to the subscribe transaction.
*          hMsg    - Handle to the subscribe request message.
***************************************************************************/
static RvStatus RVCALLCONV SubsHandleSubscribeFinalResponseSent(
                                  IN Subscription*                     pSubs,
                                  IN RvSipTranscHandle                 hTransc,
                                  IN RvSipTransactionStateChangeReason eStateReason)
{
    RvUint16 responseStatusCode;

    RvSipTransactionGetResponseCode(hTransc,&responseStatusCode);

    if(eStateReason == RVSIP_TRANSC_REASON_TRANSACTION_CANCELED &&
       responseStatusCode == 487)
    {
        if(NULL != pSubs->hActiveTransc)
        {
            if(hTransc == pSubs->hActiveTransc)
            {
                pSubs->hActiveTransc = NULL;
            }
            else
            {
                RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsHandleSubscribeFinalResponseSent - Subs 0x%p - hTransc 0x%p != activeTransc 0x%p. (%d response at state SUBS_RCVD)",
                    pSubs, hTransc, pSubs->hActiveTransc, responseStatusCode));
            }
        }

        /* the following 3 states indicates that subscription does not know about the
           response that was sent, and so the transaction layer is the one that sent it*/
        if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_RCVD)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeFinalResponseSent - Subs 0x%p - %d response was sent by transc at state SUBS_RCVD. terminate subsscription...",
                pSubs, responseStatusCode));
            /* need to detach the transaction ,otherwise it will be terminated with the
               subscription, and the respond won't be sent */
            SubsDetachTranscAndRemoveFromList(pSubs, hTransc);

            SubsTerminate(pSubs, RVSIP_SUBS_REASON_LOCAL_REJECT);
        }
        else if(pSubs->eState == RVSIP_SUBS_STATE_REFRESH_RCVD)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeFinalResponseSent - Subs 0x%p - %d response was sent by transc at state REFRESH_RCVD",
                pSubs, responseStatusCode));
            SubsChangeState(pSubs,RVSIP_SUBS_STATE_ACTIVE, RVSIP_SUBS_REASON_LOCAL_REJECT);
        }
        else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsHandleSubscribeFinalResponseSent - Subs 0x%p - %d response was sent by transc at state UNSUBS_RCVD",
                pSubs, responseStatusCode));
            SubsChangeState(pSubs,pSubs->ePrevState, RVSIP_SUBS_REASON_LOCAL_REJECT);
        }

    }
    return RV_OK;
}

/***************************************************************************
* SubsNotifyHandleRequest
* ------------------------------------------------------------------------
* General: The function receives an incoming notify request,
*          and give it to the application. (this message will influence the
*          subscription state machine, only if user choose to accept it).
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification
*          hTransc - Handle to the NOTIFY transaction.
*          hMsg - Handle to the notify request message.
***************************************************************************/
static RvStatus RVCALLCONV SubsNotifyHandleRequest(IN  Notification*     pNotify,
                                                    IN  RvSipTranscHandle hTransc,
                                                    IN  RvSipMsgHandle    hMsg)
{
    RvStatus rv;
    RvSipSubsNotifyReason eReason = RVSIP_SUBS_NOTIFY_REASON_UNDEFINED;

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsNotifyHandleRequest -  Notify 0x%p - handling a new notify request...",pNotify));

    /* ------------------------------------------------------
       save parameters from message in the notification object
       ------------------------------------------------------ */
    rv = SetNotificationParamsFromMsg(pNotify, hMsg, &eReason);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsNotifyHandleRequest -  Notify 0x%p - no Subscription-State header. reject with 400 ",
            pNotify));
        rv = RvSipTransactionRespond(hTransc,400,NULL);
        SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_BAD_REQUEST_MESSAGE);
        return rv;
    }

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsNotifyHandleRequest - subs 0x%p Notify 0x%p - a NOTIFY(%s) request was received",
         pNotify->pSubs,pNotify,GetNotifySubsStateName(pNotify->eSubstate)));

    /* if the notify is received before 2xx response, we should update
       the dialog to-tag, according to the notify from-tag */
    if(RVSIP_SUBS_STATE_SUBS_SENT == pNotify->pSubs->eState &&
       RV_TRUE == SipCallLegIsHiddenCallLeg(pNotify->pSubs->hCallLeg))
    {
        RvSipPartyHeaderHandle hMsgFrom, hDialogTo;
        RvInt32               tagOffest;
        hMsgFrom = RvSipMsgGetFromHeader(hMsg);
        tagOffest = SipPartyHeaderGetTag(hMsgFrom);
		
		/* Get the To header of the call-leg */
		RvSipCallLegGetToHeader(pNotify->pSubs->hCallLeg, &hDialogTo);

		/* We are about to update a remote tag in the call-leg. When the 2xx will 
		   be received it might contain a different To tag due to forking. If the call-leg
		   and the SUBSCRIBE transaction would continue to share their key, the
		   transaction will run over the NOTIFY tag with the 200 tag. Therefore, we detach
		   the call-leg key from the transaction key, and the call-leg will remain with the
		   tag that was received here in the NOTIFY */
		rv = SipTransactionDetachFromOwnerKey(pNotify->pSubs->hActiveTransc);

        if(tagOffest > UNDEFINED && RV_OK == rv)
        {
            /* set it to the dialog */
			rv = SipPartyHeaderSetTag(hDialogTo, NULL, SipPartyHeaderGetPool(hMsgFrom),
										SipPartyHeaderGetPage(hMsgFrom), tagOffest);
        }
#ifdef RFC_2543_COMPLIANT
		else if (RV_OK == rv)
		{
			/* An empty tag at this point is considered as an actual tag. From now on only
			   empty tags will match this call-leg */
			rv = SipPartyHeaderSetEmptyTag(hDialogTo);
		}
#endif /* RFC_2543_COMPLIANT */
		if(rv != RV_OK)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
						"SubsNotifyHandleRequest -  Notify 0x%p - failed to update dialog to-tag. reject with 400 ",
						pNotify));
            rv = RvSipTransactionRespond(pNotify->hTransc,500,NULL);
            SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_UNDEFINED);
            return rv;
        }

    }
    pNotify->hTransc = hTransc;

    /* set referFinalStatus from notify to refer subscription, in case of notify with
    event refer */
    rv = SubsReferLoadFromNotifyRequestRcvdMsg(pNotify, hMsg);
    if(rv != RV_OK)
    {
        /* this is possible, because the notify may be because of a refresh request */
        RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsNotifyHandleRequest -  Notify 0x%p - Failed to load refer final status from notify message",
            pNotify));
    }

    /*---------------------------------------------
      Inform user of the incoming notify request
      --------------------------------------------- */
    InformNotifyStatus(pNotify,
                       RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD,
                       eReason,
                       hMsg);

    return RV_OK;

}

/***************************************************************************
* SubsNotifyHandleFinalResponseSent
* ------------------------------------------------------------------------
* General: The function update the notification, when a response was
*          sent by the transaction layer.
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify - Handle to the notification
*          hTransc - Handle to the NOTIFY transaction.
*          hMsg - Handle to the notify request message.
***************************************************************************/
static RvStatus RVCALLCONV SubsNotifyHandleFinalResponseSent(
                          IN  Notification*     pNotify,
                          IN  RvSipTranscHandle hTransc,
                          IN RvSipTransactionStateChangeReason eStateReason)
{
    RvStatus rv;

    RV_UNUSED_ARG(hTransc);
    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsNotifyHandleFinalResponseSent -  Notify 0x%p - a notify response was sent",pNotify));

    if(eStateReason == RVSIP_TRANSC_REASON_TRANSACTION_CANCELED)
    {
        if(RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD == pNotify->eStatus)
        {
            /* the response was sent by the transaction layer, and not by notification object */
            InformNotifyStatus(pNotify,RVSIP_SUBS_NOTIFY_STATUS_FINAL_RESPONSE_SENT,
                                RVSIP_SUBS_NOTIFY_REASON_UNDEFINED,NULL);

            rv = SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_UNDEFINED);
            return rv;
        }

    }

    return RV_OK;

}
/***************************************************************************
* SubsNotifyHandleResponse
* ------------------------------------------------------------------------
* General: The function handles a response for a notify request,
*          it sets the subscription state-machine (if needed) and gives the response
*          to the application. (only 2xx message will influence the
*          subscription state machine).
* Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify      - Handle to the notification
*          hMsg         - Handle to the response message.
*          responseCode - The response code in the response message.
***************************************************************************/
static RvStatus RVCALLCONV SubsNotifyHandleResponse(IN  Notification    * pNotify,
                                                    IN  RvSipMsgHandle    hMsg,
                                                    IN  RvSipTranscHandle hNotifyTransc,
                                                    IN  RvInt16           responseCode)
{
    RvStatus rv;
    RvSipSubsNotifyStatus status;
    RvBool bSubsDeleted = RV_FALSE;

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsNotifyHandleResponse -  Notify 0x%p - %d response for notify was received",
        pNotify, responseCode));

    /* in case of 3xx-6xx save the retry-after value if exists */
    if(responseCode >= 300 && responseCode < 700)
    {
        SipCommonGetRetryAfterFromMsg(hMsg, &pNotify->retryAfter);
    }

    if(responseCode == 481)
    {
        if(pNotify->pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
        {
            RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsNotifyHandleResponse -  Notify 0x%p subs 0x%p - 481 response for notify was received. subscription already in state TERMINATED",
                pNotify, pNotify->pSubs));
            return RV_OK;
        }
        else
        {
            RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsNotifyHandleResponse -  Notify 0x%p subs 0x%p - 481 response for notify was received. terminate subscription!!!",
                pNotify, pNotify->pSubs));
        
            return SubsTerminate(pNotify->pSubs, RVSIP_SUBS_REASON_481_RCVD);
        }
    }
#ifdef RV_SIP_AUTH_ON
    else if(responseCode == 401 || responseCode == 407)
    {
        status = RVSIP_SUBS_NOTIFY_STATUS_UNAUTHENTICATED;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
    else if(responseCode >= 300 && responseCode < 400)
    {
        /* 3xx */
        status = RVSIP_SUBS_NOTIFY_STATUS_REDIRECTED;
    }
    else if(responseCode >= 200 && responseCode < 300) /* 2xx */
    {
        status = RVSIP_SUBS_NOTIFY_STATUS_2XX_RCVD;
    }
    else /* other non 2xx */
    {
        status = RVSIP_SUBS_NOTIFY_STATUS_REJECT_RCVD;
    }
    /*---------------------------------------------
      Inform user of the incoming response
      --------------------------------------------- */
    InformNotifyStatus(pNotify,
                       status,
                       RVSIP_SUBS_NOTIFY_REASON_UNDEFINED,
                       hMsg);

    /* ----------------------------------------------------------
       2xx - update subscription with parameters of notification object
       ---------------------------------------------------------- */
    if(responseCode >= 200 && responseCode < 300) /* 2xx */
    {
        rv = UpdateSubsAfter2xxOnNotifyRcvd(pNotify->pSubs, pNotify, hNotifyTransc, &bSubsDeleted);
        if(rv != RV_OK)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsNotifyHandleResponse -  Subs 0x%p Notify 0x%p - Failed to update subs state machine",
                pNotify->pSubs, pNotify));
        }
    }

    /* after receiving response for the notify request, we leave the transaction
    alive, for re-transmissions, and terminate the notification object */
    if(bSubsDeleted == RV_FALSE)
    {
        SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_UNDEFINED);
    }
    return RV_OK;

}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * SubsHandleSubscribeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: The function receives an incoming subscribe request, and update
 *          state-machine as needed.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the subscribe transaction
 *          eStateReason - reason for msg send failure.
 ***************************************************************************/
static void RVCALLCONV SubsHandleSubscribeMsgSendFailure(
                              IN  Subscription*                     pSubs,
                              IN  RvSipTranscHandle                 hTransc,
                              IN  RvSipTransactionStateChangeReason eStateReason)
{
    RvSipSubsStateChangeReason eSubsReason;

    RV_UNUSED_ARG(hTransc);
    switch(eStateReason)
    {
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
        eSubsReason = RVSIP_SUBS_REASON_NETWORK_ERROR;
        break;
    case RVSIP_TRANSC_REASON_503_RECEIVED:
        eSubsReason = RVSIP_SUBS_REASON_503_RECEIVED;
        break;
    case RVSIP_TRANSC_REASON_TIME_OUT:
        eSubsReason = RVSIP_SUBS_REASON_TRANSC_TIME_OUT;
        break;
    default:
        eSubsReason = RVSIP_SUBS_REASON_UNDEFINED;
    }

    if(pSubs->eState == RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD)
    {
        /* special case: we already received a notify request and accepted it,
           and now the subscribe request failed. 
           this is an error situation, notify request should not be received, 
           if the subscribe request was not accepted */
        RvLogWarning(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "SubsHandleSubscribeMsgSendFailure - Subscription 0x%p - subscribe transaction 0x%p failed. terminate subs.",
            pSubs, hTransc));
        SubsTerminate(pSubs, eSubsReason);
        return;

    }
    SubsChangeState(pSubs, RVSIP_SUBS_STATE_MSG_SEND_FAILURE, eSubsReason);
}


/***************************************************************************
 * SubsHandleNotifyMsgSendFailure
 * ------------------------------------------------------------------------
 * General: The function receives an incoming subscribe request, and update
 *          state-machine as needed.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the notify transaction
 *          eStateReason - reason for msg send failure.
 ***************************************************************************/
static void RVCALLCONV SubsHandleNotifyMsgSendFailure(
                              IN  Notification*                     pNotify,
                              IN  RvSipTranscHandle                 hTransc,
                              IN  RvSipTransactionStateChangeReason eStateReason)
{
    RvSipSubsNotifyReason eNotifyReason;

    RV_UNUSED_ARG(hTransc);
    switch(eStateReason)
    {
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_NETWORK_ERROR;
        break;
    case RVSIP_TRANSC_REASON_503_RECEIVED:
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_503_RECEIVED;
        break;
    case RVSIP_TRANSC_REASON_TIME_OUT:
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_TRANSC_TIMEOUT;
        break;
    default:
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_UNDEFINED;
    }


    InformNotifyStatus(pNotify, RVSIP_SUBS_NOTIFY_STATUS_MSG_SEND_FAILURE,
                       eNotifyReason, NULL);
}

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

/***************************************************************************
* SubsTranscTerminatedEvHandling
* ------------------------------------------------------------------------
* General: Handle a SUBSCRIBE or NOTIFY transaction termination.
*          The function removes the transaction from it's object, and destruct
*          the object if needed.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          eStateReason - reason for termination.
*          eMethodType - Notify or Subscribe.
* Output:  bWasMsgHandled - Was the transaction handled by subscription events or not.
***************************************************************************/
static RvStatus RVCALLCONV SubsTranscTerminatedEvHandling(
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvSipTranscHandle    hTransc,
                            IN  SipTransactionMethod eMethodType,
                            IN  RvSipTransactionStateChangeReason eStateReason,
                            IN  void*               pObj,
                            OUT RvBool             *bWasMsgHandled)
{
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_UNDEFINED;
    RvSipSubsNotifyReason eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_UNDEFINED;

    RV_UNUSED_ARG(hCallLeg);
    *bWasMsgHandled = RV_TRUE;

    switch (eStateReason)
    {
    case RVSIP_TRANSC_REASON_TIME_OUT:
        eReason = RVSIP_SUBS_REASON_TRANSC_TIME_OUT;
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_TRANSC_TIMEOUT;
        break;
    case RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS:
        eReason = RVSIP_SUBS_REASON_NO_TIMERS;
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR;
        break;
    case RVSIP_TRANSC_REASON_ERROR:
        eReason = RVSIP_SUBS_REASON_TRANSC_ERROR;
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR;
        break;
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
        eReason = RVSIP_SUBS_REASON_NETWORK_ERROR;
        eNotifyReason = RVSIP_SUBS_NOTIFY_REASON_NETWORK_ERROR;
        break;
    case RVSIP_TRANSC_REASON_USER_COMMAND:
        eNotifyReason = ((Notification*)pObj)->eTerminationReason;
        break;
    default:
        break;
    }
	
    if (eMethodType == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
        eMethodType == SIP_TRANSACTION_METHOD_REFER)
    {
        return SubsHandleSubsTranscTerminated(hTransc, eReason, pObj);
    }
    else /* notify */
    {
        return SubsHandleNotifyTranscTerminated(hTransc, eNotifyReason, pObj);
    }
}



/***************************************************************************
* SubsTranscTerminatedEvHandling
* ------------------------------------------------------------------------
* General: Handle subscribe transaction termination.
*          1. remove transaction from subscription transactions list.
*          2. if the subscription is in state terminated - no need to update
*             it's state-machine. call to SubsTerminatIfPossible.
*          3. if this is a normal (not error) termination - do nothing.
*          4. else - update the state machine.
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc - handle to the transaction.
*          eReason - reason for termination.
*          eMethodType - Notify or Subscribe.
***************************************************************************/
static RvStatus RVCALLCONV SubsHandleSubsTranscTerminated(
                            IN    RvSipTranscHandle   hTransc,
                            IN  RvSipSubsStateChangeReason eReason,
                            IN  void*               pObj)
{
    Subscription* pSubs;

    pSubs = (Subscription*)pObj;

    /* ---------------------------------------------------
        remove transc from subscription transactions list
       ---------------------------------------------------*/
    SubsRemoveTranscFromList(pSubs, hTransc);

    if(pSubs->hActiveTransc != hTransc)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsHandleSubsTranscTerminated: given transc 0x%p is different then pSubs->hActiveTransc 0x%p, ignore it",
            hTransc, pSubs->hActiveTransc));
    }
    else
    {
        pSubs->hActiveTransc = NULL;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsHandleSubsTranscTerminated: transc 0x%p of subscription 0x%p was terminated.",
        hTransc, pSubs));

    if(RVSIP_SUBS_STATE_TERMINATED == pSubs->eState)
    {
        /* if the subscription is in state terminated, then the transaction termination
           should continue the subscription termination process */
        SubsTerminateIfPossible(pSubs);
        return RV_OK;
    }

    if(eReason == RVSIP_SUBS_REASON_UNDEFINED)
    {
        /* this is not a transaction error */
        return RV_OK;
    }
    
    else if(eReason == RVSIP_SUBS_REASON_TRANSC_ERROR)
    {
        /* terminate subscription */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsHandleSubsTranscTerminated: subscription 0x%p - Terminate subs because of transaction 0x%p error termination.",
            pSubs, hTransc));
        SubsTerminate(pSubs, eReason);
        return RV_OK;
    }

    /* timeout on subscribe request causes termination only for initial subscribe
    and unsubscribe. refresh will not cause termination */
    if(pSubs->eState == RVSIP_SUBS_STATE_REFRESHING)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsHandleSubsTranscTerminated: subscription 0x%p refreshing failed - transaction 0x%p termination.",
            pSubs, hTransc));
        SubsChangeState(pSubs, RVSIP_SUBS_STATE_ACTIVE, eReason);
        return RV_OK;
    }
    else if (pSubs->eState == RVSIP_SUBS_STATE_SUBS_SENT ||
             pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBING ||
             pSubs->eState == RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD ||
             pSubs->eState == RVSIP_SUBS_STATE_IDLE ||
             pSubs->eState == RVSIP_SUBS_STATE_ACTIVE) /* active - if the failure is on 2xx response sending */
    {
        /* terminate subscription */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsHandleSubsTranscTerminated: subscription 0x%p - Terminate subs because of transaction 0x%p termination.",
            pSubs, hTransc));
        SubsTerminate(pSubs, eReason);
        return RV_OK;
    }
    return RV_OK;
}

/***************************************************************************
* SubsTranscTerminatedEvHandling
* ------------------------------------------------------------------------
* General: Handle notify transaction termination
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   hCallLeg - call-leg this transaction belongs to.
*          hTransc - handle to the transaction.
*          eStateReason - reason for termination.
*          eMethodType - Notify or Subscribe.
* Output:  bWasMsgHandled - Was the transaction handled by subscription events or not.
***************************************************************************/
static RvStatus RVCALLCONV SubsHandleNotifyTranscTerminated(
                            IN  RvSipTranscHandle     hTransc,
                            IN  RvSipSubsNotifyReason eReason,
                            IN  void*                 pObj)
{
    Subscription* pSubs;
    Notification* pNotify;
    RvSipSubscriptionSubstate eNotifySubsState;

    pNotify = (Notification*)pObj;
    pSubs   = pNotify->pSubs;

    if(hTransc != pNotify->hTransc)
    {
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsHandleNotifyTranscTerminated: given transc 0x%p is different then pNotify->hTransc 0x%p, ignore it",
            hTransc, pNotify->hTransc));
        return RV_OK;
    }
    pNotify->hTransc = NULL;

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsHandleNotifyTranscTerminated: transc 0x%p of notification 0x%p was terminated.",
        hTransc, pNotify));

    eNotifySubsState = pNotify->eSubstate;

    SubsNotifyTerminate(pNotify, eReason);

    /* The RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR reason is set only when call-leg rejected the request
       (e.g. cseq too small). in this case we do not want to terminate the subscription...*/
 /*   if(eReason == RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR)
    {
        / * this is the case of notify(terminated) timeout * /
        RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsHandleNotifyTranscTerminated: notify 0x%p - eReason=transc error - terminate subs 0x%p.",
            pNotify, pSubs));
        SubsTerminate(pSubs, RVSIP_SUBS_REASON_TRANSC_ERROR);
        return RV_OK;
    }*/
    if(eNotifySubsState == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED)
    {
        if((eReason == RVSIP_SUBS_NOTIFY_REASON_TRANSC_TIMEOUT ||
            eReason == RVSIP_SUBS_NOTIFY_REASON_NETWORK_ERROR))
        {
            /* this is the case of notify(terminated) timeout */
            RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsHandleNotifyTranscTerminated: notify 0x%p - notify(terminated) eReason=timeout/error - terminate subs 0x%p.",
                pNotify, pSubs));
            SubsTerminate(pSubs, RVSIP_SUBS_REASON_TRANSC_TIME_OUT);
        }

        else if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATING &&
                eReason == RVSIP_SUBS_NOTIFY_REASON_APP_COMMAND)
        {
            /* this is that application terminated the notify, before final response
               was received - terminating the notify terminates the transaction, and then
               no timeout will be, and the subscription will stay in state TERMINATING forever */
            RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsHandleNotifyTranscTerminated: notify 0x%p - notify(terminated) terminated by app - terminate subs 0x%p in state TERMINATING.",
                pNotify, pSubs));
            SubsTerminate(pSubs, RVSIP_SUBS_REASON_UNDEFINED);
        }

    }
    return RV_OK;
}


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SipSubsGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetStateName (
                                      IN  RvSipSubsState  eState)
{
    return SubsGetStateName(eState);
}

/***************************************************************************
* SipSubsNotifyGetStatusName
* ------------------------------------------------------------------------
* General: Returns the name of a given status
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SipSubsNotifyGetStatusName (RvSipSubsNotifyStatus  eStatus)
{
    return SubsNotifyGetStatusName(eStatus);
}

/***************************************************************************
 * SipSubsReferGetNotifyReadyReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsReferGetNotifyReadyReasonName(
                               IN  RvSipSubsReferNotifyReadyReason  eReason)
{
    return SubsReferGetNotifyReadyReasonName(eReason);
}

/***************************************************************************
 * SipSubsGetChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipSubsGetChangeReasonName(
                                    IN  RvSipSubsStateChangeReason  eReason)
{
    return SubsGetChangeReasonName(eReason);
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#endif /* #ifdef RV_SIP_SUBS_ON - static functions */

#endif /*#ifndef RV_SIP_PRIMITIVES*/


