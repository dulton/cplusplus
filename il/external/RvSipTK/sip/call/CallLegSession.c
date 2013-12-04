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
 *                              <sipCallLegSession.c>
 *
 *  Handles all session request initiated by the user such as
 *  connecting a call-leg, accepting or rejecting a call-leg, disconnecting
 *  call-leg and more.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLegTypes.h"
#include "CallLegSession.h"
#include "CallLegSessionTimer.h"
#include "CallLegMgrObject.h"
#include "CallLegCallbacks.h"
#include "_SipCommonUtils.h"
#include "CallLegRefer.h"
#include "CallLegMsgEv.h"
#include "CallLegRouteList.h"
#include "CallLegInvite.h"

#include "RvSipRAckHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipOtherHeader.h"
#include "_SipMsg.h"
#include "_SipOtherHeader.h"

#include "_SipTransmitterMgr.h"
#include "_SipTransmitter.h"
#include "RvSipTransmitter.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV AddPrackInfoToMsg(
                                    IN  CallLeg*    pCallLeg,
                                    IN  RvSipMsgHandle hMsg);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSessionConnect
 * ------------------------------------------------------------------------
 * General: Initiate an outgoing call. This method may be called
 *          only after the To, From fields have been set. Calling
 *          Connect causes an INVITE to be sent out and the
 *          call-leg state machine progresses to the Inviting state.
 *          If the function failes the call-leg will be terminated.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Cannot connect due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send the invite message.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to connect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionConnect (IN  CallLeg *pCallLeg)
{
    RvSipTranscHandle hTransc;
    RvStatus rv;
    CallLegInvite*  pInvite = NULL;

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_REDIRECTED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_UNAUTHENTICATED) /* use the already exist invite object */
    {
        /* initial invite object state is always UNDEFINED ir TERMINATED */
        rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED, &pInvite);
        if(pInvite == NULL || rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionConnect - Failed for Call-leg 0x%p - failed to find invite obj on state %s",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        if(pInvite->hInviteTransc != NULL)
        {
            /* if the connect function was called synchronically, in the change state callback,
               then the transaction was not detached from invite object yet... */
            pCallLeg->bInviteReSentInCB = RV_TRUE;
            rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionConnect - Failed for Call-leg 0x%p - failed to detach transc from invite obj",pCallLeg));
                CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
                CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
                return RV_ERROR_UNKNOWN;
            }
        }
    }
    else if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)/* state idle - create an invite object */   
    {
        /*-----------------------
        Create a new invite object
        -------------------------*/
        rv = CallLegInviteCreateObj(pCallLeg, NULL, RV_FALSE, RVSIP_CALL_LEG_DIRECTION_OUTGOING, &pInvite, NULL);
        if (rv != RV_OK || NULL == pInvite)
        {
            /* invite construction failed*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionConnect - Failed for Call-leg 0x%p - failed to create invite obj",pCallLeg));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionConnect - Call-leg 0x%p - illegal state %s",pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /*------------------------------------------------------------------------
     create a new transaction - do not add the transaction to call-leg's list.
    --------------------------------------------------------------------------*/
    rv = CallLegCreateTransaction(pCallLeg, RV_FALSE, &hTransc);

    if (rv != RV_OK || NULL == hTransc)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSessionConnect - Failed for Call-leg 0x%p - failed to create transaction",pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

   
    rv = CallLegInviteSetParams(pCallLeg, pInvite, hTransc);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionConnect - Failed for Call-leg 0x%p - failed to init the invite object. terminae call...",pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /*-----------------------------------------
      Insert call-leg into hash table if this
      is the first invite for this call-leg
      ----------------------------------------*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        rv = CallLegMgrHashInsert((RvSipCallLegHandle)pCallLeg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionConnect - Failed to insert Call-leg 0x%p - to hash (rv=%d) - Terminating call",
                pCallLeg,rv));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return rv;
        }
    }
    /*----------------------------------
         send the INVITE request
     -----------------------------------*/
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionConnect - Call-leg 0x%p sending INVITE",pCallLeg));

    rv = CallLegSendRequest(pCallLeg,hTransc,SIP_TRANSACTION_METHOD_INVITE,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionConnect - Failed for Call-leg 0x%p  - failed to send Invite message with transc 0x%p - Terminating call",
            pCallLeg,hTransc));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /*set the invite transaction as the active transaction*/
    CallLegSetActiveTransc(pCallLeg, hTransc);
    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_INVITING,
                                    RVSIP_CALL_LEG_REASON_LOCAL_INVITING);

    return RV_OK;
}



/***************************************************************************
 * CallLegSessionAccept
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is willing to accept an incoming call.
 *          If the function failes the call-leg will be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Failed to accept the call.
 *               RV_OK - 200 final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to accept
 * Output:  (-)
 ***************************************************************************/

RvStatus RVCALLCONV CallLegSessionAccept (IN  CallLeg *pCallLeg)
{
    RvStatus         rv;
    RvSipTranscHandle hTransc = pCallLeg->hActiveTransc;
    CallLegInvite*  pInvite;

    if(hTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionAccept - Failed, Call-leg 0x%p - no active transaction",pCallLeg));

        return RV_ERROR_UNKNOWN;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionAccept - Call-leg 0x%p sending 200",pCallLeg));

    rv = CallLegInviteFindObjByTransc(pCallLeg,hTransc,&pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionAccept - Call-leg 0x%p - did not find invite object.",pCallLeg));
        return rv;
    }

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED &&
       (pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD &&
       pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED &&
       pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionAccept - Failed to accept call-leg 0x%p: Illegal Action in modify state %s",
            pCallLeg, CallLegGetModifyStateName(pInvite->eModifyState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED &&
       pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED &&
       pCallLeg->pMgr->manualBehavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionAccept - Failed to accept call-leg 0x%p: Illegal Action in modify state %s, manualBehavior = %d",
            pCallLeg, CallLegGetModifyStateName(pInvite->eModifyState), pCallLeg->pMgr->manualBehavior));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    /* if there is a waiting PRACK transaction, we must accept it first */
    if (pCallLeg->pMgr->manualPrack == RV_TRUE &&
        pCallLeg->ePrackState == RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionAccept - Call-leg 0x%p accepting PRACK request first",pCallLeg));
        rv = CallLegSessionSendPrackResponse(pCallLeg, 200);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionAccept - Call-leg 0x%p Failed to accept PRACK request first. continue with accepting the INVITE",
              pCallLeg));
        }
    }

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionAccept - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    rv = RvSipTransactionRespond(hTransc,200,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond - terminate call-leg*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionAccept - Call-leg 0x%p - Failed to send response (rv=%d) - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    if(RV_TRUE == pCallLeg->pMgr->bEnableNestedInitialRequestHandling)
    {
        /* now the call-leg has a to-tag, we can insert it to the hash table */
        if(pCallLeg->hashElementPtr == NULL)
        {
            /* now the call-leg has a to-tag, we can insert it to the hash table */
            rv = CallLegMgrHashInsert((RvSipCallLegHandle)pCallLeg);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
    }
    /* if this is accept on first invite change the call leg state to accepted*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLED)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_ACCEPTED,
                                    RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED);
    }
    else
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                                 RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED);
    }

    return RV_OK;

}

/***************************************************************************
 * CallLegSessionByeAccept
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is willing to accept an incoming BYE request.
 *          If the function failes the call-leg will be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Failed to accept the call.
 *               RV_OK - 200 final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to accept its BYE
 *          hTransc  - handle to the BYE transaction
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionByeAccept (IN  CallLeg          *pCallLeg,
                                              IN  RvSipTranscHandle hTransc)
{
    RvStatus         rv;
    SipTransactionMethod eMethod;

    if(hTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionByeAccept - Failed, hTransc is NULL"));

        return RV_ERROR_UNKNOWN;
    }
    SipTransactionGetMethod(hTransc, &eMethod);
    if(eMethod != SIP_TRANSACTION_METHOD_BYE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionByeAccept - Failed, transaction method is not BYE"));

        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionByeAccept - Call-leg 0x%p sending 200 to BYE transaction 0x%p",pCallLeg, hTransc));

    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionByeAccept - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                        ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }


    rv = RvSipTransactionRespond(hTransc,200,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond - terminate call-leg*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionByeAccept - Call-leg 0x%p - Failed to send response (rv=%d) - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    CallLegRemoveTranscFromList(pCallLeg,hTransc);
    if(SipTransactionDetachOwner(hTransc) != RV_OK)
    {
        RvSipTransactionTerminate(hTransc);
    }
    else
    {
        CallLegCallbackByeStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                      hTransc,RVSIP_CALL_LEG_BYE_STATE_DETACHED,
                                      RVSIP_TRANSC_REASON_UNDEFINED);
    }
    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECTED);

    return RV_OK;

}

/***************************************************************************
 * CallLegSessionReject
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is rejecting an incoming call. This method is valid only
 *          in the offering state or the connected/modified.
 *          If the function failes the call-leg will be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send reject response.
 *               RV_OK - Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to reject
 *            status - The rejection response code.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionReject (
                                         IN  CallLeg   *pCallLeg,
                                         IN  RvUint16 status)
{
    RvStatus         rv;
    RvSipTranscHandle hTransc = pCallLeg->hActiveTransc;
    CallLegInvite*            pInvite = NULL;

    /*no active transaction -  exception in this state*/
    if(pCallLeg->hActiveTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionReject - Call-leg 0x%p - No active transaction to send response with - Terminating call",
            pCallLeg));

        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    rv = CallLegInviteFindObjByTransc(pCallLeg,hTransc,&pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionReject - Call-leg 0x%p - did not find invite object.",pCallLeg));
        return rv;
    }
    

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionReject - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    pCallLeg->hReplacesHeader = NULL;
    rv = RvSipTransactionRespond(hTransc,status,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionReject -  Call-leg 0x%p - Failed to send reject response (rv=%d) - - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    /*change the call leg state to disconnected*/

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLED ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                    RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
    }
    else if (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD)
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                                     RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                                     RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
    }
    return RV_OK;

}

/***************************************************************************
 * CallLegSessionByeReject
 * ------------------------------------------------------------------------
 * General: This method may be called by the application to indicate that it
 *          is rejecting an incoming BYE request
 * Return Value: RV_ERROR_UNKNOWN - Failed to send reject response.
 *               RV_OK - Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to reject its BYE
 *          hTransc  - Handle to the BYE transaction
 *            status   - The rejection response code.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionByeReject (
                                         IN  CallLeg          *pCallLeg,
                                         IN  RvSipTranscHandle hTransc,
                                         IN  RvUint16         status)
{
    RvStatus         rv;
    SipTransactionMethod eMethod;

    if(hTransc == NULL)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionByeReject - hTransc is NULL"));

        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }
    SipTransactionGetMethod(hTransc, &eMethod);
    if(eMethod != SIP_TRANSACTION_METHOD_BYE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionByeReject - Failed, transaction method is not BYE"));

        return RV_ERROR_ILLEGAL_ACTION;
    }
    pCallLeg->hReplacesHeader = NULL;


    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionByeReject - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                        ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    rv = RvSipTransactionRespond(hTransc,status,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionByeReject -  Call-leg 0x%p - Failed to send reject response (rv=%d) - - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_ERROR_UNKNOWN;
    }

    /*detach the transaction in tcp, tls and sctp*/
    if(SipTransactionGetTransport(hTransc) == RVSIP_TRANSPORT_TCP ||
        SipTransactionGetTransport(hTransc) == RVSIP_TRANSPORT_TLS ||
        SipTransactionGetTransport(hTransc) == RVSIP_TRANSPORT_SCTP)
    {
        rv = CallLegRemoveTranscFromList(pCallLeg,hTransc);
        if (rv == RV_OK)
        {
            if(SipTransactionDetachOwner(hTransc) != RV_OK)
            {
                RvSipTransactionTerminate(hTransc);
            }
            else
            {
                CallLegCallbackByeStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                                  hTransc,RVSIP_CALL_LEG_BYE_STATE_DETACHED,
                                                  RVSIP_TRANSC_REASON_UNDEFINED);
            }
        }
    }

    return RV_OK;
}


/***************************************************************************
 * CallLegSessionProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Sends a provisional response (1xx) to the remote party.
 *          This function can be called when ever a request is received for
 *          example in the offering state.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg.
 *            status - The provisional response code.
 *          b_isReliable - is the provisional response reliable
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionProvisionalResponse (
                                         IN  CallLeg   *pCallLeg,
                                         IN  RvUint16 status,
                                         IN  RvBool   b_isReliable)
{
    RvStatus         rv;
    RvSipTranscHandle hTransc = pCallLeg->hActiveTransc;

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionProvisionalResponse - Failure in Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    if(b_isReliable == RV_FALSE)
    {
        rv = RvSipTransactionRespond(hTransc,status,NULL);
        if (rv != RV_OK)
        {
            /*transaction failed to send respond*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionProvisionalResponse - Call-leg 0x%p - Failed to send response (rv=%d)",
                pCallLeg,rv));

            return rv;
        }
    }
    else
    {
        rv = RvSipTransactionRespondReliable(hTransc,status,NULL);
        if (rv != RV_OK)
        {
            /*transaction failed to send respond*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionProvisionalResponse - Call-leg 0x%p - Failed to send reliable response (rv=%d)",
                pCallLeg,rv));

            return rv;
        }
    }
     /* 1xx bigger than 100 creates dialog */
    if(status > 100 &&
        RV_TRUE == pCallLeg->pMgr->bEnableNestedInitialRequestHandling)
    {
        /* now the call-leg has a to-tag, we can insert it to the hash table */
        if(pCallLeg->hashElementPtr == NULL)
        {
            /* now the call-leg has a to-tag, we can insert it to the hash table */
            rv = CallLegMgrHashInsert((RvSipCallLegHandle)pCallLeg);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
    }
    return RV_OK;
}

/***************************************************************************
 * CallLegSessionSendPrack
 * ------------------------------------------------------------------------
 * General: Sends a manual PRACK message to a reliable provisional
 *            response.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to ack.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendPrack(IN  CallLeg   *pCallLeg)
{
    RvStatus   rv = RV_OK;
    RvSipTranscHandle hTransc;
    RvSipMsgHandle  hMsg;

    /* -----------------------------------------------------------------------
       1. Create a PRACK transaction, and add it to call-leg transactions list
       ----------------------------------------------------------------------- */

        rv = CallLegCreateTransaction(pCallLeg, RV_TRUE, &hTransc);

        if (rv != RV_OK || NULL == hTransc)
        {
            /*transc construction failed*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionSendPrack - Call-leg 0x%p - failed to create transaction",pCallLeg));
            return rv;
        }

    /* -----------------------------------------------------------------------
       2. Set information in PRACK message
       ----------------------------------------------------------------------- */
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionSendPrack - Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        hMsg = pCallLeg->hOutboundMsg;
        pCallLeg->hOutboundMsg = NULL;
    }
    else
    {
        rv = RvSipTransactionGetOutboundMsg(hTransc, &hMsg);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionSendPrack - Call-leg 0x%p, failed to get transaction 0x%p outbound message (rv=%d:%s)",
              pCallLeg, hTransc, rv, SipCommonStatus2Str(rv)));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return rv;
        }
    }
    rv = AddPrackInfoToMsg(pCallLeg, hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSessionSendPrack - Call-leg 0x%p  - failed to set params in PRACK message",
                  pCallLeg));

        CallLegRemoveTranscFromList(pCallLeg,hTransc);
        SipTransactionTerminate(hTransc);
        return rv;
    }

    /* -----------------------------------------------------------------------
       3. Send PRACK request
       ----------------------------------------------------------------------- */
    rv = CallLegSendRequest(pCallLeg,hTransc,SIP_TRANSACTION_METHOD_PRACK,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSessionSendPrack - Call-leg 0x%p  - failed to send PRACK message with transc 0x%p",
                  pCallLeg, hTransc));

        CallLegRemoveTranscFromList(pCallLeg,hTransc);
        SipTransactionTerminate(hTransc);
        return rv;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegSessionSendPrack - Call-leg 0x%p sent PRACK",pCallLeg));

    return rv;
}

/***************************************************************************
 * CallLegSessionSendPrackResponse
 * ------------------------------------------------------------------------
 * General: Sends a manual response to PRACK message.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - Pointer to the call leg the user wishes to ack.
 *            responseCode - The response code to send.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendPrackResponse(IN  CallLeg   *pCallLeg,
                                                     IN  RvUint16  responseCode)
{
    RvStatus             rv              = RV_OK;
    RvSipTranscHandle    hTransc      = NULL;

    /* Finds the PRACK transaction that suits the active transaction. */
    rv = CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_FIND_PRACK_TRANSC, &hTransc);
    if(rv != RV_OK || NULL == hTransc)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSessionSendPrackResponse - Call-leg 0x%p - didn't find prack transaction in the transaction list",
                  pCallLeg));
        return rv;
    }
    /*hPrackParent == pCallLeg->hActiveTransc*/
    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionSendPrackResponse - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    rv = SipTransactionSendPrackResponse(hTransc,responseCode);

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionSendPrackResponse - Failed for call-leg 0x%p",pCallLeg));
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegSessionSendPrackResponse - Call-leg 0x%p sent PRACK response",pCallLeg));

    return rv;
}

/***************************************************************************
 * CallLegSessionAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request from the call-leg to the remote party.
 *          This function can be called in the remote accepted state
 *          for initial invite or in the ModifyResultRcvd call back for
 *          a re-Invite request.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to ack.
 *            pInvite  - Pointer to the invite object. must be given for sending
 *                      ACK on a re-invite with new invite behavior.
 *                      NULL for ACK on initial invite, or in old behavior.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionAck (IN  CallLeg   *pCallLeg,
                                       IN  CallLegInvite *pInvite)
{
    RvStatus   rv = RV_OK;
    CallLegInvite*  pFoundInvite = NULL;
    RvBool      bAckOnForked2xx = RV_FALSE;

    if(pCallLeg->hOriginalForkedCall != NULL &&
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)
    {
        /* A forked call-leg and ACK is sent for initial INVITE ==> we don't have an active transaction */
        bAckOnForked2xx = RV_TRUE;
    }
    /* 1. find the correct invite object */
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        if(bAckOnForked2xx == RV_TRUE)
        {
            /* We don't have an active transaction */
            rv = CallLegInviteFindObjByState(pCallLeg,RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED,&pFoundInvite);
            if (RV_OK != rv || pFoundInvite == NULL)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionAck - Failed for Call-leg 0x%p, did not find invite object by state. (forked call-leg)"
                    ,pCallLeg));
                return RV_ERROR_UNKNOWN;
            }
        }
        else
        {
            /* working with old invite. the invite transaction is still the active transaction */
            rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pFoundInvite);
            if (RV_OK != rv || pFoundInvite == NULL)
            {
                RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionAck - Failed for Call-leg 0x%p, did not find invite object by activeTransc 0x%p"
                    ,pCallLeg, pCallLeg->hActiveTransc));
                return RV_ERROR_UNKNOWN;
            }
            /* check that the invite object is in an ACK state... */
            if(pFoundInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED &&
               pFoundInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED /* initial invite*/)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionAck - Failed for Call-leg 0x%p, illegal re-invite state %s (old invite obj 0x%p)"
                    ,pCallLeg, CallLegGetModifyStateName(pFoundInvite->eModifyState), pFoundInvite));
                return RV_ERROR_ILLEGAL_ACTION;
            }
        }
        
    }
    else /* new invite handling */
    { 
       /* this is ACK on initial INVITE */
        if(pInvite == NULL)
        {
            if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED) 
            {
                /* working with new invite. invite transaction is no longer the active transaction.
                the initial invite is the only invite object that has UNDEFINED modify state.*/     
                rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED, &pFoundInvite);
                if (RV_OK != rv || pFoundInvite == NULL)
                {
                    RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegSessionAck - Failed for Call-leg 0x%p, did not find initial invite object"
                        ,pCallLeg));
                    return RV_ERROR_UNKNOWN;
                }
            }
            else
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionAck - Failed for Call-leg 0x%p, no invite object. call state = %s"
                    ,pCallLeg, CallLegGetStateName(pCallLeg->eState)));
                return RV_ERROR_ILLEGAL_ACTION;
            }
        }
        else/* this is ACK on re-INVITE */
        {
            pFoundInvite = pInvite;
        }
    }
    /* 2. set outbound message */
    if (NULL != pCallLeg->hOutboundMsg)
    {
        if(pCallLeg->pMgr->bOldInviteHandling == RV_FALSE ||
            bAckOnForked2xx == RV_TRUE)
        {
            /* no transaction at this point */
            pFoundInvite->hAckMsg = pCallLeg->hOutboundMsg;
        }
        else
        {
            /* old invite - invite transaction exists, and send the ACK */
            rv = SipTransactionSetOutboundMsg(pCallLeg->hActiveTransc, pCallLeg->hOutboundMsg);
            if (RV_OK != rv)
            {
                RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionAck - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                    ,pCallLeg,pCallLeg->hActiveTransc,rv));
                return RV_ERROR_UNKNOWN;
            }
        } 
        pCallLeg->hOutboundMsg = NULL;
    }

    /* 3. Send the ACK */
    rv = CallLegInviteSendAck(pCallLeg, pFoundInvite->hInviteTransc, pFoundInvite, 200);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionAck - Failed for call-leg 0x%p - disconnecting with no bye",pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }
    /* Patch (O.W) - in old invite we must reset the transaction here, so application
       will be able to send a re-invite within the callback */
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        CallLegResetActiveTransc(pCallLeg);
    }
    if(pInvite == NULL && pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_CONNECTED,
                                    RVSIP_CALL_LEG_REASON_ACK_SENT);
        CallLegChangeModifyState(pCallLeg,NULL,
                                RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
                                RVSIP_CALL_LEG_REASON_CALL_CONNECTED);
    }
    else /*this is ack on a response to re-Invite*/
    {
        CallLegChangeModifyState(pCallLeg, pFoundInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
                                 RVSIP_CALL_LEG_REASON_ACK_SENT);
    }
    return RV_OK;
}


/***************************************************************************
 * CallLegSeesionCreateAndModify
 * ------------------------------------------------------------------------
 * General: The function is deprecated. 
 *          You should use RvSipCallLegCreateReInvite() and RvSipCallLegReInvite().
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSeesionCreateAndModify (IN  CallLeg* pCallLeg)
{
    RvStatus        rv;
    CallLegInvite*  pInvite = NULL;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSeesionCreateAndModify - call-leg 0x%p: starting the function. (hActiveTransc=0x%p)",
               pCallLeg, pCallLeg->hActiveTransc));

    /*modify can be called only when the call is connected and no modify or
      modifying is taking place*/
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegSeesionCreateAndModify - Failed to Modify call-leg 0x%p: Illegal Action in call state %s",
               pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_FALSE &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_ACCEPTED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSeesionCreateAndModify - Failed to Modify call-leg 0x%p: Illegal Action in call state %s",
            pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pCallLeg->hActiveTransc != NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSeesionCreateAndModify - Failed to Modify call-leg 0x%p: active transc 0x%p exist",
            pCallLeg, pCallLeg->hActiveTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    /* create a new invite object */
    rv = CallLegInviteCreateObj(pCallLeg, NULL, RV_TRUE, RVSIP_CALL_LEG_DIRECTION_OUTGOING, &pInvite, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSeesionCreateAndModify - call-leg 0x%p: Failed to create an invite object",
            pCallLeg));
        return rv;
    }
    
    /* Send the re-invite */
    rv = CallLegSessionModify(pCallLeg, pInvite);
    return rv;

}


/***************************************************************************
 * CallLegSessionModify
 * ------------------------------------------------------------------------
 * General: Causes a re-INVITE to be sent. Can be called only in the connected
 *          state when there is no other pending re-Invite transaction.
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
RvStatus RVCALLCONV CallLegSessionModify (IN  CallLeg   *pCallLeg,
                                          IN  CallLegInvite* pInvite)
{
    RvSipTranscHandle hReInvTransc;

    /*-----------------------
     create a new transaction
    -------------------------*/
    RvStatus rv = CallLegCreateTransaction(pCallLeg, RV_FALSE, &hReInvTransc);
    if (rv != RV_OK || NULL == hReInvTransc)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSessionModify - Call-leg 0x%p - Failed to create transaction (rv=%d)",
                  pCallLeg,rv));
        return rv;
    }

    CallLegInviteSetParams(pCallLeg, pInvite, hReInvTransc);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /* notify application that a new reInvite callLeg has just created and it's in IDLE state.
       Application can add in any change as it wants to */
    CallLegChangeModifyState(pCallLeg, pInvite, RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
                              RVSIP_CALL_LEG_REASON_LOCAL_INVITING);
#endif
/* SPIRENT_END */

    /*----------------------------------
         send the re-INVITE request
     -----------------------------------*/
    rv = CallLegSendRequest(pCallLeg,hReInvTransc,SIP_TRANSACTION_METHOD_INVITE,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionModify - Call-leg 0x%p - Failed to send re-Invite request (rv=%d) - Terminating call"
            ,pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    CallLegSetActiveTransc(pCallLeg, hReInvTransc);
    CallLegChangeModifyState(pCallLeg, pInvite, RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT,
                                       RVSIP_CALL_LEG_REASON_LOCAL_INVITING);
    return RV_OK;
}

/***************************************************************************
 * CallLegSessionDisconnect
 * ------------------------------------------------------------------------
 * General:Disconnect causes the call to disconnect. Disconnect may
 *         be called at any state. The behavior of the function depends on
 *         the call-leg state:
 *         Connected,Accepted,Inviting - Bye is send and the call moves
 *                                         the Disconnecting.
 *         Offering  - The incoming invite is rejected with status code 403.
 *         Idle,Disconnecting,Disconnected - The call is terminated.
 *         If the functions failes to send the BYE request the call-leg will
 *         be terminated.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send bye message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionDisconnect(IN  CallLeg   *pCallLeg)
{
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionDisconnect - Disconnecting call-leg 0x%p - in state %s",
              pCallLeg,CallLegGetStateName(pCallLeg->eState)));

    switch(pCallLeg->eState)
    {

    /*if the state is idle or the application wants to terminate a call that already
      sent a bye request and is waiting for ok there is no need to send bye*/
    case RVSIP_CALL_LEG_STATE_TERMINATED:
        {
           return RV_OK;
        }
    case RVSIP_CALL_LEG_STATE_IDLE:
    case RVSIP_CALL_LEG_STATE_DISCONNECTED:
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        /* In case DISCONNECTING - the BYE was already sent, but the final response was not received yet.
           in this case we will detach from the bye transaction first, in order to
           continue with the BYE retransmissions */
           CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_DETACH_BYE, NULL);
           /* continue to the next case. no break */
    case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE:
    case RVSIP_CALL_LEG_STATE_REDIRECTED:
    case RVSIP_CALL_LEG_STATE_UNAUTHENTICATED:
    case RVSIP_CALL_LEG_STATE_INVITING:
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_DISCONNECTED);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    /*at offering decline the call*/
    case RVSIP_CALL_LEG_STATE_OFFERING:
        return CallLegSessionReject(pCallLeg,403);
    /*if we just sent the invite we need to cancel it.*/
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT:
        if(pCallLeg->hActiveTransc != NULL)
        {
            return CallLegSessionCancel(pCallLeg);
        }
        else if(RV_FALSE == pCallLeg->bOriginalCall)
        {
            /* this is a forked call, and has no transaction to cancel.
               terminate call instead... */
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegSessionDisconnect - Call-leg 0x%p - forked call. terminating call, with no cancel.",
               pCallLeg));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_DISCONNECTED);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_OK;
        }
        else
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionDisconnect - Call-leg 0x%p - no active transaction in the %s state",
                       pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        }
        break;
    case RVSIP_CALL_LEG_STATE_CANCELLED:
        if(pCallLeg->hActiveTransc != NULL)
        {
            return CallLegSessionReject(pCallLeg, 487);
        }
        else
        {
             RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionDisconnect - Disconnecting call-leg 0x%p failed. active transaction is NULL",
                      pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    default:
        break;
    }

    return CallLegSessionSendBye(pCallLeg);
}



/***************************************************************************
 * CallLegSessionSendBye
 * ------------------------------------------------------------------------
 * General:Bye is send and the call moves the Disconnecting state.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send bye message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionSendBye(IN  CallLeg   *pCallLeg)
{

    RvSipTranscHandle hBye;
    RvStatus         rv;

    /*detach from invite active transaction on state Ack sent*/
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE
        && pCallLeg->hActiveTransc != NULL)
    {
        RvSipTransactionState eState;
        rv = RvSipTransactionGetCurrentState(pCallLeg->hActiveTransc,&eState);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionSendBye - Getting current state of call-leg 0x%p failed. ",
                      pCallLeg));
            return rv;
        }
        if(eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT &&
            SipTransactionWasAckSent(pCallLeg->hActiveTransc) == RV_FALSE)
        {
            CallLegInvite *pInvite;
            rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
            if(rv != RV_OK || NULL == pInvite)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionSendBye - call-leg 0x%p failed to get invite obj from active transc 0x%p. ",
                    pCallLeg, pCallLeg->hActiveTransc));
                return RV_ERROR_UNKNOWN;
            }
            CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pCallLeg->hActiveTransc);
        }
#ifdef STOP_RETRANSMISSIONS_PATCH  
    /* old invite: stop initial invite retransmission of 2xx */
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_ACCEPTED)
        {
            /* terminate the transaction to stop the retransmissions */
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegSessionSendBye - call-leg 0x%p terminate active transc 0x%p to stop the 2xx retransmissions. ",
                    pCallLeg, pCallLeg->hActiveTransc));
            RvSipTransactionTerminate(pCallLeg->hActiveTransc);
        }

        /* stop re-invite retransmission of 2xx  */
        {
            RvUint16 statusCode;
            SipTransactionGetResponseCode(pCallLeg->hActiveTransc, &statusCode);
            if(eState == RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT &&
               statusCode >= 200 && statusCode < 300)
            {
                RvSipTransactionTerminate(pCallLeg->hActiveTransc);
            }
        }
#endif
    } /* old invite */

    CallLegResetActiveTransc(pCallLeg);

#ifdef STOP_RETRANSMISSIONS_PATCH  

	/*-------------------------------------------
	  stop re-invite (request and response) 
	  retransmissions if exist
	  ------------------------------------------- */
	{
		CallLegInvite *pInvite = NULL;
        rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT, &pInvite);
		if(pInvite == NULL)
		{
			rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT, &pInvite);
		}
        if(NULL != pInvite)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionSendBye - call-leg 0x%p stop re-invite retransmissions for pInvite 0x%p. ",
                pCallLeg, pInvite));
			CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_DISCONNECTED);
        }
        
	}
    /*-------------------------------------------
	  stop general transactions (request and response) 
	  retransmissions if exist
	  ------------------------------------------- */
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegSessionSendBye - Call-leg 0x%p - stop all pending general transactions." ,pCallLeg));

   CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_STOP_GENERAL_RETRANS, NULL);
#endif /*#ifdef STOP_RETRANSMISSIONS_PATCH */

    /*-------------------------------------------
     create a new transaction for the bye request
    ---------------------------------------------*/
    rv = CallLegCreateTransaction(pCallLeg, RV_TRUE, &hBye);
    if (rv != RV_OK || NULL == hBye)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSessionSendBye - Call-leg 0x%p - Failed to create new transaction (rv=%d) - - Terminating call",
                  pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /*----------------------------------
         send the BYE request
     -----------------------------------*/
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionSendBye - Call-leg 0x%p sending BYE",pCallLeg));

    rv = CallLegSendRequest(pCallLeg,hBye,SIP_TRANSACTION_METHOD_BYE,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionSendBye - Call-leg 0x%p - Failed to send Bye request (rv=%d) - Terminating call",
            pCallLeg,rv));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }
    /*set the bye to be the active transaction*/
    CallLegSetActiveTransc(pCallLeg, hBye);
    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTING,
                       RVSIP_CALL_LEG_REASON_LOCAL_DISCONNECTING);

    return RV_OK;

}

/***************************************************************************
 * CallLegSessionRequest
 * ------------------------------------------------------------------------
 * General:Sends a general reqest related to this call-leg.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send request message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - request message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 *          strMethod - The request method
 *          hTransc - The transaction handle to use for the request. if NULL
 *                     create a new transaction.
 * Output:  hTransc - The handle of the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionRequest(
                                           IN  CallLeg*             pCallLeg,
                                           IN  RvChar*             strMethod,
                                           OUT RvSipTranscHandle    *hTransc)
{
    RvStatus rv;
    RvBool   bNewTransc = RV_FALSE; /*indicates whether a transaction was supplied to this
                                       function*/
    /*----------------------------------
     create a new transaction if needed
    -----------------------------------*/
    if(*hTransc == NULL)
    {
        rv = CallLegCreateTransaction(pCallLeg, RV_TRUE, hTransc);

        if (rv != RV_OK || NULL == *hTransc)
        {
            /*transc construction failed*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSessionRequest - Failed for Call-leg 0x%p - failed to create transaction",pCallLeg));
            *hTransc = NULL;
            return rv;
        }
        bNewTransc = RV_TRUE;
    }
    /*set the application as the initiator of the transaction*/
    SipTransactionSetAppInitiator(*hTransc);
     /*----------------------------------
         send the Request
     -----------------------------------*/
    rv = CallLegSendRequest(pCallLeg,*hTransc,SIP_TRANSACTION_METHOD_OTHER,strMethod);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSessionRequest - Failed for Call-leg 0x%p  - failed to send %s message with transc 0x%p",
                  pCallLeg,strMethod,*hTransc));

        if(bNewTransc == RV_TRUE)
        {
            CallLegRemoveTranscFromList(pCallLeg,*hTransc);
            SipTransactionTerminate(*hTransc);
            *hTransc = NULL;
        }
        return rv;
    }
    return RV_OK;
}
/***************************************************************************
 * CallLegSessionRequestRefresh
 * ------------------------------------------------------------------------
 * General:Creates a transaction related to the call-leg and sends a
 *          Request message with the given method in order to refresh the call.
 *          The request will have the To, From and Call-ID of the call-leg and
 *          will be sent with a correct CSeq step. It will be record routed if needed.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send cancel message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - CANCEL message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 *          sessionExpires - session time that will attach to this call.
 *          minSE - minimum session expires time of this call
 *          eRefresher - the refresher preference for this call
 * Output:  hTransc - The handle to the newly created transaction.
 *                    if a transaction was supplied, this transaction will be used,
 *                    and a new transaction will not be created
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionRequestRefresh(
    IN  CallLeg*                                      pCallLeg,
    IN  RvChar*                                       strMethod,
    IN  RvInt32                                       sessionExpires,
    IN  RvInt32                                       minSE,
    IN  RvSipCallLegSessionTimerRefresherPreference   eRefresher,
    INOUT RvSipTranscHandle                          *hTransc)
{
    RvStatus rv = RV_OK;
    CallLegNegotiationSessionTimer      *info = NULL;

    /* if the Application did not create a transaction for us, we will create a transaction */
    if (NULL == *hTransc)
    {
        rv = CallLegCreateTransaction(pCallLeg, RV_TRUE, hTransc);
    }

    if (rv != RV_OK || NULL == *hTransc)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionRequestRefresh - Failed for Call-leg 0x%p - failed to create transaction",pCallLeg));
        *hTransc = NULL;
        return rv;
    }
    /* Get the session timer params handle from the transaction */
    info = (CallLegNegotiationSessionTimer *)SipTransactionGetSessionTimerMemory(*hTransc);
    /* Allocate the session timer handle in the transaction*/
    if(info == NULL)
    {
        info = (CallLegNegotiationSessionTimer *)SipTransactionAllocateSessionTimerMemory(
            *hTransc, sizeof(CallLegNegotiationSessionTimer));
        /* Initialize the structure in the default values from the call mgr*/
        CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr,RV_TRUE,info);
    }
    info->bInRefresh =RV_TRUE;

    if (sessionExpires!= UNDEFINED)
    {
        info->sessionExpires = sessionExpires;
    }
    if (minSE != UNDEFINED)
    {
        info->minSE = minSE;
    }
    if (eRefresher != RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_NONE)
    {
        info->eRefresherPreference = eRefresher;
    }
    /*set the application as the initiator of the transaction*/
    SipTransactionSetAppInitiator(*hTransc);
     /*----------------------------------
         send the Request
     -----------------------------------*/
    rv = CallLegSendRequest(pCallLeg,*hTransc,SIP_TRANSACTION_METHOD_OTHER,strMethod);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionRequestRefresh - Failed for Call-leg 0x%p  - failed to send %s message with transc 0x%p - Terminating call",
            pCallLeg,strMethod,*hTransc));
        CallLegRemoveTranscFromList(pCallLeg,*hTransc);
        SipTransactionTerminate(*hTransc);
        hTransc = NULL;
        return rv;
    }
    return RV_OK;
}
/***************************************************************************
 * CallLegSessionCancel
 * ------------------------------------------------------------------------
 * General:Cancels an Invite active transaction.
 * Return Value: RV_ERROR_UNKNOWN - Function failed. Can't send cancel message.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK - CANCEL message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to disconnect.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionCancel(IN  CallLeg   *pCallLeg)
{

    RvStatus rv;
    RvSipTranscHandle hCancelTransc = NULL;
    CallLegInvite*  pInvite = NULL;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionCancel - Call-leg 0x%p  - cancelling invite transc 0x%p",
               pCallLeg,pCallLeg->hActiveTransc));

    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionCancel - Call-leg 0x%p - did not find invite object.",pCallLeg));
        return rv;
    }
    
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(pCallLeg->hActiveTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSessionCancel - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,pCallLeg->hActiveTransc,rv));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }
    rv = RvSipTransactionCancel(pCallLeg->hActiveTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegSessionCancel - Failed for Call-leg 0x%p  - failed to send cancel on transc 0x%p - Terminating call",
               pCallLeg,pCallLeg->hActiveTransc));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /*get the handle of the cancel transaction*/
    rv = RvSipTransactionGetCancelTransc(pCallLeg->hActiveTransc,&hCancelTransc);
    if(rv != RV_OK || hCancelTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionCancel - Failed for Call-leg 0x%p  - failed to get cancel transc - Terminating call",
            pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /*add the transaction handle to the transaction list*/
    rv = CallLegAddTranscToList(pCallLeg,hCancelTransc);
    if(rv != RV_OK)
    {
        SipTransactionTerminate(hCancelTransc);
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSessionCancel - Failed for Call-leg 0x%p  - failed to insert cancel transc to list- Terminating call",
            pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return rv;
    }

    /* checking if initial invite or re-invite to know which call back to call */
    if(pInvite->bInitialInvite == RV_TRUE)
    {
        /* this is cancel on initial invite */
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_CANCELLING,
                                    RVSIP_CALL_LEG_REASON_LOCAL_CANCELLING);
    }
    else
    {
        /* this is cancel on  a re-invite */
        CallLegChangeModifyState(pCallLeg, pInvite,
                                RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING,
                                RVSIP_CALL_LEG_REASON_LOCAL_CANCELLING);
    }
    return RV_OK;
}

/***************************************************************************
 * AddPrackInfoToMsg
 * ------------------------------------------------------------------------
 * General: Adds information to the PRACK message. The RAck header parameters
 *          were saved in the call-leg when the 1xx response was received.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *          pCallLeg - The call-leg handle.
 *          hMsg - the handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddPrackInfoToMsg(
                                    IN  CallLeg        *pCallLeg,
                                    IN  RvSipMsgHandle  hMsg)
{

    RvStatus               rv;
    RvSipRAckHeaderHandle  hRAck;
    RvSipCSeqHeaderHandle  hCSeq;
	RvInt32				   cseqVal;	
    RvSipOtherHeaderHandle hTimeStamp;

    /*set the to and from headers*/

    /*copy a timestamp header from the provisional response to the new
      transaction if exists*/
    rv = RvSipRAckHeaderConstructInMsg(hMsg,RV_FALSE,&hRAck);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddPrackInfoToMsg - pCallLeg 0x%p - Failed to add RAck to PRACK msg",
                   pCallLeg));
        return rv;
    }

	if (pCallLeg->incomingRSeq.bInitialized == RV_FALSE)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"AddPrackInfoToMsg - pCallLeg 0x%p - incoming RSeq is NOT initialized",pCallLeg));
        return RV_ERROR_UNINITIALIZED;
	}

    rv = RvSipRAckHeaderSetResponseNum(hRAck,pCallLeg->incomingRSeq.val);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddPrackInfoToMsg - pCallLeg 0x%p - Failed to set response num in RAck of PRACK msg",
                   pCallLeg));
        return rv;
    }
    rv = RvSipCSeqHeaderConstructInRAckHeader(hRAck,&hCSeq);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddPrackInfoToMsg - pCallLeg 0x%p, Failed to construct CSeq in RAck of PRACK msg",
                   pCallLeg));
        return rv;
    }

	rv = SipCommonCSeqGetStep(&pCallLeg->incomingRel1xxCseq,&cseqVal);
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"AddPrackInfoToMsg - pCallLeg 0x%p, failed to get incoming rel cseq (might be undefined)",
			pCallLeg));
        return RV_ERROR_UNKNOWN;
	}

    rv = RvSipCSeqHeaderSetStep(hCSeq, cseqVal);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddPrackInfoToMsg - pCallLeg 0x%p, Failed to set step in CSeq inside RAck of PRACK msg",
                   pCallLeg));
        return rv;
    }
    rv = RvSipCSeqHeaderSetMethodType(
                        hCSeq,RVSIP_METHOD_INVITE,
                        NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "AddPrackInfoToMsg - pCallLeg 0x%p, Failed to set method in CSeq inside RAck of PRACK msg",
                   pCallLeg));

        return rv;
    }
    /*copy the time stamp from the call-leg to the message*/
    if(pCallLeg->hTimeStamp != NULL)
    {
        rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_FALSE,&hTimeStamp);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AddPrackInfoToMsg - pCallLeg 0x%p, failed construct time stamp header",
                pCallLeg));
            return rv;
        }
        rv = RvSipOtherHeaderCopy(hTimeStamp,pCallLeg->hTimeStamp);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AddPrackInfoToMsg - pCallLeg 0x%p, failed to copy Time Stamp header",
                pCallLeg));
            return rv;
        }
    }

    return RV_OK;
}


#endif /*RV_SIP_PRIMITIVES */

