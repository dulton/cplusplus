/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportTLS.h
 *
 *  This file provides headers for the TLS layer of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/


#ifndef TRANSPORT_TLS_H
#define TRANSPORT_TLS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "RV_SIP_DEF.h"
#include "AdsRpool.h"
#include "TransportProcessingQueue.h"


#if (RV_TLS_TYPE != RV_TLS_NONE)


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          MACROS DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define GET_ENGINE_LOCK(mgr,eng) LOCK_GetMutex((mgr)->hTlsLockPool, \
                                RA_GetByPointer((mgr)->tlsMgr.hraEngines,(RA_Element)eng))

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

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
                                OUT RvSipTransportTlsMethod       *eSipTlsMethod);

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
                                OUT RvSipTransportPrivateKeyType       *eSipTlsKeyType);

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
                                OUT  RvPrivKeyType      *eCoreTlsKeyType);

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
                                OUT  RvTLSMethod *eCoreTlsMethod);

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
                                         IN TransportConnection                *pConn);

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
RvStatus RVCALLCONV TransportTlsRenegotiate(IN TransportConnection   *pConn);

/************************************************************************************
 * TransportTLSHandleReadWriteEvents
 * ----------------------------------------------------------------------------------
 * General: Handles read and write event for TLS (similar function exists for TCP)
 *          Translates the event and act accordingly
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          event    - The event to be treated
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTLSHandleReadWriteEvents(IN TransportConnection   *pConn,
                                                       IN RvSelectEvents           event);

/************************************************************************************
 * TransportTlsContinueHandshake
 * ----------------------------------------------------------------------------------
 * General: starts/continues the handshake for a TLS connection:
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the connection on which to preform the handshake
 * Output:  (-)
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsContinueHandshake(IN TransportConnection    *pConn);

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
                            IN RvBool               bAddClose);

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
RvStatus RVCALLCONV TransportTlsClose(IN TransportConnection   *pConn);

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
 *          hMsg    - in case of incoming messages the message is suplied
 * Output:  pbAssertionResult - assertion result.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportTlsPostConnectionAssertion(IN  TransportConnection   *pConn,
                                                         IN  RvSipMsgHandle         hMsg,
                                                         OUT RvBool               *pbAssertionResult);

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
                                                    IN  RvSipTransportTlsCertificate  certificate);

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
                                                              IN RvInt32 objUniqueIdentifier);

/***************************************************************************
 * TransportTlsShutdown
 * ------------------------------------------------------------------------
 * General: kills all remaining TLS engins
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pMgr - transport mgr
 ***************************************************************************/
void  RVCALLCONV TransportTlsShutdown(IN TransportMgr* pMgr );


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
                                OUT RvSize_t*            pRecvBytes);

#endif /*  (RV_TLS_TYPE != RV_TLS_NONE)*/

#endif /*TRANSPORT_TLS_H*/

#ifdef __cplusplus
}
#endif

