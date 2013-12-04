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
 *                              <CallLegDNS.c>
 *
 * This file defines the contains implementation for DNS functions.
 * All functions in this file are for RV_DNS_ENHANCED_FEATURES_SUPPORT compilation
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 August 2002
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

#include "CallLegDNS.h"
#include "CallLegRefer.h"
#include "CallLegCallbacks.h"
#include "CallLegInvite.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus GiveUpDNSCallLegMsg (IN CallLeg* pCallLeg);

static RvStatus GiveUpDNSModifyMsg (IN CallLeg* pCallLeg, CallLegInvite* pInvite);

static RvStatus ContinueDNSCallLegMsg (IN CallLeg* pCallLeg, CallLegInvite* pInvite);

static RvStatus ContinueDNSModifyMsg (IN CallLeg* pCallLeg, CallLegInvite* pInvite);

static RvStatus continueDNSForGeneralActiveTransaction(IN CallLeg* pCallLeg,
                                                 OUT RvSipTranscHandle* phNewTransc);
static RvStatus continueDNSForInviteTransaction(IN CallLeg* pCallLeg,
                                                IN CallLegInvite* pInvite,
                                                IN RvBool  bFirstInvite,
                                                OUT RvSipTranscHandle* phNewTransc);

static RvStatus ReSendDNSCallLegMsg (IN CallLeg* pCallLeg);

static RvStatus ReSendDNSModifyMsg (IN CallLeg* pCallLeg, CallLegInvite* pInvite);


static RvStatus GiveUpDNSReferMsgIfNeeded (IN CallLeg* pCallLeg);

static RvStatus ContinueDNSReferMsgIfNeeded (IN CallLeg* pCallLeg);

static RvStatus ReSendDNSReferMsgIfNeeded (IN CallLeg* pCallLeg);

static RvStatus GetListDNSReferIfNeeded (IN  CallLeg                     * pCallLeg,
                                         OUT RvSipTransportDNSListHandle *phDnsList);

static void ReportLogOfDNSError(IN CallLeg      *pCallLeg,
                                IN const RvChar *strDNSAction,
                                IN CallLegInvite *pInvite);

static RvBool   IsSubsMsgSendFailure(CallLeg *pCallLeg);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, delete the sending of the message, and
 *          change state of the state machine back to previous state.
 *          You can use this function for INVITE, BYE, RE-INVITE and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSGiveUp (IN  CallLeg*   pCallLeg)
{
    RvStatus rv = RV_OK;
    CallLegInvite* pInvite = NULL;

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE)
    {
        rv = GiveUpDNSCallLegMsg(pCallLeg);
        return rv;
    }
    
    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc ,&pInvite);
    if (rv == RV_OK && pInvite != NULL &&
        pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE)
    {
        rv = GiveUpDNSModifyMsg(pCallLeg, pInvite);
    }
    else if(RV_TRUE == IsSubsMsgSendFailure(pCallLeg)) 
    {
        rv = GiveUpDNSReferMsgIfNeeded(pCallLeg);
    }
    else
    {
        /* error - no msg was failed */
        ReportLogOfDNSError(pCallLeg,"GiveUp", pInvite);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    return rv;
}

/***************************************************************************
 * CallLegDNSContinue
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, creates a new transaction that will
 *          be sent ti the next ip in dns list.
 *          You can use this function for INVITE, BYE, RE-INVITE and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSContinue (IN  CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;
    CallLegInvite* pInvite = NULL;
    SipTransactionMethod eMethod;

    SipTransactionGetMethod(pCallLeg->hActiveTransc, &eMethod);
        
    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc ,&pInvite);
    if (rv == RV_OK && pInvite!= NULL)
    {
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE)
        {
            rv = ContinueDNSCallLegMsg(pCallLeg, pInvite);
            return rv;
        }
        else if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE)/* re-invite */   
        {
            rv = ContinueDNSModifyMsg(pCallLeg, pInvite);
        }
    }
    else if (pCallLeg->ePrevState == RVSIP_CALL_LEG_STATE_DISCONNECTING &&
             eMethod == SIP_TRANSACTION_METHOD_BYE)
    {
        /* bye msg-sent-failure */
        rv = ContinueDNSCallLegMsg(pCallLeg,NULL);
        return rv;
    }
    else if (RV_TRUE == IsSubsMsgSendFailure(pCallLeg))
    {
        rv = ContinueDNSReferMsgIfNeeded(pCallLeg);
        return rv;
    }
    else
    {
        /* error - no msg was failed */
        ReportLogOfDNSError(pCallLeg,"Continue", pInvite);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    return rv;
}

/***************************************************************************
 * CallLegDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, re-send the message to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          You can use this function for INVITE, BYE, RE-INVITE, and
 *          REFER messages.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg which is responsible for the message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSReSendRequest (IN  CallLeg* pCallLeg)
{
    RvStatus rv = RV_OK;
    CallLegInvite* pInvite = NULL;
    
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE)
    {
        rv = ReSendDNSCallLegMsg(pCallLeg);
        return rv;
    }
    
    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc ,&pInvite);
    if (rv == RV_OK && pInvite != NULL)
    {
        rv = ReSendDNSModifyMsg(pCallLeg, pInvite);
    }
    else if (RV_TRUE == IsSubsMsgSendFailure(pCallLeg))
    {
        rv = ReSendDNSReferMsgIfNeeded(pCallLeg);
    }
    else
    {
        /* error - no msg was failed */
        ReportLogOfDNSError(pCallLeg,"ReSendRequest",pInvite);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    return rv;
}

/***************************************************************************
* CallLegDNSGetList
* ------------------------------------------------------------------------
* General: retrives DNS list object from the transaction object.
*
* Return Value: RV_OK or RV_ERROR_BADPARAM
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - call leg which is responsible for the message.
* Output   phDnsList - DNS list handle
***************************************************************************/
RvStatus RVCALLCONV CallLegDNSGetList(
                              IN  CallLeg                     *pCallLeg,
                              OUT RvSipTransportDNSListHandle *phDnsList)
{
    RvSipTranscHandle hTransc;
    RvStatus         rv;
    CallLegInvite* pInvite = NULL;

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE)
    {
        switch (pCallLeg->ePrevState)
        {
        case RVSIP_CALL_LEG_STATE_INVITING:
        case RVSIP_CALL_LEG_STATE_PROCEEDING:
        case RVSIP_CALL_LEG_STATE_DISCONNECTING:
            /* invite / bye is active transactions */
            hTransc = pCallLeg->hActiveTransc;
            break;
        default:
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegDNSGetList: call-leg 0x%p - invalid ePrevState %s for call-leg state MSG_SEND_FAILURE.",
                pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    else
    {
        rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc ,&pInvite);
        if (rv == RV_OK && pInvite!= NULL &&
            pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE)
        {
            switch (pInvite->ePrevModifyState)
            {
            case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT:
            case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING:
                hTransc = pCallLeg->hActiveTransc;
                break;
            default:
                RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegDNSGetList: call-leg 0x%p invite 0x%p - invalid ePrevModifyState %s for modify state MSG_SEND_FAILURE.",
                    pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));
                return RV_ERROR_ILLEGAL_ACTION;
            }
        }
        else if (RV_TRUE == IsSubsMsgSendFailure(pCallLeg))
        {
            rv = GetListDNSReferIfNeeded(pCallLeg,phDnsList);
            return rv;
        }    
        else
        {
            /* error - no msg was failed */
            ReportLogOfDNSError(pCallLeg,"GetList",pInvite);
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    rv = RvSipTransactionDNSGetList(hTransc, phDnsList);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSGetList: call-leg 0x%p - failed to get DNS list of tansaction 0x%p",
            pCallLeg, hTransc));
    }
    return rv;
}

/***************************************************************************
 * CallLegDNSTranscGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE status of a general
 *          transaction.
 *          Calling to this function, delete the sending of the message, and
 *          terminate it's transaction.
 *          You can use this function if you got MSG_SEND_FAILURE status in
 *          RvSipCallLegTranscResolvedEv event.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 *          hTransc  - Handle to the transaction the request belongs to.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscGiveUp (
                                            IN  CallLeg*             pCallLeg,
                                            IN  RvSipTranscHandle    hTransc)
{
    RvStatus rv;

    /* 1. terminate the transaction - in transcTerminated event, it will be
          removed from list */
    rv = RvSipTransactionTerminate(hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSTranscGiveUp: call-leg 0x%p - failed to terminate transaction 0x%p",
            pCallLeg, hTransc));
        return rv;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif
    return RV_OK;
}

/***************************************************************************
* CallLegDNSTranscContinue
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the message to the next ip
*          address.
*          You can use this function if you got MSG_SEND_FAILURE status in
*          RvSipCallLegTranscResolvedEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - call leg that sent the request.
*          hTransc  - Handle to the transaction the request belongs to.
***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscContinue (
                                               IN  CallLeg*             pCallLeg,
                                               IN  RvSipTranscHandle    hTransc,
                                               OUT RvSipTranscHandle*   phNewTransc)
{
    RvStatus rv;

    /* 1. continue DNS */
    rv = SipTransactionDNSContinue(hTransc, (RvSipTranscOwnerHandle)pCallLeg, RV_FALSE, phNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSTranscContinue: call-leg 0x%p - failed to continue DNS for tansaction 0x%p",
            pCallLeg, hTransc));
        return rv;
    }
    /* 2. add new transaction to call-leg list */
    rv = CallLegAddTranscToList(pCallLeg, *phNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSTranscContinue: call-leg 0x%p - failed to add new transaction 0x%p to list",
            pCallLeg, *phNewTransc));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
* CallLegDNSTranscReSendRequest
* ------------------------------------------------------------------------
* General: This function is for use at MSG_SEND_FAILURE state.
*          Calling to this function, re-send the message to the next ip
*          address.
*          You can use this function if you got MSG_SEND_FAILURE status in
*          RvSipCallLegTranscResolvedEv event.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
*                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
*               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send re-Invite.
*               RV_OK - Invite message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - call leg that sent the request.
*          hTransc  - Handle to the transaction the request belongs to.
***************************************************************************/
RvStatus RVCALLCONV CallLegDNSTranscReSendRequest (
                                               IN  CallLeg*             pCallLeg,
                                               IN  RvSipTranscHandle    hNewTransc)
{
    RvStatus rv;
    RvChar            strMethod[SIP_METHOD_LEN];

    /* send the new transaction.*/

    /* get the transaction method */
    rv = RvSipTransactionGetMethodStr(hNewTransc, SIP_METHOD_LEN, strMethod);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSTranscReSendRequest - call-leg 0x%p - Failed to get methodStr from transc 0x%p",
            pCallLeg, hNewTransc));
        return rv;
    }

    rv = CallLegSendRequest(pCallLeg, hNewTransc, SIP_TRANSACTION_METHOD_UNDEFINED, strMethod);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDNSTranscReSendRequest: call-leg 0x%p - Failed to re-send request (cloned transc 0x%p). (status %d)",
            pCallLeg, hNewTransc, rv));
        return rv;
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * GiveUpDNSCallLegMsg
 * ------------------------------------------------------------------------
 * General: This function is give up DNS when callLeg eState is MSG_FAILURE.
 *          the failed msg can be : BYE, INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus GiveUpDNSCallLegMsg (CallLeg* pCallLeg)
{
    RvStatus rv;
    RvSipTranscHandle hActiveTransc;
    CallLegInvite *pInvite = NULL;
    RvUint16         responseCode = 0;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "GiveUpDNSCallLegMsg: call-leg 0x%p - call prev state is %s.",
        pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));

    hActiveTransc = pCallLeg->hActiveTransc;

    if(hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GiveUpDNSCallLegMsg: call-leg 0x%p - active transaction is NULL!!!.",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    switch (pCallLeg->ePrevState)
    {
    case RVSIP_CALL_LEG_STATE_INVITING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING:

        /* 1. if (503 received) - send ACK */
        rv = RvSipTransactionGetResponseCode((RvSipTranscHandle)hActiveTransc, &responseCode);
        if ((rv == RV_OK) && (responseCode == 503))
        {
            rv = CallLegInviteSendAck(pCallLeg,hActiveTransc, NULL, 503);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "GiveUpDNSCallLegMsg: call-leg 0x%p - Failed to send ACK for 503. transc 0x%p",
                    pCallLeg, hActiveTransc));
                return rv;
            }
        }
        /*make the transaction a stand alone transaction to let is keep
          retransmit ack without the call-leg existence */
        rv = CallLegInviteFindObjByTransc(pCallLeg, hActiveTransc, &pInvite);
        if(rv != RV_OK || NULL == pInvite)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GiveUpDNSCallLegMsg: call-leg 0x%p - Failed to get invite obj from transc 0x%p.",
                pCallLeg, hActiveTransc));
            return RV_ERROR_UNKNOWN;
        }
        rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hActiveTransc);
        /* no need to check the return value - if function failed, the transaction
        is terminated */
        CallLegResetActiveTransc(pCallLeg);
        /* 2. terminate the call-leg */
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        break;

    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        CallLegResetActiveTransc(pCallLeg);
        /* 1. Terminate BYE transaction (this function removes it from list too) */
        rv = RvSipTransactionTerminate(hActiveTransc);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GiveUpDNSCallLegMsg: call-leg 0x%p - Failed to terminate BYE transaction 0x%p.",
                pCallLeg, hActiveTransc));
            return rv;
        }

        /* 2. move to disconnected and then to terminated */
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        break;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GiveUpDNSCallLegMsg: call-leg 0x%p - nothing to do with call prev state %s.",
            pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return RV_OK;
}


/***************************************************************************
 * GiveUpDNSModifyMsg
 * ------------------------------------------------------------------------
 * General: This function is give up DNS when callLeg eModifyState is MSG_FAILURE.
 *          the failed msg can be : RE-INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus GiveUpDNSModifyMsg (CallLeg* pCallLeg, CallLegInvite* pInvite)
{
    RvStatus rv;
    RvSipTranscHandle hActiveTransc;
    RvUint16         responseCode = 0;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "GiveUpDNSModifyMsg: call-leg 0x%p invite 0x%p - prev modify state is %s.",
        pCallLeg,pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));

    hActiveTransc = pCallLeg->hActiveTransc;
    if(hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GiveUpDNSModifyMsg: call-leg 0x%p invite 0x%p - active transaction is NULL!!!.",
            pCallLeg, pInvite));
        return RV_ERROR_UNKNOWN;
    }

    if(pInvite->ePrevModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT &&
       pInvite->ePrevModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GiveUpDNSModifyMsg: call-leg 0x%p invite 0x%p  - nothing to do with call prev modify state %s.",
            pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* 1. if (503 received) - send ACK */
    rv = RvSipTransactionGetResponseCode((RvSipTranscHandle)hActiveTransc, &responseCode);
    if ((rv == RV_OK) && (responseCode == 503))
    {
        rv = CallLegInviteSendAck(pCallLeg, hActiveTransc, NULL, 503);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GiveUpDNSModifyMsg: call-leg 0x%p invite 0x%p - Failed to send ACK for 503. transc 0x%p",
                pCallLeg, pInvite, hActiveTransc));
            return rv;
        }
        /* 2. reset active transaction */
        CallLegResetActiveTransc(pCallLeg);
        
        /* 3. If ACK was sent, we inform application about it. 
              when the transaction informs the call-leg that the ack was realy sent,
              the invite is terminated. */
        CallLegChangeModifyState(pCallLeg, pInvite,
                                RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
                                RVSIP_CALL_LEG_REASON_GIVE_UP_DNS);
    }
    else
    {
        rv = RvSipTransactionTerminate(hActiveTransc);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GiveUpDNSModifyMsg: call-leg 0x%p invite 0x%p - Failed to terminate modify transaction 0x%p.",
                pCallLeg, pInvite,pCallLeg->hActiveTransc));
            return rv;
        }

        /* 2. reset active transaction */
        CallLegResetActiveTransc(pCallLeg);
        
        /* 3. no ACK was sent. we can terminate the invite object here. */
        CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_GIVE_UP_DNS);
    }
    

    return RV_OK;
}


/***************************************************************************
 * ContinueDNSCallLegMsg
 * ------------------------------------------------------------------------
 * General: This function continue DNS when callLeg eState is MSG_FAILURE.
 *          the failed msg can be : BYE, INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ContinueDNSCallLegMsg (CallLeg* pCallLeg,
                                       CallLegInvite* pInvite)
{
    RvStatus rv;
    RvSipTranscHandle hNewTransc;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "ContinueDNSCallLegMsg: call-leg 0x%p - call prev state is %s.",
        pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));

    if(pCallLeg->hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ContinueDNSCallLegMsg: call-leg 0x%p - active transaction is NULL!!!.",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    switch (pCallLeg->ePrevState)
    {
    case RVSIP_CALL_LEG_STATE_INVITING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
        /* continue DNS + send ACK on 503 */
        rv = continueDNSForInviteTransaction(pCallLeg, pInvite, RV_TRUE, &hNewTransc);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "ContinueDNSCallLegMsg: call-leg 0x%p - Failed to continue DNS for INVITE transaction",
                pCallLeg));
            return rv;
        }
        break;

    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        rv = continueDNSForGeneralActiveTransaction(pCallLeg, &hNewTransc);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "ContinueDNSCallLegMsg: call-leg 0x%p - Failed to continue DNS for BYE transaction",
                pCallLeg));
            return rv;
        }
        break;

    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ContinueDNSCallLegMsg: call-leg 0x%p - nothing to do with call prev state %s.",
            pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return RV_OK;
}



/***************************************************************************
 * ContinueDNSModifyMsg
 * ------------------------------------------------------------------------
 * General: This function continue DNS when callLeg eModifyState is MSG_FAILURE.
 *          the failed msg can be : RE-INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ContinueDNSModifyMsg (CallLeg* pCallLeg, CallLegInvite* pInvite)
{
    RvStatus rv;
    RvSipTranscHandle hNewTransc;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "ContinueDNSModifyMsg: call-leg 0x%p invite 0x%p - call prev modify state is %s.",
        pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));

    if(pCallLeg->hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ContinueDNSModifyMsg: call-leg 0x%p invite 0x%p - active transaction is NULL!!!.",
            pCallLeg, pInvite));
        return RV_ERROR_UNKNOWN;
    }

    if(pInvite->ePrevModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT &&
       pInvite->ePrevModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ContinueDNSModifyMsg: call-leg 0x%p invite 0x%p - nothing to do with call prev state %s.",
            pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* continue DNS + send ACK on 503 */
    rv = continueDNSForInviteTransaction(pCallLeg, pInvite, RV_FALSE, &hNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ContinueDNSModifyMsg: call-leg 0x%p invite 0x%p - Failed to continue DNS for RE-INVITE transaction",
            pCallLeg, pInvite));
        return rv;
    }

    return RV_OK;
}


/***************************************************************************
 * continueDNSForGeneralActiveTransaction
 * ------------------------------------------------------------------------
 * General: This function continue DNS for non-invite active transaction.
 *           (good for BYE, REFER)
 *          function actions are:
 *          1. continueDNS for transaction.
 *          2. remove transaction from transactions list.
 *          3. add new transaction to list, and set active transaction.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 * Output:  phNewTransc - the new transaction to be send.
 ***************************************************************************/
static RvStatus continueDNSForGeneralActiveTransaction(IN CallLeg* pCallLeg,
                                                 OUT RvSipTranscHandle* phNewTransc)
{
    RvStatus rv;
    RvSipTranscHandle hActiveTransc = NULL;

    /* 1. continueDNS for transaction */
    hActiveTransc = pCallLeg->hActiveTransc;

    rv = SipTransactionDNSContinue(hActiveTransc,
                                    (RvSipTranscOwnerHandle)pCallLeg,
                                    RV_FALSE,
                                    phNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "continueDNSForGeneralActiveTransaction: call-leg 0x%p - Failed to continue DNS for transaction 0x%p. (status %d)",
            pCallLeg, hActiveTransc, rv));
        return rv;
    }

    /* 2. no need to remove transaction from list. this is done inside continueDNS
          function, in transcTerminated event. */

    /* 3. add new transaction to list, and set active transaction */
    rv = CallLegAddTranscToList(pCallLeg, *phNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "continueDNSForGeneralActiveTransaction: call-leg 0x%p - Failed to add new transaction 0x%p to list. (status %d)",
            pCallLeg, *phNewTransc, rv));
        return rv;
    }

    CallLegSetActiveTransc(pCallLeg, *phNewTransc);
    return RV_OK;
}

/***************************************************************************
 * continueDNSForInviteTransaction
 * ------------------------------------------------------------------------
 * General: This function continue DNS for an INVITE transaction. (good for
 *          INVITE, RE_INVITE)
 *          function actions are:
 *          1. continueDNS for transaction.
 *          2. send ACK if needed.
 *          3. add new transaction to list, and set active transaction.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - call leg that sent the request.
 *          bFirstInvite - If first invite, we shall remove the to-tag from
 *                       cloned transaction. if re-invite, we should not.
 * Output:  phNewTransc - the new invite transaction to be send.
***************************************************************************/
static RvStatus continueDNSForInviteTransaction(IN CallLeg* pCallLeg,
                                                IN CallLegInvite* pInvite,
                                                IN RvBool  bFirstInvite,
                                                OUT RvSipTranscHandle* phNewTransc)
{
    RvStatus rv;
    RvUint16 responseCode = 0;
    CallLegTranscMemory* pMemory;

    /* 1. continueDNS for transaction */
    rv = SipTransactionDNSContinue(pCallLeg->hActiveTransc,
                                    (RvSipTranscOwnerHandle)pCallLeg,
                                    bFirstInvite,
                                    phNewTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "continueDNSForInviteTransaction: call-leg 0x%p - Failed to continue DNS for transaction 0x%p. (status %d)",
            pCallLeg, pCallLeg->hActiveTransc, rv));
        return rv;
    }

    /* 2. if 503 - send ACK */
    rv = RvSipTransactionGetResponseCode(pCallLeg->hActiveTransc, &responseCode);
    if ((rv == RV_OK) && (responseCode == 503))
    {
        rv = CallLegInviteSendAck(pCallLeg,pCallLeg->hActiveTransc, NULL, 503);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "continueDNSForInviteTransaction: call-leg 0x%p - Failed to send ACK for 503. transc 0x%p",
                pCallLeg, pCallLeg->hActiveTransc));
            return rv;
        }
        if(RV_FALSE == bFirstInvite) /* re-invite */
        {
             /* ACK for re-invite was sent, so we inform application about it. 
            when the transaction informs the call-leg that the ack was really sent, the re-invite is terminated. */
            CallLegChangeModifyState(pCallLeg, pInvite,
                RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
                RVSIP_CALL_LEG_REASON_GIVE_UP_DNS);
        }
    }

    /* 3. add new transaction to list, and set active transaction */
    /*lint -e413*/
	rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
   
    pInvite->hInviteTransc = *phNewTransc;
   
    /* save a pointer to the invite object in the transaction object */
    rv = CallLegAllocTransactionCallMemory(*phNewTransc, &pMemory);
    if(rv != RV_OK || NULL == pMemory)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "continueDNSForInviteTransaction - Call-leg 0x%p - failed to set invite pointer in new transaction 0x%p",
            pCallLeg, *phNewTransc));
        return RV_ERROR_OUTOFRESOURCES;
    }
    pMemory->pInvite = pInvite;
    
    CallLegSetActiveTransc(pCallLeg, *phNewTransc);
    return RV_OK;
}



/***************************************************************************
 * ReSendDNSCallLegMsg
 * ------------------------------------------------------------------------
 * General: This function re-send DNS when callLeg eState is MSG_FAILURE.
 *          the failed msg can be : BYE, INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ReSendDNSCallLegMsg (CallLeg* pCallLeg)
{
    RvStatus rv;
    RvSipTranscHandle hNewTransc;
    RvSipCallLegState  eNewState;
    SipTransactionMethod eMethod;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "ReSendDNSCallLegMsg: call-leg 0x%p - call prev state is %s.",
        pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));

    if(pCallLeg->hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ReSendDNSCallLegMsg: call-leg 0x%p - active transaction is NULL !!!.",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    switch (pCallLeg->ePrevState)
    {
    case RVSIP_CALL_LEG_STATE_INVITING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
        eNewState = RVSIP_CALL_LEG_STATE_INVITING;
        eMethod = SIP_TRANSACTION_METHOD_INVITE;
        hNewTransc = pCallLeg->hActiveTransc;
        break;

    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        eNewState = RVSIP_CALL_LEG_STATE_DISCONNECTING;
        eMethod = SIP_TRANSACTION_METHOD_BYE;
        hNewTransc = pCallLeg->hActiveTransc;
        break;

    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ReSendDNSCallLegMsg: call-leg 0x%p - nothing to do with call prev state %s.",
            pCallLeg, CallLegGetStateName(pCallLeg->ePrevState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegSendRequest(pCallLeg, hNewTransc, eMethod, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ReSendDNSCallLegMsg: call-leg 0x%p - Failed to re-send request (cloned transc 0x%p). (status %d)",
            pCallLeg, hNewTransc, rv));
        return rv;
    }

    /* Change call state back to DISCONNECTING */
    CallLegChangeState(pCallLeg, eNewState, RVSIP_CALL_LEG_REASON_CONTINUE_DNS);

    return RV_OK;
}

/***************************************************************************
 * ReSendDNSModifyMsg
 * ------------------------------------------------------------------------
 * General: This function re-send DNS when callLeg eState is MSG_FAILURE.
 *          the failed msg can be : RE-INVITE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ReSendDNSModifyMsg (CallLeg* pCallLeg, CallLegInvite* pInvite)
{
    RvStatus rv;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "ReSendDNSModifyMsg: call-leg 0x%p invite 0x%p - call prev modify state is %s.",
        pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->ePrevModifyState)));

    if(pCallLeg->hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ReSendDNSModifyMsg: call-leg 0x%p invite 0x%p - active transaction is NULL!!!.",
            pCallLeg, pInvite));
        return RV_ERROR_UNKNOWN;
    }


    rv = CallLegSendRequest(pCallLeg, pCallLeg->hActiveTransc, SIP_TRANSACTION_METHOD_INVITE, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ReSendDNSModifyMsg: call-leg 0x%p invite 0x%p - Failed to re-send request (cloned transc 0x%p). (status %d)",
            pCallLeg, pInvite, pCallLeg->hActiveTransc, rv));
        return rv;
    }

    /* Change modify state */
    CallLegChangeModifyState(pCallLeg, pInvite,
                             RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT, 
                             RVSIP_CALL_LEG_REASON_CONTINUE_DNS);

    return RV_OK;
}

/***************************************************************************
 * GiveUpDNSReferMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: This function is give up DNS when callLeg eState is
 *          RVSIP_SUBS_STATE_MSG_SEND_FAILURE.
 *          the failed msg can be : REFER, SUBSCRIBE
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus GiveUpDNSReferMsgIfNeeded (IN CallLeg* pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
    return CallLegReferDNSGiveUp(pCallLeg);
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * ContinueDNSReferMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: This function continue DNS when callLeg eState is
 *          RVSIP_SUBS_STATE_MSG_SEND_FAILURE.
 *          the failed msg can be : REFER, SUBSCRIBE
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ContinueDNSReferMsgIfNeeded (IN CallLeg* pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
    return CallLegReferDNSContinue(pCallLeg);
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * ReSendDNSReferMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: This function re-send DNS when callLeg eState is
 *          RVSIP_SUBS_STATE_MSG_SEND_FAILURE.
 *          the failed msg can be : REFER, SUBSCRIBE
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvStatus ReSendDNSReferMsgIfNeeded (IN CallLeg* pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
    return CallLegReferDNSReSend(pCallLeg);
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * GetListDNSReferIfNeeded
 * ------------------------------------------------------------------------
 * General: This function re-send DNS when callLeg eState is
 *          RVSIP_SUBS_STATE_MSG_SEND_FAILURE.
 *          the failed msg can be : REFER, SUBSCRIBE
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg  - call leg that sent the request.
 * Output:    phDnsList - The CallLeg DNS list.
 ***************************************************************************/
static RvStatus GetListDNSReferIfNeeded (IN  CallLeg                     * pCallLeg,
                                         OUT RvSipTransportDNSListHandle *phDnsList)
{
	*phDnsList = NULL;
#ifdef RV_SIP_SUBS_ON
    return CallLegReferDNSGetList(pCallLeg, phDnsList);  
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * ReportLogOfDNSError
 * ------------------------------------------------------------------------
 * General: Report the log of an error in the DNS functions
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg     - call leg that sent the request.
 *         strDNSAction - A string that identifies a specific DNS action.
 ***************************************************************************/
static void ReportLogOfDNSError(IN CallLeg      *pCallLeg,
                                IN const RvChar *strDNSAction,
                                IN CallLegInvite *pInvite)
{
#ifdef RV_SIP_SUBS_ON
#if (RV_LOGMASK & RV_LOGLEVEL_INFO)    
    const RvChar *strSubsState = CallLegReferGetSubsStateName(pCallLeg);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
#else
    const RvChar *strSubsState = "Irrelevant";
#endif /* #ifdef RV_SIP_SUBS_ON */

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegDNS%s: call-leg 0x%p - no msg was failed. call state %s, modify state %s, refer state %s",
        strDNSAction,pCallLeg, CallLegGetStateName(pCallLeg->eState),
        (pInvite==NULL)?"Undefined":CallLegGetModifyStateName(pInvite->eModifyState),
        strSubsState));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((pInvite,strDNSAction,pCallLeg));
#endif

}

/***************************************************************************
 * IsSubsMsgSendFailure
 * ------------------------------------------------------------------------
 * General: This function check if the input CallLeg is in state
 *          RVSIP_SUBS_STATE_MSG_SEND_FAILURE.
 *          the failed msg can be : REFER, SUBSCRIBE
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call leg that sent the request.
 ***************************************************************************/
static RvBool IsSubsMsgSendFailure(CallLeg *pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
    if (pCallLeg->hActiveReferSubs != NULL &&
        CallLegReferGetCurrentReferSubsState(pCallLeg) == RVSIP_SUBS_STATE_MSG_SEND_FAILURE)
    {
        return RV_TRUE;
    }
#else
    RV_UNUSED_ARG(pCallLeg);
#endif /*#ifdef RV_SIP_SUBS_ON*/
    return RV_FALSE;
}

#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

#endif /*#ifdef RV_SIP_PRIMITIVES*/

