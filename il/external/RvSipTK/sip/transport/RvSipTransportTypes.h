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
 *                              <RvSipTransportTypes.h>
 *
 * The transport layer of the RADVISION SIP Toolkit allows you to control sending
 * and receiving of messages over the network.
 * This file defines the prototypes for message-received and message-to-send
 * callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2002
 *********************************************************************************/


#ifndef RVSIP_TRANSPORTTYPES_H
#define RVSIP_TRANSPORTTYPES_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RvSipMsg.h"
#include "RvSipMsgTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/*Sip default port for UDP, TCP and SCTP*/
#define RVSIP_TRANSPORT_DEFAULT_PORT 5060

/*Sip default port for TLS (secure port)*/
#define RVSIP_TRANSPORT_DEFAULT_TLS_PORT 5061

/*Length of the string, representing IP, used by Stack*/
#define RVSIP_TRANSPORT_LEN_STRING_IP 51 /* sizeof("[ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255]%dd")+'\0' */

/*Number of bytes in IPv6 address*/
#define RVSIP_TRANSPORT_LEN_BYTES_IPV6 16

/*
 * RvSipTransportMgrHandle
 * ---------------------------------------------------------------------------
 * The declaration of a handle to a Transport Manager object (TransportMgr).
 */
RV_DECLARE_HANDLE(RvSipTransportMgrHandle);

/*
 * RvSipAppTransportMgrHandle
 * ---------------------------------------------------------------------------
 * The declaration of an application handle to a TransportMgr.
 * This handle can be used as a context that the application can hold in the
 * TransportMgr.
*/
RV_DECLARE_HANDLE(RvSipAppTransportMgrHandle);

/*
 * RvSipTransportConnectionHandle
 * ---------------------------------------------------------------------------
 * The declaration of handle to a connection object (connection). A connection is used
 * for TCP/TLS communication.
 */
RV_DECLARE_HANDLE(RvSipTransportConnectionHandle);

/*
 * RvSipTransportConnectionOwnerHandle
 * ---------------------------------------------------------------------------
 * The declaration of handle to the owner of a connection. A connection
 * can have several owners. All owners are notified of connection events.
 */
RV_DECLARE_HANDLE(RvSipTransportConnectionOwnerHandle);


/*
 * RvSipTransportTlsEngineHandle
 * ---------------------------------------------------------------------------
 * The declaration of handle to a Transport TLS Engine. The Stack Toolkit can hold
 * several TLS engines, each with different attributes. Different TLS engines
 * can be used to set different sets of TLS attributes to different connections.
 */
RV_DECLARE_HANDLE(RvSipTransportTlsEngineHandle);

/*
 * RvSipTransportConnectionAppHandle
 * ---------------------------------------------------------------------------
 * An application handle that the application can set and get from the connection.
 * A connection can hold only one application handle.
 */
RV_DECLARE_HANDLE(RvSipTransportConnectionAppHandle);

/*
 * RvSipTransportLocalAddrHandle
 * ---------------------------------------------------------------------------
 * The handle to the Local Address object, which is used by Stack for receiving
 * or sending messages, and for listening for new connections.
 */
RV_DECLARE_HANDLE(RvSipTransportLocalAddrHandle);

/*
 * RvSipTransportLocalAddrAppHandle
 * ---------------------------------------------------------------------------
 * An application handle to the Local Address object that the application
 * can set and get from the connection. A local address can hold only one
 * application handle.
 */
RV_DECLARE_HANDLE(RvSipTransportLocalAddrAppHandle);

/*
 * RvSipTransportTlsCertificate
 * ---------------------------------------------------------------------------
 * A pointer to a certificate retrieved from a connection, or used in the compare
 * certificate callback function.
 */
RV_DECLARE_HANDLE(RvSipTransportTlsCertificate);

/*
 * RvSipTransportPersistencyLevel
 * ---------------------------------------------------------------------------
 * Defines the persistency level in the system. Undefined means that there is no
 * persistency of connections. One connection is used for a request and its response.
 * After the response is sent, the connection is closed.
 *
 * RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED--When the SIP Stack is configured to
 * use an undefined persistency level, the following rules apply:
 * --SIP Stack objects do not look for suitable connections in the hash before sending
 * a message, and therefore always open new connections for sending requests.
 * (Responses are still sent on the connection on which the request was received).
 * --Newly created connections are not inserted into the hash.
 *
 * RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC-- When the SIP Stack is configured to use
 * the transaction persistency level, the following rules apply:
 * --A transaction that wishes to send a request will first try to locate a suitable
 * connection in the hash.
 * --If there is a suitable open connection, the transaction will use it. If there
 * is not, the transaction will open a new connection and insert it into the connections
 * hash. In both cases, the transaction will attach itself to the connection and become
 * the connection owner.
 * --The transaction will detach from the connection only before the transaction terminates.
 *
 RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER     -- A Transaction User (TU) is an object
 * that uses transactions for sending requests. A call-leg, subscription and a register-client
 * are all Transaction Users. When the SIP Stack is configured to use the TU persistency level,
 * the following rules apply:
 * --SIP Stack transactions behave as defined in the transaction persistency level.
 * --A TU tries to use the same connection for all outgoing requests (sent by different
 * client transactions).
 */
typedef enum
{
    RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED = -1,
    RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC,
    RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER
} RvSipTransportPersistencyLevel;


/* RvSipTransportAddressType
 * ---------------------------
 * Determines whether the address is an IPv4 or an IPv6 address.
 */
typedef enum
{
    RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED = -1,
    RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
    RVSIP_TRANSPORT_ADDRESS_TYPE_IP6
} RvSipTransportAddressType;


/*  RvSipTransportAddr
 * -------------------
 *  A structure containing parameters of the address that can be used
 *  by the Stack for network communication.
 *
 *  eTransportType  - The type of the transport protocol that is used with the address.
 *  eAddrType       - The type of IP address (IPv4 or IPv6).
 *  port            - The port.
 *  strIP           - A NULL terminated string, representing the IP address.
 *                    (If the address is an IPv6 address, the string should be in
 *                    the following format: xxxx:xxxx:..:xxxx)
 *  if_name         - A NULL terminated string, representing the local interface name. For remote addresses is meaningless.
 *  vdevblock          - local hardware port or else. For remote addresses is meaningless.
 */
typedef struct
{
    RvSipTransport              eTransportType;
    RvSipTransportAddressType   eAddrType;
    RvUint16                    port;
    RvChar                      strIP[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvInt                       Ipv6Scope;
  // SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
  RvChar if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
  RvUint16 vdevblock;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
  // SPIRENT_END
} RvSipTransportAddr;

/*  RvSipTransportAddrOptions
* -------------------
*  A structure containing parameters of the address.
*  Currently the structure serves remote addresses.
*
*  eCompression  - The type of data compression sent to the remote address.
*/
typedef struct
{
    RvSipCompType  eCompression;
} RvSipTransportAddrOptions;

/* RvSipTransportBsAction
 * ---------------------------
 * Determines how to proceed handling a received
 * message with bad-syntax.
 * This definition is for use as an output parameter in the RvSipTransportBadSyntaxMsgEv()
 * and RvSipTransportBadSyntaxStartLineMsgEv() callback functions.
 * RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG - Does nothing. Discards the bad-syntax message.
 *                                        (Same behavior as in Stack version 2.2.)
 * RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG - Sends a 400 response for the bad-syntax message.
 *                                        (Relevant only for request messages.)
 * RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS - Stack continues message processing,
 *                                        for as long as possible. The Stack will process the
 *                                        bad-syntax message until it finds that an
 *                                        essential header has a syntax error and then sends
 *                                        a 400 response. If all essential headers are correct,
 *                                        the message will be processed successfully by the Stack.
 */
typedef enum
{
    RVSIP_TRANSPORT_BS_ACTION_UNDEFINED = -1,
    RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG,
    RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG,
    RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS
}RvSipTransportBsAction;


/*  RvSipTransportConnectionCfg
 * --------------------------------
 *  A structure containing the configuration needed to initialize
 *  a new connection. You need to supply this structure when calling the
 *  RvSipTransportConnectionInit() function.
 *
 *  eTransportType - The transport type: TCP/TLS.
 *  strLocalIp - The local IP as a string. IPv6 should be in []%sid format. If NULL
 *               is supplied a default, the local IP is chosen.
 *  localPort  - Local port. Zero is replaced with the transport default port
 *               (5060 for TCP and 5061 for TLS).
 *               The local port is ignored if the strLocalIp is not specified.
 *
 *  strDestIp - The destination IP as a string. IPv6 should be supplied in a [] format.
 *  strDestPort - The destination port. Zero is replaced with the transport default port
 *               (5060 for TCP and 5061 for TLS).
 *  bEnablePersistency- Specifies whether or not to insert the connection to the hash.
 *
 *  strHostName - If the connection type is TLS, this string will be used for post connection assertion
 *  sctpMsgSendParams - Parameters that are used by default while calling a
 *                      send() system call for an SCTP connection.
 */
typedef struct
{
     RvSipTransport     eTransportType;
     RvChar*            strLocalIp;
     RvUint16           localPort;
#if defined(UPDATED_BY_SPIRENT)
     RvChar*            if_name;
     RvUint16           vdevblock;
#endif
     RvChar*            strDestIp;
     RvUint16           destPort;
     RvChar*            strHostName;
#if (RV_NET_TYPE & RV_NET_SCTP)
     RvSipTransportSctpMsgSendingParams sctpMsgSendParams;
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP)*/
} RvSipTransportConnectionCfg;


/*  RvSipTransportConnectionState
 * ------------------------------
 * Represents the state of a connection. The state changes are reported using the
 * RvSipTransportConnectionStateChangedEv() callback function with a state change reason.
 */
typedef enum
{
    RVSIP_TRANSPORT_CONN_STATE_UNDEFINED = -1,
    RVSIP_TRANSPORT_CONN_STATE_IDLE,
    RVSIP_TRANSPORT_CONN_STATE_READY,
    RVSIP_TRANSPORT_CONN_STATE_CONNECTING,
    RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED,
    RVSIP_TRANSPORT_CONN_STATE_CLOSING,
    RVSIP_TRANSPORT_CONN_STATE_CLOSED,
    RVSIP_TRANSPORT_CONN_STATE_TERMINATED,
    RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED
}RvSipTransportConnectionState;


/* RvSipTransportConnectionTlsState
 * --------------------------------
 * The TLS state of the connection.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED - A TLS sequence was not initiated on the connection.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED - The handshake procedure on the connection was completed.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY - The connection is TCP-connected and is ready to start the TLS handshake.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED - The connection is the performing a handshake.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED - The TLS handshake failed. Data cannot be transmitted on the connection.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED - The connection received or sent a close request but the
 * connection did not close yet
 * RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED - Data can be sent on the connection.
 * RVSIP_TRANSPORT_CONN_TLS_STATE_TERMINATED - The connection is terminated. After this, the connection may not be accessed again.
 */
typedef enum
{
    RVSIP_TRANSPORT_CONN_TLS_STATE_UNDEFINED = -1,
    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_COMPLETED,
    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_READY,
    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_STARTED,
    RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED,
    RVSIP_TRANSPORT_CONN_TLS_STATE_CLOSE_SEQUENSE_STARTED,
    RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED,
    RVSIP_TRANSPORT_CONN_TLS_STATE_TERMINATED
}RvSipTransportConnectionTlsState;

/*  RvSipTransportConnectionStatus
 * -------------------------------
 * Connection events that do not effect the connection state.
 * UNDEFINED - Unspecified status.
 * ERROR - An error occurred. The connection will be closed.
 * MSG_SENT - For internal use only. The application will not be notified of this status.
 * MSG_NOT_SENT - A message that was supposed to be sent by this connection was not sent and the connection was closed.
 *                Only the owner of this message is notified about this status.
 * SCTP_PEER_ADDR_CHANGED - The status of the address of the remote endpoint
 *                          was changed (e.g., failure or return to service).
 * SCTP_REMOTE_ERROR - An error message of Operation Error was received from
 *                     the remote peer.
 */
typedef enum
{
    RVSIP_TRANSPORT_CONN_STATUS_UNDEFINED = 1,
    RVSIP_TRANSPORT_CONN_STATUS_ERROR,
    RVSIP_TRANSPORT_CONN_STATUS_MSG_SENT,
    RVSIP_TRANSPORT_CONN_STATUS_MSG_NOT_SENT,
    RVSIP_TRANSPORT_CONN_STATUS_SCTP_PEER_ADDR_CHANGED,
    RVSIP_TRANSPORT_CONN_STATUS_SCTP_REMOTE_ERROR
}RvSipTransportConnectionStatus;

/*  RvSipTransportConnectionTlsStatus
 * -----------------------------------
 * Represents the status of a connection. The SIP Stack reports the status to the application using
 * the RvSipTransportConnectionTlsStatusEv() callback.
 * UNDEFINED - No security status.
 * HANDSHAKE_PROGRESS - Data was sent or received during a handshake process.
*/
typedef enum
{
    RVSIP_TRANSPORT_CONN_TLS_STATUS_UNDEFINED = 1,
    RVSIP_TRANSPORT_CONN_TLS_STATUS_HANDSHAKE_PROGRESS
}RvSipTransportConnectionTlsStatus;

/*  RvSipTransportConnectionStateChangedReason
 *  ------------------------------------------
 *  The reason supplied with the state changes function of a connection.
 *  The reason is used only if it supplies more information about the new
 *  state. Otherwise the reason is undefined.
 */
typedef enum
{
    RVSIP_TRANSPORT_CONN_REASON_UNDEFINED = -1,
    RVSIP_TRANSPORT_CONN_REASON_ERROR,
    RVSIP_TRANSPORT_CONN_REASON_CLIENT_CONNECTED,
    RVSIP_TRANSPORT_CONN_REASON_SERVER_CONNECTED,
    RVSIP_TRANSPORT_CONN_REASON_TLS_POST_CONNECTION_ASSERTION_FAILED,
    RVSIP_TRANSPORT_CONN_REASON_DISCONNECTED
}RvSipTransportConnectionStateChangedReason;


/* RvSipTransportPrivateKeyType
 * ---------------------------
 * Determines the key type to use in a TLS engine.
 */
typedef enum
{
    RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_UNDEFINED = -1,
    RVSIP_TRANSPORT_PRIVATE_KEY_TYPE_RSA_KEY
}RvSipTransportPrivateKeyType;

/* RvSipTransportTlsMethod
 * ---------------------------
 * Determines the version of TLS to use in an engine.
 */
typedef enum
{
    RVSIP_TRANSPORT_TLS_METHOD_UNDEFINED = -1,
    RVSIP_TRANSPORT_TLS_METHOD_SSL_V2,
    RVSIP_TRANSPORT_TLS_METHOD_SSL_V3,
    RVSIP_TRANSPORT_TLS_METHOD_SSL_V23 = RVSIP_TRANSPORT_TLS_METHOD_SSL_V3,
    RVSIP_TRANSPORT_TLS_METHOD_TLS_V1
}RvSipTransportTlsMethod;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
typedef struct
{
    unsigned char TOSByte;
} RV_TOSDiffserv;
#endif
/* SPIRENT_END */

/* RvSipTransportTlsHandshakeSide
 * ---------------------------
 * Determines the side of TLS handshake to assume.
 * Default means the TCP client will act as a TLS client, and the TCP server will act as a TLS server.
 * Default is the recommended way of working.
 */
typedef enum
{
    RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_UNDEFINED = -1,
    RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_DEFAULT,
    RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_CLIENT,
    RVSIP_TRANSPORT_TLS_HANDSHAKE_SIDE_SERVER
}RvSipTransportTlsHandshakeSide;

/*  RvSipTransportMsgAddrCfg
 * --------------------------------
 *  A structure containing the configuration needed to inject a message into the SIP Stack.
 *  You need to supply this structure when calling the RvSipTransportInjectMsg() function.
 *  When injecting a message into the SIP Stack, the SIP Stack behaves as if the
 *  message was received from the network. Therefore, you need to supply the addresses
 *  (local and remote) that you want to use for this message with the injected message.
 *  These addresses are included in the RvSipTransportMsgAddrCfg structure.Using this
 *  structure you can simulate that a message was received from a certain destination
 *  and on a specific local address. The local address will also be used if a response
 *  is sent to an incoming injected request.
 *  eTransportType - The transport type (UDP/TCP/TLS). If UUNDEFINED, use the default UDP.
 *  strLocalIp - The local IP as a string. IPv6 should be in []%sid format. If NULL
 *               is supplied a default, the local IP is chosen.
 *  localPort  - The local port. Zero is replaced with the transport default port
 *               (5060 for UDP/TCP and 5061 for TLS).
 *  strDestIp  - The destination IP as a string. IPv6 should be supplied in a [] format.
 *               If NULL, a destination address will no be set, and a response will be
 *                sent according to the address in the top Via header.
 *  strDestPort - The destination port. Zero is replaced with the default transport port
 *               (5060 for UDP/TCP and 5061 for TLS).
 */
typedef struct
{
     IN  RvSipTransport     eTransportType;
     IN  RvChar*            strLocalIp;
     IN  RvUint16           localPort;
#if defined(UPDATED_BY_SPIRENT)
     RvChar*                if_name;
     RvUint16               vdevblock;
#endif
     IN  RvChar*            strDestIp;
     IN  RvUint16           destPort;
} RvSipTransportMsgAddrCfg;


/*  RvSipTransportOutboundProxyCfg
 * --------------------------------
 *  A structure containing the configurations needed to set
 *  the outbound proxy details, These will be used in case of outgoing
 *  messages toward a proxy.
 *
 * strIpAddress
 * -----------------------
 * Indicates the IP address of an outbound proxy that the SIP Stack uses.
 * Default Value: 0. No outbound proxy.
 *
 * port
 * -----------------
 * Indicates the port of the outbound proxy that the SIP Stack uses.
 * Default Value: 5060
 *
 * strHostName
 * ---------------------
 * The host name of an outbound proxy that the SIP Stack uses. For each outgoing request
 * the DNS will be queried for this host IP address.
 * Note: If you set the outboundProxyIPAddress parameter, the outboundProxyHostName
 * parameter will be ignored.
 * Default value: NULL
 *
 * eTransport
 * ----------------------
 * Indicates the transport of the outbound proxy that the SIP Stack uses.
 * Default Value: RVSIP_TRANSPORT_UNDEFINED
 *
 * eCompression
 * --------------------------
 * Indicates the compression type of the SIP Stack messages that are sent to
 * the outbound proxy.
 * Default Value: RVSIP_COMP_UNDEFINED
 *
 * strSigcompId
 * -----------------------
 * The sigcomp-id string representing the outbound proxy, according to RFC 5049.
 * Default value: NULL
 */
typedef struct
{
    RvChar         *strIpAddress;
    RvInt32         port;
    RvChar         *strHostName;
    RvSipTransport  eTransport;
#ifdef RV_SIGCOMP_ON
    RvSipCompType   eCompression;
	RvChar         *strSigcompId;
#endif
} RvSipTransportOutboundProxyCfg;

/*  RvSipTransportRcvdMsgDetails
 * --------------------------------
 * A structure containing all the data which is related to a received message.
 *
 * hConnInfo
 * -----------------------
 * The message connection handle or NULL when UDP was used.
 *
 * hLocalAddr
 * -----------------
 * The local address on which the message was received.
 *
 * pRecvFromAddr
 * ---------------------
 * The address from which the message was received.
 *
 * eCompression
 * --------------------------
 * Indicates the compression type of the message
 *
 * eBSAction
 * -------------
 * Bad-syntax action, which was given by the application in bad-syntax callbacks.
 * May be continue processing, reject, or discard.
 *
 */
typedef struct
{
	RvSipTransportConnectionHandle  hConnInfo;
	RvSipTransportLocalAddrHandle   hLocalAddr;
	RvSipTransportAddr			    recvFromAddr;
	RvSipCompType					eCompression;
	RvSipTransportBsAction			eBSAction;
} RvSipTransportRcvdMsgDetails;

/******************************************************************************
 * RvSipTransportConnectionStateChangedEv
 * ----------------------------------------------------------------------------
 * General: The connection is a stateful object that can assume different
 *          states according to the Connection state machine.
 *          Through this function, you receive notifications
 *          of connection state changes and the associated state change reason.
 *          In a regular connection life cycle, the reason for the state is set
 *          to RVSIP_TRANSPORT_CONN_REASON_UNDEFINED. When the connection
 *          is closed because of an error, the reason is set to
 *          RVSIP_TRANSPORT_CONN_REASON_ERROR.
 *          Note: You do not have to register to this callback
 *          if you do not want to get connection states.
 * Return Value: Ignored.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   -   Connection handle.
 *          hObject -   For a server connection: application handle.
 *                      for a client connection: handle to the connection owner.
 *          eState  -   The connection state.
 *          eReason -   The reason for the new state, or undefined if there is
 *                      no special reason.
 *****************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipTransportConnectionStateChangedEv) (
                        IN RvSipTransportConnectionHandle              hConn,
                        IN RvSipTransportConnectionOwnerHandle         hObject,
                        IN RvSipTransportConnectionState               eState,
                        IN RvSipTransportConnectionStateChangedReason  eReason);

/* RvSipTransportSctpPeerAddrChangeOp
 * --------------------------------
 * Operations performed on the multihoming SCTP address by the peer.
 */
typedef enum
{
    RVSIP_TRANSPORT_SCTP_PEERADDRCHANGEOP_ADDED = 0,
    RVSIP_TRANSPORT_SCTP_PEERADDRCHANGEOP_REMOVED
}RvSipTransportSctpPeerAddrChangeOp;

/*  RvSipTransportSctpPeerAddrChangeInfo
 * ----------------------------------
 *  A structure containing details of the multihoming address and type of
 *  operation performed on it by the peer.
 *  The structure is supplied through the RvSipTransportConnectionStatusEv()
 *  callback on RVSIP_TRANSPORT_CONN_STATUS_SCTP_PEER_ADDR_CHANGED notification.
 *
 *  eOperation - The operation performed by the peer on the address (ADDED, REMOVED).
 *  pTransportAddress - The details of the address, whose status was changed.
 */
typedef struct
{
    RvSipTransportSctpPeerAddrChangeOp eOperation;
    RvSipTransportAddr address;
} RvSipTransportSctpPeerAddrChangeInfo;

/***************************************************************************
 * RvSipTransportConnectionStatusEv
 * ------------------------------------------------------------------------
 * General: The connection notifies about events that do not effect the
 *   connection state using the connection status callback. If, for example,
 *   there was an error in the connection, the connection will notify the
 *   application with RVSIP_TRANSPORT_CONN_STATUS_ERROR. The connection will then
 *   disconnect with the RVSIP_TRANSPORT_CONN_REASON_ERROR reason.
 *   Note: You do not have to register to this callback if you do not want to get
 *   connection statuses.
 * Return Value: Ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hConn   -   The connection handle.
 *           hOwner  -   The handle to the connection owner.
 *           eStatus -   The connection status.
 *           pInfo   -   Points to the RvSipTransportSctpPeerAddrChangeInfo
 *                       structure, containing information about changed
 *                       multihoming address in case of SCTP_PEER_ADDR_CHANGED
 *                       notification.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipTransportConnectionStatusEv) (
                        IN RvSipTransportConnectionHandle      hConn,
                        IN RvSipTransportConnectionOwnerHandle hOwner,
                        IN RvSipTransportConnectionStatus      eStatus,
                        IN void*                               pInfo);

/***************************************************************************
 * RvSipTransportVerifyCertificateEv
 * ------------------------------------------------------------------------
 * General:  This function notifies the application whenever a certificate
 *           needs to be processed. If you wish to leave the Stack decision
 *           regarding a certificate, return the prevError parameter as the
 *           return value.
 * Return Value: RvInt32
 * ------------------------------------------------------------------------
 * Arguments: prevError - The error previously detected by the Stack. A positive
 *            number indicates that the certificate is OK.
 *            Certificate - The certificate for which the callback is called. This
 *            certificate can be encoded using RvSipTransportTlsEncodeCert().
 * Input:
 ***************************************************************************/
typedef RvInt32   (* RvSipTransportVerifyCertificateEv)(
                                                        IN    RvInt32 prevError,
                                                        IN    RvSipTransportTlsCertificate  certificate);
/*  RvSipTransportTlsEngineCfg
 * --------------------------------
 *  A structure containing the configuration needed to initialize
 *  a new TLS engine. You need to supply this structure when calling the
 *  RvSipTransportTlsEngineConstruct() function.
 *
 *  eTlsMethod      - The SSL/TLS version.
 *  strPrivateKey   - The private key for the engine.
 *  ePrivateKeyType - The private key type.
 *  privateKeyLen   - The size of key.
 *  strCert         - The certificate issued for the engine.
 *  certLen         - The size of the certificate.
 *  certDepth       - The maximum length of a chain of certificates before it is considered invalid.
 */

typedef struct
{
    RvSipTransportTlsMethod                       eTlsMethod;
    RvChar*                                       strPrivateKey;
    RvSipTransportPrivateKeyType                  ePrivateKeyType;
    RvInt32                                       privateKeyLen;
    RvChar*                                       strCert;
    RvInt32                                       certLen;
    RvInt32                                       certDepth;
} RvSipTransportTlsEngineCfg;

/***************************************************************************
 * RvSipTransportConnectionTlsSequenceStartedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a connection has reached the
 *           state where a TLS sequence has started.
 *           This is where the application should exchange handles with
 *           the TLS connection. If an AppHandle was previously set to the connection
 *           it will be in phAppConn. In this way the application can keep track
 *           if the connection was created by the application.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - The connection that started the TLS sequence.
 * Output:     phAppConn - The handle given by the application.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipTransportConnectionTlsSequenceStartedEv)(
                                   IN  RvSipTransportConnectionHandle            hConn,
                                   INOUT RvSipTransportConnectionAppHandle*   phAppConn);

/***************************************************************************
 * RvSipTransportConnectionTlsStateChangedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application of TLS
 *           connection state changes. This callback is called only for
 *           TLS state changes and not for connection state changes.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConnection  - The handle of the connection that changed TLS states.
 *          hAppHandle   - The application handle for the connection.
 *           eState       - The connection state.
 *          eReason      - The reason for the state change.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipTransportConnectionTlsStateChangedEv)(
                                                        IN    RvSipTransportConnectionHandle             hConnection,
                                                        IN    RvSipTransportConnectionAppHandle       hAppConnection,
                                                        IN    RvSipTransportConnectionTlsState              eState,
                                                        IN    RvSipTransportConnectionStateChangedReason eReason);

/***************************************************************************
 * RvSipTransportConnectionTlsPostConnectionAssertionEv
 * ------------------------------------------------------------------------
 * General:  This callback is used to override the default post connection
 *           assertion of the Stack. Once a connection has completed the handshake, it is
 *           necessary to make sure that the certificate presented was indeed issued
 *           for the address to which the connection was made. This assertion is
 *           automatically performed by the Stack. If, for some reason, the application wishes
 *           to override a failed assertion, it can implement this callback.
 *           For example, this callback can be used to compare the host name against
 *           a predefined list of outgoing proxies.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConnection   - The handle of the connection that changed TLS states.
 *          hAppConnection - The application handle for the connection.
 *           strHostName   - A NULL terminated string, indicating the host name
 *                          (IP/FQDN) to which the connection was meant to connect.
 *          hMsg          - The message if the connection was asserted against a message.
 * Output: pbAsserted     - RV_TRUE if the connection was asserted successfully.
 *                          RV_FALSE if the assertion failed. In this case, the connection
 *                          will be terminated automatically.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipTransportConnectionTlsPostConnectionAssertionEv)(
                                                        IN    RvSipTransportConnectionHandle             hConnection,
                                                        IN    RvSipTransportConnectionAppHandle          hAppConnection,
                                                        IN    RvChar*                                   strHostName,
                                                        IN    RvSipMsgHandle                             hMsg,
                                                        OUT   RvBool*                                   pbAsserted);

#if defined(RV_SSL_SESSION_STATUS)
/***************************************************************************
 * RvSipTransportConnectionTlsStatusEv
 * ------------------------------------------------------------------------
 * General: Notifies the application about the status of the TLS connection.
 * The application can stop the establishment of the connection with the
 * pbContinue parameter.

 * Return Value: Ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hConn           -   The connection handle.
 *           hAppConnection  -   The handle to the connection application handle.
 *           eStatus         -   The connection status.
 * Output    pbContinue      -   The application instructions.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipTransportConnectionTlsStatusEv) (
                        IN  RvSipTransportConnectionHandle          hConn,
                        IN  RvSipTransportConnectionAppHandle       hAppConnection,
                        IN  RvSipTransportConnectionTlsStatus       eStatus,
                        OUT RvBool*                                 pbContinue);
#endif /*RV_SSL_SESSION_STATUS*/

/***************************************************************************
 * RvSipTransportObjectEventHandler
 * ------------------------------------------------------------------------
 * General:  Processes the termination event of a specific object. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pObj         - A pointer to the object to terminate.
 *        eventReason  - The reason for the event.
 ***************************************************************************/
typedef RvStatus   (RVCALLCONV* RvSipTransportObjectEventHandler)(
                                                        IN    void      *pObj,
                                                        IN    RvInt32   eventReason);

/* RvSipTransportObjEventInfo
 * ------------------------------
 * Each object that wishes to terminate using the event queue must hold such
 * a structure.
 * next                    - A pointer to next object to be terminated.
 * param1                  - Parameter, to be passed to the object in the event handler.
 * func                    - The termination function to be applied. 
 */
typedef struct TransportObjEventInfo {
    struct TransportObjEventInfo            *next;
    RvInt32                                 reason;
    RvSipTransportObjectEventHandler        func;
    void*                                   objHandle;
	RvBool									bInTermination;
} RvSipTransportObjEventInfo;


/***************************************************************************
 * RvSipTransportMsgToSendEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a
 *           new message is about to be sent.
 *           The application can decide whether or not the Transport layer
 *           should transmit the message to its destination.
 * Return Value: RV_TURE to transmit the message. RV_FALSE to discard the
 *               message without transmitting it to destination.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the TransportMgr object.
 *            hAppTransportMgr - The application handle. You supply this handle
 *                         when setting the event handles.
 *            msgBuffer - The message that is about to be sent (given in a consecutive
 *                      buffer).
 *            bufferLen - The length of the message buffer.
 ***************************************************************************/
typedef RvBool (RVCALLCONV * RvSipTransportMsgToSendEv)(
                      IN    RvSipTransportMgrHandle   hTransportMgr,
                      IN    RvSipAppTransportMgrHandle hAppTransportMgr,
                      IN    RvChar                  *msgBuffer,
                      IN    RvUint                   bufferLen);


/***************************************************************************
 * RvSipTransportMsgReceivedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a new message is received.
 *           The application can decide whether the Transport layer
 *           should discard the message, in which case the Stack will not
 *           process this message.
 * IMPORTANT: This method is deprecated and should be replaced with
 *			  RvSipTransportMsgReceivedExtEv().
 * Return Value: RV_TURE to accept the receipt of the message and process it
 *               in the Stack. RV_FALSE to discard the message without
 *               further processing.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the TransportMgr object.
 *            hAppTransportMgr - Application handle. You supply this handle
 *                               when setting the event handles.
 *            hMsgReveived - Received message.
 ***************************************************************************/
typedef RvBool (RVCALLCONV * RvSipTransportMsgReceivedEv)(
                      IN    RvSipTransportMgrHandle   hTransportMgr,
                      IN    RvSipAppTransportMgrHandle hAppTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived);

/***************************************************************************
 * RvSipTransportMsgReceivedExtEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new message was received.
 *          The application can decide whether or not the Transport layer
 *          should discard the message, in which case the Stack will not
 *          process this message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr       - The handle to the TransportMgr object.
 *          hAppTransportMgr    - The application handle. You supply this handle
 *                                when setting the event handles.
 *          hReceivedMsg        - The handle to the received message.
 *          pReceivedMsgDetails - A pointer to received message details structure.
 * Output:  pbProcessMsg		- RV_TRUE if you wish the SIP Stack to processes the
 *                                incoming message. RV_FALSE if you wish the SIP Stack
 *                                to discard the message.
 ***************************************************************************/
typedef void (RVCALLCONV *RvSipTransportMsgReceivedExtEv) (
						 IN    RvSipTransportMgrHandle       hTransportMgr,
						 IN    RvSipAppTransportMgrHandle    hAppTransportMgr,
                         IN    RvSipMsgHandle                hMsgReceived,
                         IN    RvSipTransportRcvdMsgDetails	*pRcvdMsgDetails,
						 OUT   RvBool						*pbProcessMsg);

/***************************************************************************
 * RvSipTransportMsgThreadError
 * ------------------------------------------------------------------------
 * General:  Notifies that one of the Stack processing threads terminated
 *           unexpectedly. Multithreaded applications can register to this
 *           callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the TransportMgr object.
 *            error         - The RV_ERROR_NUM_OF_THREADS_DECREASED error.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipTransportMsgThreadError)(
                     IN RvSipTransportMgrHandle   hTransportMgr,
                     IN RvStatus                  error);

/***************************************************************************
 * RvSipTransportBadSyntaxMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new bad-syntax message was received.
 *           The application can fix the message in this callback and not
 *           at a later time.
 *           The application should use the eAction parameter to decide how
 *           the Stack will handle this message: discard it, continue with message
 *           processing, or send a 400 response (in case of a request message).
 *           See the RvSipTransportBsAction definition for more details.
 *           If the application did not implement this callback, the bad-syntax
 *           message will be discarded.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - The handle to the TransportMgr object.
 *           hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hMsgReveived     - The received bad-syntax message.
 * Output:  peAction         - The decision of the user on how the SIP Stack
 *                             should handle this message.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipTransportBadSyntaxMsgEv)(
                      IN    RvSipTransportMgrHandle   hTransportMgr,
                      IN    RvSipAppTransportMgrHandle hAppTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction);

/***************************************************************************
 * RvSipTransportBadSyntaxStartLineMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new message was received with a bad-syntax start-line.
 *           The application can fix the message in this callback and not
 *           at a later time.
 *           The application should use the eAction parameter to decide how
 *           the Stack will handle this message: discard it, continue with message
 *           processing, or send a 400 response (in case of request message).
 *           See the RvSipTransportBsAction enumeration for more details.
 *           If the application did not implement this callback, the bad-syntax
 *           message will be discarded.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - The handle to the TransportMgr object.
 *           hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hMsgReveived     - The received message with a bad-syntax start-line.
 * Output:  peAction         - The decision of the user on how the SIP Stack
 *                             should handle this message.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipTransportBadSyntaxStartLineMsgEv)(
                      IN    RvSipTransportMgrHandle   hTransportMgr,
                      IN    RvSipAppTransportMgrHandle hAppTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction);

/******************************************************************************
 * RvSipTransportConnectionCreatedEv
 * ----------------------------------------------------------------------------
 * General: Notifies the application about an incoming TCP connection.
 *          The callback is called immediately after the connection is accepted
 *          The application can order the SIP  Stack to close the connection
 *          by means of the pbDrop parameter. In this case, the connection
 *          will be closed immediately after returning from the callback and
 *          its resources will be freed. No data will be received or sent
 *          on the connection. If the application did not register to this callback,
 *          the connection will not be closed, and will be used for
 *          the sending and receiving of data.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - The handle to the TransportMgr.
 *          hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hConn            - The handle to the created connection.
 * Output:  phAppConn        - The handle that the application set for the
 *                             connection.
 *          pbDrop           - If set to RV_TRUE by the application, the connection
 *                             will be dropped immediately after returning from
 *                             the callback. Otherwise, the connection will not
 *                             be dropped and will be used for data reception
 *                             and sending. The default value is RV_FALSE.
 *****************************************************************************/
typedef void (RVCALLCONV * RvSipTransportConnectionCreatedEv)(
                    IN  RvSipTransportMgrHandle             hTransportMgr,
                    IN  RvSipAppTransportMgrHandle          hAppTransportMgr,
                    IN  RvSipTransportConnectionHandle      hConn,
                    OUT RvSipTransportConnectionAppHandle   *phAppConn,
                    OUT RvBool                              *pbDrop);

/***************************************************************************
 * RvSipTransportBufferReceivedEv
 * ------------------------------------------------------------------------
 * General: Exposes the raw data buffer to an application that contains exactly
 *          one SIP message that was received on the TCP/UDP layer.
 *          The application can dump the data by means of this callback.
 *          Also, the application can order the SIP Stack to discard the buffer
 *          and not to parse it, by means of the pbDiscardBuffer parameter.
 *
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - The handle to the TransportMgr object.
 *          hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hLocalAddr       - The handle to local the address object, corresponding to
 *                             the address on which the buffer was received.
 *          pSenderAddrDetails - A pointer to the transport address structure,
 *                             which contains details of the address from which
 *                             the message was sent.
 *          hConn            - The handle of the connection, on which the buffer
 *                             was received. NULL for UDP.
 *          hAppConn         - The handle set by application for the connection.
 *          buffer           - A pointer to the buffer,  which contains the message.
 *          buffLen          - The length of the message in the buffer (in bytes).
 * Output:  bDiscardBuffer   - If set to RV_TRUE, the buffer will not be
 *                             processed and the resources will be freed.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipTransportBufferReceivedEv)(
                    IN  RvSipTransportMgrHandle             hTransportMgr,
                    IN  RvSipAppTransportMgrHandle          hAppTransportMgr,
                    IN  RvSipTransportLocalAddrHandle       hLocalAddr,
                    IN  RvSipTransportAddr                  *pSenderAddrDetails,
                    IN  RvSipTransportConnectionHandle      hConn,
                    IN  RvSipTransportConnectionAppHandle   hAppConn,
                    IN  RvChar                              *buffer,
                    IN  RvUint32                            buffLen,
                    OUT RvBool                              *pbDiscardBuffer);

/***************************************************************************
 * RvSipTransportBufferToSendEv
 * ------------------------------------------------------------------------
 * General: Exposes the raw data buffer to an application. The buffer contains
 *          exactly one SIP message, which is going to be sent on the TCP/UDP layer.
 *          The application can dump the data by means of this callback.
 *          Also, the application can decide whether the Transport layer should
 *          not transmit the message to its destination.
 *          Note that discarding a message does not effect the state of the sender.
 *          object. Discarding a message can simulate message loss on the net.
 *
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - The handle to the TransportMgr object.
 *          hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hLocalAddr       - The handle to local address object, corresponding to
 *                             address from which the buffer is going to be sent.
 *          pDestAddrDetails - A pointer to the transport address structure.
 *                             The address contains details of the address
 *                             to which the message is going to be sent.
 *          hConn            - The handle of the connection on which the buffer
 *                             is going to be sent. NULL for UDP.
 *          hAppConn         - The handle set by the application for the connection.
 *          buffer           - A pointer to the buffer,  which contains the message.
 *          buffLen          - The length of the message in the buffer (in bytes).
 * Output:  bDiscardBuffer   - If set to RV_TRUE, the buffer will be not sent and
 *                             the resources will be freed.
 ***************************************************************************/
typedef void (RVCALLCONV * RvSipTransportBufferToSendEv)(
                    IN  RvSipTransportMgrHandle             hTransportMgr,
                    IN  RvSipAppTransportMgrHandle          hAppTransportMgr,
                    IN  RvSipTransportLocalAddrHandle       hLocalAddr,
                    IN  RvSipTransportAddr                  *pDestAddrDetails,
                    IN  RvSipTransportConnectionHandle      hConn,
                    IN  RvSipTransportConnectionAppHandle   hAppConn,
                    IN  RvChar                              *buffer,
                    IN  RvUint32                            buffLen,
                    OUT RvBool                              *pbDiscardBuffer);

/******************************************************************************
 * RvSipTransportConnectionParserResultEv
 * ----------------------------------------------------------------------------
 * General: Indicates the parsing result for the message to an application
 *          which arrived over the TCP connection.
 *          If the parser encounters bad syntax, the bLegalSyntax is RV_FALSE.
 *          Otherwise it is RV_TRUE.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - The handle to the transport manager object (TransportMgr).
 *          hAppTransportMgr - The application handle. You supply this handle
 *                             when setting the event handles.
 *          hMsg             - The handle to the message that was parsed.
 *          hConn            - The handle to the connection, on which the message
 *                             to be parsed has arrived.
 *          hAppConn         - The handle that the application sets for
 *                             the connection.
 *          bLegalSyntax     - RV_TRUE, if no bad syntax was discovered by
 *                             parser. Otherwise, RV_FALSE.
 * Output:  none.
*****************************************************************************/
typedef void (RVCALLCONV * RvSipTransportConnectionParserResultEv)(
                        IN  RvSipTransportMgrHandle           hTransportMgr,
                        IN  RvSipAppTransportMgrHandle        hAppTransportMgr,
                        IN  RvSipMsgHandle                    hMsg,
                        IN  RvSipTransportConnectionHandle    hConn,
                        IN  RvSipTransportConnectionAppHandle hAppConn,
                        IN  RvBool                            bLegalSyntax);

/******************************************************************************
* RvSipTransportConnectionDataReceivedEv
* ----------------------------------------------------------------------------
* General: Notifies the application that data was read from the socket
*          of the incoming connection.
* Return Value: None.
* ----------------------------------------------------------------------------
* Arguments:
* Input:   hTransportMgr    - The handle to the TransportMgr.
*          hAppTransportMgr - The application handle. You supply this handle
*                             when setting the event handles.
*          hConn            - The handle to the connection on which the
*                             message data was received.
*          hAppConn         - The handle that the connection sets into the application.
*          buff             - The buffer containing the read data.
*          buffSize         - The size of the read data in bytes.
* Output:  pbDiscard        - If RV_TRUE, the received data will be discarded.
*****************************************************************************/
typedef void (RVCALLCONV * RvSipTransportConnectionDataReceivedEv)(
                        IN  RvSipTransportMgrHandle           hTransportMgr,
                        IN  RvSipAppTransportMgrHandle        hAppTransportMgr,
                        IN  RvSipTransportConnectionHandle    hConn,
                        IN  RvSipTransportConnectionAppHandle hAppConn,
                        IN  RvChar                            *buff,
                        IN  RvInt32                           buffSize,
                        OUT RvBool                            *pbDiscard);

/******************************************************************************
* RvSipTransportConnectionServerReuseEv
* ----------------------------------------------------------------------------
* General: When an incoming request has an alias parameter in its top-most
*          Via header, this callback function is called.
*          This callback informs application that a server connection
*          should be reused, and it has to be authorized first.
*          In this callback application should authenticate the connection, and
*          if the connection was authorized, the application should call the
*          RvSipTransportConnectionEnableConnByAlias() function. Calling
*          this function enables the connection for reuse.
* Return Value: none.
* ----------------------------------------------------------------------------
* Arguments:
* Input:   hTransportMgr    - The handle to the TransportMgr.
*          hAppTransportMgr - The application handle. You supply this handle
*                             when setting the event handlers.
*          hConn            - The handle to the connection that needs to be authorized.
*          hAppConn         - The handle that the application set for the connection.
* Output:  none.
*****************************************************************************/
typedef void (RVCALLCONV * RvSipTransportConnectionServerReuseEv)(
                        IN  RvSipTransportMgrHandle           hTransportMgr,
                        IN  RvSipAppTransportMgrHandle        hAppTransportMgr,
                        IN  RvSipTransportConnectionHandle    hConn,
                        IN  RvSipTransportConnectionAppHandle hAppConn);


/* RvSipTransportMgrEvHandlers
 * ------------------------------------------------------------------------
 * Structure with function pointers to the Transport module callbacks.
 * This structure is used to set the application call-backs in the function
 * RvSipTransportMgrSetEvHandlers();
 *
 * pfnEvMsgToSend       - Notifies the application that a message is about to be sent.Allows
 *                        the application to decide whether or not to continue
 *                        sending this message.
 * pfnEvMsgRecvd        - Notifies the application that a message was received. Allows
 *                        the application to decide whether or not to continue
 *                        processing this message.
 * pfnThreadError       - Notifies the application of a thread that was exited unexpectedly.
 * pfnEvBadSyntaxMsg    - Notifies the application that a bad-syntax message was received.
 * pfnEvBadSyntaxStartLineMsg - Notifies the application that a message with a bad-syntax
 *                        start-line was received.
 * pfnEvTlsStateChanged - Notifies the application on TLS connection state
 *                        changes.
 * pfnEvTlsPostConnectionAssertion - If set to a value other the NULL, this parameter will
 *                        let the application override post connection assertions that fail.
 * pfnEvTlsSeqStarted   - Notifies the application of the beginning of a TLS procedure on a connection.
 * pfnEvBufferReceived  - Gives the application the opportunity to discard
 *                        the buffer before parsing. The remote and local
 *                        addresses are supplied. The application can also dump the buffer.
 * pfnEvBufferToSend    - Allows the application to dump the message buffer.
 * pfnEvConnCreated     - Notifies the application about the creation of a connection
 *                        object for an incoming TCP connection.
 * pfnEvConnParserResult- Notifies the application of the result of the parsing of an incoming message.
 * pfnEvConnStateChanged- Notifies application about incoming connection
 *                        state change.
 * pfnEvMsgRecvdExt      - Notifies application about the reception of a SIP message.
 * pfnEvConnDataReceived - Notifies application about receiving data on the connection.
 * pfnEvConnServerReuse  - Notifies application about a server connection that can be reused.
 */
typedef struct
{
    RvSipTransportMsgToSendEv                               pfnEvMsgToSend;
    RvSipTransportMsgReceivedEv                             pfnEvMsgRecvd;
    RvSipTransportMsgThreadError                            pfnThreadError;
    RvSipTransportBadSyntaxMsgEv                            pfnEvBadSyntaxMsg;
    RvSipTransportBadSyntaxStartLineMsgEv                   pfnEvBadSyntaxStartLineMsg;
    RvSipTransportConnectionTlsStateChangedEv               pfnEvTlsStateChanged;
    RvSipTransportConnectionTlsPostConnectionAssertionEv    pfnEvTlsPostConnectionAssertion;
    RvSipTransportConnectionTlsSequenceStartedEv            pfnEvTlsSeqStarted;
    RvSipTransportBufferReceivedEv                          pfnEvBufferReceived;
    RvSipTransportBufferToSendEv                            pfnEvBufferToSend;
    RvSipTransportConnectionCreatedEv                       pfnEvConnCreated;
    RvSipTransportConnectionParserResultEv                  pfnEvConnParserResult;
    RvSipTransportConnectionStateChangedEv                  pfnEvConnStateChanged;
#if defined(RV_SSL_SESSION_STATUS)
    RvSipTransportConnectionTlsStatusEv                     pfnEvConnTlsStatus;
#endif /*RV_SSL_SESSION_STATUS*/
	RvSipTransportMsgReceivedExtEv						    pfnEvMsgRecvdExt;
    RvSipTransportConnectionDataReceivedEv                  pfnEvConnDataReceived;
    RvSipTransportConnectionServerReuseEv                   pfnEvConnServerReuse;
} RvSipTransportMgrEvHandlers;


/* RvSipTransportConnectEvHandlers
 * ------------------------------------------------------------------------
 * A structure with function pointers to the connection callbacks.
 * You should supply this structure for every new connection.
 * pfnConnStateChangedEvHandler - The callback function to notify the owner
 *                                about the connection status.
 * pfnConnStausEvHandler - The callback function to notify owner about the connection
 *                         status, such as ERROR.
 */
typedef struct
{
    RvSipTransportConnectionStateChangedEv  pfnConnStateChangedEvHandler;
    RvSipTransportConnectionStatusEv        pfnConnStausEvHandler;
} RvSipTransportConnectionEvHandlers;

/* RvSipTransportConnectionType
 * ---------------------------------------
 * type of the connection oriented connection (e.g. TCP,SCTP).
 * RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER stands for the Connection object,
 * which holds listening for incoming connections socket
 */
typedef enum{
    RVSIP_TRANSPORT_CONN_TYPE_UNDEFINED,
    RVSIP_TRANSPORT_CONN_TYPE_CLIENT,
    RVSIP_TRANSPORT_CONN_TYPE_SERVER,
    RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER
} RvSipTransportConnectionType;

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* TransportError handlers 
*/
#if defined(UPDATED_BY_SPIRENT)

typedef RvStatus (RVCALLCONV *RvSipTransportErrorEv) (
                                          IN  RvChar   *pEventTextMsg);

typedef struct 
{
    RvSipTransportErrorEv pfnRejectMsgEvHandler;
} RvSipTransportErrorEvHandlers;

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RVSIP_TRANSPORTTYPES_H */



