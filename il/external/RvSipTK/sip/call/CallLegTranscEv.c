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
 *                              <sipCallLegTranscEv.h>
 *
 *  Handles state changed events received from the transaction layer.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegTranscEv.h"
#include "CallLegObject.h"
#include "CallLegSession.h"
#include "CallLegRouteList.h"
#include "CallLegCallbacks.h"
#include "CallLegMsgEv.h"
#include "CallLegAuth.h"
#include "CallLegRefer.h"
#include "CallLegReplaces.h"
#include "CallLegSubs.h"
#include "RvSipCallLegTypes.h"
#include "_SipCallLeg.h"
#include "CallLegSessionTimer.h"
#include "RvSipCallLeg.h"
#include "CallLegForking.h"
#include "CallLegInvite.h"
#ifdef RV_SIP_IMS_ON
#include "CallLegSecAgree.h"
#endif /* RV_SIP_IMS_ON */

#include "RvSipHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipPartyHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipMsg.h"
#include "RvSipOtherHeader.h"
#include "_SipOtherHeader.h"
#include "RvSipRSeqHeader.h"
#include "_SipSubs.h"
#include "RvSipCommonList.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"
#include "_SipTransactionMgr.h"
#include "rvtimestamp.h"
#include "_SipCommonCore.h"
#include "_SipTransmitter.h"
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV RejectInvalidRequests(
                            IN  CallLeg               *pCallLeg,
                            IN    RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState,
                            IN  RvBool                *bWasRejected);
/*------------------------------------------------------------------------
        I N V I T E   R E Q U E S T   R E C E I V E D
 -------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleTranscStateInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN    RvSipTranscHandle     hTransc);


static RvStatus RVCALLCONV HandleInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc);


static RvStatus RVCALLCONV HandleReInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc);

static RvStatus RVCALLCONV HandleTranscStateInviteProceedingTimeOut(
                            IN  CallLeg        *pCallLeg,
                            IN  CallLegInvite*  pInvite);


static RvStatus RVCALLCONV HandleInvite2xxResponse(
						    IN  CallLeg                          *pCallLeg,
						    IN  CallLegInvite                    *pInvite,
						    IN  RvSipTranscHandle                 hTransc,
						    IN  RvUint16                          statusCode,
						    IN  RvSipTransactionStateChangeReason eStateReason);


/*------------------------------------------------------------------------
        G E N E R A L   R E Q U E S T   R E C E I V E D
 -------------------------------------------------------------------------*/
static RvStatus RVCALLCONV HandleTranscStateGeneralRequestRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc,
                               IN  RvSipTransactionStateChangeReason eStateReason);


static RvStatus RVCALLCONV HandleByeRequestRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc);

static void RVCALLCONV GetRejectRespCodeForGeneralRcvdReq(
                                    IN  CallLeg              *pCallLeg,
                                    IN  SipTransactionMethod  eMethod,
                                    OUT RvInt32              *pRespCode);

/*------------------------------------------------------------------------
      G E N E R A L    F I N A L   R E S P O N S E    S E N T
 -------------------------------------------------------------------------*/
static RvStatus RVCALLCONV HandleTranscStateGeneralFinalResponseSent(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc,
                               IN  RvSipTransactionStateChangeReason eStateReason);

/*------------------------------------------------------------------------
      I N V I T E    F I N A L   R E S P O N S E    R E C V I E V E D
 -------------------------------------------------------------------------*/
static RvStatus RVCALLCONV HandleInviteFinalResponse(
                            IN  CallLeg                         *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason,
                            IN  RvUint16                          statusCode);

static RvStatus RVCALLCONV HandleReInviteFinalResponse(
                            IN  CallLeg                          *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason);

static RvStatus RVCALLCONV HandleInviteFinalResponseInCancellingState(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc);

static RvStatus RVCALLCONV HandleInvite2xxRetransmission(
                         IN  CallLeg        *pCallLeg,
                         IN  CallLegInvite*   pInvite,
                         IN  RvSipMsgHandle  hMsg,
                         IN  RvUint16        statusCode);

static RvSipCallLegStateChangeReason RVCALLCONV GetCallLegRemoteResponseReason(
                                           IN  RvSipTransactionStateChangeReason eTranscReason);

static RvStatus RVCALLCONV UpdateReceivedMsg(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState);

static void RVCALLCONV ResetReceivedMsg(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTransactionState  eTranscState);

/*------------------------------------------------------------------------
      I N V I T E    F I N A L   R E S P O N S E    S E N T
 -------------------------------------------------------------------------*/
static RvStatus RVCALLCONV HandleTranscStateInviteFinalResponseSent(
                            IN  CallLeg                           *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason);

/*------------------------------------------------------------------------
        G E N E R A L   R E S P O N S E   R E C E I V E D
 -------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleTranscStateGeneralFinalResponseRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc,
                               IN  RvSipTransactionStateChangeReason eStateReason);

static RvStatus RVCALLCONV HandleTranscStateCancelFinalResponseRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc);

static RvStatus RVCALLCONV HandleByeResponseRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc);


/*------------------------------------------------------------------------
        M S G   S E N D   F A I L U R E
-------------------------------------------------------------------------*/

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus RVCALLCONV HandleTranscStateMsgSendFailure(
                                IN  CallLeg                         *pCallLeg,
                                IN  RvSipTranscHandle                 hTransc,
                                IN  CallLegInvite                    *pInvite,
                                IN  SipTransactionMethod              eMethod,
                                IN  RvSipTransactionStateChangeReason eStateReason);

static void RVCALLCONV HandleInviteMsgSendFailure(
                              IN  CallLeg                      *pCallLeg,
                              IN  RvSipTranscHandle             hTransc,
                              IN  CallLegInvite                *pInvite,
                              IN  RvSipCallLegStateChangeReason eStateReason);

static void RVCALLCONV HandleByeMsgSendFailure(
                               IN  CallLeg                       *pCallLeg,
                               IN    RvSipTranscHandle             hTransc,
                               IN  RvSipCallLegStateChangeReason eCallReason);

#endif

/*-------------------------------------------------------------------------
      T R A N S A C T I O N     T E R M I N A T E D
 --------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleTranscStateTerminated(
                            IN  CallLeg                          *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason);

static RvStatus RVCALLCONV HandleAppTranscStateTerminated(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc,
                               IN  RvSipTransactionStateChangeReason eStateReason);


static RvStatus RVCALLCONV CopyTimestampFromMsg(IN CallLeg   *pCallLeg,
                                     IN RvSipMsgHandle hMsg);

static void RVCALLCONV ResetIncomingRel1xxParamsOnFinalResponse(CallLeg* pCallLeg);

static RvStatus RVCALLCONV HandleTranscInviteAckRcvd(
                            IN  CallLeg                 *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc);

static RvStatus RVCALLCONV HandleAckNoTranscRequest(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipMsgHandle         hAckMsg,
							IN   SipTransportAddress  *pRcvdFromAddr);

static RvStatus RVCALLCONV HandleTranscTimeOut(
                            IN  CallLeg                 *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN    RvSipTranscHandle       hTransc);

static RvStatus RVCALLCONV HandleTranscRanOutOfTimers(
                            IN  CallLeg               *pCallLeg,
                            IN    RvSipTranscHandle     hTransc);

static RvStatus RVCALLCONV HandleTranscErrorTermination(
                            IN  CallLeg               *pCallLeg,
                            IN    RvSipTranscHandle       hTransc);

static RvStatus RVCALLCONV GetCseqStepFromMsg(IN  CallLeg        *pCallLeg,
											  IN  RvSipMsgHandle  hMsg,
											  OUT RvInt32        *pCSeqStep);

static RvStatus RVCALLCONV HandleInviteResponseNoTransc(
                                                 IN CallLeg*				pCallLeg,
                                                 IN RvSipMsgHandle			hReceivedMsg,
                                                 IN SipTransportAddress*    pRcvdFromAddr,
												 IN RvInt32					curIdentifier,
                                                 IN RvInt16					responseCode);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegTranscEvCancelled
 * ------------------------------------------------------------------------
 * General: Called when a cancel request is received on a transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc:      - A pointer to new transaction.
 *          hTranscOwner: - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvCancelled(
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg)
{
    CallLeg              *pCallLeg =  (CallLeg*)hCallLeg;
    SipTransactionMethod  eMethod;
    RvStatus              rv      = RV_OK;
    
    SipTransactionGetMethod(hTransc,&eMethod);

    /* if this is a cancel on invite call Change State. it it is cancel on
       re-invite, call Change Modify State */
    if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
    {
        CallLegInvite* pInvite = NULL;

        rv = CallLegInviteFindObjByTransc(pCallLeg,hTransc,&pInvite);
        if(rv != RV_OK || pInvite == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscEvCancelled - Call-leg 0x%p - did not find invite object.",pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    
        if(RV_TRUE == pInvite->bInitialInvite)
        {
            /* This is an initial invite */
            CallLegChangeState(pCallLeg,
                               RVSIP_CALL_LEG_STATE_CANCELLED,
                               RVSIP_CALL_LEG_REASON_REMOTE_CANCELED);
        }
        else
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                                     RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED,
                                     RVSIP_CALL_LEG_REASON_REMOTE_CANCELED);
        }
    }

    return rv;
}



/***************************************************************************
 * CallLegTranscEvRelProvRespRcvdEv
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvRelProvRespRcvdEv (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    RvUint32                 rseqResponseNum)
{
    CallLeg        *pCallLeg =  (CallLeg*)hCallLeg;
    RvSipMsgHandle  hMsg     = NULL;
    RvStatus        rv;

    RV_UNUSED_ARG(hTransc);

    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK || NULL == hMsg)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvRelProvRespRcvdEv: call-leg 0x%p - Failed to get received msg from transc 0x%p (rv=%d)",
            pCallLeg, hTransc, rv));
        return rv;
    }
	pCallLeg->hReceivedMsg = hMsg;
    rv = HandleRelProvRespRcvd(pCallLeg, rseqResponseNum, hMsg);
	pCallLeg->hReceivedMsg = NULL;
    return rv;
}



/***************************************************************************
 * CallLegTranscEvIgnoreRelProvRespEvHandler
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvIgnoreRelProvRespEvHandler (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    RvUint32                 rseqStep,
                      OUT   RvBool                  *pbIgnore)
{
    CallLeg   *pCallLeg =  (CallLeg*)hCallLeg;

    RV_UNUSED_ARG(hTransc);

    /* check the RSeq value -
       if bigger that RSeq saved in the call-leg - save parameters from msg
       in call-leg, for later PRACK sending, and change state in PRACK state machine.
       else - this is a retransmission of the 1xx. ignore it */
    if(pCallLeg->incomingRSeq.bInitialized == RV_FALSE ||
	   pCallLeg->incomingRSeq.val		   < rseqStep)
    {
        *pbIgnore = RV_FALSE;
    }
    else
    {
        *pbIgnore = RV_TRUE;
    }
}

/***************************************************************************
 * CallLegTranscEvInviteTranscMsgSent
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that the invite transaction sent a msg.
 *          1. On first 2xx sending, when working with new-invite handling. 
 *          2. On non-2xx Ack sending, when working with new-invite handling.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvInviteTranscMsgSent (
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      IN    SipTransactionMsgSentOption eOption)
{
    CallLeg        *pCallLeg =  (CallLeg*)hCallLeg;
    RvStatus        rv;
    CallLegInvite*  pInvite;
    
    rv = CallLegInviteFindObjByTransc(pCallLeg, hTransc, &pInvite);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvInviteTranscMsgSent - Call-Leg 0x%p Failed to find invite object for transc 0x%p"
            ,pCallLeg, hTransc));
        return rv;
    }
    
    if(SIP_TRANSACTION_MSG_SENT_OPTION_2XX == eOption)
    {
        
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvInviteTranscMsgSent - Call-Leg 0x%p invite 0x%p - 2xx was sent"
            ,pCallLeg, pInvite));
        
        /* take the transmitter of the invite transaction, and save it in the invite object */
        rv = SipTransactionGetTrxControl(hTransc, 
                                    (RvSipAppTransmitterHandle)pInvite,
                                    &pCallLeg->pMgr->trxEvHandlers, 
                                    sizeof(pCallLeg->pMgr->trxEvHandlers),
                                    &pInvite->hTrx);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscEvInviteTranscMsgSent - Call-Leg 0x%p, invite 0x%p: Failed to get control over trx"
                ,pCallLeg, pInvite));
            pInvite->hTrx = NULL;
            return rv;
        }
		
        /* Set the 2xx timer of the invite object */
        rv = CallLegInviteSet2xxTimer(pCallLeg, pInvite, RV_TRUE);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscEvInviteTranscMsgSent - Call-Leg 0x%p, invite 0x%p: Failed to set 2xx timer"
                ,pCallLeg,pInvite));
            CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            return rv;
        }
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvInviteTranscMsgSent - Call-Leg 0x%p invite 0x%p - ack was sent"
            ,pCallLeg, pInvite));
        
    }
    
    if(hTransc == pCallLeg->hActiveTransc)
    {
        CallLegResetActiveTransc(pCallLeg);
    }
    return RV_OK;
    
}
/***************************************************************************
 * CallLegTranscEvSupplyTranscParamsEv
 * ------------------------------------------------------------------------
 * General: Supply the transaction with the next Call-Leg CSeq step
 *          and the request uri calculated according to contact and
 *          Route list.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The new sip stack transaction handle
 *          hAppTransc  - Application handle for the transaction
 * Output:  pCSeqStep - CSeq step for the transaction
 *          phRequestUri - Request URI for the transaction.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvSupplyTranscParamsEv(
                      IN    RvSipTranscHandle        hTransc,
                      IN    RvSipTranscOwnerHandle   hCallLeg,
                      OUT   RvInt32                 *pCSeqStep,
                      OUT   RvSipAddressHandle      *phRequestUri)
{
	RvStatus   rv; 
    CallLeg   *pCallLeg =  (CallLeg*)hCallLeg;
    

	rv = SipCommonCSeqPromoteStep(&pCallLeg->localCSeq);
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegTranscEvSupplyTranscParamsEv - Call-leg=0x%p,transc=0x%p failed in SipCommonCSeqPromoteStep(rv=%d)",
			pCallLeg,hTransc,rv));
		return;
	}
	
	rv = SipCommonCSeqGetStep(&pCallLeg->localCSeq,pCSeqStep); 
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegTranscEvSupplyTranscParamsEv - Call-leg=0x%p,transc=0x%p failed to retrieve a cseq (rv=%d)",
			pCallLeg,hTransc,rv));
		return;
	}
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegTranscEvSupplyTranscParamsEv - Call-leg=0x%p,transc=0x%p supplying cseq=%d(%u)",
              pCallLeg,hTransc,*pCSeqStep,*pCSeqStep));
    CallLegGetRequestURI(pCallLeg, hTransc, phRequestUri, NULL);
}

/***************************************************************************
 * CallLegTranscEvStateChangedHandler
 * ------------------------------------------------------------------------
 * General: Handle a transaction state change.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg this transaction belongs to.
 *          hTransc - Handle to the transaction whose state was changed.
 *            eTranscState - The new state of the transaction
 *            eStateReason - The reason for the change of state.
 ***************************************************************************/
void RVCALLCONV  CallLegTranscEvStateChangedHandler(
                        IN  RvSipTranscHandle                 hTransc,
                        IN  RvSipTranscOwnerHandle            hCallLeg,
                        IN  RvSipTransactionState             eTranscState,
                        IN  RvSipTransactionStateChangeReason eStateReason)
{

    RvStatus  rv                 = RV_OK;
    CallLeg  *pCallLeg           = (CallLeg*)hCallLeg;
    RvBool    b_wasRejected      = RV_FALSE;
    RvBool    b_wasHandledBySubs = RV_FALSE;
    void     *pSubsInfo          = NULL;
    CallLegInvite *pInvite       = NULL;
    SipTransactionMethod eMethod;

    SipTransactionGetMethod(hTransc,&eMethod);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegTranscEvStateChangedHandler - Transaction state was changed (call-leg=0x%p hTransc=0x%p state=%s. reason=%d)",
              pCallLeg,hTransc,
              SipTransactionGetStateName(eTranscState),eStateReason));

     rv =  RejectInvalidRequests(pCallLeg,hTransc,eTranscState,&b_wasRejected);
    if(rv != RV_OK || b_wasRejected == RV_TRUE)
    {
       return;
    }

    /* if this state change occurred due to a received message, update
       this message in the call-leg */
    rv = UpdateReceivedMsg(pCallLeg, hTransc, eTranscState);
    if (RV_OK != rv)
    {
        return;
    }

#ifdef RV_SIP_IMS_ON
	if (eTranscState == RVSIP_TRANSC_STATE_TERMINATED &&
		(eStateReason == RVSIP_TRANSC_REASON_TIME_OUT ||
		 eStateReason == RVSIP_TRANSC_REASON_ERROR ||
		 eStateReason == RVSIP_TRANSC_REASON_NETWORK_ERROR ||
		 eStateReason == RVSIP_TRANSC_REASON_OUT_OF_RESOURCES))
	{
		/* Notify the security-agreement layer on the failure in trancation */
		CallLegSecAgreeMsgSendFailure(pCallLeg);
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    pSubsInfo = SipTransactionGetSubsInfo(hTransc);
    if(pSubsInfo != NULL)
    {
#ifdef RV_SIP_IMS_ON
		if (eTranscState == RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE)
		{
			/* The call-leg will not change state to message send failure if it is owned
			   by a subscription. Therefore we notify the security-agreement layer on the 
			   message send failure */
			CallLegSecAgreeMsgSendFailure(pCallLeg);
		}
#endif /* #ifdef RV_SIP_IMS_ON */

       rv = SipSubsHandleTranscChangeState(hTransc,
                                            (RvSipCallLegHandle)pCallLeg,
                                            eTranscState,
                                            eStateReason,
                                            eMethod,
                                            pSubsInfo,
                                            &b_wasHandledBySubs);
    }
    if(pSubsInfo != NULL && b_wasHandledBySubs == RV_FALSE)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - transc 0x%p subsInfo 0x%p, but transc was not handled",
            pCallLeg,hTransc,pSubsInfo));
    }
    if(b_wasHandledBySubs == RV_FALSE)
    {
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE )
        {
            rv = CallLegInviteFindObjByTransc(pCallLeg, hTransc, &pInvite);
            if(rv != RV_OK || pInvite == NULL) /* INVITE transaction, no related invite object */
            {
                RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - Failed to find invite obj for transc 0x%p state %d (rv=%d)",
                    pCallLeg,hTransc,eTranscState,rv));
                
                if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE &&
                    eTranscState == RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD)
                {
                    RvUint16 responseCode = 0;
                    /* if this is a re-invite that was received in the middle of a re-invite process,
                    then an invite object was not created */
                    SipTransactionGetResponseCode(hTransc, &responseCode);
                    if (responseCode == 491 || responseCode == 500)
                    {
                        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                            "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - in the middle of an outgoing re-Invite. rejecting transc 0x%p with %d",
                            pCallLeg, hTransc, responseCode));
                        rv = RvSipTransactionRespond(hTransc,responseCode,"Server Internal Error - re-Invite inside a re-Invite");
                            
                    }
                    else
                    {
                        /* excep */
                        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                            "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - transc 0x%p has no invite, rejectCode=%d ???",
                            pCallLeg, hTransc, responseCode));
                    }

                    if(rv != RV_OK)
                    {
                        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                            "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - Failed in sending %d response (re-invite in re-invite)",
                            pCallLeg, responseCode));
                    }  
                    SipTransactionDetachOwner(hTransc);
                    return;
            
                }
                
                if(eTranscState != RVSIP_TRANSC_STATE_TERMINATED)
                {
                    return;
                }
            }
        }

        /*-------------------------------------------------------
            Handle the new transaction state
          -------------------------------------------------------*/
        switch(eTranscState)
        {
        case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
            rv = HandleTranscStateInviteRequestRcvd(pCallLeg, pInvite, hTransc);
            break;
        case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
            rv = HandleTranscStateGeneralRequestRcvd(pCallLeg,hTransc, eStateReason);
            break;
        case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
            {
                if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
                {
                    CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,
                        hTransc,
                        RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_PROCEEDING,
                        eStateReason);
                }
            }
            break;
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
            rv = CallLegTranscEvHandleInviteFinalResponseRcvd(pCallLeg, pInvite, hTransc, eStateReason);
            break;
        case RVSIP_TRANSC_STATE_TERMINATED:
            rv = HandleTranscStateTerminated(pCallLeg, pInvite, hTransc, eStateReason);
            break;
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
            rv = HandleInviteProvisionalResponseRecvd(pCallLeg, pInvite);
            break;
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT:
            rv = HandleTranscStateInviteProceedingTimeOut(pCallLeg, pInvite);
            break;
        case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
            rv = HandleTranscStateInviteFinalResponseSent(pCallLeg, pInvite, hTransc, eStateReason);
            break;
        /*the following are notification the call-leg already know since
          they are results of its own actions*/
        case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT: /*general was sent*/
            if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
            {
                CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                                    hTransc,
                                                     RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT,
                                                    eStateReason);
            }
            else if(eMethod == SIP_TRANSACTION_METHOD_PRACK)
            {
                CallLegCallbackPrackStateChangedEv(pCallLeg,RVSIP_CALL_LEG_PRACK_STATE_PRACK_SENT,
                                             RVSIP_CALL_LEG_REASON_UNDEFINED,-1,hTransc);

            }
            break;
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING: /*invite was sent*/
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT: /*invite ack was sent*/
        case RVSIP_TRANSC_STATE_IDLE: /*transaction created*/
             break;
        case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
            rv = HandleTranscStateGeneralFinalResponseRcvd(pCallLeg, hTransc, eStateReason);
            break;
        case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
            rv = HandleTranscStateCancelFinalResponseRcvd(pCallLeg,hTransc);
            break;
        case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
            rv = HandleTranscInviteAckRcvd(pCallLeg,pInvite, hTransc);
            break;
        case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT: /*bye final response sent*/
            rv = HandleTranscStateGeneralFinalResponseSent(pCallLeg,hTransc, eStateReason);
            break;
    #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
            rv = HandleTranscStateMsgSendFailure(pCallLeg, hTransc, pInvite, eMethod, eStateReason);
            break;
    #endif
        default:
            break;
        }

    } /* if(b_wasHandledBySubs == RV_FALSE)*/
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegTranscEvStateChangedHandler - Call-leg 0x%p - Failed to handle transc 0x%p state %d (rv=%d)",
                pCallLeg,hTransc,eTranscState,rv));

    }
    ResetReceivedMsg(pCallLeg, eTranscState);
#ifdef RV_SIP_IMS_ON
	/* reset message received transaction */
	pCallLeg->hMsgRcvdTransc = NULL;
#endif
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * CallLegTranscEvAuthCredentialsFound
 * ------------------------------------------------------------------------
 * General:  Supply an authorization header, to pass it to the user,
 *           that will continue the authentication procedure, according to
 *           the realm and username parameters in it.
 *           in order to continue the procedure, user shall use AuthenticateHeader()
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The sip stack transaction handle
 *          hCallLeg  - Call-leg handle for the transaction
 *          hAuthorization - Handle to the authorization header that was found.
 *          bCredentialsSupported - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV CallLegTranscEvAuthCredentialsFound(
                        IN    RvSipTranscHandle               hTransc,
                        IN    RvSipTranscOwnerHandle          hCallLeg,
                        IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                        IN    RvBool                         bCredentialsSupported)
{
    CallLeg   *pCallLeg =  (CallLeg*)hCallLeg;
    SipTransactionMethod eMethod;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscEvAuthCredentialsFound - Call-leg 0x%p - transc 0x%p - authorization 0x%p, supported %d ",
            pCallLeg,hTransc,hAuthorization, bCredentialsSupported));

    SipTransactionGetMethod(hTransc,&eMethod);

    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipSubsNotifyHandleAuthCredentialsFound(hTransc, eMethod, hAuthorization,bCredentialsSupported);
        return;
    }

    CallLegCallbackAuthCredentialsFoundEv(hTransc,
                                           (RvSipCallLegHandle)pCallLeg,
                                           hAuthorization,
                                           bCredentialsSupported);
}

/***************************************************************************
* CallLegTranscEvAuthCompleted
* ------------------------------------------------------------------------
* General:  Notify that authentication procedure is completed.
*           If it is completed because a correct authorization was found,
*           bAuthSucceed is RV_TRUE.
*           If it is completed because there are no more authorization headers
*           to check, or because user ordered to stop the searching for
*           correct header, bAuthSucceed is RV_FALSE.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     hTransc      - The sip stack transaction handle
*          hCallLeg     - Call-leg handle for the transaction
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
***************************************************************************/
void RVCALLCONV CallLegTranscEvAuthCompleted(
                                             IN    RvSipTranscHandle        hTransc,
                                             IN    RvSipTranscOwnerHandle   hCallLeg,
                                             IN    RvBool                  bAuthSucceed)
{
    CallLeg   *pCallLeg =  (CallLeg*)hCallLeg;
    SipTransactionMethod eMethod;

    SipTransactionGetMethod(hTransc,&eMethod);

    if(bAuthSucceed == RV_TRUE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscEvAuthCompleted - Auth was completed with success!!! Call-leg 0x%p, transc 0x%p",
            pCallLeg,hTransc));
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscEvAuthCompleted - Auth was completed with failure!!! Call-leg 0x%p, transc 0x%p",
            pCallLeg,hTransc));
    }

    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipSubsNotifyHandleAuthCompleted(hTransc, eMethod, bAuthSucceed);
        return;
    }

    CallLegCallbackAuthCompletedEv(hTransc, (RvSipCallLegHandle)pCallLeg, bAuthSucceed);
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * CallLegTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hCallLeg       - Call-leg handle for the transaction
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvOtherURLAddressFound(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hCallLeg,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved)
{
    CallLeg                *pCallLeg   =  (CallLeg*)hCallLeg;
    RvStatus             rv            = RV_OK;
    SipTransactionMethod eMethod;

    if (NULL == hCallLeg || NULL == hAddress || NULL == hSipURLAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscEvOtherURLAddressFound - Call-leg 0x%p - transc 0x%p - Other URL was found ",
            pCallLeg,hTransc));

    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipTransactionGetMethod(hTransc,&eMethod);
        rv = SipSubsNotifyHandleOtherURLAddressFound(hTransc,
                                                     eMethod,
                                                     hMsg,
                                                     hAddress,
                                                     hSipURLAddress,
                                                     bAddressResolved);
        return rv;
    }

    rv = CallLegCallbackTranscOtherURLAddressFoundEv(pCallLeg,
                                               hTransc,
                                               hMsg,
                                               hAddress,
                                               hSipURLAddress,
                                               bAddressResolved);
    return rv;
}


/***************************************************************************
 * CallLegTranscNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the call-leg transc is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.

 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hCallLeg       - Call-leg handle for the transaction
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscNewConnInUseEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hCallLeg,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    CallLeg              *pCallLeg   =  (CallLeg*)hCallLeg;
    RvStatus              rv            = RV_OK;
    SipTransactionMethod  eMethod;

    if (NULL == hCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /*if this is a connection the call-leg already knows*/
    if(hConn == pCallLeg->hTcpConn)
    {
        return RV_OK;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegTranscNewConnInUseEv - Call-leg 0x%p - transc 0x%p - new conn 0x%p in use ",
            pCallLeg,hTransc,hConn));

   /*attach to the new connection and notify the application*/
    if(pCallLeg->bIsPersistent == RV_TRUE)
    {
        if(pCallLeg->hTcpConn != NULL)
        {
            CallLegDetachFromConnection(pCallLeg);
        }
        rv = CallLegAttachOnConnection(pCallLeg,hConn);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscNewConnInUseEv - Call-leg 0x%p - transc 0x%p - failed to attach to conn 0x%p",
                pCallLeg,hTransc,hConn));
            return rv;
        }
    }
    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipTransactionGetMethod(hTransc,&eMethod);
        rv = SipSubsNotifyHandleNewConnInUse(hTransc,
                                             eMethod,
                                             hConn,
                                             bNewConnCreated);
        return rv;
    }
    rv = CallLegCallbackNewConnInUseEv(pCallLeg,
                                       hConn,
                                       bNewConnCreated);
    return rv;
}





/***************************************************************************
 * CallLegTranscFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *          When this callback is called the application can query the
 *          transaction about the destination address using the
 *          RvSipTransactionGetCurrentDestAddr() API function.
 *          If the application wishes it can update the sent-by part of
 *          the top most via. The application must not update the branch parameter.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hCallLeg     - The call-leg handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscFinalDestResolvedEv(
                            IN RvSipTranscHandle       hTransc,
                            IN RvSipTranscOwnerHandle  hCallLeg,
                            IN RvSipMsgHandle          hMsgToSend)
{
    CallLeg                *pCallLeg =  (CallLeg*)hCallLeg;
    RvStatus             rv          = RV_OK;
    SipTransactionMethod eMethod;

    if (NULL == hCallLeg || NULL == hMsgToSend)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Save the active transmitter before going up to the callback */
    rv = RvSipTransactionGetTransmitter(hTransc, &pCallLeg->hActiveTrx);
    if (RV_OK != rv || NULL == pCallLeg->hActiveTrx)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscFinalDestResolvedEv - Call-leg 0x%p - Failed to get trx from transc 0x%p, msg=0x%p",
            pCallLeg,hTransc,hMsgToSend));
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegTranscFinalDestResolvedEv - Call-leg 0x%p - transc 0x%p msg 0x%p - Final dest address was resolved ",
                pCallLeg,hTransc,hMsgToSend));

#ifdef RV_SIP_IMS_ON 
	CallLegSecAgreeDestResolved(pCallLeg, hMsgToSend, NULL, hTransc);
#endif /*RV_SIP_IMS_ON */

    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipTransactionGetMethod(hTransc,&eMethod);
        rv = SipSubsNotifyHandleFinalDestResolved(hTransc,
                                                  eMethod,
                                                  hMsgToSend);
    }
    else
    {
        rv = CallLegCallbackFinalDestResolvedEv(pCallLeg,
                                                hTransc,
                                                hMsgToSend);

        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscFinalDestResolvedEv - failure during FinalDestResolvedEv for Call-leg 0x%p - transc 0x%p msg 0x%p",
                pCallLeg,hTransc,hMsgToSend));
            pCallLeg->hActiveTrx = NULL;
            return rv;
        }
    }

    /* Reset the active transmitter after the callback */
    pCallLeg->hActiveTrx = NULL;


    return rv;
}


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * CallLegTranscSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp compressed request
 *          was sent over the transaction and a SigComp timer expired
 *          before receiving any response. This callback is part of a
 *          recovery mechanism and it supplies information about the previous
 *          sent request (that wasn't responded yet). The callback also
 *          gives the application the ability to determine if there will be
 *          additional sent request and what is the type of the next sent
 *          request.
 *          NOTE: A RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED value of the out
 *          parameter is referred as a request from the application to block
 *          the sending of additional requests in the current transaction
 *          session.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK no further message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hCallLeg      - The call-leg handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscSigCompMsgNotRespondedEv(
                        IN  RvSipTranscHandle            hTransc,
                        IN  RvSipTranscOwnerHandle       hCallLeg,
                        IN  RvInt32                      retransNo,
                        IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                        OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    CallLeg                *pCallLeg =  (CallLeg*)hCallLeg;
    RvStatus                rv          = RV_OK;
    SipTransactionMethod    eMethod;
    RvSipAppTranscHandle    hAppTransc = NULL;


    if (NULL == hCallLeg || NULL == hTransc || NULL == peNextMsgComp)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegTranscSigCompMsgNotRespondedEv - Call-leg 0x%p, transc 0x%p,prev=%s retrans=%d SigComp timeout expired",
                pCallLeg,hTransc,SipTransactionGetMsgCompTypeName(ePrevMsgComp),retransNo));

    /* check if the call-leg callback or the subscription callback should be called */
    if(SipTransactionGetSubsInfo(hTransc) != NULL)
    {
        SipTransactionGetMethod(hTransc,&eMethod);
        rv = SipSubsNotifyHandleSigCompMsgNotResponded(
                                                   hTransc,
                                                   hAppTransc,
                                                   eMethod,
                                                   retransNo,
                                                   ePrevMsgComp,
                                                   peNextMsgComp);
    }
    else
    {

        rv = CallLegCallbackSigCompMsgNotRespondedEv(
                                                 pCallLeg,
                                                 hTransc,
                                                 hAppTransc,
                                                 retransNo,
                                                 ePrevMsgComp,
                                                 peNextMsgComp);

        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTranscSigCompMsgNotRespondedEv - failure during SigCompMsgNotRespondedEv for Call-leg=0x%p, transc=0x%p,prev=%s,retrans=%d",
                pCallLeg,hTransc,SipTransactionGetMsgCompTypeName(ePrevMsgComp),retransNo));
            return rv;
        }
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */


/***************************************************************************
 * CallLegTranscEvInviteResponseNoTranscRcvdEvHandler
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new 1xx/2xx response
 *          for INVITE, and the response is not related to any transaction.
 *          The callback will be called in the following cases:
 *          1. The INVITE request was forked by a forking proxy
 *          to several UAS, and each one of them returns 1xx and 2xx responses.
 *          2. For 2xx retransmissions since the first 2xx terminates the
 *          transaction.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLegMgr
 *          hReceivedMsg - The received 1xx/2xx response.
 *          pRcvdFromAddr - The address that the response was received from
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvInviteResponseNoTranscRcvdEvHandler (
                      IN    void*					hCallLegMgr,
                      IN    RvSipMsgHandle			hReceivedMsg,
                      IN    SipTransportAddress*    pRcvdFromAddr,
                      OUT   RvBool*					pbWasHandled)
{
    RvStatus       rv       = RV_OK;
    CallLeg       *pCallLeg = NULL;
    CallLegMgr    *pMgr     = (CallLegMgr*)hCallLegMgr;
    RvInt16        responseCode;
    RvInt32         curIdentifier ;
	RvBool			bNotifyApp = RV_TRUE;
    

    *pbWasHandled = RV_FALSE;

    responseCode = RvSipMsgGetStatusCode(hReceivedMsg);
    if(responseCode < 100 || responseCode >= 300)
    {
        /* this function handles only 1xx/2xx responses!!! */
        RvLogExcep(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler: mgr 0x%p - illegal response code %d message 0x%p...",
            pMgr, responseCode, hReceivedMsg));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    *pbWasHandled = RV_TRUE;

    RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler: mgr 0x%p - start to handle %d response message 0x%p...",
        pMgr, responseCode, hReceivedMsg));
    
    /* 1. find a call-leg. */
    rv = CallLegMgrFindCallLegByMsg(hCallLegMgr, 
                                    hReceivedMsg, 
                                    RVSIP_CALL_LEG_DIRECTION_OUTGOING,
                                    RV_FALSE /*bOnlyEstablishedCall*/, 
                                    (RvSipCallLegHandle*)&pCallLeg,
                                    &curIdentifier);
    if(rv != RV_OK || pCallLeg == NULL) 
    {
        /* call-leg was not found. create forked call-leg if needed.*/
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler - did not find valid call-leg. "));

        rv = CallLegForkingCreateForkedCallLegFromMsg(pMgr, hReceivedMsg, &pCallLeg, &curIdentifier, &bNotifyApp);
        if(rv != RV_OK || NULL == pCallLeg)
        {
            RvLogWarning(pMgr->pLogSrc, (pMgr->pLogSrc,
                "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler - Failed to create a forked call-leg from msg 0x%p",
                hReceivedMsg));
            *pbWasHandled = RV_FALSE;
            return rv;
        }
        else /* new forked call-leg was created */
        {
            RvBool bTerminated = RV_FALSE;
            
            /* ----------- Lock the call-Leg we crated -----------*/
            rv = CallLegLockMsg(pCallLeg);
            if(rv != RV_OK) /*general failure*/
            {
                RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler - failed to lock new forked call-leg 0x%p. rv=%d.",
                    pCallLeg, rv));
				/*If the bNotifyApp flag is true it means that new call-leg was created, since the lockMsg failed
				  we need to destruct this new call-leg.*/
				if (bNotifyApp == RV_TRUE)
				{
					CallLegDestruct(pCallLeg);
				}
				
                return rv;
            }

			/*Check whether the application already got notification about this forked dialog, if so don't notify again*/
			if (bNotifyApp == RV_TRUE)
			{
				/* inform application about successful creation of forked call-leg */
				CallLegCallbackCreatedDueToForkingEv(pMgr,
												  (RvSipCallLegHandle)pCallLeg,
												  &bTerminated);
       
				if(bTerminated == RV_TRUE)
            {
					RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
					  "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler: mgr 0x%p, application terminated new forked call 0x%p",
					  pMgr, pCallLeg));
					CallLegDestruct(pCallLeg);
					CallLegUnLockMsg(pCallLeg);
					return RV_OK;
				}
			}
        }
    }
    else
    {
        RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler: mgr 0x%p - Found Call-leg 0x%p",
            pMgr, pCallLeg));
        
        /* ----------- Lock the call-Leg we found -----------*/
        rv = CallLegLockMsg(pCallLeg);
        if(rv != RV_OK) /*general failure*/
        {
            RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler - failed to lock found call-leg 0x%p. rv=%d.",
                pCallLeg, rv));
            return rv;
        }
    }
    
    rv = HandleInviteResponseNoTransc(pCallLeg, hReceivedMsg, pRcvdFromAddr, curIdentifier, responseCode);
    if(rv != RV_OK) /*general failure*/
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegTranscEvInviteResponseNoTranscRcvdEvHandler - call-leg 0x%p. failed to handle the response message(rv=%d:%s)",
            pCallLeg, rv, SipCommonStatus2Str(rv)));
        CallLegUnLockMsg(pCallLeg);
        return rv;
    }
	
	/* ----------- Unlock the call-Leg -----------*/
    CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}


/***************************************************************************
 * CallLegTranscEvAckNoTranscEvHandler
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives an ACK request
 *          that does not match any transaction. This will be the case
 *          when the INVITE was responded with 2xx response and was terminated
 *          immediately after that.
 * Return Value: RV_Status - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hAckMsg       - The handle to the received ACK message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvAckNoTranscEvHandler(
                     IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hAckMsg,
                     IN   SipTransportAddress*    pRcvdFromAddr,
                     OUT  RvBool*                 pbWasHandled)
{
    RvStatus rv = RV_OK;
    CallLeg* pCallLeg = NULL;
    CallLegMgr* pMgr = (CallLegMgr*)hCallLegMgr;
    RvInt32 curIdentifier ;
    

    RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegTranscEvAckNoTranscEvHandler: mgr 0x%p - start to handle ACK message 0x%p...",
        pMgr, hAckMsg));

    *pbWasHandled = RV_FALSE;

    rv = CallLegMgrFindCallLegByMsg(hCallLegMgr, 
                                  hAckMsg, 
                                  RVSIP_CALL_LEG_DIRECTION_INCOMING,
                                  RV_TRUE /*bOnlyEstablishedCall*/, 
                                  (RvSipCallLegHandle*)&pCallLeg,
                                  &curIdentifier);

    if(rv != RV_OK || NULL == pCallLeg) /*general failure*/
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegTranscEvAckNoTranscEvHandler - failed to find valid call-leg. ignore ACK..."));
        return rv;
    }
    
    RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegTranscEvAckNoTranscEvHandler: mgr 0x%p - found call-leg 0x%p for ACK msg 0x%p",
        pMgr, pCallLeg, hAckMsg));

    /* ----------- Lock the forked call-Leg we found -----------*/
    rv = CallLegLockMsg(pCallLeg);
    if(rv != RV_OK) /*general failure*/
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegTranscEvAckNoTranscEvHandler - failed to lock found call-leg 0x%p. rv=%d.",
            pCallLeg, rv));
        return rv;
    }
    
    if(RV_FALSE == CallLegVerifyValidity(pMgr, pCallLeg, curIdentifier))
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegTranscEvAckNoTranscEvHandler - call-leg 0x%p is not valid.",
            pCallLeg));
        
        CallLegUnLockMsg(pCallLeg);  /*unlock the call-leg*/
        return RV_ERROR_UNKNOWN;
    }
    /* if we reached this point, we know that a related call-leg was found,
       so we declared that we handled this ACK message */
    *pbWasHandled = RV_TRUE;

	/* here we have a locked call-leg to work with */
    /* find the invite object */
    rv = HandleAckNoTranscRequest(pCallLeg, hAckMsg, pRcvdFromAddr);
	
	CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}


/***************************************************************************
* CallLegTranscEvHandleInviteFinalResponseRcvd
* ------------------------------------------------------------------------
* General: This function is called when a transaction receives a final
*          response for an Invite request, or the reInvite request.
*          This function call two sub functions to handle the two
*          cases.
* Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
*                            state.
*               RV_OK - on success.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - The call-leg that received the transaction event.
*          hTransc - Handle to the related transaction.
*            eStateReason - the reason for the final response.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV CallLegTranscEvHandleInviteFinalResponseRcvd(
                                 IN  CallLeg                           *pCallLeg,
                                 IN  CallLegInvite*                    pInvite,
                                 IN  RvSipTranscHandle                 hTransc,
                                 IN  RvSipTransactionStateChangeReason eStateReason)
{

    RvStatus rv = RV_OK;
    RvUint16 statusCode;

    statusCode = RvSipMsgGetStatusCode(pCallLeg->hReceivedMsg);


    if (pCallLeg->pMgr->eSessiontimeStatus ==
                RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
    {
        rv = CallLegSessionTimerHandleFinalResponseRcvd(
						pCallLeg,hTransc,pCallLeg->hReceivedMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvHandleInviteFinalResponseRcvd - Call-leg 0x%p\
            Failed in CallLegSessionTimerHandleFinalResponseRcvd",
            pCallLeg));
            return rv;

        }
    }

    switch(pCallLeg->eState)
    {
    case RVSIP_CALL_LEG_STATE_INVITING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
        rv = HandleInviteFinalResponse(pCallLeg, pInvite, hTransc,eStateReason, statusCode);
        break;
    case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
        rv = HandleReInviteFinalResponse(pCallLeg, pInvite, hTransc,eStateReason);
        break;
    case RVSIP_CALL_LEG_STATE_CANCELLING:
        rv = HandleInviteFinalResponseInCancellingState(pCallLeg, pInvite, hTransc);
        break;
    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        /* after sending the BYE, still need to send ACK of the 487 response that may arrive
           for pending invites */
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvHandleInviteFinalResponseRcvd  - Call-leg 0x%p - send ACK on %d response on call state %s",
              pCallLeg,statusCode, CallLegGetStateName(pCallLeg->eState)));
        if(statusCode >= 200 && statusCode < 300)
        {
            /* save the To header of the 2xx response (we will use it in building the ACK) */
            rv = CallLegInviteCopyToHeaderFromMsg(pCallLeg,pInvite,pCallLeg->hReceivedMsg);
            if(RV_OK != rv)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegTranscEvHandleInviteFinalResponseRcvd - Call-leg 0x%p failed to copy TO header in invite 0x%p",
                    pCallLeg, pInvite));
                return rv;
            }
        }
        
        rv = CallLegInviteSendAck(pCallLeg, hTransc, pInvite, statusCode);
        break;
    case RVSIP_CALL_LEG_STATE_DISCONNECTED:
    case RVSIP_CALL_LEG_STATE_TERMINATED:
    case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"CallLegTranscEvHandleInviteFinalResponseRcvd  - Call-leg 0x%p - nothing to do with %d response on call state %s",
					pCallLeg,statusCode, CallLegGetStateName(pCallLeg->eState)));
        return RV_OK;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegTranscEvHandleInviteFinalResponseRcvd - Call-leg 0x%p - Invalid transc state for call state %s",
              pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        return RV_ERROR_UNKNOWN;
    }
    if (RV_OK != rv)
    {
        return rv;
    }
    if (pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED &&
        statusCode >=200 && statusCode <300)
    {
        rv = CallLegSessionTimerHandleTimers(
					pCallLeg,hTransc,pCallLeg->hReceivedMsg,RV_TRUE);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegTranscEvHandleInviteFinalResponseRcvd - Call-Leg 0x%p \
            Failed in CallLegSessionTimerHandleTimers"
            ,pCallLeg));
            return rv;
        }
    }

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
        T R A N S A C T I O N  S T A T E  C H A N G E
 ------------------------------------------------------------------------*/


/*------------------------------------------------------------------------
        I N V I T E   R E Q U E S T   R E C E I V E D
 -------------------------------------------------------------------------*/

/***************************************************************************
 * HandleTranscStateInviteRequestRcvd
 * ------------------------------------------------------------------------
 * General: The behavior of the sip stack when an invite is received is defined
 *          on rfc2543 section 4.2.1 p.25.
 *          A new invite can be received only on state idle.
 *          A reInvite can be received only on state connected with no other
 *          pending invite transaction.
 *          If there is a pending outgoing invite the response is 500 with
 *          retry after header.
 *          If there is a pending incoming invite the response is 400.
 * Return Value:  RV_ERROR_UNKNOWN - an error occurred while trying to send the
 *                               response message (400/500).
 *                RV_OK - On success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the invite request state.
 *            hTransc - The transaction the request belongs to.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle      hTransc)

{
    RvStatus retStatus;
    RvBool   bInformOfAutoResponseSent = RV_FALSE;

    switch(pCallLeg->eState)
    {
    /*-----------------------------
        Initial Invite received
    -------------------------------*/
    case RVSIP_CALL_LEG_STATE_IDLE:
        retStatus = HandleInviteRequestRcvd(pCallLeg, pInvite, hTransc);
        break;
    /*------------------------------
        A re-Invite received
     -------------------------------*/
    case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
            retStatus = HandleReInviteRequestRcvd(pCallLeg, pInvite, hTransc);
            break;
    /*------------------------------
        Error
     -------------------------------*/
    /* state inviting - Invite was sent and no final response was rcvd
      and now new Invite was rcvd - return 500 - server error.*/
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
    case RVSIP_CALL_LEG_STATE_INVITING:
    case RVSIP_CALL_LEG_STATE_CANCELLING:
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "HandleTranscStateInviteRequestRcvd - Call-leg 0x%p - received INVITE in the state %s - rejecting with 500",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        retStatus = RvSipTransactionRespond(hTransc,500,NULL);
        if(retStatus == RV_OK)
        {
            bInformOfAutoResponseSent = RV_TRUE;
        }
        break;
    case RVSIP_CALL_LEG_STATE_OFFERING:
        {
            CallLegTranscMemory* pMemory;
            retStatus = CallLegAllocTransactionCallMemory(hTransc, &pMemory);
            if(retStatus != RV_OK || NULL == pMemory)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleTranscStateInviteRequestRcvd - Call-leg 0x%p - cannot set the transaction context, when an INVITE is received in state %s",
                    pCallLeg, CallLegGetStateName(pCallLeg->eState)));
                return retStatus;
            }
            else
            {
                pMemory->dontDestructRouteList = RV_TRUE;
            }
        }
        
    /*for any other state the Invite is a bad request and a 400 (bad
      request) response is sent*/
    default:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "HandleTranscStateInviteRequestRcvd - Call-leg 0x%p - cannot handle incoming INVITE in state %s - rejecting with 400",
                pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        retStatus =  RvSipTransactionRespond(hTransc,400,NULL); /*bad request*/
        if(retStatus == RV_OK)
        {
            bInformOfAutoResponseSent = RV_TRUE;
        }
    }

    if(RV_TRUE == bInformOfAutoResponseSent && pInvite != NULL)
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                     RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                     RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
    }
        
    return retStatus;
}

/***************************************************************************
 * HandleTranscStateInviteProceedingTimeOut
 * ------------------------------------------------------------------------
 * General: Handles request timeout. This function is called if an UAC INVITE
 *          transaction received provisional response, but didn't receive final
 *          response, and reached provisional timeout. This function will send
 *          CANCEL to the INVITE.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - the handle to the call leg its transaction reached
 *        invite request timeout.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateInviteProceedingTimeOut(
                            IN  CallLeg        *pCallLeg,
                            IN  CallLegInvite*  pInvite)
{
    RvStatus rv = RV_OK;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscStateInviteProceedingTimeOut - Call-leg 0x%p - received transaction request timeout state",
              pCallLeg));

	if(NULL == pInvite)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"HandleTranscStateInviteProceedingTimeOut - Call-leg 0x%p - Failed, pInvite is NULL",
			pCallLeg));
		return RV_ERROR_INVALID_HANDLE;
	}

    if (RV_TRUE == pCallLeg->pMgr->enableInviteProceedingTimeoutState)
    {
        /* check if the time out refers to a invite or to re-invite by the modify
           state */
        if (RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING == pInvite->eModifyState)
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                                    RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT,
                                    RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        }
        else
        {
            CallLegChangeState(pCallLeg,
                               RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT,
                               RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        }
    }
    else
    {
        rv = CallLegSessionDisconnect(pCallLeg);
    }
    return rv;
}

/***************************************************************************
 * HandleInviteRequestRcvd
 * ------------------------------------------------------------------------
 * General: When an initial invite is received the state of the call is
 *          changed to offering.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the invite request state.
 *            hTransc - The transaction the request belongs to.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc)

{
    RvStatus rv            = RV_OK;
    RvBool    bLegalReplaces  = RV_TRUE;
    RvSipMsgHandle            hMsg;
    RvBool                   bIsRejected = RV_FALSE;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "HandleInviteRequestRcvd - Call-leg 0x%p - INVITE received", pCallLeg));

    CallLegSetActiveTransc(pCallLeg, hTransc);

    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = CallLegUpdateReplacesStatus(pCallLeg, hMsg);
    if(rv != RV_OK)
    {
        return rv;
    }
    if(pCallLeg->pMgr->statusReplaces != RVSIP_CALL_LEG_REPLACES_UNDEFINED)
    {
        rv = CallLegReplacesLoadReplacesHeaderToCallLeg(pCallLeg, hMsg, &bLegalReplaces);
        if(rv != RV_OK)
        {
            return rv;
        }

        if(bLegalReplaces != RV_TRUE)
        {
            /* There is more then one replaces header in the Call Leg */
            rv = CallLegSessionReject(pCallLeg,400);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                           "HandleInviteRequestRcvd - CallLeg 0x%p hTransc=0x%p - CallLegSessionReject (Replaces) failed(rv=%d:%s)",
                           pCallLeg, hTransc, rv, SipCommonStatus2Str(rv)));
                CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
                CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            }
            return rv;
        }
    }

    if (pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
    {
        rv = CallLegSessionTimerHandleRequestRcvd(pCallLeg,hTransc,hMsg,&bIsRejected);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleInviteRequestRcvd - Call-leg 0x%p Failed in CallLegSessionTimerHandleInviteRequestRcvd",
                      pCallLeg));
            rv = CallLegSessionReject(pCallLeg,400);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleInviteRequestRcvd - CallLeg 0x%p hTransc=0x%p - CallLegSessionReject (SessionTimer) failed(rv=%d:%s)",
                    pCallLeg, hTransc, rv, SipCommonStatus2Str(rv)));
                CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
                CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
                return RV_ERROR_UNKNOWN;
            }
            return rv;
        }
        if (bIsRejected == RV_TRUE)
        {
            return rv;
        }
    }



    if(pCallLeg->hReplacesHeader != NULL)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_OFFERING,
                                RVSIP_CALL_LEG_REASON_REMOTE_INVITING_REPLACES);
    }
    else
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_OFFERING,
                                RVSIP_CALL_LEG_REASON_REMOTE_INVITING);
    }

    RV_UNUSED_ARG(pInvite)
    return RV_OK;
}

/***************************************************************************
 * HandleReInviteRequestRcvd
 * ------------------------------------------------------------------------
 * General: Handles a reInvite request.
 *          If the call-leg is already modifying (reInvite was sent) the
 *          response is 491 with retry after header.
 *          If the call-leg is already modified (reInvite was received)
 *          the response is 500.
 *          (Old invite behavior - only one 'active' re-invite object may exist,
 *           so it is enough to check the state of it.
 *           New invite behavior - may be several invite objects, that are waiting
 *           for ACK to be sent/received. in this case the active transaction will
 *           be NULL. only if there is an invite object that still waits for final
 *           response, we shall reject the re-invite, and in this case, the active
 *           transaction of the call-leg is not NULL).
 *          If the reInvite is valid the connection state is changed to
 *          modified and the user receives a notification using the
 *          RvSipCallLegModifyRequestRcvdEv.
 * Return Value:  RV_ERROR_UNKNOWN - an error occurred while trying to send the
 *                                 response message.(400/500).
 *                RV_OK - On success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the invite request state.
 *            hTransc - The transaction the request belongs to.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleReInviteRequestRcvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc)

{
    RvStatus            rv;
    RvBool              bIsRejected = RV_FALSE;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteRequestRcvd - Call-leg 0x%p invite 0x%p - Re-Invite received", 
                pCallLeg, pInvite));


    /* For both new invite and old invite - 
       If we are in the middle of a re-invite process, the call-leg's active transaction
       is not NULL.
       old-invite: we will not handle new re-invite until an ACK is sent/received
       --> as long as the active transaction exists.
       new-invite: we will not handle new re-invite until a 2xx is sent/received
       --> as long as the active transaction exists. */

    if(pCallLeg->hActiveTransc != NULL)
    {
        RvInt16 rejectCode = CallLegInviteDefineReInviteRejectCode(pCallLeg, pCallLeg->hActiveTransc);
        if(rejectCode > 0)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteRequestRcvd - Call-leg 0x%p invite 0x%p - reject transc 0x%p with %d re-Invite inside a re-Invite",
                    pCallLeg, pInvite, hTransc, rejectCode));
            rv = RvSipTransactionRespond(hTransc,rejectCode,"Server Internal Error - re-Invite inside a re-Invite");
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleReInviteRequestRcvd - Call-leg 0x%p invite 0x%p - Failed in sending %d response",
                    pCallLeg, pInvite, rejectCode));
            }
            return CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
        }
        else
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteRequestRcvd - Call-leg 0x%p - active transc 0x%p rejectCode=0. can't be",
                pCallLeg, pCallLeg->hActiveTransc));
        }
    }
    
    if (pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
    {
		RvSipMsgHandle hMsg;
		/* Get the received message*/
		rv = RvSipTransactionGetReceivedMsg(hTransc,&hMsg);
		if(rv != RV_OK)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"HandleReInviteRequestRcvd - Call-leg 0x%p - Failed to retrieve Msg handle of transc 0x%p (rv=%d)",
				pCallLeg,hTransc,rv));
			return rv;
			
		}

        rv = CallLegSessionTimerHandleRequestRcvd(pCallLeg,hTransc,hMsg,&bIsRejected);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleReInviteRequestRcvd - Call-leg 0x%p invite 0x%p - Failed in CallLegSessionTimerHandleInviteRequestRcvd",
                      pCallLeg,pInvite));
            return rv;
        }
        if (bIsRejected == RV_TRUE)
        {
            return rv;
        }
    }

    /*the connection substate is Idle*/
    /*set the reInvite to be the active transaction*/
    CallLegSetActiveTransc(pCallLeg, hTransc);
    CallLegChangeModifyState(pCallLeg, pInvite,
                             RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD,
                             RVSIP_CALL_LEG_REASON_REMOTE_INVITING);


    CallLegCallbackModifyRequestRcvdEv(pCallLeg);

    return RV_OK;
}

/*------------------------------------------------------------------------
        G E N E R A L   R E Q U E S T   R E C E I V E D
 -------------------------------------------------------------------------*/


/***************************************************************************
 * HandleTranscStateGeneralRequestRcvd
 * ------------------------------------------------------------------------
 * General:This function is called when a transaction receives a general
 *         request. The request can be for example a BYE request.
 *         The function changes the call-leg state and respond with a 200
 *         code.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateGeneralRequestRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason)
{
    /* Get the method type from the transaction */
    RvStatus              rv = RV_OK;
    RvInt32               respCode;
    SipTransactionMethod  eMethod;

    RvSipTransaction100RelStatus relStatus   = RVSIP_TRANSC_100_REL_UNDEFINED;
    RvBool                       bIsRejected = RV_FALSE;
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvBool                       bIsGoodUpdate = RV_FALSE;
#endif
    //SPIRENT_END
    RvChar                         strMethodBuffer[SIP_METHOD_LEN] = {0};

    RvSipTransactionGetMethodStr(hTransc,SIP_METHOD_LEN,strMethodBuffer);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscStateGeneralRequestRcvd - Call-leg 0x%p - General request was received",
              pCallLeg));

    SipTransactionGetMethod(hTransc,&eMethod);

    /* Bye is handled by the stack as part of the call leg state machine */
    if(eMethod == SIP_TRANSACTION_METHOD_BYE)
    {
        if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
        {
            CallLegCallbackByeStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                         hTransc,
                                         RVSIP_CALL_LEG_BYE_STATE_REQUEST_RCVD,
                                         eStateReason);
            return RV_OK;
        }
        else
        {
            return HandleByeRequestRcvd(pCallLeg,hTransc);
        }
    }

    /*reject transactions after bye was sent*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscStateGeneralRequestRcvd - Call-leg 0x%p - Responding with 400 to a general request message received in illegal state",
                  pCallLeg));
        return RvSipTransactionRespond(hTransc,400,NULL);
    }

    /*handle session timer*/
    if (pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
    {
      RvSipMsgHandle hMsg;
      /* Get the received message*/
      rv = RvSipTransactionGetReceivedMsg(hTransc,&hMsg);
      if(rv != RV_OK)
	{
	  RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					      "HandleTranscStateGeneralRequestRcvd - Call-leg 0x%p - Failed to retrieve Msg handle of transc 0x%p (rv=%d)",
					      pCallLeg,hTransc,rv));
	  return rv;
	  
	}
        rv =  CallLegSessionTimerHandleRequestRcvd(pCallLeg,hTransc,hMsg,&bIsRejected);
	if (RV_OK != rv || bIsRejected == RV_TRUE) 
        {
	  return rv;
        }
	//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
	if(strcmp(strMethodBuffer,"UPDATE")==0) {
	  rv = CallLegSessionTimerHandleTimers(pCallLeg,hTransc,hMsg,RV_TRUE);
	  if(rv==RV_OK) {
	    bIsGoodUpdate=RV_TRUE;
	  }
	}
#endif
	//SPIRENT_END
    }

    /*notify application for application transaction*/
    if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
    {
        /*notify the application about the incoming request*/
        if(pCallLeg->callLegEvHandlers != NULL &&
           pCallLeg->callLegEvHandlers->pfnTranscStateChangedEvHandler != NULL)
        {
            CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                               hTransc,RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD
                                               ,eStateReason);
        }
        else
        {
            CallLegCallbackTranscRequestRcvdEv(pCallLeg,hTransc);
        }
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscStateGeneralRequestRcvd - Call-leg 0x%p - General request received for application transaction",
                  pCallLeg));
        return RV_OK;
    }
    relStatus = SipTransactionMgrGet100relStatus(pCallLeg->pMgr->hTranscMgr);
    if(eMethod == SIP_TRANSACTION_METHOD_PRACK && relStatus != RVSIP_TRANSC_100_REL_UNDEFINED)
    {
        if(pCallLeg->ePrackState != RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD)
        {
            pCallLeg->hActiveIncomingPrack = hTransc;
            CallLegCallbackPrackStateChangedEv(pCallLeg,RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD,
                RVSIP_CALL_LEG_REASON_UNDEFINED,-1,hTransc);

        }
        return RV_OK;
    }

	//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    respCode=200;
    if(!bIsGoodUpdate) 
#endif
      //SPIRENT_END
    GetRejectRespCodeForGeneralRcvdReq(pCallLeg,eMethod,&respCode);
    if(respCode == 501)
    {
      rv = RvSipTransactionRespond(hTransc,501,NULL);
    }
    else if (respCode == 481)
    {
        rv = RvSipTransactionRespond(hTransc, 481, "Subscription does not exist");
    }
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    else if(respCode == 200) 
    {
      rv = RvSipTransactionRespond(hTransc,200,NULL);
    }
#endif
    //SPIRENT_END

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscStateGeneralRequestRcvd - Call-leg 0x%p - Failed to send %d on transaction 0x%p",
            pCallLeg, respCode, hTransc));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * HandleByeRequestRcvd
 * ------------------------------------------------------------------------
 * General: A bye request was received. A 200 response is sent and
 *          the call changes it state to disconnected.
 *          We do not leave the bye transaction for retransmissions.
 *          We terminate all transactions and then the call.
 * Return Value: RV_OK only.RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call-leg this transaction belongs to.
 *          hTransc - handle to the transaction.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleByeRequestRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc)
{

    RvStatus rv;
    RvSipMsgHandle hOutboundBackup = NULL;
    SipTransactionMethod   eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleByeRequestRcvd - Call-leg 0x%p - BYE received",
                pCallLeg));

    /*when bye is received we reset the current outbound message - so that it will not be used
      for the 487 automatic response*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        hOutboundBackup = pCallLeg->hOutboundMsg;
        pCallLeg->hOutboundMsg = NULL;
    }

    if(pCallLeg->hActiveTransc != NULL)
    {
        SipTransactionGetMethod(pCallLeg->hActiveTransc, &eMethod);
    }

    /*send 487 on a pending re-Invite*/
    if(pCallLeg->hActiveTransc != NULL &&
       SIP_TRANSACTION_METHOD_INVITE == eMethod )
    {
        CallLegInvite* pReInvite = NULL;

        CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pReInvite);

        if(pReInvite != NULL &&
           pReInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD)
        {
            /* If the application created a message in advance use it*/
              RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "HandleByeRequestRcvd - Call-leg 0x%p - automatically responding to pending re-INVITE 0x%p with stack 487 response. ",
                      pCallLeg, pReInvite));
            rv = RvSipTransactionRespond(pCallLeg->hActiveTransc,487,NULL);
            if (rv != RV_OK)
            {
                /*transaction failed to send respond*/
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                         "HandleByeRequestRcvd - Call-leg 0x%p - Failed to send response for a pending re-Invite(rv=%d)",
                          pCallLeg, rv));
                /*continue on failure and terminate the call-leg*/

            }
            else 
            {
                /* detach the transaction so that the response will be retransmitted and ACK will
				be mapped to the transaction*/
                rv = CallLegInviteDetachInviteTransc(pCallLeg, pReInvite, pCallLeg->hActiveTransc);
                
                CallLegResetActiveTransc(pCallLeg);
            }
        }
    }

    /*send 487 on a pending general transactions*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "HandleByeRequestRcvd - Call-leg 0x%p - Send 487 to all pending general transactions", pCallLeg));
    
    CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_487_ON_GEN_PENDING, NULL);

    /*restore the application outbound message*/
    if(hOutboundBackup != NULL)
    {
        pCallLeg->hOutboundMsg = hOutboundBackup;
    }
    /*at state disconnected the application has a chance to set the 200 outbound
      message*/
    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECTED);



    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
          RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleByeRequestRcvd - Call-leg 0x%p, was terminated by application in the disconnected state. no response will be sent for the BYE"
                      ,pCallLeg));
          return RV_OK;
    }

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleByeRequestRcvd - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                        ,pCallLeg,hTransc,rv));
            return RV_ERROR_UNKNOWN;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleByeRequestRcvd - Call-leg 0x%p - BYE received sending 200 automatically",
                pCallLeg));

    rv = RvSipTransactionRespond(hTransc,200,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleByeRequestRcvd - Call-leg 0x%p - Failed to send 200 response (rv=%d)",
            pCallLeg, rv));

    }
    CallLegRemoveTranscFromList(pCallLeg,hTransc);
    if(SipTransactionDetachOwner(hTransc) != RV_OK)
    {
        RvSipTransactionTerminate(hTransc);
    }
    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    return RV_OK;
}

/***************************************************************************
 * GetRejectRespCodeForGeneralRcvdReq
 * ------------------------------------------------------------------------
 * General: Gets the appropriate reject code response for the general
 *          received requests which was not handled by the application.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - Pointer to a call-leg, which received the message.
 *         hTransc - handle to the transaction.
 * Output:  (-)
 ***************************************************************************/
 static void RVCALLCONV GetRejectRespCodeForGeneralRcvdReq(
                                    IN  CallLeg              *pCallLeg,
                                    IN  SipTransactionMethod  eMethod,
                                    OUT RvInt32              *pRespCode)
{
    *pRespCode = UNDEFINED;

    switch (eMethod)
    {
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER:
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
        if(CallLegMgrGetSubsMgr(pCallLeg->pMgr) == NULL)
        {
            *pRespCode = 501;
        }
        break;
    case SIP_TRANSACTION_METHOD_NOTIFY:
        if(RV_TRUE == CallLegGetIsHidden(pCallLeg))
        {
            /* notify in hidden call-leg must be a notify for subscription that
               was already destructed, and therefore was not found */
            *pRespCode = 481;
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        /* Continue to the default code block */
    default:
        *pRespCode = 501;
    }
#ifndef RV_SIP_SUBS_ON
	RV_UNUSED_ARG(pCallLeg)
#endif /* RV_SIP_SUBS_ON */
}

/*------------------------------------------------------------------------
I N V I T E    F I N A L   R E S P O N S E    R E C V I E V E D
-------------------------------------------------------------------------*/


/***************************************************************************
 * HandleInvite2xxResponse
 * ------------------------------------------------------------------------
 * General: A 2xx response was received for an Invite. 
 *          build the ACK message and transmitter for 2xx
 *          Sends the ACK in automatic mode
 * Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
 *                            state.
 *               RV_OK - on success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event.
 *            pInvite  - The invite object related to the response
 *            hTransc - Handle to the related transaction.
 *            statusCode - The specific status code.
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInvite2xxResponse(
                            IN  CallLeg                          *pCallLeg,
                            IN  CallLegInvite                    *pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvUint16                          statusCode,
                            IN  RvSipTransactionStateChangeReason eStateReason)
{
    RvStatus rv;
    
	if(pInvite == NULL)
	{
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInvite2xxResponse - Call-leg 0x%p, pInvite==NULL",
            pCallLeg));
		return RV_ERROR_INVALID_HANDLE;
	}

	/* save the server connection from the transaction, because it might receive some more
       2xx retransmissions. */
    rv = CallLegInviteAttachOnInviteConnections(pCallLeg, pInvite, hTransc);
    if(rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInvite2xxResponse - Call-leg 0x%p failed to attach invite 0x%p to Connections",
            pCallLeg, pInvite));
    }

    rv = CallLegInviteCopyToHeaderFromMsg(pCallLeg,pInvite,pCallLeg->hReceivedMsg);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInvite2xxResponse - Call-leg 0x%p failed to copy TO header in invite 0x%p",
            pCallLeg, pInvite));
        return rv;
    }
        
    /* if not manual ack - send the ack request */
    if(RV_FALSE == pCallLeg->pMgr->manualAckOn2xx)
    {
        rv = CallLegInviteSendAck(pCallLeg, hTransc, pInvite, statusCode);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInvite2xxResponse - Call-leg 0x%p failed to send ACK with trx 0x%p- Terminating call",
                pCallLeg,pInvite->hTrx));
            return RV_ERROR_UNKNOWN;
        }
    }

    if(RV_FALSE == pCallLeg->pMgr->bOldInviteHandling ||
       (RV_TRUE == pCallLeg->pMgr->bOldInviteHandling && RV_FALSE == pCallLeg->pMgr->manualAckOn2xx))
    {
        /* 200 was received. from this point, application may send a re-invite */
        CallLegResetActiveTransc(pCallLeg);
    }
    
    RV_UNUSED_ARG(eStateReason)
    return RV_OK;
}

/***************************************************************************
 * HandleInvite2xxRetransmission
 * ------------------------------------------------------------------------
 * General: A 2xx response was received for an Invite. 
 *          build the ACK message and transmitter for 2xx
 *          Sends the ACK in automatic mode
 * Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
 *                            state.
 *               RV_OK - on success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event.
 *          hTransc - Handle to the related transaction.
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInvite2xxRetransmission(
                            IN  CallLeg        *pCallLeg,
                            IN  CallLegInvite*  pInvite,
                            IN  RvSipMsgHandle  hMsg,
                            IN  RvUint16        statusCode)
{
    RvStatus rv;
    
    /* retransmit the ACK - If was already sent once */
    if(pInvite->bFirstAck == RV_FALSE)
    {
        rv = CallLegInviteSendAck(pCallLeg, NULL, pInvite, statusCode);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInvite2xxRetransmission - Call-leg 0x%p failed to retransmit ACK with trx 0x%p- Terminating call",
                pCallLeg,pInvite->hTrx));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInvite2xxRetransmission - Call-leg 0x%p waiting for application to send the first ACK",
            pCallLeg));
    }
    RV_UNUSED_ARG(hMsg)
    return RV_OK;
}

/***************************************************************************
 * HandleInviteFinalResponse
 * ------------------------------------------------------------------------
 * General: A final response was received for an Invite. Ack is sent and
 *          the call changes state according to the response.
 *          for 4xx,4xx,6xx responses the call will be disconnected
 *          When disconnecting the Invite transaction will be detached from
 *          the call-leg (for ack retransmissions) and the call will terminate.
 * Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
 *                            state.
 *               RV_OK - on success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event.
 *          hTransc - Handle to the related transaction.
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteFinalResponse(
                            IN  CallLeg                          *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason,
                            IN   RvUint16                         statusCode)
{

    RvStatus rv;
 
    /*for every response ack should be sent first, if we failed to ACK, there is something
      wrong with the connection - Terminate the call*/

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleInviteFinalResponse - Call-leg 0x%p - received final response for INVITE",
              pCallLeg));

    /*on final response - reset the parameters saved from an incoming reliable 1xx*/
    ResetIncomingRel1xxParamsOnFinalResponse(pCallLeg);

    if(statusCode >= 200 && statusCode <300)
    {
        /*on 2xx reset the replaces header that was sent in the initial INVITE*/
        pCallLeg->hReplacesHeader = NULL;
    }
    else
    {
        CallLegRouteListProvDestruct(pCallLeg);
    }

    /* send ACK for non-2xx */
    if(statusCode >= 300)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleInviteFinalResponse - Call-leg 0x%p - Sending ACK automatically",
              pCallLeg));

        rv = CallLegInviteSendAck(pCallLeg, hTransc, NULL, statusCode);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteFinalResponse - Call-leg 0x%p failed to send ACK with transc 0x%p- Terminating call",
                pCallLeg,hTransc));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
    }
    else /* 2xx*/
    {
        rv = HandleInvite2xxResponse(pCallLeg, pInvite,hTransc, statusCode, eStateReason);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteFinalResponse - Call-leg 0x%p failed in HandleInvite2xxResponse - Terminating call",
                pCallLeg));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }   
    }

    if((statusCode < 300 || statusCode >= 400) &&
       statusCode != 401 && statusCode != 407 && statusCode != 503)
    {
        /* initial request got final response, can destruct the headers list */
        if(pCallLeg->hHeadersListToSetInInitialRequest != NULL)
        {
            RvSipCommonListDestruct(pCallLeg->hHeadersListToSetInInitialRequest);
            pCallLeg->hHeadersListToSetInInitialRequest = NULL;
        }
    }

    switch(eStateReason)
    {
    /*a 2xx response was received for the invite request*/
    case RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD:
        if(pCallLeg->pMgr->manualAckOn2xx == RV_TRUE)
        {
            /*ack was not automatically sent*/
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED,
                               RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);
        }
        else
        {
            /*ack was sent*/
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_CONNECTED,
                               RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);
            CallLegChangeModifyState(pCallLeg,NULL,
                                    RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
                                    RVSIP_CALL_LEG_REASON_CALL_CONNECTED);
        }
        break;
    /*a 3xx response was received for the invite request*/
    case RVSIP_TRANSC_REASON_RESPONSE_REDIRECTION_RECVD:
        if(pCallLeg->hRedirectAddress != NULL)
        {
            /*Move the new destination to the contact address so that it will
              be the request URI in the next invite*/
            CallLegSetRemoteContactAddress(pCallLeg, pCallLeg->hRedirectAddress);
            pCallLeg->hRedirectAddress = NULL;
        }
        pCallLeg->bInviteReSentInCB = RV_FALSE;
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_REDIRECTED,
                           RVSIP_CALL_LEG_REASON_REDIRECTED);

        /* If application resent the invite synchronicaly, then
        CallLegInviteDetachInviteTransc was already called in the Connect function,
        inside the callback. Otherwise, we should do it here. */
        if(pCallLeg->bInviteReSentInCB == RV_FALSE)
        {
            CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hTransc);
        }
		SipCommonCSeqReset(&pCallLeg->remoteCSeq); 

        break;
    /*-------------------------------------------------------
      4xx,5xx,6xx Responses - Disconnect the call-leg
      ------------------------------------------------------*/
    case RVSIP_TRANSC_REASON_RESPONSE_REQUEST_FAILURE_RECVD: /* 4xx */

#ifdef RV_SIP_AUTH_ON
       if(statusCode == 401 || statusCode == 407)
       {
           return CallLegAuthHandleUnAuthResponse(pCallLeg,hTransc);
       }
#endif /* #ifdef RV_SIP_AUTH_ON */

    case RVSIP_TRANSC_REASON_RESPONSE_SERVER_FAILURE_RECVD:  /* 5xx */
    case RVSIP_TRANSC_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:  /* 6xx */
        {
            RvSipCallLegStateChangeReason eDisconnectReason =
                                    GetCallLegRemoteResponseReason(eStateReason);

            /*make the transaction a stand alone transaction to let is keep
                retransmit ack without the call-leg existence*/
            rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hTransc);
            
            /*disconnect the call*/
            CallLegDisconnectWithNoBye(pCallLeg,eDisconnectReason);
            /*terminate the call*/
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        }
        break;
    /*the reason is not reasonable for this state*/
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInviteFinalResponse - Call-leg 0x%p received invalid transc reason %d for transc 0x%p",
            pCallLeg, eStateReason,hTransc));
        return RV_ERROR_UNKNOWN;
    }  /*END OF:switch(eStateReason)*/

    RV_UNUSED_ARG(pInvite)
    return RV_OK;
}

/***************************************************************************
 * HandleInviteFinalResponseInCancellingState
 * ------------------------------------------------------------------------
 * General: A final response was received for an Invite. Ack is sent and
 *          the call changes state according to the response.
 *          for 4xx,4xx,6xx responses the call will be disconnected
 *          When disconnecting the Invite transaction will be detached from
 *          the call-leg (for ack retransmissions) and the call will terminate.
 * Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
 *                            state.
 *               RV_OK - on success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event.
 *          hTransc - Handle to the related transaction.
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteFinalResponseInCancellingState(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc)
{

    RvStatus rv;
    RvUint16 statusCode;

    statusCode = RvSipMsgGetStatusCode(pCallLeg->hReceivedMsg);

    /*for every response ack should be sent first, if we failed to ACK, there is something
      wrong with the connection - Terminate the call*/

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleInviteFinalResponseInCancellingState - Call-leg 0x%p - received final response for INVITE, sending ACK",
              pCallLeg));


	/*update the 2xx to header in the invite object*/
    /* 1. find the invite object */
    if(statusCode >= 200 || statusCode < 300)
	{
		/* save the server connection from the transaction, because it might receive some more
		2xx retransmissions. */
		rv = CallLegInviteAttachOnInviteConnections(pCallLeg, pInvite, hTransc);
		if(rv != RV_OK)
		{
			RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"HandleInviteFinalResponseInCancellingState - Call-leg 0x%p failed to attach invite 0x%p to Connections",
				pCallLeg, pInvite));
		}
		
		/* save the To header of the 2xx response (we will use it in building the ACK) */
		rv = CallLegInviteCopyToHeaderFromMsg(pCallLeg,pInvite,pCallLeg->hReceivedMsg);
		if(RV_OK != rv)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"HandleInviteFinalResponseInCancellingState - Call-leg 0x%p failed to copy TO header in invite 0x%p",
				pCallLeg, pInvite));
			return rv;
		}
        
	}
    if(pCallLeg->pMgr->manualBehavior == RV_FALSE ||
       (statusCode >= 300))
    {  /* The ACK message will not send automatically by the Stack if the application works with both
           manualAck and manualBehavior and the response is 2xx */
        rv = CallLegInviteSendAck(pCallLeg,hTransc,pInvite,statusCode);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteFinalResponseInCancellingState - Call-leg 0x%p failed to send ACK with transc 0x%p- Terminating call",
                pCallLeg,hTransc));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
    }

    /*if 2xx was received after canceling the invite - send BYE*/
    if(statusCode >=200 && statusCode <300)
    {
        if(pCallLeg->pMgr->manualBehavior == RV_FALSE)
        {
            return CallLegSessionDisconnect(pCallLeg);
        }
        else /* manualBehavior = true */
        {
        /* In this case the ACK wasn't sent and the call moves to the
            remote-accepted state */
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED,
                RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);

            return RV_OK;
        }
    }
    else
    {
        /*make the transaction a stand alone transaction to let is keep
          retransmit ack without the call-leg existence */
        rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, hTransc);

        /*disconnect the call*/
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_CANCELLING);
        /*terminate the call*/
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    }

}


/***************************************************************************
 * HandleReInviteFinalResponse
 * ------------------------------------------------------------------------
 * General: A final response was received for a reInvite. Ack is sent and
 *          the connection sub state returns to idle. the application is
 *          being notified about the response.
 * Return Value: RV_ERROR_UNKNOWN - If the response is not expected in this call
 *                            state.
 *               RV_OK - on success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg that received the transaction event.
 *          hTransc - Handle to the related transaction.
 *            eStateReason - the reason for the final response.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleReInviteFinalResponse(
                            IN  CallLeg                           *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason)
{
    RvStatus rv;
    RvUint16 responseStatusCode;

    RvSipTransactionGetResponseCode(hTransc,&responseStatusCode);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleReInviteFinalResponse - Call-leg 0x%p - received final response for re-INVITE",
              pCallLeg));

    if(pInvite == NULL)
	{
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleReInviteFinalResponse - Call-leg 0x%p. invite object is NULL",
            pCallLeg));
        return RV_ERROR_INVALID_HANDLE;

	}
	
	/*on final response - reset the parameters saved from an incoming reliable 1xx*/
    ResetIncomingRel1xxParamsOnFinalResponse(pCallLeg);

    if(pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT &&
        pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING &&
        pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleReInviteFinalResponse - Call-leg 0x%p. invite 0x%p received re-invalid response in an unexpected state %s",
            pCallLeg,pInvite,CallLegGetModifyStateName(pInvite->eModifyState)));
        return RV_ERROR_UNKNOWN;
    }

    /* save the To header of the 2xx response (we will use it in building the ACK) */
    if(responseStatusCode >= 200 && responseStatusCode < 300)
    {
		rv = CallLegInviteCopyToHeaderFromMsg(pCallLeg,pInvite,pCallLeg->hReceivedMsg);
		
        if(RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteFinalResponse - Call-leg 0x%p failed to copy TO header in invite 0x%p",
                pCallLeg, pInvite));
            return rv;
        }
    }
    
    if(RV_FALSE == pCallLeg->pMgr->bOldInviteHandling &&
        responseStatusCode >= 200 && responseStatusCode < 300)
    {
        /* 200 was received. from this point, application may send another re-invite */
        CallLegResetActiveTransc(pCallLeg);
    }

    CallLegCallbackModifyResultRcvdEv(pCallLeg,responseStatusCode);

    /* Send ACK for final responses: non-2xx or 2xx with auto-ack */
    if(responseStatusCode >= 300 ||
        pCallLeg->pMgr->manualAckOn2xx == RV_FALSE) 
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                                RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_RCVD,
                                GetCallLegRemoteResponseReason(eStateReason));

        if (RVSIP_CALL_LEG_STATE_TERMINATED        == pCallLeg->eState ||
            RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED == pInvite->eModifyState)
        {
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteFinalResponse - Cal-Leg was terminated from StateChange callback. Call-leg 0x%p, Invite 0x%p",
                pCallLeg,pInvite));
            return RV_OK;
        }

        /* If the application created a message in advance use it*/
        if (NULL != pCallLeg->hOutboundMsg)
        {
            rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
            if (RV_OK != rv)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                          "HandleReInviteFinalResponse - Failed to set outbound message for the ack. Call-leg 0x%p, transc 0x%p (rv=%d)"
                          ,pCallLeg,hTransc,rv));
                return RV_ERROR_UNKNOWN;
            }
            pCallLeg->hOutboundMsg = NULL;
        }

        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleReInviteFinalResponse - Call-leg 0x%p, pInvite=0x%p - about to send ACK",
            pCallLeg, pInvite));

        rv = CallLegInviteSendAck(pCallLeg, hTransc, pInvite, responseStatusCode);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleReInviteFinalResponse - Call-leg 0x%p failed to send ACK with transc 0x%p- Terminating call",
                pCallLeg,hTransc));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }

        /* if this is not 2xx we still have an active transaction. we must reset it here, so
           the application will be able to send another re-invite from the callback,  
           state RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT.
           (the transc is also reset when the trx indicates of message sent, but for TCP
            it is too late...) */
        CallLegResetActiveTransc(pCallLeg);

#ifdef RV_SIP_AUTH_ON
        if(responseStatusCode == 401 || responseStatusCode == 407)
        {
            RvBool bValid;
            RvSipCallLegStateChangeReason eReason;

            rv = SipAuthenticatorCheckValidityOfAuthList(
                pCallLeg->pMgr->hAuthMgr,pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj, &bValid);
            if (RV_OK != rv)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleReInviteFinalResponse - Call-leg 0x%p - failed to check Authentication data validity (rv=%d:%s)",
                    pCallLeg, rv, SipCommonStatus2Str(rv)));
                CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
                CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
                return rv;
            }

            if (RV_TRUE == bValid)
            {
                eReason = RVSIP_CALL_LEG_REASON_AUTH_NEEDED;
            }
            else
            {
                eReason = RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS;
            }
            CallLegChangeModifyState(pCallLeg, pInvite,
                                    RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
                                    eReason);
            return RV_OK;
        }
#endif /* #ifdef RV_SIP_AUTH_ON */
        CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT,
                                 RVSIP_CALL_LEG_REASON_ACK_SENT);
    }

    /* 2xx with manual ack */
    if(responseStatusCode>=200 && responseStatusCode < 300 &&
        pCallLeg->pMgr->manualAckOn2xx == RV_TRUE) 
    {
        /*ack will be sent by the application*/
        CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED,
                                 RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED);
    }
    return RV_OK;
}


/***************************************************************************
 * GetCallLegRemoteResponseReason
 * ------------------------------------------------------------------------
 * General: Returns a call-leg state change reason for a
 *          corresponds a transaction reason.
 * Return Value: The call-leg state change reason
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eTranscReason - The transaction disconnect reason.
 ***************************************************************************/
static RvSipCallLegStateChangeReason RVCALLCONV GetCallLegRemoteResponseReason(
                                           IN  RvSipTransactionStateChangeReason eTranscReason)
{
    switch(eTranscReason)
    {
    case RVSIP_TRANSC_REASON_RESPONSE_REDIRECTION_RECVD:
        return RVSIP_CALL_LEG_REASON_REDIRECTED;
    case RVSIP_TRANSC_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
        return RVSIP_CALL_LEG_REASON_REQUEST_FAILURE;
    case RVSIP_TRANSC_REASON_RESPONSE_SERVER_FAILURE_RECVD:
        return RVSIP_CALL_LEG_REASON_SERVER_FAILURE;
    case RVSIP_TRANSC_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
        return RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE;
    case RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD:
        return RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED;
    default:
        return RVSIP_CALL_LEG_REASON_DISCONNECTED;
    }
}

/***************************************************************************
 * HandleInviteProvisionalResponseRecvd
 * ------------------------------------------------------------------------
 * General: Handles receipt of a provisional response to previously sent
 *          INVITE request.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - A pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV HandleInviteProvisionalResponseRecvd(
                            IN  CallLeg                *pCallLeg,
                            IN  CallLegInvite*          pInvite)
{

    if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_PROCEEDING,
                           RVSIP_CALL_LEG_REASON_REMOTE_PROVISIONAL_RESP);
    }
    else if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING,
                                 RVSIP_CALL_LEG_REASON_REMOTE_PROVISIONAL_RESP);
    }

    return RV_OK;
}

/***************************************************************************
 * HandleRelProvRespRcvd
 * ------------------------------------------------------------------------
 * General:  Called when a reliable provisional response is received.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - A pointer to new transaction.
 *          hCallLeg - The transactions owner.
 ***************************************************************************/
RvStatus RVCALLCONV HandleRelProvRespRcvd (
                      IN    CallLeg           *pCallLeg,
                      IN    RvUint32           rseqResponseNum,
                      IN    RvSipMsgHandle     hMsg)
{
    RvInt32                   cseqStep;
    RvStatus                  rv = RV_OK;

    /* Check if the Call-leg wasn't terminated */
    if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleRelProvRespRcvd: call-leg 0x%p - is already terminated. PRACK won't be sent",
            pCallLeg));
        return RV_OK;
    }
    /* check of the RSeq value is done before, on taking decision whether to
       ignore this reliable response or not */

    rv = GetCseqStepFromMsg(pCallLeg, hMsg,&cseqStep);
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleRelProvRespRcvd: call-leg 0x%p - failed to retrieve CSeq from msg 0x%p",
            pCallLeg,hMsg));
        return rv;
	}

    /* save parameters from 1xx in call-leg */
	CallLegSetIncomingRSeq(pCallLeg,rseqResponseNum); 
    
	rv = SipCommonCSeqSetStep(cseqStep,&pCallLeg->incomingRel1xxCseq); 
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleRelProvRespRcvd: call-leg 0x%p - failed to store 1xx cseq",
            pCallLeg));

        return rv;
	}

    CopyTimestampFromMsg(pCallLeg, hMsg);

    if(pCallLeg->pMgr->manualPrack == RV_TRUE)
    {
        CallLegCallbackPrackStateChangedEv(pCallLeg,
            RVSIP_CALL_LEG_PRACK_STATE_REL_PROV_RESPONSE_RCVD,
            RVSIP_CALL_LEG_REASON_UNDEFINED,-1,NULL);
    }
    else
    {
        rv = CallLegSessionSendPrack(pCallLeg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleRelProvRespRcvd: call-leg 0x%p - Failed to send PRACK request. (rv=%d)",
                pCallLeg,rv));
            return rv;
        }
    }
    return RV_OK;
}

/*------------------------------------------------------------------------
    I N V I T E    F I N A L   R E S P O N S E    S E N T
  ------------------------------------------------------------------------*/

/***************************************************************************
* HandleTranscStateInviteFinalResponseSent
* ------------------------------------------------------------------------
* General: Handles only final responses that are sent directly by the
*          transaction (such as responses to cancel)
* Return Value: The call-leg state change reason
* ------------------------------------------------------------------------
* Arguments:
* Input:     eTranscReason - The transaction disconnect reason.
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateInviteFinalResponseSent(
                                    IN  CallLeg                           *pCallLeg,
                                    IN  CallLegInvite*                    pInvite,
                                    IN  RvSipTranscHandle                 hTransc,
                                    IN  RvSipTransactionStateChangeReason eStateReason)
{

    RvStatus rv = RV_OK;
    RvUint16 responseStatusCode;
    RvSipTransactionGetResponseCode(hTransc,&responseStatusCode);

    if(eStateReason == RVSIP_TRANSC_REASON_TRANSACTION_CANCELED)
    {
        if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING) ||
           (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLED))
        {
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                        RVSIP_CALL_LEG_REASON_REMOTE_CANCELED);
        }
        else if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                                     RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                                     RVSIP_CALL_LEG_REASON_REMOTE_CANCELED);
        }
    }
    /* If the stack supports session timer, the session timer parameters need to be handles*/
    if (pCallLeg->pMgr->eSessiontimeStatus
        == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED &&
        eStateReason != RVSIP_TRANSC_REASON_TRANSACTION_CANCELED)
    {	
        rv = CallLegSessionTimerHandleFinalResponseSend(
					pCallLeg,hTransc,NULL /* Fix- instead of the irrelevant pCallLeg->hOutboundMsg */);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscStateInviteFinalResponseSent - Call-Leg 0x%p\
            Failed in CallLegSessionTimerHandleFinalResponseSend"
            ,pCallLeg));
            return rv;
        }
        if (responseStatusCode >=200 && responseStatusCode <300)
        {
            rv = CallLegSessionTimerHandleTimers(
					pCallLeg,hTransc,NULL /* Fix- instead of the irrelevant pCallLeg->hOutboundMsg */,RV_FALSE);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscStateInviteFinalResponseSent - Call-Leg 0x%p\
                Failed in CallLegSessionTimerHandleTimers"
                ,pCallLeg));
                return rv;
             }
        }
    }
    if((responseStatusCode == 500) &&
       (eStateReason == RVSIP_TRANSC_REASON_REL_PROV_RESP_TIME_OUT))
    {
        if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscStateInviteFinalResponseSent - Call-leg 0x%p - Time out in state %s - Terminating call",
                pCallLeg,CallLegGetStateName(pCallLeg->eState)));

            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                RVSIP_CALL_LEG_REASON_LOCAL_DISCONNECTING);
        }
        else /* re-Invite */
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        }
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_FALSE &&
        (responseStatusCode >= 200 && responseStatusCode < 300))
    {
        /* new invite - the invite transaction is not relevant after the 200 was sent */
        CallLegResetActiveTransc(pCallLeg);
    }
    
    
    return RV_OK;
}


/*------------------------------------------------------------------------
G E N E R A L  F I N A L   R E S P O N S E    S E N T
-------------------------------------------------------------------------*/

/***************************************************************************
* HandleTranscStateGeneralFinalResponseSent
* ------------------------------------------------------------------------
* General: Handles only final responses that are sent directly by the
*          transaction.
* Return Value: The call-leg state change reason
* ------------------------------------------------------------------------
* Arguments:
* Input:     eTranscReason - The transaction disconnect reason.
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateGeneralFinalResponseSent(
                                  IN  CallLeg                           *pCallLeg,
                                  IN    RvSipTranscHandle                hTransc,
                                  IN  RvSipTransactionStateChangeReason  eStateReason)

{
    SipTransactionMethod eMethod;
    RvStatus             rv = RV_OK;
    RvUint16             responseCode          = 0;
    RvChar               strMethodBuffer[SIP_METHOD_LEN]   = {0};
    RvSipTransaction100RelStatus e100RelStatus =
                    SipTransactionMgrGet100relStatus(pCallLeg->pMgr->hTranscMgr);


    RvSipTransactionGetMethodStr(hTransc,SIP_METHOD_LEN,strMethodBuffer);
    SipTransactionGetResponseCode(hTransc,&responseCode);
    SipTransactionGetMethod(hTransc,&eMethod);
       RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscStateGeneralFinalResponseSent - Call-leg 0x%p - received transaction final response sent state",
              pCallLeg));


    if(eMethod == SIP_TRANSACTION_METHOD_PRACK && e100RelStatus != RVSIP_TRANSC_100_REL_UNDEFINED)
    {
        if(hTransc == pCallLeg->hActiveIncomingPrack)
        {
             CallLegCallbackPrackStateChangedEv(pCallLeg,RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_SENT,
                                                RVSIP_CALL_LEG_REASON_UNDEFINED,(RvInt16)responseCode,hTransc);
             pCallLeg->hActiveIncomingPrack = NULL;
        }
    }
    if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
    {
        /* Settle the Session-Timer mechanism (MUST be done before StateChangedEv is called) */
        if (CallLegIsUpdateTransc(hTransc)     == RV_TRUE &&
            pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED &&
            eStateReason != RVSIP_TRANSC_REASON_TRANSACTION_CANCELED)
        {			
            rv = CallLegSessionTimerHandleFinalResponseSend(
							pCallLeg,hTransc,NULL /* Fix- instead of the irrelevant pCallLeg->hOutboundMsg */);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscStateGeneralFinalResponseSent - Call-Leg 0x%p hTransc 0x%p - \
                Failed in CallLegSessionTimerHandleGeneralFinalResponseSend"
                ,pCallLeg,hTransc));
                return rv;
            }
            if (responseCode >=200 && responseCode <300)
            {
                rv = CallLegSessionTimerHandleTimers(
							pCallLeg,hTransc,NULL /* Fix- instead of the irrelevant pCallLeg->hOutboundMsg */,RV_FALSE);
                if(rv != RV_OK)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleTranscStateGeneralFinalResponseSent - Call-Leg 0x%p \
                    Failed in CallLegSessionTimerHandleTimers"
                    ,pCallLeg));
                    return rv;
                }
            }
        }
        /* Update the application of the changed state */
        if(eMethod != SIP_TRANSACTION_METHOD_BYE)
        {
            CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                           hTransc,
                                           RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT,
                                           eStateReason);
        }
        else
        {
                CallLegCallbackByeStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                                hTransc,
                                                RVSIP_CALL_LEG_BYE_STATE_RESPONSE_SENT,
                                                eStateReason);
        }
        return RV_OK;
    }
    return RV_OK;
}
/*------------------------------------------------------------------------
G E N E R A L   R E S P O N S E   R E C E I V E D
-------------------------------------------------------------------------*/

/***************************************************************************
* HandleTranscStateGeneralFinalResponseRcvd
* ------------------------------------------------------------------------
* General: This function is called when a general transaction receives final
*          response.
* Return Value: RV_OK on success and RV_ERROR_UNKNOWN on error.
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - The call-leg related to the terminated transaction
*          hTransc - Handle to the transaction.
*            eStateReason - The termination reason.
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateGeneralFinalResponseRcvd(
                                         IN  CallLeg                           *pCallLeg,
                                         IN    RvSipTranscHandle                 hTransc,
                                         IN  RvSipTransactionStateChangeReason eStateReason)
{
    RvStatus rv = RV_OK;
    SipTransactionMethod eMethod;
    RvUint16 responseStatusCode;
    RvChar     strMethodBuffer[SIP_METHOD_LEN] = {0};
    RvSipTransactionGetMethodStr(hTransc,SIP_METHOD_LEN,strMethodBuffer);
    RvSipTransactionGetResponseCode(hTransc,&responseStatusCode);
    SipTransactionGetMethod(hTransc,&eMethod);
    if (SIP_TRANSACTION_METHOD_BYE == eMethod)
    {
        return HandleByeResponseRcvd(pCallLeg,hTransc);
    }


    switch(eStateReason)
    {
    case RVSIP_TRANSC_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_SERVER_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_REDIRECTION_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD:
        /* If the stack supports session timer, the session timer parameters need to be handles*/
        if (CallLegIsUpdateTransc(hTransc) == RV_TRUE  &&
            pCallLeg->pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED)
        {
			RvSipMsgHandle hMsg;
			
			/* Get the sent message*/
			rv = RvSipTransactionGetReceivedMsg(hTransc,&hMsg);
			if(rv != RV_OK)
			{
				RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"HandleTranscStateGeneralFinalResponseRcvd - Call-leg 0x%p - Failed to retrieve Msg handle of transc 0x%p (rv=%d)",
					pCallLeg,hTransc,rv));
				return rv;		
			}

            rv = CallLegSessionTimerHandleFinalResponseRcvd(pCallLeg,hTransc,hMsg);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			       "HandleTranscStateGeneralFinalResponseRcvd - Call-leg 0x%p\
		           Failed in CallLegSessionTimerHandleFinalResponseRcvd",
		           pCallLeg));
                return rv;

            }
            if (responseStatusCode >=200 && responseStatusCode <300)
            {
                rv = CallLegSessionTimerHandleTimers(pCallLeg,hTransc,hMsg,RV_TRUE);
                if(rv != RV_OK)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleTranscStateGeneralFinalResponseRcvd - Call-Leg 0x%p \
                    Failed in CallLegSessionTimerHandleTimers"
                    ,pCallLeg));
                    return rv;
                }
            }
        }
        /*if the application initiated the transaction give it the response*/
        if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
        {
            if(pCallLeg->callLegEvHandlers != NULL &&
               pCallLeg->callLegEvHandlers->pfnTranscStateChangedEvHandler != NULL)
            {
                CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                                    hTransc,
                                                    RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD,
                                                    eStateReason);
            }
            else
            {
                CallLegCallbackTranscResolvedEv((RvSipCallLegHandle)pCallLeg,
                                            hTransc,
                                            RVSIP_CALL_LEG_TRANSC_STATUS_RESPONSE_RCVD,
                                            responseStatusCode);
            }

        return rv;
        }
        if(eMethod == SIP_TRANSACTION_METHOD_PRACK)
        {
            CallLegCallbackPrackStateChangedEv(pCallLeg,
                                    RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD,
                                    RVSIP_CALL_LEG_REASON_UNDEFINED,responseStatusCode,hTransc);
#ifdef RV_SIP_AUTH_ON
            /* quick patch for MTF - PRACK authentication */
            if((responseStatusCode == 401 || responseStatusCode == 407) &&
               pCallLeg->callLegEvHandlers->pfnPrackStateChangedEvEvHandler == NULL &&
               RV_FALSE == pCallLeg->pMgr->manualPrack)
            {
                RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleTranscStateGeneralFinalResponseRcvd - Call-leg 0x%p, transc 0x%p, authenticating PRACK request",
                  pCallLeg,hTransc));
                rv = CallLegSessionSendPrack(pCallLeg);
            }
#endif /*#ifdef RV_SIP_AUTH_ON*/
        }
        break;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscStateGeneralFinalResponseRcvd - Call-leg 0x%p, transc 0x%p,Invalid transc reason %d",
                  pCallLeg,hTransc,eStateReason));
        rv = RV_ERROR_UNKNOWN;
    }
    return rv;
}

/*-------------------------------------------------------------------------
      T R A N S A C T I O N     T E R M I N A T E D
 --------------------------------------------------------------------------*/

/***************************************************************************
 * HandleTranscStateTerminated
 * ------------------------------------------------------------------------
 * General: A transaction can terminate for several reasons:
 *          1. Normal termination
 *          2. Ack for Invite was received
 *          3. Final response for bye was received
 *          4. Time out (an expected message never arrived)
 *          In all cases the transaction in removed from the call-leg transc
 *          list and is no longer the active transaction.
 * Return Value: RV_OK on success and RV_ERROR_UNKNOWN on error.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg related to the terminated transaction
 *          hTransc - Handle to the transaction.
 *            eStateReason - The termination reason.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateTerminated(
                            IN  CallLeg                          *pCallLeg,
                            IN  CallLegInvite*                    pInvite,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason)

{
    RvStatus            rv = RV_OK;
    SipTransactionMethod eMethod;
    RvUint16            responseStatusCode;
    RvBool              bTerminateCallLeg = RV_FALSE;

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        bTerminateCallLeg = RV_TRUE;
    }

    RvSipTransactionGetResponseCode(hTransc,&responseStatusCode);
    SipTransactionGetMethod(hTransc,&eMethod);

    /*first handle application transactions*/
    if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
    {
        rv = HandleAppTranscStateTerminated(pCallLeg,hTransc,eStateReason);
        if(rv != RV_OK)
        {
            return rv;
        }

        if(bTerminateCallLeg == RV_TRUE)
        {
            CallLegTerminateIfPossible(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        }
        return RV_OK;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscStateTerminated - Call-leg 0x%p - stack transaction terminated, removing from list",
              pCallLeg));

    
    if(pInvite == NULL)
    {
        /*--------------------------------------
        remove general transaction from transaction list
        -----------------------------------------*/
        rv = CallLegRemoveTranscFromList(pCallLeg,hTransc);

        if(rv != RV_OK)  /*transaction not found in call*/
            return RV_ERROR_UNKNOWN;
    }
    if(hTransc == pCallLeg->hActiveTransc)
    {
        CallLegResetActiveTransc(pCallLeg);
    }
    
    /* if this is an invite transaction - delete it from the invite object */
    if(pInvite != NULL)
    {
        RvBool bTimeout = (eStateReason == RVSIP_TRANSC_REASON_TIME_OUT)?RV_TRUE:RV_FALSE;
        CallLegInviteResetTranscInInviteObj(pCallLeg, pInvite,bTimeout);
    }
    
    switch(eStateReason)
    {
   /*--------------------------------------------------------------------------
     general final response - handled in the general final response rcvd state.
     --------------------------------------------------------------------------*/
    case RVSIP_TRANSC_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_SERVER_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_REDIRECTION_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
    case RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD:
    case RVSIP_TRANSC_REASON_CONTINUE_DNS:
    case RVSIP_TRANSC_REASON_USER_COMMAND:
    case RVSIP_TRANSC_REASON_REL_PROV_RESP_TIME_OUT:
        break;

    /*-----------------------------------------------------------------
     Transaction terminated because of time out.
     ------------------------------------------------------------------*/
    case RVSIP_TRANSC_REASON_TIME_OUT:
        rv = HandleTranscTimeOut(pCallLeg,pInvite, hTransc);
        break;
    case RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS:
        rv = HandleTranscRanOutOfTimers(pCallLeg,hTransc);
        break;
    case RVSIP_TRANSC_REASON_NORMAL_TERMINATION:
        break;
    case RVSIP_TRANSC_REASON_ERROR:
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
    case RVSIP_TRANSC_REASON_OUT_OF_RESOURCES:
        rv = HandleTranscErrorTermination(pCallLeg,hTransc);
        break;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscStateTerminated - Call-leg 0x%p, transc 0x%p,Invalid transc reason %d",
                  pCallLeg,hTransc,eStateReason));
        rv = RV_ERROR_UNKNOWN;
    }

    if(bTerminateCallLeg == RV_TRUE)
    {
        CallLegTerminateIfPossible(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    }
    return rv;
}


/***************************************************************************
 * HandleAppTranscStateTerminated
 * ------------------------------------------------------------------------
 * General: Handle the termination of transaction that the application
 *          is responsible. Remove the transaction from the transaction list
 *          and notify the application that the transaction was terminated
 * Return Value: RV_OK on success and RV_ERROR_UNKNOWN on error.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg related to the terminated transaction
 *          hTransc - Handle to the transaction.
 *            eStateReason - The termination reason.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleAppTranscStateTerminated(
                            IN  CallLeg                           *pCallLeg,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  RvSipTransactionStateChangeReason eStateReason)
{

    RvStatus rv;
    SipTransactionMethod eMethod;
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "HandleAppTranscStateTerminated - application transaction was terminated - call-leg 0x%p, transc 0x%p",
              pCallLeg,hTransc));
    rv = CallLegRemoveTranscFromList(pCallLeg,hTransc);
    if(rv != RV_OK)  /*transaction not found in call*/
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleAppTranscStateTerminated - Call-leg 0x%p, transc 0x%p,failed to remove from list",
                  pCallLeg,hTransc));
        return RV_ERROR_UNKNOWN;
    }
    /* check is the transaction is BYE transaction */
    SipTransactionGetMethod(hTransc, &eMethod);
    if(eMethod == SIP_TRANSACTION_METHOD_BYE)
    {
        CallLegCallbackByeStateChangedEv((RvSipCallLegHandle)pCallLeg,
                                        hTransc,
                                        RVSIP_CALL_LEG_BYE_STATE_TERMINATED,
                                        eStateReason);

        return RV_OK;
    }

    if(pCallLeg->callLegEvHandlers != NULL &&
        pCallLeg->callLegEvHandlers->pfnTranscStateChangedEvHandler != NULL)
    {
        CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,hTransc,
            RVSIP_CALL_LEG_TRANSC_STATE_TERMINATED,
            eStateReason);
    }
    else /*this part is for backwards compatibility - the resolved callback is obsolete*/
    {
        if(eStateReason == RVSIP_TRANSC_REASON_TIME_OUT)
        {
            /*the time out is on a transaction created by the application*/
            CallLegCallbackTranscResolvedEv((RvSipCallLegHandle)pCallLeg,
                hTransc,
                RVSIP_CALL_LEG_TRANSC_STATUS_TIME_OUT,
                0);
        }
    }
    return RV_OK;
}


/***************************************************************************
 * HandleTranscStateCancelFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: A final response was received to a cancel request.
 *          If the response is 481 terminate the call if it is on the
 *          canceling state.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call-leg this transaction belongs to.
 *          hTransc - handle to the transaction.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateCancelFinalResponseRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc)
{
    RvUint16 statusCode;

    RvSipTransactionGetResponseCode(hTransc,&statusCode);
     RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "HandleTranscStateCancelFinalResponseRcvd - Call-leg 0x%p - %d response received for cancel",
                      pCallLeg,statusCode));

    if(statusCode == 481)
    {        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "HandleTranscStateCancelFinalResponseRcvd - Call-leg 0x%p - 481 response received for cancel, terminating the call",
                  pCallLeg));

        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_REQUEST_FAILURE);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    }
    return RV_OK;
}


/***************************************************************************
 * HandleByeResponseRcv
 * ------------------------------------------------------------------------
 * General: A final response was received to a bye request.
 *          first the bye transaction is removed from the transaction list,
 *          then  all pending transactions are terminated.
 *          finally the call is terminated.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call-leg this transaction belongs to.
 *          hTransc - handle to the transaction.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleByeResponseRcvd(
                            IN  CallLeg                           *pCallLeg,
                            IN    RvSipTranscHandle                 hTransc)
{
    RvUint16 statusCode;

    RvSipTransactionGetResponseCode(hTransc,&statusCode);
    if(statusCode < 200 || statusCode >= 700)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleByeResponseRcvd - Call-leg 0x%p - statusCode is Illegal",
                  pCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleByeResponseRcvd - Call-leg 0x%p - received a response for BYE",
              pCallLeg));

#ifdef RV_SIP_AUTH_ON
   if(statusCode == 401 || statusCode == 407)
   {
       return CallLegAuthHandleUnAuthResponse(pCallLeg,hTransc);
   }
#endif /* #ifdef RV_SIP_AUTH_ON */
   
    /* detach the current Bye transaction, so it will use the T4 timer */
    CallLegRemoveTranscFromList(pCallLeg,hTransc);
    if(SipTransactionDetachOwner(hTransc) != RV_OK)
    {
       RvSipTransactionTerminate(hTransc);
    }

    CallLegTerminateAllGeneralTransc(pCallLeg);
    
    
    CallLegTerminateAllInvites(pCallLeg, RV_FALSE);
    CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                            RVSIP_CALL_LEG_REASON_DISCONNECTED);
    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);

    return RV_OK;
}

/***************************************************************************
 * HandleTranscInviteAckRcvd
 * ------------------------------------------------------------------------
 * General: This function is called when an invite transaction receives an
 *          Ack. This function changes the call state.
 * Return Value: RV_ERROR_UNKNOWN - If the transaction is not part of the call
 *                            transaction list.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - call-leg this transaction belongs to.
 *          hTransc - handle to the transaction
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscInviteAckRcvd(
                            IN  CallLeg                  *pCallLeg,
                            IN  CallLegInvite*            pInvite,
                            IN  RvSipTranscHandle         hTransc)

{

    if(hTransc != pCallLeg->hActiveTransc && NULL != pCallLeg->hActiveTransc)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscInviteAckRcvd - Call-leg 0x%p - received ACK on a transaction that is not the active transaction. ignored",
              pCallLeg));
        return RV_OK;
    }

    if(NULL == pInvite)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscInviteAckRcvd - call-leg 0x%p - No invite obj was found for Transc 0x%p. ignoring",
            pCallLeg,hTransc));
        return RV_OK;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscInviteAckRcvd - Call-leg 0x%p Invite 0x%p - received ACK",
              pCallLeg, pInvite));

    /*change the state of the call*/
    if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED) /* this is the initial invite transaction */
    {
        switch(pCallLeg->eState)
        {
        case RVSIP_CALL_LEG_STATE_ACCEPTED:
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_CONNECTED,
                                    RVSIP_CALL_LEG_REASON_REMOTE_ACK);
            /* after informing the connected state, we can reset the active transaction. 
               we do it here, so in the ModifyStateChange callback, the active transaction 
               will be empty.*/
            CallLegResetActiveTransc(pCallLeg);
            CallLegChangeModifyState(
                                  pCallLeg, NULL,
                                  RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
                                  RVSIP_CALL_LEG_REASON_CALL_CONNECTED);
            break;
        case RVSIP_CALL_LEG_STATE_DISCONNECTED:
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            break;
        default:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscInviteAckRcvd - Call-leg 0x%p Invite 0x%p - nothing to do with ACK on Transc 0x%p on state %s",
                  pCallLeg, pInvite, hTransc, CallLegGetStateName(pCallLeg->eState)));
            return RV_OK;
        }
    }
    else if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT)
    {
        CallLegChangeModifyState(pCallLeg, pInvite,
                          RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD,
                          RVSIP_CALL_LEG_REASON_REMOTE_ACK);
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscInviteAckRcvd - Call-leg 0x%p Invite 0x%p - Terminating invite object...",
            pCallLeg, pInvite));
        CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_REMOTE_ACK);
    }
    else
    {
         RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscInviteAckRcvd - Call-leg 0x%p Invite 0x%p - nothing to do with ACK on Transc 0x%p on invite state %s",
                  pCallLeg, pInvite, hTransc, CallLegGetModifyStateName(pInvite->eModifyState)));
         
         return RV_OK;
    }
    /* reset the active transaction only if it is still the transaction we handle.
       O.W: this is a bug fix for the following case: 
       old-invite, in the MODIFY_IDLE state the application sent re-invite (from cb).
       in this case, the active transc t this point will be the re-invite transc, and not the
       initial invite transc. */
    if(hTransc == pCallLeg->hActiveTransc)
    {
        CallLegResetActiveTransc(pCallLeg);
    }
    return RV_OK;
}


/***************************************************************************
 * HandleAckNoTranscRequest
 * ------------------------------------------------------------------------
 * General: This function is called when an ACK request was received, but the
 *          invite transaction was already destructed.
 * Return Value: RV_ERROR_UNKNOWN - If the transaction is not part of the call
 *                            transaction list.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg      - call-leg this ACK belongs to.
 *          hAckMsg       - handle to the ACK message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleAckNoTranscRequest(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipMsgHandle         hAckMsg,
							IN   SipTransportAddress  *pRcvdFromAddr)
{
    RvInt32 cseqStep;
    CallLegInvite* pInvite;
    RvStatus rv;

	/* Copy received from address to call-leg */
	CallLegSaveReceivedFromAddr(pCallLeg, pRcvdFromAddr);

    rv = GetCseqStepFromMsg(pCallLeg, hAckMsg,&cseqStep);
	if (rv != RV_OK)
	{
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleAckNoTranscRequest - Call-leg 0x%p - failed to retrieve CSeq for ack msg 0x%p",
            pCallLeg,hAckMsg));
		return RV_OK;
	}
    
    rv = CallLegInviteFindObjByCseq(pCallLeg, cseqStep, RVSIP_CALL_LEG_DIRECTION_INCOMING, &pInvite);
    if(rv != RV_OK || NULL == pInvite)
    {
        /* invite object was not found - may be already terminated (if this is a 
           second ACK... )*/
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleAckNoTranscRequest - Call-leg 0x%p - did not find invite object.",
            pCallLeg));
        return RV_OK;
    }
    
    /* in case this is an invalid ACK on non-2xx, that had a wrong via branch,
       we failed to find the transaction, and therefore reached this code.
       in this case - the INVITE transaction still exists, and we can check 
       whether its final response was non-2xx. 
       in this case we know that this is a non-valid ACK, and we should ignore it */
    if(pInvite->hInviteTransc != NULL)
    {
        RvUint16 responseCode = 0;
        RvSipTransactionGetResponseCode(pInvite->hInviteTransc, &responseCode);
        if(responseCode >=300 && responseCode <700)
        {
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleAckNoTranscRequest - Call-leg 0x%p Invite 0x%p - Got an ACK with no transc, after sending non-2xx. ignore the ACK",
                pCallLeg, pInvite));
            return RV_OK;
        }
    }

    if(pInvite->bInitialInvite == RV_TRUE)
    {
        /* we might get an ACK on 482, that was sent with no transaction.
           we will ignore it */
        if(RVSIP_CALL_LEG_STATE_OFFERING == pCallLeg->eState)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleAckNoTranscRequest - Call-leg 0x%p - ACK on state OFFERING. (482?). ignore",
                pCallLeg));
            return RV_OK;
        }
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "HandleAckNoTranscRequest - Call-leg 0x%p Invite 0x%p - received ACK",
        pCallLeg, pInvite));
    
    /* release the 2xx timer */
    CallLegInviteTimerRelease(pInvite);

    pCallLeg->hReceivedMsg = hAckMsg;

    /* There is no transaction, so we must call the msgReceived event handling from here.
       we shall do it only for first ACK, and not for every retransmission */
    if(pInvite->bFirstAck == RV_TRUE)
    {
        rv = CallLegMsgAckMsgRcvdHandler(pCallLeg, hAckMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "HandleAckNoTranscRequest - Call-leg 0x%p, Failed in CallLegMsgAckMsgRcvdHandler. rv=%d",
               pCallLeg, rv));
            pCallLeg->hReceivedMsg = NULL;
            return rv;
        }
        pInvite->bFirstAck = RV_FALSE;
    }
    
    if(pInvite->bInitialInvite == RV_TRUE)
    {
        /* This is an initial invite object - change the state of the call*/
        switch(pCallLeg->eState)
        {
        case RVSIP_CALL_LEG_STATE_ACCEPTED:
            CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_CONNECTED,
                                        RVSIP_CALL_LEG_REASON_REMOTE_ACK);
            break;
        case RVSIP_CALL_LEG_STATE_DISCONNECTED:
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            break;
        case RVSIP_CALL_LEG_STATE_CONNECTED:
            /* do nothing */
            break;
        case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE:
            /* do nothing - Ack came too late. timer was already expired, and BYE was already sent*/
            break;
        case RVSIP_CALL_LEG_STATE_OFFERING:
            /* probably a case of ACK on 482 that was sent without a transaction */
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleAckNoTranscRequest - Call-leg 0x%p Invite 0x%p - Ack received on call state %s. (probably after 482)",
                pCallLeg, pInvite, CallLegGetStateName(pCallLeg->eState)));
            pCallLeg->hReceivedMsg = NULL;
            return RV_OK;
        default:
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleAckNoTranscRequest - Call-leg 0x%p Invite 0x%p - Ack received on call state %s",
                pCallLeg, pInvite, CallLegGetStateName(pCallLeg->eState)));
            pCallLeg->hReceivedMsg = NULL;
            return RV_OK;
        }
    }
    else
    {
        /* This is a re-invite object - change the state of the modify*/
        if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT)
        {
            CallLegChangeModifyState(pCallLeg, pInvite,
                                    RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD,
                                    RVSIP_CALL_LEG_REASON_REMOTE_ACK);
        }
    }
    
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "HandleAckNoTranscRequest - Call-leg 0x%p Invite 0x%p - Terminating invite object...",
        pCallLeg, pInvite));
    
    pCallLeg->hReceivedMsg = NULL;

    CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_REMOTE_ACK);
    return RV_OK;
}

/***************************************************************************
 * HandleTranscTimeOut
 * ------------------------------------------------------------------------
 * General: This function is called when a transaction is terminated because
 *          There was a timeout.
 *          For invite and bye method this will cause the call to disconnect.
 *          For reInvite the call will return to connect/idle.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - the call-leg related to this transaction.
 *          hTransc - the transaction handle
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscTimeOut(
                            IN  CallLeg                 *pCallLeg,
                            IN  CallLegInvite*          pInvite,
                            IN  RvSipTranscHandle       hTransc)

{
    SipTransactionMethod eMethod;

    SipTransactionGetMethod(hTransc,&eMethod);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleTranscTimeOut - Call-leg 0x%p - Transaction 0x%p timeout on state %s",
                       pCallLeg,hTransc,CallLegGetStateName(pCallLeg->eState)));

    /*------------------
      Invite time-out
      -----------------*/
    if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
    {
        if(NULL == pInvite)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscTimeOut - Call-leg 0x%p - Transaction 0x%p no invite object was given",
                pCallLeg,hTransc));
            return RV_ERROR_UNKNOWN;
        }
        switch(pCallLeg->eState)
        {
        /*-----------------------------------------------------------
        time out on state accept and inviting will disconnect the call
        ------------------------------------------------------------*/
        case RVSIP_CALL_LEG_STATE_OFFERING: /*will happen only when the provisional is retransmitted (PRACK)*/
        case RVSIP_CALL_LEG_STATE_DISCONNECTED: /*waiting for ack after reject*/
        case RVSIP_CALL_LEG_STATE_DISCONNECTING: /*invite transaction is alive but bye was sent*/
        case RVSIP_CALL_LEG_STATE_INVITING:
        case RVSIP_CALL_LEG_STATE_PROCEEDING:
        case RVSIP_CALL_LEG_STATE_CANCELLING:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleTranscTimeOut - Call-leg 0x%p - Time out in state %s - Terminating call",
                       pCallLeg,CallLegGetStateName(pCallLeg->eState)));
            CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);

            break;
        /*----------------------------------------------------------------------
        time out on state connected is a reInvite timeout - back to connect/idle
        ------------------------------------------------------------------------*/
        case RVSIP_CALL_LEG_STATE_CONNECTED:  /*a time out on a sent reInvite*/
            if (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT ||
                pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT ||
                pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING ||
                pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING)
            {
                RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "HandleTranscTimeOut - Call-leg 0x%p - Time out on re-Invite 0x%p - call-leg will disconnect",
                       pCallLeg, pInvite));
                if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
                {
                    /* in old invite the state idle was called before the BYE was sent.
                       in order to keep this behavior, we change the state here, and mark the
                       invite object to not inform of idle state. */
                    CallLegChangeModifyState(pCallLeg, pInvite,
                                             RVSIP_CALL_LEG_MODIFY_STATE_IDLE,
                                             RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
                    pInvite->bModifyIdleInOldInvite = RV_TRUE;
                }
                CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
                return CallLegSessionDisconnect(pCallLeg);
            }
             /*this is the case that we have timeout on a reliable provisional response*/
          /*
            if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD)
          {
              RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscTimeOut - Call-leg 0x%p - Time out on re-Invite 0x%p reliable provisional response",
                  pCallLeg, pInvite));
              CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
          }*/
          
            break;
        case RVSIP_CALL_LEG_STATE_ACCEPTED: /*waiting for ack to server invite*/
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "HandleTranscTimeOut - Call-leg 0x%p - Time out on Invite transaction- calling disconnect",
                        pCallLeg));
            return CallLegSessionDisconnect(pCallLeg);
        default:
            break;
        }
        return RV_OK;
    }
    /*-----------------------
        bye time out
      ----------------------*/
    if(eMethod == SIP_TRANSACTION_METHOD_BYE &&
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING) /*waiting for final response to bye*/
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "HandleTranscTimeOut - Call-leg 0x%p - Time out on BYE - Terminating call",
                   pCallLeg));
        if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
        {
            /* in case of old invite, the application should not get an idle state of a
                pending re-invite */
            RLIST_ITEM_HANDLE       pItem        = NULL;     /*current item*/
            RLIST_ITEM_HANDLE       pNextItem    = NULL;     /*next item*/
            CallLegInvite           *pReInvite     = NULL;
            RLIST_POOL_HANDLE       hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
            
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscTimeOut - Call-leg 0x%p - set pending invite."
                ,pCallLeg));
            
            /*get the first item from the list*/
            RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);
            pReInvite = (CallLegInvite*)pItem;
            
            /*go over all the objects*/
            while(pReInvite != NULL)
            {
                pReInvite->bModifyIdleInOldInvite = RV_TRUE; /* do not inform of state Idle */
                RLIST_GetNext(hInvitePool, pCallLeg->hInviteObjsList, pItem, &pNextItem);
                pItem = pNextItem;
                pReInvite = (CallLegInvite*)pItem;
            }
        }
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        return RV_OK;
    }

#ifdef RV_SIP_SUBS_ON
    if(eMethod == SIP_TRANSACTION_METHOD_NOTIFY)
    {
        if(RVSIP_CALL_LEG_STATE_IDLE == pCallLeg->eState)
        {
            /* Terminate call-leg due to time out in the idle state */
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);

            return RV_OK;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */ 

    if(eMethod == SIP_TRANSACTION_METHOD_PRACK)
    {
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
           pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTranscTimeOut - Call-leg 0x%p - Time out on PRACK - call-leg will disconnect",
                 pCallLeg));
            return CallLegSessionDisconnect(pCallLeg);
        }
        if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED &&
            pCallLeg->hActiveTransc != NULL)
        {
            CallLegInvite* pReInvite;
            
            CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pReInvite);

            if(pReInvite != NULL &&
               (pReInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT ||
                pReInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING))
            {
                CallLegInviteTerminate(pCallLeg, pReInvite, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
                RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "HandleTranscTimeOut - Call-leg 0x%p - Time out on PRACK - call-leg will disconnect",
                    pCallLeg));
                return CallLegSessionDisconnect(pCallLeg);
            }
        }
    }



    return RV_OK;
}



/***************************************************************************
 * HandleTranscRanOutOfTimers
 * ------------------------------------------------------------------------
 * General:
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg -
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscRanOutOfTimers(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle     hTransc)
{
    RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscRanOutOfTimers - Call-leg 0x%p - transc 0x%p ran out of timers - Call is Terminated",
               pCallLeg,hTransc));

    CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hTransc);
#endif
    return RV_OK;
}


/***************************************************************************
 * HandleTranscErrorTermination
 * ------------------------------------------------------------------------
 * General:
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg -
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTranscErrorTermination(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle       hTransc)
{
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "HandleTranscErrorTermination - Call-leg 0x%p - transc 0x%p Terminated with error - Terminating call",
               pCallLeg,hTransc));
    CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hTransc);
#endif

    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
* HandleTranscStateMsgSendFailure
* ------------------------------------------------------------------------
* General: This function handles the client_msg_send_failure transaction state.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - the call-leg related to this transaction.
*          hTransc - the transaction handle
*          eMethod - method of the transaction.
*          eStateReason - The reason for this state.
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateMsgSendFailure(
                            IN  CallLeg                          *pCallLeg,
                            IN  RvSipTranscHandle                 hTransc,
                            IN  CallLegInvite                    *pInvite,
                            IN  SipTransactionMethod              eMethod,
                            IN  RvSipTransactionStateChangeReason eStateReason)

{
    RvSipCallLegStateChangeReason eCallReason;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "HandleTranscStateMsgSendFailure - Call-leg 0x%p - Transaction 0x%p msg send failure",
        pCallLeg,hTransc));
    

     /* Detach the call-leg from its connection, since it won't use it anymore... */
    RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
         "HandleTranscStateMsgSendFailure -  Call-leg 0x%p: detach call-leg from its connection 0x%p",
         pCallLeg, pCallLeg->hTcpConn));
    CallLegDetachFromConnection(pCallLeg);
    

    if(SipTransactionIsAppInitiator(hTransc) == RV_TRUE)
    {
#ifdef RV_SIP_IMS_ON        
		/* Notify the security-agreement layer on the failure in trancation */
		CallLegSecAgreeMsgSendFailure(pCallLeg);
#endif /*#ifdef RV_SIP_IMS_ON*/

        if(pCallLeg->callLegEvHandlers != NULL &&
           pCallLeg->callLegEvHandlers->pfnTranscStateChangedEvHandler)
        {
            CallLegCallbackTranscStateChangedEv((RvSipCallLegHandle)pCallLeg,hTransc,
                                           RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE,
                                           eStateReason);
        }
        else
        {
            CallLegCallbackTranscResolvedEv((RvSipCallLegHandle)pCallLeg,hTransc,
                                           RVSIP_CALL_LEG_TRANSC_STATUS_MSG_SEND_FAILURE,
                                           0);
        }
        return RV_OK;
    }
	
#ifdef RV_SIP_IMS_ON
	CallLegSecAgreeMsgSendFailure(pCallLeg);
#endif

    switch(eStateReason)
    {
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
        eCallReason = RVSIP_CALL_LEG_REASON_NETWORK_ERROR;
        break;
    case RVSIP_TRANSC_REASON_503_RECEIVED:
        eCallReason = RVSIP_CALL_LEG_REASON_503_RECEIVED;
        break;
    case RVSIP_TRANSC_REASON_TIME_OUT:
        eCallReason = RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT;
        break;
    default:
        eCallReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    }

    switch(eMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
        HandleInviteMsgSendFailure(pCallLeg, hTransc, pInvite, eCallReason);
        break;
    case SIP_TRANSACTION_METHOD_BYE:
        HandleByeMsgSendFailure(pCallLeg, hTransc, eCallReason);

        break;
    case SIP_TRANSACTION_METHOD_PRACK:
        /*if we failed to send prack and we are in the proceeding state - call the disconnect function
          so that bye will be sent*/
        CallLegSessionDisconnect(pCallLeg);
    break;
    case SIP_TRANSACTION_METHOD_CANCEL:

        /*HandleCancelMsgSendFailure(pCallLeg, hTransc, eCallReason);*/
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscStateMsgSendFailure - Call-leg 0x%p - send failed - Terminating call",
            pCallLeg));
        CallLegDisconnectWithNoBye(pCallLeg,RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        break;
    default:
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTranscStateMsgSendFailure - Call-leg 0x%p - Transaction 0x%p unknown method",
            pCallLeg,hTransc));
        break;
    }


    return RV_OK;
}

/***************************************************************************
 * HandleInviteMsgSendFailure
 * ------------------------------------------------------------------------
 * General: This function called when an invite message sending was failed.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - the call-leg related to this transaction.
 *          hTransc      - the transaction handle
 *          eCallReason - The reason for this state.
 * Output:  (-)
 ***************************************************************************/
static void RVCALLCONV HandleInviteMsgSendFailure(
                            IN  CallLeg                       *pCallLeg,
                            IN  RvSipTranscHandle             hTransc,
                            IN  CallLegInvite                 *pInvite,
                            IN  RvSipCallLegStateChangeReason eCallReason)
{
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING)
    {
        /* initial invite failure */
        pCallLeg->ePrevState = pCallLeg->eState;

        CallLegChangeState(pCallLeg,
                           RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE,
                           eCallReason);
    }
    else if((pInvite != NULL) &&
            (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT ||
             pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING))
    {
        /* re-invite failure */
        pInvite->ePrevModifyState = pInvite->eModifyState;
        CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE,
                                 eCallReason);
    }
    else if(SipTransactionGetLastState(hTransc) == RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT)
    {
        if(NULL == pInvite)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteMsgSendFailure: call-leg 0x%p - transc 0x%p ACK failure. no invite object",
                pCallLeg,hTransc));
            
        }
        else if(RV_TRUE == pInvite->bInitialInvite)
        {
            /* ack for initial INVITE */
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteMsgSendFailure: call-leg 0x%p - ACK failure for initial INVITE - terminate call-leg.",
                pCallLeg));
            CallLegTerminate(pCallLeg, eCallReason);
        }
        else
        {
            /*  ACK for re-invite, do nothing. modify state remains idle.
                call state remains connected.*/
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteMsgSendFailure: call-leg 0x%p - ACK failure for RE-INVITE - do nothing.",
                pCallLeg));
        }

    }
    else
    {
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLING)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteMsgSendFailure: call-leg 0x%p - nothing to do on state %s",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        }
        else
        {
            /* bug */
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleInviteMsgSendFailure: call-leg 0x%p - unexpected call state %s",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        }
    }
}

/***************************************************************************
 * HandleByeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: This function called when a bye message sending failed.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - the call-leg related to this transaction.
 *          hTransc      - the transaction handle
 *          eCallReason - The reason for this state.
 * Output:  (-)
 ***************************************************************************/
static void RVCALLCONV HandleByeMsgSendFailure(
                            IN  CallLeg                       *pCallLeg,
                            IN    RvSipTranscHandle            hTransc,
                            IN  RvSipCallLegStateChangeReason  eCallReason)
{
    RV_UNUSED_ARG(hTransc);
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING)
    {
        /* initial invite failure */
        pCallLeg->ePrevState = RVSIP_CALL_LEG_STATE_DISCONNECTING;

        CallLegChangeState(pCallLeg,
                           RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE,
                           eCallReason);
    }
    else
    {
        /* bug */
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleByeMsgSendFailure:  Call-leg 0x%p - no msg to fail. call state is %s",
            pCallLeg, CallLegGetStateName(pCallLeg->eState)));
    }
}


#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT */
/***************************************************************************
 * UpdateReceivedMsg
 * ------------------------------------------------------------------------
 * General: Update the received message in the call-leg.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg handle
 *          hTransc - The transaction handle.
 *          eTranscState  - The transaction state.
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateReceivedMsg(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState)
{
    RvStatus rv;

    switch (eTranscState)
    {
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
        rv = RvSipTransactionGetReceivedMsg(hTransc, &(pCallLeg->hReceivedMsg));
        if (RV_OK != rv)
        {
            pCallLeg->hReceivedMsg = NULL;
            return rv;
        }
        break;
    default:
        break;
    }
    return RV_OK;
}

/***************************************************************************
 * ResetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Reset the received message in the call-leg.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg handle
 *          eTranscState  - The transaction state.
 ***************************************************************************/
static void RVCALLCONV ResetReceivedMsg(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTransactionState  eTranscState)
{
    switch (eTranscState)
    {
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
        pCallLeg->hReceivedMsg = NULL;
        break;
    default:
        break;
    }
}

                            
/***************************************************************************
 * RejectInvalidRequests
 * ------------------------------------------------------------------------
 * General: Reject a transaction and change the call-leg state accordingly.
 *          This function will be called when the CSeq is out of order /
 *          when a required option is unsupported / When a Replaces header is
 *          in a request other then INVITE.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg handle
 *          hTransc  - The transaction handle
 *          responseCode - The response code
 *          eTranscState  - The transaction state.
 * Output:  (-)
 ***************************************************************************/
static RvStatus RVCALLCONV RejectInvalidRequests(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState,
                            IN  RvBool                *bWasRejected)


{
    RvStatus              rv;
    SipTransactionMethod  eMethod;
    void                 *pSubsInfo;
    RvBool                bCseqToSmall = RV_FALSE;
    RvUint16              responseCode = 0;
    RvChar*               pstrResponse = NULL;
    RvInt32               transcCseq;
    RvBool                bChangeModifyState = RV_FALSE;
    CallLegInvite*        pInvite = NULL;
    
    *bWasRejected = RV_TRUE;
    SipTransactionGetMethod(hTransc,&eMethod);
    
    if(eTranscState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
        eTranscState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD)
    {
        /* nothing to reject */
        *bWasRejected = RV_FALSE;
        return RV_OK;
    }
    
    if(pCallLeg->createdRejectStatusCode == 0)
    {
        rv = CallLegShouldRejectInvalidRequest(pCallLeg, hTransc, &responseCode, &bCseqToSmall);
    }
    else
    {
        responseCode = pCallLeg->createdRejectStatusCode;
        pCallLeg->createdRejectStatusCode = 0;
    }
    
    if(responseCode > 0)
    {
        rv = CallLegInviteFindObjByTransc(pCallLeg, hTransc, &pInvite);
        if(rv == RV_OK && pInvite != NULL && pInvite->bInitialInvite == RV_FALSE)
        {
            /* this is a reject on re-invite */
            bChangeModifyState = RV_TRUE;
        }

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RejectInvalidRequests - Call-leg 0x%p invite 0x%p - Rejecting transaction 0x%p with code %d",
            pCallLeg, pInvite, hTransc,responseCode));

        if(bCseqToSmall == RV_TRUE)
        {
            pstrResponse = "CSeq Too Small For This Call";
        }
        rv = RvSipTransactionRespond(hTransc,responseCode,pstrResponse);
        if (rv != RV_OK)
        {
            /*transaction failed to send respond*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RejectInvalidRequests - Call-leg 0x%p - Failed to send 0x%p response (rv=%d)",
                pCallLeg, responseCode,rv));
            CallLegDisconnectWithNoBye(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return rv;
        }

        /* inform subscription of final response sent - for SUBSCRIBE / NOTIFY */
        pSubsInfo = SipTransactionGetSubsInfo(hTransc);
        if(pSubsInfo != NULL)
        {
            rv = SipSubsInformOfRejectedRequestByDialog(pSubsInfo, hTransc);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RejectInvalidRequests - Call-leg 0x%p - SipSubsInformOfRejectedRequestByDialog failed (rv %d)",
                pCallLeg, rv));
                return rv;
            }
            return RV_OK;
        }
        if(eMethod == SIP_TRANSACTION_METHOD_INVITE)
        {
            if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
            {
            /*the initial invite was invalid - change the state to disconnected and
              wait for the ack*/
                CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED,
                                   RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
                CallLegSetActiveTransc(pCallLeg, hTransc);

            }
            else
            {
                if(RV_TRUE == bChangeModifyState && pInvite != NULL)
                {
                    CallLegChangeModifyState(pCallLeg, pInvite,
                                 RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT,
                                 RVSIP_CALL_LEG_REASON_LOCAL_REJECT);
                }
                else if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
                {
                    /* if a re0invite was received in the middle of a re-invite process,
                       (old invite) no invite object was created. 
                       so pInvite is NULL, and we need to detach the transaction, otherwise
                       we will have error while trying to take it out of the transactions list
                       on its termination */
                    SipTransactionDetachOwner(hTransc);
                }
            }
        }
        return RV_OK;
    }



    RvSipTransactionGetCSeqStep(hTransc,&transcCseq);
	rv = SipCommonCSeqSetStep(transcCseq,&pCallLeg->remoteCSeq); 
    if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"RejectInvalidRequests - Call-leg 0x%p - failed to set the Call-leg remote Cseq (rv %d)",
			pCallLeg, rv));
		return rv;
	}

    *bWasRejected = RV_FALSE;
    return RV_OK;
}


/***************************************************************************
 * CopyTimestampFromMsg
 * ------------------------------------------------------------------------
 * General: Copy time stamp header from the message to the call-leg
 *          page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The Call-leg handle
 *          hMsg - handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV CopyTimestampFromMsg(IN CallLeg   *pCallLeg,
                                                IN RvSipMsgHandle hMsg)
{
    RvSipOtherHeaderHandle    hTimeStamp;
    RvSipHeaderListElemHandle hElement;
    RvStatus rv;

    hTimeStamp =  RvSipMsgGetHeaderByName(hMsg,"Timestamp",
                                          RVSIP_FIRST_HEADER,&hElement);
    if(hTimeStamp == NULL)
    {
        return RV_OK;
    }
    if(pCallLeg->hTimeStamp == NULL)
    {
        rv = RvSipOtherHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                       pCallLeg->pMgr->hGeneralPool,
                                       pCallLeg->hPage,
                                       &(pCallLeg->hTimeStamp));
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "CopyTimestampFromMsg - pCallLeg 0x%p, failed construct Timestamp header",
                      pCallLeg));
            return rv;
        }
    }

    rv = RvSipOtherHeaderCopy(pCallLeg->hTimeStamp,hTimeStamp);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CopyTimestampFromMsg - pCallLeg 0x%p, failed to copy Timestamp header",
                  pCallLeg));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * ResetIncomingRel1xxParamsOnFinalResponse
 * ------------------------------------------------------------------------
 * General: On INVITE (initial or modify) we should reset the parameters saved
 *          from an incoming reliable 1xx, for handling another reliable 1xx
 *          in the future.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The Call-leg handle
 ***************************************************************************/
static void RVCALLCONV ResetIncomingRel1xxParamsOnFinalResponse(CallLeg* pCallLeg)
{
	CallLegResetIncomingRSeq(pCallLeg); 
	SipCommonCSeqReset(&pCallLeg->incomingRel1xxCseq); 
}



/***************************************************************************
 * GetCseqStepFromMsg
 * ------------------------------------------------------------------------
 * General: Gets the cseq step from a given message.
 * Return Value: cseq step
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The Call-leg handle
 ***************************************************************************/
static RvStatus RVCALLCONV GetCseqStepFromMsg(IN  CallLeg        *pCallLeg,
											  IN  RvSipMsgHandle  hMsg,
											  OUT RvInt32        *pCSeqStep)
{
    RvSipCSeqHeaderHandle     hCSeq = NULL;
	RvStatus                  rv    = RV_OK;	
    
    hCSeq = RvSipMsgGetCSeqHeader(hMsg);
    if(hCSeq == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc, 
            "GetCseqStepFromMsg: Failed to get CSEQ header from msg 0x%p", hMsg));
        return RV_ERROR_NOT_FOUND;
    }

	rv = SipCSeqHeaderGetInitializedCSeq(hCSeq,pCSeqStep);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GetCseqStepFromMsg: failed to retrieve CSeq from msg 0x%p", hMsg));
        return rv;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

    return RV_OK;
}

/***************************************************************************
 * HandleInviteResponseNoTransc
 * ------------------------------------------------------------------------
 * General: Call-leg handling of a 1xx/2xx response for INVITE, when the response is not 
 *          related to any transaction.
 *          1. Checks the call-leg validity.
 *          2. Find the correct invite object, using the cseq value.
 *          3. Handles the received message:
 *             If the message is 1xx - must be a forking case
 *             If the message is 2xx -
 *                  If the call state is inviting or proceeding --> this is the
 *                  first 2xx, must be a forking case.
 *                  Else --> this is a retransmission of the 2xx, and
 *                  the transaction was already terminated.
 * Return Value:(-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLegMgr
 *          hReceivedMsg - The received 1xx/2xx response.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteResponseNoTransc(
                                                 IN CallLeg*				pCallLeg,
                                                 IN RvSipMsgHandle			hReceivedMsg,
                                                 IN SipTransportAddress*    pRcvdFromAddr,
												 IN RvInt32					curIdentifier,
                                                 IN RvInt16					responseCode)
{
    RvStatus rv;
    RvInt32  cseq;
    CallLegInvite* pInvite = NULL;

    if(RV_FALSE == CallLegVerifyValidity(pCallLeg->pMgr, pCallLeg, curIdentifier))
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleInviteResponseNoTransc - call-leg 0x%p is not valid.",
            pCallLeg));
        
        return RV_ERROR_UNKNOWN;
    }

	/* Save received from address to message */
	CallLegSaveReceivedFromAddr(pCallLeg, pRcvdFromAddr);
    
    rv = GetCseqStepFromMsg(pCallLeg, hReceivedMsg,&cseq);
    if (rv != RV_OK)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
            "HandleInviteResponseNoTransc - Failed to retrieve CSeq for msg 0x%p",
            hReceivedMsg));
        return RV_OK;
    }
    
    rv = CallLegInviteFindObjByCseq(pCallLeg, cseq, RVSIP_CALL_LEG_DIRECTION_OUTGOING, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        /* did not find invite object for the response message (may be was terminated already...) */
        RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
            "HandleInviteResponseNoTransc - call-leg 0x%p - Failed to find invite object",
            pCallLeg));
        return rv;
    }
    
    /* now we have a locked call-leg that can handle the message */
    if(responseCode < 200) /*1xx*/
    {
        rv = CallLegForkingHandle1xxMsg(pCallLeg, pInvite, hReceivedMsg);       
        /* if we fail to handles the forked 1xx, and the call-leg timer was not set yet,
          we better terminate the call, otherwise it won't be free never... */
        if(RV_OK != rv)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
                "HandleInviteResponseNoTransc - call-leg 0x%p - Failed in CallLegForkingHandle1xxMsg. terminate call...",
                pCallLeg));
            CallLegForkingTerminateOnFailure(pCallLeg);
        }
    }
    else /* 2xx */
    {
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING ||
            pCallLeg->eState == RVSIP_CALL_LEG_STATE_PROCEEDING)
        {
            /* this is first 2xx response - possible only in forked call-leg */
            rv = CallLegForkingHandleFirst2xxMsg(pCallLeg, pInvite, hReceivedMsg);
            /* if we fail to handles the forked 1xx, and the call-leg timer was not set yet,
              we better terminate the call, otherwise it won't be free never... */
            if(RV_OK != rv)
            {
                CallLegForkingTerminateOnFailure(pCallLeg);
            }
        }
        else if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
                "HandleInviteResponseNoTransc - call-leg 0x%p - invite 0x%p already terminated. do not retransmit the ACK",
                pCallLeg, pInvite));
            return RV_OK;
        }
        else
        {
            /* retransmissions of 2xx for invite or re-invite */
            rv = HandleInvite2xxRetransmission(pCallLeg, pInvite, hReceivedMsg, responseCode);
        }
        
    }
    return rv;
}


#endif /*RV_SIP_PRIMITIVES */


