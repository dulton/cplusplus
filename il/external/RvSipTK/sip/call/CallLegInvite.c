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
 *                              <CallLegInvite.c>
 *
 *  Handles INIVTE process methods. 
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 March 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegInvite.h"
#include "CallLegObject.h"
#include "_SipCallLeg.h"
#include "CallLegRouteList.h"
#include "RvSipCallLeg.h"

#include "CallLegCallbacks.h"
#include "CallLegSessionTimer.h"
#include "CallLegMsgEv.h"
#include "CallLegForking.h"
#include "CallLegSession.h"
#ifdef RV_SIP_IMS_ON
#include "CallLegSecAgree.h"
#endif /* RV_SIP_IMS_ON */
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"


#include "_SipTransactionTypes.h"
#include "_SipTransportTypes.h"
#include "_SipTransport.h"
#include "_SipTransmitterMgr.h"
#include "_SipTransmitter.h"
#include "_SipPartyHeader.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV FillAckMsgFromCallLeg (
                                                  IN CallLeg*         pCallLeg,
                                                  IN CallLegInvite*   pInvite,
                                                  OUT RvBool*   pbSendToFirstRouteHeader);

static RvStatus RVCALLCONV SetParamsInAckTransmitter(CallLeg*       pCallLeg,
                                                     CallLegInvite* pInvite);

static RvStatus RVCALLCONV CreateAckMsg (IN CallLeg*         pCallLeg,
                                         IN CallLegInvite*   pInvite,
                                         OUT RvBool*   pbSendToFirstRouteHeader);

RvBool HandleInviteAckTimerTimeout(IN void *pContext,
                                   IN RvTimer *timerInfo);

static RvStatus RVCALLCONV CreateAckTransmitter(IN CallLeg*         pCallLeg,
                                                IN CallLegInvite*   pInvite);

static RvStatus RVCALLCONV CallLegRemoveInviteObj(IN  CallLeg*        pCallLeg,
                                                  IN  CallLegInvite  *pInvite);

static void RVCALLCONV HandleTrxMsgSendFailureState(CallLeg*               pCallLeg,
                                                    CallLegInvite*         pInvite,
													RvSipMsgHandle         hMsg,
                                                    RvSipTransmitterReason eReason);

static void RVCALLCONV HandleTrxNewConnInUseState(CallLeg*               pCallLeg,
                                                  RvSipTransmitterReason eReason,
                                                  void*                  pExtraInfo);

static RvStatus RVCALLCONV HandleTrxDestResolvedState(
											IN  CallLeg*                 pCallLeg,
											IN  RvSipTransmitterHandle   hTrx,
											IN  RvSipMsgHandle           hMsg);

static RvStatus RVCALLCONV InviteSendAckOn2xx(
                                              IN CallLeg          *pCallLeg,
                                              IN CallLegInvite*   pInvite,
                                              IN RvUint16         responseCode);

static RvStatus RVCALLCONV InviteSendAckWithTransc(
                                   IN CallLeg                       *pCallLeg,
                                   IN RvSipTranscHandle             hTransc,
                                   IN RvUint16                     responseCode);

static RvStatus RVCALLCONV TerminateInviteEventHandler(IN void *obj, IN RvInt32 reason);

static RvStatus RVCALLCONV RemoveOldInvitePendingInvites(IN CallLeg *pCallLeg,
                                                         OUT RvUint16 *pRejectCode);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegInviteCreateObj
* ------------------------------------------------------------------------
* General: Creates a new invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
*          bReInvite  - initial invite or re-invite object.
* Output:  phNotify   - Handle to the new created notification object.
*          pRejectCode - in case of invite inside invite (old invite). may be NULL!!!
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCreateObj(IN  CallLeg*        pCallLeg,
                                           IN  RvSipAppCallLegInviteHandle hAppInvite,
                                           IN  RvBool            bReInvite,
                                           IN  RvSipCallLegDirection eDirection,
                                           OUT CallLegInvite*    *ppInviteObj,
                                           OUT RvUint16          *pRejectCode)
{
    RvStatus          rv;
    RLIST_ITEM_HANDLE  listItem;
    CallLegInvite* pNewObj;
    RvUint16 rejectCode = 0;

	/*----------------------------------------------------------------------------
     Before creating a new object remove the old (irrelevant) objects ONLY in old 
	 INVITE handling
    -----------------------------------------------------------------------------*/
	if (RV_TRUE == pCallLeg->pMgr->bOldInviteHandling && RV_TRUE == bReInvite)
	{
		rv = RemoveOldInvitePendingInvites(pCallLeg, &rejectCode);
		if(0 < rejectCode && pRejectCode != NULL)
        {
            *pRejectCode = rejectCode;
            return RV_OK;
        }
        if (RV_OK != rv) 
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegInviteCreateObj - Call-leg 0x%p - Couldn't remove already completed INVITE objects (rv=%d)", pCallLeg, rv));
			return rv;
		}
        
	}
	
    /*--------------------------------
     create a new object
    ----------------------------------*/
    RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    rv = RLIST_InsertTail(pCallLeg->pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList,&listItem);
    RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteCreateObj - Call-leg 0x%p - Failed to insert new invite obj to list (rv=%d)", pCallLeg, rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pNewObj = (CallLegInvite*)listItem;

    memset(&(pNewObj->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));
	SipCommonCSeqReset(&pNewObj->cseq); 
    if(RV_TRUE == bReInvite)
    {
        pNewObj->eModifyState = RVSIP_CALL_LEG_MODIFY_STATE_IDLE;
        pNewObj->bInitialInvite = RV_FALSE;
    }
    else /* initial invite */
    {
        pNewObj->eModifyState = RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED;
        pNewObj->bInitialInvite = RV_TRUE;
    }
    pNewObj->ePrevModifyState = RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED;
    pNewObj->eLastModifyState = RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED;
    pNewObj->eTerminateReason = RVSIP_CALL_LEG_REASON_UNDEFINED;
    pNewObj->hAppInvite = hAppInvite;
    pNewObj->eDirection = eDirection;
    pNewObj->hAckMsg = NULL;
    pNewObj->hTrx = NULL;
    pNewObj->pCallLeg = pCallLeg;
    pNewObj->bFirstAck = RV_TRUE;
    pNewObj->hInviteTransc = NULL;
    pNewObj->numOf2xxRetransmissions = 0;
    pNewObj->long2xxTimeout = RvInt64Const2(0);
    pNewObj->bObjSentToTerminationQueue = RV_FALSE;
    pNewObj->hActiveConn = NULL;
    pNewObj->hClientResponseConnBackUp = NULL;
#ifdef RV_SIP_AUTH_ON
    pNewObj->hListAuthorizationHeaders = NULL;
#endif /* #ifdef RV_SIP_AUTH_ON */
    pNewObj->hToFrom2xx = NULL;
    pNewObj->hInvitePage = NULL;
    pNewObj->bModifyIdleInOldInvite = RV_FALSE;
    SipCommonCoreRvTimerInit(&pNewObj->timer);

    *ppInviteObj = pNewObj;


    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteCreateObj - Call-leg 0x%p - new invite obj 0x%p was created",
        pCallLeg, *ppInviteObj));

    return RV_OK;
}

/***************************************************************************
* CallLegInviteSetParams
* ------------------------------------------------------------------------
* General: Set parameters in a new invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg  
*          pInvite 
*          hTransc 
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSetParams(IN  CallLeg          *pCallLeg,
                                           IN  CallLegInvite    *pInvite,
                                           IN  RvSipTranscHandle hTransc)
{
    RvInt32  cseqVal;
    RvStatus rv;
    CallLegTranscMemory* pMemory;

    rv = RvSipTransactionGetCSeqStep(hTransc, &cseqVal);
    if (rv != RV_OK || NULL == hTransc)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteSetParams - Call-leg 0x%p - failed to get cseq from transaction",pCallLeg));
        return rv;
    }

    pInvite->hInviteTransc = hTransc;
	rv = SipCommonCSeqSetStep(cseqVal,&pInvite->cseq); 
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteSetParams - Call-leg 0x%p - failed in SipCSeqSet()",pCallLeg));
        return rv;
	}

    /* save a pointer to the invite object in the transaction object */
    rv = CallLegAllocTransactionCallMemory(hTransc, &pMemory);
    if(rv != RV_OK || NULL == pMemory)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteSetParams - Call-leg 0x%p - failed to set invite pointer in transaction 0x%p",
            pCallLeg, hTransc));
        return RV_ERROR_OUTOFRESOURCES;
    }
    pMemory->pInvite = pInvite;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteSetParams - Call-leg 0x%p invite 0x%p - got cseq %u, transaction 0x%p",
        pCallLeg, pInvite, cseqVal, hTransc));
    
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif
    return RV_OK;
}

/***************************************************************************
 * CallLegInviteDetachInviteTransc
 * ------------------------------------------------------------------------
 * General: The function cuts the relationships between the invite transaction,
 *          and the call-leg. 
 *          The function remove the transaction from the invite object - and so,
 *          from the call-leg invite transactions list.
 *          It also detach the transaction owner - so it won't hold the call-leg 
 *          handle, and reset the pointer to the invite object, in the context memory.
 * Return Value: cseq step
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteDetachInviteTransc(IN  CallLeg          *pCallLeg,
											        IN  CallLegInvite    *pInvite,
											        IN  RvSipTranscHandle hTransc)
{
    RvStatus                  rv    = RV_OK;
    CallLegTranscMemory* pTranscMemory;

    if(pInvite == NULL || hTransc == NULL)
	{
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDetachInviteTransc - Call-leg 0x%p - Failed Invite =0x%p hTransc=0x%p",
            pCallLeg, pInvite, hTransc));
		return RV_ERROR_INVALID_HANDLE;
	}
	
	
	/* reset transaction handle in invite object */
    pInvite->hInviteTransc = NULL;

    /* reset transaction owner and context */
    pTranscMemory = SipTransactionGetContext(hTransc);
    if(NULL == pTranscMemory)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDetachInviteTransc - Call-leg 0x%p invite 0x%p - no context in transc 0x%p",
            pCallLeg, pInvite, hTransc ));
    }
    else
    {
        pTranscMemory->pInvite = NULL;
    }

    rv = SipTransactionDetachOwner(hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDetachInviteTransc - Call-leg 0x%p invite 0x%p - Failed to detach transc 0x%p",
            pCallLeg, pInvite, hTransc ));
        SipTransactionTerminate(hTransc);
    }
    
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteDetachInviteTransc - Call-leg 0x%p invite 0x%p - transaction 0x%p was detached successfuly",
        pCallLeg, pInvite, hTransc ));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif
    return RV_OK;
}


/***************************************************************************
* CallLegInviteTerminate
* ------------------------------------------------------------------------
* General: terminates invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg  - Handle to the call-leg, owns the invite object.
*          pInvite   - Handle to the invite object.
*          eReason   - termination reason.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteTerminate(IN  CallLeg*        pCallLeg,
                                           IN  CallLegInvite  *pInvite,
                                           IN  RvSipCallLegStateChangeReason eReason)
{
    RvBool bSendInviteToTerminationQueue = RV_TRUE;
    RvStatus rv;
    RvBool  bInitial = (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED)?RV_TRUE:RV_FALSE;
    
    if(pInvite == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pInvite->eTerminateReason == RVSIP_CALL_LEG_REASON_UNDEFINED)
    {
        pInvite->eTerminateReason = eReason;
    }
    
    if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED &&
       pInvite->bObjSentToTerminationQueue == RV_TRUE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - already in state Terminated",
            pCallLeg, pInvite));
        
        return RV_OK;
    }

    if(NULL != pInvite->hTrx)
    {
        RvSipTransmitterState eTrxState;

        rv = RvSipTransmitterGetCurrentState(pInvite->hTrx, &eTrxState);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - failed to get state of trx 0x%p",
                pCallLeg, pInvite,pInvite->hTrx));
        }
        else
        {
            switch(eTrxState)
            {
            case RVSIP_TRANSMITTER_STATE_ON_HOLD:
            case RVSIP_TRANSMITTER_STATE_UNDEFINED:
            case RVSIP_TRANSMITTER_STATE_IDLE:
            case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
            case RVSIP_TRANSMITTER_STATE_MSG_SENT:
            case RVSIP_TRANSMITTER_STATE_TERMINATED:
                /* the transmitter is not in the middle of sending a request */
                RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegInviteTerminate - call-leg 0x%p invite 0x%p - destructing transmitter 0x%p ...",
                    pCallLeg, pInvite, pInvite->hTrx));
            
                SipTransmitterDestruct(pInvite->hTrx);
                break;

            default:
                /* the transmitter is in the middle of sending a request */
                RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegInviteTerminate - call-leg 0x%p invite 0x%p - detaching from transmitter 0x%p ...",
                    pCallLeg, pInvite, pInvite->hTrx));
            
                SipTransmitterDetachOwner(pInvite->hTrx);
            }
        }
        pInvite->hTrx = NULL;
    }

    CallLegInviteTimerRelease(pInvite);
    
    if(pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED)
    {
        pInvite->eLastModifyState = pInvite->eModifyState;
        pInvite->eModifyState = RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED;
    }

    if(pInvite->hInviteTransc != NULL)
    {
       RvSipTransactionState eTranscState;

       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - handle invite transc 0x%p",
            pCallLeg, pInvite,pInvite->hInviteTransc));

       if(pCallLeg->hActiveTransc == pInvite->hInviteTransc)
       {
           CallLegResetActiveTransc(pCallLeg);
       }
       rv = RvSipTransactionGetCurrentState(pInvite->hInviteTransc, &eTranscState);
       if(rv != RV_OK)
       {
           RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - failed to get state of transc 0x%p",
               pCallLeg, pInvite,pInvite->hInviteTransc));
       }
       else
       {
           switch(eTranscState)
           {
           case RVSIP_TRANSC_STATE_IDLE:
               /* the terminate was called before the transaction came back from the created event...
                  in this case the transaction won't have the call-leg as owner, and we need to
                  terminate the invite object from here... (can't wait for the transaction termination
                  in order to terminate the invite object...)*/
               RvSipTransactionTerminate(pInvite->hInviteTransc);	
			   /*As mentioned in the comment above the transaction has no owner so we can't wait for the transaction 
			     termination in order to terminate the invite object. because of that we need to NULLIFY the hInviteTransc
			     of the pInvite explicitly.*/
			   pInvite->hInviteTransc = NULL;
               bSendInviteToTerminationQueue = RV_TRUE;
               break;
           case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
               /* detach, so transaction will continue to handle the retransmissions */
               rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
               if(rv != RV_OK)
               {
                   RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - failed to detach transc 0x%p on state CLIENT_INVITE_ACK_SENT",
                       pCallLeg, pInvite,pInvite->hInviteTransc));
               }
               break;
           case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
		       {
			       /*There are two optional conditions to terminate the transaction:
				   1. If this is a transaction that the call-leg took its 
			       transmitter, and the transport is not UDP, then this is
			       a transaction that does not have a timer. In this case we must 
			       terminate this transaction or it will remain forever.
				   2. If we work in old invite mode and the INVITE was responded with 2xx.
				   In all other cases, we detach the transaction from the invite object, and
				   it remains independent and terminates according to its own timeout */
			       RvSipTransmitterHandle hTrx = NULL;
			       RvUint16               responseCode;
				   RvSipTransactionGetResponseCode(pInvite->hInviteTransc, &responseCode);
				   RvSipTransactionGetTransmitter(pInvite->hInviteTransc, &hTrx);
			       if((hTrx == NULL && SipTransactionGetTransport(pInvite->hInviteTransc) == RVSIP_TRANSPORT_UDP) ||
					  (pCallLeg->pMgr->bOldInviteHandling == RV_TRUE && responseCode >= 200 && responseCode < 300))
			       {
				       RvSipTransactionTerminate(pInvite->hInviteTransc);
				       bSendInviteToTerminationQueue = RV_FALSE;
			       }
			       else
			       {
					   rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
					   if(rv != RV_OK)
					   {
						   RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
							   "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - failed to detach transc 0x%p on state INVITE_FINAL_RESPONSE_SENT",
							   pCallLeg, pInvite,pInvite->hInviteTransc));
					   }
			       }
			       break;
		       }
           case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
           case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
               /* if we sent a re-invite and then a BYE, and the 2xx on BYE is received before the 487 on
                  re-invite, we want to detach from the invite transaction, so that the transaction will be able to
                  catch the 487 and send the ACK.
                  for initial INVITE it is not relevant, because you can't send a BYE until the call is established
                  (--> already received 2xx).
               */
               if(RV_FALSE == bInitial) /* not initial invite */
               {
                   RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - detach transc 0x%p on state CLIENT_INVITE_CALLING/PROCEEDING for sending the ACK later",
                       pCallLeg, pInvite,pInvite->hInviteTransc));
                   rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
                   if(rv != RV_OK)
                   {
                       RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                           "CallLegInviteTerminate - call-leg 0x%p Invite 0x%p - failed to detach transc 0x%p on state CLIENT_INVITE_CALLING/PROCEEDING",
                           pCallLeg, pInvite,pInvite->hInviteTransc));
                   }
                    break;
               }
               /*else - initial invite, continue to the default */
           default:
               /* terminate the transaction. only when the transaction is out of the 
               termination queue, the invite object will be sent to the termination queue */
               RvSipTransactionTerminate(pInvite->hInviteTransc);
               bSendInviteToTerminationQueue = RV_FALSE;
           }
       }       
    }

    if(RV_TRUE == bSendInviteToTerminationQueue)
    {
        /* insert the invite object to the event queue */
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteTerminate - Send invite 0x%p of Call-leg 0x%p to the event queue for termination",
             pInvite,pCallLeg));
    
        pInvite->bObjSentToTerminationQueue = RV_TRUE;
        return SipTransportSendTerminationObjectEvent(
                                    pCallLeg->pMgr->hTransportMgr,
                                    (void *)pInvite,
                                    &(pCallLeg->terminationInfo),
                                    (RvInt32)pInvite->eTerminateReason,
                                    TerminateInviteEventHandler,
                                    RV_TRUE,
                                    CALL_LEG_STATE_TERMINATED_STR,
                                    CALL_LEG_INVITE_OBJECT_NAME_STR);
    } 
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteTerminate - invite 0x%p of Call-leg 0x%p - wait for transc 0x%p to get out of event queue",
            pInvite,pCallLeg, pInvite->hInviteTransc));
        return RV_OK;
    }
}

/***************************************************************************
* CallLegInviteFindObjByCseq
* ------------------------------------------------------------------------
* General: finds the invite object with the given cseq..
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
*          cseq
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByCseq(IN   CallLeg*        pCallLeg,
                                               IN   RvInt32         cseq,
                                               IN   RvSipCallLegDirection eDirection,
                                               OUT  CallLegInvite*  *ppInviteObj)
{
    RLIST_POOL_HANDLE    hInvitePool;
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    CallLegInvite*       pFoundInvite;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops;
    
    hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
    maxLoops = MAX_LOOP(pCallLeg);
    
    /*remove invite object from call-leg's list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    if(NULL == pItem)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByCseq - Call-leg 0x%p - no invite object in the list",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    while(pItem != NULL &&  safeCounter < maxLoops)
    {
        pFoundInvite = (CallLegInvite*)pItem;
        if(SipCommonCSeqIsEqual(&pFoundInvite->cseq,cseq) == RV_TRUE &&
           pFoundInvite->eDirection					== eDirection)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInviteFindObjByCseq - call-leg 0x%p - invite obj 0x%p was found",
                pCallLeg, pFoundInvite));
            
            *ppInviteObj = pFoundInvite;
            return RV_OK;
        }
        else
        {
            RLIST_GetNext(hInvitePool, pCallLeg->hInviteObjsList, pItem, &pNextItem);
            pItem = pNextItem;
            safeCounter++;
        }
    }
    
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByCseq - Call-leg 0x%p - Reached MaxLoops %d, infinite loop",
            pCallLeg,maxLoops));
        return RV_ERROR_UNKNOWN;
    }
    
    *ppInviteObj = NULL;
    return RV_OK;
}

/***************************************************************************
* CallLegInviteFindObjByState
* ------------------------------------------------------------------------
* General: finds the invite object of the initial invite request.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByState(IN  CallLeg*        pCallLeg,
                                                IN  RvSipCallLegModifyState eModifyState,
                                               OUT  CallLegInvite*  *ppInviteObj)
{
    RLIST_POOL_HANDLE    hInvitePool;
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    CallLegInvite*       pFoundInvite;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops;
    
    hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
    maxLoops = MAX_LOOP(pCallLeg);
    
    /*remove invite object from call-leg's list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    if(NULL == pItem)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByState - Call-leg 0x%p - no invite object in the list",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    while(pItem != NULL &&  safeCounter < maxLoops)
    {
        pFoundInvite = (CallLegInvite*)pItem;
        if(pFoundInvite->eModifyState == eModifyState)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInviteFindObjByState - call-leg 0x%p - invite obj 0x%p was found",
                pCallLeg, pFoundInvite));
            
            *ppInviteObj = pFoundInvite;
            return RV_OK;
        }
        else
        {
            RLIST_GetNext(hInvitePool, pCallLeg->hInviteObjsList, pItem, &pNextItem);
            pItem = pNextItem;
            safeCounter++;
        }
    }
    
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByState - Call-leg 0x%p - Reached MaxLoops %d, infinite loop",
            pCallLeg,maxLoops));
        return RV_ERROR_UNKNOWN;
    }
    *ppInviteObj = NULL;
    return RV_OK;
}


/***************************************************************************
* CallLegInviteFindObjByTransc
* ------------------------------------------------------------------------
* General: finds the invite object of the initial invite request.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg
* Output:  ppInviteObj   - Handle to the invite object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteFindObjByTransc(IN  CallLeg*        pCallLeg,
                                                 IN  RvSipTranscHandle hTransc,
                                                 OUT  CallLegInvite*  *ppInviteObj)
{
    CallLegTranscMemory* pTranscMemory;
    SipTransactionMethod eMethod;

    *ppInviteObj = NULL;

    if(NULL == hTransc)
    {
        return RV_ERROR_NOT_FOUND;
    }

    SipTransactionGetMethod(hTransc, &eMethod);
    if(eMethod != SIP_TRANSACTION_METHOD_INVITE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByTransc - Call-leg 0x%p - this is not an invite transc (0x%p)",
            pCallLeg,hTransc));
        return RV_ERROR_UNKNOWN;
    }
    
    pTranscMemory = (CallLegTranscMemory*)SipTransactionGetContext(hTransc);
    if(NULL == pTranscMemory)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteFindObjByTransc - Call-leg 0x%p - no invite call memory in transc 0x%p",
            pCallLeg,hTransc));
        return RV_ERROR_UNKNOWN;
    }

    *ppInviteObj = pTranscMemory->pInvite;
     RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteFindObjByTransc - call-leg 0x%p - invite obj 0x%p was found in transc 0x%p call memory",
        pCallLeg, *ppInviteObj, hTransc));
            
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	 RV_UNUSED_ARG(pCallLeg);
#endif
     return RV_OK;
}

/***************************************************************************
 * CallLegInviteTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the timer of the invite object, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pInvite - The invite object to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegInviteTimerRelease(IN CallLegInvite *pInvite)
{
    if (NULL != pInvite &&
        SipCommonCoreRvTimerStarted(&pInvite->timer))
    {
        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pInvite->timer);
        RvLogDebug((pInvite->pCallLeg->pMgr)->pLogSrc ,((pInvite->pCallLeg->pMgr)->pLogSrc ,
                  "CallLegInviteTimerRelease - Call-Leg 0x%p, invite 0x%p: Timer 0x%p was released",
                  pInvite->pCallLeg, pInvite, &pInvite->timer));
    }
}

/***************************************************************************
 * CallLegInviteCreateAckMsgAndTrx
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCreateAckMsgAndTrx (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite)
{
    RvStatus rv;
    RvBool   bSendToFirstRouteHeader = RV_FALSE;

    /* create ACK message in the invite object -
       so we have what to send in CallLegInviteSendAck function */
    rv = CreateAckMsg(pCallLeg, pInvite, &bSendToFirstRouteHeader);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteCreateAckMsgAndTrx - pCallLeg 0x%p invite 0x%p - failed to create the ACK message. rv=%d",
            pCallLeg, pInvite, rv));
        return rv;
    }
    /* create ACK transmitter in the call-leg */
    rv = CreateAckTransmitter(pCallLeg, pInvite);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteCreateAckMsgAndTrx - pCallLeg 0x%p invite 0x%p - failed to create the ACK transmitter. rv=%d",
            pCallLeg, pInvite, rv));
        return rv;
    }
    if (RV_TRUE == bSendToFirstRouteHeader)
    {
        RvSipTransmitterSetUseFirstRouteFlag(pInvite->hTrx, bSendToFirstRouteHeader);
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteCreateAckMsgAndTrx - pCallLeg 0x%p invite 0x%p - ACK transmitter 0x%p was created",
        pCallLeg, pInvite, pInvite->hTrx));
    return RV_OK;
}

/***************************************************************************
 * CallLegInviteCopyToHeaderFromMsg
 * ------------------------------------------------------------------------
 * General: Copies the to header from a message to the invite object.
 *          To header is copied from INVITE 2xx response and then placed
 *          in the outgoing INVITE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - pointer to the call-leg.
 *          pInvite - pointer to the invite object.
 *          hMsg - Handle to the msg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteCopyToHeaderFromMsg (
                                    IN CallLeg*         pCallLeg,
									IN CallLegInvite*   pInvite,
									IN RvSipMsgHandle   hMsg)
{

	RvStatus rv = RV_OK;
    RvSipPartyHeaderHandle hTo = NULL;
	/* save the To header of the 2xx response (we will use it in building the ACK) */
	hTo = RvSipMsgGetToHeader(hMsg);
	
	if(pInvite->hInvitePage == NULL)
	{
		rv = RPOOL_GetPage(pCallLeg->pMgr->hElementPool, 0, &pInvite->hInvitePage);
		if(rv != RV_OK)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegInviteCopyToHeaderFromMsg - Call-leg 0x%p invite 0x%p - Failed to get page (rv=%d)", 
				pCallLeg, pInvite, rv));
			return rv;
		}
	}
	
	rv = SipPartyHeaderConstructAndCopy(pCallLeg->pMgr->hMsgMgr, 
										pCallLeg->pMgr->hElementPool, 
										pInvite->hInvitePage, 
										hTo, RVSIP_HEADERTYPE_TO,
										&pInvite->hToFrom2xx);
	if(RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegInviteCopyToHeaderFromMsg - Call-leg 0x%p failed to copy TO header in invite 0x%p",
			pCallLeg, pInvite));
		return rv;
	}
	return RV_OK;
}

/***********************************************************************
 * CallLegInviteSendAck
 * ------------------------------------------------------------------------
 * General: Sends an Ack message to the remote party. First the Call-leg's
 *          request URI is determined and then a Ack is sent using the
 *          transaction's API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The Ack was not sent.
 *               RV_OK - Ack message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 *            hTransc  - The transaction used to send the Ack message.
 *            pInvite  - The invite object sends the ACK.
 *          responseCode - the response code of the response message that trigered the ACK
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSendAck(
                            IN CallLeg                       *pCallLeg,
                            IN RvSipTranscHandle             hTransc,
                            IN CallLegInvite                *pInvite,
                            IN RvUint16                     responseCode)
{
    if((pCallLeg->pMgr->bOldInviteHandling == RV_FALSE ||
        (pCallLeg->pMgr->bOldInviteHandling == RV_TRUE && hTransc == NULL))
       && responseCode < 300)
    {
        /* send ACK with no transc - new invite with 200 response,
           or old invite - with 200 response and forked call-leg!!! */
        return InviteSendAckOn2xx(pCallLeg, pInvite, responseCode);
    }
    else
    {
        return InviteSendAckWithTransc(pCallLeg, hTransc, responseCode);
    }
}


/***************************************************************************
* HandleInvite2xxTimerTimeout
* ------------------------------------------------------------------------
* General: The invite object is responsible for 2xx retransmissions.
*          When the 2xx timer expires, we check if we reached the long timer
*          timeout value. 
*          If we did - disconnect the call.
*          Else - retransmit the 2xx message.
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pContext - The call-Leg this timer was called for.
***************************************************************************/
RvBool HandleInvite2xxTimerTimeout(IN void *pContext,
                                   IN RvTimer *timerInfo)
{
    CallLegInvite* pInvite = (CallLegInvite*)pContext;
    CallLeg* pCallLeg = pInvite->pCallLeg;
    RvInt64  curTime = RvTimestampGet(NULL);
    RvStatus rv;
    
    if (RV_OK != CallLegLockMsg(pCallLeg))
    {
        CallLegInviteTimerRelease(pInvite);
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pInvite->timer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
            "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: 2xx long timer expired but was already released. do nothing...",
            pCallLeg, pInvite));
        
        CallLegUnLockMsg(pCallLeg);
        return RV_FALSE;
    }

    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pInvite->timer);
    CallLegInviteTimerRelease(pInvite);
    
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING)
    {
         RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: call state is %s, nothing to do",
            pCallLeg, pInvite, CallLegGetStateName(pCallLeg->eState)));
         CallLegUnLockMsg(pCallLeg);
         return RV_FALSE;
    }

    if(RvInt64IsLessThan(pInvite->long2xxTimeout, curTime) ||
       RvInt64IsEqual(pInvite->long2xxTimeout, curTime))
    {
        /* long timer expiration. ACK  was not received - terminate call-leg */
        RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: 2xx long timer expired. Disconnect call-leg...",
            pCallLeg, pInvite));

        if(pInvite->bInitialInvite == RV_FALSE)
        {
            /* This is a re-invite. Terminate the invite object before calling to disconnect */
            CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        }
        
        rv = CallLegSessionDisconnect(pCallLeg);
        if(rv != RV_OK)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: Failed to disconnect. Terminate call...",
                pCallLeg, pInvite));
            CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
        }
    }
    else
    {
        RvInt32 retransLimit = UNDEFINED;
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: 2xx timer was expired. retransmit msg",
            pCallLeg, pInvite));

        if(pCallLeg->pTimers != NULL && pCallLeg->pTimers->maxInviteResponseRetransmissions != UNDEFINED)
        {
            /* first 2xx sending was done by the transaction, so we decrease one from
               the max retransmissions value */
            retransLimit = pCallLeg->pTimers->maxInviteResponseRetransmissions - 1;
        }
        if( retransLimit == UNDEFINED ||
            pInvite->numOf2xxRetransmissions < retransLimit)
        {
            rv = SipTransmitterRetransmit(pInvite->hTrx);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: Failed to retransmit 2xx. (pInvite->hTrx=0x%p). Disconnect...",
                    pCallLeg, pInvite, pInvite->hTrx));
                rv = CallLegSessionDisconnect(pCallLeg);
                if(rv != RV_OK)
                {
                    RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                        "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: Failed to disconnect. Terminate call...",
                        pCallLeg, pInvite));
                    CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
                }
            }
            pInvite->numOf2xxRetransmissions += 1;
            /* set timer again */
            CallLegInviteSet2xxTimer(pCallLeg, pInvite, RV_FALSE);
        }
        else
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: reached numOf2xxRetransmissions=%d. Disconnect...",
                pCallLeg, pInvite, retransLimit));
            rv = CallLegSessionDisconnect(pCallLeg);
            if(rv != RV_OK)
            {
                RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "HandleInvite2xxTimerTimeout - Call-Leg 0x%p, invite 0x%p: Failed to disconnect. Terminate call...",
                    pCallLeg, pInvite));
                CallLegTerminate(pCallLeg, RVSIP_CALL_LEG_REASON_LOCAL_FAILURE);
            }
        }
    }    
    
    CallLegUnLockMsg(pCallLeg);
    return RV_TRUE;
}

/***************************************************************************
 * CallLegInviteSet2xxTimer
 * ------------------------------------------------------------------------
 * General: Set the 2xx-timer of an invite object.
 *          1. Get T1 and T2 values.
 *          2. If this is the first time - set the longTimeout. (when will 
 *             the call-leg be terminate, if will not receive an ACK)
 *          3. Decide for how long to set the 2xx-timer:
 *             A. If this is the first time - set timer to T1 ms.
 *             B. If not - set timer to min(T2, T1*2^numOfRetransmissions)
 *             C. Check what will come first - the decision we have just made,
 *                or the longTimeout. if longTimeout is sooner, we shall will
 *                set the timer to the time until longTimeout.
 *          4. Set the timer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSet2xxTimer(CallLeg*       pCallLeg,
                                             CallLegInvite* pInvite,
                                             RvBool         bFirstSetting)
{
    RvUint32 T1, T2;
    RvInt64 curTime = RvTimestampGet(NULL);
    RvStatus rv = RV_OK;
    RvInt64  timeToSet;
    RvUint32 timeToSetInMS;
    
    /* T1 and T2 are in milliseconds. 
       curTime and timeToSet are in nanoseconds.
    */
    /* ------------------------------------------------------------------
        1. Get T1 and T2 values. 
       ------------------------------------------------------------------ */
    if(pCallLeg->pTimers != NULL && pCallLeg->pTimers->T1Timeout > 0)
    {
        T1 = (RvUint32)pCallLeg->pTimers->T1Timeout;
    }
    else
    {
        T1 = (RvUint32)SipTransactionMgrGetT1(pCallLeg->pMgr->hTranscMgr);
    }

    if(pCallLeg->pTimers != NULL && pCallLeg->pTimers->T2Timeout > 0)
    {
        T2 = (RvUint32)pCallLeg->pTimers->T2Timeout;
    }
    else
    {
        T2 = (RvUint32)SipTransactionMgrGetT2(pCallLeg->pMgr->hTranscMgr);
    }

    /* -------------------------------------------------------------------
        2. If this is the first time - set the longTimeout to 64*T1. 
           (when will the call-leg be terminate, if will not receive an ACK)
       ------------------------------------------------------------------- */
    if(RV_TRUE == bFirstSetting)
    {
        RvInt64 msec;
        /* long2xxTimeout is saved in nanoseconds --> 64000 and not 64 */
        msec = RvInt64Mul(RvInt64Const2(64), RvInt64FromRvUint32(T1));
        pInvite->long2xxTimeout = RvInt64Add(
                RvInt64Mul(msec, RV_TIME64_NSECPERMSEC), curTime);
    }

    /* --------------------------------------------------------------------
        3. Decide for how long to set the 2xx-timer:
       ------------------------------------------------------------------- */
    if(RV_TRUE == bFirstSetting)
    {
        /* A. If this is the first time - set timer to T1 ms. (converted to nanosec) */
        timeToSet = RvInt64Mul(RvInt64FromRvUint32(T1), RV_TIME64_NSECPERMSEC);
    }
    else
    {
        RvInt64 timeToSet_msec;
        /* B. If not - set timer to min(T2, T1*2^numOfRetransmissions) (converted to nanosec)*/
        timeToSet_msec = RvInt64Mul(RvInt64FromRvUint32(T1), RvInt64FromRvInt32(SipCommonPower(pInvite->numOf2xxRetransmissions)));
        timeToSet = RvInt64Mul(timeToSet_msec, RV_TIME64_NSECPERMSEC);

        if (RV_TRUE == RvInt64IsGreaterThan(timeToSet, RvInt64Mul(RvInt64FromRvUint32(T2),RV_TIME64_NSECPERMSEC)))
        {
             timeToSet = RvInt64Mul(RvInt64FromRvUint32(T2),RV_TIME64_NSECPERMSEC);
        }
    }

        /* C. Check what will come first - the decision we have just made,
              or the longTimeout. if longTimeout is sooner, we shall will
              set the timer to the time until longTimeout. */

    if(RV_TRUE == RvInt64IsGreaterThan(RvInt64Add(curTime, timeToSet), pInvite->long2xxTimeout))
    {
        timeToSet = RvInt64Sub(pInvite->long2xxTimeout, curTime);
    }


    /* ----------------------------------------------------------------------
        4. Set the timer. 
       ---------------------------------------------------------------------- */
    /* convert from Nanosec to ms */
    timeToSetInMS = RvInt64ToRvUint32(RvInt64Div(timeToSet, RV_TIME64_NSECPERMSEC));
     rv = SipCommonCoreRvTimerStartEx(&pInvite->timer,
                                    pCallLeg->pMgr->pSelect,
                                    timeToSetInMS,
                                    HandleInvite2xxTimerTimeout,
                                    (void*)pInvite);
    if (rv != RV_OK)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegInviteSet2xxTimer - Call-Leg 0x%p, invite 0x%p: No timer is available",
            pCallLeg, pInvite));
        return rv;
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegInviteSet2xxTimer - Call-Leg 0x%p, invite 0x%p: timer was set to %u milliseconds",
            pCallLeg, pInvite, timeToSetInMS));
    }
    return rv;

}

/***************************************************************************
* CallLegInviteDestruct
* ------------------------------------------------------------------------
* General: Destructs an invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteDestruct(IN  CallLeg*        pCallLeg,
                                                 IN  CallLegInvite  *pInvite)
{
    if(pInvite == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteDestruct - call-leg 0x%p. invite 0x%p - start in destructing ...",
            pCallLeg, pInvite));
    
    /* release timer and transmitter if needed */
    if(NULL != pInvite->hAckMsg)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDestruct - call-leg 0x%p. invite 0x%p - destructing ACK message 0x%p ...",
            pCallLeg, pInvite, pInvite->hAckMsg));
        
        RvSipMsgDestruct(pInvite->hAckMsg);
        pInvite->hAckMsg = NULL;
    }
    if(NULL != pInvite->hActiveConn)
    {
        RvSipTransportConnectionDetachOwner(pInvite->hActiveConn,
            (RvSipTransportConnectionOwnerHandle)pInvite);
        pInvite->hActiveConn = NULL;
    }

    if(NULL != pInvite->hClientResponseConnBackUp)
    {
        RvSipTransportConnectionDetachOwner(pInvite->hClientResponseConnBackUp,
                                            (RvSipTransportConnectionOwnerHandle)pInvite);
        pInvite->hClientResponseConnBackUp = NULL;
    }
	
    if(pInvite->hTrx != NULL)
    {
        SipTransmitterDestruct(pInvite->hTrx);
    }
    if(pInvite->hInviteTransc != NULL)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteDestruct - call-leg 0x%p. invite 0x%p - hInviteTransc 0x%p. should be NULL",
            pCallLeg, pInvite, pInvite->hInviteTransc));
    }

    CallLegInviteTimerRelease(pInvite);

#ifdef RV_SIP_AUTH_ON
    if(pInvite->hListAuthorizationHeaders != NULL)
    {
        SipAuthenticatorInviteDestructAuthorizationList(pInvite->pCallLeg->pMgr->hAuthMgr,
                                               pInvite->hListAuthorizationHeaders);
        pInvite->hListAuthorizationHeaders = NULL;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

	if(NULL_PAGE != pInvite->hInvitePage)
	{
		RPOOL_FreePage(pInvite->pCallLeg->pMgr->hElementPool,pInvite->hInvitePage);
		pInvite->hInvitePage = NULL_PAGE;
	}
    
    CallLegRemoveInviteObj(pCallLeg, pInvite);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInviteDestruct - call-leg 0x%p. invite 0x%p - end of destruction ...",
            pCallLeg, pInvite));

    return RV_OK;
}

/***************************************************************************
 * CallLegInviteResetTranscInInviteObj
 * ------------------------------------------------------------------------
 * General: Check if the given transaction is an invite transaction,
 *          and if so, delete the transaction handle in the correct invite object.
 *          Note - When working with old invite behavior, the termination of
 *          an invite transaction means that the invite procedure is finished,
 *          so we can terminate the invite object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The CallLeg handle
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteResetTranscInInviteObj(
                                                  CallLeg*          pCallLeg,
                                                  CallLegInvite*    pInvite,
                                                  RvBool            bTimeout)
{
    RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
        "CallLegInviteResetTranscInInviteObj: call-leg 0x%p, invite 0x%p reset transc",
        pCallLeg, pInvite));

    pInvite->hInviteTransc = NULL;

    /* for aggressive termination (using RvSipCallLegInviteTerminate), invite object first
       terminate the transmitter (so pInvite->hTrx is NULL), and then send the transaction 
       to the event queue. in this case too, we want to terminate the invite object here.*/
    if(pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED)
    {
        return CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_UNDEFINED);
    }
    else if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        return CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_UNDEFINED);
    }

    /* for timeout termination - we want to continue handling the invite transaction */
    if(bTimeout == RV_TRUE)
    {
        return RV_OK;
    }

    /* When working with old invite - we can destruct the invite object now. */
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
            "CallLegInviteResetTranscInInviteObj: call-leg 0x%p - removing invite object 0x%p... (old invite)",
            pCallLeg, pInvite));
        CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_UNDEFINED);
    }
    else/* new invite */
    {
        /* terminate the invite object, only the response was non-2xx.
           --> The transaction received/sent the ACK 
           --> invite object does not hold the transmitter */
        /* in case of 2xx response with manual ack mode, the trx is NULL too.
           for this reason, we must also check the states */
        if(pInvite->hTrx == NULL && 
           pCallLeg->eState != RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED &&
           pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
                "CallLegInviteResetTranscInInviteObj: call-leg 0x%p - removing invite object 0x%p... (new invite)",
                pCallLeg, pInvite));
            CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_UNDEFINED);
        }
    }
    return RV_OK;
}

/*---------------------------------------------------------------------------------
                I N V I T E    T R X   H A N D L I N G   F U N T I O N S
  ---------------------------------------------------------------------------------*/



/***************************************************************************
 * CallLegInviteTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg of the Transmitter state change.
 *          (ack transmitter, and 2xx transmitter).
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -     The sip stack Transmitter handle
 *            hAppTrx -     The transaction handle.
 *            eState  -     The new Transmitter state
 *            eReason -     The reason for the state change.
 *            hMsg    -     When the state relates to the outgoing message, the
 *                          message is supplied.
 *            pExtraInfo - specific information for the new state.
 ***************************************************************************/
void RVCALLCONV CallLegInviteTrxStateChangeEv(
       IN  RvSipTransmitterHandle            hTrx,
       IN  RvSipAppTransmitterHandle         hAppTrx,
       IN  RvSipTransmitterState             eState,
       IN  RvSipTransmitterReason            eReason,
       IN  RvSipMsgHandle                    hMsg,
       IN  void*                             pExtraInfo)

{
    CallLegInvite *pInvite = (CallLegInvite*)hAppTrx;
    CallLeg*    pCallLeg = pInvite->pCallLeg;
    SipTransportAddress  destAddr;

    SipTransmitterGetDestAddr(pInvite->hTrx, &destAddr);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(destAddr.eTransport == RVSIP_TRANSPORT_UNDEFINED) {
      RvSipTransport      eOutboundProxyTransport=RVSIP_TRANSPORT_UNDEFINED;
      SipTransmitterGetOutboundTransport(pInvite->hTrx,
					 &eOutboundProxyTransport);
      if(eOutboundProxyTransport==RVSIP_TRANSPORT_UNDEFINED) {
	RvSipTransactionGetOutboundTransport(pInvite->hInviteTransc,
					     &eOutboundProxyTransport);
	if(eOutboundProxyTransport!=RVSIP_TRANSPORT_UNDEFINED) {
	  destAddr.eTransport=eOutboundProxyTransport;
	  SipTransmitterSetOutboundTransport(pInvite->hTrx,eOutboundProxyTransport);
	}
      }
    }
#endif 
//SPIRENT_END

    RV_UNUSED_ARG(hTrx);
    switch (eState)
    {
    case RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED:
        /* Save the active transmitter before going up to the callback */
        pCallLeg->hActiveTrx = pInvite->hTrx;
		HandleTrxDestResolvedState(pCallLeg, hTrx, hMsg);
        /* Reset the active transmitter after the callback */
        pCallLeg->hActiveTrx = NULL;
        break;
    case RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE:
        HandleTrxNewConnInUseState(pCallLeg, eReason, pExtraInfo);
        break;
    case RVSIP_TRANSMITTER_STATE_MSG_SENT:
        /* if this is the ACK transmitter - set the ACK timer here */
        {
            if(pInvite->eDirection == RVSIP_CALL_LEG_DIRECTION_OUTGOING &&
               RV_TRUE == pInvite->bFirstAck)
            {
                /* set the ACK timer - telling us when to destruct the transmitter and invite object */
                CallLegInviteSetAckTimer(pCallLeg, pInvite);
                pInvite->bFirstAck = RV_FALSE;
            }
        }
        break;
    case RVSIP_TRANSMITTER_STATE_READY_FOR_SENDING:
        break;

    case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
        HandleTrxMsgSendFailureState(pCallLeg, pInvite, hMsg, eReason);
        break;
    default:
        break;
    }
    return;
}

/*****************************************************************************
 * CallLegInviteTrxOtherURLAddressFoundEv
 * ---------------------------------------------------------------------------
 * General: implementation to the other url found event handler of the transmitter.
 * Return Value: RvStatus - RvStatus
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx        - the transaction transmitter
 *          hAppTrx     - App handle. in this case always points to transaction
 *          hMsg        - Handle of the message that contains the Other
 *                        URL address.
 *          hAddress    - The other URL address handle to be "converted"
 *                        to SIP URL address .
 *
 * Output:  hSipURLAddress      - Handle to the constructed SIP URL address.
 *          pbAddressResolved   - Indication of a successful/failed address
 *                                resolving.
 *
 ****************************************************************************/
RvStatus RVCALLCONV CallLegInviteTrxOtherURLAddressFoundEv(
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    pbAddressResolved)
{
    CallLegInvite *pInvite = (CallLegInvite*)hAppTrx;
    CallLeg*    pCallLeg = pInvite->pCallLeg;
    RvStatus    rv       = RV_OK;

    RV_UNUSED_ARG(hTrx);
    rv = CallLegCallbackTranscOtherURLAddressFoundEv(pCallLeg,
                                                    NULL,
                                                    hMsg,
                                                    hAddress,
                                                    hSipURLAddress,
                                                    pbAddressResolved);

    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteTrxOtherURLAddressFoundEv - pCallLeg 0x%p: Application failed to handle unknown URL address(rv=%d)",
            pCallLeg, rv));
        return rv;
    }
    return rv;
}

/***************************************************************************
* CallLegInviteSetAckTimer
* ------------------------------------------------------------------------
* General: sets the forked Ack Transmitter timer.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - The call-Leg this timer was called for.
***************************************************************************/
RvStatus RVCALLCONV CallLegInviteSetAckTimer(IN CallLeg*      pCallLeg,
                                             IN CallLegInvite* pInvite)
{
    RvStatus rv = RV_OK;
    RvInt32 inviteLingerTimeout = 
        (pCallLeg->pTimers != NULL)? pCallLeg->pTimers->inviteLingerTimeout:pCallLeg->pMgr->inviteLingerTimeout;
    
    if( inviteLingerTimeout == 0 )
    {
        SipTransportAddress          destAddr;
	SipTransmitterGetDestAddr(pInvite->hTrx, &destAddr);
        if(destAddr.eTransport == RVSIP_TRANSPORT_TLS)
        {
            inviteLingerTimeout = OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
        }
        else
        {
            inviteLingerTimeout = OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT;
        }
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegInviteSetAckTimer - Call-Leg  0x%p: inviteLingerTimeout=0. set timer to %d ms",
            pCallLeg, inviteLingerTimeout));

    }
    rv = SipCommonCoreRvTimerStartEx(&pInvite->timer,
                                   pCallLeg->pMgr->pSelect,
                                   inviteLingerTimeout,
                                   HandleInviteAckTimerTimeout,
                                   (void*)pInvite);
    if (rv != RV_OK)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegInviteSetAckTimer - Call-Leg  0x%p: No timer is available",
            pCallLeg));
        return rv;
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegInviteSetAckTimer - Call-Leg  0x%p: ack timer was set to %u milliseconds",
            pCallLeg, inviteLingerTimeout));
    }
    
    return rv;

}

/***************************************************************************
 * CallLegConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn   -   The connection handle
 *          hOwner  -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInviteConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hInvite,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{
    CallLegInvite* pInvite = (CallLegInvite*)hInvite;
    CallLeg*    pCallLeg;

    RV_UNUSED_ARG(eReason);

    if(pInvite == NULL)
    {
        return RV_OK;
    }
    pCallLeg = pInvite->pCallLeg;

    if (CallLegLockMsg(pCallLeg) != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegInviteConnStateChangedEv - call-leg 0x%p, conn=0x%p,Failed to lock call-leg",
                   pCallLeg,hConn));
        return RV_ERROR_UNKNOWN;
    }

    switch(eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegInviteConnStateChangedEv - CallLeg 0x%p received a connection closed event",pCallLeg));
        if (pInvite->hClientResponseConnBackUp == hConn)
        {
            pInvite->hClientResponseConnBackUp = NULL;
        }
        if (pInvite->hActiveConn == hConn)
        {
            pInvite->hActiveConn = NULL;
        }
        break;
    default:
        break;
    }
    CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}
/***************************************************************************
 * CallLegInviteAttachOnInviteConnections
 * ------------------------------------------------------------------------
 * General: Attach the invite object as the transaction connections owner.
 *          We are doing it, because the transaction will be terminated in 
 *          a second, and so if the transaction is the only owner, the connections
 *          will be set to the 'not-in-use' connections list. 
 *          2xx retransmissions, however, may still be received on the connections.
 *          If the invite object is an owner too, only when it detaches from
 *          the connection (after invite linger timeout) the connection is set
 *          to the list, and 2xx is not expected to be received on it.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus CallLegInviteAttachOnInviteConnections(IN  CallLeg        *pCallLeg,
                                                IN CallLegInvite     *pInvite,
                                                IN RvSipTranscHandle  hTransc)
{

    RvStatus rv;
    RvSipTransportConnectionEvHandlers evHandler;
    RvSipTransportConnectionHandle hConn = NULL;
    
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler = CallLegInviteConnStateChangedEv;

    /*active connection*/
    rv = RvSipTransactionGetConnection(hTransc, &hConn);
    if(rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteAttachOnInviteConnections - Call-leg 0x%p failed to get client-response-conn-backup",
            pCallLeg));
    }
    else if(NULL != hConn)
    {
        /*attach to new conn*/
        rv = SipTransportConnectionAttachOwner(hConn,
                                            (RvSipTransportConnectionOwnerHandle)pInvite,
                                            &evHandler,sizeof(evHandler));
        
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInviteAttachOnInviteConnections - Failed to attach invite 0x%p of Call-leg 0x%p to connection 0x%p",
                pInvite,pCallLeg,hConn));
            pInvite->hActiveConn = NULL;
        }
        else
        {
            pInvite->hActiveConn = hConn;
        }
    }

    /*hClientResponseConnBackUp*/
    rv = SipTransactionGetClientResponseConnBackUp(hTransc, &hConn);
    if(rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteAttachOnInviteConnections - Call-leg 0x%p failed to get client-response-conn-backup",
            pCallLeg));
    }
    else if(NULL != hConn)
    {
        /*attach to new conn*/
        rv = SipTransportConnectionAttachOwner(hConn,
                                               (RvSipTransportConnectionOwnerHandle)pInvite,
                                               &evHandler,sizeof(evHandler));

        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInviteAttachOnInviteConnections - Failed to attach invite 0x%p of Call-leg 0x%p to connection 0x%p",
                pInvite,pCallLeg,hConn));
            pInvite->hClientResponseConnBackUp = NULL;
        }
        else
        {
            pInvite->hClientResponseConnBackUp = hConn;
        }
    }
    
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif
    return rv;
}

/***************************************************************************
 * CallLegInviteDefineReInviteRejectCode
 * ------------------------------------------------------------------------
 * General: Define if we should reject a re-invite request, because of an
 *          active re-invite process.
 *
 * IMPORTANT: This function should be called ONLY in oldInviteHandling mode
 *
 * Return Value: 0 - no need to reject.
 *               491 or 500 - need to reject.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 ***************************************************************************/
RvUint16 CallLegInviteDefineReInviteRejectCode(CallLeg*         pCallLeg,
                                               RvSipTranscHandle hTransc)
{
    RvSipTransactionState eTranscState;
    RvSipTransactionGetCurrentState(hTransc, &eTranscState);
    switch(eTranscState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT:
    case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDefineReInviteRejectCode - Call-leg 0x%p - is in the middle of an outgoing re-Invite. rejecting with 491",
            pCallLeg));
        return 491;            
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegInviteDefineReInviteRejectCode - Call-leg 0x%p - is in the middle of another incoming re-Invite. rejecting with 500",
            pCallLeg));
        return 500;
    default:
        return 0;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TerminateInviteEventHandler
 * ------------------------------------------------------------------------
 * General: Terminates an invite object, Free the resources taken by it and
 *          remove it from the manager call-leg list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     obj        - pointer to Call-leg to be deleted.
 *          reason    - reason of the call-leg termination
 ***************************************************************************/
static RvStatus RVCALLCONV TerminateInviteEventHandler(IN void    *obj, 
                                                       IN RvInt32  reason)
{
    CallLegInvite   *pInvite;
    CallLeg         *pCallLeg;
    RvStatus         retCode;

    pInvite = (CallLegInvite*)obj;
    if (pInvite == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    pCallLeg = pInvite->pCallLeg;

    retCode = CallLegLockMsg(pCallLeg);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "TerminateInviteEventHandler - Invite 0x%p of Call-leg 0x%p is out of termination queue...",
            pInvite, pCallLeg));

    if(pInvite->bInitialInvite == RV_FALSE)
    {
        /* for re-invite: change state to terminated */
        CallLegChangeModifyState(pCallLeg, pInvite,
                                RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED,
                                (RvSipCallLegStateChangeReason)reason);
    }

    CallLegInviteDestruct(pCallLeg, pInvite);

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        CallLegTerminateIfPossible(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    }

    CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}

/***************************************************************************
* CallLegRemoveInviteObj
* ------------------------------------------------------------------------
* General: Creates a new invite object.
*
* Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
*               RV_OK        - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs      - Handle to the subscription, relates to the new notification.
*          hAppNotify - Handle to the application notification object.
* Output:  phNotify   - Handle to the new created notification object.
***************************************************************************/
static RvStatus RVCALLCONV CallLegRemoveInviteObj(IN  CallLeg*        pCallLeg,
                                                  IN  CallLegInvite  *pInvite)
{
    RLIST_POOL_HANDLE    hInvitePool;
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    CallLegInvite*       pFoundInvite;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops;
    
    if(pInvite == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
    maxLoops = MAX_LOOP(pCallLeg);
    
    /*remove invite object from call-leg's list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    while(pItem != NULL &&  safeCounter < maxLoops)
    {
        pFoundInvite = (CallLegInvite*)pItem;
        if(pFoundInvite == pInvite)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRemoveInviteObj - call-leg 0x%p - invite obj 0x%p is removed",
                pCallLeg, pInvite));
            
            RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
            RLIST_Remove(hInvitePool, pCallLeg->hInviteObjsList, pItem);
            RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
            return RV_OK;
        }
        else
        {
            RLIST_GetNext(hInvitePool, pCallLeg->hInviteObjsList, pItem, &pNextItem);
            pItem = pNextItem;
            safeCounter++;
        }
    }
    
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRemoveInviteObj - Call-leg 0x%p - Reached MaxLoops %d, infinite loop",
            pCallLeg,maxLoops));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}


/***************************************************************************
 * CreateAckMsg
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
static RvStatus RVCALLCONV CreateAckMsg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    OUT RvBool*   pbSendToFirstRouteHeader)
{
    RvStatus rv;
    
    if(pInvite->hAckMsg == NULL)
    {
        rv = RvSipMsgConstruct(pCallLeg->pMgr->hMsgMgr,
                           pCallLeg->pMgr->hMsgMemPool,
                           &pInvite->hAckMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CreateAckMsg: Call-leg 0x%p invite 0x%p - Failed to construct an ACK message. rv=%d",
                pCallLeg, pInvite, rv));
            return rv;
        }
    }

    rv = FillAckMsgFromCallLeg(pCallLeg, pInvite, pbSendToFirstRouteHeader);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CreateAckMsg: Call-leg 0x%p - Failed to fill ACK message. rv=%d",
            pCallLeg, rv));
        RvSipMsgDestruct(pInvite->hAckMsg);
        pInvite->hAckMsg = NULL;
        return rv;
    }


    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CreateAckMsg: Call-leg 0x%p invite 0x%p - ACK message was build successfully",
            pCallLeg, pInvite));
    return RV_OK;
}

/***************************************************************************
 * FillAckMsgFromCallLeg
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 ***************************************************************************/
static RvStatus RVCALLCONV FillAckMsgFromCallLeg (
                                    IN CallLeg*         pCallLeg,
                                    IN CallLegInvite*   pInvite,
                                    OUT RvBool*   pbSendToFirstRouteHeader)
{
    RvStatus rv;
    RvSipAddressHandle        hReqURI;
    RvSipPartyHeaderHandle    hFrom;
    RPOOL_Ptr                 strCallId;
    RvSipPartyHeaderHandle    h2xxToHeader;
    
    rv = CallLegGetRequestURI(pCallLeg, NULL, &hReqURI, pbSendToFirstRouteHeader);
    if (rv != RV_OK || hReqURI == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "FillAckMsgFromCallLeg - Call-leg 0x%p Invite 0x%p -  ACK on 2xx - no request URI available"
            ,pCallLeg, pInvite));
        return RV_ERROR_UNKNOWN;
    }
    
    if(pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_OUTGOING)
    {
        hFrom = pCallLeg->hFromHeader;
    }
    else
    {
        hFrom = pCallLeg->hToHeader;
    }
    
    strCallId.hPage = pCallLeg->hPage;
    strCallId.hPool = pCallLeg->pMgr->hGeneralPool;
    strCallId.offset = pCallLeg->strCallId;

    /* 17.1.1.3 Construction of the ACK Request
    ... The To header field in the ACK MUST equal the To header field in the
    response being acknowledged, and therefore will usually differ from
    the To header field in the original request by the addition of the
    tag parameter.
    --> we copy the TO header we saved from the 2xx response to the ACK request.
    */
       
    h2xxToHeader = pInvite->hToFrom2xx;
       
    rv = SipTransactionFillBasicRequestMessage(hReqURI, 
                                                hFrom, h2xxToHeader, 
                                                &strCallId, 
                                                RVSIP_METHOD_ACK, NULL, 
                                                &(pInvite->cseq), 
                                                RV_TRUE, 
                                                pInvite->hAckMsg);
   

    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "FillAckMsgFromCallLeg - Call-leg 0x%p Invite 0x%p, Failed to fill the ACK message"
            ,pCallLeg, pInvite));
        return RV_ERROR_UNKNOWN;
    }

    /* route, contact will be set in call-leg msgToSend */
    return RV_OK;
}

/***************************************************************************
 * CreateAckTransmitter
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
 static RvStatus RVCALLCONV CreateAckTransmitter(IN CallLeg*         pCallLeg,
                                                 IN CallLegInvite*   pInvite)
{
    RvStatus rv;

    /* evHandlers pool and page for the transmitter... */
    rv = SipTransmitterMgrCreateTransmitter(pCallLeg->pMgr->hTrxMgr,
                                            (RvSipAppTransmitterHandle)pInvite,
                                            &(pCallLeg->pMgr->trxEvHandlers),
                                            NULL,NULL,
                                            pCallLeg->tripleLock,
                                            &pInvite->hTrx);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CreateAckTransmitter: call-leg 0x%p - failed to create new transmitter. rv=%d",
            pCallLeg, rv));
        return rv;
    }

    /* set outbound proxy in the transmitter */
    rv = SetParamsInAckTransmitter(pCallLeg, pInvite);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CreateAckTransmitter: call-leg 0x%p - failed to set transmitter parameters. rv=%d",
            pCallLeg, rv));
        SipTransmitterDestruct(pInvite->hTrx);
        pInvite->hTrx = NULL;
        return rv;
    }

    /* add via to ACK message */
    rv = SipTransmitterAddNewTopViaToMsg(pInvite->hTrx, pInvite->hAckMsg, NULL, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CreateAckTransmitter: call-leg 0x%p - transmitter 0x%p failed to add top via to ACK msg. rv=%d",
            pCallLeg, pInvite->hTrx, rv));
        SipTransmitterDestruct(pInvite->hTrx);
        pInvite->hTrx = NULL;
        return rv;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    /* set the CallLeg SCTP message sending parameters to the transmitter */
    SipTransmitterSetSctpMsgSendingParams(pInvite->hTrx,&pCallLeg->sctpParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
    /* Set Security Object to be used for dialog message protection */
    if (NULL != pCallLeg->hSecObj)
    {
        rv = RvSipTransmitterSetSecObj(pInvite->hTrx, pCallLeg->hSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CreateAckTransmitter - call-leg 0x%p - failed to set Security Object %p into Transmitter %p(rv=%d:%s)",
                pCallLeg, pCallLeg->hSecObj, pInvite->hTrx, rv,
                SipCommonStatus2Str(rv)));
            SipTransmitterDestruct(pInvite->hTrx);
            pInvite->hTrx = NULL;
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    return RV_OK;
}

/***************************************************************************
 * SetParamsInAckTransmitter
 * ------------------------------------------------------------------------
 * General: This function creates an ACK message, and save it in the call-leg
 *          object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hResponseMsg - The received 1xx/2xx response message.
 ***************************************************************************/
static RvStatus RVCALLCONV SetParamsInAckTransmitter(CallLeg*       pCallLeg,
                                                     CallLegInvite* pInvite)
{
    RvStatus rv;

    /* set local address, persistence, compartment */
    SipTransmitterSetLocalAddressesStructure(pInvite->hTrx, &pCallLeg->localAddr);

    rv = SipTransmitterSetConnection(pInvite->hTrx,pCallLeg->hTcpConn);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetParamsInAckTransmitter: call-leg 0x%p - failed to set tcp connection in transmitter 0x%p. rv=%d",
            pCallLeg, pInvite->hTrx, rv));
        return rv;
    }

    SipTransmitterSetPersistency(pInvite->hTrx,pCallLeg->bIsPersistent);

#ifdef RV_SIGCOMP_ON
    if (pCallLeg->bSigCompEnabled == RV_FALSE)
    {
        rv = SipTransmitterDisableCompression(pInvite->hTrx);
		if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"SetParamsInAckTransmitter: call-leg 0x%p - failed to set disable compression in transmitter. rv=%d",
				pCallLeg, rv));
			SipTransmitterDestruct(pInvite->hTrx);
			pInvite->hTrx = NULL;
            return rv;  /*error message will be printed in the function*/
        }
    }
#endif /* RV_SIGCOMP_ON */
    {
        RvBool   bInitialRequest = CallLegIsInitialRequest(pCallLeg);
        if(RV_FALSE == bInitialRequest)
        {
            /* we shall not use the outbound proxy for ACK, since this is not an initial request.
               (proxies that want to get non-initial requests too, should 
               add a record-route header...) */
            RvSipTransmitterSetIgnoreOutboundProxyFlag(pInvite->hTrx,RV_TRUE);
        }
    }
    if(pCallLeg->bForceOutboundAddr == RV_TRUE)
    {
		/*set the outbound address and force it*/
		/* set the call-leg outbound proxy to the transaction */
		rv = SipTransmitterSetOutboundAddsStructure(pInvite->hTrx, &pCallLeg->outboundAddr);
		if(rv != RV_OK)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"SetParamsInAckTransmitter: call-leg 0x%p - failed to set outbound address is transmitter 0x%p. rv=%d",
				pCallLeg, pInvite->hTrx, rv));
			return rv;  /*error message will be printed in the function*/
		}
        RvSipTransmitterSetForceOutboundAddrFlag(pInvite->hTrx,pCallLeg->bForceOutboundAddr);
    }
   return RV_OK;
}

/***************************************************************************
* HandleInviteAckTimerTimeout
* ------------------------------------------------------------------------
* General: Called when ever a the ack transmitter timer expires.
*          On expiration we destruct all 'forked ack' parameters.
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pContext - The call-Leg this timer was called for.
***************************************************************************/
RvBool HandleInviteAckTimerTimeout(IN void *pContext,
                                   IN RvTimer *timerInfo)
{
    CallLegInvite*  pInvite = (CallLegInvite*)pContext;
    CallLeg* pCallLeg = pInvite->pCallLeg;

    if (RV_OK != CallLegLockMsg(pCallLeg))
    {
        CallLegInviteTimerRelease(pInvite);
        return RV_FALSE;
    }
    
    if(SipCommonCoreRvTimerIgnoreExpiration(&(pInvite->timer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc  ,
            "HandleInviteAckTimerTimeout - Call-Leg 0x%p, invite 0x%p: ack timer expired but was already released. do nothing...",
            pCallLeg, pInvite));
        
		CallLegUnLockMsg(pCallLeg);
        return RV_FALSE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
        "HandleInviteAckTimerTimeout - Call-Leg  0x%p invite 0x%p: releasing timer...",
        pCallLeg, pInvite));
    
    /* The ACK timer expired - we can destruct the invite object */
    CallLegInviteTimerRelease(pInvite);

    RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
        "HandleInviteAckTimerTimeout - Call-Leg  0x%p invite 0x%p: terminating invite object...",
        pCallLeg, pInvite));

    CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT);
    CallLegUnLockMsg(pCallLeg);
    return RV_TRUE;
}


/***************************************************************************
 * HandleTrxNewConnInUseState
 * ------------------------------------------------------------------------
 * General: Handles the Transmitter state RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    -     The call-leg handle
 *            eReason     -     The reason for the state change.
 *            pExtraInfo  -     specific information for the new state.
 ***************************************************************************/
static void RVCALLCONV HandleTrxNewConnInUseState(CallLeg*               pCallLeg,
                                                  RvSipTransmitterReason eReason,
                                                  void*                  pExtraInfo)
{
    RvStatus rv = RV_OK;
    RvSipTransmitterNewConnInUseStateInfo *pStateInfo =
        (RvSipTransmitterNewConnInUseStateInfo*)pExtraInfo;

    RvBool bNewConnCreated =
        (eReason == RVSIP_TRANSMITTER_REASON_NEW_CONN_CREATED)?RV_TRUE:RV_FALSE;
    
    if(pCallLeg->bIsPersistent == RV_TRUE)
    {
        /*replace the active connection*/
        CallLegDetachFromConnection(pCallLeg);
        rv = CallLegAttachOnConnection(pCallLeg, pStateInfo->hConn);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "HandleTrxNewConnInUseState - CallLeg 0x%p failed to attach to new connection 0x%p rv=%d",
                pCallLeg,pStateInfo->hConn,rv));
            return;
        }
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "HandleTrxNewConnInUseState - CallLeg 0x%p will use a new connection 0x%p",
            pCallLeg,pStateInfo->hConn));
    }
    CallLegCallbackNewConnInUseEv(pCallLeg,pStateInfo->hConn,bNewConnCreated);
}


/***************************************************************************
 * HandleTrxDestResolvedState
 * ------------------------------------------------------------------------
 * General: Handles the Transmitter state RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    -     The call-leg handle
 *            eReason     -     The reason for the state change.
 *            pExtraInfo  -     specific information for the new state.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTrxDestResolvedState(
											IN  CallLeg*                 pCallLeg,
											IN  RvSipTransmitterHandle   hTrx,
											IN  RvSipMsgHandle           hMsg)
{
	RvStatus    rv;   

#ifdef RV_SIP_IMS_ON 
	CallLegSecAgreeDestResolved(pCallLeg, hMsg, hTrx, NULL);
#else /* RV_SIP_IMS_ON */
	RV_UNUSED_ARG(hTrx);
#endif /*RV_SIP_IMS_ON */

	/*notify the application that the message destination address was resolved*/
	rv = CallLegCallbackFinalDestResolvedEv(pCallLeg,NULL,hMsg);
	if(rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "HandleTrxDestResolvedState - CallLeg 0x%p message will not be sent, FinalDestResolvedEv returned with rv=%d",
				   pCallLeg,rv));
		return rv;
	}
    return RV_OK;

}


/***************************************************************************
 * HandleTrxMsgSendFailureState
 * ------------------------------------------------------------------------
 * General: Handles the Transmitter state RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    -     The call-leg handle
 *            pInvite     -     The invite object
 *            hMsg        -     The message who failed to be sent
 *            eReason     -     The reason for the state change.
 ***************************************************************************/
static void RVCALLCONV HandleTrxMsgSendFailureState(CallLeg*               pCallLeg,
                                                    CallLegInvite*         pInvite,
													RvSipMsgHandle         hMsg,
                                                    RvSipTransmitterReason eReason)
{
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "HandleTrxMsgSendFailureState - CallLeg 0x%p, invite 0x%p: 200 OK/ACK Message failed to be sent. nothing to do",
        pCallLeg, pInvite));

#ifdef RV_SIP_IMS_ON
	CallLegSecAgreeMsgSendFailure(pCallLeg);    
#else
	RV_UNUSED_ARG(pCallLeg);
#endif

	RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(eReason);
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pInvite);
#endif
    return;

}

/***************************************************************************
 * InviteSendAckOn2xx
 * ------------------------------------------------------------------------
 * General: Sends an Ack message on 2xx to the remote party.(there is no transaction
 *          to use).
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The Ack was not sent.
 *               RV_OK - Ack message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - The call-leg.
 *          pInvite  - The invite object sending the Ack message.
 *          responseCode - the response code of the response message that triggered the ACK
 ***************************************************************************/
static RvStatus RVCALLCONV InviteSendAckOn2xx(
                            IN CallLeg          *pCallLeg,
                            IN CallLegInvite*   pInvite,
                            IN RvUint16         responseCode)
{
    RvStatus       rv;
    
    if(pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "InviteSendAckOn2xx - CallLeg 0x%p invite=NULL!!!",
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    /* There is no transaction, so we must call the msgToSend event handling from here.
       we shall do it only for first ACK sending, and not for every retransmission */
    if(pInvite->bFirstAck == RV_TRUE && pInvite->hTrx == NULL)
    {
        RvSipCallLegState eState;

        rv = CallLegInviteCreateAckMsgAndTrx(pCallLeg, pInvite);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "InviteSendAckOn2xx - CallLeg 0x%p pInvite 0x%p - failed to create ACK msg and trx",
                pCallLeg, pInvite));
            return rv;
        }

        /* Store state of callleg before call to application callback,
        done by CallLegMsg2xxAckMsgToSendHandler() */
        eState = pCallLeg->eState;

        rv = CallLegMsg2xxAckMsgToSendHandler(pCallLeg, responseCode, pInvite->hAckMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "InviteSendAckOn2xx - Call-leg 0x%p, pInvite 0x%p - Failed in CallLegMsg2xxAckMsgToSendHandler. rv=%d",
               pCallLeg,pInvite, rv));
            return rv;
        }

        /* Termination of object by application from ObjectCreatedEv, MsgSendEv
        and MsgRcvdEv callbacks is not supported by the current version of
        toolkit (4.5.0.18). Therefore if the CallLeg was terminated
        by the application from MsgSendEv callback, called by previous
        CallLegMsg2xxAckMsgToSendHandler(), no further processing can be done*/
		if (eState != pCallLeg->eState)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
						"InviteSendAckOn2xx - Call-leg 0x%p, pInvite 0x%p - CallLeg state was changed from MsgSend callback. Stop.",
						pCallLeg,pInvite));
            return RV_ERROR_UNKNOWN;
		}

        rv = SipTransmitterSendMessage(pInvite->hTrx, pInvite->hAckMsg, SIP_TRANSPORT_MSG_UNDEFINED);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "InviteSendAckOn2xx - Call-leg 0x%p, pInvite 0x%p - Failed to send ACK on transmitter 0x%p. rv=%d",
               pCallLeg, pInvite,pInvite->hTrx, rv));
            /* In error case SipTransmitterSendMessage() doesn't free the page*/
            RvSipMsgDestruct(pInvite->hAckMsg);
            pInvite->hAckMsg = NULL;
        }
        else
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "InviteSendAckOn2xx - Call-leg 0x%p, pInvite 0x%p - ACK was sent successfully",pCallLeg,pInvite));
        }

        /* setting the ACK timer is done only when trx notifies on the msg-sent state.
           (otherwise, the ACK won't have the time to be sent - NUCLEUS)*/
        /*CallLegInviteSetAckTimer(pCallLeg, pInvite);
         pInvite->bFirstAck = RV_FALSE;*/

        /* the message is now saved in the transmitter. no need to hold it in call-leg */
        pInvite->hAckMsg = NULL;
        
    }
    else /* not first ACK */
    {
        rv = SipTransmitterRetransmit(pInvite->hTrx);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "InviteSendAckOn2xx - Call-leg 0x%p, pInvite 0x%p - Failed to retransmit ACK on transmitter 0x%p. rv=%d",
               pCallLeg,pInvite, pInvite->hTrx, rv));
        }
        else
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "InviteSendAckOn2xx - Call-leg 0x%p,pInvite 0x%p -  ACK was retransmited successfully",
                pCallLeg,pInvite));
        }
    }

    return rv;
}

/***************************************************************************
 * InviteSendAckWithTransc
 * ------------------------------------------------------------------------
 * General: Sends an Ack message to the remote party. First the Call-leg's
 *          request URI is determined and then a Ack is sent using the
 *          transaction's API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The Ack was not sent.
 *               RV_OK - Ack message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 *            hTransc  - The transaction used to send the Ack message.
 *            pInvite  - The invite object sends the ACK.
 *          responseCode - the response code of the response message that trigered the ACK
 ***************************************************************************/
static RvStatus RVCALLCONV InviteSendAckWithTransc(
                            IN CallLeg                       *pCallLeg,
                            IN RvSipTranscHandle             hTransc,
                            IN RvUint16                     responseCode)
{
    RvStatus          rv;
    RvSipAddressHandle hReqURI;

    /*----------------------------------------------
       get the request URI from the call-leg
    ------------------------------------------------*/
    if(responseCode >= 200 && responseCode < 300)
    {
        rv = CallLegGetRequestURI(pCallLeg, hTransc, &hReqURI, NULL);
        if (rv != RV_OK || hReqURI == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "InviteSendAckWithTransc - Failed for Call-leg 0x%p, no request URI available"
                      ,pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        hReqURI = NULL; /* Take the request URI of the INVITE from the transaction */
    }
    rv = SipTransactionAck(hTransc,hReqURI);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "InviteSendAckWithTransc - Failed for Call-leg 0x%p, transc 0x%p failed to send Ack (error code %d)"
                  ,pCallLeg,hTransc,rv));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * RemoveOldInvitePendingInvites
 * ------------------------------------------------------------------------
 * General: This function removes pending (to be terminated) INVITE objects
 *		    from the call-leg INVITE objects list. This function was 
 *			main purpose is to avoid "inter-mixture" of the states of few
 *			INVITE objects which are handled by only ONE state-machine.
 *			This way there's only a single "active" INVITE object
 *			per call-leg within it's list if the function is called during
 *			new INVITE object creation.
 *
 * IMPORTANT: This function should be called ONLY in oldInviteHandling mode
 *
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. 
 *               RV_OK - The "cleaning" operation was successful
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the Ack message.
 ***************************************************************************/
static RvStatus RVCALLCONV RemoveOldInvitePendingInvites(IN CallLeg *pCallLeg,
                                                         OUT RvUint16 *pRejectCode) 
{
	RLIST_ITEM_HANDLE  listItem;
	CallLegInvite     *pInvite;
	RvStatus		   rv; 

	RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
	RLIST_GetHead(pCallLeg->pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList,&listItem);
	RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);

    *pRejectCode = 0;
	while (listItem != NULL)
	{
		pInvite = (CallLegInvite*)listItem;
        if(pInvite->hInviteTransc != NULL) /* in case of forked call-leg, the invite object
                                              might not have a transaction */
        {
            *pRejectCode = CallLegInviteDefineReInviteRejectCode(pCallLeg,pInvite->hInviteTransc);
            if(*pRejectCode > 0)
            {
                return RV_OK;
            }
        
            rv = CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pInvite->hInviteTransc);
		    if (RV_OK != rv) 
		    {
			    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				    "RemoveOldInvitePendingInvites - Call-leg 0x%p, invite object 0x%p failed to detach from transc 0x%p (error code %d)",
				    pCallLeg,pInvite,pInvite->hInviteTransc,rv));
			    return rv;
		    }
        }
        
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RemoveOldInvitePendingInvites - call-leg 0x%p. invite 0x%p - destructing the invite object...",
            pCallLeg, pInvite));

		rv = CallLegInviteDestruct(pCallLeg,pInvite);
		if (RV_OK != rv) 
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"RemoveOldInvitePendingInvites - Call-leg 0x%p failed to destruct invite object 0x%p (error code %d)",
				pCallLeg,pInvite,rv));
			return rv;
		}

		RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
		RLIST_GetHead(pCallLeg->pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList,&listItem);
		RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
	}	

	return RV_OK;
}

#endif /* ifndef RV_SIP_PRIMITIVES */
