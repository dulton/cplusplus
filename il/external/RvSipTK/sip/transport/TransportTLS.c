
#ifdef __cplusplus
extern "C" {
#endif

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
* Last Revision:                                                       *
*********************************************************************************
*/


/*********************************************************************************
 *                              TransportTLS.c
 *
 * This c file contains implementations for the transport layer.
 * it contain functions for opening and closing sockets, functions for sending and
 * receiving messages and functions for identifying the destination host and it's
 * ip address.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    UdiWeintrob                   Jan 2003
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                               MACROES                                 */
/*-----------------------------------------------------------------------*/
#define TLS_EVENT_STR_SIZE 128

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCore.h"
#include "_SipTransport.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#include "TransportCallbacks.h"
#include "rvtls.h"

#include "RvSipAddress.h"
#include "RvSipPartyHeader.h"

#if (RV_TLS_TYPE != RV_TLS_NONE)

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV TlsHandleHandshakeComplete(IN TransportConnection   *pConn);

static RvStatus RVCALLCONV TlsContinueHandshake(IN TransportConnection    *pConn);

static RvBool TlsSaftyTimerExpiredEvHandler(IN void           *pContext,
                                            IN RvTimer *timerInfo);


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar * TlsEvent2Str (IN  RvTLSEvents tlsEvent, OUT RvChar* strEv);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
/*-------------------------------------------------------------------------*/
/*                      FUNCTIONS IMPLEMENTATION                           */
/*-------------------------------------------------------------------------*/
/************************************************************************************
 * ConvertCoreTLSMethodToSipTlsMethod
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
void RVCALLCONV ConvertCoreTLSMethodToSipTlsMethod(
                                IN  RvTLSMethod eCoreTlsMethod,
                                OUT RvSipTransportTlsMethod   *eSipTlsMethod)

{
    /*convert types*/
    switch (eCoreTlsMethod)
    {
    case RV_TLS_SSL_V2:
        *eSipTlsMethod = RVSIP_TRANSPORT_TLS_METHOD_SSL_V2;
        break;
    case RV_TLS_SSL_V3:
        *eSipTlsMethod = RVSIP_TRANSPORT_TLS_METHOD_SSL_V3;
        break;
    case RV_TLS_TLS_V1:
        *eSipTlsMethod = RVSIP_TRANSPORT_TLS_METHOD_TLS_V1;
        break;
    default:
        *eSipTlsMethod = RVSIP_TRANSPORT_TLS_METHOD_UNDEFINED;

    }
}

/************************************************************************************
 * ConvertCoreTLSKeyToSipTlsKey
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
void RVCALLCONV ConvertCoreTLSKeyToSipTlsKey(
                                IN  RvPrivKeyType eCoreTlsKeyType,
                                OUT RvSipTransportPrivateKeyType  *eSipTlsKeyType)
{
    /*convert types*/
    switch (eCoreTlsKeyType)
    {
    case RV_TLS_RSA_KEY:
        *eSipTlsKeyType = RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_RSA_KEY;
        break;
    default:
        *eSipTlsKeyType = RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_UNDEFINED;

    }
}

/************************************************************************************
 * ConvertSipTLSKeyToCoreTlsKey
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
void RVCALLCONV ConvertSipTLSKeyToCoreTlsKey(
                                IN RvSipTransportPrivateKeyType       eSipTlsKeyType,
                                OUT  RvPrivKeyType      *eCoreTlsKeyType)
{
    /*convert types*/
    switch (eSipTlsKeyType)
    {
    case RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_RSA_KEY:
        *eCoreTlsKeyType = RV_TLS_RSA_KEY;
        break;
    default:
    break;
    }
}

/************************************************************************************
 * ConvertSipTlsMethodToCoreTLSMethod
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
void RVCALLCONV ConvertSipTlsMethodToCoreTLSMethod(
                                IN RvSipTransportTlsMethod       eSipTlsMethod,
                                OUT  RvTLSMethod *eCoreTlsMethod)

{
    /*convert types*/
    switch (eSipTlsMethod)
    {
    case RVSIP_TRANSPORT_TLS_METHOD_SSL_V2:
        *eCoreTlsMethod = RV_TLS_SSL_V2;
        break;
    case RVSIP_TRANSPORT_TLS_METHOD_SSL_V3:
        *eCoreTlsMethod = RV_TLS_SSL_V3;
        break;
    case RVSIP_TRANSPORT_TLS_METHOD_TLS_V1:
        *eCoreTlsMethod = RV_TLS_TLS_V1;
        break;
    default:
        break;
    }
}

/************************************************************************************
 * TransportTLSFullHandshakeAndAllocation
 * ----------------------------------------------------------------------------------
 * General: starts the handshake for a TLS connection:
 *          - Allocates and initiates date
 *          - Start the TLS handshake procedure
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - the connection on which to preform the handshake
 * Output:  (-)
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTLSFullHandshakeAndAllocation(
                                         IN TransportConnection                *pConn)
{
    RvStatus           rv              = RV_OK;

    rv = RA_Alloc(pConn->pTransportMgr->tlsMgr.hraSessions,(RA_Element*)&(pConn->pTlsSession));
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTLSFullHandshakeAndAllocation - connection 0x%p: failed to allocate TLS session for the connection rv=%d",pConn,rv));
    }

    if (RV_OK == rv)
    {
        rv = RvTLSSessionConstructEx(
                                   &pConn->pTlsEngine->tlsEngine,
                                   &pConn->pTlsEngine->engineLock,
                                   &pConn->pTripleLock->hLock,
                                   NULL, /*cfg - use default configuration*/
                                   pConn->pTransportMgr->pLogMgr,
                                   pConn->pTlsSession);
        if (RV_OK != rv)
        {
            rv = CRV2RV(rv);
            RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportTLSFullHandshakeAndAllocation - connection 0x%p: failed to build the TLS session for the connection rv=%d",pConn,rv));
        }
    }
    if (RV_OK != rv)
    {
        TransportConnectionNotifyStatus(
                       pConn,
                       NULL,
                       NULL,
                       RVSIP_TRANSPORT_CONN_STATUS_ERROR,
                       RV_TRUE);
        TransportTCPClose(pConn);
        return rv;
    }
    pConn->eTlsHandShakeState      = TRANSPORT_TLS_HANDSHAKE_STARTED;


    /* Starting a timer on the handshake. if the timer has expired and no handshake was completed yet, the connection will be closed */
    SipCommonCoreRvTimerStartEx(
                &pConn->saftyTimer,
                pConn->pTransportMgr->pSelect,
                pConn->pTransportMgr->tlsMgr.TlsHandshakeTimeout,
                TlsSaftyTimerExpiredEvHandler,
                pConn);

    /* Calling to handshake must go through select at least once (core reasons)
       I'm inserting the handshake to select. handshake will be completed once
       a few select calls will be made */
    pConn->eTlsEvents = RV_TLS_HANDSHAKE_EV;

    TransportCallbackTLSStateChange(pConn,
                            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED,
                            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                            RV_TRUE, RV_TRUE);

    TransportTlsCallOnThread(pConn,RV_TRUE);

    return rv;
}

/************************************************************************************
 * TransportTlsRenegotiate
 * ----------------------------------------------------------------------------------
 * General: does a renegotiation on a TLS connection
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsRenegotiate(IN TransportConnection   *pConn)
{
    RvStatus crv = RV_OK;
#if RV_TLS_ENABLE_RENEGOTIATION
    crv = RvTLSSessionRenegotiateEx(pConn->pTlsSession,
                                    &pConn->pTripleLock->hLock,
                                    pConn->pTransportMgr->pLogMgr);
    if (RV_OK != crv)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportTlsRenegotiate - connection 0x%p: error in renegotiation. rv=%d",
            pConn, RvErrorGetCode(crv)));
        return CRV2RV(crv);
    }
#else
    RV_UNUSED_ARG(pConn);
#endif /*RV_TLS_ENABLE_RENEGOTIATION*/
    return CRV2RV(crv);
}


/******************************************************************************
 * TransportTLSHandleReadWriteEvents
 * ----------------------------------------------------------------------------
 * General: Handles read and write event for TLS.
 *          Translates the event and acts accordingly.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          event - The event to be treated
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportTLSHandleReadWriteEvents(
                                                IN TransportConnection* pConn,
                                                IN RvSelectEvents       event)
{
    RvStatus       rv          = RV_OK;
    RvTLSEvents    tlsEvents   = 0;

    if (pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTLSHandleReadWriteEvents - connection 0x%p: Waiting for user to start handshake",pConn));
        return RV_OK;
    }

    RvTLSTranslateSelectEvents(pConn->pTlsSession,
                               event,
                               &pConn->pTripleLock->hLock,
                               pConn->pTransportMgr->pLogMgr,
                               &tlsEvents);

    /* If the TLS_READ or TLS_WRITE was raised as a result of socket shutdown
    performed by TransportTCPClose() on RvTLSSessionClient/ServerHandshake()
    failure, just clean the socket and return, while waiting for CLOSE.
    No TLS proceeding should be done, since the handshake was not completed. */
    if ((tlsEvents & (RV_TLS_WRITE_EV | RV_TLS_WRITE_EV)) &&
        RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED == pConn->eTlsState)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTLSHandleReadWriteEvents - connection 0x%p: read/write event in state HANDSHAKE_FAILED. Clean socket and return.",
            pConn));
        /* Empty socket */
        RvSocketClean(pConn->pSocket,pConn->pTransportMgr->pLogMgr);
        /* Re-register socket events */
        rv = TransportConnectionRegisterSelectEvents(
                    pConn, RV_TRUE/*bAddClose*/, RV_FALSE/*bFirstCall*/);
        if (rv != RV_OK)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTLSHandleReadWriteEvents - connection 0x%p: failed to register socket events (rv=%d:%s)",
                pConn, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        return RV_OK;
    }

    if (tlsEvents & RV_TLS_HANDSHAKE_EV)
    {
        rv = TlsContinueHandshake(pConn);
        return rv;
    }
    else if (tlsEvents & RV_TLS_WRITE_EV)
    {
        void* pEv;

        /* If previous WRITE_EVENT was not peeked out from TPQ -> no write was
        made. Therefore just ignore the current TLS_write.
        In addition - remove TLS_write from requested events. */
        if (RV_TRUE == pConn->bWriteEventInQueue)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTLSHandleReadWriteEvents - connection 0x%p: bWriteEventInQueue is RV_TRUE => Ignore current TLS_WRITE",
                pConn));
            pConn->eTlsEvents = RV_TLS_READ_EV;
            TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
            return RV_OK;
        }
        rv = TransportConnectionAddWriteEventToTPQ(pConn, &pEv);

        /* If TransportConnectionAddWriteEventToTPQ() succeeded, the WRITE was
        added successfully to the TPQ. Remove registration for TLS_WRITE until
        write() will be performed. The last occurs on peek WRITE from TPQ. */
        if (RV_OK == rv)
        {
            pConn->eTlsEvents = RV_TLS_READ_EV;
            rv = TransportTlsCallOnThread(pConn, RV_TRUE/*bAddClose*/);
            if (RV_OK != rv)
            {
                RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "TransportTLSHandleReadWriteEvents - connection 0x%p: failed to register select events(rv=%d:%s)",
                    pConn, rv, SipCommonStatus2Str(rv)));
                TransportProcessingQueueFreeEvent(
                    (RvSipTransportMgrHandle)pConn->pTransportMgr,
                    (TransportProcessingQueueEvent*)pEv);
                return rv;
            }
        }
    }
    else if (tlsEvents & RV_TLS_READ_EV)
    {
        if (pConn->eTlsHandShakeState != TRANSPORT_TLS_HANDSHAKE_COMPLEATED)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "TransportTLSHandleReadWriteEvents - connection 0x%p: read event when handshake was not completed",
                pConn));
            TransportTCPClose(pConn);
            /* Re-register socket events */
            rv = TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
		    if (rv != RV_OK)
		    {
                RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                    "TransportTLSHandleReadWriteEvents - connection 0x%p: TransportTlsCallOnThread() failed while handling TLS_READ event(rv=%d:%s)",
                    pConn, rv, SipCommonStatus2Str(rv)));
                return rv;
		    }
            return RV_OK;
        }

        if (pConn->eTlsState == RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED)
        {
            TransportTlsClose(pConn);
            return RV_OK;
        }

        rv = TransportConnectionReceive(pConn);
    }
    else if (tlsEvents & RV_TLS_SHUTDOWN_EV)
    {
        rv = TransportTlsClose(pConn);
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTLSHandleReadWriteEvents - connection 0x%p: Could not match event for the connection",pConn));
    }

    if (RV_OK != rv)
    {
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        RvChar str[TLS_EVENT_STR_SIZE];
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportTLSHandleReadWriteEvents - connection 0x%p: error on handling event %s (rv=%d:%s)",
            pConn, TlsEvent2Str(tlsEvents, str), rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
}


/************************************************************************************
 * TransportTlsCallOnThread
 * ----------------------------------------------------------------------------------
 * General: Implement call on thread for Tls connections
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          bAddClose - whether Close event should be called on
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsCallOnThread(
                            IN TransportConnection* pConn,
                            IN RvBool               bAddClose)
{
    RvStatus rv;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvChar str[TLS_EVENT_STR_SIZE];
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    RvTLSTranslateTLSEvents(pConn->pTlsSession,
                            pConn->eTlsEvents,
                            &pConn->pTripleLock->hLock,
                            pConn->pTransportMgr->pLogMgr,
                            &pConn->ccEvent);

    RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
        "TransportTlsCallOnThread- Connection 0x%p (socket %d) calling on TLS events %s (select events %x)",
        pConn, pConn->ccSocket, TlsEvent2Str(pConn->eTlsEvents,str),
        pConn->ccEvent));

    rv = TransportConnectionRegisterSelectEvents(pConn,bAddClose,RV_FALSE);
    return rv;
}

/* sipit correction */
#define TLS_SHUTDOWN_SAFE_COUNTER_MAX_TRIES 5
#define TLS_SHUTDOWN_SAFTY_TIMER            4000
/************************************************************************************
 * TransportTlsClose
 * ----------------------------------------------------------------------------------
 * General: stars the disconnect and shut sown process for TLS connections.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsClose(IN TransportConnection   *pConn)
{
    RvStatus rv = RV_OK;
    
    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
        "TransportTlsClose - Connection 0x%p: Starting TLS close",
        pConn));
    
    if (RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED != pConn->eTlsState &&
        RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED              != pConn->eTlsState )
    {
        TransportConnectionHashRemove(pConn);
        
        TransportCallbackTLSStateChange(pConn,
            RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED,
            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
            RV_TRUE,RV_TRUE);

        /* Cancel waiting for HANDSHAKE event */
        pConn->eTlsEvents &= ~RV_TLS_HANDSHAKE_EV;
        pConn->eTlsEvents |= RV_TLS_READ_EV;
        TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
    }

    /* make sure we know that we asked for a connection closure, that is 
    our "mark" to stop sending data on this connection*/
    pConn->bClosedByLocal = RV_TRUE;
    
    if (TRANSPORT_TLS_HANDSHAKE_COMPLEATED != pConn->eTlsHandShakeState)
    {
        /* The handshake did not start yet, but the connection has to be closed.
           There is no need for TLS close alert. we can be satisfied with TCP teardown */
        TransportTCPClose(pConn);
        return RV_OK;
    }

    /* the counter advances each time we try to gracefully shutdown, after 5 times,
    we disconnect TCP without care to TLS shutdown alert swappings */
    if(pConn->pSocket != NULL                                                &&
        NULL != pConn->pTlsSession                                            &&
        TLS_SHUTDOWN_SAFE_COUNTER_MAX_TRIES > pConn->tlsShutdownSafeCounter++ )
    {
        /* Starting a timer on the handshake. if the timer has expired and no close seq was not 
           completed yet, the connection will be closed */
        if (!SipCommonCoreRvTimerStarted(&pConn->saftyTimer))
        {
            SipCommonCoreRvTimerStartEx(
                &pConn->saftyTimer,
                pConn->pTransportMgr->pSelect,
                TLS_SHUTDOWN_SAFTY_TIMER,
                TlsSaftyTimerExpiredEvHandler,
                pConn);
        }

        
        rv = RvTLSSessionShutdown(pConn->pTlsSession,
            &pConn->pTripleLock->hLock,
            pConn->pTransportMgr->pLogMgr);
        
        if (RV_TLS_ERROR_WILL_BLOCK == RvErrorGetCode(rv) ||
            RV_TLS_ERROR_INCOMPLETE == RvErrorGetCode(rv))
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportTlsClose - Connection 0x%p: Call to shutdown blocked , (rv=%d)",
                pConn,RvErrorGetCode(rv)));
            /* On BLOCK we have to register select events,needed for completion
            of the blocked operation. RvTLSTranslateTLSEvents() called from
            TransportTlsCallOnThread() will tell us which events are needed. */
            TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
            /* it is ok for the call to hand shake to fail, so we return success */
            return RV_OK;
        }
        
        else if (RV_OK != RvErrorGetCode(rv))
        {
            RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TransportTlsClose - Connection 0x%p: failed to shutdown gracefully. closing (crv=%d)",
                pConn, rv));
        }
    }
    
    /* direct close will be called if TLS shutdown failed or if it has succeeded */
    SipCommonCoreRvTimerCancel(&pConn->saftyTimer);
    TransportTCPClose(pConn);
    return RV_OK;
}

/************************************************************************************
 * TransportTlsPostConnectionAssertion
 * ----------------------------------------------------------------------------------
 * General: Asserts that the certificate on the connection was indeed given
 *          to the computer we are connecting to.
 *          Assertion can be obtained through one of the following conditions:
 *          1. If the connection was already asserted, it is asserted.
 *          2. If the core function for assertion tell us the connection is asserted.
 *          3. If the application declares the connection as asserted regardless of the
 *             stack decision.
 *          4. If the connection did not request for client certificate.
 *          If the connection was not asserted successfully, the connection moves to state HS failed,
 *             and notifies all owners on the error.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn   - Pointer to the relevant connection.
 *          hMsg    - in case of incoming messages the message is supplied
 * Output:  pbAssertionResult - assertion result.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsPostConnectionAssertion(IN  TransportConnection   *pConn,
                                                         IN  RvSipMsgHandle         hMsg,
                                                         OUT RvBool               *pbAssertionResult)
{
    RvStatus   rv                          = RV_OK;
    RvBool     bPostConnectionAssertion    = RV_TRUE;
    RvChar strSipAddr[MAX_HOST_NAME_LEN];
    RvChar strBufferForCore[MAX_HOST_NAME_LEN];

    RV_UNUSED_ARG(hMsg);

    *pbAssertionResult = RV_FALSE;
    /* if the connection was already asserted or no certificate checking was required,
       there is no need to re-assert it */
    if (RV_TRUE != pConn->bConnectionAsserted && NULL != pConn->pfnVerifyCertEvHandler &&
        UNDEFINED != pConn->strSipAddrOffset)
    {
        RvInt32 nameLen = 0;
        nameLen = RPOOL_Strlen(pConn->pTransportMgr->hElementPool,pConn->hConnPage,pConn->strSipAddrOffset);

        rv = RPOOL_CopyToExternal(pConn->pTransportMgr->hElementPool,
                                  pConn->hConnPage,
                                  pConn->strSipAddrOffset,
                                  (void*)strSipAddr,
                                  nameLen+1);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTlsPostConnectionAssertion - Connection 0x%p: post connection assertion failed because the str name could not be retrieved",
                  pConn));
        }

        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
              "TransportTlsPostConnectionAssertion - Connection 0x%p: post connection check asserting with name: %s",
              pConn,strSipAddr));

        /* post connection assertion with core function */
        rv = RvTLSSessionCheckCertAgainstName(
            pConn->pTlsSession,
            (RvChar*)strSipAddr,
            &pConn->pTripleLock->hLock,
            strBufferForCore,
            MAX_HOST_NAME_LEN,
            pConn->pTransportMgr->pLogMgr);
        if (RV_OK != rv)
        {
            bPostConnectionAssertion = RV_FALSE;
        }

        /* Allowing the application to over ride the post connection assertion decision */
        if (RV_FALSE == bPostConnectionAssertion)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTlsPostConnectionAssertion - Connection 0x%p: post connection assertion failed, consulting the application",
                  pConn));
            TransportCallbackTLSAppPostConnectionAssertionEv(pConn,(RvChar*)strSipAddr,NULL,&bPostConnectionAssertion);
        }

        /* If the application does not approve the certificate, we kill the connection. it is not safe */
        if (RV_FALSE == bPostConnectionAssertion)
        {
            RvLogWarning(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTlsPostConnectionAssertion - Connection 0x%p: post connection assertion failed. this might be a security hazard",
                  pConn));

            TransportCallbackTLSStateChange(pConn,
                                    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED,
                                    RVSIP_TRANSPORT_CONN_REASON_TLS_POST_CONNECTION_ASSERTION_FAILED,
                                    RV_FALSE,RV_TRUE);

            TransportTlsClose(pConn);
            return RV_OK;
        }
    }

    if (UNDEFINED == pConn->strSipAddrOffset)
    {
            RvLogWarning(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TransportTlsPostConnectionAssertion - Connection 0x%p: no host name was set for post connection assertion asserting by default",
                  pConn));
    }

    *pbAssertionResult = pConn->bConnectionAsserted = RV_TRUE;

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
          "TransportTlsPostConnectionAssertion - Connection 0x%p: post connection assertion succeeded.",
          pConn));

    rv = TransportCallbackTLSStateChange(pConn,
                            RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED,
                            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                            RV_FALSE, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportTlsPostConnectionAssertion - Connection 0x%p: failed to report CONNECTED state(rv=%d:%s). Close the connection",
            pConn, rv, SipCommonStatus2Str(rv)));
        TransportTlsClose(pConn);
        return rv;
    }

    return RV_OK;
}

/************************************************************************************
 * TransportTlsDefaultCertificateVerificationCB
 * ----------------------------------------------------------------------------------
 * General: This is the most basic implementation of certification verification CB.
 *          All we do is return the code that we got from SSL
 * Return Value: RvInt - what ever we got in the ok in param. This is an openSSL
 *               requirement. it enables it to move errors down a certificate chain.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   ok   - 0 means not OK. o/w it's ok
 *          certificate the certificate object, we do not touch it in this implementation.
 * Output:  (-)
 ***********************************************************************************/
RvInt32 TransportTlsDefaultCertificateVerificationCB(IN  RvInt32 ok,
                                                    IN  RvSipTransportTlsCertificate  certificate)
{
    RV_UNUSED_ARG(certificate);
    return ok;
}

/***************************************************************************
 * TransportTlsStateChangeQueueEventHandler
 * ------------------------------------------------------------------------
 * General: call the TLS state change function of a connection.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pConnObj - The connection to terminate.
 *            status -      The connection status
 ***************************************************************************/
RvStatus  RVCALLCONV TransportTlsStateChangeQueueEventHandler(IN void    *pConnObj,
                                                              IN RvInt32 state,
                                                              IN RvInt32 notUsed,
                                                              IN RvInt32 objUniqueIdentifier)
{


    TransportConnection* pConn  = (TransportConnection*)pConnObj;
    RvStatus            rv     = RV_OK;

    if (pConnObj == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return rv;
    }

    if(objUniqueIdentifier != pConn->connUniqueIdentifier)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc, 
            "TransportTlsStateChangeQueueEventHandler - Conn 0x%p - different identifier. Ignore the event.",
            pConn));
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }

    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc, 
            "TransportTlsStateChangeQueueEventHandler - Conn 0x%p - event is out of the event queue", pConn));

    /* Call "state changed" callback */
    rv = TransportCallbackTLSStateChange(pConn,
                            (RvSipTransportConnectionTlsState)state,
                            pConn->eTlsReason,
                            RV_FALSE, RV_FALSE);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
            "TransportTlsStateChangeQueueEventHandler - Connection 0x%p: failed to report %d state(rv=%d:%s). Close the connection",
            pConn, state,  rv, SipCommonStatus2Str(rv)));
        TransportTlsClose(pConn);
        TransportConnectionUnLockEvent(pConn);
        return rv;
    }

    TransportConnectionUnLockEvent(pConn);
    RV_UNUSED_ARG(notUsed)
    return RV_OK;
}

/***************************************************************************
 * TransportTlsShutdown
 * ------------------------------------------------------------------------
 * General: kills all remaining TLS engins
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pMgr - transport mgr
 ***************************************************************************/
void  RVCALLCONV TransportTlsShutdown(IN TransportMgr* pMgr )
{
    RvStatus        found   = RV_OK;
    SipTlsEngine*   pEng    = NULL;
    RvInt32         loc     = 0;
    RV_Resource     engRes;

    while (RV_OK == found)
    {
        RA_GetResourcesStatus(pMgr->tlsMgr.hraEngines,&engRes);
        if (0 == engRes.numOfUsed)
        {
            break;
        }
        found = RA_GetFirstUsedElement(pMgr->tlsMgr.hraEngines,loc,(RA_Element*)&pEng,&loc);
        RvTLSEngineDestruct(&pEng->tlsEngine,&pEng->engineLock,pMgr->pLogMgr);
        RvMutexDestruct(&pEng->engineLock,pMgr->pLogMgr);
        RA_DeAlloc(pMgr->tlsMgr.hraEngines,(RA_Element)pEng);
    }
}

/******************************************************************************
 * TransportTlsReceive
 * ----------------------------------------------------------------------------
 * General: reads data from Connection socket
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn      - The Connection object.
 * Output:  recvBuf    - Buffer into which the data should be read from socket.
 *          pRecvBytes - Number of received bytes.
 *****************************************************************************/
RvStatus RVCALLCONV TransportTlsReceive(
                                IN  TransportConnection* pConn,
                                OUT RvChar*              recvBuf,
                                OUT RvSize_t*            pRecvBytes)
{
    RvInt    bytesToRead = 0;
    RvStatus crv;

    bytesToRead = pConn->pTransportMgr->maxBufSize-1;
    crv = RvTLSSessionReceiveBufferEx(pConn->pTlsSession,
                                      recvBuf,
                                      &pConn->pTripleLock->hLock,
                                      pConn->pTransportMgr->pLogMgr,
                                      &bytesToRead,
                                      &pConn->eTlsEvents);

    if (RV_TLS_ERROR_WILL_BLOCK == RvErrorGetCode(crv))
    {
        /* On BLOCK we have to register select events,needed for
           completion of the blocked operation. RvTLSTranslateTLSEvents()
           called from TransportTlsCallOnThread() will tell us which events are
           needed. */
        TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
        return RV_OK;
    }
    else if (RV_TLS_ERROR_SHUTDOWN == RvErrorGetCode(crv))
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTlsReceive - Connection 0x%p: Got shutdown event. shutting down the TLS connection",pConn));
        TransportTlsClose(pConn);
        return RV_OK;
    }
    else if (RV_OK != RvErrorGetCode(crv))
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportTlsReceive - Connection 0x%p: TLS failure in reading returning RV_ERROR_UNKNOWN (err=%d)",
            pConn, RvErrorGetCode(crv)));
        /* Error on receiving indicates some problem with protocol flow,
           e.g. reception of junk data instead of handshake packets.
           Further behavior of SSL session, serving the connection, is
           unpredictable. Therefore terminate the connection here.*/
        TransportTlsClose(pConn);
        return RV_ERROR_UNKNOWN;
    }
    else if (bytesToRead > 0)
    { /* read() succeeded */
        /* Register to TLS_READ again - listen for incoming messages */
        pConn->eTlsEvents |= RV_TLS_READ_EV;
        TransportTlsCallOnThread(pConn, RV_TRUE /*bAddClose*/);
    }

    *pRecvBytes = bytesToRead;

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TlsSaftyTimerExpiredEvHandler
 * ------------------------------------------------------------------------
 * General: Called when a TLS handshake timer is expired. The connection
 *          will be closed
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The connection using this timer.
 ***************************************************************************/
static RvBool TlsSaftyTimerExpiredEvHandler(
                                            IN void    *pContext,
                                            IN RvTimer *timerInfo)
{
    RvStatus rv;
    TransportConnection* pConn = (TransportConnection*)pContext;
    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pConn->saftyTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc  ,
            "TlsSaftyTimerExpiredEvHandler - Connection 0x%p : timer expired but was already released. do nothing...",
            pConn));
        
        TransportConnectionUnLockEvent(pConn);
        return RV_FALSE;
    }

    SipCommonCoreRvTimerExpired(&pConn->saftyTimer);
    SipCommonCoreRvTimerCancel(&pConn->saftyTimer);

    RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
              "TlsSafetyTimerExpiredEvHandler- Connection 0x%p - timer expired, closing connection ",pConn));

    if (RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED == pConn->eTlsState)
    {
        TransportTCPClose(pConn);
        TransportConnectionUnLockEvent(pConn);
        return RV_FALSE;
    }

    TransportConnectionNotifyStatus(
                   pConn,
                   NULL,
                   NULL,
                   RVSIP_TRANSPORT_CONN_STATUS_ERROR,
                   RV_FALSE);

    TransportConnectionDisconnect(pConn);
    TransportConnectionUnLockEvent(pConn);

    return RV_FALSE;
}

/************************************************************************************
 * TlsHandleHandshakeComplete
 * ----------------------------------------------------------------------------------
 * General: Handles read and write event for TLS (similar function exists for TCP)
 *          Translates the event and act accordingly
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTcpConn - Pointer to the relevant connection.
 *          event    - The event to be treated
 * Output:
 ***********************************************************************************/
static RvStatus RVCALLCONV TlsHandleHandshakeComplete(IN TransportConnection   *pConn)
{
    RvStatus   rv          = RV_OK;
    RvBool bPostConnectionAssertion = RV_TRUE;


    RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
          "TlsHandleHandshakeComplete - Connection 0x%p: Handshake completed (TCP side=%d, TLS side=%d)",
          pConn,pConn->eConnectionType,pConn->eTlsHandshakeSide));

    /* Kill timers waiting for handshake */
    if(SipCommonCoreRvTimerStarted(&pConn->saftyTimer))
    {
        SipCommonCoreRvTimerCancel(&pConn->saftyTimer);
    }

    pConn->eTlsHandShakeState = TRANSPORT_TLS_HANDSHAKE_COMPLEATED;

    TransportCallbackTLSStateChange(pConn,
                            RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED,
                            RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                            RV_FALSE, RV_TRUE);

    /* post connection assertion is done here*/
    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT == pConn->eConnectionType)
    {

        rv = TransportTlsPostConnectionAssertion(pConn,
                                                 NULL,
                                                 &bPostConnectionAssertion);
        TransportConnectionFreeConnPage(pConn);

        if (RV_OK != rv)
        {
            return RV_ERROR_UNKNOWN;
        }
        if (RV_FALSE == bPostConnectionAssertion)
        {
            return rv;
        }
    }
    else /* server connection does not need post assertion checks, and go directly to state connected */
    {
        rv = TransportCallbackTLSStateChange(pConn,
                                RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED,
                                RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                                RV_FALSE, RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                "TlsHandleHandshakeComplete - Connection 0x%p: failed to report CONNECTED state(rv=%d:%s). Close the connection",
                pConn, rv, SipCommonStatus2Str(rv)));
            TransportTlsClose(pConn);
            return rv;
        }
    }

    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT == pConn->eConnectionType)
    {
        pConn->eTlsEvents = RV_TLS_WRITE_EV | RV_TLS_READ_EV;
    }
    else if (RVSIP_TRANSPORT_CONN_TYPE_SERVER == pConn->eConnectionType ||
             RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER == pConn->eConnectionType)
    {
        pConn->eTlsEvents = RV_TLS_READ_EV;
    }

    TransportTlsCallOnThread(pConn, RV_TRUE);
    return rv;
}

/************************************************************************************
 * TlsContinueHandshake
 * ----------------------------------------------------------------------------------
 * General: starts/continues the handshake for a TLS connection:
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the connection on which to preform the handshake
 * Output:  (-)
 ***********************************************************************************/
static RvStatus RVCALLCONV TlsContinueHandshake(IN TransportConnection    *pConn)
{
    RvStatus rv = RV_OK;

    switch (pConn->eTlsHandshakeSide)
    {
    case RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_CLIENT:
        {
            rv = RvTLSSessionClientHandshake(pConn->pTlsSession,
                                             (RvCompareCertificateCB)pConn->pfnVerifyCertEvHandler,
                                             pConn->ccSocket,
                                             &pConn->pTripleLock->hLock,
                                             pConn->pTransportMgr->pLogMgr);
        }
        break;
    case RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_SERVER:
        {
            rv = RvTLSSessionServerHandshake(pConn->pTlsSession,
                                             (RvCompareCertificateCB)pConn->pfnVerifyCertEvHandler,
                                             pConn->ccSocket,
                                             &pConn->pTripleLock->hLock,
                                             pConn->pTransportMgr->pLogMgr);
        }
        break;
    default:
        break;
    } /* switch (pConn->eConnectionType) */

    if (RV_TLS_ERROR_WILL_BLOCK == RvErrorGetCode(rv))
    {
#if defined(RV_SSL_SESSION_STATUS)
        RvBool bContinue = RV_TRUE;
#endif /*RV_SSL_SESSION_STATUS*/
        RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TlsContinueHandshake - Connection 0x%p: Call to handshake blocked (TCP side=%d, TLS side=%d), (rv=%d)",
                  pConn, pConn->eConnectionType,pConn->eTlsHandshakeSide,RvErrorGetCode(rv)));

#if defined(RV_SSL_SESSION_STATUS)
        TransportCallbackConnectionTlsStatus(pConn,
                                            RVSIP_TRANSPORT_CONN_TLS_STATUS_HANDSHAKE_PROGRESS,
                                            &bContinue);
        if (RV_TRUE != bContinue)
        {
            RvLogDebug(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                      "TlsContinueHandshake - Connection 0x%p: application has stopped the handshake",
                      pConn));
            rv = RV_ERROR_UNKNOWN;
        }
        else
#endif /*RV_SSL_SESSION_STATUS*/
        /* Handle RV_TLS_ERROR_WILL_BLOCK error*/
        {
            /* On BLOCK we have to register select events,needed for
               completion of the blocked operation. RvTLSTranslateTLSEvents()
               called from TransportTlsCallOnThread() will tell us
               which events are needed. */
            TransportTlsCallOnThread(pConn, RV_TRUE);
            /* it is ok for the call to hand shake to fail, so we return success */
            return RV_OK;
        }
    }

    if (RV_OK != RvErrorGetCode(rv))
    {
        RvLogError(pConn->pTransportMgr->pLogSrc ,(pConn->pTransportMgr->pLogSrc ,
                  "TlsContinueHandshake - Connection 0x%p: Call to handshake failed side=%d closing connection, (rv=%d)",
                  pConn, pConn->eConnectionType,RvErrorGetCode(rv)));

        /* If the HS failed we want to go though select to "eat" all remaining TLS messages on the
           connection. on the return from the select we will close the connection */
        TransportCallbackTLSStateChange(pConn,
                                RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED,
                                RVSIP_TRANSPORT_CONN_REASON_UNDEFINED,
                                RV_TRUE, RV_TRUE);

        /* The handshake was not completed, so TLS Close alert can not be sent.
           we can close the connection */
        TransportTCPClose(pConn);
        return RV_ERROR_UNKNOWN;
    }

    pConn->eTlsHandShakeState = TRANSPORT_TLS_HANDSHAKE_COMPLEATED;
    rv = TlsHandleHandshakeComplete(pConn);

    return rv;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/********************************************************************************************
 * TlsEvent2Str
 * purpose : This function convert event to string (in order to print it).
 * input   : Event.
 * output  : String.
 * return  : String.
 ********************************************************************************************/
static RvChar * TlsEvent2Str (IN  RvTLSEvents tlsEvent, OUT RvChar* strEv)
{
    strEv[0] = 0;
    if (tlsEvent & RV_TLS_HANDSHAKE_EV)    strcat(strEv, " TLS_Handshake ");
    if (tlsEvent & RV_TLS_READ_EV)  strcat(strEv, " TLS_read ");
    if (tlsEvent & RV_TLS_WRITE_EV)   strcat(strEv, " TLS_write ");
    if (tlsEvent & RV_TLS_SHUTDOWN_EV)   strcat(strEv, " TLS_shutdown ");
    if (tlsEvent & ~(RV_TLS_HANDSHAKE_EV|RV_TLS_READ_EV|RV_TLS_WRITE_EV|RV_TLS_SHUTDOWN_EV)) strcat(strEv, " Unknown ");

    return strEv;
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */

#ifdef __cplusplus
}
#endif

