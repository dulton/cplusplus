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
 *                              <TransmitterControl.c>
 *
 * This file contains all transmitter message sending activities.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos               January 2004
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransmitterObject.h"
#include "TransmitterControl.h"
#include "TransmitterCallbacks.h"
#include "TransmitterDest.h"
#include "_SipTransport.h"
#include "RvSipViaHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipTransport.h"
#include "_SipCommonUtils.h"
#include "_SipCommonConversions.h"
#ifdef RV_SIP_IMS_ON
#include "_SipSecuritySecObj.h"
#endif
#ifdef RV_SIGCOMP_ON
#include "_SipViaHeader.h"
#include "_SipAddress.h"
#include "_SipCommonUtils.h"
#endif 


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"

#if defined(UPDATED_BY_SPIRENT_ABACUS)
void  HSTPOOL_Serialize_TRACE ( void * buff, char* file, int line );
#define HSTPOOL_Serialize(buff) HSTPOOL_Serialize_TRACE(buff,__FILE__,__LINE__)
void  HSTPOOL_SetBlockSize ( void * buff, int size );
void  HSTPOOL_SetType( void * buf, char type);
int   HSTPOOL_GetBlockSize ( void * buff);
#endif

#include "MsgTypes.h"

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV ProcessAndTransmit(
                        IN Transmitter*      pTrx);

static RvStatus PrepareSentOutboundMsg(
									   IN Transmitter*   pTrx,
									   IN RvSipMsgHandle hMsgToSend,
									   IN RvChar*        strBuff,
									   IN RvInt32        buffSize);

static void RVCALLCONV PostMessageSendingActiviteis(
                                        IN Transmitter*          pTrx,
                                        IN SipTransportSendInfo* pSendInfo);

static RvStatus SendMsgOnConnection(
                                IN Transmitter* pTrx,
                                IN RvSipTransportConnectionHandle hSecureConn);

#ifdef RV_SIGCOMP_ON
static RvStatus HandleSigCompSendIfNeeded(IN Transmitter    *pTrx,
                                         IN RvSipMsgHandle  hMsgToSend,
                                         IN RvBool          bUpdateTopVia);

static RvStatus UpdateSigCompRequestVia(IN Transmitter     *pTrx,
                                        IN RvSipMsgHandle   hRequest); 

#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER TRANSPORT FUNCTIONS                   */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TransmitterControlStartMessageSending
 * ------------------------------------------------------------------------
 * General: Send the message object held by the transmitter to the
 *          remote party. The function performs address resolution if
 *          needed notifies the application that address was resolved and
 *          call the TransmitterControlContinueMessageSending that will
 *          do the actual sending.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx - handle to the transmitter
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlStartMessageSending(
                                        IN Transmitter*  pTrx)
{
    RvStatus rv = RV_OK;
    rv = TransmitterDestDiscovery(pTrx,RV_TRUE);
    return rv;
}

/***************************************************************************
 * TransmitterControlContinueMessageSending
 * ------------------------------------------------------------------------
 * General: Send the message after address resolution was completed.
 *          If the dest address has been reset, go for another round with
 *          the resolution process
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - handle to the transmitter.
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from the dns callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlContinueMessageSending(
                                        IN Transmitter*      pTrx,
                                        IN RvBool            bReachedFromApp)
{
    RvStatus rv = RV_OK;
    rv = ProcessAndTransmit(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlContinueMessageSending - pTrx=0x%p: Failed to transmit message",
                   pTrx));
        return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,rv,
                                                      RVSIP_TRANSMITTER_REASON_UNDEFINED,
                                                      bReachedFromApp);
    }
    return RV_OK;
}

/***************************************************************************
 * TransmitterControlSend
 * ------------------------------------------------------------------------
 * General: Send the message after address resolution was completed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - handle to the transmitter.
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from the dns callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlSend(
                                        IN Transmitter*      pTrx,
                                        IN RvBool            bReachedFromApp)
{
    RvStatus rv = RV_OK;
    rv = ProcessAndTransmit(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlSend - pTrx=0x%p: Failed to transmit message",
                   pTrx));
        return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,rv,
                                                      RVSIP_TRANSMITTER_REASON_UNDEFINED,
                                                      bReachedFromApp);
    }
    return RV_OK;

}

/***************************************************************************
 * TransmitterControlSendBuffer
 * ------------------------------------------------------------------------
 * General: Sends a buffer to the destination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx     - handle to the transmitter.
 *          strBuff  - The buffer to send.
 *          buffSize - The buffer size.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlSendBuffer(
                                        IN Transmitter*      pTrx,
										IN RvChar*           strBuff,
										IN RvInt32           buffSize)
{
    RvStatus rv = RV_OK;

	/*update the local address again in case the application changed the dest addr*/
    rv = TransmitterSetInUseLocalAddrByDestAddr(pTrx);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
			"TransmitterControlSendBuffer - pTrx=0x%p: There is no local address defined for this transmitter.",
			pTrx));
        return RV_ERROR_UNKNOWN;
    }
    
    /* Encode the message */
    rv = PrepareSentOutboundMsg(pTrx,NULL,strBuff,buffSize);
	
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
			"TransmitterControlSendBuffer - pTrx=0x%p: Failed to prepare outbound msg (rv=%d).",
			pTrx,rv));
        return rv;
    }
	
    rv = TransmitterMsgToSendListAddElement(pTrx,pTrx->hSentOutboundMsg);
	
    if(rv != RV_OK)
    {
        TransmitterFreeEncodedOutboundMsg(pTrx);
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterControlSendBuffer - trx=0x%p: Couldn't allocate outgoing msg to list",pTrx));
        return rv;
    }
	
    /* Send message */
    rv = TransmitterControlTransmitMessage(pTrx);
	
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "TransmitterControlSendBuffer - trx=0x%p: Message failed to be sent. (rv=%d)",pTrx,rv));
        return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,rv,
                                                      RVSIP_TRANSMITTER_REASON_UNDEFINED,
                                                      RV_FALSE);
    }

#ifdef RV_SIGCOMP_ON
	/* After TrnasmitterControlTrnasmitMessage, the connection is valid for sure. */
	/* Time to make the compartment valid as well. */
	rv = TransmitterUseCompartment(pTrx);
	if (rv != RV_OK)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
			"TransmitterControlSendBuffer - trx=0x%p: failed adding compartment 0x%p. (rv=%d)",
			pTrx,pTrx->hSigCompCompartment,rv));
		return (rv);
	}
#endif
    return RV_OK;
}



/***************************************************************************
 * TransmitterControlConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   -   The connection handle
 *          hTrx    -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTrx,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{

    Transmitter*    pTrx    = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_OK;
    }
    RV_UNUSED_ARG(eReason);
    if (TransmitterLockEvent(pTrx) != RV_OK)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStateChangedEv - transmitter 0x%p, conn 0x%p - Failed to lock transmitter",
                   pTrx,hConn));
        return RV_ERROR_UNKNOWN;
    }

    switch(eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterControlConnStateChangedEv - transmitter 0x%p received a connection closed event",pTrx));
        if (pTrx->hConn == hConn)
        {
            pTrx->hConn = NULL;
        }
        break;
    default:
        break;
    }
    TransmitterUnLockEvent(pTrx);
    return RV_OK;
}


/***************************************************************************
 * TransmitterControlConnStatusEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection status was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   -   The connection handle
 *          hTrx    -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          pInfo   -   For future usage.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlConnStatusEv(
                    IN  RvSipTransportConnectionHandle        hConn,
                    IN  RvSipTransportConnectionOwnerHandle   hTrx,
                    IN RvSipTransportConnectionStatus         eStatus,
                    IN  void*                                 msgInfo)
{
    Transmitter*    pTrx    = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_OK;
    }
    if (TransmitterLockEvent(pTrx) != RV_OK)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - transmitter 0x%p, conn 0x%p - Failed to lock transmitter",
                   pTrx,hConn));
        return RV_ERROR_UNKNOWN;
    }

	/* We might reach here in the slight synchronization point: between the execution */
	/* of the trx's connection status notification callback (before acquiring the    */
	/* transmitter's lock above) and the trx termination, so it's checked here       */ 
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
		TransmitterUnLockEvent(pTrx);
        return RV_OK;
    }

    /* Sanity check: ensure that this transmitter owns the connection */
#ifdef RV_SIP_IMS_ON
    if(hConn != pTrx->hConn  &&  hConn != pTrx->hSecureConn)
    {
        if (pTrx->hConn == NULL  &&  pTrx->hSecureConn == NULL)
        {
            RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - Transmitter 0x%p, unknown conn 0x%p. Probably it was already detached",
                   pTrx, hConn));
        }
        else
        {
            RvLogExcep(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - Transmitter 0x%p, unknown conn 0x%p (trx->hConn= 0x%p, trx->hSecureConn=0x%p)", 
                   pTrx, hConn, pTrx->hConn, pTrx->hSecureConn));
        }
        TransmitterUnLockEvent(pTrx);
        return RV_OK;
    }
#else
    if(hConn != pTrx->hConn)
    {
        if (pTrx->hConn == NULL)
        {
            RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - Transmitter 0x%p, unknown conn 0x%p. Probably it was already detached",
                   pTrx, hConn));
        }
        else
        {
            RvLogExcep(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - Transmitter 0x%p, unknown conn 0x%p (trx->hConn= 0x%p)", 
                   pTrx, hConn, pTrx->hConn));
        }
        TransmitterUnLockEvent(pTrx);
        return RV_OK;
    }
#endif


    switch (eStatus)
    {
    case RVSIP_TRANSPORT_CONN_STATUS_MSG_NOT_SENT:
    case RVSIP_TRANSPORT_CONN_STATUS_ERROR:
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterControlConnStatusEv - Transmitter 0x%p: received a connection 0x%p error status (event = %d)",
                   pTrx,hConn,eStatus));
        TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,
                                               RV_ERROR_UNKNOWN,
                                               RVSIP_TRANSMITTER_REASON_CONNECTION_ERROR,
                                               RV_FALSE);
        break;
    case RVSIP_TRANSPORT_CONN_STATUS_MSG_SENT:
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterControlConnStatusEv - Transmitter 0x%p received a send successful event on conn 0x%p",
                  pTrx,hConn));

        PostMessageSendingActiviteis(pTrx,(SipTransportSendInfo*)msgInfo);
        break;
    default:
        break;
    }
    TransmitterUnLockEvent(pTrx);
    return RV_OK;
}

/***************************************************************************
 * TransmitterControlTransmitMessage
 * ------------------------------------------------------------------------
 * General: transmit the message sent by the transaction, over tcp or udp
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx             - Pointer to the Transmitter
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlTransmitMessage(IN Transmitter* pTrx)
{
    RvStatus             rv = RV_OK;
    SipTransportSendInfo msg;
    RvSipTransport       eTransport;

    if(pTrx->hSentOutboundMsg == NULL)
    {
        return RV_OK;
    }

    /* Store eTransport for further logging */
    eTransport = pTrx->destAddr.eTransport;

    switch(pTrx->destAddr.eTransport)
    {
#ifndef RV_SIP_IMS_ON /* When Security is used, transport can't be UNDFINED*/
    case RVSIP_TRANSPORT_UNDEFINED:
#endif
    case RVSIP_TRANSPORT_UDP:
        {
            rv = SipTransportUdpSendMessage(pTrx->pTrxMgr->hTransportMgr,
                    pTrx->pTrxMgr->hMessagePool, pTrx->hSentOutboundMsg,
                    *(pTrx->localAddr.hAddrInUse), &pTrx->destAddr.addr);
            if (RV_OK == rv)
            {
                memset(&msg,0,sizeof(msg));
                msg.msgType   = pTrx->eMsgType;
                msg.msgToSend = pTrx->hSentOutboundMsg;
                PostMessageSendingActiviteis(pTrx,&msg);
            }
        }
        break;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
#endif
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
#endif
    case RVSIP_TRANSPORT_TCP:
        {
#ifdef RV_SIP_IMS_ON
            rv = SendMsgOnConnection(pTrx, pTrx->hSecureConn);
#else
            rv = SendMsgOnConnection(pTrx, NULL);
#endif
        }
        break;

    default:
        RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterControlTransmitMessage - pTrx=0x%p: can not send message, %s is not supported",
            pTrx, SipCommonConvertEnumToStrTransportType(pTrx->destAddr.eTransport)));
        break;
    }

    /* If Transmitter was destructed, don't touch it */
    if (RV_OK != rv  &&  RV_ERROR_DESTRUCTED != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterControlTransmitMessage - pTrx=0x%p: failed to send message %p over %s (Connection 0x%p)(rv=%d:%s)",
            pTrx, pTrx->hSentOutboundMsg,
            SipCommonConvertEnumToStrTransportType(eTransport),
            pTrx->hConn, rv, SipCommonStatus2Str(rv)));

        /*pTrx->hConn=NULL;*/

        if (RV_ERROR_INSUFFICIENT_BUFFER != rv)
        {
            rv = RV_ERROR_NETWORK_PROBLEM;
        }
        return rv;
    }
    else
    if (RV_OK == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterControlTransmitMessage - pTrx=0x%p: message %p was sent over %s (Connection 0x%p)(rv=%d:%s)",
            pTrx, pTrx->hSentOutboundMsg,
            SipCommonConvertEnumToStrTransportType(eTransport),
            pTrx->hConn, rv, SipCommonStatus2Str(rv)));
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * PostMessageSendingActiviteis
 * ------------------------------------------------------------------------
 * General:
 * Return Value:-
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      -   The transmitter handle
 *          pSendInfo - information about the message that was sent.
 ***************************************************************************/
static void RVCALLCONV PostMessageSendingActiviteis(
                                        IN Transmitter*          pTrx,
                                        IN SipTransportSendInfo* pSendInfo)
{
    HPAGE    hMsgPage = pSendInfo->msgToSend;
    TransmitterMsgToSendListRemoveElement(pTrx,hMsgPage);
    TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_MSG_SENT,
                           RVSIP_TRANSMITTER_REASON_UNDEFINED,NULL,(void*)pSendInfo);
}

/***************************************************************************
 * ProcessAndTransmit
 * ------------------------------------------------------------------------
 * General: Does the actual final handling and sending of a message, or
 *          returns error code if there is a problem
 *          - Checks DNS status
 *          - Does Sigcomp handling
 *          - Consults destination with application
 *          - updates Via and local address
 *          - Encodes and sends
 * Return Value: RV_OK or failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Transaction to send message
 ***************************************************************************/
static RvStatus RVCALLCONV ProcessAndTransmit(
                                     IN Transmitter*      pTrx)
{
    RvStatus                        rv              = RV_OK;

    /* After the application callback */

    /*update the local address again incase the application changed the dest addr*/
    rv = TransmitterSetInUseLocalAddrByDestAddr(pTrx);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "ProcessAndTransmit - pTrx=0x%p: There is no local address defined for this transmitter.",
                 pTrx));
        return RV_ERROR_UNKNOWN;
    }

    rv = TransmitterUpdateViaByFinalLocalAddress(pTrx,pTrx->bFixVia);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "ProcessAndTransmit - pTrx=0x%p:TransmitterUpdateViaByFinalLocalAddress failed.",
            pTrx));
        return RV_ERROR_UNKNOWN;
    }
        
    /* in this callback the transaction has a chance to update it's via, and do SigComp operation */
    TransmitterChangeState(pTrx,
                           RVSIP_TRANSMITTER_STATE_READY_FOR_SENDING,
                           RVSIP_TRANSMITTER_REASON_UNDEFINED,
                           pTrx->hMsgToSend,
                           NULL);

    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "ProcessAndTransmit - pTrx=0x%p: was terminated from callback. returning from function",
                 pTrx));
        return RV_OK;
    }

#ifdef RV_SIP_IMS_ON
	/* 
		for security object, ordinary address resolution isn't done. However, we do want to notify
		the application that the final destination address is resolved. So we manually change the transmitter
		to final dest resolved state
	  */
	if (NULL != pTrx->hSecObj)
	{
		rv = TransmitterMoveToStateFinalDestResolved (pTrx,RVSIP_TRANSMITTER_REASON_UNDEFINED,pTrx->hMsgToSend,
				NULL);
		if (RV_OK != rv)
		{
			RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "ProcessAndTransmit - pTrx=0x%p: Failed to change state (rv=%d).",
				pTrx,rv));
			return rv;
		}
	}
#endif

    /* Encode the message */
    rv = PrepareSentOutboundMsg(pTrx,pTrx->hMsgToSend,NULL,0);

    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "ProcessAndTransmit - pTrx=0x%p: Failed to prepare outbound msg (rv=%d).",
                 pTrx,rv));
        return rv;
    }

    rv = TransmitterMsgToSendListAddElement(pTrx,pTrx->hSentOutboundMsg);

    if(rv != RV_OK)
    {
        TransmitterFreeEncodedOutboundMsg(pTrx);
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "ProcessAndTransmit - trx=0x%p: Couldn't allocate outgoing msg to list",pTrx));
        return rv;
    }

    /*we don't need the message object any more*/
    if(pTrx->bKeepMsg == RV_FALSE)
    {
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend = NULL;
    }

    /* Send message */
    rv = TransmitterControlTransmitMessage(pTrx);

    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "ProcessAndTransmit - trx=0x%p: Message failed to be sent. (rv=%d)",pTrx,rv));
        return rv;
    }

#ifdef RV_SIGCOMP_ON
	/* After TrnasmitterControlTrnasmitMessage, the connection is valid for sure. */
	/* Time to make the compartment valid as well. */
	rv = TransmitterUseCompartment(pTrx);
	if (rv != RV_OK)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
			"ProcessAndTransmit - trx=0x%p: failed adding compartment 0x%p. (rv=%d)",
			pTrx,pTrx->hSigCompCompartment,rv));
		return (rv);
	}
#endif

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * PrepareSentOutboundMsg
 * ------------------------------------------------------------------------
 * General: Prepare the outbound message that is going to be sent over
 *          the transaction, by encoding the message and compressing it
 *          if necessary
 * Return Value: RV_success/RV_ERROR_UNKNOWN
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 
 *        pTrx        - Pointer to the Transaction.
 *        hMsgToSend  - Handle to the transmitted message or NULL if we are sending a buffer.
 *        strBuff     - A buffer in case we are sending a non sip message
 *        buffSize    - The size of the buffer.
 ***************************************************************************/
static RvStatus PrepareSentOutboundMsg(
              IN Transmitter*   pTrx,
			  IN RvSipMsgHandle hMsgToSend,
			  IN RvChar*        strBuff,
			  IN RvInt32        buffSize)
{
    RvStatus rv;

#ifdef RV_SIGCOMP_ON

	rv = HandleSigCompSendIfNeeded(pTrx,hMsgToSend,RV_TRUE);
	if(RV_OK != rv)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"PrepareSentOutboundMsg - HandleSigCompSendIfNeeded(Transmitter 0x%p,hMsg=0x%p,RV_TRUE) failed",
			pTrx,hMsgToSend));
		return rv;
	}

    /* NOTE: It's important to first free the redundant message pages  */
    /* before using new pages in order to avoid memory leaks and waste */
    /* memory. Thus, the function should be called first.  */
    TransmitterFreeRedundantOutboundMsgIfNeeded(pTrx);
#endif /* RV_SIGCOMP_ON */

    pTrx->hSentOutboundMsg = NULL_PAGE;

	/*sending a sip message*/
    if(hMsgToSend != NULL)
	{
		
		/* Encode the message into the hEncodedOutboundMsg*/
		rv = RvSipMsgEncode(hMsgToSend,
							pTrx->pTrxMgr->hMessagePool,
							&(pTrx->hEncodedOutboundMsg),
							&(pTrx->encodedMsgLen));
		if (RV_OK != rv)
		{
			pTrx->hEncodedOutboundMsg = NULL_PAGE;
			return rv;
		}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
      {
         RvChar* SPIRENT_buffer = NULL;
         int     SPIRENT_size   = 0;
         SPIRENT_buffer = (RvChar*)RvSipMsgGetSPIRENTBody(hMsgToSend);
         if(SPIRENT_buffer != NULL)
         {
            SPIRENT_size = HSTPOOL_GetBlockSize(SPIRENT_buffer);
            if(SPIRENT_size >= (int)pTrx->encodedMsgLen)
            {
               rv = RPOOL_CopyToExternal(pTrx->pTrxMgr->hMessagePool, pTrx->hEncodedOutboundMsg, 0, 
                       SPIRENT_buffer, pTrx->encodedMsgLen);
               if(rv == RV_OK)
               {
                  HSTPOOL_SetBlockSize (SPIRENT_buffer, pTrx->encodedMsgLen);
                  HSTPOOL_SetType( SPIRENT_buffer, 6 /*=ATYPE_SIP */);
                  HSTPOOL_Serialize(SPIRENT_buffer);
                  ((MsgMessage*)hMsgToSend)->SPIRENT_msgBody = NULL;
               }
               else
               {
                  RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                      "PrepareSentOutboundMsg - Trx 0x%p, failed=%d to copy encodedbuf to SPIRENT_buffer",pTrx,rv));
                  RvSipMsgReleaseSPIRENTBody(hMsgToSend);
               }
            }
            else
            {
               RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                   "PrepareSentOutboundMsg - Trx 0x%p, can't copy encodedbuf because SPIRENT_size=%d < encodedsize=%d",pTrx,SPIRENT_size,pTrx->encodedMsgLen));
               RvSipMsgReleaseSPIRENTBody(hMsgToSend);
            }
         }
      }
#endif
/* SPIRENT_END */
	}
	else /*sending a buffer, copy to the encoded page*/
	{
        RvInt32           offset = UNDEFINED;
		rv = RPOOL_GetPage(pTrx->pTrxMgr->hMessagePool, 0, &(pTrx->hEncodedOutboundMsg));
		if(RV_OK != rv)
		{
			RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"PrepareSentOutboundMsg - RPOOL_GetPage failed (Transmitter=%0x,pool=%0x)",
				pTrx,pTrx->pTrxMgr->hMessagePool));
			pTrx->hEncodedOutboundMsg = NULL_PAGE;
		}
		
		rv = RPOOL_AppendFromExternal(pTrx->pTrxMgr->hMessagePool,pTrx->hEncodedOutboundMsg,
			                          strBuff,buffSize,RV_FALSE,&offset,NULL);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "PrepareSentOutboundMsg - Trx 0x%p RPOOL_AppendFromExternal failed rv=%d",pTrx,rv));
            TransmitterFreeEncodedOutboundMsg(pTrx);
            return rv;
        }
        pTrx->encodedMsgLen = buffSize;
	}
    /* Set the pointer of the sent outbound message to the encoded message */
    pTrx->hSentOutboundMsg = pTrx->hEncodedOutboundMsg;

#ifdef RV_SIGCOMP_ON
    /* Compressing the encoded message */
    if (RVSIP_COMP_SIGCOMP ==
        SipTransmitterConvertMsgCompTypeToCompType(pTrx->eOutboundMsgCompType))
    {
		/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
		if(RvSipMsgGetMsgType(pTrx->hMsgToSend) == RVSIP_MSG_REQUEST)
		{  
			RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"COMPRESS Request %d",RvSipMsgGetRequestMethod(pTrx->hMsgToSend)));
		}
		else{
			RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"COMPRESS Response %d",RvSipMsgGetStatusCode(pTrx->hMsgToSend)));
		}
#endif
		/* SPIRENT_END */
		
        rv = TransmitterPrepareCompressedSentMsgPage(pTrx);
		
        if (RV_OK != rv)
        {
            TransmitterFreeEncodedOutboundMsg(pTrx);
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "PrepareSentOutboundMsg - failed to compress Transc %0x message", pTrx));
			
            return rv;
        }
    }
	/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    else {
		
		if(RvSipMsgGetMsgType(pTrx->hMsgToSend) == RVSIP_MSG_REQUEST)
		{  
			RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"NO COMPRESS Request %d",RvSipMsgGetRequestMethod(pTrx->hMsgToSend)));
		}
		else{
			RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"NO COMPRESS Response %d",RvSipMsgGetStatusCode(pTrx->hMsgToSend)));
		}
		
    }
#endif
	/* SPIRENT_END */
#endif /* RV_SIGCOMP_ON */

    return rv;
}

/******************************************************************************
 * SendMsgOnConnection
 * ----------------------------------------------------------------------------
 * General: transmit the message sent by the transaction, over TCP/SCTP/TLS
 * Return Value: -  RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx        - Pointer to the Transmitter
 *        hSecureConn - Secure Connection to be used. Can be NULL.
 *****************************************************************************/
static RvStatus SendMsgOnConnection(
                                IN Transmitter* pTrx,
                                IN RvSipTransportConnectionHandle hSecureConn)
{
    RvStatus rv;
    TransmitterMgr* pTrxMgr = pTrx->pTrxMgr;

    if (RV_TRUE != SipTransportTcpEnabled(pTrxMgr->hTransportMgr))
    {
        RvLogError(pTrxMgr->pLogSrc  ,(pTrxMgr->pLogSrc,
            "SendMsgOnConnection - pTrx=0x%p: TCP is disabled", pTrx));
        return RV_ERROR_UNKNOWN;
    }

    /*create a new connection if needed*/
    rv = TransmitterCreateConnectionIfNeeded(pTrx, hSecureConn);
    /* If the transmitter was terminated, don't touch it */
    if(rv == RV_ERROR_DESTRUCTED)
    {
        RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "SendMsgOnConnection - pTrx=0x%p: was terminated. Msg will not be sent",
            pTrx));
        return rv;
    }
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "SendMsgOnConnection - pTrx=0x%p: failed to construct a new connection",
            pTrx));
        return rv;
    }
    
    rv = SipTransportConnectionSend(
                            pTrxMgr->hTransportMgr,
                            pTrx->hConn,
                            (pTrx->pTrxMgr)->hMessagePool,
                            pTrx->hSentOutboundMsg,
                            RV_FALSE,
                            pTrx->eMsgType,
                            (RvSipTransportConnectionOwnerHandle)pTrx
                    #if (RV_NET_TYPE & RV_NET_SCTP)
                            ,
                            &pTrx->sctpParams
                    #endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
                            );
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "SendMsgOnConnection - pTrx=0x%p: failed to send message on connection %p. Detach from it.",
            pTrx, pTrx->hConn));
        TransmitterDetachFromConnection(pTrx, pTrx->hConn);
        /* pTrx->hConn is reset in TransmitterDetachFromConnection()
        pTrx->hConn = NULL;
        */
        return rv;
    }

    return RV_OK;
}

#ifdef RV_SIGCOMP_ON

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
static int SpirentRvSipUpdateOutboundMsgSigcompId(IN Transmitter      *pTrx, IN RvSipMsgHandle    hMsg)
{
	RvSipRouteHopHeaderHandle   hRouteHop = NULL;
	RvSipHeaderListElemHandle   listElem;
	RvSipRouteHopHeaderType     routeHopType;
	RvSipAddressHandle  Route = NULL;
				
	// get the first record route header
	hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
		RVSIP_HEADERTYPE_ROUTE_HOP, RVSIP_FIRST_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
	while (hRouteHop != NULL)
	{
		routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);
		// if the route hop is a service-route, save the route for sending out future requests
		if(routeHopType == RVSIP_ROUTE_HOP_ROUTE_HEADER)
		{			
			Route = RvSipRouteHopHeaderGetAddrSpec(hRouteHop);
			int offset;
			if(Route && (offset = SipAddrUrlGetSigCompIdParam(Route)) > UNDEFINED)				
			{
				TransmitterUpdateOutboundMsgSigcompId(pTrx, SipAddrGetPool(Route), SipAddrGetPage(Route), offset);
				return RV_OK;
			}
			return -1;			
		}
		// get the next route hop
		hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
			RVSIP_HEADERTYPE_ROUTE_HOP, RVSIP_NEXT_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
	}
	return -1;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * HandleSigCompSendIfNeeded
 * ------------------------------------------------------------------------
 * General: Checks if the sent transmitter message should be compressed
 *          using SigComp and if it does than a 'comp=sigcomp' parameter is
 *          added automatically to the top Via of sent message
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx          - The transmitter handle.
 *          hMsgToSend    - Handle to the transmitted message.
 *          bUpdateTopVia - Defines whether the stack should update the top via header.
 * Output:  (-)
 ***************************************************************************/
static RvStatus HandleSigCompSendIfNeeded(IN Transmitter      *pTrx,
                                          IN RvSipMsgHandle    hMsgToSend,
                                          IN RvBool            bUpdateTopVia)
{
    RvSipMsgType  eMsgType = (hMsgToSend == NULL) ? 
										RVSIP_MSG_UNDEFINED : RvSipMsgGetMsgType(hMsgToSend);
	RvStatus	  rv; 

    if (SipTransmitterConvertMsgCompTypeToCompType(pTrx->eOutboundMsgCompType)
                      != RVSIP_COMP_SIGCOMP)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "HandleSigCompSendIfNeeded - : Noneed SIGCOMP %d",pTrx->eOutboundMsgCompType));
#endif
/* SPIRENT_END */
        return RV_OK;
    }
    
    /* If the application has disabled the compression mechanism */
    /* then no further SigComp updates take place .              */
    if (pTrx->bSigCompEnabled == RV_FALSE)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
          "HandleSigCompSendIfNeeded - trx=0x%p: Contains SigComp outbound msg but SigComp is disabled - changing the msg type",
          pTrx));
        pTrx->eOutboundMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED;

        return RV_OK;
    }

    /* Update the Via header with SigComp parameters */ 
    if(bUpdateTopVia == RV_TRUE && eMsgType == RVSIP_MSG_REQUEST)
    {
        rv = UpdateSigCompRequestVia(pTrx,hMsgToSend);
        if (rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "HandleSigCompSendIfNeeded - pTrx=0x%p: failed to update Via header with SigComp parameters",pTrx));
            return rv;
        }
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#ifdef RV_SIGCOMP_ON
    /* Get here only in Sigcomp case */ 
    /* Retrieve the sigcomp id from the SIP URI address and store */
    /* it in the current Transmitter object */		  
	   if(pTrx->sigcompIdOffset == UNDEFINED && RvSipMsgGetMsgType(hMsgToSend) == RVSIP_MSG_REQUEST) 
	   {
          int offset;		  
		  //dprintf("R %d Msg %d ",pTrx->bSendToFirstRoute, RvSipMsgGetRequestMethod(hMsgToSend)); 
		  if(pTrx->bSendToFirstRoute)
		  {
			  if(SpirentRvSipUpdateOutboundMsgSigcompId(pTrx, hMsgToSend) != RV_OK)
			  {
			    dprintf_T("******* WHICHMESSAGE FAIL? %d\n",RvSipMsgGetRequestMethod(hMsgToSend));
			  }
		  }
		  else{
		     RvSipAddressHandle hSipUrlAddress = RvSipMsgGetRequestUri(hMsgToSend);		     
		     if(hSipUrlAddress && (offset = SipAddrUrlGetSigCompIdParam(hSipUrlAddress)) > UNDEFINED)	         
				TransmitterUpdateOutboundMsgSigcompId(pTrx, SipAddrGetPool(hSipUrlAddress), SipAddrGetPage(hSipUrlAddress), offset); 
		  }
	   }	
#endif /* RV_SIGCOMP_ON */
#endif
/* SPIRENT_END */
    
	/* Relate the transmitter to a compartment if needed */  
    rv = TransmitterHandleSigCompCompartment(pTrx,hMsgToSend);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "HandleSigCompSendIfNeeded - pTrx=0x%p: failed handling SigComp transaction",pTrx));
        return rv;
    }
	
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
		"HandleSigCompSendIfNeeded - Transmitter 0x%p using compartment 0x%p",pTrx,pTrx->hSigCompCompartment));
    

    
    return RV_OK;
}

/***************************************************************************
 * UpdateSigCompRequestVia
 * ------------------------------------------------------------------------
 * General: Updates the Via header within the sent SigComp compressed 
 *          request with relevant SigComp parameters. 
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - The transmitter handle.
 *          hRequest  - Handle to the transmitted SigComp compressed request.
 * Output:  (-)
 ***************************************************************************/
static RvStatus UpdateSigCompRequestVia(IN Transmitter     *pTrx,
                                        IN RvSipMsgHandle   hRequest)
{
    RvStatus                  rv; 
    RvSipViaHeaderHandle      hTopVia;
    RvSipHeaderListElemHandle hListItem;      
    
    /* 1. Retrieve the topmost Via header */ 
    hTopVia = RvSipMsgGetHeaderByTypeExt(hRequest,
                                         RVSIP_HEADERTYPE_VIA,
                                         RVSIP_FIRST_HEADER,
                                         RVSIP_MSG_HEADERS_OPTION_ALL,
                                         &hListItem);
    if (hTopVia == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "UpdateSigCompRequestVia - pTrx=0x%p: failed to retrieve the topmost Via header from request msg 0x%x",
            pTrx,hRequest));

        return RV_ERROR_UNKNOWN; 
    }

    /* 2. Add comp=sigcomp parameter */ 
    rv = SipViaHeaderSetCompParam(hTopVia,RVSIP_COMP_SIGCOMP, NULL, NULL, NULL_PAGE, UNDEFINED);
    if (rv != RV_OK) {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "UpdateSigCompRequestVia - pTrx=0x%p: failed to set 'comp=sigcomp' parameter within the topmost Via header",
            pTrx));
        return rv;
    }
    
	/* sigcomp-id will be added in xxxFinalDestResolvedEv, by application */

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTrx);
#endif

    return RV_OK;
}

#endif /* RV_SIGCOMP_ON */

