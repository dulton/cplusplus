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
 *                              <TransmitterObject.c>
 *
 * This file defines the Transmitter object, attributes and interface functions.
 * Transmitter represents a SIP Transmitter as defined in RFC 2543.
 * This means that a SIpTrx is defined using it's Call-Id, From and To
 * fields.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef TRANSMITTER_OBJECT_H
#define TRANSMITTER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransportDNSTypes.h"
#include "_SipTransportTypes.h"
#include "_SipCommonTypes.h"
#include "TransmitterMgrObject.h"
#include "_SipResolverTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecurityTypes.h"
#endif
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* TransmitterResolutionState
 *-----------------------------
 * The TransmitterResolutionState enumeration used by the transmitter to 
 * follow it resolution status
 * UNRESOLVED           - resolution was requested but nothing has happened yet
 * RESOLUTION_STARTED   - Resolution has started
 * WAITING_FOR_URI      - Trying to identify URI
 * RESOLVING_URI_TRANSPORT_NAPTR    - Trying to identify transport by NAPTR
 * RESOLVING_URI_TRANSPORT_3WAY_SRV - Trying to identify transport by applying
 *                                    SRV queries
 * TRANSPORT_FOUND                  - The transport was found
 * RESOLVING_URI_HOSTPORT           - Trying to identify the host and the port
 * RESOLVING_IP         - Trying to identify IP (A/AAAA)
 * RESOLVED             - resolution is complete
 */

typedef enum {
    TRANSMITTER_RESOLUTION_STATE_UNDEFINED = -1,
    TRANSMITTER_RESOLUTION_STATE_UNRESOLVED,
    TRANSMITTER_RESOLUTION_STATE_RESOLUTION_STARTED,
    TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI,
    TRANSMITTER_RESOLUTION_STATE_URI_FOUND,
    TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR,
    TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_3WAY_SRV,
    TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND,
    TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT,
    TRANSMITTER_RESOLUTION_STATE_RESOLVING_IP,
    TRANSMITTER_RESOLUTION_STATE_RESOLVED
}TransmitterResolutionState;

typedef enum {
    TRANSMITTER_MSG_UNDEFINED = -1,
    TRANSMITTER_MSG_FIRST_ROUTE,
    TRANSMITTER_MSG_REQ_URI,
    TRANSMITTER_MSG_OUTBOUND_ADDR
}TransmitterMsgAddrUsed;

/* TransportDnsMsgParams
 *---------------------------
 * The TransportDnsMsgParams structure keeps temporary parameters,
 * needed to proceed with asynchronous DNS query.
 * The TransportDnsMsgParams structure is used as part of the
 * Transmitter object
 * msgType       - used to distinguish SIP client and SIP server
 * strHostName   - host name as specified in the message
 * port          - port number as specified in the message
 * transportType - transport type as specified in the message
 * bIsSecure     - should TLS being considered
 */
typedef struct {
    RvSipMsgType              msgType;
    RvChar*                   strHostName;
    RvInt32                   hostNameOffset;
    RvInt32                   port;
    RvSipTransport            transportType;
    RvBool                    bIsSecure;
} TransportDnsMsgParams;


/* Transmitter
 * --------------------------------------------------------------------------
 * The Transmitter object is used for sending a single sip messages on
 * behalf of the application or one or more sip messages on behalf of
 * the transaction.
 * When a transmitter belongs to a transaction they share a lock.
 * Application transmitter or a transmitter that does not belong to a transaction
 * has it own lock.
 * The transmitter contains all addressing information needed for sending the
 * message such as the outbound proxy address and is responsible for the
 * entire address resolution process.
 *
 * hAppTrx          -- Application handle to the transmitter.
 * terminationInfo  -- termination info, that helps to process out of resources when
 *                     sending object termination event
 * pTrxMgr          -- pointer the the manager this Transmitter belongs to.
 * trxUniqueIdentifier - A unique identifier for this Transmitter (randomly generated
 *                     when the Transmitter is created.
 * evHandlers       --  Transmitter event handler structure
 * eState           -- The transmitter current state.
 * localAddr        -- The transmitter local addresses information.
 * pTripleLock      -- Pointer to the transmitter triple lock that consists of object lock, API lock & processing lock
 * trxTripleLock    -- Triple lock structure of the transmitter.
 * outboundAddr     -- Information of the transmitter outbound details
 * outboundProxyHostNameOffset -- if the outbound proxy is a host name it is kept on the transmitters page
 * bIsPersistent    -- Indicates if the transmitter will try to use persistent connections.
 * bKeepMsg         -- Indicates that the message will be destructed only when the transmitter terminates.
 * hConn            -- The connection used by the transmitter.
 * bSigCompEnabled      -- Indication if the application chose to stop the SigComp processing.
 * hSigCompCompartment  -- Handle to the SigComp compartment that is related to the Transmitter.
 * hSigCompOutboundMsg  -- The last message sent, in sigComp format.
 * eOutboundMsgCompType -- Indicates what is the type of the outbound sent message.
 * eNextMsgCompType     --  The type of the next sent message that might be different than the 
 *                          current outbound message type.
 * sigcompIdOffset  -- The offset of the sigcomp-id string inside the page
 * bCopyAppMsg      -- indicates whether to copy the message of application transmitter.
 *                     The default is to copy the message.
 * hPage            -- A page used by the transmitter to save data such as the DNS list.
 * hDnsList         -- The DNS list that is currently used by the transmitter.
 * strQuery         -- a consecutive buffer wit the string moved to the query
 * destAddr         -- A destination address to use for sending the message.
 * bIgnoreOutboundProxy -- indicates that the transmitter should ignore any outbound proxy configured to it
 *                     or to the stack
 * bSendToFirstRoute - Indicates that the next message should be sent to the first route and
 *                     not according to the request URI.
 * bHandleTopVia    -- Indicate whether the transmitter should add a top Via header to request messages and
 *                     remove the top via from response messages.
 * bTrxCreatedVia    -- Indicates whether the transmitter created the via. If this case it needs
 *                      to update it before moving to the the final dest resolved state.
 *                      If the application changes the remote address in the callback and wants
 *                      the transmitter to fix the via it should set the fixVia to RV_TRUE.
 * bFixVia          -- The application indicated that it wishes the transmitter to always
 *                     update the via header before the message is sent according to the local 
 *                     and remote address and transport types. 
 * bSkipViaProcessing -- if this parameter is set to RV_TRUE, the transmitter will not perform any activity that
 *                     relates to the via header.
 * viaBranchOffset  -- offset to a via branch supplied by the application that the transmitter will
 *                     set in the message if the bHandleTopVia is RV_TRUE
 * bIsAppTrx        -- The transmitter belongs to the application.This means that
 *                     it will copy messages before sending and will not be used
 *                     for more then one message.
 * eProtoDiscoveryState - state of the transport protocol discovery process
 * ePortIPDiscoveryState - state of the IP address & port number discovery process
 * bAddrManuallySet -- did the app manually set set the dest address
 * hMsgToSend       -- The message handle the transmitter is about to send, received from upper layers.
 * hNextMessageHop  -- the next hop as taken from the message
 * strViaBranch     -- the via branch string located on the transmitter page.
 * hTcpBuffPage     -- The page allocated when Ares asks for TCP allocation
 * hQueryBuffPage   -- The page allocated when Ares asks for query allocation
 * bMoveToMsgSendFailure -- indicates that the transmitter should move to the msg send failure state
 *                          after it notified the final dest resolved state.
 * bConnWasSetByApp -- Was the connection set in transmitter object by application, or was it 
 *                     set because we found it in the hash (connection reuse).
 * bSendByAlias     -- TRUE if the trx found a connection by alias, that can be used.
 * hSecObj          -- Handle to the Security Object, protecting Transmitter's messages
 * eMsgAddrUsedForSending -- indicate where to actual address for sending was taken from.
 *                      We use this indication to know which part of the message to fix from
 *                      im:/pres: to sip:
 * hEncodedOutboundMsg -- The Encoded SIP message (encoded hMsgToSend).
 * hSentOutboundMsg    -- pointing to the real message the Trx is about to send. This is used as a
 *                        single handle the Trx API will work with. If the message is compressed,
 *                        this handle will point to hSigCompOutboundMsg. Else, it will point to 
 *                        hEncodedOutboundMsg.
 */
typedef struct
{
    RvSipAppTransmitterHandle        hAppTrx;
    RvSipTransportObjEventInfo       terminationInfo;
    TransmitterMgr*                  pTrxMgr;
    RvInt32                          trxUniqueIdentifier;
    RvSipTransmitterEvHandlers       evHandlers;
    RvSipTransmitterState            eState;
    SipTransportObjLocalAddresses    localAddr;
    SipTripleLock*                   pTripleLock;
    SipTripleLock                    trxTripleLock;
    SipTransportOutboundAddress      outboundAddr;
    RvInt32                          outboundProxyHostNameOffset;
    RvBool                           bIsPersistent;
    RvBool                           bKeepMsg;
    RvSipTransportConnectionHandle   hConn;
#ifdef RV_SIGCOMP_ON
    RvBool                           bSigCompEnabled;
    RvSipCompartmentHandle           hSigCompCompartment;  
    HPAGE                            hSigCompOutboundMsg; 
    RvSipTransmitterMsgCompType      eOutboundMsgCompType;
    RvSipTransmitterMsgCompType      eNextMsgCompType;
	RvInt32                          sigcompIdOffset;
#endif /*#ifdef RV_SIGCOMP_ON*/
    RvBool                           bCopyAppMsg;
    HPAGE                            hPage;
    HRPOOL                           hPool;    
    RvSipTransportDNSListHandle      hDnsList;
    RvChar*                          strQuery;
    SipTransportAddress              destAddr;
    RvBool                           bIgnoreOutboundProxy;
    RvBool                           bSendToFirstRoute;
    RvBool                           bTrxCreatedVia;
    RvBool                           bFixVia;
    RvBool                           bSkipViaProcessing;
    RvInt32                          viaBranchOffset;
    RvBool                           bIsAppTrx;
    TransmitterResolutionState       eResolutionState;
    RvSipResolverHandle              hRslv;
    RvBool                           bAddrManuallySet;
    RvSipMsgHandle                   hMsgToSend;
    RvSipAddressHandle               hNextMessageHop;
    TransportDnsMsgParams            msgParams;
        
    /* new params */
    HPAGE                            hSentOutboundMsg;
    HPAGE                            hEncodedOutboundMsg;

    RvUint32                         encodedMsgLen;
    SipTransportMsgType              eMsgType;
    RLIST_HANDLE                     hMsgToSendList;
    RvBool                           bMoveToMsgSendFailure;
    RvBool                           bForceOutboundAddr;
    RvBool                           bFreePageOnTermination;
    RvBool                           bConnWasSetByApp;
	RvBool                           bSendingBuffer;
    RvBool                           bSendByAlias;

#ifdef SIP_DEBUG
    RvChar*                          strViaBranch;
#endif /*ifdef SIP_DEBUG*/    

#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpMsgSendingParams sctpParams;
#endif
#ifdef RV_SIP_IMS_ON
    RvSipSecObjHandle                hSecObj;
    RvSipTransportConnectionHandle   hSecureConn;
#endif

#ifdef RV_SIP_OTHER_URI_SUPPORT
    TransmitterMsgAddrUsed eMsgAddrUsedForSending;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
    
} Transmitter;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* TransmitterFreeEncodedOutboundMsg
* ------------------------------------------------------------------------
* General:Free the outbound messages that are related to the Transmitter.
*         The Transmitter can own both hEncodedOutboundMsg and
*         hSigCompOutboundMsg
* Return Value: -.
* ------------------------------------------------------------------------
* Arguments:
* input: pTrx- The Transmitter handle.
***************************************************************************/
void RVCALLCONV TransmitterFreeEncodedOutboundMsg(IN Transmitter* pTrx);

/***************************************************************************
 * TransmitterInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new transmitter in the Idle state.
 *          If a page is supplied the transmitter will use it. Otherwise
 *          a new page will be allocated.
 *          Note: When calling this function the transmitter manager is locked.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the new Transmitter
 *            bIsAppTrx - indicates whether the transmitter was created
 *                        by the application or by the stack.
 *            hAppTrx   - Application handle to the transmitter.
 *            hPool     - A pool for this transmitter
 *            hPage     - A memory page to be used by this transmitter. If NULL
 *                        is supplied the transmitter will allocate a new page.
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterInitialize(
                             IN Transmitter*              pTrx,
                             IN RvBool                    bIsAppTrx,
                             IN RvSipAppTransmitterHandle hAppTrx,
                             IN HRPOOL                    hPool,
                             IN HPAGE                     hPage,
							 IN SipTripleLock*            pTripleLock);

/***************************************************************************
 * TransmitterSetTripleLock
 * ------------------------------------------------------------------------
 * General: Sets the transmitter triple lock.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx - The transmitter handle
 *           pTripleLock - The new triple lock
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV TransmitterSetTripleLock(
                         IN Transmitter*       pTrx,
                         IN SipTripleLock*     pTripleLock);


/************************************************************************************
 * TransmitterCopy
 * ----------------------------------------------------------------------------------
 * General: Copies relevant information from the source transmitter
 *          to the destination transmitter
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pDestTrx      - Destination transmitter
 *          pSrcTrx       - Source transmitter
 *          bCopyDnsList - Indicates whether to copy the DNS list or not.
 *          
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterCopy(
                     IN  Transmitter*    pDestTrx,
                     IN  Transmitter*    pSrcTrx,
                     IN  RvBool          bCopyDnsList);

/***************************************************************************
 * TransmitterTerminate
 * ------------------------------------------------------------------------
 * General: Moves the transmitter object to the terminated state and
 *          send the transmitter to the termination queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter to terminate.
 ***************************************************************************/
void RVCALLCONV TransmitterTerminate(
                                 IN  Transmitter    *pTrx);


/***************************************************************************
 * TransmitterDestruct
 * ------------------------------------------------------------------------
 * General: Free the Transmitter resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter to free.
 ***************************************************************************/
void RVCALLCONV TransmitterDestruct(IN  Transmitter    *pTrx);

/***************************************************************************
 * TransmitterChangeState
 * ------------------------------------------------------------------------
 * General: Change the Transmitter state and notify the application about it.
 *
 * Caution !!!
 *  TransmitterChangeState(...,RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED,...)
 *  should be called indirectly by call to
 *  TransmitterMoveToStateFinalDestResolved() only.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 *            hMsg      - A message related to this state
 *            pExtraInfo- Extra info related to this state.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV TransmitterChangeState(
                                 IN  Transmitter                      *pTrx,
                                 IN  RvSipTransmitterState             eNewState,
                                 IN  RvSipTransmitterReason            eReason,
                                 IN  RvSipMsgHandle                    hMsg,
                                 IN  void*                             pExtraInfo);

/***************************************************************************
 * TransmitterMoveToMsgSendFaiureIfNeeded
 * ------------------------------------------------------------------------
 * General: Change the transmitter state to msg send failure in
 *          one of two cases. The rv indicates a network error
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter.
 *            rv   - The status that caused us to call this function.
 *            eTrxReason - If the reason is undefined then the return value should be used.
 *            bReachedFromApp  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMoveToMsgSendFaiureIfNeeded(
                               IN   Transmitter*           pTrx,
                               IN   RvStatus               rv,
                               IN   RvSipTransmitterReason eTrxReason, 
                               IN   RvBool                 bReachedFromApp);

/******************************************************************************
 * TransmitterMoveToStateFinalDestResolved
 * ----------------------------------------------------------------------------
 * General: Performs some action before state change,
 *          after that move the Transmitter to the FINAL_DEST_RESOLVED state
 *          by call to TransmitterChangeState().
 * Return Value: RV_OK on success,
 *               error code, declared in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx      - Pointer to the Transmitter.
 *            eReason   - The state change reason.
 *            hMsg      - A message related to the state
 *            pExtraInfo- Extra info related to the state.
 * Output:  (-)
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterMoveToStateFinalDestResolved(
                            IN  Transmitter                      *pTrx,
                            IN  RvSipTransmitterReason            eReason,
                            IN  RvSipMsgHandle                    hMsg,
                            IN  void*                             pExtraInfo);

/******************************************************************************
 * TransmitterCreateConnectionIfNeeded
 * ----------------------------------------------------------------------------
 * General: Checks if the transmitter's current connection can be used and
 *          if not constructs a new connection. In this case NEW_CONN_IN_USE
 *          event will be reported.
 *          If the Secure Connection is provided, it will be used.
 *          If the Secure Connection is used the first time by the Transmitter,
 *          NEW_CONN_IN_USE will be reported.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx        - The transmitter pointer
 *          hSecureConn - Handle to the Secure Connection to be used
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterCreateConnectionIfNeeded(
                            IN Transmitter*                   pTrx,
                            IN RvSipTransportConnectionHandle hSecureConn);

/***************************************************************************
 * TransmitterMsgToSendListAddElement
 * ------------------------------------------------------------------------
 * General: Adds an element to the message to send list.
 * Return Value:void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx   - The transmitter pointer
 *            msgPage - The message to send page
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMsgToSendListAddElement(
                                       IN   Transmitter*          pTrx,
                                       IN   HPAGE                 hMsgPage);

/***************************************************************************
 * TransmitterMsgToSendListRemoveElement
 * ------------------------------------------------------------------------
 * General: Go over the transmitter sent list and free all the pages of messages
 *          that where sent before the current message
 *
 * Return Value:void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx   - The transmitter pointer
 *            hMsgPage - The message page
 ***************************************************************************/
void RVCALLCONV TransmitterMsgToSendListRemoveElement(
                                       IN   Transmitter*          pTrx,
                                       IN   HPAGE                 hMsgPage);

/***************************************************************************
 * TransmitterAttachToConnection
 * ------------------------------------------------------------------------
 * General: Attach the Transmitter as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx  - pointer to Transmitter
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus TransmitterAttachToConnection(
                                IN Transmitter                    *pTrx,
                                IN RvSipTransportConnectionHandle  hConn);

/***************************************************************************
 * TransmitterDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx    - pointer to Transmitter
 *            hConn   - the connection to detach from.
 ***************************************************************************/
void TransmitterDetachFromConnection(IN Transmitter                    *pTrx,
                                     IN RvSipTransportConnectionHandle  hConn);

/***************************************************************************
 * TransmitterDetachFromAres
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx    - pointer to Transmitter
 *            hConn   - the connection to detach from.
 ***************************************************************************/
void TransmitterDetachFromAres(IN Transmitter   *pTrx);

/***************************************************************************
 * TransmitterGetLocalAddressHandleByTypes
 * ------------------------------------------------------------------------
 * General: Get the local address handle according to a given transport
 *          and address type.
 * Return Value: RV_ERROR_NOT_FOUND - This type of address does not exist.
 *               RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction
 *          eTransportType - The transport type (udp or tcp).
 *          eAddressType - The address type (ip4 or ip6).
 * Output:  pphLocalAddr   - the address of the local address handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterGetLocalAddressHandleByTypes(
                            IN  Transmitter                 *pTrx,
                            IN  RvSipTransport              eTransportType,
                            IN  RvSipTransportAddressType   eAddressType,
                            OUT RvSipTransportLocalAddrHandle **pphLocalAddr);

/***************************************************************************
 * TransmitterSetLocalAddressHandleByTypes
 * ------------------------------------------------------------------------
 * General: Set the local address handle into the Transmitter's Local Addresses
 *          structure according to a given transport and address type.
 * Return Value: RV_OK on success, error code otherwise
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx           - Pointer to the transmitter
 *          eTransportType - The transport type (udp or tcp).
 *          eAddressType   - The address type (ip4 or ip6).
 *          hLocalAddr     - The local address handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterSetLocalAddressHandleByTypes(
                            IN  Transmitter*                  pTrx,
                            IN  RvSipTransport                eTransportType,
                            IN  RvSipTransportAddressType     eAddressType,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr);

/***************************************************************************
 * TransmitterSetLocalAddressHandle
 * ------------------------------------------------------------------------
 * General: Set a local address handle in the transmitter local address
 *          structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTrx - Pointer to the transmitter
 *          hLocalAddr - the local address handle
 *          bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV TransmitterSetLocalAddressHandle(
                            IN  Transmitter*                  pTrx,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            IN  RvBool                        bSetAddrInUse);



/************************************************************************************
 * TransmitterAddNewTopViaToMsg
 * ----------------------------------------------------------------------------------
 * General: Adds top via header to the message before address resolution and
 *          according to the transport of the request URI combined with the
 *          available local addresses. This is a best shot via header. The via
 *          header will be fixed after the final destination will be resolved.
 *          with the function UpdateViaByFinalLocalAddress.
 *          Note: this function will use the branch found in the transmitter or
 *          will generate a new branch.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - Handle to the transmitter
 *          hMsg      - A message to add a top via to.
 *          hSourceVia- if NULL, use values extracted from transport,
 *                      o/w - copy this value to the new via header
 *          bIgnorTrxBranch - If the transmitter needs to add a branch it should create
 *                            a new branch and ignore its own existing branch.
 *                            this is good for ack on 2xx.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterAddNewTopViaToMsg (
                     IN  Transmitter*           pTrx,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipViaHeaderHandle   hSourceVia,
                     IN  RvBool                 bIgnorTrxBranch);


/************************************************************************************
 * TransmitterSetBranch
 * ----------------------------------------------------------------------------------
 * General: Sets the branch parameter to the transmitter object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - Pointer to the transmitter
 *          strViaBranch  - The Via branch to add to the top via header.
 *                          This parameter is ignored for response messages or when
 *                          the handleTopVia parameter is RV_FALSE.
 *          pRpoolViaBranch - The branch supplied on a page. You should set this
 *                          parameter to NULL if the branch was supplied as a string.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterSetBranch(
                                  IN Transmitter*           pTrx,
                                  IN RvChar*                strViaBranch,
                                  IN RPOOL_Ptr*             pRpoolViaBranch);


/***************************************************************************
 * TransmitterGenerateBranchStr
 * ------------------------------------------------------------------------
 * General: Generate branch string to be used for branch.
 * Return Value:none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx       - pointer to the Transmitter.
 * Output:    strBranch  - a generated string that contains the random ID.
 *                       the string must be of size SIP_COMMON_ID_SIZE
 ***************************************************************************/
void RVCALLCONV TransmitterGenerateBranchStr(IN   Transmitter* pTrx,
                                             OUT  RvChar*      strBranch);


#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * TransmitterLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transmitter according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterLockAPI(
                                     IN  Transmitter *pTrx);


/************************************************************************************
 * TransmitterUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Release Transmitter according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
void RVCALLCONV TransmitterUnLockAPI(
                                 IN  Transmitter *pTrx);



/************************************************************************************
 * TransmitterUnLockEvent
 * ----------------------------------------------------------------------------------
 * General: UnLocks Transmitter according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
void RVCALLCONV TransmitterUnLockEvent(
                                   IN  Transmitter *pTrx);



#else
#define TransmitterLockAPI(a) (RV_OK)
#define TransmitterUnLockAPI(a)
#define TransmitterUnLockEvent(a)
#endif

/************************************************************************************
 * TransmitterLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Transmitter according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 *          NOTE: If the stack is compiled in a non multithreaded way, the transmitter 
 *          can still be destructed while it is wating for a DNS response.
 *          this is why in non multithreaded mode we still check that the transmitter 
 *          location is not vacant
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterLockEvent(IN  Transmitter *pTrx);

/*-----------------------------------------------------------------------
      TRANSMITTER:  G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
          N A M E    F U N C T I O N S
 -------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * TransmitterGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransmitterGetStateName (
                                      IN  RvSipTransmitterState  eState);

/***************************************************************************
 * TransmitterGetStateChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state change reason.
 * Return Value: The string with the state change reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The state change reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransmitterGetStateChangeReasonName (
                                IN  RvSipTransmitterReason  eReason);

const RvChar*  RVCALLCONV TransmitterGetResolutionStateName (
                                      IN  TransmitterResolutionState  eState);
#endif  /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */



#ifdef RV_SIGCOMP_ON

/***************************************************************************
 * TransmitterHandleSigCompCompartment
 * ------------------------------------------------------------------------
 * General: The function should be called when a SigComp-SIP message is
 *          sent. According to the SigComp model each SigComp-SIP
 *          message MUST be related to any SigComp compartment. Thus, the
 *          function verifies that the Transaction is currently associated
 *          with a SigComp Compartment. In case that the Trx instance is not
 *          related to any compartment, then new one is created for it.
 *
 * NOTE:    This function assumes that the transmitter has already resolved
 *          its destination address.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx       - A pointer to the Transmitter.
 *        hMsgToSend - Handle to the transmitted message.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterHandleSigCompCompartment(
                                    IN Transmitter     *pTrx,
                                    IN RvSipMsgHandle   hMsgToSend);

/***************************************************************************
* TransmitterFreeRedundantOutboundMsgIfNeeded
* ------------------------------------------------------------------------
* General: Frees the redundant encoded outbound message page in case that
*          only the SigComp outbound message page was sent. (The inverted
*          case is impossible because if the transaction message is
*          decompressed successfully the SigComp outbound message is created
*          and sent first). The redundant outbound message page can exists
*          only when the SigComp feature is enabled.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* input:   pTrx         - A pointer to the Transmitter.
***************************************************************************/
void RVCALLCONV TransmitterFreeRedundantOutboundMsgIfNeeded(
                                       IN Transmitter   *pTrx);

/***************************************************************************
 * TransmitterPrepareCompressedSentMsgPage
 * ------------------------------------------------------------------------
 * General: Prepare the outbound message that is going to be sent over
 *          the transaction, by compressing it.
 *          NOTE: The function assumes that the hEncodedOutboundMsg contains
 *                the valid data to be sent.
 * Return Value: RV_success/RV_ERROR_UNKNOWN
 * ------------------------------------------------------------------------
 * Arguments:
 * Input/Output:
 *      pTrx            - A pointer to the Transmitter.
 *     
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterPrepareCompressedSentMsgPage(
                                     IN  Transmitter  *pTrx);

/***************************************************************************
* TransmitterUseCompartment
* ------------------------------------------------------------------------
* General:Assign a transmitter with a compartment
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* input: pTrx- The Transmitter handle.
***************************************************************************/
RvStatus RVCALLCONV TransmitterUseCompartment(IN Transmitter *pTrx);

#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * TransmitterSetDestAddr
 * ------------------------------------------------------------------------
 * General: Sets the destination address the transmitter will use.
 *          Use this function when you want the transmitter to use
 *          a specific address regardless of the message content.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx              - The Transmitter handle
 *            pDestAddr         - destination addr
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterSetDestAddr(
                          IN Transmitter*                  pTrx,
                          IN SipTransportAddress*          pDestAddr,
                          IN RvSipTransmitterExtOptions*   pOptions);


/***************************************************************************
 * TransmitterSetInUseLocalAddrByDestAddr
 * ------------------------------------------------------------------------
 * General: Sets the inuse local address according to the destination address
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc   - the transaction,
 *          pDestAddr - the destination address.
 ***************************************************************************/
RvStatus TransmitterSetInUseLocalAddrByDestAddr(
                             IN Transmitter* pTrx);

/***************************************************************************
* TransmitterUpdateViaByFinalLocalAddress
* ------------------------------------------------------------------------
* General: Update the via header with the local address that is suitable for
*          the destination address.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input: pTrx             - The transmitter handle.
*        bFixVia          - The function should fix the via header
***************************************************************************/
RvStatus TransmitterUpdateViaByFinalLocalAddress(
                                IN Transmitter* pTrx,
                                IN RvBool       bFixVia);

/************************************************************************************
 * TransmitterSetInUseLocalAddrByLocalAddr
 * ----------------------------------------------------------------------------------
 * General: Set the inUse local address to point to the place that holds the
 *          supplied local address.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - the transmitter pointer
 *          hLocalAddr - the local address to set in the address in use.
 ***********************************************************************************/
void RVCALLCONV TransmitterSetInUseLocalAddrByLocalAddr (
                            IN  Transmitter*                   hTrx,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransmitterNoOwner
 * ------------------------------------------------------------------------
 * General: Called when attaching NULL owner. 
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx - The trx handle
 ****************************************************************************/
RvStatus RVCALLCONV TransmitterNoOwner(IN Transmitter* pTrx);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransmitterSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transmitter.
 *          As a result of this operation, all messages, sent by this
 *          Transmitter, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTrx    - Pointer to the Transmitter object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TransmitterSetSecObj(IN  Transmitter*      pTrx,
                              IN  RvSipSecObjHandle hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIGCOMP_ON
/*****************************************************************************
 * TransmitterUpdateOutboundMsgSigcompId
 * ---------------------------------------------------------------------------
 * General: Update the Trx outbound msg sigcomp-id String of the outbound destination 
 * Return Value: RvStatus.
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - The sending transmitter.
 *          hSrcPool   - The source pool holding the sigcomp-id
 *          hSrcPage   - The source page holding the sigcomp-id
 *          srcOffset  - The offset in the page of the sigcomp-id string
 * Output:  
 ****************************************************************************/
RvStatus RVCALLCONV TransmitterUpdateOutboundMsgSigcompId(
                           IN Transmitter       *pTrx,
                           IN HRPOOL             hSrcPool,
						   IN HPAGE              hSrcPage,
						   IN RvInt32            srcOffset);
#endif /* RV_SIGCOMP_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSMITTER_OBJECT_H*/

