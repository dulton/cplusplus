/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportMgrObject.h
 *
 *  This file provides headers for the transport layer of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/


#ifndef TRANSPORT_MGR_OBJECT_H
#define TRANSPORT_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

/* ccore*/
#include "rvselect.h"
#include "rvsocket.h"
#include "ares.h"
#include "rvares.h"
#include "rvmutex.h"
#include "rvtls.h"

#include "AdsRlist.h"
#include "AdsRa.h"
#include "AdsPqueue.h"
#include "AdsHash.h"
    
#include "_SipTransportTypes.h"
#include "_SipCommonTypes.h"
#include "RvSipTransportDNSTypes.h"
#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTPTypes.h"
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* Next define should be updated, when the RvSipTransportAddressType enumeration is updated  */
#define TRANSPORT_NUM_OF_ADDR_TYPES (3)
/* Next define should be updated, when the RvSipTransport enumeration is updated  */
#define TRANSPORT_NUM_OF_TRANSPORT_TYPES (6)

/*
 * TransportOutboundAddr
 * ----------------------
 * Information about the outbound proxy used by the stack for sending
 * outgoing request. (A host name is used only if there is no ip address)
 */
typedef struct  {
    RvChar*        strIp;        /*ip address of the outbound proxy*/
    RvChar*        strHostName;  /*host name of the outbound proxy if there is no ip*/
    RvUint16       port;         /*port of the outbound proxy*/
    RvSipTransport transport;    /*transport to use with the outbound proxy*/
#ifdef RV_SIGCOMP_ON
    RvSipCompType  compType;     /*compression type to be used with outbound proxy*/
	RvChar*        strSigcompId; /* sigcomp-id */
#endif
} TransportOutboundAddr;


#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * Definition of TlsMgr:
 * hLocalTlsAddrList - TLS addresses
 * numOfEngines      - max number of engines
 * maxSessions       - max number of sessions
 * hraEngines        - RA for TLS engines
 * hraSessions    - RA for TLS sessions
 * TlsHandshakeTimeout  - the time given to complete a handshake (currently T1*64)
 */
typedef struct {
    /* tls */
    RLIST_HANDLE                    hLocalTlsAddrList;
    RvInt32                         numOfEngines;
    RvInt32                         maxSessions;
    HRA                             hraEngines;
    HRA                             hraSessions;
    RvInt32                         numOfTlsAddresses;
    RvInt32                         TlsHandshakeTimeout;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE                     hLocalTlsAddrHash;
#endif
} TlsMgr;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/******************************************************************************
 * Definition of TransportMgr:
 * maxBufSize           - Maximum length of the udp buffer
 * hraSendBuffer        - RA handle for allocating sending udp buffer
 * hraRecvBuffer        - RA handle for allocating receiving udp buffer
 * sendBufferId         - ra Id for the sending buffer
 * recvBufferId         - ra Id for the receiving buffer
 * logHandle            - handle to the log
 * preemptionDispatchEvents - number that indicates dispach events event
 * preemptionReadBuffersReady - number that indicates a buffers ready event
 * preemptionCellAvailable - number that indicates a cell available event
 * pMBLogSrc            - used for log printing
 * recvCallbackFunction - the callback function to be invoked when receiving a message
 * hMsgPool             - handle to the HRPOOL message pool
 * hGeneralPool         - handle to general pool
 * hCallbackContext     - handle to the entity that will receive the callbacks
 * tcpEnabled           - TCP enables or not by configuration
 * maxConnections       - how many connection are available
 * hConnPool            - pool to hold the connection
 * hConnList            - the list of connections
 * connectionPoolLock   - locks the connection pool before adding / removing elements.
 * hMsgToSendPoolList   - the pool of msg to send lists
 * hConnectionOwnersPoolList- the pool of list holds the connection owners
 * maxPageNuber         - amount of pages available.
 * evHandlers           - implementation for call-back functions.
 * pAppContext          - Application transport manager.
 * hMessageMemoryBuffPool   - pool of buffers to be used for reading incoming
 *                            SIP messages. Note that each buffer contains entire
 *                            SIP message.
 * noEnoughBuffersInPool    - mark that specifies if there was no enough buffers
 *                            during last memory buffer allocation from
 *                            the hMessageMemoryBuffPool
 * notEnoughBuffersInPoolForUdp - marks that UDP OOR recovery needs to be done
 * hProcessingQueue         - handler of the processing queue
 * processingQueueLockPool  - processing queue lock pool
 * numberOfProcessingThreads- number of processing threads
 * processingThreads        - array of processing threads objects
 * hSeli                    - SELI handler, used for resource available
 *                            notifications.
 * pEventToBeNotified       - pointer to list of events that failed to be
 *                            notified due to not enough resources
 * pTpqObjEventList         - pointer to list of TPQ Object Events that were
 *                            not insreted into the Transport Processing Queue
 *                            due to luck of resources.
 * hRcvBufferAllocLock      - lock to protect the receive buffers allocation
 *
 * processingTaskPriority   - relevant only for RTOS (VxWorks, NUCLEUS)
 *                            when stack is executed in multi-threaded mode.
 *                            Defines priority of processing tasks.
 * processingTaskStackSize  - relevant only for RTOS (VxWorks, NUCLEUS).
 *                            Defines size of processing tasks stack in bytes.
 *
 * lockLocalAddresses   - Lock for lists of local addresses (of all types - UDP, TCP, e.t.c)
 * bDLAEnabled          - If RV_FALSE, adding and removal local addresses will be disabled
 * localAddrCounters    - 2-dimensional array, which contains counters of local
 *                        addresses, opened in the stack, grouped by Transport
 *                        Protocol type and by IP address type
 * hSigCompMgr          - Handle to the SigComp manager.
 * bReuseServerConnection - Indicates if the stack is configured to re-use server
 *                        connections or not.
 * connectionCapacityPercent - Defines the capacity percent of open connections. 
 *                        When stack reaches this limit, it will close a connection that is not used,
 *                        whenever a new connection is opened.
 * bMultiThreadEnvironmentConstructed - Have we already constructed the multi-thread environment
 * secEvHandlers    - pointers to event handlers,implemented by Security Module
 *****************************************************************************/
typedef struct {

    /* ccore additions */
    RvSelectEngine*                 pSelect;
#if !defined(UPDATED_BY_SPIRENT)
#if (RV_DNS_TYPE == RV_DNS_ARES)
    RvDnsEngine                     dnsEngine;
#endif
#endif
    RvLogMgr*                       pLogMgr;
    RvUint8                         preemptionDispatchEvents;
    RvUint8                         preemptionReadBuffersReady;
    RvUint8                         preemptionCellAvailable;
    RvInt32                         maxTimers;
    RLIST_POOL_HANDLE               oorListPool;
    RLIST_HANDLE                    oorList;
    /* end ccore*/

    RvUint32                        maxBufSize;
    HRA                             hraSendBuffer;
    HRA                             hraRecvBuffer;
    RA_Element                      sendBuffer;
    RA_Element                      recvBuffer;
    RvLogSource*                    pLogSrc;
    RvLogSource*                    pMBLogSrc;

    RvInt32                         maxNumOfLocalAddresses;
    RLIST_POOL_HANDLE               hLocalAddrPool;
    TransportOutboundAddr           outboundAddr;
    RLIST_HANDLE                    hLocalTcpAddrList;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE                     hLocalTcpAddrHash;
#endif
    RvInt32                         numOfTcpAddresses;
    RLIST_HANDLE                    hLocalUdpAddrList;
#if defined(UPDATED_BY_SPIRENT)
    HASH_HANDLE                     hLocalUdpAddrHash;
#endif

    RvInt32                         numOfUdpAddresses;

    RvMutex                         udpBufferLock;
    RvMutex                         hObjEventListLock;
    RvMutex                         hRcvBufferAllocLock;
    RvMutex                         hOORListLock;
    RvMutex                         lockLocalAddresses;

    HRPOOL                          hMsgPool;
    HRPOOL                          hGeneralPool;
    HRPOOL                          hElementPool;
    RvSipMsgMgrHandle               hMsgMgr;
    SipTransportCallbackContextHandle hCallbackContext;
    SipTransportEvHandlers          eventHandlers;
    RvSipTransportMgrEvHandlers     appEvHandlers;
    RvSipAppTransportMgrHandle      hAppTransportMgr;
    RvUint32                        localAddrCounters[TRANSPORT_NUM_OF_ADDR_TYPES][TRANSPORT_NUM_OF_TRANSPORT_TYPES];

    RvRandomGenerator               *seed;
    void                            *hStack;              /* Handle to the stack instance */
    RvBool                          bIsShuttingDown;
    RvBool                          tcpEnabled;
    RvInt32                         maxConnections;
    RvInt32                         maxConnOwners;
    RvInt32                         maxDNSQueries;
    RvSipTransportPersistencyLevel  ePersistencyLevel;
    RvInt32                         serverConnectionTimeout;
    RLIST_POOL_HANDLE               hConnPool;
    RLIST_HANDLE                    hConnList;
    HASH_HANDLE                     hConnHash;
    RvInt32                         connHashSize;
    HASH_HANDLE                     hConnOwnersHash;
    RvInt32                         connOwnersHashSize;
    RvMutex                         connectionPoolLock;
    RLIST_POOL_HANDLE               hMsgToSendPoolList;
    RLIST_POOL_HANDLE               hConnectionOwnersPoolList;
    RvInt32                         maxPageNuber;
    RvBool                          bMultiThreadEnvironmentConstructed;
#if defined(UPDATED_BY_SPIRENT)
    RV_TOSDiffserv                  localPriority;
    RvBool                          bLdaValid;      /* Lda field is valid      */
    SipTransportObjLocalAddresses   Lda;            /* Local Default Addresses */
#endif /* defined(UPDATED_BY_SPIRENT) */ 


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvUint32                        maxElementsInSingleDnsList;
#endif
    HRA                             hMessageMemoryBuffPool;
    RvBool                          noEnoughBuffersInPool;
    RvBool                          notEnoughBuffersInPoolForUdp;
    RvBool                          timerOOR;
    RV_PROCESSING_QUEUE             hProcessingQueue;
    RvMutex                         processingQueueLock;
    RvUint32                        numberOfProcessingThreads;
    RvThread*                       processingThreads;
    RvSipTransportObjEventInfo*     pEventToBeNotified;
    RvUint32                        processingTaskPriority;
    RvUint32                        processingTaskStackSize;

    RvUint32                        connectionCapacityPercent;
    RLIST_POOL_HANDLE               hConnectedNoOwnerConnPool;
    RLIST_HANDLE                    hConnectedNoOwnerConnList;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    TlsMgr                          tlsMgr;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    RvBool                          bDLAEnabled;
#ifdef RV_SIGCOMP_ON
    RvSigCompMgrHandle              hSigCompMgr;
#endif /* RV_SIGCOMP_ON */
#if (RV_NET_TYPE & RV_NET_SCTP)
    TransportMgrSctpData            sctpData;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
    SipTransportSecEvHandlers       secEvHandlers;
#endif

}TransportMgr;


/* TransportMgrLocalAddr
 * --------------------------------------------------------------------------
 * TransportMgrLocalAddr represents a local address that the stack listen
 * to. The stack open each local address on initialization, or in run-time
 * by application requests.
 * hAppLocalAddr  - An application handle to the local address object.
 * ccSocket       - Descriptor of the socket, bound to the local address 'addr'
 * pSocket        - Pointer to the 'ccSocket'. Indicates if the socket was
 *                  allocated.
 * ccSelectFd     - Structure, used by Select Engine for socket event handling.
 * pMgr           - Pointer to the Transport Manager object.
 * addr           - Structure, containing IP, port and type of the protocol,
 *                  served by the local address.
 * strLocalAddress- the local address in string form,
 *                  (IPv6 :with zero conversion and no scope ID).
 * bVacant        - RV_TRUE, if the address is in use. Socket is active.
 *                  RV_FALSE, if the address is not in use. Socket is closed.
 * hLock          - Defends pass between vacant and not vacant states and
 *                  state validation.
 * hConnListening - the handle of the TCP connection, which was created
 *                  in order to listen for incoming TCP connections
 *                  on this local address. NULL, if there is no connection.
 * hSentByPage    - Each local address can be associated with a sent-by string.
 *                  When the local address is used for outgoing request, this string
 *                  will be set in the sent by parameter of the top via header.
 * strSentByHost  - The offset of the sent-by string on the page.
 * sentByPort     - The port of the sent-by or UNDEFINED if there should not be a port.
 * strSentBy      - pointer to the position in the page where the string is held.
 * eSockAddrType  - Type of addresses, supported by the socket (IPv4 / IPv6)
 * pSecOwner      - Pointer to the Security Object that uses this Local Address
 * secOwnerCounter- Counter of Security Owners, using the local address.
 *                  It can be few owners despite that only one is refrenced by
 *                  pSecOwner. Such desicion was made to enable renegotiation
 *                  of IPsec while using same Port-S
 * impi           - Pointer to the IMPI, set into the Local Address by
 *                  Application through Security Agreement-Security Object API.
 *                  IMPI - IP Multimedia Private Identity, as it is defined in
 *                  TG 33.208.
 */
typedef struct
{
	RvSipTransportLocalAddrAppHandle hAppLocalAddr;
    RvSocket						 ccSocket;
    RvSocket*						 pSocket;
    RvSelectFd						 ccSelectFd;
    TransportMgr*					 pMgr;
    SipTransportAddress				 addr;
    RvChar							 strLocalAddress[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvBool							 bVacant;
    RvMutex							 hLock;
    RvSipTransportConnectionHandle   hConnListening;
	HPAGE                            hSentByPage;
    RvInt32                          sentByHostOffset;
	RvInt32                          sentByPort;
#ifdef SIP_DEBUG
	RvChar*                          strSentByHost;
#endif
    RvSipTransportAddressType        eSockAddrType;
#ifdef RV_SIP_IMS_ON
    void*                            pSecOwner;
    RvUint32                         secOwnerCounter;
    void*                            impi;
#endif

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
  RvUint8             tos;
  RvChar if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
  RvUint16 vdevblock;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

} TransportMgrLocalAddr;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * TransportMgrInitialize
 * ----------------------------------------------------------------------------------
 * General: initialize the transport manager object.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   TransportMgr        - manager object
 *          SipTransportCfg     - manager configuration.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrInitialize(
                                   IN    TransportMgr*     pTransportMgr,
                                   INOUT SipTransportCfg*  pTransportCfg);

/************************************************************************************
 * TransportMgrDestruct
 * ----------------------------------------------------------------------------------
 * General: Destruct the transport manager.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - Pointer to the transport manager
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrDestruct(IN TransportMgr* pTransportMgr);


/************************************************************************************
 * TransportMgrAllocateRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Allocates receive buffer for Message Received event
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   transportInfo    - transport info instance
 *            forRead            - flag that notifies that allocation is
 *                              for reading from socket
 * Output:  buff            - allocated buffer
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrAllocateRcvBuffer(IN  TransportMgr    *transportInfo,
                                                   IN  RvBool        forRead,
                                                   OUT RvChar        **buff);

/************************************************************************************
 * TransportMgrFreeRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Frees allocated receive buffer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   transportInfo    - transport info instance
 *            buff            - allocated buffer
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrFreeRcvBuffer(IN  TransportMgr    *transportInfo,
                                               IN  RvChar        *buff);

/************************************************************************************
 * TransportMgrGetDefaultLocalAddress
 * ----------------------------------------------------------------------------------
 * General: returns the handle to a local address according to address
 *          type and transport.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to transport manager
 *          eTransportType      - Transport type (udp/tcp)
 *          eAddressType        - address type (Ipv6/Ipv4)
 * Output:phLocalAddress - The local address handle
 ***********************************************************************************/
void RVCALLCONV TransportMgrGetDefaultLocalAddress(
                            IN  TransportMgr                *pTransportMgr,
                            IN  RvSipTransport              eTransportType,
                            IN  RvSipTransportAddressType   eAddressType,
                            OUT RvSipTransportLocalAddrHandle *phLocalAddress);

/************************************************************************************
 * TransportMgrFindLocalAddressHandle
 * ----------------------------------------------------------------------------------
 * General: returns the handle of a local address from the local address list
 *          that is equal to the supplied local address.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to transport manager.
 *          pLocalAddr      - local address structure pointer.
 * Output:  phLocalAddress  - Handle to an equal local address.
 ***********************************************************************************/
void RVCALLCONV TransportMgrFindLocalAddressHandle(
                            IN  TransportMgr                *pTransportMgr,
                            IN  TransportMgrLocalAddr       *pLocalAddr,
                            OUT RvSipTransportLocalAddrHandle *phLocalAddress);



/************************************************************************************
 * TransportMgrInitLocalAddressStructure
 * ----------------------------------------------------------------------------------
 * General: Initialize a local address structure based on the given parameters.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr           - pointer to transport manager
 *          eCoreTransportType      - Transport type (udp/tcp)
 *          eCoreAddressType        - address type (Ipv6/Ipv4)
 *          strLocalIp              - local IP as a string
 *          localPort               - local port
 *          if_name                 - local interface name
 *          vdevblock                  - local hardware port or else
 * Output:phLocalAddress - The local address handle
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrInitLocalAddressStructure(
                            IN TransportMgr*              pTransportMgr,
                            IN  RvSipTransport            eTransport,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar*                   strLocalIp,
                            IN  RvUint16                  localPort,
#if defined(UPDATED_BY_SPIRENT)
                            IN  RvChar*                   if_name,
			    IN  RvUint16                  vdevblock,
#endif
                            OUT TransportMgrLocalAddr*    pLocalAddr);

/************************************************************************************
 * TransportMgrGetLocalAddressByHandle
 * ----------------------------------------------------------------------------------
 * General: returns the string Ip and port for a given local address handle.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - Pointer to transport manager
 *          pLocalAddr       - Local address structure
 *          bConvertZeroToDefaultHost - Specifies whether to convert a zero IP to
 *                                      the default host.
 *          bAddScopeIp - Specifies whether to add the scope id to the end of the
 *                         string ip.
 * Output   localIp          - string IP of this local address
 *          localPort        - port of this local address
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetIPandPortStrings(
                            IN  TransportMgr*           pTransportMgr,
                            IN  RvAddress*              pAddress,
                            IN  RvBool                 bConvertZeroToDefaultHost,
                            IN  RvBool                 bAddScopeIp,
                            OUT RvChar                 *localIp,
                            OUT RvUint16               *localPort);

/************************************************************************************
 * TransportMgrConvertString2Address
 * ----------------------------------------------------------------------------------
 * General: make RV_LI_AddressType from string
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 *          bIsIpV4   - Defines if the address is ip4 or ip6
 *          bConstructAddr - the received address needs to be constructed.
 * Output:  address - the address in RV_LI_AddressType format
 ***********************************************************************************/
RvStatus TransportMgrConvertString2Address(IN  RvChar*                StringIp,
                                            OUT SipTransportAddress*    address,
                                            IN  RvBool                 bIsIpV4,
                                            IN  RvBool                 bConstructAddr);

/******************************************************************************
 * TransportMgrConvertTransportAddr2RvAddress
 * ----------------------------------------------------------------------------
 * General: construct the RvAddress object and initialize it with the address
 *          details, contained in the RvSipTransportAddr structure.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager object.
 *          pAddressDetails - details of the address to be condtructed.
 * Output:  pRvAddress      - address to be constructed.
 *****************************************************************************/
RvStatus TransportMgrConvertTransportAddr2RvAddress(
                                      IN  TransportMgr*       pTransportMgr,
                                      IN  RvSipTransportAddr  *pAddressDetails,
                                      OUT RvAddress           *pRvAddress);

/******************************************************************************
 * TransportMgrConvertRvAddress2TransportAddr
 * ----------------------------------------------------------------------------
 * General: extracts data from RvAddress object and
 *          put it into the RvSipTransportAddr structure.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager object.
 *          pRvAddress      - common core address.
 * Output:  pAddressDetails - details of the address.
 *****************************************************************************/
void TransportMgrConvertRvAddress2TransportAddr(
                                      IN  RvAddress          *pRvAddress,
                                      OUT RvSipTransportAddr *pAddressDetails);

/***************************************************************************
* TransportMgrSipAddressIsEqual
* ------------------------------------------------------------------------
* General: Compares to LI addresses
* Return Value: TRUE if the addresses are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     firstAddr  - first address
*           secondAddr  - second address
***************************************************************************/
RvBool TransportMgrSipAddressIsEqual(IN SipTransportAddress*  firstAddr,
                                     IN SipTransportAddress*  secondAddr);

/***************************************************************************
* TransportMgrRvAddressIsEqual
* ------------------------------------------------------------------------
* General:      Compares Common Core addresses in context of SIP:
*               their IPs and ports.
* Return Value: TRUE if the addresses are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input: firstAddr   - the first address.
*        secondAddr  - an another address.
***************************************************************************/
RvBool TransportMgrRvAddressIsEqual(IN RvAddress*  firstAddr,
                                    IN RvAddress*  secondAddr);

/***************************************************************************
* TransportMgrRvAddressPortAreEqual
* ------------------------------------------------------------------------
* General:      Compares ports of Common Core addresses.
*               The addresses can be of different IPv4/IPv6 types.
* Return Value: TRUE if the ports are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input: firstAddr   - the first address.
*        secondAddr  - an another address.
***************************************************************************/
RvBool TransportMgrRvAddressPortAreEqual(IN RvAddress*  firstAddr,
                                         IN RvAddress*  secondAddr);

/************************************************************************************
 * TransportMgrConvertCoreAddrTypeToSipAddrType
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
RvSipTransportAddressType TransportMgrConvertCoreAddrTypeToSipAddrType(
                                IN RvInt eCoreAddressType);

/******************************************************************************
 * TransportMgrSipAddrGetDetails
 * ----------------------------------------------------------------------------
 * General: auxiliary function, that fills RvSipTransportAddr structure with
 *          data, extracted from SipTransportAddress structure.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSipAddr - pointer to the SipTransportAddress structure.
 * Output:  pAddrDetails - pointer to the RvSipTransportAddr structure.
 *****************************************************************************/
void TransportMgrSipAddrGetDetails( IN  SipTransportAddress *pSipAddr,
                                    OUT RvSipTransportAddr  *pAddrDetails);

/************************************************************************************
 * TransportMgrSetSocketDefaultAttrb
 * ----------------------------------------------------------------------------------
 * General: Set Default attributes to a TCP socket:
 *          TCP:
 *               Non blocking
 *               NO Linger
 *          All:
 *               Reuse address
 *               Buffer Size
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pSocket - Pointer to the relevant socket.
 *          pMgr     - transport mgr
 *          eProt    - core protocol
 * Output:
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrSetSocketDefaultAttrb(IN RvSocket*       pSocket,
                                                     IN TransportMgr*   pMgr,
                                                     IN RvSocketProtocol eProt);

/************************************************************************************
 * TransportMgrProccesEvents
 * ----------------------------------------------------------------------------------
 * General: calls select
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr        - pointer to transport manager
 *          msMaxTimeout         - timeout for select to exit. if 0, will wait indefinitely
 * Output:  -
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrProccesEvents(IN RvSipTransportMgrHandle  hTransportMgr,
                                               IN RvUint32                msMaxTimeout);

/******************************************************************************
 * TransportMgrLocalAddressAdd
 * ----------------------------------------------------------------------------
 * General: add local address the manager, open it for listening and sending
 *
 * Return Value: RvStatus - RV_OK, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 *          eLocationInList - indication, where the new address should be placed
 *                            in the list of local addresses.
 *          hBaseLocalAddr  - existing address in the list, relative to which
 *                            the new addresses can be added.
 *                            The parameter is meaningless, if 'eLocationInList'
 *                            is not set to RVSIP_PREV_ELEMENT or RVSIP_NEXT_ELEMENT
 *          bConvertZeroPort- If RV_TRUE, and pAddressDetails contains zero
 *                            port, the port will be set to well known SIP port
 *                            according the Transport contained in
 *                            pAddressDetails.
 *          bDontOpen       - if RV_TRUE, no socket will be opened,
 *                            for TCP/TLS/SCTP address - no listening
 *                            connection will be constructed.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr              *pAddressDetails,
                        IN  RvUint32                        addrStructSize,
                        IN  RvSipListLocation               eLocationInList,
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        IN  RvBool                          bConvertZeroPort,
                        IN  RvBool                          bDontOpen,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddressRemove
 * ----------------------------------------------------------------------------
 * General: remove local address from manager database, close it's socket
 *
 * Return Value: RvStatus - RV_OK, error code otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the Transport Manager
 *          hLocalAddr      - handle to the address to be removed
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressRemove(
                            IN  RvSipTransportMgrHandle         hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: returns details of the Local Address, the handle to which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     -  handle of the Local Address.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        OUT RvSipTransportAddr              *pAddressDetails);

/******************************************************************************
 * TransportMgrLocalAddressGetFirst
 * ------------------------------------------------------------------------
 * General: gets handle of the Local Address, which is located at the head
 *          of the List of Local Addresses of the requested Transport Protocol
 *          type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          eTransportType  - type of the Transport Protocol.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetFirst(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransport                  eTransportType,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddressGetNext
 * ------------------------------------------------------------------------
 * General: gets handle of the Local Address, located in the List of Local
 *          addresses next to the address, handle of which is supplied
 *          as a parameter to the function.
 *          type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBaseLocalAddr  - handle to the Local Address, next of which is
 *                            requested.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetNext(
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddrLock
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and locks it.
 * Return Value: RV_OK on success, error code if check was failed
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddrLock(IN  TransportMgrLocalAddr *pLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddrUnlock
 * ----------------------------------------------------------------------------
 * General: Unlocks Local Address.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
 *****************************************************************************/
void RVCALLCONV TransportMgrLocalAddrUnlock(IN  TransportMgrLocalAddr *pLocalAddr);

/******************************************************************************
 * TransportMgrLocalAddrLockRelease
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and release it lock
 *          (removes all nested locks).
 * Return Value: RV_OK on success, error code if check was failed
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLocalAddr        - pointer to the Local Address object.
 * Output:  numOfnestedLocks  - number of nested locks.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddrLockRelease(
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                OUT RvInt32                 *pNumOfNestedLocks);

/******************************************************************************
 * TransportMgrLocalAddrLockRestore
 * ----------------------------------------------------------------------------
 * General: Performs check for Local Address validity and locks it lock
 *          numOfNestedLock times.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLocalAddr        - pointer to the Local Address object.
 *          numOfnestedLocks  - number of nested locks.
 *****************************************************************************/
void RVCALLCONV TransportMgrLocalAddrLockRestore(
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  RvInt32                 numOfNestedLocks);

/******************************************************************************
 * TransportMgrCounterLocalAddrsGet
 * ----------------------------------------------------------------------------
 * General: Returns counter of the opened addresses of the required type.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - the Transport Manager.
 *            eTransportType- type of Transport Protocol.
 * Output:    pV4Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *            pV6Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *****************************************************************************/
void RVCALLCONV TransportMgrCounterLocalAddrsGet(
                                    IN  TransportMgr           *pTransportMgr,
                                    IN  RvSipTransport          eTransportType,
                                    OUT RvUint32               *pV4Counter,
                                    OUT RvUint32               *pV6Counter);

/************************************************************************************
 * TransportMgrGetDefaultHost
 * ----------------------------------------------------------------------------------
 * General: Get ip address of the default local host as received from the operating
 *          system. Can be used to get one of the local addresses of the system,
 *          when you opened a socket with IP 0. Openning a socket with ip 0 will
 *          cause listenning to all local IP addresses, including the one returned
 *          here.
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to transport instance
 *          eAddressType - indicates the class of address we want to get ipv4/ipv6
 *
 * Output:  pAddr - The default local address
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrGetDefaultHost(
                                        IN    TransportMgr*     pTransportMgr,
                                        IN    RvInt            eAddressType,
                                        INOUT RvAddress*        pAddr);

/************************************************************************************
 * TransportMgrSelectUpdateForAllUdpAddrs
 * ----------------------------------------------------------------------------------
 * General: performs a select update for all UDP listening sockets.
 *          This function is used to go in to OOR state (with event=0) and to get out 
 *          of OOR (with event=read).
 * Return Value: RvStatus - RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr - pointer to transport manager
 *          selectEvent - seelct event to update for all UDP addresses
 ***********************************************************************************/
RvStatus RVCALLCONV TransportMgrSelectUpdateForAllUdpAddrs(
                                        IN    TransportMgr*   pMgr,
                                        IN    RvSelectEvents  selectEvent);

/******************************************************************************
 * TransportMgrLocalAddressSetTypeOfService
 * ------------------------------------------------------------------------
 * General: Set the type of service (called also DiffServ Code Point)
 *          in decimal form for all IP packets, sent out from the local address
 *          (For 02/2004 the first 6 bits of the TOS byte in IP header
 *          can be used for TOS)
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address to be updated.
 *          typeOfService   - number, to be set as a TypeOfService.
 * Output : none.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressSetTypeOfService(
                                IN TransportMgr             *pTransportMgr,
                                IN TransportMgrLocalAddr    *pLocalAddr,
                                IN RvInt32                  typeOfService);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV TransportMgrLocalAddressSetSockSize(
							IN TransportMgr             *pTransportMgr,
							IN TransportMgrLocalAddr    *pLocalAddr,
							IN RvUint32                 sockSize);
#endif
//SPIRENT_END
/******************************************************************************
 * TransportMgrLocalAddressGetTypeOfService
 * ------------------------------------------------------------------------
 * General: Get type of service (called also DiffServ Code Point), which
 *          was set for the specified Local Address.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address to be updated.
 * Output : pTypeOfService  - pointer to the memory, where the TypeOfService
 *                            will be stored by the function.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetTypeOfService(
                                IN  TransportMgr             *pTransportMgr,
                                IN  TransportMgrLocalAddr    *pLocalAddr,
                                OUT RvInt32                  *pTypeOfService);


#ifdef RV_SIP_JSR32_SUPPORT
/******************************************************************************
 * TransportMgrLocalAddressSetSentBy
 * ------------------------------------------------------------------------
 * General: Sets a sent-by host and port to the local address object. These values
 *          will be set in the sent-by parameter of the top most via header
 *          of outgoing requests.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr      - pointer to the local address
 *          strSentByHost   - Null terminated sent-by string.
 *          sentByPort - the sent by port, or UNDEFINED if the application does
 *                       not wish to set a port in the sent-by.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressSetSentBy(
										IN  TransportMgr*          pTransportMgr,
										IN  TransportMgrLocalAddr* pLocalAddr,
										IN  RvChar*                strSentByHost,
										IN  RvInt32                sentByPort);

#endif /*#ifdef RV_SIP_JSR32_SUPPORT*/

/******************************************************************************
 *TransportMgrLocalAddressGetConnection
 * ------------------------------------------------------------------------
 * General: Gets the handle of the listening Connection object,
 *          bound to the Local Address object of TCP/TLS/SCTP type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport Manager.
 *          pLocalAddr - handle to the local address to be updated
 * Output : phConn     - pointer to the memory, where the handle will be stored
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressGetConnection(
                            IN  TransportMgr                   *pTransportMgr,
                            IN  TransportMgrLocalAddr          *pLocalAddr,
                            OUT RvSipTransportConnectionHandle *phConn);



/******************************************************************************
 * TransportMgrAddLocalAddressToList
 * ----------------------------------------------------------------------------
 * General:      Insert the new Local Address into the List of Local Addresses 
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager.
 *          pAddressDetails - pointer to the memory, where the details of the
 *                            address to be add are stored.
 *          eLocationInList - location of the new address in the list.
 *          pBaseLocalAddr  - pointer to the local address, relatively to which
 *                            the new address should be inserted
 *          bConvertZeroPort- If RV_TRUE, and pAddressDetails contains zero
 *                            port, the port will be set to well known SIP port
 *                            according the Transport contained in
 *                            pAddressDetails.
 * Output:  ppLocalAddr     - pointer to the memory, where the pointer to
 *                            the local address (handle) will be returned.
 *          pPort           - pointer to the memory, where the port, actually
 *                            used while local address opening,will be returned
 *                            The actually set port may differ from the one,
 *                            supplied with pAddressDetails structure.
 *                            If the supplied port was 0, the default one will
 *                            be set.
 *****************************************************************************/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV TransportMgrAddLocalAddressToList(
                                IN  TransportMgr            *pTransportMgr,
                                IN  RvSipTransportAddr      *pAddressDetails,
                                IN  RvSipListLocation       eLocationInList,
                                IN  TransportMgrLocalAddr   *pBaseLocalAddr,
                                IN  RvBool                  bConvertZeroPort,
                                OUT TransportMgrLocalAddr   **ppLocalAddr,
                                OUT RvUint16                *pPort,
                                RV_TOSDiffserv              priority);
#else
RvStatus RVCALLCONV TransportMgrAddLocalAddressToList(
                                IN  TransportMgr            *pTransportMgr,
                                IN  RvSipTransportAddr      *pAddressDetails,
                                IN  RvSipListLocation       eLocationInList,
                                IN  TransportMgrLocalAddr   *pBaseLocalAddr,
                                IN  RvBool                  bConvertZeroPort,
                                OUT TransportMgrLocalAddr   **ppLocalAddr,
                                OUT RvUint16                *pPort);
#endif
/* SPIRENT_END */

/******************************************************************************
 * TransportMgrLocalAddressCloseSocket
 * ----------------------------------------------------------------------------
 * General: closes underlying socket of the Local Address.
 * Return Value: RV_OK on success, error code otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to the Local Address.
 *****************************************************************************/
RvStatus RVCALLCONV TransportMgrLocalAddressCloseSocket(
                                        IN  TransportMgrLocalAddr *pLocalAddr);
/******************************************************************************
 * TransportMgrLocalAddrListLock
 * ----------------------------------------------------------------------------
 * General: locks supplied List of Local Addresses.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the Transport Manager.
 *            hListOfLocalAddr - handle of the List of Local Addresses
 *****************************************************************************/
void TransportMgrLocalAddrListLock(IN  TransportMgr    *pTransportMgr);

/******************************************************************************
 * TransportMgrLocalAddrListUnlock
 * ----------------------------------------------------------------------------
 * General: unlocks supplied List of Local Addresses.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the Transport Manager.
 *            hListOfLocalAddr - handle of the List of Local Addresses
 *****************************************************************************/
void TransportMgrLocalAddrListUnlock(IN  TransportMgr    *pTransportMgr);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
RvStatus RVCALLCONV TransportMgrPrintLocalAddresses(
                                IN TransportMgr *pTransportMgr);

void TransportMgrLogLocalAddressError(
                                IN TransportMgr          *pTransportMgr,
                                IN TransportMgrLocalAddr *pLocalAddr,
                                IN RvInt32               errCode,
                                IN RvChar                *strLogLine);

void TransportMgrLogLocalAddressWarning(IN TransportMgr *pTransportMgr,
										IN TransportMgrLocalAddr *pLocalAddr,
										IN RvInt32 errCode,
										IN RvChar  *strLogLine);


void TransportMgrLogLocalAddressInfo(
                            IN TransportMgr *pTransportMgr,
                            IN TransportMgrLocalAddr *pLocalAddr,
                            IN RvChar *strLogLine);

#else /* ELSE OF: #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
#define TransportMgrPrintLocalAddresses(_tm)
#define TransportMgrLogLocalAddressError(_tm,_addr,_err,_logLine)
#define TransportMgrLogLocalAddressWarning(_tm,_addr,_err,_logLine)
#define TransportMgrLogLocalAddressInfo(_tm,_addr,_logLine)
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/***************************************************************************
 * TransportMgrCheckConnCapacity
 * ------------------------------------------------------------------------
 * General: Check if the connections capacity reached the maximum we want.
 *          If so, we close the least-recently-used connection.
 *          (The first connection in the not-in-use list).
 *          If there is no connection in the list, so we are in a 
 *          out-of-resource condition. nothing to do here.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Output:     pMgr - pointer to the transport manager.
 ***************************************************************************/
void RVCALLCONV TransportMgrCheckConnCapacity(TransportMgr   *pMgr);

#if(RV_NET_TYPE & RV_NET_IPV6)
/******************************************************************************
 * TransportMgrAddrCopyScopeId
 * ----------------------------------------------------------------------------
 * General: Sets Scope ID of the Source address into the Destination Address.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSrcAddr - pointer to the address - source of Scope ID
 *          pDstAddr - pointer to the address to be updated
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV TransportMgrAddrCopyScopeId(
                                        IN  RvAddress* pSrcAddr,
                                        OUT RvAddress* pDstAddr);
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6)*/

#if(RV_NET_TYPE & RV_NET_IPV6)
/******************************************************************************
* TransportMgrConvertIPv4RvAddressToIPv6
* -----------------------------------------------------------------------------
* General:      Converts IPv4 address to IPv6 form.
* Return Value: RV_OK, if the operation was performed, error code - otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input: pAddr   - the first address.
******************************************************************************/
RvStatus TransportMgrConvertIPv4RvAddressToIPv6(IN RvAddress*  pAddr);
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6)*/

/******************************************************************************
* TransportMgrSocketDestruct
* -----------------------------------------------------------------------------
* General:      destructs Common Core object RvSocket and correspondent
*               File Descriptor object, set into the Select Engine.
*.
* Return Value: RV_OK, if the operation was performed, error code - otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input: pSocket         - socket object to be destrcuted.
*        bCleanSocket    - if RV_TRUE, the underlying socket will be cleaned
*                          from received data.
*        pSelectFd       - file descriptor object to be destrcuted.Can be NULL.
******************************************************************************/
RvStatus TransportMgrSocketDestruct(
                                IN TransportMgr* pTransportMgr,
                                IN RvSocket*     pSocket,
                                IN RvBool        bCleanSocket,
                                IN RvSelectFd*   pSelectFd);

#ifdef __cplusplus
}
#endif

#endif /*TRANSPORT_MGR_OBJECT_H*/

