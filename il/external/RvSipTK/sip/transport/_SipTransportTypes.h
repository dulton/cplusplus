
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
 *                              SipTransportTypes.h
 *
 *  This file provides definitions for the transport layer implementation of the sip
 *  stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20-Nov-2000
 *********************************************************************************/


#ifndef _SIP_TRANSPORT_TYPES_H
#define _SIP_TRANSPORT_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RvSipMsg.h"
#include "RV_SIP_DEF.h"
#include "RvSipTransportTypes.h"

#include "rvlog.h"
#include "rvaddress.h"
#include "rvrandomgenerator.h"
#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTPTypes.h"
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* declaring the message context */
RV_DECLARE_HANDLE(SipTransportCallbackContextHandle);

/* SipTransportMsgType
 * ---------------------
 * The type of message sent/about to be sent by the connection.
 * This is important to the transaction layer. It needs to know the
 * type of message that was sent when there is a sent event in order to
 * adjust the transaction timers.
 */
typedef enum{
    SIP_TRANSPORT_MSG_UNDEFINED = -1,
    SIP_TRANSPORT_MSG_REQUEST_SENT,
    SIP_TRANSPORT_MSG_ACK_SENT,
    SIP_TRANSPORT_MSG_FINAL_RESP_SENT,
    SIP_TRANSPORT_MSG_PROV_RESP_REL_SENT,
    SIP_TRANSPORT_MSG_OUT_OF_CONTEXT,
    SIP_TRANSPORT_MSG_PROXY_2XX_SENT,
    SIP_TRANSPORT_MSG_PROV_RESP_SENT,
    SIP_TRANSPORT_MSG_APP
}SipTransportMsgType;

/* SipTransportSendInfo
 * --------------------
 * Holds information about the message that was/about to be sent using TCP
 */
typedef struct {
    HPAGE          msgToSend;   /*the message to be sent on a page*/
    RvInt32        msgSize;     /*the size of the message to be sent*/
    RvInt32        currPos;     /*the size we sent so far - needed for event read*/
    SipTransportMsgType   msgType;
    RvBool         bFreeMsg;   /*indicates whether the connection is responsible to free the message page*/
    RvBool         bWasNotifiedMsgNotSent;
                        /*indicates whether the MSG_NOT_SENT status was already
                          notified for this message*/
    RvSipTransportConnectionOwnerHandle hOwner;
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpMsgSendingParams  sctpData; /* Parameters, used while sending message over SCTP */
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    RvBool         bDontRemove; /*if RV_TRUE, the message should never be removed
                                  from MsgToBeSent list unless it is sent.
                                  Even if the message owner is terminated. */
}SipTransportSendInfo;



/*
 * SipTransportCfg
 * ----------------
 * regId            - log registration number, used for log printing
 * hLog             - handle to the log module
 * pMBLogSrc        - msg builder log registration number, used for log printing
 * pLogMgr          - ccore log manager
 * maxTimers        - maximum timers stack will need
 * hMsgMgr          - handle to the message manager
 * hMsgPool         - handle to the message pool - used to build incoming messages.
 * maxConnOwners    - max owners possible for all/each connection.
 * maxBufSize       - Maximum length of the udp buffer
 * TCPAddress       - the ip address of the TCP;
 * TCPPort          - the port of the TCP;
 * maxConnection    - the number of TCP connections available.
 * ePersistencyLevel - the persistency level to use
 * tcpEnabled       - Boolean indicating if TCP is enabled or not.
 * maxPageNuber     - Number of pages is the message pool - limit the number of cuncurrent
 *                    messages that can be sent by the transport layer.
 * Port             - The UDP port that we are going to use for opening socket and receive data
 * localIp          - the IP address of the udp
 * maxElementsInSingleDnsList
 *                    - maximum number of elements in single DNS list
 * numberOfProcessingThreads
 *                    _ number of processing threads
 * processingQueueSize
 *                    - size of the processing queue
 * numOfReadBuffers    - number of read buffers
 *
 * processingTaskPriority - relevant only for RTOS (VxWorks, NUCLEUS).
 *                            Defines priority of processing tasks
 *
 * processingTaskStackSize - relevant only for RTOS (VxWorks, NUCLEUS).
 *                             Defines size of processing tasks stack in bytes.
 *
 * hStack - Handle to the stack manager.
 * TlsHandshakeTimeout    - the time given to complete a handshake (currently T1*64)
 *
 * bDLAEnabled - if RV_FALSE, no local address adding and removal will be enabled
 *
 * hSigCompMgr  - Handle to the SigComp manager.
 */
typedef struct {
    RvLogSource*            regId;
    RvLogSource*            pMBLogSrc;
    RvLogMgr*               pLogMgr;

    RvInt32                 maxTimers;
    RvSipMsgMgrHandle       hMsgMgr;
    HRPOOL                  hMsgPool;
    HRPOOL                  hGeneralPool;
    HRPOOL                  hElementPool;
    RvInt32                 maxConnOwners;
    RvInt32                 maxDNSQueries;
    RvUint32                maxBufSize;
    RvUint16                udpPort;
    RvChar                  udpAddress[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvRandomGenerator*      seed;
    void*                   hStack;
    RvChar*                 outboundProxyIp;
    RvChar*                 outboundProxyHostName;
    RvInt32                 outboundProxyPort;
    RvSipTransport          eOutboundProxyTransport;
#ifdef RV_SIGCOMP_ON
    RvSipCompType           eOutboundProxyCompression;
#endif /* RV_SIGCOMP_ON */
    RvChar                  tcpAddress[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvUint16                tcpPort;
    RvInt32                 maxConnections;
    RvSipTransportPersistencyLevel ePersistencyLevel;
    RvInt32                 serverConnectionTimeout;
    RvBool                  tcpEnabled;
    RvInt32                 maxPageNuber;
    RvChar**                localUdpAddresses;
    RvUint16*               localUdpPorts;
    RvChar**                localTcpAddresses;
    RvUint16*               localTcpPorts;
    RvInt32                 numOfExtraUdpAddresses;
    RvInt32                 numOfExtraTcpAddresses;

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvUint32                maxElementsInSingleDnsList;
#endif
    RvUint32                numberOfProcessingThreads;
    RvInt32                 processingQueueSize;
    RvInt32                 numOfReadBuffers;
    RvUint32                processingTaskPriority;
    RvUint32                processingTaskStackSize;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* tls parameters */
    RvChar**                localTlsAddresses;
    RvUint16*               localTlsPorts;
    RvInt32                 numOfTlsAddresses;
    RvInt32                 numOfTlsEngines;
    RvInt32                 maxTlsSessions;
    RvInt32                 TlsHandshakeTimeout;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    RvInt32                 maxNumOfLocalAddresses;
	RvBool                  bIgnoreLocalAddresses;
    RvBool                  bDLAEnabled;
    RvInt32                 connectionCapacityPercent;
#if (RV_NET_TYPE & RV_NET_SCTP)
    /* SCTP parameters */
    TransportCfgSctp        sctpCfg;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
  RV_TOSDiffserv         localPriority;
  RV_TOSDiffserv *       localPriorities;
  RvBool                 isImsClient;
  RvChar                 if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
  RvUint16               vdevblock;
  RvChar**               localIfNames;
  RvUint16*              localVDevBlocks;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */

}SipTransportCfg;


/* TransportLocalAddressType
 * ---------------------------
 * The TransportLocalAddressType Defines 4 types of local address:
 * UDP address on IP4, UDP address on IP6, TCP address on IP4 and TCP address on IP6.
 */
typedef enum
{
    TRANSPORT_LOCAL_ADDRESS_UNDEFINED = -1,
    TRANSPORT_LOCAL_ADDRESS_UDP_IP4,
    TRANSPORT_LOCAL_ADDRESS_UDP_IP6,
    TRANSPORT_LOCAL_ADDRESS_TCP_IP4,
    TRANSPORT_LOCAL_ADDRESS_TCP_IP6
} TransportLocalAddressType;



/* SipTransportObjLocalAddresses
 * --------------------------------------------------------------------------
 * SipTransportObjLocalAddresses is a structure held by each of the stack 
 * objects that is used for sending messages.
 *
 */
typedef struct
 {
      RvSipTransportLocalAddrHandle hLocalUdpIpv4;
      RvSipTransportLocalAddrHandle hLocalUdpIpv6;

      RvSipTransportLocalAddrHandle hLocalTcpIpv4;
      RvSipTransportLocalAddrHandle hLocalTcpIpv6;
#if (RV_TLS_TYPE != RV_TLS_NONE)
      RvSipTransportLocalAddrHandle hLocalTlsIpv4;
      RvSipTransportLocalAddrHandle hLocalTlsIpv6;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
      RvSipTransportLocalAddrHandle hLocalSctpIpv4;
      RvSipTransportLocalAddrHandle hLocalSctpIpv6;
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP)*/

      RvSipTransportLocalAddrHandle *hAddrInUse;
} SipTransportObjLocalAddresses;

/* SipTransportAddress
 * ---------------------------------
 * Contains information about an address:
 * RvAddress: with port and ip
 * Transport
 */
typedef struct
{
    RvAddress           addr;
    RvSipTransport      eTransport;
}SipTransportAddress;

/* SipTransportOutboundAddress
 * ---------------------------------
 * Contains information about the outbound address of the object
 */
typedef struct
{
    RPOOL_Ptr                   rpoolHostName;
    RvBool                      bIpAddressExists; /*indicate that the ip address structure contains a valid ip*/
    SipTransportAddress         ipAddress;
    RvBool                      bUseHostName;  /*indicates whether to use the host name in case both
                                                 ip and host exists for an object*/
#ifdef RV_SIGCOMP_ON
    RvSipCompType               eCompType;      /* Indicates what is the compression type of */
	RPOOL_Ptr                   rpoolSigcompId; /* sigcomp-id, RFC 5049 */
                                                /* the connection towards the outbound proxy */
#endif
#ifdef SIP_DEBUG
    RvChar                      *strHostName;
#ifdef RV_SIGCOMP_ON
	RvChar                      *strSigcompId;
#endif
#endif
} SipTransportOutboundAddress;



/* SipTransportSigCompMsgInfo
 * --------------------------------------------------------------------------
 * TransportSigCompMsgInfo structure contains event SigComp specific info
 * of the MESSAGE_RCVD_EVENT event of the processing queue.
 *
 * bExpectComparmentID - Indicates if the UDVM is waiting for a compartment
 *                       ID to be related with its unique ID.
 * uniqueID            - The uncompressed message unique ID that is returned
 *                       by the UDVM.
 * comparmentID        - The compartment ID that is going to be related with
 *                       the unique ID.
 */

typedef struct
{
    RvBool   bExpectComparmentID;
    RvUint32 uniqueID;
} SipTransportSigCompMsgInfo;

/* SipTransportMgrBuilderPrintMsgDir
 * --------------------------------------------------------------------------
 * SipTransportMgrBuilderPrintMsgDir enum defines different directions of 
 * incoming/outgoing message that are about to be printed 
 * 
 * TRANSPORT_MSG_BUILDER_INCOMING - Presents incoming message.
 * TRANSPORT_MSG_BUILDER_OUTGOING - Presents outgoing message.
 *
 */
typedef enum{
    SIP_TRANSPORT_MSG_BUILDER_INCOMING,
    SIP_TRANSPORT_MSG_BUILDER_OUTGOING,
    SIP_TRANSPORT_MSG_BUILDER_UNDEFINED
} SipTransportMgrBuilderPrintMsgDir;

/******************************************************************************
 * SipTransportObjectStateEventHandler
 * ----------------------------------------------------------------------------
 * General: This function processes event from the Transport Processing Queue,
 *          raised for the specified object.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj   - The pointer to the Object, which should handle the event.
 *          pFuncDetails - Pointer to the structure containing all parameters 
 *                   required for this function.
 *****************************************************************************/
typedef RvStatus (RVCALLCONV* SipTransportObjectStateEventHandler)(
                                IN void*     pObj,
                                IN RvInt32   param1,
                                IN RvInt32   param2,
                                IN RvInt32   objUniqueIdentifier);

/*-----------------------------------------------------------------------
       T R A N S P O R T:  C A L L - B A C K  F U N C T I O N S
 ------------------------------------------------------------------------*/


/***************************************************************************
 * SipTransportMessageRcvdEv
 * ------------------------------------------------------------------------
 * General: Called when when a new message is passed from the transport layer
 *          to the upper (transaction layer)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     msgContext      - Handle to layer receiving the message
 *          hReceivedMsg    - Handle to the received message
 *          hConnInfo       - The connection handle or NULL when UDP was used
 *          pLocalAddr      - The local address that the message was received on.
 *          pRecvFromAddr   - The address from which the message was received.
 *          eBSAction       - Bad-syntax action, that was given by application
 *                            in bad-syntax callbacks. continue processing,
 *                            reject, discard.
 *          pSigCompMsgInfo - SigComp info related to the received message.
 *                            The information contains indication if the UDVM
 *                            waits for a compartment ID for the given unique
 *                            ID that was related to message by the UDVM.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTransportMessageRcvdEv) (
                         IN    SipTransportCallbackContextHandle msgContext,
                         IN    RvSipMsgHandle                    hReceivedMsg,
                         IN    RvSipTransportConnectionHandle    hConnInfo,
                         IN    RvSipTransportLocalAddrHandle       hLocalAddr,
                         IN    SipTransportAddress*              pRecvFromAddr,
                         IN    RvSipTransportBsAction            eBSAction,
                         INOUT SipTransportSigCompMsgInfo       *pSigCompMsgInfo);

/* SipTransportEvHandlers
 * ----------------------
 * Holds the callbacks of the transport layer.
 * pfnMsgRcvdEvHandler - called after a message is received and parsed by the
 *                       transport layer. The message is passed to the transaction
 *                       layer for further processing.
 */

typedef struct {
    SipTransportMessageRcvdEv   pfnMsgRcvdEvHandler;
}SipTransportEvHandlers;

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportProtectedMsgRcvdEv
 * ----------------------------------------------------------------------------
 * General: is called when a protected message was received.
 *          The message can be discraded through pbApproved parameter.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - Handle to the received message
 *          pRecvFromAddr - The address from which the message was received.
 *          pSecOwner     - Pointer to the Security Object, owning the local
 *                          address or connection, on which the message was
 *                          received.
 * Output:
 *          pbApproved    - RV_TRUE, if the message was approved by Security
 *                          Module, RV_FALSE - otherwise. In case of RV_FALSE
 *                          the Message will be discraded.
 *****************************************************************************/
typedef void (RVCALLCONV *SipTransportProtectedMsgRcvdEv) (
                                        IN  RvSipMsgHandle       hMsg,
                                        IN  SipTransportAddress* pRecvFromAddr,
                                        IN  void*                pSecOwner,
                                        OUT RvBool*              pbApproved);

/******************************************************************************
 * SipTransportMsgSendFailureEv
 * ----------------------------------------------------------------------------
 * General: Is called when a protected message sending was failed due to error,
 *          returned by the system call, which sends buffers.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgPage   - Handle to the page,where the encoded message is stored
 *          hLocalAddr - Handle to the local address, if the message was being
 *                       sent over UDP
 *          hConn      - Handle to the connection, if the message was being
 *                       sent over TCP / TLS / SCTP
 *          pSecOwner  - Pointer to the Security Object, owning the local
 *                       address or connection, on which the message was
 *                       being sent.
 *****************************************************************************/
typedef void (RVCALLCONV *SipTransportMsgSendFailureEv) (
                                IN HPAGE                          hMsgPage,
                                IN RvSipTransportLocalAddrHandle  hLocalAddr,
                                IN RvSipTransportConnectionHandle hConn,
                                IN void*                          pSecOwner);

/* SipTransportSecEvHandlers
 * -------------------------
 * Holds the callbacks of the transport layer, implemented by Security Module.
 * pfnMsgRcvdHandler - Is called after a message is received and parsed.
 *                     Security module can disapporve message. In this case
 *                     the message will be discarded.
 * pfnMsgSendFailureHandler - Is called if the system call, causing message
 *                            sending, failed.
 * pfnConnStateChangedHandler - Is called on connection state change.
 */
typedef struct {
    SipTransportProtectedMsgRcvdEv	        pfnMsgRcvdHandler;
    SipTransportMsgSendFailureEv	        pfnMsgSendFailureHandler;
    RvSipTransportConnectionStateChangedEv  pfnConnStateChangedHandler;
}SipTransportSecEvHandlers;
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _RV_SIP_TRANSPORT_DEF_H*/
