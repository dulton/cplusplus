/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportCallbacks.h
 *
 * transport layer callback function wrappers
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                1.4.2003
 *********************************************************************************/


#ifndef TRANSPORT_CALLBACKS_H
#define TRANSPORT_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransportMgrObject.h"
#include "TransportConnection.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransportCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General:  notifies the application that a new message is about to be sent.
 *           The application can decide whether the transport layer
 *           should not transmit the message to its destination.
 * Return Value: RV_TURE to transmit the message. RV_FALSE to discard the
 *               message without transmitting it to destination.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrasportMgr - A pointer to the transport manager object.
 *          msgBuffer - The about to be sent message (given in a consecutive
 *                      buffer).
 *          bufferLen - The length of the message buffer.
 ***************************************************************************/
RvBool RVCALLCONV  TransportCallbackMsgToSendEv(
                      IN    TransportMgr               *pTrasportMgr,
                      IN    RvChar                    *msgBuffer,
                      IN    RvUint                    bufferLen);



/***************************************************************************
 * TransportCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a new message was received.
 *           The application can decide whether the transport layer
 *           should discard the message, in which case the stack will not
 *           process this message.
 * Return Value: RV_TURE to accept the receipt of the message and process it
 *               in the stack. RV_FALSE to discard the message without
 *               further processing.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - A pointer to the transport manager object.
 *          hMsgReveived - The received message.
 ***************************************************************************/
RvBool RVCALLCONV TransportCallbackMsgReceivedEv(
                      IN    TransportMgr              *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived);

/******************************************************************************
 * TransportCallbackMsgReceivedExtEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr	- Pointer to the transport manager object.
 *          hMsgReceived	- Handle to the received message.
 *          pRcvdMsgDetails - Pointer to received message details struct
 *          pRecvFromAddr   - Pointer to Received From Address in SIPTK format
 *          pSecOwner       - Pointer to Security Object, owning the Local
 *                            Address or Connection, on which the message was
 *                            received. Can be NULL.
 * Output:  pbProcessMsg    - The application can use this parameter for
 *							  asking the SIP Stack to ignore the incoming
 *							  message by setting this out parameter to
 *							  RV_FALSE (otherwise its value should be set
 *							  to RV_TRUE).
 *****************************************************************************/
void RVCALLCONV TransportCallbackMsgReceivedExtEv (
                         IN  TransportMgr                 *pTransportMgr,
                         IN  RvSipMsgHandle				   hMsgReceived,
						 IN  RvSipTransportRcvdMsgDetails *pRcvdMsgDetails,
                         IN  SipTransportAddress          *pRecvFromAddr,
                         IN  void                         *pSecOwner,
						 OUT RvBool						  *pbProcessMsg); 

/******************************************************************************
 * TransportCallbackTcpMsgReceivedExtEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received on TCP.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgReceived	- Handle to the received message. 
 *		    pRcvdMsgDetails - Pointer to received message details struct
 *          pRecvFromAddr   - Pointer to Received From Address in SIPTK format
 *          pSecOwner       - Pointer to Security Object, owning the Local
 *                            Address or Connection, on which the message was
 *                            received. Can be NULL.
 * Output:  pbProcessMsg    - The application can use this parameter for
 *							  asking the SIP Stack to ignore the incoming
 *							  message by setting this out parameter to
 *							  RV_FALSE (otherwise its value should be set
 *							  to RV_TRUE).
 *****************************************************************************/
void RVCALLCONV TransportCallbackTcpMsgReceivedExtEv(
                         IN    RvSipMsgHandle                hMsgReceived,
						 IN    RvSipTransportRcvdMsgDetails *pRcvdMsgDetails,
                         IN    SipTransportAddress          *pRecvFromAddr,
                         IN    void                         *pSecOwner,
						 OUT   RvBool						*pbProcessMsg); 

/***************************************************************************
 * TransportCallbackMsgThreadErr
 * ------------------------------------------------------------------------
 * General:  Notify the application on an abnormal thread termination.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - A pointer to the transport manager object.
 *           error         - RV_ERROR_NUM_OF_THREADS_DECREASED - There is one less thread
 *                          in the system since one thread terminated abnormally.
 ***************************************************************************/
void RVCALLCONV TransportCallbackMsgThreadErr (
                     IN TransportMgr             *pTransportMgr,
                     IN RvStatus                  error);


/***************************************************************************
 * TransportCallbackBadSyntaxStartLineMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new message is received, with bad-syntax start-line.
 *           The application can fix the message in this callback.
 *           The application should use the eAction parameter to decide how the
 *           transport layer will handle this message: discard it, continue
 *           with message processing, or send '400 Bad Syntax' response (in case
 *           of request message).
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - A pointer to the transport manager object.
 *          hMsgReveived     - The received message, with bad-syntax info.
 * Output:  peAction         - User decision of stack way of handling this message.
 ***************************************************************************/
RvStatus RVCALLCONV TransportCallbackBadSyntaxStartLineMsgEv(
                      IN    TransportMgr             *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction);


/***************************************************************************
 * TransportCallbackBadSyntaxMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new message was received, with bad-syntax.
 *           The application can fix the message in this callback.
 *           The application should use the eAction parameter to decide how the
 *           transport layer will handle this message: discard it, continue
 *           with message processing, or send '400 Bad Syntax' response (in case
 *           of request message).
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - A pointer to the transport manager object.
 *          hMsgReveived     - The received message, with bad-syntax info.
 * Output:  peAction         - User decision of stack way of handling this message.
 ***************************************************************************/
RvStatus RVCALLCONV TransportCallbackBadSyntaxMsgEv(
                      IN    TransportMgr             *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction);

/************************************************************************************
 * TransportCallbackMsgRcvdEv
 * ----------------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager object.
 *          hReceivedMsg  - Handle to the received message.
 *          pConn         - Pointer to the relevant connection.
 *          pLocalAddr   -  The local address that the message was received on.
 *          pRecvFromAddr - The address from which the message was received.
 *          eBSAction    -  Bad-syntax action, that was given by application in
 *                          bad-syntax callbacks. continue processing, reject, discard.
 *          pSigCompMsgInfo - SigComp info related to the received message.
 *                            The information contains indication if the UDVM
 *                            waits for a compartment ID for the given unique
 *                            ID that was related to message by the UDVM.
 ***********************************************************************************/
void RVCALLCONV TransportCallbackMsgRcvdEv (
                         IN    TransportMgr*                  pTransportMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    RvSipTransportConnectionHandle hConn,
                         IN    RvSipTransportLocalAddrHandle    hLocalAddr,
                         IN    SipTransportAddress           *pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo    *pSigCompMsgInfo);

/******************************************************************************
 * TransportCallbackConnectionMsgRcvdEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received on
 *          connection (e.g. TCP, SCTP).
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - the Transport Manager object.
 *          hReceivedMsg    - the Message object
 *          pConn           - Pointer to the relevant connection.
 *          hLocalAddr      - the Local Address object, used by the Connection.
 *          pRecvFromAddr   - the remote connection address.
 *          eBSAction       - Bad-syntax action, that was given by application
 *                            in bad-syntax callbacks(continue|reject|discard).
 *          pSigCompMsgInfo - SigComp info related to the received message.
 *                            The information contains indication if the UDVM
 *                            waits for a compartment ID for the given unique
 *                            ID that was related to message by the UDVM.
 *****************************************************************************/
void RVCALLCONV TransportCallbackConnectionMsgRcvdEv (
                         IN    TransportMgr*                  pTransportMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    TransportConnection*           pConn,
                         IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                         IN    SipTransportAddress*           pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo*    pSigCompMsgInfo);

#if (RV_TLS_TYPE != RV_TLS_NONE)
/************************************************************************************
 * TransportCallbackTLSAppPostConnectionAssertionEv
 * ----------------------------------------------------------------------------------
 * General: notifies the application on tls state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          strSipAddress - the address to which the assertion was made.
 *          hMsg          - the entire msg that the assertion was made against.
 *                          This parameter will be NULL if assertion was made
 *                          against an outgoing message or predefined string.
 * Output:  pbAssertion - is the connection asserted.
 ***********************************************************************************/
void RVCALLCONV TransportCallbackTLSAppPostConnectionAssertionEv(IN  TransportConnection *pConn,
                                                       IN RvChar              *strSipAddress,
                                                       IN RvSipMsgHandle        hMsg,
                                                       OUT RvBool             *pbAssertion);

/************************************************************************************
 * TransportCallbackTLSStateChange
 * ----------------------------------------------------------------------------------
 * General: notifies the application on tls state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          eState - Tls State to assume
 *          eReason - reason for state change
 *          bSendToQueue - send event though queue
 *          bChangeInternalState - whether to change internal state
 * Output:  RV_OK on success, error code - otherwise.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportCallbackTLSStateChange(
                    IN TransportConnection*             pConn,
                    IN RvSipTransportConnectionTlsState eState,
                    IN RvSipTransportConnectionStateChangedReason   eReason,
                    IN RvBool                           bSendToQueue,
                    IN RvBool                           bChangeInternalState);

/************************************************************************************
* TransportCallbackConnectionTlsStatus
 * ----------------------------------------------------------------------------------
 * General: notifies the application on security events
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    hConn           -   The connection handle
 *           eStatus         -   The connection security status
 * Output    pbContinue      -   application instructions.
  ***********************************************************************************/
void RVCALLCONV TransportCallbackConnectionTlsStatus(IN TransportConnection*               pConn,
                                                    IN  RvSipTransportConnectionTlsStatus  eStatus,
                                                    OUT RvBool*                            pbContinue);

/************************************************************************************
 * TransportCallbackTLSSeqStarted
 * ----------------------------------------------------------------------------------
 * General: notifies the application on tls seq start.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
void RVCALLCONV TransportCallbackTLSSeqStarted(IN TransportConnection* pConn);

#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/***************************************************************************
 * TransportCallbackConnCreated
 * ------------------------------------------------------------------------
 * General: Notifies an application about COnnection object creation for 
 *          incoming TCP connection.
 *          The callback is called immediately after accept.
 *          Application can order Stack to close the connection by means of
 *          parameter 'bDrop'.
 *          In this case the connection will be closed immediately after
 *          return from callback. It resources will be freed.
 *          No data will be received or sent on it.
 *          If application didn't register to this callback,
 *          the connection will not be closed, and will be used for data
 *          sending and reception.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - A pointer to the transport manager object.
 *          pConn            - pointer to the created connection.
 * Output:  pbDrop           - If set to RV_TRUE by application, the connection
 *                             will be dropped immediately after return from
 *                             callback.
 *                             Otherwise the connection will be not dropped and
 *                             will be used for data receiption and sending. 
***************************************************************************/
void RVCALLCONV TransportCallbackConnCreated(
                                    IN  TransportMgr            *pTransportMgr,
                                    IN  TransportConnection     *pConn,
                                    OUT RvBool                  *pbDrop);

/***************************************************************************
 * TransportCallbackBufferReceived
 * ------------------------------------------------------------------------
 * General: Exposes to an application row data buffer, containing exactly
 *          one SIP message, received on TCP/UDP layer.
 *          Application can dump the data by means of this callback.
 *          Also application can order Stack to discard the buffer, and not
 *          to parse it, by means of pbDiscardBuffer parameter.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - A pointer to the transport manager object.
 *          pLocalAddr      - pointer to the local address object,corresponding
 *                            to address, on which the message was received.
 *          pSenderAddr     - A pointer to the SIP Transport Address structure,
 *                            which contains details of the address,
 *                            from which the message was sent.
 *          pConn           - A pointer to the connection object, on which
 *                            the buffer was received. NULL in case of UDP.
 *          buffer          - pointer to the buffer,which contains the message
 *          buffLen         - length of the message in the buffer (in bytes)
 * Output:  bDiscardBuffer  - if set to RV_TRUE, the buffer will be not
 *                            processed, the resources will be freed. 
 ***************************************************************************/
void RVCALLCONV TransportCallbackBufferReceived(
                                IN  TransportMgr            *pTransportMgr,
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  SipTransportAddress     *pSenderAddr,
                                IN  TransportConnection     *pConn,
                                IN  RvChar                  *buffer,
                                IN  RvUint32                buffLen,
                                OUT RvBool                  *pbDiscardBuffer);

/***************************************************************************
 * TransportCallbackBufferToSend
 * ------------------------------------------------------------------------
 * General: Exposes to an application row data buffer, containing exactly
 *          one SIP message, that is going to be sent on TCP/UDP layer.
 *          Application can dump the data by means of this callback.
 *          Also the application can decide whether the transport layer
 *          should not transmit the message to its destination.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the Transport Manager object.
 *          pLocalAddr       - pointer to the local address object,corresponding
 *                             to address, from which the message is sent.
 *          pDestinationAddr - pointer to the Transport Address structure,
 *                             which contains details of the address,
 *                             to which the message is going to be sent.
 *          pConn            - A pointer to the connection object, on which
 *                             the buffer is going to be sent. NULL for UDP.
 *          buffer           - pointer to the buffer,which contains the message
 *          buffLen          - length of the message in the buffer (in bytes)
 * Output:  bDiscardBuffer   - if set to RV_TRUE, the buffer will be not sent,
 *                             the resources will be freed. 
 ***************************************************************************/
void RVCALLCONV TransportCallbackBufferToSend(
                                IN  TransportMgr            *pTransportMgr,
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  SipTransportAddress     *pDestinationAddr,
                                IN  TransportConnection     *pConn,
                                IN  RvChar                  *buffer,
                                IN  RvUint32                buffLen,
                                OUT RvBool                  *pbDiscardBuffer);

/******************************************************************************
 * TransportCallbackConnParserResult
 * ----------------------------------------------------------------------------
 * General: Notifies an application about result of parsing of incoming
 *          over TCP/SCTP connection message. 
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - A pointer to the transport manager object.
 *          hMsg          - Handle to the message, which was parsed.
 *          pConn         - pointer to the connection, on which the parsed
 *                          message arrived.
 *          hAppConn      - Handle stored by the Application in the connection.
 *          bLegalSyntax  - If RV_FALSE, parser encountered into bad syntax.
 *                          Otherwise - RV_FALSE.
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV TransportCallbackConnParserResult(
                            IN TransportMgr*                     pTransportMgr,
                            IN RvSipMsgHandle                    hMsg,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvBool                            bLegalSyntax);

/******************************************************************************
 * TransportCallbackIncomingConnStateChange
 * ----------------------------------------------------------------------------
 * General: notifies the application on incoming connection state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          eState - state to notify
 *          eReason - reason for state change
 * Output:
 *****************************************************************************/
void RVCALLCONV TransportCallbackIncomingConnStateChange(
                        IN TransportConnection*                       pConn,
                        IN RvSipTransportConnectionState              eState,
                        IN RvSipTransportConnectionStateChangedReason eReason);

/***************************************************************************
 * TransportCallbackConnDataReceived
 * ------------------------------------------------------------------------
 * General: Notifies an application that data was read form the socket
 *          of the incoming connection.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - A pointer to the transport manager object.
 *          pConn         - pointer to the created connection.
 *          buff          - A buffer, containing read data.
 *          buffSize      - Size of the read data in bytes.
 * Output:  pbDiscard     - If is set to RV_TRUE by the Application,
 *                          the received data will be discarded.
 ***************************************************************************/
void RVCALLCONV TransportCallbackConnDataReceived(
                                  IN  TransportMgr            *pTransportMgr,
                                  IN  TransportConnection     *pConn,
                                  IN  RvChar                  *buff,
                                  IN  RvInt32                 buffSize,
                                  OUT RvBool                  *pbDiscard);

/***************************************************************************
* TransportCallbackConnServerReuse
* ------------------------------------------------------------------------
* General: This callback function informs application that a server TLS connection
*          should be reuse, and it has to be authorized first.
*          In this callback application should authenticate the connection, as 
*          described in draft-ietf-sip-connect-reuse-03. 
*          If the connection was authorized,Application should call to 
*          RvSipTransportConnectionUseTlsConnByAlias() function.
* Return Value: none.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr - A pointer to the transport manager object.
*          pConn         - pointer to the connection.
*          hAppConn      - handle stored by the application in the connection.
* Output:  none.
***************************************************************************/
void RVCALLCONV TransportCallbackConnServerReuse(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn);

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransportCallbackMsgSendFailureEv
 * ----------------------------------------------------------------------------
 * General: Is called when a protected message sending was failed due to error,
 *          returned by the system call, which sends buffers.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the Transport Manager
 *          hMsgPage   - Handle to the page,where the encoded message is stored
 *          pLocalAddr - Pointer to the local address, if the message was being
 *                       sent over UDP
 *          pConn      - Pointer to the connection, if the message was being
 *                       sent over TCP / TLS / SCTP
 *          pSecOwner  - Pointer to the Security Object, owning the local
 *                       address or connection, on which the message was
 *                       being sent.
 *****************************************************************************/
void RVCALLCONV TransportCallbackMsgSendFailureEv(
                                        IN TransportMgr*          pTransportMgr,
                                        IN HPAGE                  hMsgPage,
                                        IN TransportMgrLocalAddr* pLocalAddr,
                                        IN TransportConnection*   pConn,
                                        IN void*                  pSecOwner);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef __cplusplus
}
#endif

#endif /*TRANSPORT_CALLBACKS_H*/

