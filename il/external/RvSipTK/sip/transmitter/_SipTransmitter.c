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
 *                              <_SipTransmitter.c>
 *
 *  The _SipTransmitter.h file contains Internal API functions of the
 *  Transmitter layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                 January 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransmitterObject.h"
#include "TransmitterCallbacks.h"
#include "TransmitterDest.h"
#include "_SipTransport.h"
#include "_SipCommonUtils.h"
#include "TransmitterControl.h"
#include "RvSipTransportDNS.h"
#ifdef RV_SIGCOMP_ON
#include "_SipCompartment.h"
#include "_SipAddress.h"
#endif 

#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIGCOMP_ON
static RvStatus RVCALLCONV HandleUpdatedSentOutboundMsg(
                        IN Transmitter                  *pTrx,
                        IN RvSipTransmitterMsgCompType   eUpdatedType);
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER INTERNAL API FUNCTIONS                */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransmitterSendMessageToPreDiscovered
 * ----------------------------------------------------------------------------------
 * General: Send a message to the remote party. if sending with this function,
 *          Dns procedure will not start, but the "old" results from previous
 *          DNS will be used
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          hMsgToSend   - The message to send.
 *          eMsgType     - The type of the message (transaction-wise) to send
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterSendMessageToPreDiscovered (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN SipTransportMsgType       eMsgType)
{
    Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;

	/* If sigcomp is supported, update the sigcomp-id in the transmitter,
	   As no regular resolving is being done */
#ifdef RV_SIGCOMP_ON
	RvSipMethodType    eMethodType;
	RvSipAddressHandle hSipUrlAddress = NULL;

	eMethodType = RvSipMsgGetRequestMethod(hMsgToSend);
	/* no need to do the following for ACK on reject, as it uses the same Trx of the INVITE, */
	/* which is already updated. */
	if (eMethodType != RVSIP_METHOD_ACK)
	{
		hSipUrlAddress = RvSipMsgGetRequestUri(hMsgToSend);
		if (hSipUrlAddress != NULL)
		{
			rv = TransmitterUpdateOutboundMsgSigcompId(pTrx, SipAddrGetPool(hSipUrlAddress), 
				SipAddrGetPage(hSipUrlAddress),	SipAddrUrlGetSigCompIdParam(hSipUrlAddress));
			if (RV_OK != rv)
			{
				RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
					"SipTransmitterSendMessageToPreDiscovered - pTrx=0x%p: failed to update sigcomp-id (rv=%d)",
					pTrx, rv));
				return rv;
			}
		}
	}
#endif
    /* If Security is supported, transport and destination address,
	protected by the Security Object should be used instead of pre-discovered address. */
#ifdef RV_SIP_IMS_ON
    if (NULL != pTrx->hSecObj)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "SipTransmitterSendMessageToPreDiscovered - pTrx=0x%p: use Security Object %p",
            pTrx, pTrx->hSecObj));

		if(pTrx->hMsgToSend != NULL)
		{
			RvLogExcep(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
					   "SipTransmitterSendMessageToPreDiscovered - #1 - Transmitter= 0x%p, contains a message object 0x%p. message will be destructed",
						pTrx,pTrx->hMsgToSend));
			RvSipMsgDestruct(pTrx->hMsgToSend);
			pTrx->hMsgToSend = NULL;
		}
		pTrx->hMsgToSend        = hMsgToSend;
		pTrx->eMsgType          = eMsgType;
		pTrx->hNextMessageHop   = NULL;
        rv = TransmitterControlSend(pTrx,RV_TRUE/*bReachedFromApp*/);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "SipTransmitterSendMessageToPreDiscovered - pTrx=0x%p: failed to send secure message(rv=%d:%s)",
                pTrx, rv, SipCommonStatus2Str(rv)));
        }
        return rv;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    if (TRANSMITTER_RESOLUTION_STATE_RESOLVED  != pTrx->eResolutionState)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessageToPreDiscovered - Trx=0x%p, destination is not discovered yet (res state=%s)",
            pTrx,TransmitterGetResolutionStateName(pTrx->eResolutionState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
               "SipTransmitterSendMessageToPreDiscovered - Trx=0x%p, Sending message to prediscoverd",
                pTrx));


    if(pTrx->hMsgToSend != NULL)
    {
        RvLogExcep(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                   "SipTransmitterSendMessageToPreDiscovered - Transmitter= 0x%p, contains a message object 0x%p. message will be destructed",
                    pTrx,pTrx->hMsgToSend));
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend = NULL;
    }

    pTrx->hMsgToSend        = hMsgToSend;
    pTrx->eMsgType          = eMsgType;
    pTrx->hNextMessageHop   = NULL;

    /*notify that the address was already resolved*/
    rv = TransmitterMoveToStateFinalDestResolved(pTrx,
                                RVSIP_TRANSMITTER_REASON_UNDEFINED,
                                hMsgToSend,NULL);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessageToPreDiscovered - TransmitterMoveToStateFinalDestResolved(Transmitter 0x%p, hMsgToSend=0x%p) failed",
            pTrx,pTrx->hMsgToSend));
        return rv;
    }
    /* If the application holds and resumes from the DEST_RESOLVED callback,
    called above from the TransmitterMoveToStateFinalDestResolved(),
    the message will be sent before this point. So nothing should be done
    further. Just return. The Transmitter will be terminated on the Transaction
    termination. */
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_MSG_SENT)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessageToPreDiscovered - Transmitter 0x%p, message was sent already. Return.",
            pTrx));
        return RV_OK;
    }

    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessageToPreDiscovered - Transmitter 0x%p, will not send the message",
            pTrx));
        return RV_OK;
    }
    
    /*if the transmitter was terminated from the callback just return error*/
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "SipTransmitterSendMessageToPreDiscovered - pTrx=0x%p: was terminated from callback. returning from function",
            pTrx));
        return RV_ERROR_DESTRUCTED;
    }

    /* It is possible that during the resolved callback the application will decide to hold,
       in that case, we do not send the message
    */
    if (RVSIP_TRANSMITTER_STATE_ON_HOLD != pTrx->eState)
    {
        rv = TransmitterControlSend(pTrx,RV_TRUE);
    }
    return rv;
}


/************************************************************************************
 * SipTransmitterSendMessage
 * ----------------------------------------------------------------------------------
 * General: Send a message to the remote party. The remote address is calculated
 *          according to RFC 3261 that takes into account the existence of
 *          outbound proxy, Route headers and loose routing rules.
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          hMsgToSend   - The message to send.
 *          eMsgType     - The type of the message (transaction-wise) to send
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterSendMessage (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN SipTransportMsgType       eMsgType)
{
    Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;

    /*check the transmitter state*/
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR ||
        pTrx->eState == RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED ||
        pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessage - Transmitter= 0x%p, cannot send message 0x%p in state %s",
            pTrx,pTrx->hMsgToSend,TransmitterGetStateName(pTrx->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pTrx->hMsgToSend != NULL)
    { /* this print prevent overwriting of the msgToSend pointer  */ 
        RvLogExcep(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterSendMessage - Transmitter= 0x%p, hMsgToSend 0x%p is not empty",
            pTrx,pTrx->hMsgToSend));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#ifdef RV_SIGCOMP_ON
	{
	   RvSipTransmitterEvHandlers *pEvHandlers = &pTrx->evHandlers;
	   if(pEvHandlers->pfnLastChangeBefSendEvHandler)       	   
		   pEvHandlers->pfnLastChangeBefSendEvHandler(hMsgToSend, pTrx->hAppTrx);		   
	}
#endif
#endif
/* SPIRENT_END */
    /*set the message in the transmitter*/
    pTrx->eMsgType          = eMsgType;
    pTrx->hMsgToSend        = hMsgToSend;
    pTrx->hNextMessageHop   = NULL;
    pTrx->msgParams.transportType = RVSIP_TRANSPORT_UNDEFINED;

    rv = TransmitterControlStartMessageSending(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                   "SipTransmitterSendMessage - Transmitter= 0x%p, Failed send message 0x%p (rv=%d), Destructing msg",
                    pTrx,pTrx->hMsgToSend,rv));
        if(pTrx->hMsgToSend != NULL && pTrx->bKeepMsg == RV_FALSE)
        {
            pTrx->hMsgToSend = NULL;
        }
        /*change the transmitter state back to idle*/
        if(pTrx->eState != RVSIP_TRANSMITTER_STATE_TERMINATED &&
            pTrx->eState != RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE &&
            pTrx->eState !=  RVSIP_TRANSMITTER_STATE_IDLE)
        {

			pTrx->eResolutionState = TRANSMITTER_RESOLUTION_STATE_UNDEFINED;
            TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_IDLE,
                RVSIP_TRANSMITTER_REASON_UNDEFINED,NULL,NULL);

        }
        return rv;
    }
    return RV_OK;
}



/***************************************************************************
 * SipTransmitterRetransmit
 * ------------------------------------------------------------------------
 * General: Retransmit the last message sent by the transmitter, if exists.
 *          The transmitter holds the last message it sent in encoded format,
 *          and also the address it sent this message to (after DNS).
 *          This function will send this encoded message to this address
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterRetransmit(
                          IN RvSipTransmitterHandle        hTrx)
{
    Transmitter*  pTrx = (Transmitter*)hTrx;
    RvStatus      rv   = RV_OK;

    if(NULL == hTrx)
    {
        return RV_ERROR_BADPARAM;
    }

    /*We have nothing to retransmit return ok*/
    if(pTrx->hSentOutboundMsg == NULL)
    {
        return RV_OK;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "SipTransmitterRetransmit - Transmitter 0x%p - Retransmitting msg page 0x%p",
               pTrx,pTrx->hSentOutboundMsg));
    rv = TransmitterControlTransmitMessage(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterRetransmit - Transmitter 0x%p - Failed to retransmit msg page 0x%p",
                   pTrx,pTrx->hSentOutboundMsg));
        return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,rv,RVSIP_TRANSMITTER_REASON_UNDEFINED,RV_TRUE);
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransmitterMoveToMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Mark the transmitter that it should move to msg send failure
 *          when the state changed callback returns from the final dest
 *          resolved state.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 ***************************************************************************/
void RVCALLCONV SipTransmitterMoveToMsgSendFailure(
                          IN RvSipTransmitterHandle        hTrx)
{
    Transmitter*  pTrx = (Transmitter*)hTrx;
    pTrx->bMoveToMsgSendFailure = RV_TRUE;

}


/***************************************************************************
 * SipTransmitterGetDestAddr
 * ------------------------------------------------------------------------
 * General: Gets the destination address the transmitter will use.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            pDestAddr         - destination addr
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetDestAddr(
                          IN RvSipTransmitterHandle        hTrx,
                          IN SipTransportAddress*          pDestAddr)
{
    Transmitter*  pTrx = (Transmitter*)hTrx;
    memcpy(pDestAddr,&pTrx->destAddr,sizeof(SipTransportAddress));
    return RV_OK;
}


/***************************************************************************
 * SipTransmitterSetTripleLock
 * ------------------------------------------------------------------------
 * General: sets the transmitter triple lock
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - the transmitter handle
 *           pTripleLock - pointer to a triple lock.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetTripleLock(
                          IN RvSipTransmitterHandle  hTrx,
                          IN SipTripleLock*          pTripleLock)
{
    TransmitterSetTripleLock((Transmitter*)hTrx,pTripleLock);
}


/***************************************************************************
 * SipTransmitterDestruct
 * ------------------------------------------------------------------------
 * General: Free the Transmitter resources and terminate the transmitter
 *          without sending it to the event queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter to destruct
 ***************************************************************************/
void RVCALLCONV SipTransmitterDestruct(
                          IN RvSipTransmitterHandle hTrx)
{
    TransmitterDestruct((Transmitter*)hTrx);
}


/***************************************************************************
 * SipTransmitterDNSListReset
 * ------------------------------------------------------------------------
 * General: resets the transmitter DNS list. ie empty the list
 *
 * Return Value: RvStatus, RV_ERROR_UNKNOWN
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - the transmitter
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDNSListReset(
                    IN  RvSipTransmitterHandle    hTrx)
{
    Transmitter* pTrx = (Transmitter*)hTrx;
    RvStatus             rv;

    if (pTrx == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "SipTransmitterDNSListReset - Transmitter 0x%p - resetting DNS list",
        pTrx));
    
    pTrx->eResolutionState = TRANSMITTER_RESOLUTION_STATE_UNDEFINED;
    
    rv = SipTransportDNSListConstruct(
        pTrx->hPool,
        pTrx->hPage,
        SipTransportGetMaxDnsElements(pTrx->pTrxMgr->hTransportMgr),
        &pTrx->hDnsList);
    if (RV_OK != rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "SipTransmitterDNSListReset - Transmitter 0x%p - faild to reset DNS list (rv=%d_",
            pTrx,rv));
        return rv;
    }

    return rv;
}


/***************************************************************************
 * SipTransmitterSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx  - Handle to the transmitter
 *          hConn - Handle to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetConnection(
                   IN  RvSipTransmitterHandle           hTrx,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    RvStatus        rv;
    Transmitter*    pTrx= (Transmitter *)hTrx;
    RvSipTransportConnectionHandle hPrevConn;
    if (NULL == pTrx)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetConnection - Transmitter 0x%p - Setting connection 0x%p", pTrx,hConn));

    /*if the received connection is the same connection that we already have just return*/
    if(hConn == pTrx->hConn)
    {
        return RV_OK;
    }

    hPrevConn = pTrx->hConn;

    if(hConn != NULL)
    {
        /*first try to attach the Transmitter to the connection*/
        rv = TransmitterAttachToConnection(pTrx,hConn);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "SipTransmitterSetConnection - Failed to attach Transmitter 0x%p to connection 0x%p",pTrx,hConn));
            return rv;
        }
        pTrx->bConnWasSetByApp = RV_TRUE;
    }
    /*detach from the Transmitter current connection if there is one*/
    if(hPrevConn != NULL)
    {
        TransmitterDetachFromConnection(pTrx,hPrevConn);
    }
    return RV_OK;
}


/***************************************************************************
 * SipTransmitterSetPersistency
 * ------------------------------------------------------------------------
 * General: Sets the persistency definition of the transmitter
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx - The Transmitter handle
 *          bIsPersistent - persistency definition
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetPersistency(
                           IN RvSipTransmitterHandle  hTrx,
                           IN RvBool                  bIsPersistent)
{
    Transmitter*  pTrx = (Transmitter*)hTrx;
    pTrx->bIsPersistent = bIsPersistent;
}
/***************************************************************************
 * SipTransmitterGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the transmitter persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 ***************************************************************************/
RvBool RVCALLCONV SipTransmitterGetPersistency(
                     IN  RvSipTransmitterHandle  hTrx)
{

    Transmitter*  pTrx = (Transmitter*)hTrx;
    return pTrx->bIsPersistent;
}


/***************************************************************************
 * TransmitterDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from its connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - pointer to the Transmitter
 ***************************************************************************/
void SipTransmitterDetachFromConnection(IN  RvSipTransmitterHandle  hTrx)
{
    Transmitter*            pTrx = (Transmitter*)hTrx;
    TransmitterDetachFromConnection(pTrx, pTrx->hConn);
}


/***************************************************************************
 * SipTransmitterDetachFromAres
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the ARES if it is in the middle of 
 *          resolution process (so no ARES events will be called).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - pointer to the Transmitter
 ***************************************************************************/
void SipTransmitterDetachFromAres(IN  RvSipTransmitterHandle  hTrx)
{
    Transmitter*            pTrx = (Transmitter*)hTrx;
    TransmitterDetachFromAres(pTrx);
}

/***************************************************************************
 * SipTransmitterDontCopyMsg
 * ------------------------------------------------------------------------
 * General: Indicates not to copy the message of application transmitter.
 *          This function will be used by standalone transmitters that are
 *          created by the stack.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 ***************************************************************************/
void RVCALLCONV SipTransmitterDontCopyMsg(
                     IN  RvSipTransmitterHandle  hTrx)
{

    Transmitter*  pTrx = (Transmitter*)hTrx;
    pTrx->bCopyAppMsg = RV_FALSE;
}



/***************************************************************************
 * SipTransmitterGetOutboundAddsStructure
 * ------------------------------------------------------------------------
 * General: Get the outbound address structure of the transmitter
 * Return Value: outboundAddr.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the transaction
 ***************************************************************************/
SipTransportOutboundAddress* RVCALLCONV SipTransmitterGetOutboundAddsStructure(
                            IN  RvSipTransmitterHandle       hTrx)
{
    Transmitter*   pTrx =(Transmitter *)hTrx;
    return &pTrx->outboundAddr;
}

/***************************************************************************
 * SipTransmitterSetOutboundAddsStructure
 * ------------------------------------------------------------------------
 * General: Set the outbound address structure of the transmitter
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the transaction
 *            outboundAddr  - The outbound proxy to set.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundAddsStructure(
                            IN  RvSipTransmitterHandle       hTrx,
                            IN  SipTransportOutboundAddress* outboundAddr)
{
    Transmitter*   pTrx;
    RvStatus       rv;
    if (NULL == hTrx)
    {
        return RV_ERROR_NULLPTR;
    }
    pTrx = (Transmitter *)hTrx;

    pTrx->outboundAddr = *outboundAddr;

    /*copy the host name to the transmitters page if it is not there already.*/
    if(outboundAddr->rpoolHostName.hPage != NULL_PAGE)
    {
        if(outboundAddr->rpoolHostName.hPage != pTrx->hPage)
        {
            RPOOL_Ptr srcPtr;
            srcPtr.hPool  = outboundAddr->rpoolHostName.hPool;
            srcPtr.hPage  = outboundAddr->rpoolHostName.hPage;
            srcPtr.offset = outboundAddr->rpoolHostName.offset;
            pTrx->outboundAddr.rpoolHostName.hPool = pTrx->hPool;
            pTrx->outboundAddr.rpoolHostName.hPage = pTrx->hPage;
            pTrx->outboundAddr.rpoolHostName.offset = UNDEFINED;
            rv = SipCommonCopyStrFromPageToPage(&srcPtr,
                                                &pTrx->outboundAddr.rpoolHostName);
            if(rv != RV_OK)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                    "SipTransmitterSetOutboundAddsStructure - transmitter 0x%p - Failed to copy outbound host",pTrx));
                return rv;
            }
#ifdef SIP_DEBUG
            pTrx->outboundAddr.strHostName = RPOOL_GetPtr(pTrx->outboundAddr.rpoolHostName.hPool,
                                                          pTrx->outboundAddr.rpoolHostName.hPage,
                                                          pTrx->outboundAddr.rpoolHostName.offset);
#endif
        }
    }
	
#ifdef RV_SIGCOMP_ON
	/* copy the sigcomp-id to the transmitter page, so the transmitter will not have      */
	/* to depend on the source page. Unless, the sigcomp-id is already on the Trx's page. */
    if(outboundAddr->rpoolSigcompId.hPage != NULL_PAGE)
    {
        if(outboundAddr->rpoolSigcompId.hPage != pTrx->hPage)
        {
            RPOOL_Ptr srcPtr;

            srcPtr.hPool  = outboundAddr->rpoolSigcompId.hPool;
            srcPtr.hPage  = outboundAddr->rpoolSigcompId.hPage;
            srcPtr.offset = outboundAddr->rpoolSigcompId.offset;
            pTrx->outboundAddr.rpoolSigcompId.hPool = pTrx->hPool;
            pTrx->outboundAddr.rpoolSigcompId.hPage = pTrx->hPage;
            pTrx->outboundAddr.rpoolSigcompId.offset = UNDEFINED;
            rv = SipCommonCopyStrFromPageToPage(&srcPtr,
                                                &pTrx->outboundAddr.rpoolSigcompId);
            if(rv != RV_OK)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                    "SipTransmitterSetOutboundAddsStructure - transmitter 0x%p - Failed to copy outbound sigcomp-id",pTrx));
                return rv;
            }
#ifdef SIP_DEBUG
            pTrx->outboundAddr.strSigcompId = RPOOL_GetPtr(pTrx->outboundAddr.rpoolSigcompId.hPool,
                                                           pTrx->outboundAddr.rpoolSigcompId.hPage,
                                                           pTrx->outboundAddr.rpoolSigcompId.offset);
#endif
        }
    }
	
#endif
    return RV_OK;
}


/***************************************************************************
 * SipTransmitterSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the transmitter will send outgoing requests.
 * The SIP Stack can be configured to listen to many local addresses. Each local
 * address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 * When the SIP Stack sends an outgoing request, the local address (from where
 * the request is sent) is chosen according to the characteristics of the remote
 * address. Both the local and remote addresses must have the same characteristics,
 * such as the same transport type and address type. If several configured local
 * addresses match the remote address characteristics, the first configured address
 * is taken.
 * You can use SipTransmitterSetLocalAddress() to force the transmitter to
 * choose a specific local address for a specific transport and address type. For
 * example, you can force the transmitter to use the second configured UDP/
 * IPv4 local address. If the transmitter sends a request to a UDP/IPv4 remote
 * address, it will use the local address that you set instead of the default first local
 * address.
 * Note: The localAddress string you provide for this function must match exactly
 * with the local address that was inserted in the configuration structure in the
 * initialization of the SIP Stack. If you configured the SIP Stack to listen to a 0.0.0.0
 * local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 *            localAddress - The local address to be set to this transmitter.
 *          localPort - The local port to be set to this transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetLocalAddress(
                            IN  RvSipTransmitterHandle         hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    )
{
    RvStatus           rv;
    Transmitter        *pTrx= (Transmitter *)hTrx;
    RvSipTransportLocalAddrHandle hLocalAddr = NULL;

    if (NULL == pTrx || localAddress==NULL || localPort==(RvUint16)UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    if((eTransportType != RVSIP_TRANSPORT_TCP &&
#if (RV_TLS_TYPE != RV_TLS_NONE)
        eTransportType != RVSIP_TRANSPORT_TLS &&
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        eTransportType != RVSIP_TRANSPORT_SCTP &&
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
        eTransportType != RVSIP_TRANSPORT_UDP) ||
       (eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP &&
        eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP6))
    {
        return RV_ERROR_BADPARAM;
    }


    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetLocalAddress - Transmitter 0x%p - Setting %s %s local address %s:%d",
               pTrx,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType),
               (localAddress==NULL)?"0":localAddress,localPort));

    rv = SipTransportLocalAddressGetHandleByAddress(pTrx->pTrxMgr->hTransportMgr,
                                                    eTransportType,
                                                    eAddressType,
                                                    localAddress,
                                                    localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name,
						    vdevblock,
#endif
                                                    &hLocalAddr);

    if(hLocalAddr == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetLocalAddress - Failed, local aadress was not found"));
        return RV_ERROR_NOT_FOUND;
    }

    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetLocalAddress - Transmitter 0x%p, Failed (rv=%d)",pTrx,rv));
        return rv;
    }

    TransmitterSetLocalAddressHandle(pTrx,hLocalAddr,RV_FALSE);
    return RV_OK;
}

/***************************************************************************
 * SipTransmitterSetLocalAddressHandle
 * ------------------------------------------------------------------------
 * General: Sets a local address handle in the transmitter local addr structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            hLocalAddress - The local address handle.
 *           bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetLocalAddressHandle(
                            IN  RvSipTransmitterHandle        hTrx,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            IN  RvBool                        bSetAddrInUse)
{
    TransmitterSetLocalAddressHandle((Transmitter*)hTrx,hLocalAddr,bSetAddrInUse);
}



/***************************************************************************
 * SipTransmitterSetLocalAddressForAllTransports
 * ------------------------------------------------------------------------
 * General: gets a string local ip and port and set it to all handles that have
 *          a matching address.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            strLocalAddress - The local address string
 *            localPort - The local port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetLocalAddressForAllTransports(
                            IN  RvSipTransmitterHandle        hTrx,
                            IN  RvChar                        *strLocalAddress,
                            IN  RvUint16                       localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                      *if_name
			    , IN  RvUint16                     vdevblock
#endif
			    )
{
    Transmitter        *pTrx= (Transmitter *)hTrx;
    return SipTransportSetLocalAddressForAllTransports(pTrx->pTrxMgr->hTransportMgr,
						       &pTrx->localAddr,
						       strLocalAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
						       ,if_name
						       ,vdevblock
#endif
						       );
}


/***************************************************************************
 * SipTransmitterGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transmitter will use to send outgoing
 *          messages.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType - The address type(ip or ip6).
 * Output:    localAddress - The local address this transmitter is using.
 *            localPort - The local port this transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT RvChar*                   localAddress,
                            OUT RvUint16*                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                *if_name
			    , OUT  RvUint16*              vdevblock
#endif
			    )
{
    Transmitter*   pTrx = (Transmitter*)hTrx;
    RvStatus       rv   = RV_OK;


    if (NULL == pTrx || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterGetLocalAddress - Transmitter 0x%p - Getting local %s,%s address",pTrx,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType)));


    rv = SipTransportGetLocalAddressByType(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->localAddr,
                                           eTransportType,
                                           eAddressType,
                                           localAddress,
                                           localPort
#if defined(UPDATED_BY_SPIRENT)
					   ,if_name
					   ,vdevblock
#endif
					   );
    if(RV_OK != rv)
    {
       RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetLocalAddress - Transmitter 0x%p - Local %s %s address not found", pTrx,
                   SipCommonConvertEnumToStrTransportType(eTransportType),
                   SipCommonConvertEnumToStrTransportAddressType(eAddressType)));
       return rv;
    }

    return RV_OK;

}

/***************************************************************************
 * SipTransmitterGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transmitter will use to send
 *          the next message this function returns the actual address from
 *          the 6 addresses that was used or going to be used.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 * Output:    eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType  - The address type(IP4 or IP6).
 *            localAddress - The local address this transmitter is using(must be longer than 48).
 *            localPort - The local port this transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetCurrentLocalAddress(
                            IN  RvSipTransmitterHandle     hTrx,
                            OUT RvSipTransport*            eTransportType,
                            OUT RvSipTransportAddressType* eAddressType,
                            OUT RvChar*                    localAddress,
                            OUT RvUint16*                  localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                 *if_name
			    , OUT  RvUint16*               vdevblock
#endif
			    )
{

    Transmitter*    pTrx = (Transmitter*)hTrx;
    RvStatus        rv   = RV_OK;
    if (NULL == pTrx || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    localAddress[0] = '\0'; /*reset the local address*/
    *localPort = 0;
    *eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    *eAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;


    if(pTrx->localAddr.hAddrInUse == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetCurrentLocalAddress - Transmitter 0x%p - Current local address was not set", pTrx));
        return RV_ERROR_NOT_FOUND;
    }

    rv = SipTransportLocalAddressGetAddressByHandle(pTrx->pTrxMgr->hTransportMgr,
						    *(pTrx->localAddr.hAddrInUse),
						    RV_FALSE,RV_TRUE,localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
						    ,if_name
						    ,vdevblock
#endif
						    );

    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetCurrentLocalAddress - Transmitter 0x%p - Failed to get info for local address 0x%p (rv=%d)",
                  pTrx,*(pTrx->localAddr.hAddrInUse),rv));
        return rv;
    }

    rv = SipTransportLocalAddressGetTransportType(*(pTrx->localAddr.hAddrInUse),eTransportType);
    if(RV_OK != rv  ||  RVSIP_TRANSPORT_UNDEFINED == *eTransportType)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetCurrentLocalAddress - Transmitter 0x%p - Failed to get transport type for local address 0x%p",
                  pTrx,*(pTrx->localAddr.hAddrInUse)));
        return ((RV_OK != rv) ? rv : RV_ERROR_UNKNOWN);
    }
    rv = SipTransportLocalAddressGetAddrType(*(pTrx->localAddr.hAddrInUse),eAddressType);
    if(RV_OK != rv  ||  RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == *eAddressType)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetCurrentLocalAddress - Transmitter 0x%p - Failed to get address type for local address 0x%p",
                  pTrx,*(pTrx->localAddr.hAddrInUse)));
        return ((RV_OK != rv) ? rv : RV_ERROR_UNKNOWN);
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterGetCurrentLocalAddress - Transmitter 0x%p - Local address is: ip=%s, port=%d, transport=%s, address type=%s",
               pTrx,localAddress,*localPort,
               SipCommonConvertEnumToStrTransportType(*eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(*eAddressType)));

    return RV_OK;
}

#if defined(UPDATED_BY_SPIRENT)
RvSipTransportLocalAddrHandle SipTransmitterGetCurrentLocalAddressHandle(IN  RvSipTransmitterHandle     hTrx)
{
  RvSipTransportLocalAddrHandle handle=NULL;
  Transmitter*    pTrx = (Transmitter*)hTrx;
  if (NULL == pTrx) {
    return NULL;
  }

  if(pTrx->localAddr.hAddrInUse == NULL) {
    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				       "SipTransmitterGetCurrentLocalAddressHandle - Transmitter 0x%p - Current local address was not set", pTrx));
    return NULL;
  }

  handle = *(pTrx->localAddr.hAddrInUse);

  return handle;
}
#endif

/***************************************************************************
 * SipTransmitterSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address the transmitter will use as the request
 *          destination address. A requests sent
 *          by this transmitter will be sent according to the transmitter
 *          outbound address and not according to the Request-URI.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx         - Handle to the transmitter
 *            strOutboundAddrIp    - The outbound ip to be set to this
 *                                 transmitter.
 *          outboundPort         - The outbound port to be set to this
 *                                 transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundAddress(
                            IN  RvSipTransmitterHandle     hTrx,
                            IN  RvChar*                    strOutboundAddrIp,
                            IN  RvUint16                   outboundPort)

{
    RvStatus      rv = RV_OK;
    Transmitter*  pTrx;

    pTrx = (Transmitter *)hTrx;

    if (NULL == pTrx)
    {
        return RV_ERROR_NULLPTR;
    }

    if(strOutboundAddrIp == NULL || strcmp(strOutboundAddrIp,"") == 0 ||
       strcmp(strOutboundAddrIp,IPV4_LOCAL_ADDRESS) == 0 ||
       memcmp(strOutboundAddrIp, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) == 0)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterSetOutboundAddress - Failed, ip address  not supplied"));
        return RV_ERROR_BADPARAM;
    }

      RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetOutboundAddress - Transmitter 0x%p - Setting an outbound address %s:%d ",
              pTrx, (strOutboundAddrIp==NULL)?"0":strOutboundAddrIp,outboundPort));

    rv = SipTransportSetObjectOutboundAddr(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           strOutboundAddrIp, (RvInt32)outboundPort);

    if (RV_OK != rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "SipTransmitterSetOutboundAddress - Failed to set outbound address for transmitter 0x%p (rv=%d)",
                   pTrx,rv));
        return rv;
    }
    return RV_OK;
}



/***************************************************************************
 * SipTransmitterGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Get the outbound address the transmitter is using. If an
 *          outbound address was set to the transmitter it is returned. If
 *          no outbound address was set to the transmitter, but an outbound
 *          proxy was configured for the stack the configured address is
 *          returned. '\0' is returned no address was defined for
 *          the transmitter or the stack.
 *          NOTE: you must supply a valid consecutive buffer of size 20 for the
 *          strOutboundIp parameter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx      - Handle to the transmitter
 * Output:
 *            strOutboundIp   - A buffer with the outbound IP address the transmitter.
 *                               is using
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundAddress(
                            IN   RvSipTransmitterHandle    hTrx,
                            OUT  RvChar*                   strOutboundIp,
                            OUT  RvUint16*                 pOutboundPort)
{
    Transmitter*   pTrx;
    RvStatus       rv    = RV_OK;

    pTrx = (Transmitter *)hTrx;

    if (NULL == pTrx)
    {
        return RV_ERROR_NULLPTR;
    }

    if(strOutboundIp == NULL || pOutboundPort == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetOutboundAddress - Transmitter 0x%p Failed, NULL pointer supplied",pTrx));
        return RV_ERROR_BADPARAM;
    }

    rv = SipTransportGetObjectOutboundAddr(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           strOutboundIp,
                                           pOutboundPort);
    if(rv != RV_OK)
    {
          RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                     "SipTransmitterGetOutboundAddress - Failed to get outbound address of transmitter 0x%p,(rv=%d)",
                     pTrx, rv));
          return rv;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
             "SipTransmitterGetOutboundAddress - outbound address of transmitter 0x%p: %s:%d",
              pTrx, strOutboundIp, *pOutboundPort));

    return RV_OK;
}


/***************************************************************************
 * SipTransmitterSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy host name of the transmitter object.
 * The outbound host name will be resolved each time a request is sent to this host.
 * the request will be sent to the resolved IP address.
 * Note: To set a specific IP address, use SipTransmitterSetOutboundAddress().
 * If you configure a transmitter with both an outbound IP address and an
 * outbound host name, the transmitter will ignore the outbound host name and
 * will use the outbound IP address.
 * When using an outbound host all outgoing requests will be sent directly to the transmitter
 * outbound proxy host unless the application specifically ordered to ignore the outbound host.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx           - Handle to the transmitter
 *            strOutboundHostName    - The outbound proxy host to be set to this
 *                               transmitter.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               transmitter. If you set the port to zero it
 *                               will be determined using the DNS procedure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundHostName(
                            IN  RvSipTransmitterHandle  hTrx,
                            IN  RvChar*                 strOutboundHost,
                            IN  RvUint16                outboundPort)
{
    RvStatus      rv    = RV_OK;
    Transmitter*  pTrx;

    pTrx = (Transmitter*)hTrx;

    if (NULL == pTrx)
    {
        return RV_ERROR_NULLPTR;
    }

    if (strOutboundHost == NULL ||
        strcmp(strOutboundHost,"") == 0)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterSetOutboundHostName - Failed, host name was not supplied"));
        return RV_ERROR_BADPARAM;
    }

     RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetOutboundHostName - Transmitter 0x%p - Setting an outbound host name %s:%d ",
              pTrx, (strOutboundHost==NULL)?"NULL":strOutboundHost,outboundPort));

    /*we set all info on the size an it will be copied to the Transmitter on success*/
    rv = SipTransportSetObjectOutboundHost(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           strOutboundHost,
                                           outboundPort,
                                           pTrx->hPool,
                                           pTrx->hPage);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterSetOutboundHostName - Transmitter 0x%p Failed (rv=%d)", pTrx,rv));
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * SipTransmitterGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy that the transmitter is using.
 * If an outbound host was set to the transmitter this host will be returned. If no
 * outbound host was set to the transmitter but an outbound host was configured
 * for the SIP Stack, the configured host is returned. If the transmitter is not
 * using an outbound host, '\0' is returned.
 * Note: You must supply a valid consecutive buffer to get the strOutboundHost host
 *       name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx      - Handle to the transmitter
 * Output:
 *          strOutboundHost   -  A buffer with the outbound host name
 *          pOutboundPort - The outbound host port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundHostName(
                            IN   RvSipTransmitterHandle  hTrx,
                            OUT  RvChar*                 strOutboundHostName,
                            OUT  RvUint16*               pOutboundPort)
{
    Transmitter*       pTrx;
    RvStatus           rv   = RV_OK;
    pTrx = (Transmitter *)hTrx;

    if (NULL == pTrx)
    {
        return RV_ERROR_NULLPTR;
    }

    if (strOutboundHostName == NULL || pOutboundPort == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetOutboundHostName - Failed, NULL pointer supplied"));
        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetObjectOutboundHost(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           strOutboundHostName,
                                           pOutboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                 "SipTransmitterGetOutboundHostName - Failed for transmitter 0x%p (rv=%d)", pTrx,rv));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransmitterSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the transmitter outbound proxy.
 * This transport will be used for the outbound proxy that you set using the
 * SipTransmitterSetOutboundAddress() function or the
 * SipTransmitterSetOutboundHostName() function. If you do not set an
 * outbound transport, the transport will be determined using the DNS procedures.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx           - Handle to the transmitter
 *          eOutboundTransport - The outbound transport to be set
 *                               to this transmitter.
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetOutboundTransport(
                            IN  RvSipTransmitterHandle           hTrx,
                            IN  RvSipTransport              eOutboundTransport)
{
    Transmitter*         pTrx;
    pTrx = (Transmitter *)hTrx;

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterSetOutboundTransport - Transmitter 0x%p - Setting outbound transport to %s",
               pTrx, SipCommonConvertEnumToStrTransportType(eOutboundTransport)));
    pTrx->outboundAddr.ipAddress.eTransport = eOutboundTransport;
}



/***************************************************************************
 * SipTransmitterGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of the outbound proxy that the transmitter is using.
 * If an outbound transport was set to the transmitter, this transport will be
 * returned. If no outbound transport was set to the transmitter but an outbound
 * proxy was configured for the SIP Stack, the configured transport is returned.
 * RVSIP_TRANSPORT_UNDEFINED is returned if the transmitter is not
 * using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx           - Handle to the transmitter
 * Output: eOutboundTransport - The outbound proxy transport the transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundTransport(
                            IN   RvSipTransmitterHandle    hTrx,
                            OUT  RvSipTransport      *eOutboundProxyTransport)
{
    Transmitter*  pTrx = (Transmitter *)hTrx;
    RvStatus      rv;
    if (NULL == pTrx)
    {
        return RV_ERROR_NULLPTR;
    }

    if(eOutboundProxyTransport == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetOutboundTransport - Failed, eOutboundProxyTransport is NULL"));
        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetObjectOutboundTransportType(pTrx->pTrxMgr->hTransportMgr,
                                                    &pTrx->outboundAddr,
                                                    eOutboundProxyTransport);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterGetOutboundTransport - Failed for pTrx 0x%p (rv=%d)", pTrx,rv));
        return rv;
    }
    return RV_OK;
}

/************************************************************************************
 * SipTransmitterCanContinue
 * ----------------------------------------------------------------------------------
 * General: checks if there is a valid DNS list in the transmitter and if that list
 *          has at least 1 element that we can continue to
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RvBool RVCALLCONV SipTransmitterCanContinue (
              IN RvSipTransmitterHandle    hTrx)
{
    RvUint32        ipAddrElements  = 0;
    RvUint32        srvElements     = 0;
    RvUint32        hostNameElements= 0;
    RvStatus        rv              = RV_OK;
    Transmitter*    pTrx            = (Transmitter*)hTrx;

    if (NULL == pTrx->hDnsList)
        return RV_FALSE;

    rv = RvSipTransportGetNumberOfDNSListEntries(pTrx->pTrxMgr->hTransportMgr,
            pTrx->hDnsList,&srvElements,&hostNameElements,&ipAddrElements);
    if (RV_OK != rv)
        return RV_FALSE;

    if ((srvElements == 0) && (hostNameElements == 0) && (ipAddrElements == 0))
        return RV_FALSE;

    return RV_TRUE;
}



/************************************************************************************
 * SipTransmitterCopy
 * ----------------------------------------------------------------------------------
 * General: Copies relevant information from the source transmitter
 *          to the destination transmitter
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hDestTrx      - Destination transmitter
 *          hSrcTrx       - Source transmitter
 *          bCopyDnsList - Indicates whether to copy the DNS list or not.
 *
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterCopy(
                     IN  RvSipTransmitterHandle hDestTrx,
                     IN  RvSipTransmitterHandle hSrcTrx,
                     IN  RvBool                 bCopyDnsList)
{
    Transmitter *pDestTrx  = (Transmitter *)hDestTrx;
    Transmitter *pSrcTrx = (Transmitter *)hSrcTrx;

    return TransmitterCopy(pDestTrx,pSrcTrx,bCopyDnsList);
}


/************************************************************************************
 * SipTransmitterAddNewTopViaToMsg
 * ----------------------------------------------------------------------------------
 * General: Adds top via header to the message before address resolution and
 *          according to the transport of the request URI combined with the
 *          available local addresses. This is a best shot via header. The via
 *          header will be fixed after the final destination will be resolved.
 *          Note: this function will use the branch found in the transmitter or
 *          will generate a new branch.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx      - Handle to the transmitter
 *          hMsg      - A message to add a top via to.
 *          hSourceVia- if NULL, use values extracted from transport,
 *                      o/w - copy this value to the new via header
 *          bIgnorTrxBranch - If the transmitter needs to add a branch it should create
 *                            a new branch and ignore its own existing branch.
 *                            this is good for ack on 2xx.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterAddNewTopViaToMsg (
                     IN  RvSipTransmitterHandle          hTrx,
                     IN  RvSipMsgHandle                  hMsg,
                     IN  RvSipViaHeaderHandle            hSourceVia,
                     IN  RvBool                          bIgnorTrxBranch)
{
    Transmitter*                  pTrx = (Transmitter*)hTrx;
    return TransmitterAddNewTopViaToMsg(pTrx,hMsg,hSourceVia,bIgnorTrxBranch);
}

/***************************************************************************
 * SipTransmitterSetLocalAddressesStructure
 * ------------------------------------------------------------------------
 * General: Sets the local address structure to the transmitter
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx - Handle to the transmitter
 *         pLocalAddresses - The local address structure
***************************************************************************/
void RVCALLCONV SipTransmitterSetLocalAddressesStructure (
                            IN  RvSipTransmitterHandle           hTrx,
                            IN  SipTransportObjLocalAddresses*   pLocalAddresses)
{
    Transmitter*                  pTrx = (Transmitter*)hTrx;
    memcpy(&pTrx->localAddr,pLocalAddresses,sizeof(SipTransportObjLocalAddresses));
    if(pLocalAddresses->hAddrInUse  != NULL &&
       *(pLocalAddresses->hAddrInUse) != NULL)
    {
        TransmitterSetInUseLocalAddrByLocalAddr(pTrx,*(pLocalAddresses->hAddrInUse));
    }
}

/***************************************************************************
 * SipTransmitterGetLocalAddressesStructure
 * ------------------------------------------------------------------------
 * General: Gets the local address structure to the transmitter
 *          The address in use is not defined in this stage.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx - Handle to the transmitter
 *         pLocalAddresses - The local address structure
***************************************************************************/
void RVCALLCONV SipTransmitterGetLocalAddressesStructure (
                            IN  RvSipTransmitterHandle   hTrx,
                            IN  SipTransportObjLocalAddresses*   pLocalAddresses)
{
    Transmitter*                  pTrx = (Transmitter*)hTrx;
    memcpy(pLocalAddresses,&pTrx->localAddr,sizeof(SipTransportObjLocalAddresses));
    pLocalAddresses->hAddrInUse = NULL;
}

/***************************************************************************
 * SipTransmitterGetAddrInUse
 * ------------------------------------------------------------------------
 * General: Gets the local-address-in-use from the transmitter
 * Return Value: A pointer to the local address in use
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx - Handle to the transmitter
 ***************************************************************************/
RvSipTransportLocalAddrHandle* RVCALLCONV SipTransmitterGetAddrInUse (
                            IN  RvSipTransmitterHandle           hTrx)
{
    Transmitter*    pTrx = (Transmitter*)hTrx;
    return pTrx->localAddr.hAddrInUse;
}

#ifdef RV_SIGCOMP_ON

/***************************************************************************
* SipTransmitterConvertMsgCompTypeToCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter outbound message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvSipCompType RVCALLCONV SipTransmitterConvertMsgCompTypeToCompType(
                             IN RvSipTransmitterMsgCompType eMsgCompType)
{
    switch (eMsgCompType)
    {
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE:
        return RVSIP_COMP_SIGCOMP;
    default:
        return RVSIP_COMP_UNDEFINED;
    }
}

/***************************************************************************
* SipTransmitterGetOutboundMsgCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter outbound message compression type
* Return Value: The message compression type.
* ------------------------------------------------------------------------
* Arguments:
* input:  hTrx - Handle to the transmitter.
***************************************************************************/
RvSipTransmitterMsgCompType RVCALLCONV SipTransmitterGetOutboundMsgCompType(
                         IN  RvSipTransmitterHandle       hTrx)
{
    Transmitter *pTrx = (Transmitter*)hTrx;

    return pTrx->eOutboundMsgCompType;
}


/***************************************************************************
* SipTransmitterGetNextMsgCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter next message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvSipTransmitterMsgCompType RVCALLCONV SipTransmitterGetNextMsgCompType(
                                            IN  RvSipTransmitterHandle  hTrx)
{
    Transmitter *pTrx = (Transmitter*)hTrx;

    return pTrx->eNextMsgCompType;
}

/***************************************************************************
* SipTransmitterSetNextMsgCompType
* ------------------------------------------------------------------------
* General: Set the transmitter next sent message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetNextMsgCompType(
                      IN  RvSipTransmitterHandle      hTrx,
                      IN  RvSipTransmitterMsgCompType eNextMsgCompType)
{
    Transmitter *pTrx = (Transmitter*)hTrx;

    if (pTrx->bSigCompEnabled  == RV_FALSE &&
        SipTransmitterConvertMsgCompTypeToCompType(eNextMsgCompType) == RVSIP_COMP_SIGCOMP)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                     "SipTransmitterSetNextMsgCompType - Set next msg SigComp type in Transmitter 0x%p which is SigComp disabled",
                     pTrx));
    }

    pTrx->eNextMsgCompType = eNextMsgCompType;

    return RV_OK;
}

/***************************************************************************
 * SipTransmitterMatchSentOutboundMsgToNextMsgType
 * ------------------------------------------------------------------------
 * General: The next retransmitted message type was set in either of
 *          the following cases:
 *          (1) A timeout on outgoing SigComp message was expired and the
 *              the application chose the next re-sent message type.
 *          (2) An incoming message was re-received (in case of original
 *              compressed message) and triggered a retransmission process
 *              of the suitable response/ACK. The re-received message type
 *              effected the next retransmitted message type.
 *          The next retransmitted message type might change the sent
 *          outbound message page.
 * Return Value: RV_OK on successful completion.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - Handle to the transmitter.
 * Output: pbZeroRetrans - Indication if the retransmission counter has to
 *                         become zero.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMatchSentOutboundMsgToNextMsgType(
                                     IN  RvSipTransmitterHandle  hTrx,
                                     OUT RvBool                 *pbZeroRetrans)
{
    RvStatus					 rv   = RV_OK;
    Transmitter                 *pTrx = (Transmitter*)hTrx;
	RvSipTransmitterMsgCompType  eNextMsgCompType;  

	/* Keep and reset the next message type for any case of transmission */
	eNextMsgCompType       = pTrx->eNextMsgCompType; 
    pTrx->eNextMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
	
    /* Set a default value in the output parameter */
    *pbZeroRetrans = RV_FALSE;

    /* No changes has to be done in the re-sent outbound message */
    if (eNextMsgCompType == RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED)
    /* 'eNextMsgCompType == pTrx->eOutboundMsgCompType' - SigComp code change 
       According to RFC5049, chapter 8:
       "Implementations MUST NOT cache the result of compressing the message and 
       retransmit such a cached result." Consequently, the sent message would be 
       re-compressed (using HandleUpdatedSentOutboundMsg()) before each retransmission */
    {
        return RV_OK;
    }

    switch (pTrx->eOutboundMsgCompType)
    {
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE:
        switch (eNextMsgCompType)
        {
        case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE:
            /* SipCompartmentResetByteCodeFlag is deprecated since it's meaningless in dynamic compression */
            /* Continue to the next case block */
        case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
            /* The Transmitter's sent message is re-compressed */
            rv = HandleUpdatedSentOutboundMsg(pTrx,eNextMsgCompType);
            break;
        case RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED:
            /* If the application wishes to stop sending SigComp  */
            /* messages the sent outbound page has to point to    */
            /* the general encoded message page */
            pTrx->hSentOutboundMsg = pTrx->hEncodedOutboundMsg;
            /* The new SentOutboundMsg has to be added to MsgToSendList */
            rv = HandleUpdatedSentOutboundMsg(pTrx,eNextMsgCompType);
            break;
        default:
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "SipTransmitterMatchSentOutboundMsgToNextMsgType - Shouldn't reach here, Transmitter 0x%p",
                  pTrx));
        }
        break;
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED:
        /* We shouldn't reach here since it's impossible to request */
        /* to send compressed message after decompressed */
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterMatchSentOutboundMsgToNextMsgType - Transmitter 0x%p: It's impossible to send SigComp message after decompressed",
            pTrx));
        return RV_ERROR_UNKNOWN;

    default:
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "SipTransmitterMatchSentOutboundMsgToNextMsgType - Transmitter 0x%p: Shouldn't reach here",
            pTrx));
    }

    if (pTrx->eOutboundMsgCompType != eNextMsgCompType) {
        /* Set the transaction new message type */
        pTrx->eOutboundMsgCompType  = eNextMsgCompType;
        /* Reset the retransmission counter because of the change in the sent msg type */
        *pbZeroRetrans              = RV_TRUE;
    }
    

    return RV_OK;
}

/***************************************************************************
 * SipTransmitterDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a Transmitter.
 *          This means that even if the message indicates compression
 *          it will not be used.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx    - The transmitter handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDisableCompression(
                                IN  RvSipTransmitterHandle hTrx)
{
    RvStatus     rv   = RV_OK;
    Transmitter *pTrx = (Transmitter*)hTrx;

    pTrx->bSigCompEnabled = RV_FALSE;

    return rv;
}

#endif /* RV_SIGCOMP_ON */

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransmitterSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Transmitter object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - Handle to the Transmitter object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    Transmitter *pTrx = (Transmitter*)hTrx;
    memcpy(&pTrx->sctpParams, pParams, sizeof(RvSipTransportSctpMsgSendingParams));
}

/***************************************************************************
 * SipTransmitterGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Transmitter object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - Handle to the Transmitter object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransmitterGetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    Transmitter *pTrx = (Transmitter*)hTrx;
    memcpy(pParams, &pTrx->sctpParams, sizeof(RvSipTransportSctpMsgSendingParams));
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransmitterSetCompartment
 * ------------------------------------------------------------------------
 * General: Sets an handle to a SigComp Compartment within a Transmitter
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - Handle to the Transmitter object.
 *          hCompartment - Handle to the set Compartment.
 *
 * Output:  None.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetCompartment(
                    IN  RvSipTransmitterHandle  hTrx,
                    IN  RvSipCompartmentHandle  hCompartment)
{
    Transmitter *pTrx = (Transmitter *)hTrx;
    RvStatus     rv; 
    
    /* Check if the Trx already keeps the same Compartment handle */ 
    if (pTrx->hSigCompCompartment == hCompartment) 
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "SipTransmitterSetCompartment - Transmitter 0x%p, already keeps hCompartment=0x%p (should not be replaced)",
            pTrx,pTrx->hSigCompCompartment));
        return RV_OK;
    }

    if (pTrx->hSigCompCompartment != NULL)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "SipTransmitterSetCompartment - Transmitter 0x%p, replacing current hCompartment=0x%p by detaching from it",
            pTrx,pTrx->hSigCompCompartment));
        rv = SipCompartmentDetach(pTrx->hSigCompCompartment, (void*)pTrx);
        if (rv != RV_OK)
        {
            RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "SipTransmitterSetCompartment - Transmitter 0x%p, failed to detach from hCompartment=0x%p",
                pTrx,pTrx->hSigCompCompartment));
            /* Move on even in case of failure */ 
        }
        pTrx->hSigCompCompartment = NULL;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "SipTransmitterSetCompartment - Attaching Transmitter 0x%p to compartment 0x%p",
        pTrx,hCompartment));

    rv = SipCompartmentAttach(hCompartment,(void*)pTrx);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "SipTransmitterSetCompartment - Transmitter 0x%p, failed to attach to compartment=0x%p",
            pTrx,hCompartment));   
        return rv;
    }

    pTrx->hSigCompCompartment = hCompartment;

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransmitterGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves an handle to a SigComp Compartment from a Transmitter
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx          - Handle to the Transmitter object.
 *
 * Output:  phCompartment - Handle to the retrieved SigComp Compartment.
 ***************************************************************************/
void RVCALLCONV SipTransmitterGetCompartment(
                         IN  RvSipTransmitterHandle  hTrx,
                         OUT RvSipCompartmentHandle *phCompartment)
{
    Transmitter *pTrx = (Transmitter*)hTrx;
    
    *phCompartment = pTrx->hSigCompCompartment;

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "SipTransmitterGetCompartment - Transmitter 0x%p, retrieved compartment 0x%p",
        pTrx,pTrx->hSigCompCompartment));
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIGCOMP_ON
/************************************************************************************
 * HandleUpdatedSentOutboundMsg
 * ----------------------------------------------------------------------------------
 * General: Handles an updated sent message, which is about to be sent. The
 *          updated message has to be compressed again and added to the MsgToSendList
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx         - The transmitter pointer
 *          eUpdatedType - The updated message type.
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleUpdatedSentOutboundMsg(
                        IN Transmitter                  *pTrx,
                        IN RvSipTransmitterMsgCompType   eUpdatedType)
{
    RvStatus rv = RV_OK;

    switch (eUpdatedType)
    {
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE:
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
        rv = TransmitterPrepareCompressedSentMsgPage(pTrx);
        if (rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "HandleUpdatedSentOutboundMsg - Failed to prepare new compressed buffer for Transmitter 0x%p",pTrx));
            return rv;
        }
        /* Continue to the next default block */
    default:
        /* The new SentOutboundMsg has to be added to MsgToSendList */
        rv = TransmitterMsgToSendListAddElement(pTrx,pTrx->hSentOutboundMsg);
        if(rv != RV_OK)
        {
            TransmitterFreeEncodedOutboundMsg(pTrx);
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "HandleUpdatedSentOutboundMsg - trx=0x%p: Couldn't allocate outgoing 0x%p msg to list",
                pTrx,pTrx->hSentOutboundMsg));
            return rv;
        }
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransmitterChangeOwner
 * ------------------------------------------------------------------------
 * General: Changes the owner of a transmitter. (hAppTrx, and eventHandlers)
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - handle to the transmitter.
 *            hAppTrx     - new application handle to the transmitter.
 *            pEvHanders  - new event handlers structure for this transmitter.
 *            sizeofEvHandlers - size of the event handler structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterChangeOwner(
                   IN  RvSipTransmitterHandle      hTrx,
                   IN  RvSipAppTransmitterHandle   hAppTrx,
                   IN  RvSipTransmitterEvHandlers* pEvHandlers,
                   IN  RvInt32                     sizeofEvHandlers)
{
    Transmitter*  pTrx = (Transmitter*)hTrx;

    if(hTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterChangeOwner - Transmitter 0x%p, Changing owner from 0x%p to 0x%p",
              pTrx, pTrx->hAppTrx, hAppTrx));

    pTrx->hAppTrx = hAppTrx;

    /*set the transmitter event handler*/
    if(pEvHandlers != NULL)
    {
        memcpy(&pTrx->evHandlers,pEvHandlers,sizeofEvHandlers);
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransmitterDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach the owner of a transmitter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - handle to the transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDetachOwner(
                   IN  RvSipTransmitterHandle      hTrx)
{

    Transmitter*      pTrx    = (Transmitter*)hTrx;

    if(hTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "SipTransmitterDetachOwner - Transmitter 0x%p, Detach from owner 0x%p",
              pTrx, pTrx->hAppTrx));

   return TransmitterNoOwner(pTrx);
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/

#if defined(UPDATED_BY_SPIRENT)

RvStatus SipTransmitterGetLocalAddr(Transmitter *trx,RvSipTransportAddr *addressDetails) {

    RvStatus rv = RV_ERROR_UNKNOWN;

    if(trx) {

        RvSipTransportLocalAddrHandle localAddr = (RvSipTransportLocalAddrHandle)0;

        if(trx->localAddr.hAddrInUse) {
            localAddr=*(trx->localAddr.hAddrInUse);
        }

        if(!localAddr) localAddr = trx->localAddr.hLocalUdpIpv4;
        if(!localAddr) localAddr = trx->localAddr.hLocalUdpIpv6;
        if(!localAddr) localAddr = trx->localAddr.hLocalTcpIpv4;
        if(!localAddr) localAddr = trx->localAddr.hLocalTcpIpv6;

        if(localAddr && addressDetails) {
            memset(addressDetails,0,sizeof(RvSipTransportAddr));
#if (RV_NET_TYPE & RV_NET_IPV6)
            addressDetails->Ipv6Scope = UNDEFINED;
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
            rv = TransportMgrLocalAddressGetDetails(localAddr,addressDetails);
            if(rv >=0 ) {
                RVSIP_ADJUST_VDEVBLOCK(addressDetails->vdevblock);
            }
        }
    }
    return rv;
}
#endif
