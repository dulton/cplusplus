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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <TransactionTransport.c>
 *
 *  This file provides a method for identifing a destination address in a message.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20 Nov 2000
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "TransactionTransport.h"
#include "_SipTransactionTypes.h"
#include "_SipTransport.h"
#include "TransactionObject.h"
#include "TransactionTimer.h"
#include "RvSipTransportTypes.h"
#include "TransactionCallBacks.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipTransmitter.h"
#ifdef RV_SIP_IMS_ON
#include "TransactionSecAgree.h"
#endif /* RV_SIP_IMS_ON */
#ifdef RV_SIGCOMP_ON
#include "_SipCompartment.h"
#endif 


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV InitializeOutOfContextTransmitter(
                               IN    TransactionMgr*                pTranscMgr,
                               IN    RvSipTransmitterHandle         hTrx,
                               IN    RvChar*                        strViaBranch,
                               IN    RPOOL_Ptr*                     pViaBranch,
                               IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                               IN    RvChar                        *strLocalAddress,
                               IN    RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
                               IN    RvChar                        *if_name,
			       IN    RvUint16                       vdevblock,
#endif
                               IN    RvBool                         bIgnoreOutboundProxy,
                               IN    RvBool                         bSendToFirstRouteHeader,
                               IN    RvSipMsgType                   eMsgType);


static RvSipTransactionStateChangeReason RVCALLCONV ConvertTrxReasonToTranscReason(
                             IN RvSipTransmitterReason eTrxReason);
#ifdef RV_SIGCOMP_ON
static void AdaptToTrxCompartmentIfNeeded(
                         IN Transaction              *pTransc,
                         IN RvSipTransmitterHandle    hTrx);
#endif /* #ifdef RV_SIGCOMP_ON */ 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionTransportConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 *          The transaction will handle the received event.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hOwner  -   Handle to the connection owner.
 *          hConn   -   The connection handle
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTransportConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTransc,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{

    Transaction*    pTransc    = (Transaction*)hTransc;

    if(pTransc == NULL)
    {
        return RV_OK;
    }

    RV_UNUSED_ARG(eReason);
    if (TransactionLockMsg(pTransc) != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }
    switch(eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                     "TransactionTransportConnStateChangedEv - Transaction 0x%p received a connection 0x%p closed event",pTransc,hConn));
            if (pTransc->hActiveConn == hConn)
            {
                pTransc->hActiveConn = NULL;
            }
            else if(pTransc->hClientResponseConnBackUp == hConn)
            {
                pTransc->hClientResponseConnBackUp = NULL;
            }
            
            /* fix 5.0 - if the transaction has no owner (for example - notify transaction, is detached from the
              notification object, after state final-response-sent) - terminate it. */
            if(NULL == pTransc->pOwner)
            {
                RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                     "TransactionTransportConnStateChangedEv - Terminate Transaction 0x%p on connection 0x%p closed event",pTransc,hConn));
                if(RV_OK != TransactionTerminate(RVSIP_TRANSC_REASON_NETWORK_ERROR, pTransc))
                {
                    RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                     "TransactionTransportConnStateChangedEv - Failed to terminate Transaction 0x%p",pTransc));
                
                }
            }
        break;
    default:
        break;
    }
    TransactionUnLockMsg(pTransc);
    return RV_OK;

}

/***************************************************************************
 * TransactionTransportDetachAllConnections
 * ------------------------------------------------------------------------
 * General: Detaches the transaction from all connections.
 *          after calling this function the transaction will no longer receive TCP events
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - pointer to transaction
 ***************************************************************************/
void RVCALLCONV TransactionTransportDetachAllConnections(
                                    IN  Transaction         *pTransc)
{
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "TransactionTransportDetachAllConnections - Transaction 0x%p: All connections will be detached: (Active:0x%p, Resp backup = 0x%p)",
              pTransc,pTransc->hActiveConn,pTransc->hClientResponseConnBackUp));

    if (pTransc->hActiveConn != NULL)
    {
        RvSipTransportConnectionDetachOwner(pTransc->hActiveConn,
                                (RvSipTransportConnectionOwnerHandle)pTransc);
        pTransc->hActiveConn = NULL;
    }
    if (pTransc->hClientResponseConnBackUp != NULL)
    {

        RvSipTransportConnectionDetachOwner(pTransc->hClientResponseConnBackUp,
                                (RvSipTransportConnectionOwnerHandle)pTransc);
        pTransc->hClientResponseConnBackUp = NULL;
    }


}


/***************************************************************************
 * TransactionTransportTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the transaction of a Transmitter state change.
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -    The sip stack Transmitter handle
 *            hAppTrx - The transaction handle.
 *            eState  -      The new Transmitter state
 *            eReason -     The reason for the state change.
 *            hMsg    -     When the state relates to the outgoing message, the
 *                          message is supplied.
 *            pExtraInfo - specific information for the new state.
 ***************************************************************************/
void RVCALLCONV TransactionTransportTrxStateChangeEv(
       IN  RvSipTransmitterHandle            hTrx,
       IN  RvSipAppTransmitterHandle         hAppTrx,
       IN  RvSipTransmitterState             eState,
       IN  RvSipTransmitterReason            eReason,
       IN  RvSipMsgHandle                    hMsg,
       IN  void*                             pExtraInfo)

{
    Transaction*                      pTransc         = (Transaction *)hAppTrx;
    RvStatus                          rv              = RV_OK;
    RvSipTransactionStateChangeReason eTranscReason;
    RvSipTransport                    eDestAddrTransport = RVSIP_TRANSPORT_UNDEFINED;

    /* Get Transport Type to be used for message sending */
    if (RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED == eState ||
        RVSIP_TRANSMITTER_STATE_READY_FOR_SENDING   == eState)
    {
        if(pTransc->hTrx == NULL)
        {
            RvLogWarning(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "TransactionTransportTrxStateChangeEv - Transaction 0x%p - pTransc->hTrx is NULL (trx 0x%p changed state)",
                pTransc, hTrx));
        }
        else
        {
            SipTransportAddress destAddr;
            SipTransmitterGetDestAddr(pTransc->hTrx, &destAddr);
            eDestAddrTransport = destAddr.eTransport;
        }
    }

    RV_UNUSED_ARG(hTrx);
    switch (eState)
    {
    case RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED:
#ifdef RV_SIP_IMS_ON 
		TransactionSecAgreeDestResolved(pTransc, hMsg, hTrx);
#endif /* RV_SIP_IMS_ON */
		/*notify the application that the message destination address was resolved*/
        rv = TransactionCallbackFinalDestResolvedEv(pTransc,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "TransactionTransportTrxStateChangeEv - Transaction 0x%p message will not be sent, FinalDestResolvedEv returned with rv=%d",
                pTransc,rv));
            SipTransmitterMoveToMsgSendFailure(hTrx);
            return;
        }
        pTransc->eTranspType = eDestAddrTransport;
        break;
    case RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE:
        {
            RvSipTransmitterNewConnInUseStateInfo *pStateInfo =
                                    (RvSipTransmitterNewConnInUseStateInfo*)pExtraInfo;
            RvBool bNewConnCreated =
                   (eReason == RVSIP_TRANSMITTER_REASON_NEW_CONN_CREATED)?RV_TRUE:RV_FALSE;

            /*replace the active connection*/
            TransactionDetachFromConnection(pTransc,pTransc->hActiveConn,RV_TRUE);
            rv = TransactionAttachToConnection(pTransc,pStateInfo->hConn,RV_TRUE);
            if(rv != RV_OK)
            {
                RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "TransactionTransportTrxStateChangeEv - Transaction 0x%p failed to attach to new connection 0x%p rv=%d",
                    pTransc,pStateInfo->hConn,rv));
                return;
            }
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "TransactionTransportTrxStateChangeEv - Transaction 0x%p will use a new connection 0x%p",
                    pTransc,pStateInfo->hConn));

            TransactionCallbackNewConnInUseEv(pTransc,pStateInfo->hConn,bNewConnCreated);
        }
        break;
    case RVSIP_TRANSMITTER_STATE_MSG_SENT:
        {
            SipTransactionMsgSentOption eOption  = SIP_TRANSACTION_MSG_SENT_OPTION_UNDEFINED;
            SipTransportMsgType         eMsgType = ((SipTransportSendInfo*)pExtraInfo)->msgType;
            
#ifdef RV_SIGCOMP_ON
            AdaptToTrxCompartmentIfNeeded(pTransc,hTrx);
#endif /* #ifdef RV_SIGCOMP_ON */ 

            if(eMsgType == SIP_TRANSPORT_MSG_ACK_SENT)
            {
                pTransc->bAckSent = RV_TRUE;
                eOption = SIP_TRANSACTION_MSG_SENT_OPTION_ACK;
            }

            /* If MSG_SENT was reported for retransmitted 18X, check
            if the PRACK sent in response to previous 18X was already handled
            and, as a result of this, the Transaction was moved to next states.
            In this case the MSG_SENT notification should be ignored. */
            if(eMsgType == SIP_TRANSPORT_MSG_PROV_RESP_REL_SENT &&
               pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT)
            {
                RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "TransactionTransportTrxStateChangeEv - Transaction 0x%p - ignore MSG_SENT notification for 18X retransmission: PRACK was received / 200 INV was sent",
                    pTransc));
                break;
            }

            TransactionTimerSetTimers(pTransc,((SipTransportSendInfo*)pExtraInfo)->msgType);
           
#ifndef RV_SIP_PRIMITIVES			
			/* The call-leg needs to take control over the transaction when 2xx is sent for
			new invite. It will use the transaction transmitter to send retransmissions. The 
			transaction will be kept alive to handle INVITE retransmissions but will not handle
			incoming ACK*/
			if(pTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT &&
				pTransc->responseCode<300 && pTransc->responseCode>=200 &&
				pTransc->pMgr->bOldInviteHandling == RV_FALSE && 
				((SipTransportSendInfo*)pExtraInfo)->msgType == SIP_TRANSPORT_MSG_FINAL_RESP_SENT)
            {
                eOption = SIP_TRANSACTION_MSG_SENT_OPTION_2XX;
            }
            
            if(eOption != SIP_TRANSACTION_MSG_SENT_OPTION_UNDEFINED &&
               SIP_TRANSACTION_OWNER_CALL == pTransc->eOwnerType )
            {
                /* 2xx on invite was sent for the first time, so according to new invite handling,
                the transaction is going to be terminated.
                in this case we ought to inform the call-leg, so it will take control over
                the 2xx transmitter, for 2xx-retransmissions */
                RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "TransactionTransportTrxStateChangeEv - Transaction 0x%p - tell call-leg about 2xx/Ack sending",
                    pTransc));
                TransactionCallbackCallInviteTranscMsgSentEv(pTransc, eOption);
            }
#endif /* RV_SIP_PRIMITIVES */
        }
        break;
    case RVSIP_TRANSMITTER_STATE_READY_FOR_SENDING:
        pTransc->eTranspType = eDestAddrTransport;
        TransactionUpdateTopViaFromMsg(pTransc,hMsg);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        if(pTransc->pMgr && pTransc->pMgr->pAppEvHandlers.pfnEvMsgToSendFinal) {
          pTransc->pMgr->pAppEvHandlers.pfnEvMsgToSendFinal(
            (RvSipTranscHandle)pTransc,
            (RvSipTranscOwnerHandle)pTransc->pOwner,
            hMsg);
        }
#endif
/* SPIRENT END */
        break;

    case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionTransportTrxStateChangeEv - pTransc=0x%p: Message failed to be sent (rv=%d:%s)",
            pTransc, rv, SipCommonStatus2Str(rv)));
        
        eTranscReason = ConvertTrxReasonToTranscReason(eReason);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        /* if we support sent-failure we fail on the sending */
        /* Call to send failure callback - we only call with client transactions */
        if ((pTransc->pEvHandlers)->pfnEvStateChanged != NULL &&
            RV_TRUE == pTransc->bIsUac &&
            pTransc->eMethod != SIP_TRANSACTION_METHOD_CANCEL &&
            (eReason == RVSIP_TRANSMITTER_REASON_NETWORK_ERROR ||
             eReason == RVSIP_TRANSMITTER_REASON_CONNECTION_ERROR))
        {
            SipTransportMsgType eMsgType = (SipTransportMsgType)((RvIntPtr)pExtraInfo);

            /* we do not handle msg sent failure for Ack */
            if (SIP_TRANSPORT_MSG_REQUEST_SENT == eMsgType)
            {
                /* we want to stop retransmissions for the transaction */
                TransactionTimerReleaseAllTimers(pTransc);
                TransactionHandleMsgSentFailureEvent(pTransc,RVSIP_TRANSC_REASON_NETWORK_ERROR);
            }
            else
            {
                rv = TransactionTerminate(eTranscReason,pTransc);
            }
        }
        else
        {
            rv = TransactionTerminate(eTranscReason,pTransc);
        }
#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
        /* if send failure is not supported, the transaction will terminate */
        {
#ifdef RV_SIP_IMS_ON
			/* The security-agreement is notified on message send failure */
			TransactionSecAgreeMsgSendFailure(pTransc);
#endif /* #ifdef RV_SIP_IMS_ON */
            TransactionTerminate(eTranscReason,pTransc);
        }
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    break;
    default:
        break;
    }
    return;
}

/*****************************************************************************
 * TransactionTransportTrxOtherURLAddressFoundEv
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
RvStatus RVCALLCONV TransactionTransportTrxOtherURLAddressFoundEv(
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    pbAddressResolved)
{
    Transaction*        pTransc = (Transaction*)hAppTrx;
    RvStatus            rv      = RV_OK;

    RV_UNUSED_ARG(hTrx);
    rv = TransactionCallbackOtherURLAddressFoundEv(
                                                    pTransc,
                                                    hMsg,
                                                    hAddress,
                                                    hSipURLAddress,
                                                    pbAddressResolved);

    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionTransportTrxOtherURLAddressFoundEv - pTransc 0x%p: Appliaction failed to handle unknown URL address(rv=%d)",pTransc, rv));
        return rv;
    }
    return rv;
}


/***************************************************************************
 * TransactionTransportOutOfContextTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the transaction of a Transmitter state change.
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -    The sip stack Transmitter handle
 *            hAppTrx - The transaction handle.
 *            eState  -      The new Transmitter state
 *            eReason -     The reason for the state change.
 *            hMsg    -     When the state relates to the outgoing message, the
 *                          message is supplied.
 *            pExtraInfo - specific information for the new state.
 ***************************************************************************/
void RVCALLCONV TransactionTransportOutOfContextTrxStateChangeEv(
                                   IN  RvSipTransmitterHandle            hTrx,
                                   IN  RvSipAppTransmitterHandle         hAppTrx,
                                   IN  RvSipTransmitterState             eState,
                                   IN  RvSipTransmitterReason            eReason,
                                   IN  RvSipMsgHandle                    hMsg,
                                   IN  void*                             pExtraInfo)

{
    TransactionMgr*   pTranscMgr = (TransactionMgr*)hAppTrx;
    RvStatus          rv         = RV_OK;

    RV_UNUSED_ARG(eReason);
    RV_UNUSED_ARG(pExtraInfo);

    if(eState == RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED)
    {
        rv = TransactionCallbackMgrFinalDestResolvedEv(pTranscMgr,hMsg,hTrx);
        if(rv != RV_OK)
        {
            RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionTransportOutOfContextTrxStateChangeEv - TranscMgr= 0x%p, FinalDestResolvedEv returned error(rv=%d) terminating transmitter 0x%p",
                    pTranscMgr,rv,hTrx));
            RvSipTransmitterTerminate(hTrx);
        }
    }

    if(eState == RVSIP_TRANSMITTER_STATE_MSG_SENT ||
       eState == RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE)
    {
        RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionTransportOutOfContextTrxStateChangeEv - TranscMgr= 0x%p, destructs transmitter 0x%p",
                    pTranscMgr,hTrx));
        RvSipTransmitterTerminate(hTrx);
    }
}


/***************************************************************************
 * TransactionTransportOutOfContextTrxOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a message needs to be sent and
 *          the destination address is a URL type that is currently not
 *          supported by the stack. The URL has to be converted to a SIP URL
 *          for the message to be sent.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTrx           - The sip stack Transmitter handle
 *        hAppTrx        - The transaction manager.
 *        hMsg           - The message that includes the other URL address.
 *        hAddress       - Handle to unsupport address to be converted.
 * Output:hSipURLAddress - Handle to the SIP URL address - this is an empty
 *                         address object that the application should fill.
 *        pbAddressResolved -Indication wether the SIP URL address was filled.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTransportOutOfContextTrxOtherURLAddressFoundEv (
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool                    *pbAddressResolved)
{
    TransactionMgr*   pTranscMgr = (TransactionMgr*)hAppTrx;
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hTrx);
#endif	
    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
               "TransactionTransportOutOfContextTrxOtherURLAddressFoundEv - TranscMgr= 0x%p, Transmitter 0x%p, hMsg = 0x%p",
               pTranscMgr,hTrx,hMsg));
    return TransactionCallbackMgrOtherURLAddressFoundEv(pTranscMgr,hMsg,hAddress,
                                                        hSipURLAddress,pbAddressResolved);
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "RvSipSecAgree.h"
#include "RvSipSecurity.h"
#include "RvSipCallLeg.h"

static RvStatus RVCALLCONV TransactionCheckSecAgree(Transaction*   pTransc,
                                                    RvBool isResp) {
  RvStatus rv=RV_OK;
  
  if(pTransc) {
    
    if(pTransc->hSecAgree && pTransc->hTrx) {
      
      Transmitter        *pTrx=(Transmitter*)pTransc->hTrx;
      
      RvSipSecObjHandle soact=pTransc->hSecObj;
      RvSipSecObjHandle sotrx=pTrx->hSecObj;
      RvSipSecObjHandle sosa=NULL;
      
      RvSipSecAgreeGetSecObj(pTransc->hSecAgree,&sosa);
      
      if(sotrx && !RvSipSecObjIsReady(sotrx,isResp)) {
        if(soact && RvSipSecObjIsReady(soact,isResp)) {
          RvSipTransmitterSetSecObj(pTransc->hTrx,soact);
        } else if(sosa && RvSipSecObjIsReady(sosa,isResp)) {
          RvSipTransmitterSetSecObj(pTransc->hTrx,sosa);
        } else {
          if(pTransc->pOwner) {
            if(pTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL) {
              RvSipCallLegHandle cl=pTransc->pOwner;
              RvSipSecAgreeHandle sa=NULL;
              RvSipCallLegGetSecAgree(cl,&sa);
              if(sa) {
                RvSipSecAgreeGetSecObj(sa,&sosa);
                if(sosa && RvSipSecObjIsReady(sosa,isResp)) {
                  RvSipTransmitterSetSecObj(pTransc->hTrx,sosa);
                }
              }
            }
          }
        }
      }
    }
  }
  
  return rv;
}

#endif
/* SPIRENT_END */

/***************************************************************************
* TransactionTransportSendMsg
* ------------------------------------------------------------------------
* General: Find the address to send the message within the message itself,
*          encode the message, and send it to this address.
* Return Value: RV_ERROR_NULLPTR - pTransc or hMsgToSend are NULL pointers.
*               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
*               RV_ERROR_UNKNOWN - a failure occured.
* ------------------------------------------------------------------------
* Arguments:
* Input: pTransc  - The transaction to send message with.
*        bResolveAddr - RV_TRUE if we want the transmitter to resolve the dest addr
*                       from the message, RV_FALSE if the transmitter should use the
*                       address it currently hold.
* Output: TransactionTransportSendMsg
***************************************************************************/
RvStatus TransactionTransportSendMsg(IN  Transaction    *pTransc,
                                     IN  RvBool          bResolveAddress)
{
    RvStatus                    rv                      = RV_OK;

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "TransactionTransportSendMsg - Transaction 0x%p is about to send message 0x%p. msgType = %s, bResolveAddress = %d",
              pTransc,pTransc->hOutboundMsg,SipTransportGetMsgTypeName(pTransc->msgType),bResolveAddress));

    if(pTransc->hTrx == NULL)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionTransportSendMsg - Transaction 0x%p no transmitter object in the transaction!!!",
            pTransc));
        return RV_ERROR_UNKNOWN;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(pTransc->hOutboundMsg) {
      TransactionCheckSecAgree(pTransc,
        (RvSipMsgGetMsgType(pTransc->hOutboundMsg)==RVSIP_MSG_RESPONSE));
    }
#endif
/* SPIRENT_END */

    if(bResolveAddress == RV_TRUE)
    {

        rv = SipTransmitterSendMessage(pTransc->hTrx, pTransc->hOutboundMsg,
                                       pTransc->msgType);
    }
    else
    {
        rv = SipTransmitterSendMessageToPreDiscovered(pTransc->hTrx,pTransc->hOutboundMsg,
                                                      pTransc->msgType);
    }

    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionTransportSendMsg - Transaction 0x%p failed to send message 0x%p",
            pTransc,pTransc->hOutboundMsg));
        /* In error case SipTransmitterSendMessage() doesn't free the page*/
        if(bResolveAddress == RV_TRUE)
        {
            RvSipMsgDestruct(pTransc->hOutboundMsg);
        }
    }
    pTransc->hOutboundMsg = NULL;
    return rv;
}


/***************************************************************************
 * TransactionTransportRetransmitMessage
 * ------------------------------------------------------------------------
 * General: Retransmit the last message sent by the transaction
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to retransmit from.
 ***************************************************************************/

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
int RvRetransmitsCounter = 0;

/**
 * Converts transaction method to SIP method for the Retransmit callback 
 * @param method transaction method
 * @return SIP method
 */
static RvSipMethodType TransactionMethodToSipMethod(SipTransactionMethod method)
{
    switch(method) {
    case SIP_TRANSACTION_METHOD_UNDEFINED: return RVSIP_METHOD_UNDEFINED;
    case SIP_TRANSACTION_METHOD_INVITE: return RVSIP_METHOD_INVITE;
    case SIP_TRANSACTION_METHOD_BYE: return RVSIP_METHOD_BYE;
    case SIP_TRANSACTION_METHOD_REGISTER: return RVSIP_METHOD_REGISTER;
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER: return RVSIP_METHOD_REFER;
    case SIP_TRANSACTION_METHOD_NOTIFY: return RVSIP_METHOD_NOTIFY;
    case SIP_TRANSACTION_METHOD_SUBSCRIBE: return RVSIP_METHOD_SUBSCRIBE;
#endif /* RV_SIP_SUBS_ON */
    case SIP_TRANSACTION_METHOD_CANCEL: return RVSIP_METHOD_CANCEL;
#ifndef RV_SIP_PRIMITIVES
    case SIP_TRANSACTION_METHOD_PRACK: return RVSIP_METHOD_PRACK;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    case SIP_TRANSACTION_METHOD_OTHER: return RVSIP_METHOD_OTHER;
    default: return RVSIP_METHOD_UNDEFINED;
    }
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
RvStatus TransactionTransportRetransmitMessage(IN Transaction *pTransc)
{
    RvStatus           rv = RV_OK;

#ifdef RV_SIGCOMP_ON
    RvBool             bZeroRetrans = RV_FALSE;

    rv = SipTransmitterMatchSentOutboundMsgToNextMsgType(
                                            pTransc->hTrx,&bZeroRetrans);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionTransportRetransmitMessage - Transaction 0x%p: Transmitter 0x%p failed to match the hSentOutboundMsg to next msg type",
                  pTransc,pTransc->hTrx));
        return rv;
    }
    if (bZeroRetrans == RV_TRUE)
    {
        pTransc->retransmissionsCount = 0;
    }
#endif /* RV_SIGCOMP_ON */

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    RvRetransmitsCounter++;

    if (pTransc->pMgr->pfnRetransmitEvHandler) {
#if defined(UPDATED_BY_SPIRENT_ABACUS)       
        pTransc->pMgr->pfnRetransmitEvHandler(TransactionMethodToSipMethod(pTransc->eMethod), pTransc->cctContext);
#else
        if(pTransc->pOwner &&
           pTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL &&
           pTransc->msgType == SIP_TRANSPORT_MSG_REQUEST_SENT)
           pTransc->pMgr->pfnRetransmitEvHandler(TransactionMethodToSipMethod(pTransc->eMethod), pTransc->pOwner);
#endif        
    }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    rv = SipTransmitterRetransmit(pTransc->hTrx);

    if (RV_OK == rv)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                 "TransactionTransportRetransmitMessage - Transaction 0x%p: A message was successfully retransmitted.",
                 pTransc));
    }
    else
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionTransportRetransmitMessage - Transaction 0x%p: Error in trying to retransmit a message in transaction",
                  pTransc));
        return rv;
    }

#ifdef SIP_DEBUG
    /* Update statistics */
    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD:
        {
            pTransc->pMgr->msgStatistics.sentINVITE += 1;
            pTransc->pMgr->msgStatistics.sentINVITERetrans += 1;
            break;
        }
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING:
        {
            pTransc->pMgr->msgStatistics.sentNonInviteReq += 1;
            pTransc->pMgr->msgStatistics.sentNonInviteReqRetrans += 1;
            break;
        }
    case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT:
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
#endif /*#ifndef RV_SIP_PRIMITIVES*/
        {
            pTransc->pMgr->msgStatistics.sentResponse += 1;
            pTransc->pMgr->msgStatistics.sentResponseRetrans += 1;
            break;
        }
    default:
        break;
    }
#endif /*SIP_DEBUG*/


    return rv;
}


/***************************************************************************
* TransactionTransportSendOutofContextMsg
* ------------------------------------------------------------------------
* General: Send a given message that is not related to the transaction.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: TranscpMgr         - The transaction manager handle.
*        hMsgToSend         - The message to send.
*        bHandleTopVia      - Indicate whether to add top via to request or to
*                             remove top via from response.
*        strViaBranch       - via branch to add to the message befor sending
*                            (used ONLY on request and if bHandleTopVia = RV_TRUE)
*        pViaBranch         - the via branch as an rpool string.
*        hLocalAddr        - the local address handle to use.
*        strLocalAddress      - The local address to use when sending the message. If
*                                 NULL is given the deafult local address will be used.
*        localPort            - The local port to use when sending the message.
*        bIgnoreOutboundProxy - Indicate the proxy whether to ignore an
*                                 outbound proxy.
*        bSendToFirstRouteHeader  - Determines weather to send a request to to a loose router
*                             (to the first in the route list) or to a strict router
*                             (to the request URI). When the message sent is a response
*                             this parameter is ignored.
*        bCopyMsg           - mark if the transmitter should copy the message.
***************************************************************************/
RvStatus TransactionTransportSendOutofContextMsg(
                               IN    TransactionMgr                *pTranscMgr,
                               IN    RvSipMsgHandle                 hMsgToSend,
                               IN    RvBool                         bHandleTopVia,
                               IN    RvChar                        *strViaBranch,
                               IN    RPOOL_Ptr                     *pViaBranch,
                               IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                               IN    RvChar                        *strLocalAddress,
                               IN    RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
                               IN    RvChar                        *if_name,
			       IN    RvUint16                       vdevblock,
#endif
                               IN    RvBool                         bIgnoreOutboundProxy,
                               IN    RvBool                         bSendToFirstRouteHeader,
                               IN    RvBool                         bCopyMsg)
{

    RvStatus                rv   = RV_OK;
    RvSipTransmitterHandle  hTrx = NULL;
    RvSipMsgType            eMsgType = RvSipMsgGetMsgType(hMsgToSend);


    rv = RvSipTransmitterMgrCreateTransmitter(pTranscMgr->hTrxMgr,
                                              (RvSipAppTransmitterHandle)pTranscMgr,
                                              &pTranscMgr->mgrTrxEvHandlers,
                                              sizeof(RvSipTransmitterEvHandlers),&hTrx);

    if(rv != RV_OK || hTrx == NULL)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionTransportSendOutofContextMsg - TransactionMgr 0x%p:Failed to create transmitter (rv=%d)",
                   pTranscMgr,rv));
        if(bCopyMsg == RV_FALSE)
        {
            RvSipMsgDestruct(hMsgToSend);
        }
        return rv;
    }


    rv = InitializeOutOfContextTransmitter(pTranscMgr,hTrx,strViaBranch,pViaBranch,
                                           hLocalAddr,strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
					   if_name,
					   vdevblock,
#endif
                                           bIgnoreOutboundProxy,bSendToFirstRouteHeader,eMsgType);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionTransportSendOutofContextMsg - TransactionMgr 0x%p:Failed to initialize transmitter 0x%p (rv=%d) - terminating the transmitter",
                   pTranscMgr,hTrx,rv));
        SipTransmitterDestruct(hTrx);
        if(bCopyMsg == RV_FALSE)
        {
            RvSipMsgDestruct(hMsgToSend);
        }
        return rv;
    }

    if(bCopyMsg == RV_FALSE)
    {
        SipTransmitterDontCopyMsg(hTrx);
    }

    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
           "TransactionTransportSendOutofContextMsg - TransactionMgr 0x%p: going to send msg 0x%p on trx 0x%p...",
           pTranscMgr,hMsgToSend,hTrx));

    rv = RvSipTransmitterSendMessage(hTrx,hMsgToSend,bHandleTopVia);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionTransportSendOutofContextMsg - TransactionMgr 0x%p:Failed to send msg with transmitter 0x%p (rv=%d) - terminating the transmitter",
                   pTranscMgr,hTrx,rv));
        SipTransmitterDestruct(hTrx); /*the transmitter will destruct the message*/
        return rv;
    }
    return rv;
}


/***************************************************************************
 * InitializeOutOfContextTransmitter
 * ------------------------------------------------------------------------
 * General: Sets different parameters to a transmitter that is used for
 *          out of context messages.
 *          Note regarding local address handling: Since the original API did
 *          not include address/transport type, the transmitter will have to
 *          look for this address in every defined transport and set what ever
 *          is found in the local address structure.
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTranscMgr - The transaction manager handle
 *        hTrx       - The transmitter to initialize.
 *        strViaBranch - via branch to set in the transmitter.
 *        pViaBranch - via branch as rpool string.
 *        hLocalAddr - The local address. The message will be sent from this address.
 *        strLocalAddress      - The local address to use when sending the message. If
 *                                 NULL is given the default local address will be used.
 *        localPort            - The local port to use when sending the message.
 *        bIgnoreOutboundProxy - Indicate the proxy whether to ignore an
 *                                 outbound proxy.
 *        bSendToFirstRouteHeader - Determines weather to send a request to to a loose router
 *                                 (to the first in the route list) or to a strict router
 *                                 (to the request URI).
 *        eMsgType  - indicates if this is a request or a response message.
 ***************************************************************************/
static RvStatus RVCALLCONV InitializeOutOfContextTransmitter(
                               IN    TransactionMgr*                pTranscMgr,
                               IN    RvSipTransmitterHandle         hTrx,
                               IN    RvChar*                        strViaBranch,
                               IN    RPOOL_Ptr*                     pViaBranch,
                               IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                               IN    RvChar                        *strLocalAddress,
                               IN    RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
                               IN    RvChar                        *if_name,
			       IN    RvUint16                       vdevblock,
#endif
                               IN    RvBool                         bIgnoreOutboundProxy,
                               IN    RvBool                         bSendToFirstRouteHeader,
                               IN    RvSipMsgType                   eMsgType)
{
    RvStatus rv = RV_OK;

    if(strViaBranch != NULL || pViaBranch != NULL)
    {
        rv = RvSipTransmitterSetViaBranch(hTrx, strViaBranch, pViaBranch);
        if(rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                      "InitializeOutOfContextTransmitter - Failed set branch to transmitter 0x%p (rv=%d)",
                       hTrx,rv));
            return rv;
        }
    }

    rv = RvSipTransmitterSetIgnoreOutboundProxyFlag(hTrx,bIgnoreOutboundProxy);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "InitializeOutOfContextTransmitter - Failed set bIgnoreOutboundProxy=%d to transmitter 0x%p (rv=%d)",
            bIgnoreOutboundProxy,hTrx,rv));
        return rv;
    }

    rv = RvSipTransmitterSetUseFirstRouteFlag(hTrx,bSendToFirstRouteHeader);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "InitializeOutOfContextTransmitter - Failed set bSendToFirstRouteHeader=%d to transmitter 0x%p (rv=%d)",
            bSendToFirstRouteHeader,hTrx,rv));
        return rv;
    }

    if(hLocalAddr != NULL)
    {
        SipTransmitterSetLocalAddressHandle(hTrx,hLocalAddr,RV_FALSE);
    }
    else
    {
        if(strLocalAddress != NULL && localPort != 0)
        {
            rv = SipTransmitterSetLocalAddressForAllTransports(hTrx,strLocalAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
							       ,if_name
							       ,vdevblock
#endif
							       );

            if(rv == RV_ERROR_NOT_FOUND)
            {
                RvLogWarning(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "InitializeOutOfContextTransmitter - Trx 0x%p, The supplied local address %s %d is not one of local addresses in the stack. Default will be used",
                    hTrx,strLocalAddress,localPort));
                rv = RV_OK;
            }
            else if(rv != RV_OK)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "InitializeOutOfContextTransmitter - Trx 0x%p, Failed to set local address %s %d",
                    hTrx,strLocalAddress,localPort));
                return rv;

            }
        }
    }

    RV_UNUSED_ARG(eMsgType);
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTranscMgr);
#endif

    return RV_OK;
}

/***************************************************************************
 * ConvertTrxReasonToTranscReason
 * ------------------------------------------------------------------------
 * General: Converts a transmitter reason to transaction reason.
 * Return Value: -  RvSipTransactionStateChangeReason
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eTrxReason
 ***************************************************************************/
static RvSipTransactionStateChangeReason RVCALLCONV ConvertTrxReasonToTranscReason(
                  IN   RvSipTransmitterReason eTrxReason)
{
    switch(eTrxReason)
    {
    case RVSIP_TRANSMITTER_REASON_NETWORK_ERROR:
    case RVSIP_TRANSMITTER_REASON_CONNECTION_ERROR:
        return RVSIP_TRANSC_REASON_NETWORK_ERROR;
    case RVSIP_TRANSMITTER_REASON_OUT_OF_RESOURCES:
        return RVSIP_TRANSC_REASON_OUT_OF_RESOURCES;
    case  RVSIP_TRANSMITTER_REASON_USER_COMMAND:
        return RVSIP_TRANSC_REASON_ERROR; /*this is the correct reason for user command*/
    default:
        return RVSIP_TRANSC_REASON_ERROR;
    }
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * AdaptToTrxCompartmentIfNeeded
 * ------------------------------------------------------------------------
 * General: Adapt to the Trx Compartment. According to 
 *          RFC5049 chapter 9 a whole Transaction has
 *          to be related to a single Compartment (for compressing it's 
 *          outgoing message and decompressing it's incoming message). 
 *          This function has to be executed right after the Trx has sent
 *          a SigComp compressed request as long as the given Transaction is
 *          not related to a Compartment. 
 *
 * Return Value: None 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - Pointer to the transaction object.
 *        hTrx    - The Transmitter handle       
 ***************************************************************************/
static void AdaptToTrxCompartmentIfNeeded(
                          IN Transaction              *pTransc,
                          IN RvSipTransmitterHandle    hTrx)
{
    RvSipCompartmentHandle hComp; 
    
    if (pTransc->hSigCompCompartment != NULL) 
    {
        return;
    }

    SipTransmitterGetCompartment(hTrx,&hComp);
    if (hComp != NULL)
    {
		RvStatus rv;

        /* Attach the given Transaction to it's Trx compartment object */ 
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "AdaptToTrxCompartmentIfNeeded - Attaching Transc 0x%p to Compartment 0x%p",
            pTransc,hComp));

		rv =  SipCompartmentAttach(hComp,(void*)pTransc);

        if (rv != RV_OK) 
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "AdaptToTrxCompartmentIfNeeded - Transc 0x%p, Failed to attach to Compartment 0x%p (Trx's 0x%p Compartment)",
                pTransc,hComp,hTrx));
            /* The attachment failed, thus nothing should be done */ 
            return;
        }
        pTransc->hSigCompCompartment = hComp;

        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "AdaptToTrxCompartmentIfNeeded - Transc 0x%p is now attached to Compartment 0x%p (Trx's 0x%p Compartment)",
            pTransc,hComp,hTrx));
    }
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

#endif /*#ifndef RV_SIP_LIGHT*/


