
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
 *                              <CallLegRefer.c>
 *
 *  Handles REFER process methods. Such as: ReferRequest, NotifyRequest,
 *                                          ReferAccept, ReferReject, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
#include "CallLegRefer.h"
#include "RvSipEventHeader.h"
#include "RvSipReferToHeader.h"
#include "CallLegMsgLoader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipCallLeg.h"
#include "CallLegCallbacks.h"
#include "_SipSubs.h"
#include "_SipSubsMgr.h"
#include "RvSipSubscription.h"
#include "CallLegSubs.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV CallLegReferFindReferSubs(IN  CallLeg         *pCallLeg,
                                                     IN  RvInt32          cseqStep,
                                                     OUT RvSipSubsHandle *phReferSubs);
static void RVCALLCONV CallLegReferHandleSubsTerminatedState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason);
static void RVCALLCONV CallLegReferHandleSubsActiveState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason);
static void RVCALLCONV CallLegReferHandleSubs2xxRcvdState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason);
static void RVCALLCONV CallLegReferHandleSubsMsgSendFailureState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason);
static void RVCALLCONV CallLegReferHandleSubsRcvdState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason);
static RvStatus RVCALLCONV CreateDefaultReferredByHeader(
                                     IN CallLeg*                      pCallLeg,
                                     IN RvSipSubsHandle               hSubs,
                                     OUT RvSipReferredByHeaderHandle* phReferredBy);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static const RvChar*  RVCALLCONV CallLegReferGetStateName (
                                      IN  RvSipCallLegReferState  eState);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegReferCreateSubsAndRefer
 * ------------------------------------------------------------------------
 * General: Sends a REFER associated with the call-leg. This function may be
 *          called only after the To and From header fields were set.
 *          Calling Refer causes a REFER request to be sent out and the
 *          call-leg refer state machine to progress to the Refer Sent
 *          state.
 *          This function is also used to send an authenticated refer
 *          request in the RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call leg the user wishes to send REFER.
 *          hReferTo     - The Refer-To header to be sent in the REFER request.
 *          hReferredBy  - The Referred-By header to be sent in the REFER request.
 *          strReferTo   - The Refer-To header in a string format.
 *          strReferredBy - The Referred-By header in a string format.
 *          strReplaces  - The Replaces for Refer-To header in a string format.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferCreateSubsAndRefer (
                                      IN  CallLeg                 *pCallLeg,
                                      IN  RvSipReferToHeaderHandle    hReferTo,
                                      IN  RvSipReferredByHeaderHandle hReferredBy,
                                      IN  RvChar             *strReferTo,
                                      IN  RvChar             *strReferredBy,
                                      IN  RvChar              *strReplaces)
{
    RvStatus               rv;
    RvSipReferredByHeaderHandle hReferredByToSet;

    /*Refer can be called only on state idle or connected of the call-leg,
      when all sub states are Idle*/

    if((pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED) ||
        ((pCallLeg->hActiveReferSubs != NULL) &&
        (SipSubsGetCurrState(pCallLeg->hActiveReferSubs) != RVSIP_SUBS_STATE_IDLE) &&
         (SipSubsGetCurrState(pCallLeg->hActiveReferSubs) != RVSIP_SUBS_STATE_UNAUTHENTICATED) &&
         (SipSubsGetCurrState(pCallLeg->hActiveReferSubs) != RVSIP_SUBS_STATE_REDIRECTED)))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegReferCreateSubsAndRefer - Failed for call-leg 0x%p: Illegal Action in state %s, refer sub state %s",
                  pCallLeg,CallLegGetStateName(pCallLeg->eState),
                  CallLegReferGetSubsStateName(pCallLeg)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* To and From headers must be set */
    if ((NULL == pCallLeg->hToHeader) ||
        (NULL == pCallLeg->hFromHeader))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegReferCreateSubsAndRefer - Failed for call-leg 0x%p: Illegal Refer when not all mandatory parameters are set",
                  pCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* if no refer subscription - create one, and initiate its parameters */
    if(NULL == pCallLeg->hActiveReferSubs)
    {
        rv = SipSubsMgrSubscriptionCreate(pCallLeg->pMgr->hSubsMgr,
                                          (RvSipCallLegHandle)pCallLeg,
                                          (void*)pCallLeg,
                                          RVSIP_SUBS_TYPE_SUBSCRIBER,
                                          RV_FALSE,
                                          &(pCallLeg->hActiveReferSubs)
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, -1 /* cctContext*/, 0xFF /*URI_cfg*/ /*will be determined by pCallLeg*/
#endif
//SPIRENT_END
                                          );
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegReferCreateSubsAndRefer - Failed for call-leg 0x%p: failed to create new refer subscription (rv=%d)",
                  pCallLeg, rv));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return rv;
        }

        if(NULL == hReferredBy && NULL == strReferredBy)
        {
            rv = CreateDefaultReferredByHeader(pCallLeg, pCallLeg->hActiveReferSubs, &hReferredByToSet);
            if(rv != RV_OK)
            {
                hReferredByToSet = hReferredBy;
            }
        }
        else
        {
            hReferredByToSet = hReferredBy;
        }

        rv = SipSubsReferInit(pCallLeg->hActiveReferSubs, hReferTo, hReferredByToSet, strReferTo, strReferredBy, strReplaces);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegReferCreateSubsAndRefer - Failed for call-leg 0x%p: failed to init new refer subscription (rv=%d)",
                  pCallLeg, rv));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return rv;
        }
    }

    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipSubsSetOutboundMsg(pCallLeg->hActiveReferSubs, pCallLeg->hOutboundMsg);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegReferCreateSubsAndRefer - call-leg 0x%p: failed to set outbound msg in subscription (rv=%d)",
                  pCallLeg, rv));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    /* when working with 'old refer' we will disable the no-notify timer,
       so subscription won't be terminated when not receiving notify */
    rv = RvSipSubsSetNoNotifyTimer(pCallLeg->hActiveReferSubs, 0);
    rv = RvSipSubsRefer(pCallLeg->hActiveReferSubs);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegReferCreateSubsAndRefer - Failed for call-leg 0x%p: failed to on sending the REFER. terminate call-leg (rv=%d:%s)",
              pCallLeg, rv, SipCommonStatus2Str(rv)));
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }
    return rv;
}


/***************************************************************************
 * CallLegReferChangeReferState
 * ------------------------------------------------------------------------
 * General: Changes the call-leg refer state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new refer sub state.
 *            eReason  - The refer sub state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegReferChangeReferState(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferState                eState,
                               IN  RvSipCallLegStateChangeReason         eReason)
{
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegReferChangeReferState - Call-leg 0x%p state changed to: %s, (reason = %s)"
              ,pCallLeg,
              CallLegReferGetStateName(eState),
              CallLegGetStateChangeReasonName(eReason)));

    /*notify the application about the new state*/
    CallLegCallbackChangeReferStateEv(pCallLeg, eState, eReason);

    if(RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE == eState ||
       RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED == eState)
    {
        pCallLeg->hActiveReferSubs = NULL;
    }
}


/***************************************************************************
* CallLegReferNotifyRequest
* ------------------------------------------------------------------------
* General: Sends a NOTIFY assosiated with the call-leg. The
*          call-leg refer sub state machine will progress to the Refer Notify
*          Sent state.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
*                                   transaction.
*               RV_ERROR_UNKNOWN - An error occurred while trying to send the
*                              message (Couldn't send a message to the given
*                            Request-Uri).
*               RV_OK - REFER message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - pointer to the call leg the user wishes to send REFER.
*          status   - The status code that will be used to create a status
*                     line for the NOTIFY request message body.
*          cSeqStep - The Cseq step of the REFER transaction that this
*                     NOTIFY relate to. This value will be set to the cseq
*                     parameter of the Event header of the NOTIFY request.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV CallLegReferNotifyRequest (IN  CallLeg            *pCallLeg,
                                                IN  RvInt16            status,
                                                IN  RvInt32            cSeqStep)
{
    RvSipSubsHandle     hSubs = NULL;
    RvSipNotifyHandle   hNotify;
    RvStatus           rv;

    rv = CallLegReferFindReferSubs(pCallLeg, cSeqStep, &hSubs);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegReferNotifyRequest - Failed for Call-leg 0x%p - can't find subs with id = %d",pCallLeg, cSeqStep));
        return rv;
    }

    /*-------------------------
      create a new notification
      -------------------------*/
    rv = RvSipSubsCreateNotify(hSubs, NULL, &hNotify);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferNotifyRequest - Failed for Call-leg 0x%p - failed to create notofication",pCallLeg));
        return rv;
    }
    rv = RvSipNotifySetReferNotifyBody(hNotify, status);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferNotifyRequest - Failed for Call-leg 0x%p - failed to set notify body", pCallLeg));
        rv = RvSipNotifyTerminate(hNotify);
        return rv;
    }

    /* set Subscription-State header */
    rv = RvSipNotifySetSubscriptionStateParams(hNotify,
                                               RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED,
                                               RVSIP_SUBSCRIPTION_REASON_NORESOURCE,
                                               UNDEFINED);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferNotifyRequest - Failed for Call-leg 0x%p - failed to set notify Subscription-State", pCallLeg));
        rv = RvSipNotifyTerminate(hNotify);
        return rv;
    }

    rv = RvSipNotifySend(hNotify);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferNotifyRequest - Failed for Call-leg 0x%p - failed to send notify 0x%p",pCallLeg, hNotify));
        rv = RvSipNotifyTerminate(hNotify);
        return rv;
    }

    return RV_OK;
}


/***************************************************************************
 * CallLegReferEnd
 * ------------------------------------------------------------------------
 * General: 1. When the application receives the Notify Ready notification
 *          from the RvSipCallLegReferNotifyEv callback it must decide
 *          weather to send a NOTIFY request to the REFER sender or not.
 *          If the application decides NOT to send a NOTIFY request to the
 *          REFER sender, it must call this function to complete the REFER
 *          process. If the application chooses to send a NOTIFY request to
 *          the REFER sender, it must not call this function.
 *          Note: If you call this function with a call-leg in the Idle state,
 *          the call-leg will be terminated.
 *          2. This function can be used in the Unauthenticated and Redirected
 *          refer states of the call-leg in order to complete the refer process
 *          without re-sending a refer request. Calling ReferEnd() from
 *          states Unauthenticated or Redirected will return the call-leg to
 *          Idle refer state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_OK - REFER process was completed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to send REFER.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferEnd (IN  CallLeg   *pCallLeg)
{
    if (RVSIP_CALL_LEG_STATE_IDLE == pCallLeg->eState)
    {
        /* Notify was not sent, and the call-leg is in Idle state,
           thus should be terminated */
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferEnd - Terminate Call-leg 0x%p without sending NOTIFY in Idle state",pCallLeg));
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    }
    else if (pCallLeg->hActiveReferSubs != NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferEnd - Call-leg 0x%p refer subs 0x%p is terminated"
                  ,pCallLeg, pCallLeg->hActiveReferSubs));
        SipSubsTerminate(pCallLeg->hActiveReferSubs);
        pCallLeg->hActiveReferSubs = NULL;
    }

    return RV_OK;
}


/***************************************************************************
 * CallLegReferAccept
 * ------------------------------------------------------------------------
 * General: Called to indicate that the application is willing to accept
 *          an incoming REFER request. This function will respond with
 *          202-Accepted response to the REFER request,
 *          create a new call-leg object, assosiate the new call-leg and
 *          the call-leg, and set to the new call-leg the following paramateres:
 *          Call-Id: the Call-Id of the REFER request, To: The Refer-To header
 *          of the REFER request, From: The local contact,  Referred-By: The
 *          Referred-By header of the RFEER request.
 *          To complete the acceptens of the REFER request the application must
 *          use the newly created call-leg and cal RvSipCallLegConnect on it.
 * Return Value: RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 202 response, or
 *                                   to create and inititialize the new call-leg).
 *               RV_OK       -  Accepted final response was sent successfully,
 *                                   and a new call-leg was created and initialized
 *                                   successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to accept refer from.
 *          hAppCallLeg - Application handle to the newly created call-leg.
 * Output:  phNewCallLeg - The new call-leg that is created and initialized by
 *                        this function.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferAccept (
                                      IN  CallLeg              *pCallLeg,
                                      IN  RvSipAppCallLegHandle hAppCallLeg,
                                      OUT RvSipCallLegHandle   *phNewCallLeg)
{
    RvStatus         rv;
    RvSipCommonStackObjectType eNewObjType;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegReferAccept - Call-leg 0x%p sending 202",pCallLeg));

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipSubsSetOutboundMsg(pCallLeg->hActiveReferSubs, pCallLeg->hOutboundMsg);
        if (rv != RV_OK)
        {
            /* if the function failed, it mesans that the outbound msg was not NULL.
               and this must be a bug, because no one should have access to this subs */
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegReferAccept - Call-leg 0x%p - Failed to set subs outbound msg(rv=%d:%s)",
                      pCallLeg, rv, SipCommonStatus2Str(rv)));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    rv = RvSipSubsReferAccept(pCallLeg->hActiveReferSubs, (void*)hAppCallLeg,
                              RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
                              &eNewObjType, (void**)phNewCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferAccept - Call-leg 0x%p - Failed in RvSipSubsReferAccept (rv=%d) - Terminating call",
                  pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if((rv == RV_OK) && phNewCallLeg) {
      CallLeg *pNewCallLeg=(CallLeg*)phNewCallLeg;
      pNewCallLeg->cctContext = pCallLeg->cctContext;
      pNewCallLeg->URI_Cfg_Flag = pCallLeg->URI_Cfg_Flag;
    }
#endif
//SPIRENT_END
    return RV_OK;

}


/***************************************************************************
 * CallLegReferReject
 * ------------------------------------------------------------------------
 * General: Can be used in the Refer Received refer state to reject an
 *          incoming REFER request.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg the user wishes to reject
 *            status   - The rejection response code.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferReject (IN  CallLeg       *pCallLeg,
                                         IN  RvUint16      status)
{
    RvStatus   rv;

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipSubsSetOutboundMsg(pCallLeg->hActiveReferSubs, pCallLeg->hOutboundMsg);
        if (rv != RV_OK)
        {
            /* if the function failed, it mesans that the outbound msg was not NULL.
               and this must be a bug, because no one should have access to this subs */
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegReferReject - Call-leg 0x%p - Failed to set subs outbound msg(rv=%d:%s)",
                      pCallLeg,rv,SipCommonStatus2Str(rv)));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    rv = RvSipSubsRespondReject(pCallLeg->hActiveReferSubs, status, NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferReject -  Call-leg 0x%p - Failed to send reject response (rv=%d) - - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    /*third-party REFER call-leg terminated automatically*/
    /*if (RVSIP_CALL_LEG_STATE_IDLE == pCallLeg->eState)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegReferReject - Call-leg 0x%p terminated automatically after rejecting a third party REFER request",
                  pCallLeg));
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    }
    pCallLeg->hActiveReferSubs = NULL;*/
    return RV_OK;

}

/***************************************************************************
 * CallLegReferNotifyReferGenerator
 * ------------------------------------------------------------------------
 * General: Notify the refer generator (if exists) about the call-leg received
 *          response.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call leg.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferNotifyReferGenerator (
                                         IN  CallLeg          *pCallLeg,
                                         IN  RvSipSubsReferNotifyReadyReason eReason,
                                         IN  RvSipMsgHandle     hResponseMsg)
{
    RvStatus       rv;

    if(pCallLeg->hReferSubsGenerator != NULL)
    {
        rv = SipSubsReferGeneratorCallNotifyReady(pCallLeg->hReferSubsGenerator,
                                             eReason,
                                             hResponseMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferNotifyReferGenerator: call-leg 0x%p - Failed to notify generator(rv=%d:%s)",
                pCallLeg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    return RV_OK;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * CallLegReferGetNotifyEventName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given notify sub state
 * Return Value: The string with the notify sub state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegReferGetNotifyEventName (
                                   IN  RvSipCallLegReferNotifyEvents  eState)
{
    switch(eState)
    {
    case RVSIP_CALL_LEG_REFER_NOTIFY_READY:
        return "Refer Notify Ready";
    case RVSIP_CALL_LEG_REFER_NOTIFY_SENT:
        return "Refer Notify Sent";
    case RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RCVD:
        return "Refer Notify Response Received";
    case RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED:
        return "Refer Notify Unauthenticated";
    case RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED:
        return "Refer Notify Redirected";
    case RVSIP_CALL_LEG_REFER_NOTIFY_RCVD:
        return "Refer Notify Received";
    default:
        return "Undefined";
    }
}

#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

/***************************************************************************
* CallLegReferGetCurrentReferSubsState
* ------------------------------------------------------------------------
* General: Gets the call-leg current refer subscription state;
* Return Value: RvSipSubsState
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
***************************************************************************/
RvSipSubsState RVCALLCONV CallLegReferGetCurrentReferSubsState (
                                     IN  CallLeg* pCallLeg)
{
    if(pCallLeg->hActiveReferSubs == NULL)
    {
        return RVSIP_SUBS_STATE_UNDEFINED;
    }
    else
    {
        return SipSubsGetCurrState(pCallLeg->hActiveReferSubs);
    }
}
/***************************************************************************
* CallLegReferGetCurrentReferState
* ------------------------------------------------------------------------
* General: Gets the call-leg current refer state
* Return Value: RvSipCallLegReferState
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
***************************************************************************/
RvSipCallLegReferState RVCALLCONV CallLegReferGetCurrentReferState (
                                     IN  RvSipCallLegHandle   hCallLeg)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;
    RvSipSubsState  eSubsState;

    if(pCallLeg->hActiveReferSubs == NULL)
    {
        return RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED;
    }

    eSubsState = SipSubsGetCurrState(pCallLeg->hActiveReferSubs);
    switch(eSubsState)
    {
        case RVSIP_SUBS_STATE_UNDEFINED:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED;
        case RVSIP_SUBS_STATE_IDLE:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE;
        case RVSIP_SUBS_STATE_SUBS_SENT:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_SENT;
        case RVSIP_SUBS_STATE_REDIRECTED:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_REDIRECTED;
        case RVSIP_SUBS_STATE_UNAUTHENTICATED:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED;
        case RVSIP_SUBS_STATE_SUBS_RCVD:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_RCVD;
        case RVSIP_SUBS_STATE_MSG_SEND_FAILURE:
            return RVSIP_CALL_LEG_REFER_STATE_MSG_SEND_FAILURE;
        default:
            return RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED;
    }
}

/*------------------------------------------------------------------------
                            REFER DNS
  ------------------------------------------------------------------------ */
/***************************************************************************
 * CallLegReferDNSGiveUp
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSGiveUp for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSGiveUp(IN  CallLeg *pCallLeg)
{
    RvStatus rv;

    rv = RvSipSubsDNSGiveUp(pCallLeg->hActiveReferSubs, NULL);

    return rv;
}

/***************************************************************************
 * CallLegReferDNSContinue
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSContinue for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSContinue(IN  CallLeg *pCallLeg)
{
    return RvSipSubsDNSContinue(pCallLeg->hActiveReferSubs, NULL);
}

/***************************************************************************
 * CallLegReferDNSReSend
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSReSendRequest for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSReSend(IN  CallLeg *pCallLeg)
{
    return RvSipSubsDNSReSendRequest(pCallLeg->hActiveReferSubs, NULL);
}

/***************************************************************************
 * CallLegReferDNSGetList
 * ------------------------------------------------------------------------
 * General: The function calls to RvSipSubsDNSGetList for hActiveReferSubs.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferDNSGetList(
                            IN  CallLeg                     *pCallLeg,
                            OUT RvSipTransportDNSListHandle *phDnsList)
{
    return RvSipSubsDNSGetList(pCallLeg->hActiveReferSubs, NULL, phDnsList);
}

/***************************************************************************
 * CallLegReferSubsStateChangedEvHandler
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription change state, in case application choose
 *          to disable the new refer behavior.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eState   - The new subscription state.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsStateChangedEvHandler(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsState             eState,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RvStatus rv;

    switch(eState)
    {
        case RVSIP_SUBS_STATE_REFRESH_RCVD:
        case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
            /* always respond with 501 not implemented */
            rv = RvSipSubsRespondReject(hSubs, 501, NULL);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferSubsStateChangedEvHandler: call-leg 0x%p - subs 0x%p Failed to respond 501",
                pCallLeg,hSubs));
            }
            return;
        case RVSIP_SUBS_STATE_SUBS_RCVD:
            CallLegReferHandleSubsRcvdState(pCallLeg, hSubs, eReason);
            return;
        case RVSIP_SUBS_STATE_SUBS_SENT:
            CallLegReferChangeReferState(pCallLeg, RVSIP_CALL_LEG_REFER_STATE_REFER_SENT,
                                        RVSIP_CALL_LEG_REASON_LOCAL_REFER);
            return;
        case RVSIP_SUBS_STATE_REDIRECTED:
            CallLegReferChangeReferState(pCallLeg, RVSIP_CALL_LEG_REFER_STATE_REFER_REDIRECTED,
                                        RVSIP_CALL_LEG_REASON_REDIRECTED);
            return;
        case RVSIP_SUBS_STATE_UNAUTHENTICATED:
            CallLegReferChangeReferState(pCallLeg, RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED,
                                        RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            return;
        case RVSIP_SUBS_STATE_MSG_SEND_FAILURE:
            CallLegReferHandleSubsMsgSendFailureState(pCallLeg, hSubs, eReason);
            return;
        case RVSIP_SUBS_STATE_2XX_RCVD:
            CallLegReferHandleSubs2xxRcvdState(pCallLeg, hSubs, eReason);
            return;
        case RVSIP_SUBS_STATE_ACTIVE:
            CallLegReferHandleSubsActiveState(pCallLeg, hSubs, eReason);
            return;
        case RVSIP_SUBS_STATE_TERMINATED:
            CallLegReferHandleSubsTerminatedState(pCallLeg, hSubs, eReason);
            return;

        case RVSIP_SUBS_STATE_IDLE:
        case RVSIP_SUBS_STATE_UNDEFINED:
        case RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD:
        case RVSIP_SUBS_STATE_TERMINATING:
        case RVSIP_SUBS_STATE_PENDING:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferSubsStateChangedEvHandler: call-leg 0x%p  refer subs 0x%p- nothing to do with state %s ",
                pCallLeg,hSubs, SipSubsGetStateName(eState)));
            return;
        case RVSIP_SUBS_STATE_REFRESHING:
        case RVSIP_SUBS_STATE_UNSUBSCRIBING:
        case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
        case RVSIP_SUBS_STATE_ACTIVATED:
        default:
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferSubsStateChangedEvHandler: call-leg 0x%p  refer subs 0x%p- unexpected state %s ",
                pCallLeg,hSubs, SipSubsGetStateName(eState)));
            return;

    }
}



/***************************************************************************
 * CallLegReferSubsNotifyStateChangedEv
 * ------------------------------------------------------------------------
 * General: Handles the refer notify state.
 *          The function wraps the notify statuses, and call to the old
 *          refer notify event.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hNotification - The new created notification object handle.
 *          eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received msg (notify request or response).
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsNotifyStateChangedEv(
                                   IN  CallLeg*           pCallLeg,
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipNotifyHandle  hNotification,
                                   IN  RvSipSubsNotifyStatus eNotifyStatus,
                                   IN  RvSipSubsNotifyReason eNotifyReason,
                                   IN  RvSipMsgHandle     hNotifyMsg)
{
    RvSipCallLegReferNotifyEvents eReferNotifyStatus;
    RvSipCallLegStateChangeReason eReferReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    RvInt16                      referStatusToInform;
    RvUint16                     referStatus;
    RvInt32                      cseq;
    RvStatus                     rv;
    RvInt16 statusCode;

    SipSubsReferGetEventId(hSubs, &cseq);

    switch(eNotifyStatus)
    {
    case RVSIP_SUBS_NOTIFY_STATUS_REQUEST_SENT:
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_SENT;
        eReferReason = RVSIP_CALL_LEG_REASON_LOCAL_REFER_NOTIFY;
        referStatus = SipSubsReferGetFinalStatus(hSubs);
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD:
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_RCVD;
        eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_REFER_NOTIFY;
        referStatus = SipSubsReferGetFinalStatus(hSubs);
        cseq = UNDEFINED;
        /* old implementation accepted the notify request with 200
           before informing application about the incoming request */
        rv = RvSipNotifyAccept(hNotification);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferSubsNotifyStateChangedEv - Call-leg 0x%p - Failed to send 200 response (rv=%d)",
                pCallLeg, rv));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return;
        }
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_UNAUTHENTICATED:
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_UNAUTHENTICATED;
        eReferReason = RVSIP_CALL_LEG_REASON_REQUEST_FAILURE;
        referStatus = SipSubsReferGetFinalStatus(hSubs);
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_REDIRECTED:
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_REDIRECTED;
        eReferReason = RVSIP_CALL_LEG_REASON_REDIRECTED;
        referStatus = SipSubsReferGetFinalStatus(hSubs);
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_2XX_RCVD:
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RCVD;
        eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED;
        referStatus = 0;
        cseq = UNDEFINED;
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_REJECT_RCVD:
        statusCode = RvSipMsgGetStatusCode(hNotifyMsg);
        if(statusCode >= 400 && statusCode< 500)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_REQUEST_FAILURE;
        }
        if(statusCode >= 500 && statusCode< 600)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_SERVER_FAILURE;
        }
        if(statusCode >= 600 && statusCode< 700)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE;
        }
        eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RCVD;
        referStatus = 0;
        cseq = UNDEFINED;
        break;
    case RVSIP_SUBS_NOTIFY_STATUS_MSG_SEND_FAILURE:
        if(RVSIP_SUBS_NOTIFY_REASON_503_RECEIVED == eNotifyReason)
        {
            eReferNotifyStatus = RVSIP_CALL_LEG_REFER_NOTIFY_RESPONSE_RCVD;
            eReferReason = RVSIP_CALL_LEG_REASON_SERVER_FAILURE;
            referStatus = 0;
            cseq = UNDEFINED;
            break;
        }
    case RVSIP_SUBS_NOTIFY_STATUS_IDLE:
    case RVSIP_SUBS_NOTIFY_STATUS_TERMINATED:
    case RVSIP_SUBS_NOTIFY_STATUS_FINAL_RESPONSE_SENT:
    case RVSIP_SUBS_NOTIFY_STATUS_UNDEFINED :
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferSubsNotifyStateChangedEv: call-leg 0x%p - subs 0x%p notify 0x%p, nothing to do with state %s",
            pCallLeg, hSubs, hNotification, SipSubsNotifyGetStatusName(eNotifyStatus)));
        return;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferSubsNotifyStateChangedEv: call-leg 0x%p - subs 0x%p notify 0x%p, unexpected state %s",
            pCallLeg, hSubs, hNotification, SipSubsNotifyGetStatusName(eNotifyStatus)));
        return;
    }

    if(referStatus > 0)
    {
        referStatusToInform = (RvInt16)referStatus;
    }
    else
    {
        referStatusToInform = UNDEFINED;
    }
    CallLegCallbackReferNotifyEv(pCallLeg,
                            eReferNotifyStatus,
                            eReferReason,
                            referStatusToInform,
                            cseq);
}
/***************************************************************************
 * CallLegReferSubsNotifyReadyEv
 * ------------------------------------------------------------------------
 * General: Indicates that a refer notifier subscription is ready to
 *          send a notify request.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - The sip stack subscription handle
 *            hAppSubs      - The application handle for this subscription.
 *          eReason       - The reason for a NOTIFY request to be sent
 *          responseCode  - The response code that should be set in the
 *                          NOTIFY message body.
 *          hResponseMsg  - The message that was received on the refer related
 *                          object (provisional or final response).
 ***************************************************************************/
void RVCALLCONV CallLegReferSubsNotifyReadyEv(
                           IN  CallLeg*                         pCallLeg,
                           IN  RvSipSubsHandle                  hSubs,
                           IN  RvSipSubsReferNotifyReadyReason  eReason,
                           IN  RvInt16                         responseCode,
                           IN  RvSipMsgHandle                   hResponseMsg)
{
    RvSipCallLegStateChangeReason eReferReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    RvInt16                      status;
    RvInt32                      cseq;

    SipSubsReferGetEventId(hSubs, &cseq);
    status = UNDEFINED;

    switch(eReason)
    {
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_FINAL_RESPONSE_MSG_RCVD:
        status = responseCode;
        if(status >= 200 && status < 300)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED;
        }
       /* else if(status >= 300 && status < 400)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED;
        }*/
        else if(status >= 400 && status < 500)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_REQUEST_FAILURE;
        }
        else if(status >= 500 && status < 600)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_SERVER_FAILURE;
        }
        else if(status >= 600 && status < 700)
        {
            eReferReason = RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE;
        }
        break;
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNSUPPORTED_AUTH_PARAMS:
        status = RvSipMsgGetStatusCode(hResponseMsg); /*401 or 407 */
        eReferReason = RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS;
        break;
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_OBJ_TERMINATED:
        eReferReason = RVSIP_CALL_LEG_REASON_CALL_TERMINATED;
        break;
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_TIMEOUT:
        eReferReason = RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT;
        break;
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_ERROR_TERMINATION:
        eReferReason = RVSIP_CALL_LEG_REASON_LOCAL_FAILURE;
        break;
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_INITIAL_NOTIFY:
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_1XX_RESPONSE_MSG_RCVD:
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNDEFINED:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferSubsNotifyReadyEv: pCallLeg 0x%p - subs 0x%p - nothing to do with reason %s",
            pCallLeg, hSubs, SipSubsReferGetNotifyReadyReasonName(eReason)));
        return;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferSubsNotifyReadyEv: pCallLeg 0x%p - subs 0x%p - unexpected reason %s",
            pCallLeg, hSubs, SipSubsReferGetNotifyReadyReasonName(eReason)));
        return;
    }

    CallLegCallbackReferNotifyEv(pCallLeg,
                            RVSIP_CALL_LEG_REFER_NOTIFY_READY,
                            eReferReason,
                            status,
                            cseq);
    if(NULL == pCallLeg->callLegEvHandlers->pfnReferNotifyEvHandler)
    {
        if(eReferReason == RVSIP_CALL_LEG_REASON_LOCAL_FAILURE)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferSubsNotifyReadyEv - call-leg 0x%p - ReferEnd. local Failure reason + app did not register to notify cb",
                pCallLeg));
            CallLegReferEnd(pCallLeg);
        }
    }
}

/***************************************************************************
 * CallLegReferSubsOtherURLAddressFoundEvHandler
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 *          The function wraps the subscription callback with call-leg callback.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - Pointer to the call-leg
 *            hSubs          - The handle for this subscription.
 *            hNotify        - The notify object handle (relevant only for notify request or response)
 *          hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferSubsOtherURLAddressFoundEvHandler(
                                    IN  CallLeg                 *pCallLeg,
                                    IN  RvSipSubsHandle            hSubs,
                                    IN  RvSipNotifyHandle        hNotify,
                                    IN  RvSipMsgHandle          hMsg,
                                    IN  RvSipAddressHandle        hAddress,
                                    OUT RvSipAddressHandle        hSipURLAddress,
                                    OUT RvBool                    *bAddressResolved)
{
    RvSipTranscHandle hTransc = NULL;

    if(hNotify != NULL)
    {
        hTransc = SipSubsNotifyGetTransc(hNotify);
    }
    else
    {
        hTransc = SipSubsGetTransc(hSubs);
    }

    if(hTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferSubsOtherURLAddressFoundEvHandler: call-leg 0x%p - subs 0x%p notify 0x%p. transc is NULL",
            pCallLeg, hSubs, hNotify));
    }

    return CallLegCallbackTranscOtherURLAddressFoundEv(pCallLeg, hTransc, hMsg,
                                                       hAddress, hSipURLAddress,
                                                       bAddressResolved);
}


/***************************************************************************
 * CallLegReferSetReferredBy
 * ------------------------------------------------------------------------
 * General: Sets the Referred-By header to the call-leg,
 *          this header will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg.
 *           hReferredByHeader - Handle to the Referred-By hedaer to be set.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegReferSetReferredBy (
                                    IN  CallLeg                     *pCallLeg,
                                    IN  RvSipReferredByHeaderHandle  hReferredByHeader)

{
    RvStatus                   rv;
    RvSipReferredByHeaderHandle hTempReferredBy;

    if (NULL == hReferredByHeader)
    {
        pCallLeg->hReferredByHeader = NULL;
        return RV_OK;
    }

    if(pCallLeg->hReferredByHeader == NULL)
    {
        rv = RvSipReferredByHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                            pCallLeg->pMgr->hGeneralPool,
                                            pCallLeg->hPage,
                                            &hTempReferredBy);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegReferSetReferredBy - Call-leg 0x%p - Failed to construct Referred-By header (rv=%d)",
                      pCallLeg,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        hTempReferredBy = pCallLeg->hReferredByHeader;
    }

    /*copy the given header to the call-leg Referred-By header */
    rv = RvSipReferredByHeaderCopy(hTempReferredBy, hReferredByHeader);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegReferSetReferredBy - Call-leg 0x%p - Failed to copy header (rv=%d)",
                  pCallLeg,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pCallLeg->hReferredByHeader = hTempReferredBy;
    return RV_OK;
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegReferFindReferSubs
* ------------------------------------------------------------------------
* General: Search for a refer subscription with the given event id value.
* Return Value: RV_ERROR_NOT_FOUND -  subscription was not found.
*                RV_OK - subscription was found.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - pointer to the call leg holding the subscriptions list..
*          cSeqStep - The Cseq step of the REFER transaction that this
*                     NOTIFY relate to. This value helps ua to find the
*                     correct subscription.
* Output:  phReferSubs - The refer subscription that was found.
***************************************************************************/
static RvStatus RVCALLCONV CallLegReferFindReferSubs(IN  CallLeg         *pCallLeg,
                                                     IN  RvInt32          cseqStep,
                                                     OUT RvSipSubsHandle *phReferSubs)
{
    RvSipEventHeaderHandle  hEvent;
    RvSipSubsHandle         hSubs = NULL;
    RvChar                  cseqStepStr[12];
    RvStatus                rv;
    HPAGE                   tempPage;

    *phReferSubs = NULL;
    rv = RPOOL_GetPage(pCallLeg->pMgr->hGeneralPool, 0, &tempPage);
    if(RV_OK != rv)
    {
        return rv;
    }
    rv = RvSipEventHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                   pCallLeg->pMgr->hGeneralPool,
                                   tempPage, &hEvent);
    if(RV_OK != rv)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, &tempPage);
        return rv;
    }
    rv = RvSipEventHeaderSetEventPackage(hEvent, "refer");
    if(RV_OK != rv)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, &tempPage);
        return rv;
    }
#if defined(UPDATED_BY_SPIRENT)
    if(cseqStep!=-1)
    {
       RvSprintf(cseqStepStr, "%d%c", cseqStep, '\0');
       rv = RvSipEventHeaderSetEventId(hEvent, cseqStepStr);
       if(RV_OK != rv)
       {
           RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, &tempPage);
           return rv;
       }
    }
#else
    RvSprintf(cseqStepStr, "%d%c", cseqStep, '\0');
    rv = RvSipEventHeaderSetEventId(hEvent, cseqStepStr);
    if(RV_OK != rv)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, &tempPage);
        return rv;
    }
#endif
    rv = CallLegSubsFindSubscription((RvSipCallLegHandle)pCallLeg, hEvent, RVSIP_SUBS_TYPE_NOTIFIER, &hSubs);
    RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, tempPage);

    if(hSubs == NULL || rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferFindReferSubs - call-leg 0x%p - Failed to find subs with event id %d",
            pCallLeg,cseqStep));
        return RV_ERROR_NOT_FOUND;
    }
    else
    {
        *phReferSubs = hSubs;
        return RV_OK;
    }
}



/***************************************************************************
 * CallLegReferHandleSubsRcvdState
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription SUBS_RCVD.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV CallLegReferHandleSubsRcvdState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RvStatus rv;
    RvSipCallLegStateChangeReason eReferReason;
    RvSipMethodType eMethod = RVSIP_METHOD_UNDEFINED;
    RvSipReferToHeaderHandle  hReferTo;
    RvSipAddressHandle        hUrl;

    if(pCallLeg->hActiveReferSubs != NULL && hSubs != pCallLeg->hActiveReferSubs)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsRcvdState: call-leg 0x%p - hActiveReferSubs 0x%p. reject subs 0x%p with 400",
            pCallLeg, pCallLeg->hActiveReferSubs, hSubs));
        rv = RvSipSubsRespondReject(hSubs, 400, "Handling another REFER");
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsRcvdState: call-leg 0x%p - subs 0x%p Failed to respond 400 on second REFER",
            pCallLeg, hSubs));
        }
        return;
    }

    rv = RvSipSubsGetReferToHeader(hSubs, &hReferTo);
    hUrl = RvSipReferToHeaderGetAddrSpec(hReferTo);

    if(RvSipAddrGetAddrType(hUrl) == RVSIP_ADDRTYPE_URL)
    {
        eMethod = RvSipAddrUrlGetMethod(hUrl);
        if(eMethod != RVSIP_METHOD_INVITE && eMethod != RVSIP_METHOD_UNDEFINED)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegReferHandleSubsRcvdState: call-leg 0x%p, hActiveSubs=0x%p, hSubs=0x%p - unsupported method in Refer-To header. respond with 400",
                pCallLeg, pCallLeg->hActiveReferSubs, hSubs));
            rv = RvSipSubsRespondReject(hSubs, 400, NULL);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegReferHandleSubsRcvdState: call-leg 0x%p - subs 0x%p Failed to respond 400 (unsupported method)",
                    pCallLeg, hSubs));
            }
            return;
        }
    }
    if(eReason == RVSIP_SUBS_REASON_REFER_RCVD_WITH_REPLACES)
    {
        eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_REFER_REPLACES;
    }
    else if(eReason == RVSIP_SUBS_REASON_REFER_RCVD)
    {
        eReferReason = RVSIP_CALL_LEG_REASON_REMOTE_REFER;
    }
    else
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsRcvdState: call-leg 0x%p subs 0x%p - illegal reason %s",
            pCallLeg, hSubs, SipSubsGetChangeReasonName(eReason)));
        return;
    }

    pCallLeg->hActiveReferSubs = hSubs;

    CallLegReferChangeReferState(pCallLeg, RVSIP_CALL_LEG_REFER_STATE_REFER_RCVD, eReferReason);
}

/***************************************************************************
 * CallLegReferHandleSubsSentState
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription SUBS_SENT.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV CallLegReferHandleSubsMsgSendFailureState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RvSipCallLegStateChangeReason eReferReason;

    if(hSubs != pCallLeg->hActiveReferSubs)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsMsgSendFailureState: call-leg 0x%p - hActiveReferSubs 0x%p.different than hSubs 0x%p",
            pCallLeg, pCallLeg->hActiveReferSubs, hSubs));
        return;
    }
    switch (eReason)
    {
    case RVSIP_SUBS_REASON_NETWORK_ERROR:
        eReferReason = RVSIP_CALL_LEG_REASON_NETWORK_ERROR;
        break;
    case RVSIP_SUBS_REASON_503_RECEIVED:
        eReferReason = RVSIP_CALL_LEG_REASON_503_RECEIVED;
        break;
    case RVSIP_SUBS_REASON_TRANSC_TIME_OUT:
        eReferReason = RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT;
        break;
    default:
        eReferReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    }

    CallLegReferChangeReferState(pCallLeg,
                                RVSIP_CALL_LEG_REFER_STATE_MSG_SEND_FAILURE,
                                eReferReason);

}

/***************************************************************************
 * CallLegReferHandleSubs2xxRcvdState
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription SUBS_2XX_RCVD.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV CallLegReferHandleSubs2xxRcvdState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(eReason);

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        CallLegReferChangeReferState(pCallLeg,
                                RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED,
                                RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);
    }
    else /* call-leg is connected or established */
    {
        CallLegReferChangeReferState(pCallLeg,
                                RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE,
                                RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);
    }
}

/***************************************************************************
 * CallLegReferHandleSubsActiveState
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription SUBS_ACTIVE.
 *          The function wraps the subscription states with old refer states.
 *          refer subscription reaches the active state, after sending 202
 *          response on a REFER request.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV CallLegReferHandleSubsActiveState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RV_UNUSED_ARG(hSubs);

    if(eReason != RVSIP_SUBS_REASON_UNDEFINED)
    {
        /* this will be after accepting notify(pending) which is not
        relevant for us */
        return;
    }
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        CallLegReferChangeReferState(pCallLeg,
                                RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED,
                                RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED);
    }
    else /* call-leg is connected or established */
    {
        CallLegReferChangeReferState(pCallLeg,
                                RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE,
                                RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED);
    }
}

/***************************************************************************
 * CallLegReferHandleSubsTerminatedState
 * ------------------------------------------------------------------------
 * General: Handle a refer subscription SUBS_TERMINATED.
 *          The function wraps the subscription states with old refer states.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The Call-leg
 *            hSubs    - The handle for this subscription.
 *            eReason  - The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV CallLegReferHandleSubsTerminatedState(
                                   IN  CallLeg*                   pCallLeg,
                                   IN  RvSipSubsHandle            hSubs,
                                   IN  RvSipSubsStateChangeReason eReason)
{
    RvSipCallLegReferState eNewState;
    RvSipCallLegStateChangeReason eNewReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    RvUint16                responseCode;

    if(pCallLeg->hActiveReferSubs != hSubs)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsTerminatedState: call-leg 0x%p - hSubs 0x%p different than hActiveSubs 0x%p",
            pCallLeg, hSubs, pCallLeg->hActiveReferSubs));
        return;
    }
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE ||
       (pCallLeg->bIsHidden == RV_TRUE && pCallLeg->bIsReferCallLeg == RV_TRUE))

    {
        eNewState = RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED;
    }
    else /* call-leg is connected or established */
    {
        eNewState = RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE;
    }

    switch(eReason)
    {
    case RVSIP_SUBS_REASON_LOCAL_REJECT:
        eNewReason = RVSIP_CALL_LEG_REASON_LOCAL_REJECT;
        break;
    case RVSIP_SUBS_REASON_REMOTE_REJECT:
        responseCode = SipSubsReferGetFinalStatus(hSubs);
        if(responseCode >= 400 && responseCode < 500)
        {
            eNewReason = RVSIP_CALL_LEG_REASON_REQUEST_FAILURE;
        }
        else if(responseCode >= 500 && responseCode < 600)
        {
            eNewReason = RVSIP_CALL_LEG_REASON_SERVER_FAILURE;
        }
        else if(responseCode >= 600 && responseCode < 700)
        {
            eNewReason = RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE;
        }
        break;
    case RVSIP_SUBS_REASON_GIVE_UP_DNS:
        eNewReason = RVSIP_CALL_LEG_REASON_GIVE_UP_DNS;
        break;
    case RVSIP_SUBS_REASON_TRANSC_TIME_OUT:
        eNewReason = RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT;
        break;
    case RVSIP_SUBS_REASON_NO_NOTIFY_TIME_OUT:
        eNewReason = RVSIP_CALL_LEG_REASON_LOCAL_FAILURE;
        break;
    case RVSIP_SUBS_REASON_DIALOG_TERMINATION:
        eNewReason = RVSIP_CALL_LEG_REASON_CALL_TERMINATED;
        break;
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_SENT:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_DEACTIVATED_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_PROBATION_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_REJECTED_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_TIMEOUT_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_GIVEUP_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_TERMINATED_NORESOURCE_RCVD:
    case RVSIP_SUBS_REASON_NOTIFY_2XX_SENT:
    case RVSIP_SUBS_REASON_NOTIFY_2XX_RCVD:
    case RVSIP_SUBS_REASON_SUBS_TERMINATED:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsTerminatedState: call-leg 0x%p - hSubs 0x%p nothing to do with reason %s",
            pCallLeg, hSubs, SipSubsGetChangeReasonName(eReason)));
        return;

    default:
         RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegReferHandleSubsTerminatedState: call-leg 0x%p - hSubs 0x%p unhandled reason %d",
            pCallLeg, hSubs, eReason));
         return;
    }

    CallLegReferChangeReferState(pCallLeg, eNewState, eNewReason);

    if(eNewState == RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE)
    {
        /* the end of a refer procedure on non-idle call-leg */
        pCallLeg->hActiveReferSubs = NULL;
    }
}

/***************************************************************************
 * CreateDefaultReferredByHeader
 * ------------------------------------------------------------------------
 * General: Create a default Referred-By header, in case application did not
 *          supplied one.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 ***************************************************************************/
static RvStatus RVCALLCONV CreateDefaultReferredByHeader(
                                     IN CallLeg*                      pCallLeg,
                                     IN RvSipSubsHandle               hSubs,
                                     OUT RvSipReferredByHeaderHandle* phReferredBy)
{
    RvSipAddressHandle hTempAddr;
    RvStatus rv;

    rv = RvSipSubsGetNewHeaderHandle(hSubs, RVSIP_HEADERTYPE_REFERRED_BY, (void**)phReferredBy);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CreateDefaultReferredByHeader - Call Leg 0x%p , Failed to construct a Referred-By header"
             ,pCallLeg));
        return rv;
    }

    if (RVSIP_CALL_LEG_DIRECTION_INCOMING == pCallLeg->eDirection)
    {
        hTempAddr = RvSipPartyHeaderGetAddrSpec(pCallLeg->hToHeader);
    }
    else
    {
        hTempAddr = RvSipPartyHeaderGetAddrSpec(pCallLeg->hFromHeader);
    }
    if (NULL != hTempAddr)
    {
        rv = RvSipReferredByHeaderSetAddrSpec(*phReferredBy, hTempAddr);
    }
    else
    {
        rv = RV_ERROR_UNKNOWN;
    }
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CreateDefaultReferredByHeader - Call Leg 0x%p , Failed to update Referred-By default value for the call-leg"
                 ,pCallLeg));
        return rv;
    }
    return RV_OK;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * CallLegReferGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given refer sub state
 * Return Value: The string with the refer sub state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
static const RvChar*  RVCALLCONV CallLegReferGetStateName (
                                      IN  RvSipCallLegReferState  eState)
{
    switch(eState)
    {
    case RVSIP_CALL_LEG_REFER_STATE_REFER_UNDEFINED:
        return "Refer Undefined";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE:
        return "Refer Idle";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_SENT:
        return "Refer Sent";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_CANCELLING:
        return "Refer Cancelling";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_RCVD:
        return "Refer Received";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED:
        return "Refer Unauthenticated";
    case RVSIP_CALL_LEG_REFER_STATE_REFER_REDIRECTED:
        return "Refer Redirected";
    case RVSIP_CALL_LEG_REFER_STATE_MSG_SEND_FAILURE:
        return "Refer Msg Send Failure";
    default:
        return "Undefined";
    }
}

/***************************************************************************
 * CallLegReferGetSubsStateName
 * ------------------------------------------------------------------------
 * General: The function returns the hActiveReferSubs state as string.
 *          (The function is used to print the state to log)
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
const RvChar* RVCALLCONV CallLegReferGetSubsStateName(IN  CallLeg *pCallLeg)
{
    return SipSubsGetStateNameOfSubs(pCallLeg->hActiveReferSubs);
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* ifndef RV_SIP_PRIMITIVES */



