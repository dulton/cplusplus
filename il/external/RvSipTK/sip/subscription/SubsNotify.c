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
 *                              <SubsNotify.c>
 *
 * This file defines the functions for notifications handling.
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
#ifdef RV_SIP_SUBS_ON

#include "SubsCallbacks.h"
#include "SubsNotify.h"
#include "_SipCallLeg.h"
#include "_SipCallLegMgr.h"
#include "RvSipSubscriptionStateHeader.h"
#include "_SipSubscriptionStateHeader.h"
#include "RvSipMsg.h"
#include "_SipMsg.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define SUBS_NOTIFY_STATUS_TERMINATED_STR "Notify Terminated"
#define SUBS_NOTIFY_OBJECT_NAME_STR "Notification"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus UpdateSubsAfter2xxOnNotifySent(IN Subscription* pSubs,
                                                IN Notification* pNotify,
                                                OUT RvBool*      bSubsDeleted);

static RvSipSubsStateChangeReason NotifyReason2MsgReason(RvSipSubscriptionReason eReason);

static RvStatus Handle2xxOnNotifyPendingSent(IN Subscription* pSubs,
                                              OUT RvSipSubsState* peNewState,
                                              OUT RvBool*        pbFinished);

static RvStatus Handle2xxOnNotifyActiveSent(IN Subscription* pSubs,
                                             OUT RvSipSubsState* peNewState,
                                             OUT RvBool*        pbFinished);

static RvStatus Handle2xxOnNotifyTerminatedSent(IN Subscription* pSubs,
                                                 OUT RvSipSubsState* peNewState,
                                                 OUT RvBool*        pbFinished);

static RvInt32 AllocCopyRpoolString(IN Subscription* pSubs,
                                     IN HRPOOL   destPool,
                                     IN HPAGE    destPage,
                                     IN HRPOOL   sourcePool,
                                     IN HPAGE    sourcePage,
                                     IN RvInt32 stringOffset);
static RvStatus RVCALLCONV SubsNotifyCreateTransc(IN  Notification*      pNotify);
static RvStatus RVCALLCONV NotifyPrepareMsgBeforeSending (IN  Notification *pNotify);

static RvStatus SubsNotifyFree(IN  Notification* pNotify);

static RvStatus RVCALLCONV TerminateNotifyEventHandler(IN void *obj, 
                                                       IN RvInt32  reason);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SubsNotifyCreate
* ------------------------------------------------------------------------
* General: Creates a new notification object, related to a given subscription
*          and exchange handles with the application notification object.
*
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
*               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
*               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs      - Pointer to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyCreate(IN  Subscription*         pSubs,
                                      IN  RvSipAppNotifyHandle hAppNotify,
                                      OUT RvSipNotifyHandle    *phNotify)
{
    RvStatus          rv;

    /*--------------------------------
     create a new notification object
    ----------------------------------*/
    rv = SubsNotifyCreateObjInSubs(pSubs, hAppNotify, phNotify);
    if (rv != RV_OK )
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyCreate - Subscription 0x%p - failed to create notification object",pSubs));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
* SubsNotifyCreateObjInSubs
* ------------------------------------------------------------------------
* General: Creates a new notification object, related to a given subscription.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyCreateObjInSubs(IN  Subscription*        pSubs,
                                              IN  RvSipAppNotifyHandle hAppNotify,
                                              OUT RvSipNotifyHandle    *phNotify)
{
    RvStatus          rv;
    Notification*      pNotify;
    RLIST_ITEM_HANDLE  listItem;

    /*--------------------------------
     create a new notification object
    ----------------------------------*/
    RvMutexLock(&pSubs->pMgr->hMutex,pSubs->pMgr->pLogMgr);
    rv = RLIST_InsertTail(pSubs->pMgr->hNotifyListPool,pSubs->hNotifyList,&listItem);
    RvMutexUnlock(&pSubs->pMgr->hMutex,pSubs->pMgr->pLogMgr);
    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyCreateObjInSubs - Subscription 0x%p - Failed to insert new notification to list (rv=%d)", pSubs, rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pNotify = (Notification*)listItem;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsNotifyCreateObjInSubs - Subs 0x%p - new notify 0x%p was created",
        pSubs, pNotify));

    pNotify->hAppNotify   = hAppNotify;
    pNotify->pSubs        = pSubs;
    pNotify->hOutboundMsg = NULL;
    pNotify->eSubstate    = RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED;
    pNotify->expiresParam = UNDEFINED;
    pNotify->eReason      = RVSIP_SUBSCRIPTION_REASON_UNDEFINED;
    pNotify->strReason    = UNDEFINED;

    pNotify->hTransc      = NULL;
    pNotify->eStatus      = RVSIP_SUBS_NOTIFY_STATUS_IDLE;
    pNotify->eTerminationReason = RVSIP_SUBS_NOTIFY_REASON_UNDEFINED;
    pNotify->retryAfter   = 0;
    pNotify->pfnNotifyEvHandler = &(pSubs->subsEvHandlers->pfnNotifyEvHandler);
    memset(&(pNotify->terminationInfo), 0, sizeof(RvSipTransportObjEventInfo));

    *phNotify = (RvSipNotifyHandle)pNotify;
    ++pSubs->numOfNotifyObjs;
    return RV_OK;
}

/***************************************************************************
 * SubsNotifyGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message (request or response) that is going to be sent
 *          by the notification. You may get the message only in states where
 *          notification may send a message.
 *          You can call this function before you call a control API functions
 *          that sends a message (such as RvSipNotifySend() or
 *          RvSipNotifyAccept()).
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify -  The notification handle.
 * Output:     phMsg   -  pointer to the message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyGetOutboundMsg(
                                     IN  Notification   *pNotify,
                                     OUT RvSipMsgHandle    *phMsg)
{
    RvStatus       rv;
    RvSipMsgHandle  hOutboundMsg;

    if (NULL != pNotify->hOutboundMsg)
    {
        hOutboundMsg = pNotify->hOutboundMsg;
    }
    else
    {
        rv = RvSipMsgConstruct(pNotify->pSubs->pMgr->hMsgMgr,
                               pNotify->pSubs->pMgr->hGeneralPool,
                               &hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                       "SubsNotifyGetOutboundMsg - notify 0x%p error in constructing message",
                      pNotify));
            return rv;
        }
        pNotify->hOutboundMsg = hOutboundMsg;
    }

    *phMsg = hOutboundMsg;

    return RV_OK;
}

/***************************************************************************
 * SubsNotifySetSubscriptionStateParams
 * ------------------------------------------------------------------------
 * General: Creates the Subscription-State header in the outbound NOTIFY request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_UNKNOWN       -  Failed to set the notify body.
 *               RV_OK       -  set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify    - Handle to the notification object.
 *          eSubsState - SubsState to set in Subscription-State header.
 *          eReason    - reason to set in Subscription-State header
 *                       (RVSIP_SUBSCRIPTION_REASON_UNDEFINED for no reason).
 *          expiresParamVal - expires parameter value to set in Subscription-State
 *                       header (may be UNDEFINED)
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifySetSubscriptionStateParams(
                                            IN  Notification            *pNotify,
                                            IN  RvSipSubscriptionSubstate eSubsState,
                                            IN  RvSipSubscriptionReason eReason,
                                            IN  RvInt32                expiresParamVal)
{
    pNotify->eSubstate = eSubsState;
    pNotify->eReason = eReason;
    pNotify->expiresParam = expiresParamVal;

    /*rv = RvSipSubscriptionStateHeaderConstruct(pNotify->pSubs->pMgr->hMsgMgr,
                                               pNotify->pSubs->pMgr->hGeneralPool,
                                               pNotify->pSubs->hPage,
                                               &(pNotify->hSubscriptionStateHeader));
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
          "SubsNotifySetSubscriptionStateParams - notify 0x%p , Failed to construct Subscription-State header in notification",
          pNotify));
        return rv;
    }

    rv = RvSipSubscriptionStateHeaderSetSubstate(pNotify->hSubscriptionStateHeader, eSubsState, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
          "SubsNotifySetSubscriptionStateParams - notify 0x%p , Failed to set subsState in header",
          pNotify));
        return rv;
    }

    if(eReason != RVSIP_SUBSCRIPTION_REASON_UNDEFINED)
    {
        rv = RvSipSubscriptionStateHeaderSetReason(pNotify->hSubscriptionStateHeader, eReason, NULL);
        if(rv != RV_OK)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "SubsNotifySetSubscriptionStateParams - notify 0x%p , Failed to set reason in header",
              pNotify));
            return rv;
        }
    }
    if(expiresParamVal > UNDEFINED)
    {
        rv = RvSipSubscriptionStateHeaderSetExpiresParam(pNotify->hSubscriptionStateHeader, expiresParamVal);
        if(rv != RV_OK)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "SubsNotifySetSubscriptionStateParams - notify 0x%p , Failed to set expires param in header",
              pNotify));
            return rv;
        }
    }

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "SubsNotifySetSubscriptionStateParams - subs 0x%p notify 0x%p , Subscription-State was added to notification successfully",
              pNotify->pSubs,pNotify));
              */
    return RV_OK;
}

/***************************************************************************
 * SubsNotifySend
 * ------------------------------------------------------------------------
 * General: Sends the notify message placed in the notification object.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to send the notify request.
 *               RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifySend(IN  Subscription* pSubs,
                                   IN  Notification* pNotify)
{
    RvStatus      rv = RV_OK;
    RvInt32       cseqTransc;

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsNotifySend - Subs 0x%p notification 0x%p -  start to send a NOTIFY...",pSubs, pNotify));

    /* create the transaction - not in msg-send-failure.
       (in msg-send-failure we already have a transaction, cloned when calling to
        DNScontinue) */
    if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_MSG_SEND_FAILURE)
    {
        rv = SubsNotifyCreateTransc(pNotify);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsNotifySend - Subs 0x%p notification 0x%p - Failed to create notify transaction(rv=%d:%s)",
                pSubs,pNotify,rv,SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    if(NULL == pNotify->hTransc)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsNotifySend - Subs 0x%p notification 0x%p - Found out that notify transaction is NULL!!!",
                pSubs, pNotify));
        return RV_ERROR_UNKNOWN;
    }

    /* before sending - set the outbound message */
    rv = NotifyPrepareMsgBeforeSending(pNotify);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifySend - Subs 0x%p notification 0x%p - Failed to prepare notify request before sending(rv=%d:%s)",
            pSubs,pNotify,rv,SipCommonStatus2Str(rv)));
        SipTransactionTerminate(pNotify->hTransc);
        pNotify->hTransc = NULL;
        return rv;
    }

    /* save some of the notification parameters in the notification object */
    rv = SetNotificationParamsFromMsg(pNotify,pNotify->hOutboundMsg, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifySend - Subs 0x%p notification 0x%p - Failed to set notification parameters(rv=%d:%s)",
            pSubs,pNotify,rv,SipCommonStatus2Str(rv)));
        SipTransactionTerminate(pNotify->hTransc);
        pNotify->hTransc = NULL;
        return rv;
    }

    /*set notify outbound message to the transaction */
    SipTransactionSetOutboundMsg(pNotify->hTransc, pNotify->hOutboundMsg);
    pNotify->hOutboundMsg = NULL;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsNotifySend - Subs 0x%p notification 0x%p -  sending NOTIFY(%s)",
         pSubs, pNotify, GetNotifySubsStateName(pNotify->eSubstate)));

    rv = SipCallLegSendRequest(pSubs->hCallLeg, pNotify->hTransc,
                               SIP_TRANSACTION_METHOD_NOTIFY, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifySend - Subs 0x%p notification 0x%p - Failed to send notify request (status %d)",
            pSubs,pNotify,rv));
        SipTransactionTerminate(pNotify->hTransc);
        pNotify->hTransc = NULL;
        return rv;
    }
    
    /* save the cseq value of the last sent notify, so if 200 response of an earlier
       notify is received, we won't use it to update the subscription */
    rv = RvSipTransactionGetCSeqStep(pNotify->hTransc,&cseqTransc);
    if (RV_OK != rv)
    {	
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifySend - Subs 0x%p notification 0x%p - failed in RvSipTransactionGetCSeqStep() transc=0x%x",
            pSubs,pNotify,pNotify->hTransc));
        return rv;
    }
       
	SipCommonCSeqSetStep(cseqTransc,&pSubs->expectedCSeq);
        
    /* if this is a notify(active) on state pending -> moves to state SubsActivated */
    if(pSubs->eState == RVSIP_SUBS_STATE_PENDING &&
       pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE)
    {
        SubsChangeState(pSubs,
                        RVSIP_SUBS_STATE_ACTIVATED,
                        RVSIP_SUBS_REASON_NOTIFY_ACTIVE_SENT);
    }
    /* if this is a notify(terminated) on any state -> moves to state SubsTerminating */
    else if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED &&
            pSubs->eState != RVSIP_SUBS_STATE_TERMINATING)
    {
        SubsChangeState(pSubs,
                        RVSIP_SUBS_STATE_TERMINATING,
                        RVSIP_SUBS_REASON_NOTIFY_TERMINATED_SENT);
    }
    /* if prev state is pending, and state is UNSUBSCRIBE RCVD, we change the prev state
    to ACTIVATED, so if user reject the unsubscribe it will return to the correct state */
    else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD &&
            pSubs->ePrevState == RVSIP_SUBS_STATE_PENDING &&
            pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE)
    {
        pSubs->ePrevState = RVSIP_SUBS_STATE_ACTIVATED;
    }
    /*notify the notify sent status*/
    InformNotifyStatus(pNotify,RVSIP_SUBS_NOTIFY_STATUS_REQUEST_SENT,
                       RVSIP_SUBS_NOTIFY_REASON_UNDEFINED,NULL);

    return RV_OK;

}

/***************************************************************************
* SubsNotifyCreateTransc
* ------------------------------------------------------------------------
* General: Creates a NOTIFY transaction in the notification object,
*          and relates the transaction to the notification object.
*
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
*               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
*               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify   - Handle to the new created notification object.
***************************************************************************/
static RvStatus RVCALLCONV SubsNotifyCreateTransc(IN  Notification*      pNotify)
{
    RvStatus          rv;
    Subscription     *pSubs = pNotify->pSubs;

    /*----------------------------------------
     create the notify transaction and message
    ------------------------------------------*/
    rv = SipCallLegCreateTransaction(pSubs->hCallLeg, RV_FALSE, &(pNotify->hTransc));
    if (rv != RV_OK || NULL == pNotify->hTransc)
    {
        /*transc construction failed*/
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsNotifyCreateTransc - Subscription 0x%p - failed to create transaction",pSubs));
        pNotify->hTransc = NULL;
        return rv;
    }

    /*----------------------------------------------
    set notification handle in transaction.
    -----------------------------------------------*/
    rv = SipTransactionSetSubsInfo(pNotify->hTransc, (void*)pNotify);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyCreateTransc - Subscription 0x%p - failed to set subs info in transaction 0x%p",
            pSubs,pNotify->hTransc));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * NotifyPrepareMsgBeforeSending
 * ------------------------------------------------------------------------
 * General: set NOTIFY outbound message in notify transaction, and set the
 *          Event header in it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify - Pointer to the notification object.
 ***************************************************************************/
static RvStatus RVCALLCONV NotifyPrepareMsgBeforeSending (IN  Notification *pNotify)
{
    RvSipMsgHandle  hMsg;
    RvStatus       rv;
    Subscription*   pSubs = pNotify->pSubs;
    RvSipHeaderListElemHandle   hElem;
    RvSipSubscriptionStateHeaderHandle hSubsHeader;

    rv = SubsNotifyGetOutboundMsg(pNotify, &hMsg);
    if(rv != RV_OK || hMsg == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "NotifyPrepareMsgBeforeSending - subs 0x%p, notify 0x%p - failed to get notify outbound message",
            pSubs, pNotify));
        return rv;
    }

    if(pNotify->eSubstate != RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED)
    {
        rv = RvSipSubscriptionStateHeaderConstructInMsg(hMsg, RV_TRUE, &hSubsHeader);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "NotifyPrepareMsgBeforeSending - subs 0x%p, notify 0x%p , Failed to construct Subscription-State header in msg",
                pSubs, pNotify));
            return rv;
        }
    
        rv = RvSipSubscriptionStateHeaderSetSubstate(hSubsHeader, pNotify->eSubstate, NULL);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "NotifyPrepareMsgBeforeSending - subs 0x%p, notify 0x%p, Failed to set subsState in msg",
                pSubs, pNotify));
            return rv;
        }
    
        if(pNotify->eReason != RVSIP_SUBSCRIPTION_REASON_UNDEFINED)
        {
            rv = RvSipSubscriptionStateHeaderSetReason(hSubsHeader, pNotify->eReason, NULL);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "NotifyPrepareMsgBeforeSending - notify 0x%p , Failed to set reason in msg",
                    pNotify));
                return rv;
            }
        }
        if(pNotify->expiresParam > UNDEFINED)
        {
            rv = RvSipSubscriptionStateHeaderSetExpiresParam(hSubsHeader, pNotify->expiresParam);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "NotifyPrepareMsgBeforeSending - notify 0x%p , Failed to set expires param in header",
                    pNotify));
                return rv;
            }
        }
        
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "NotifyPrepareMsgBeforeSending - subs 0x%p notify 0x%p , Subscription-State was added to msg successfully",
             pSubs,pNotify))
    }
    /*-------------------------------------
       set event header in the NOTIFY request
    ---------------------------------------*/
    if(pSubs->hEvent == NULL)
    {
        if(pSubs->bEventHeaderExist != RV_FALSE)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "NotifyPrepareMsgBeforeSending - notify 0x%p - no Event header in subscription 0x%p. Event was not set in notify message",pNotify,pSubs));
        }
    }
    else
    {
        /* before pushing the event header - check that the message does not have
        already an event header */
        RvSipHeaderListElemHandle hListElem = NULL;
        void*                     pHeader = NULL;

        pHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_EVENT,
                                          RVSIP_FIRST_HEADER, &hListElem);
        if(pHeader == NULL)
        {
            rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER, (void*)pSubs->hEvent,
                                    RVSIP_HEADERTYPE_EVENT, &hElem, &pHeader);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "NotifyPrepareMsgBeforeSending - notify 0x%p - Failed to push Event header to notify message",pNotify));
                return rv;
            }
        }
    }
    return RV_OK;

}

/***************************************************************************
 * SubsNotifyRespond
 * ------------------------------------------------------------------------
 * General: Sends a response on notify message.
 *          If 200 response - update subscription state machine if needed.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to send the notify request.
 *               RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 *          statusCode - response code.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyRespond(IN  Subscription* pSubs,
                                       IN  Notification* pNotify,
                                       IN  RvUint16     statusCode)
{
    RvStatus rv = RV_OK;
    RvBool bWasSubsDeleted = RV_FALSE;

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "SubsNotifyRespond - Subs 0x%p notification 0x%p -  sending %d for NOTIFY",pSubs, pNotify, statusCode));

    /* Set the outbound message only if it already exists */ 
    if (pNotify->hOutboundMsg != NULL)
    {
        /*set notify outbound message to the transaction */
        SipTransactionSetOutboundMsg(pNotify->hTransc, pNotify->hOutboundMsg);
        pNotify->hOutboundMsg = NULL;
    }
        
    /*-------------------------
      Send the response message.
      --------------------------*/
    rv = RvSipTransactionRespond(pNotify->hTransc,statusCode,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyRespond - Subs 0x%p notification 0x%p - Failed to send %d response (status %d)",
            pSubs, pNotify, statusCode, rv));
        return rv;
    }

    /*notify the notify sent status*/
    InformNotifyStatus(pNotify,RVSIP_SUBS_NOTIFY_STATUS_FINAL_RESPONSE_SENT,
                       RVSIP_SUBS_NOTIFY_REASON_UNDEFINED,NULL);

    /*subs was terminated from callback*/
    if(pNotify->pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsNotifyRespond - Subs 0x%p notification 0x%p - subscription was terminated from callback. returning",
                    pSubs, pNotify));
        return RV_OK;
    }
    /*-------------------------------------------
      if 2xx - update subscription state machine.
      -------------------------------------------*/
    if(statusCode == 200)
    {
        /* if the 200 response is on notify(terminated) the subscription will be terminated,
           and as a result, the notify transaction will be terminated .
           in this case the 200 won't be sent on TCP, so we must detach it here */
        if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED)
        {
            rv = SipTransactionDetachOwner(pNotify->hTransc);
            if(rv == RV_OK)
            {
                RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsNotifyRespond - Subs 0x%p notification 0x%p - detached transc 0x%p",
                    pSubs, pNotify, pNotify->hTransc));
                pNotify->hTransc = NULL;
            }
            else
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsNotifyRespond - Subs 0x%p notification 0x%p - Failed to detach owner from transc 0x%p (status %d)",
                    pSubs, pNotify, pNotify->hTransc, rv));
                /* Even if in case the detach failed, we leave the transaction as is -
                   the notify response might be sent, even though the subscription is terminated */
            }

        }
        rv = UpdateSubsAfter2xxOnNotifySent(pSubs, pNotify, &bWasSubsDeleted);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsNotifyRespond - Subs 0x%p notification 0x%p - Failed to update subscription state machine",
                pSubs, pNotify));
        }
    }


    return RV_OK;
}

/***************************************************************************
 * SubsNotifyTerminateNotifications
 * ------------------------------------------------------------------------
 * General: Go over the subscription notification list,
 *          Terminate the notification's transaction,
 *          and remove notification from the list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs   - Pointer to the subscription.
 *        bOnlyFree - RV_FALSE - terminate notification using SubsNotifyTerminate()
 *                    RV_TRUE  - do not terminate, only free using SubsNotifyFree()
 ***************************************************************************/
void RVCALLCONV SubsNotifyTerminateNotifications(IN  Subscription* pSubs,
                                                 IN  RvBool        bOnlyFree)
{
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    Notification*        pNotify;
    RLIST_POOL_HANDLE    hNotifyPool = pSubs->pMgr->hNotifyListPool;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = pSubs->pMgr->maxNumOfSubs*10;

    if(pSubs->numOfNotifyObjs == 0)
    {
        return;
    }

    /*get the first item from the list*/
    RLIST_GetHead(hNotifyPool, pSubs->hNotifyList, &pItem);

    pNotify = (Notification*)pItem;
    if(pNotify == NULL)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyTerminateNotifications - Subs 0x%p - no notifies in list. numOfNotifyObjs != 0 ",
            pSubs));
        return;
    }

    /* go over all the notifications */
    while(pNotify  != NULL && safeCounter < maxLoops)
    {
        RLIST_GetNext(hNotifyPool,
                    pSubs->hNotifyList,
                    pItem,
                    &pNextItem);

        if (bOnlyFree == RV_TRUE)
        {
            /*only free resources */
            SubsNotifyFree(pNotify);
        }
        else
        {
            /*terminate*/
            SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_SUBS_TERMINATED);
        }


        pItem = pNextItem;
        pNotify = (Notification*)pItem;
        safeCounter++;
    }

    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsNotifyTerminateNotifications - Subscription 0x%p - Reached MaxLoops %d, infinite loop",
                  pSubs,maxLoops));
    }

}

/***************************************************************************
 * SubsNotifyTerminate
 * ------------------------------------------------------------------------
 * General: The function inform user of notification object termination, and
 *          insert the notification object to the termination queue.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification object.
 *          eReason    - reason for termination.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyTerminate(IN Notification*         pNotify,
                                         IN RvSipSubsNotifyReason eReason)
{
    RvStatus rv = RV_OK;

    if(RVSIP_SUBS_NOTIFY_STATUS_TERMINATED == pNotify->eStatus)
    {
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsNotifyTerminate - Subs 0x%p - notification 0x%p is already in status terminated ",
            pNotify->pSubs, pNotify));
        return RV_OK;
    }

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "SubsNotifyTerminate - Subs 0x%p - terminating notification 0x%p. reason=%s ",
        pNotify->pSubs, pNotify, SubsNotifyGetReasonName(eReason)));

    pNotify->eTerminationReason = eReason;
    if(pNotify->hTransc != NULL)
    {
        RvSipTranscOwnerHandle hTranscOwner = NULL;

        /* terminate the notify transaction */
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsNotifyTerminate - Subs 0x%p - terminating transc 0x%p of notify 0x%p."
            ,pNotify->pSubs, pNotify->hTransc, pNotify));
        RvSipTransactionTerminate(pNotify->hTransc);

        /*  The notify object will be terminated, when it's transaction will be
        out of the termination queue, since Notify was set as an owner of
        the transaction.
            But there are situations, when the Notify was not set as an owner
        of the transaction yet. In such case there is no need to wait for
        the transaction termination from the termination queue. Notification
        should be terminated immediately.
        Example for such situation: UpdateServerTransactionOwner() is called
        on incoming NOTIFY transaction creation. UpdateServerTransactionOwner()
        calls TransactionCallbackCallTranscCreatedEv(), few lines after this
        it calls TransactionAttachOwner(). TransactionCallbackCallTranscCreatedEv()
        initiates chain of calls, ended with call to application NotifyEvHandler()
        callback. If the application terminates the subscription from this
        callback, all subscription's notification objects will be terminated,
        and their transactions will be moved to the TERMINATED state. So when
        UpdateServerTransactionOwner() will reach call to TransactionAttachOwner(),
        the transaction will be in TERMINATED state. The UpdateServerTransactionOwner()
        will return without attaching the owner.
        */
        rv = RvSipTransactionGetOwner(pNotify->hTransc, &hTranscOwner);
        if (rv != RV_OK)
        {
            RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsNotifyTerminate - Subs 0x%p, notify 0x%p - failed to retrieve the owner of Transc 0x%p (rv=%d:%s)",
                pNotify->pSubs,pNotify,pNotify->hTransc,rv,SipCommonStatus2Str(rv)));
        }
        if (pNotify == (Notification*)hTranscOwner)
        {
            return RV_OK;
        }
    }

    /* no transaction in the notify object - send it to the termination queue. */
    pNotify->eStatus = RVSIP_SUBS_NOTIFY_STATUS_TERMINATED;

    /* insert notification object to termination queue */
    rv = SipTransportSendTerminationObjectEvent(pNotify->pSubs->pMgr->hTransportMgr,
                                     (void *)pNotify,
                                      &(pNotify->terminationInfo),
                                      eReason,
                                      TerminateNotifyEventHandler,
                                      RV_TRUE,
                                      SUBS_NOTIFY_STATUS_TERMINATED_STR,
                                      SUBS_NOTIFY_OBJECT_NAME_STR);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "SubsNotifyTerminate - Subs 0x%p - Failed to set notification 0x%p in termination queue (rv %d)",
            pNotify->pSubs, pNotify, rv));
    }

    return rv;
}


/***************************************************************************
 * InformNotifyStatus
 * ------------------------------------------------------------------------
 * General: Informs application of the notification status.
 *          For a subscriber: Gives the received Notify message, and it's related
 *          notification object handle. Application may get all notification
 *          information from the message at this callback.
 *          The message will be destructed after calling to this callback.
 *          For a Notifier: Informs application of a response message that was
 *          received for a Notify request. At the end of this callback,
 *          the stack notification object is destructed.
 *          For both subscriber and notifier it indicates about termination of
 *          the notification object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification.
 *            eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received notify msg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV InformNotifyStatus(IN  Notification          *pNotify,
                                   IN  RvSipSubsNotifyStatus eNotifyStatus,
                                   IN  RvSipSubsNotifyReason eNotifyReason,
                                   IN  RvSipMsgHandle        hNotifyMsg)
{
    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "InformNotifyStatus - notification 0x%p  - new status is: %s, reason is: %s",
               pNotify, SubsNotifyGetStatusName(eNotifyStatus),
               SubsNotifyGetReasonName(eNotifyReason)));

    pNotify->eStatus = eNotifyStatus;

    SubsCallbackChangeNotifyStateEv(pNotify, eNotifyStatus, eNotifyReason, hNotifyMsg);
}

/***************************************************************************
 * SetNotificationParamsFromMsg
 * ------------------------------------------------------------------------
 * General: Sets parameters from Subscription-State header to a notification
 *          object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the notification.
 *        hMsg  - Handle to the notify msg (request or response).
 * Output: eNotifyReason - reason for the notify event.
 ***************************************************************************/
RvStatus RVCALLCONV SetNotificationParamsFromMsg(IN  Notification* pNotify,
                                                  IN  RvSipMsgHandle hMsg,
                                                  OUT RvSipSubsNotifyReason* peNotifyReason)
{
    RvSipHeaderListElemHandle hListElem;
    RvSipSubscriptionStateHeaderHandle hSubsState;
    RvBool  bIncomingReferNotify = RV_FALSE;

    if(peNotifyReason == NULL)
    {
        bIncomingReferNotify = RV_FALSE;
    }
    else if(pNotify->pSubs->pReferInfo != NULL)
    {
        if(pNotify->pSubs->eSubsType == RVSIP_SUBS_TYPE_SUBSCRIBER)
        {
            bIncomingReferNotify = RV_TRUE;
        }
    }

    /* if the subscription-state header was set directly to the notify outbound message,
       we need to load the header information from the message */
    if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED)
    {
        hSubsState = (RvSipSubscriptionStateHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_SUBSCRIPTION_STATE, RVSIP_FIRST_HEADER, &hListElem);
        if(hSubsState == NULL) /* there is no subscription-state header in message yet */
        {
            if(RV_FALSE == bIncomingReferNotify)
            {
                RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                    "SetNotificationParamsFromMsg - Subs 0x%p notification 0x%p - No Subscription-State header in Notify message",
                    pNotify->pSubs,pNotify));
                return RV_ERROR_NOT_FOUND;
            }
            else
            {
                /* possible that an incoming Notify does not hold this header - backward compatibility */
                *peNotifyReason = RVSIP_SUBS_NOTIFY_REASON_REFER_NO_SUBSCRIPTION_STATE_HEADER;
                RvLogWarning(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                    "SetNotificationParamsFromMsg - Subs 0x%p notification 0x%p - No Subscription-State header in REFER Notify",
                    pNotify->pSubs,pNotify));
                return RV_OK;
            }
        }
        else /* Subscription-State header exists */
        {
            pNotify->eSubstate = RvSipSubscriptionStateHeaderGetSubstate(hSubsState);    
            pNotify->eReason = RvSipSubscriptionStateHeaderGetReason(hSubsState);
            if(pNotify->eReason == RVSIP_SUBSCRIPTION_REASON_OTHER)
            {
                RvInt32 tempOffset;
                HPAGE    tempPage;
                HRPOOL   tempPool;
            
                tempOffset = SipSubscriptionStateHeaderGetStrReason(hSubsState);
                tempPool = SipSubscriptionStateHeaderGetPool(hSubsState);
                tempPage = SipSubscriptionStateHeaderGetPage(hSubsState);
            
                pNotify->strReason = AllocCopyRpoolString(
                    pNotify->pSubs, pNotify->pSubs->pMgr->hGeneralPool,
                    pNotify->pSubs->hPage, tempPool, tempPage, tempOffset);
                
                RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                    "SetNotificationParamsFromMsg - Subs 0x%p notification 0x%p - saved reason string on the subs page 0x%p!",
                    pNotify->pSubs,pNotify, pNotify->pSubs->hPage));
            }
        
            pNotify->expiresParam = RvSipSubscriptionStateHeaderGetExpiresParam(hSubsState);
            if(pNotify->expiresParam != UNDEFINED &&
                (RvUint32)pNotify->expiresParam > RVSIP_SUBS_EXPIRES_VAL_LIMIT)
            {
                pNotify->expiresParam = RVSIP_SUBS_EXPIRES_VAL_LIMIT;
            }
        }
    }
    
    if(RV_TRUE == bIncomingReferNotify && UNDEFINED == pNotify->expiresParam)
    {
        /* refer notify with no expires parameter */
        if( pNotify->eSubstate != RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED &&
            (!SipCommonCoreRvTimerStarted(&pNotify->pSubs->alertTimer)))
        {
            /* the subscription timer was not set yet, so this is the first notify */
            *peNotifyReason = RVSIP_SUBS_NOTIFY_REASON_REFER_NO_EXPIRES_IN_FIRST_NOTIFY;
            RvLogWarning(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                    "SetNotificationParamsFromMsg - Subs 0x%p notification 0x%p - No expires param in refer Notify message",
                    pNotify->pSubs,pNotify));

        }
    }
    return RV_OK;
}

/***************************************************************************
 * UpdateSubsAfter2xxOnNotifyRcvd
 * ------------------------------------------------------------------------
 * General: When user receives 2xx to a notify request, it may effect the
 *          subscription state-machine.
 *          This function update it:
 *          1. if expires parameter exists - reset the expiration timer.
 *          2. update the subscription state machine:
 *             Notify(active) SubsActivated -> SubsActive
 *             Notify(terminated): SubsTerminating --> SubsTerminated.
 *             Notify(active) UnsubsRcvd -> change only prevState to SubsActive
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        pNotify - Pointer to the notification.
 * Output:bSubsDeleted - was subscription object terminated in this function or not.
 *        (we need this parameter to know whether to terminate the notify
 *        object or not, because if the subscription is terminated, and we will
 *        terminate the notification - it will be terminated twice...)
 ***************************************************************************/
RvStatus RVCALLCONV UpdateSubsAfter2xxOnNotifyRcvd(IN  Subscription* pSubs,
                                                    IN  Notification* pNotify,
                                                    IN  RvSipTranscHandle hNotifyTransc,
                                                    OUT RvBool*      bSubsDeleted)
{
    RvStatus rv;
    RvSipSubsState eNewState = RVSIP_SUBS_STATE_UNDEFINED;
    RvSipSubsStateChangeReason eReason = RVSIP_SUBS_REASON_NOTIFY_2XX_RCVD;
    RvInt32 cseq;
	RvBool	bSmaller; 

    *bSubsDeleted = RV_FALSE;

    /* First check that the 200 is for the last sent notify, and not for an earlier notify */
    rv = RvSipTransactionGetCSeqStep(hNotifyTransc, &cseq);
    if (RV_OK != rv)
    {	
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "UpdateSubsAfter2xxOnNotifyRcvd - Subs 0x%p notification 0x%p - failed in RvSipTransactionGetCSeqStep() transc=0x%x",
            pSubs,pNotify,hNotifyTransc));
        return rv;
    }

    bSmaller = SipCommonCSeqIsIntSmaller(&pSubs->expectedCSeq,cseq,RV_FALSE); 
    if(bSmaller == RV_TRUE)
    {
        /* earlier notify */
      RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "UpdateSubsAfter2xxOnNotifyRcvd - Subs 0x%p notify 0x%p - response for early notify. ignore",
            pSubs, pNotify));
        return RV_OK;
    }

    /* if this is an out of band subscription - only notify(terminated)
    may effect the subscription, by moving directly to state terminated */
    if(pSubs->bOutOfBand == RV_TRUE)
    {
        if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                "UpdateSubsAfter2xxOnNotifyRcvd - Subs 0x%p notify 0x%p - subs moves to state Terminated.",
                pSubs, pNotify));

            SubsTerminate(pSubs, eReason);
            *bSubsDeleted = RV_TRUE;
            return RV_OK;
        }
        else
        {
            return RV_OK;
        }
    }

    /* -----------------------------------------------------------
       update the alert timer according to the expires parameter
       ----------------------------------------------------------- */
    if(pNotify->expiresParam > 0)
    {
		RvInt64 delay = 0;

		if (SipCommonCoreRvTimerStarted(&pNotify->pSubs->alertTimer) == RV_TRUE)
		{
			/*If the timer started we are taking its remaining time in order to calculate new expiration time*/
			RvTimerGetRemainingTime(&pNotify->pSubs->alertTimer.hTimer, &delay);
		}		
        	
		if (pNotify->expiresParam < RvInt64ToRvInt32(RvInt64Div(delay, RV_TIME64_NSECPERSEC)) || delay == 0)
		{
			/* set the alert timer:
			expiresVal * 1000  ---  Convert it to milliseconds. */

			rv = SubsObjectSetAlertTimer(pSubs, (pNotify->expiresParam)*1000);
			if(rv != RV_OK)
			{
				RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
					"UpdateSubsAfter2xxOnNotifyRcvd - Subs 0x%p notify 0x%p - Couldn't set expiration timer",
					pSubs, pNotify));
				return rv;
			}
		}
    }

    /* --------------------------------------
       update the subscription state machine
       -------------------------------------- */
    /* pending: do nothing  */
    if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_PENDING)
    {
        return RV_OK;
    }
    /* active: SubsActivated -> SubsActive  */
    else if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE)
    {
        if(pSubs->eState == RVSIP_SUBS_STATE_ACTIVATED)
        {
            eNewState = RVSIP_SUBS_STATE_ACTIVE;
        }
        else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD &&
                pSubs->ePrevState == RVSIP_SUBS_STATE_ACTIVATED)
        {
            /* the received 2xx should change the state from activated to active,
            we change the prev state, without changeStateEv, so if user will reject the
            unsubscribe request, we will change state to the correct state - active */
            pSubs->ePrevState = RVSIP_SUBS_STATE_ACTIVE;
            return RV_OK;
        }
        else
        {
            return RV_OK;
        }
    }
    /* terminated: SubsTerminating --> SubsTerminated */
    else if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED)
    {
        if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATING)
        {
            eNewState = RVSIP_SUBS_STATE_TERMINATED;
        }
        else
        {
            return RV_OK;
        }
    }

    if(eNewState != RVSIP_SUBS_STATE_TERMINATED)
    {
        SubsChangeState(pSubs, eNewState, eReason);
    }
    else
    {
        *bSubsDeleted = RV_TRUE;
        SubsTerminate(pSubs, eReason);
    }
    return RV_OK;
}

/*-----------------------------------------------------------------------
        D N S   A P I
 ------------------------------------------------------------------------*/
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
 /***************************************************************************
 * SubsNotifyDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, terminate the transaction and the
 *          notification object.
 *          The function also update the subscription state for those cases:
 *          if subscription state is ACTIVATED - Terminate the subscription.
 *          if subscription state is TERMINATING - Change state back to previous state.
 *          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
 *          status in RvSipSubsNotifyEv event.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription object.
 *          pNotify    - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSGiveUp (IN  Subscription* pSubs,
                                          IN  Notification* pNotify)
{
    RvStatus rv = RV_OK;

    if(RVSIP_SUBS_STATE_ACTIVATED == pSubs->eState ||
       (RVSIP_SUBS_STATE_TERMINATING == pSubs->eState &&
        RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED == pNotify->eSubstate))
    {
        /* terminate subscription */
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyDNSGiveUp: subscription 0x%p notify 0x%p - state %s - terminate subscription",
            pSubs, pNotify, SubsGetStateName(pSubs->eState)));

        rv = SubsTerminate(pSubs, RVSIP_SUBS_REASON_GIVE_UP_DNS);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsNotifyDNSGiveUp: subscription 0x%p notify 0x%p - Failed to terminate subscription",
                pSubs, pNotify));
            return rv;
        }
    }
    else
    {
        /* terminate notification and it's transaction */
        rv = SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_GIVE_UP_DNS);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsNotifyDNSGiveUp: subscription 0x%p notify 0x%p - Failed to terminate notification",
                pSubs, pNotify));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
* SubsNotifyDNSContinue
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, clone the notify transaction, with a new
*          DNS list, and terminate the old transaction in the notification object.
*          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input: pSubs      - Pointer to the subscription object.
*        pNotify    - Pointer to the notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSContinue (IN  Subscription* pSubs,
                                            IN  Notification* pNotify)
{
    RvStatus rv = RV_OK;
    RvSipTranscHandle hNewTransc;

    /* continue DNS */
    rv = SipTransactionDNSContinue( pNotify->hTransc,
                                    (RvSipTranscOwnerHandle)pSubs->hCallLeg,
                                    RV_FALSE,
                                    &hNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyDNSContinue - Subscription 0x%p notify 0x%p - Failed to continue DNS for transaction 0x%p. (status %d)",
            pSubs, pNotify, pNotify->hTransc, rv));
        return rv;
    }

    /* save new transaction in notification -
        the SipTransactionDNSContinue function already terminated the old transaction,
        so we don't need to take care of it.*/
     pNotify->hTransc = hNewTransc;


    /*----------------------------------------------
    set notification handle in transaction.
    -----------------------------------------------*/
    rv = SipTransactionSetSubsInfo(pNotify->hTransc, (void*)pNotify);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsNotifyDNSContinue - Subscription 0x%p - failed to set subs info in transaction 0x%p",
            pSubs,pNotify->hTransc));
        return rv;
    }


    return RV_OK;

}
/***************************************************************************
* SubsNotifyDNSReSendRequest
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the cloned notify transaction from
*          the notification object.
*          this function is for a NOTIFY request, if user got MSG_SEND_FAILURE
*          status in RvSipSubsNotifyEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input: pSubs      - Pointer to the subscription object.
*        pNotify    - Pointer to the notification object.
***************************************************************************/
RvStatus RVCALLCONV SubsNotifyDNSReSendRequest (IN  Subscription* pSubs,
                                                 IN  Notification* pNotify)
{
    RvStatus rv = RV_OK;

    rv = SubsNotifySend(pSubs, pNotify);
    if(rv != RV_OK)
    {
         RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsNotifyDNSReSendRequest: subscription 0x%p notify 0x%p - Failed to send new transaction 0x%p. (status %d)",
             pSubs, pNotify, pNotify->hTransc, rv));
         return rv;
    }

    return RV_OK;

}

#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT*/
/*-----------------------------------------------------------------------
       S T A T I C     F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * SubsNotifyFree
 * ------------------------------------------------------------------------
 * General: Removes notification object from subscription list.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_OK       -  sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification object.
 ***************************************************************************/
static RvStatus SubsNotifyFree(IN  Notification* pNotify)
{
    RLIST_POOL_HANDLE    hNotifyPool;
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    Notification*        pFoundNotify;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(NULL != pNotify->hOutboundMsg)
    {
        RvSipMsgDestruct(pNotify->hOutboundMsg);
        pNotify->hOutboundMsg = NULL;
    }

    hNotifyPool = pNotify->pSubs->pMgr->hNotifyListPool;
    maxLoops = pNotify->pSubs->pMgr->maxNumOfSubs*10;

    /*remove notify object from subscription's notify list*/
    RLIST_GetHead(hNotifyPool, pNotify->pSubs->hNotifyList, &pItem);
    while(pItem != NULL &&  safeCounter < maxLoops)
    {
        pFoundNotify = (Notification*)pItem;
        if(pFoundNotify == pNotify)
        {
            RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                "SubsNotifyFree - Subs 0x%p - notify 0x%p is removed",
                pNotify->pSubs, pNotify));

            --(pNotify->pSubs->numOfNotifyObjs);
            RvMutexLock(&pNotify->pSubs->pMgr->hMutex,pNotify->pSubs->pMgr->pLogMgr);
            RLIST_Remove(hNotifyPool, pNotify->pSubs->hNotifyList, pItem);
            RvMutexUnlock(&pNotify->pSubs->pMgr->hMutex,pNotify->pSubs->pMgr->pLogMgr);
            return RV_OK;
        }
        else
        {
            RLIST_GetNext(hNotifyPool, pNotify->pSubs->hNotifyList, pItem, &pNextItem);
            pItem = pNextItem;
            safeCounter++;
        }
    }

    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                  "SubsNotifyFree - Subscription 0x%p - Reached MaxLoops %d, infinite loop",
                  pNotify->pSubs,maxLoops));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
* TerminateNotifyEventHandler
* ------------------------------------------------------------------------
* General: Terminates a notification object, Free the resources taken by it and
*          remove it from the manager subscription-leg list.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     obj        - pointer to subscription to be deleted.
*          reason    - reason of the subscription termination
***************************************************************************/
static RvStatus RVCALLCONV TerminateNotifyEventHandler(IN void *obj, 
                                                       IN RvInt32  reason)
{
    Notification        *pNotify;
    Subscription        *pSubs;
    RvStatus            retCode;

    pNotify = (Notification *)obj;
    if (pNotify == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    pSubs = pNotify->pSubs;

    retCode = SubsLockMsg(pSubs);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "TerminateNotifyEventHandler - Subs 0x%p - notify 0x%p is out of the termination queue",
        pSubs, pNotify));

    /* inform user of notify termination */
    InformNotifyStatus(pNotify, RVSIP_SUBS_NOTIFY_STATUS_TERMINATED,
                        (RvSipSubsNotifyReason)reason, NULL);

    SubsNotifyFree(pNotify);

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "TerminateNotifyEventHandler - Subs 0x%p - notify 0x%p is free",
        pSubs, pNotify));

    if(pSubs->numOfNotifyObjs == 0 && pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        /* subscription was already indicate application of state terminated,
        and there are no more notification objects, therefore we shall free the
        subscription object */
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "TerminateNotifyEventHandler - Subs 0x%p - last notify was free. subs state is TERMINATED -> terminate subscription object",
            pSubs));

        SubsTerminate(pSubs,RVSIP_SUBS_REASON_SUBS_TERMINATED);
    }

    SubsUnLockMsg(pSubs);

    return RV_OK;
}

/***************************************************************************
 * UpdateSubsAfter2xxOnNotifySent
 * ------------------------------------------------------------------------
 * General: When user respond with 2xx to a notify request, it may effect the
 *          subscription state-machine.
 *          This function update it:
 *          1. if expires parameter exists - reset the expiration timer.
 *          2. update the subscription state machine:
 *             Notify(pending): Subs2xxRcvd -> SubsPending + release noNotifyTimer
 *                              SubsSent -> NotifyBefore2xxRcvd
 *             Notify(active):  Subs2xxRcdv -> SubsActive + release noNotifyTimer
 *                              SubsPending -> SubsActive
 *                              NotifyBefore2xxRcvd - set bActivatedOn2xxBeforeNotifyState = true.
 *             Notify(terminated): Subs2xxRcvd, SubsPending, SubsNotifyRcvd, SubsActive,
 *                              SubsRefreshing, SubsUnsubscribing --> SubsTerminated
 *                              SubsUnsubscribe2xxRcvd --> SubsTerminated + release noNotify timer
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs - Pointer to the subscription.
 *        pNotify - Pointer to the notification.
 * Output: bSubsDeleted - was subscription object terminated in this function or not.
 *        (we need this parameter to know whether to terminate the notify
 *          object or not, because if the subscription is terminated, and we will
 *          terminate the notification - it will be terminated twice...)
 ***************************************************************************/
static RvStatus UpdateSubsAfter2xxOnNotifySent(IN Subscription* pSubs,
                                                IN Notification* pNotify,
                                                OUT RvBool*     bSubsDeleted)
{
    RvSipSubsState eNewState = RVSIP_SUBS_STATE_UNDEFINED;
    RvSipSubsStateChangeReason eReason;
    RvBool        bFinished;

    *bSubsDeleted = RV_FALSE;

    /* ------------------------------
       release no notify timer if set
       ------------------------------ */
    if(pSubs->eState != RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD ||
       (pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD &&
        pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED))
    {
        /* in case of unsubs-2xx-rcvd state, we won't reset the timer if this is
           a notify(active). we wait only for notify(terminated)...*/
        SubsNoNotifyTimerRelease(pSubs);
    }

    /* -----------------------------------------------------------
       update the alert timer according to the expires parameter
       (only for subscription which is not out-of-band. because by RV
        definition, an out-of-band subscription has no timer.).
       ----------------------------------------------------------- */
    if(pNotify->expiresParam > 0 && pSubs->bOutOfBand == RV_FALSE)
    {
        SubsCalculateAndSetAlertTimer(pSubs, pNotify->expiresParam);
    }

    eReason = NotifyReason2MsgReason(pNotify->eReason);

    /* ----------------------------------
        Handle out-of-band subscription
       ---------------------------------- */
    if(pSubs->bOutOfBand == RV_TRUE)
    {
        /* if this is an out of band subscription - only notify(terminated)
           may effect the subscription, by moving directly to state terminated*/
        if(pNotify->eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
                "UpdateSubsAfter2xxOnNotifySent - Subs 0x%p notify 0x%p - subs moves to state Terminated.",
                pSubs, pNotify));

            SubsTerminate(pSubs, eReason);
            *bSubsDeleted = RV_TRUE;
            return RV_OK;
        }
        else
        {
            return RV_OK;
        }
    }

    /* ----------------------------------
        Handle regular subscription
       ---------------------------------- */

    /* --------------------------------------
       update the subscription state machine
       -------------------------------------- */
    switch(pNotify->eSubstate)
    {
    case RVSIP_SUBSCRIPTION_SUBSTATE_PENDING:
        Handle2xxOnNotifyPendingSent(pSubs, &eNewState, &bFinished);
        break;
    case RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE:
        Handle2xxOnNotifyActiveSent(pSubs, &eNewState, &bFinished);
        break;
    case RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED:
        Handle2xxOnNotifyTerminatedSent(pSubs, &eNewState, &bFinished);
        break;
    case RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED:
    case RVSIP_SUBSCRIPTION_SUBSTATE_OTHER:
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "UpdateSubsAfter2xxOnNotifySent: Subs0x%p, Notification 0x%p - subscription-State header value is UNDEFINED!!!",
                pSubs, pNotify));
        return RV_OK;
    default:
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "UpdateSubsAfter2xxOnNotifySent: Subs0x%p, Notification 0x%p - unknown subscription-State header value 0x%p!!!",
            pSubs, pNotify,pNotify->eSubstate));
        return RV_ERROR_UNKNOWN;
    }

    if(RV_TRUE == bFinished) /* no need to update the state machine */
    {
        return RV_OK;
    }

    if(eNewState != RVSIP_SUBS_STATE_TERMINATED)
    {
        SubsChangeState(pSubs, eNewState, eReason);
    }
    else
    {
        SubsTerminate(pSubs, eReason);
        *bSubsDeleted = RV_TRUE;
    }
    return RV_OK;
}

/***************************************************************************
 * NotifyReason2MsgReason
 * ------------------------------------------------------------------------
 * General: Converts the subscription reason type to a Subscription-State
 *          header reason type.
 * Return Value: RvSipSubsStateChangeReason
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - RvSipSubscriptionReason reason.
 ***************************************************************************/
static RvSipSubsStateChangeReason NotifyReason2MsgReason(RvSipSubscriptionReason eReason)
{
    switch(eReason)
    {
    case RVSIP_SUBSCRIPTION_REASON_DEACTIVATED:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_DEACTIVATED_RCVD;
    case RVSIP_SUBSCRIPTION_REASON_PROBATION:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_PROBATION_RCVD;
    case RVSIP_SUBSCRIPTION_REASON_REJECTED:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_REJECTED_RCVD;
    case RVSIP_SUBSCRIPTION_REASON_TIMEOUT:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_TIMEOUT_RCVD;
    case RVSIP_SUBSCRIPTION_REASON_GIVEUP:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_GIVEUP_RCVD;
    case RVSIP_SUBSCRIPTION_REASON_NORESOURCE:
        return RVSIP_SUBS_REASON_NOTIFY_TERMINATED_NORESOURCE_RCVD;
    default:
        return RVSIP_SUBS_REASON_NOTIFY_2XX_SENT;
    }
}

/***************************************************************************
 * Handle2xxOnNotifyPendingSent
 * ------------------------------------------------------------------------
 * General: When user respond with 2xx to a notify(pending)
 *          Subs2xxRcvd -> SubsPending + release noNotifyTimer
 *          SubsSent    -> NotifyBefore2xxRcvd
 *          For all other cases, we do not want to call the change state callback,
 *          so we set pbFinished to true.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSubs - Pointer to the subscription.
 * Output: peNewState - The new state for the subscription.
 *         pbFinished - Indicates whether to continue with the function or not.
 ***************************************************************************/
static RvStatus Handle2xxOnNotifyPendingSent(IN Subscription* pSubs,
                                              OUT RvSipSubsState* peNewState,
                                              OUT RvBool*        pbFinished)
{
    *pbFinished = RV_FALSE;

    if(pSubs->eState == RVSIP_SUBS_STATE_2XX_RCVD)
    {
        *peNewState = RVSIP_SUBS_STATE_PENDING;
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_SENT)
    {
        *peNewState = RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD;
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBING &&
            pSubs->eStateBeforeUnsubsWasSent == RVSIP_SUBS_STATE_2XX_RCVD)
    {
        /* we change eStateBeforeUnsubsWasSent and do not call to change state event.
        now if the unsubscribe will be rejected, we will move back to active state */
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "Handle2xxOnNotifyPendingSent: Subs0x%p, updated eStateBeforeUnsubsWasSent to pending!",
            pSubs));
        pSubs->eStateBeforeUnsubsWasSent = RVSIP_SUBS_STATE_PENDING;
        *pbFinished = RV_TRUE;
    }
    else
    {
        *pbFinished = RV_TRUE;

    }
    return RV_OK;
}

/***************************************************************************
 * Handle2xxOnNotifyActiveSent
 * ------------------------------------------------------------------------
 * General: When user respond with 2xx to a notify(active)
 *          Subs2xxRcdv -> SubsActive + release noNotifyTimer
 *          SubsPending -> SubsActive
 *          Unsubscribing + the state before sending unsubscribe is Subs2xxRcdv
 *                      -> change the eStateBeforeUnsubsWasSent to SubsActive
 *          Unsubscribing + the state before sending unsubscribe is pending
 *                      -> change the eStateBeforeUnsubsWasSent to SubsActive
 *
 *          For all other cases, we don't want to call the change state callback,
 *          so we set pbFinished to true.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSubs - Pointer to the subscription.
 * Output: peNewState - The new state for the subscription.
 *         pbFinished - Indicates whether to continue with the function or not.
 ***************************************************************************/
static RvStatus Handle2xxOnNotifyActiveSent(IN Subscription* pSubs,
                                              OUT RvSipSubsState* peNewState,
                                              OUT RvBool*        pbFinished)
{
    *pbFinished = RV_FALSE;

    if(pSubs->eState == RVSIP_SUBS_STATE_2XX_RCVD)
    {
        *peNewState = RVSIP_SUBS_STATE_ACTIVE;
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_PENDING)
    {
        *peNewState = RVSIP_SUBS_STATE_ACTIVE;
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_SENT)
    {
        *peNewState = RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD;
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "Handle2xxOnNotifyActiveSent: Subs0x%p, bActivatedOn2xxBeforeNotifyState=true (subs state=%s)",
                pSubs, SubsGetStateName(pSubs->eState)));
        pSubs->bActivatedOn2xxBeforeNotifyState = RV_TRUE;
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBING)
    {
    /* we change eStateBeforeUnsubsWasSent and do not call to change state event.
       now if the unsubscribe will be rejected, we will move back to active state */
        if(pSubs->eStateBeforeUnsubsWasSent == RVSIP_SUBS_STATE_2XX_RCVD ||
           pSubs->eStateBeforeUnsubsWasSent == RVSIP_SUBS_STATE_PENDING)
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "Handle2xxOnNotifyActiveSent: Subs0x%p, updated eStateBeforeUnsubsWasSent to active!",
                pSubs));
            pSubs->eStateBeforeUnsubsWasSent = RVSIP_SUBS_STATE_ACTIVE;
            *pbFinished = RV_TRUE;
            return RV_OK;
        }
        else /* bug fix */
        {
            *pbFinished = RV_TRUE;
            return RV_OK;
        }
    }
    else if(pSubs->eState == RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "Handle2xxOnNotifyActiveSent: Subs0x%p, bActivatedOn2xxBeforeNotifyState=true (subs state=%s)",
                pSubs, SubsGetStateName(pSubs->eState)));
        pSubs->bActivatedOn2xxBeforeNotifyState = RV_TRUE;
        /* remain on the same state */
        *pbFinished = RV_TRUE;
        return RV_OK;
    }
    else
    {
        /* for all other cases, there is no need to update the subscription state machine */
        *pbFinished = RV_TRUE;
        return RV_OK;
    }
    return RV_OK;
}

/***************************************************************************
 * Handle2xxOnNotifyTerminatedSent
 * ------------------------------------------------------------------------
 * General: When user respond with 2xx to a notify(terminated)
 *          Subs2xxRcvd, SubsPending, SubsNotifyRcvd, SubsActive, SubsRefreshing,
 *          SubsUnsubscribing      --> SubsTerminated
 *          SubsUnsubscribe2xxRcvd --> SubsTerminated + release noNotify timer
 *          For all other cases, we do not want to call the change state callback,
 *          so we set pbFinished to true.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSubs - Pointer to the subscription.
 * Output: peNewState - The new state for the subscription.
 *         pbFinished - Indicates whether to continue with the function or not.
 ***************************************************************************/
static RvStatus Handle2xxOnNotifyTerminatedSent(IN  Subscription    *pSubs,
                                                OUT RvSipSubsState  *peNewState,
                                                OUT RvBool          *pbFinished)
{
    *pbFinished = RV_FALSE;

    *peNewState = RVSIP_SUBS_STATE_TERMINATED;

    switch(pSubs->eState)
    {
    case RVSIP_SUBS_STATE_SUBS_SENT:
    case RVSIP_SUBS_STATE_2XX_RCVD:
    case RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD:
    case RVSIP_SUBS_STATE_REFRESHING:
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
    case RVSIP_SUBS_STATE_ACTIVE:
    case RVSIP_SUBS_STATE_PENDING:
    case RVSIP_SUBS_STATE_MSG_SEND_FAILURE:
        RvLogInfo(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "Handle2xxOnNotifyTerminatedSent - Subs 0x%p - subs moves to state Terminated.",
            pSubs));
        break;
    default:
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "Handle2xxOnNotifyTerminatedSent: Subs 0x%p - illegal notify(terminated) request on state %s ",
            pSubs, SubsGetStateName(pSubs->eState)));
        *pbFinished = RV_TRUE;
    }
    return RV_OK;
}
/***************************************************************************
 * AllocCopyRpoolString
 * ------------------------------------------------------------------------
 * General: The function is for setting a given string (with the NULL in the end)
 *          on a non consecutive part of the page.
 *          The given string is on a RPOOL page, and is given as pool+page+offset.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for strlen+1 because of that.
 * Return Value: offset of the new string on destPage, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs        - Pointer to the subscription.
 *        destPool     - Handle of the destination rpool.
 *        destPage     - Handle of the destination page
 *        sourcePool   - Handle of the source rpool.
 *        destPage     - Handle of the source page
 *          stringOffset - The offset of the string to set.
 ***************************************************************************/
static RvInt32 AllocCopyRpoolString(  IN Subscription* pSubs,
                                      IN HRPOOL   destPool,
                                      IN HPAGE    destPage,
                                      IN HRPOOL   sourcePool,
                                      IN HPAGE    sourcePage,
                                      IN RvInt32 stringOffset)
{
    RvInt      strLen;
    RvStatus   stat;
    RvInt32    offset;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSubs);
#endif

    if(stringOffset < 0)
        return RV_ERROR_BADPARAM;

    strLen = RPOOL_Strlen(sourcePool, sourcePage, stringOffset);

    /* append strLen + 1, for terminate with NULL*/
    stat = RPOOL_Append(destPool, destPage, strLen+1, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "AllocCopyRpoolString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return UNDEFINED;
    }
    stat = RPOOL_CopyFrom(destPool,
                          destPage,
                          offset,
                          sourcePool,
                          sourcePage,
                          stringOffset,
                          strLen + 1);
    if(stat != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "AllocCopyRpoolString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return UNDEFINED;
    }
    else
        return offset;

}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
* SubsNotifyGetStatusName
* ------------------------------------------------------------------------
* General: Returns the name of a given status
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SubsNotifyGetStatusName (RvSipSubsNotifyStatus  eStatus)
{

    switch(eStatus)
    {
    case RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD:
        return "Notify Request Rcvd";
    case RVSIP_SUBS_NOTIFY_STATUS_2XX_RCVD:
        return "Notify 2xx Response Rcvd";
    case RVSIP_SUBS_NOTIFY_STATUS_REJECT_RCVD:
        return "Notify Reject response Rcvd";
    case RVSIP_SUBS_NOTIFY_STATUS_REDIRECTED:
        return "Notify Redirected Response Rcvd";
#ifdef RV_SIP_AUTH_ON
    case RVSIP_SUBS_NOTIFY_STATUS_UNAUTHENTICATED:
        return "Notify Unauthenticated Response Rcvd";
#endif /* #ifdef RV_SIP_AUTH_ON */
    case RVSIP_SUBS_NOTIFY_STATUS_TERMINATED:
        return SUBS_NOTIFY_STATUS_TERMINATED_STR;
    case RVSIP_SUBS_NOTIFY_STATUS_MSG_SEND_FAILURE:
        return "Notify Msg Send Failure";
    case RVSIP_SUBS_NOTIFY_STATUS_IDLE:
        return "Notify Idle";
    case RVSIP_SUBS_NOTIFY_STATUS_REQUEST_SENT:
        return "Notify Request Sent";
    case RVSIP_SUBS_NOTIFY_STATUS_FINAL_RESPONSE_SENT:
        return "Notify Final Response Sent";
    default:
        return "Undefined";
    }
}

/***************************************************************************
* SubsNotifyGetReasonName
* ------------------------------------------------------------------------
* General: Returns the name of a given notify reason.
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV SubsNotifyGetReasonName (RvSipSubsNotifyReason  eReason)
{

    switch(eReason)
    {
    case RVSIP_SUBS_NOTIFY_REASON_SUBS_TERMINATED:
        return "Subs Terminated";
    case RVSIP_SUBS_NOTIFY_REASON_TRANSC_TIMEOUT:
        return "Transaction Timeout";
    case RVSIP_SUBS_NOTIFY_REASON_TRANSC_ERROR:
        return "Transaction Error";
    case RVSIP_SUBS_NOTIFY_REASON_NETWORK_ERROR:
        return "Network Error";
    case RVSIP_SUBS_NOTIFY_REASON_503_RECEIVED:
        return "503 received";
    case RVSIP_SUBS_NOTIFY_REASON_GIVE_UP_DNS:
        return "Give Up DNS";
    case RVSIP_SUBS_NOTIFY_REASON_CONTINUE_DNS:
        return "Continue DNS";
    case RVSIP_SUBS_NOTIFY_REASON_APP_COMMAND:
        return "App Command";
    case RVSIP_SUBS_NOTIFY_REASON_BAD_REQUEST_MESSAGE:
        return "Bad Request Message";
    case RVSIP_SUBS_NOTIFY_REASON_REFER_NO_SUBSCRIPTION_STATE_HEADER:
        return "Refer Notify With No Subscription-State Header";
    case RVSIP_SUBS_NOTIFY_REASON_REFER_NO_EXPIRES_IN_FIRST_NOTIFY:
        return "First Refer Notify with no Expires Header";
    default:
        return "Undefined";
    }
}

/***************************************************************************
* GetNotifySubsStateName
* ------------------------------------------------------------------------
* General: Returns the name of a given notify reason.
* Return Value: The string with the status name.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - The status as enum
***************************************************************************/
const RvChar*  RVCALLCONV GetNotifySubsStateName (RvSipSubscriptionSubstate  eSubsState)
{

    switch(eSubsState)
    {
    case RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE:
        return "Active";
    case RVSIP_SUBSCRIPTION_SUBSTATE_PENDING:
        return "Pending";
    case RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED:
        return "Terminated";
    case RVSIP_SUBSCRIPTION_SUBSTATE_OTHER  :
        return "Other";
    case RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED:
    default:
        return "Undefined";
    }
}

#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES*/

