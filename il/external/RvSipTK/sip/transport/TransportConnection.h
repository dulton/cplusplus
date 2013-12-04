/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportTCP.h
 *
 *  This file provides headers for the transport layer of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/


#ifndef TRANSPORT_CONNECTION_H
#define TRANSPORT_CONNECTION_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransportMgrObject.h"
#include "_SipCommonTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define TRANSPORT_CONNECTION_TOS_UNDEFINED -1
#define TRANSPORT_CONNECTION_TOS_VALID(tos) ((tos >=0) && (tos <=255))

typedef enum{
    TRANSPORT_TLS_HANDSHAKE_IDLE,
    TRANSPORT_TLS_HANDSHAKE_STARTED,
    TRANSPORT_TLS_HANDSHAKE_COMPLEATED
}TransportTlsHandshakeState;


/********************************************************************************************
 * Definition of TransportTcpRecvInfo:
 * Holds the info about the message parts that already received, and the place in
 * the HPAGE where the last header starts (details later).
 *
 *
 ********************************************************************************************/
typedef struct {
        HPAGE         hMsgReceived;      /*message received copied to a page - accumulate packets*/
        RvInt32       msgSize;              /*the size of the message received so far*/
        RvBool        bReadingBody;  /*did we start reading the message body*/
        RvInt32       offsetInBody;      /*how many bytes we read from the body so far*/
        RvInt32       contentLength;
        RvInt32       lastHeaderOffset;   /*the beginning offset of the last header*/
        RvInt32       bodyStartIndex;
#ifdef RV_SIGCOMP_ON
        RvChar       *pDecompressedBuf;       /*pointer to the decompressed buffer that was allocated  */
                                              /* for decompression of a non-plain buffer */
        RvInt32       decompressedBufSize;    /* The size of the decompressed buffer */
        RvSipCompType eMsgCompType;          /*The compression type of the received stream  */
        RvInt32       sigCompQuotedBytesLeft; /*The number of bytes to be read without looking for 0xFF*/
        RvBool        bSigCompDelimiterRcvd;  /* Indicates if part of NOT-IGNORED SigComp delemiter    */
                                              /* was received in the last TCP packet */
#endif /* RV_SIGCOMP_ON */
}TransportTcpRecvInfo;


/******************************************************************************
 * TransportConnection
 * ----------------------------------------
 * Definition of TransportConnection object
 * Holds the parameters for each connection.
 *  pTransportMgr   - pointer to the transport struct.
 *  eConnectionType - the type of the connection (client/....)
 *  recvInfo        - information of the msg received
 *  hSocket         - the socket of the connection
 *  event           - kind of event the socket will hand over to the transport
 *  hMsgToSendList  - list of msgs about to be send. This list will send its
 *                    items when the connect event will arise.
 *  hConnectionOwnersList - list of transactions own the connection. Since
 *                    single connection can serve more then one transaction.
 *  hConnLock       - lock handle
 *  state           - state of the connection.
 *  bConnErr        - there is an error on this connection. notify it in the
 *                    first state change.
 *  address         - the address of the destination in case of client and
 *                    the local port in case of server which is the first connection.
 *  isDeleted       - mark that specifies if the TCP connection should be deleted
 *  usageCounter    - usage counter that is used to prevent deleting of the TCP info structure
 *                    when there are other threads that may try to access it.
 *  strSipAddress   - the address to which the connection was initiated.
 *                    That string will be used to verify that the certificate
 *                    was given to the right address ip/FQDN
 *  bWriteEventInQueue - Indication for WRITE_EVENT related to the connection
 *                    in the processing queue.
 *  bCloseEventInQueue - close event in queue does not effect the connection
 *                    state but we don't want new owners to use this connection.
 *  saftyTimer      - This timer is used for three safety operations:
 *                    1. set to 64*T1 for handshake. if by this time handshake is not 
 *                       completed, the connection will error.
 *                    2. set for 4 seconds for TLS close seq
 *                    3. set for TCP_SHUTDOWN_NO_CLOSE_TIMER for tcp shutdown->close
 *  bClosedByLocal  - only valid in states closing and closed
 *                    true - local close request
 *                    false - remote close request
 *  typeOfService   - TypeOfService byte, that will be set into IP header of
 *                    all outgoing message IP packets. Valid range - [0-127].
 *  localPort       - Local port of established client TCP connection.
 *  bDisconnectionTimer    - indicates if disconnection timer is set, if this timer is set,
 *                    it can not be used.
 *
 *  connection reuse parameters
 *  ============================
 *  pAlias               - An alias rpool string, allocated on the connection page from element-pool.
 *  pHashAliasElement    - application can insert a server connection to the hash. As a result, a
 *                         server connection might be inserted to the hash table twice - by its alias 
 *                         and by its IP and port. For this reason we must keep two hash elements in the
 *                         connection object, so on connection termination, we will remove it from the 
 *                         hash table in both entries.
 *	hConnNoOwnerListItem - if the connection has no owner, then it is present in the 
 *                         Connected-No-Owner list.
 *                         This is the handle to it's list object, for quick approach.
 *
 * pSecOwner      - Pointer to the Security Object that uses this Connection
 * impi           - Pointer to the IMPI, set into the Local Address by
 *                  Application through Security Agreement-Security Object API.
 *                  IMPI - IP Multimedia Private Identity, as it is defined in
 *                  TG 33.208.
 *****************************************************************************/
typedef struct
{
    TransportMgr*                                   pTransportMgr;
    /* ccore */
    RvSelectFd                                      ccSelectFd;
    RvSocket                                        ccSocket;
    RvSocket*                                       pSocket;
    SipTransportAddress                             destAddress;
    RvSelectEvents                                  ccEvent;
    RvBool                                          bFdAllocated;

    /* oor*/
    RvSelectEvents                                  oorEvents;
    RLIST_ITEM_HANDLE                               oorListLocation;
    RvBool                                          oorConnectIsError;
    /* ccore end*/

    RvSipTransportObjEventInfo                      terminationInfo;
    RvSipTransportConnectionType                    eConnectionType;
    RvSipTransport                                  eTransport;
    TransportTcpRecvInfo                            recvInfo;
    RLIST_HANDLE                                    hMsgToSendList;
    RLIST_HANDLE                                    hConnectionOwnersList;
    RvInt32                                         numOfOwners;
    SipTripleLock*                                  pTripleLock;
    SipTripleLock                                   connTripleLock;
    RvInt32                                         connUniqueIdentifier;
    RvBool                                          isDeleted;
    RvUint32                                        usageCounter;
    RvSipTransportConnectionState                   eState;
    RvBool                                          bConnErr;
    RvSipTransportConnectionStateChangedReason      eTcpReason;
    RvSipTransportLocalAddrHandle                   hLocalAddr;
    RvInt32                                         bufSize;
    RvChar*                                         pBuf;
    RvChar*                                         originalBuffer;
    RvInt32                                         msgStart;
    RvInt32                                         msgSize;
    RvInt32                                         bytesReadFromBuf;
    RvInt32                                         msgComplete;
    void*                                           pHashElement;
    SipTimer                                        hServerTimer;
    RvSipTransportConnectionAppHandle               hAppHandle;
    RvBool                                          bWriteEventInQueue;
    RvBool                                          bCloseEventInQueue;
    SipTimer                                        saftyTimer;
    RvBool                                          bClosedByLocal;
    /* connection reuse parameters: */
    void*                                           pHashAliasElement;
    RPOOL_Ptr                                       pAlias;
    RLIST_ITEM_HANDLE                               hConnNoOwnerListItem;    
    HPAGE                                           hConnPage;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* TLS */
    SipTlsEngine*                                   pTlsEngine;
    RvTLSSession*                                   pTlsSession;
    TransportTlsHandshakeState                      eTlsHandShakeState;
    RvSipTransportVerifyCertificateEv               pfnVerifyCertEvHandler;
    RvTLSEvents                                     eTlsEvents;
    RvSipTransportTlsHandshakeSide                  eTlsHandshakeSide;
    RvSipTransportObjEventInfo                      tlsStateInfo;
    RvInt32                                         strSipAddrOffset;
    RvSipTransportConnectionStateChangedReason      eTlsReason;
    RvBool                                          bConnectionAsserted;
    RvSipTransportConnectionTlsState                eTlsState;
    RvInt                                           tlsShutdownSafeCounter;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
    RvInt32                                         typeOfService;
    RvInt32                                         localPort;
	RvBool											bDisconnectionTimer;
#if (RV_NET_TYPE & RV_NET_SCTP)
    TransportConnectionSctpData                     sctpData;
#endif
#ifdef RV_SIP_IMS_ON
    void*                                           pSecOwner;
    void*                                           impi;
#endif

}TransportConnection;

/* TransportConnectionHashKey
 *---------------------------
 * Defined the information saved in the hash for each connection object.
 * The information is used for locating a specific connection.
 */
typedef struct
{
    TransportMgr                    *pTransportMgr;  /*for loging*/
    RvSipTransportLocalAddrHandle   hLocalAddr;      /*connection local address*/
    RvAddress                       *pRemoteAddress; /*pointer to a connection remote addr*/
    RvSipTransport                  eTransport;
    RPOOL_Ptr                      *pAlias;
#ifdef RV_SIGCOMP_ON
    RvSipCompType                   eComp;          /* The type of the connection compression */
#endif
}TransportConnectionHashKey;


/* TransportConnectionOwnerInfo
 *---------------------------
 * Information kept for each owner in the owner list.
 * hOwner            -     the owner handle
 * pfnStausEvHandler -       the owner status event handler
 * pfnStateChangedEvHandler - state callback function
 * hListItem         - position in the owners list
 * pHashElement      - The owner hash element in the owners hash
 * bWasNotified      - marks if an owner was notified of a specific event in a loop
 *                      of owner notifications so that it will not be notified twice.
 */
typedef struct
{
    RvSipTransportConnectionOwnerHandle    hOwner;
    RvSipTransportConnectionStatusEv       pfnStausEvHandler;
    RvSipTransportConnectionStateChangedEv pfnStateChangedEvHandler;
    RLIST_ITEM_HANDLE                      hListItem;
    void*                                  pHashElement;
    RvBool                                bWasNotified;
} TransportConnectionOwnerInfo;



/* ConnectionOwnerHashKey
 *---------------------------
 * Defined the information saved in the hash for each connection owner.
 * The information is used for locating a specific owner.
 */
typedef struct
{
    TransportConnection*                     pConn;  /*for loging*/
    RvSipTransportConnectionOwnerHandle     hOwner;
}ConnectionOwnerHashKey;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransportConnectionListConstruct
 * ------------------------------------------------------------------------
 * General: Allocated the list of Connections managed by the manager
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr   - Handle to the transport manager
 *            maxNumOfConn - Max number of Connections to allocate
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionListConstruct(
                                        IN TransportMgr      *pTransportMgr,
                                        IN RvInt32          maxNumOfConn);


/************************************************************************************
 * TransportConnectionAllocate
 * ----------------------------------------------------------------------------------
 * General: Allocates a empty new connection object.
 *          The connection parameters are set to undefined.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr       - Pointer to transport instance
 * Output:  hConnection            - free connection to use
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionAllocate(
                     IN  TransportMgr                     *pTransportMgr,
                      OUT RvSipTransportConnectionHandle   *phConn);


/************************************************************************************
 * TransportConnectionInit
 * ----------------------------------------------------------------------------------
 * General: Initialized a connection and insert it to the hash if its type is client
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - connection pointer
 *          type        - connection type - client/server
 *          eTransport  - tcp/udp
 *          hLocalAddress - local address to be used by the connection.
 *          pDestAddress - connection dest address.
 *          bInsertToHash - indicates whether to insert the connection to the hash
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionInit(
                     IN TransportConnection              *pConn,
                     IN RvSipTransportConnectionType     type,
                     IN RvSipTransport                   eTransport,
                     IN RvSipTransportLocalAddrHandle    hLocalAddress,
                     IN SipTransportAddress*             pDestAddress,
                     IN RvBool                           bInsertToHash);


/************************************************************************************
 * TransportConnectionConstruct
 * ----------------------------------------------------------------------------------
 * General: Allocate and initialize a connection object. This function is never called
 *          from the application.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr       - Pointer to transport instance
 *          bForceCreation      - indicates whether to look for such a connection in
 *                                the hash or to force the creation of a new connection.
 *          type                - client or server
 *          hLocalAddress       - connection local address
 *          pDestAddress        - The destination address
 *          hOwner              - The connection owner.
 *          pfnConnStateChangeHandler - Function pointer to notify connection state changed
 *          pfnConnStatusEvHandler - function pointer to notify of connection status.
 *          pbNewConnCreated    - Indicates whether the connection that was returned is
 *                                a new connection that was just created.
 * Output:  hConnection         - free connection to use
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionConstruct(
                         IN  TransportMgr                           *pTransportMgr,
                         IN  RvBool                                  bForceCreation,
                         IN  RvSipTransportConnectionType            type,
                         IN  RvSipTransport                          eTransport,
                         IN  RvSipTransportLocalAddrHandle           hLocalAddress,
                         IN  SipTransportAddress                    *pDestAddress,
                         IN  RvSipTransportConnectionOwnerHandle     hOwner,
                         IN  RvSipTransportConnectionStateChangedEv  pfnConnStateChangeHandler,
                         IN  RvSipTransportConnectionStatusEv        pfnConnStatusEvHandler,
                         OUT RvBool                                 *pbNewConnCreated,
                         OUT RvSipTransportConnectionHandle         *phConn);


/***************************************************************************
 * TransportConnectionListDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the list of connections. All connections that are not
 *          closed are closed.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to allocate list.
 *               RV_OK        - Call-leg list was allocated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - pointer to the transport manager
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionListDestruct(
                                        IN TransportMgr      *pTransportMgr);


/***************************************************************************
 * TransportConnectionAttachOwner
 * ------------------------------------------------------------------------
 * General: set the connection owner with the connection callback function
 *
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 *          pfnStateChangedEvHandler - connection state changes callback
 *          pfnConnStatusEvHandle - notifies connection status.
 *            hOwner - handle to the owner.
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionAttachOwner(
                           IN  TransportConnection*                   pConn,
                           IN  RvSipTransportConnectionStateChangedEv pfnStateChangedEvHandler,
                           IN     RvSipTransportConnectionStatusEv       pfnConnStatusEvHandler,
                           IN RvSipTransportConnectionOwnerHandle hOwner);


/***************************************************************************
 * TransportConnectionDetachOwner
 * ------------------------------------------------------------------------
 * General: remove the owner from the owners list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn   - Pointer to the connection.
 *          hOwner  - The owner handle
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionDetachOwner(
                            IN TransportConnection                *pConn,
                            IN RvSipTransportConnectionOwnerHandle hOwner);

/***************************************************************************
 * TransportConnectionDisconnect
 * ------------------------------------------------------------------------
 * General: Disconnects the connection.
 *          This process will evantually terminate the connection as it
 *          will asume the disconnected state.
 *          This function also removes the connection from the hash, so it will
 *          no longer be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionDisconnect(
                           IN  TransportConnection*                   pConn);

/************************************************************************************
 * TransportConnectionTerminate
 * ----------------------------------------------------------------------------------
 * General: Change the connection state to terminate internally and insert it to the
 *          processing queue.
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection    - Handle to the connection to terminate.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionTerminate(
                IN TransportConnection                       *pConn,
                IN RvSipTransportConnectionStateChangedReason eReason);

/************************************************************************************
 * TransportConnectionFree
 * ----------------------------------------------------------------------------------
 * General: Free connection resources and remove it from the hash and from the list.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionFree(IN TransportConnection    *pConn);


/******************************************************************************
 * TransportConnectionChangeState
 * ----------------------------------------------------------------------------
 * General: Changes the connection state and notify all owners.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn        - Pointer to the relevant connection.
 *          eState       - The connection new state
 *          eReason      - The reason for the new state.
 *          bSendToQueue - Pass the event through the TPQ (Transport Processing
 *                         Queue). All transport events must come from the TPQ.
 *          bChangeInternalState - change the state internally or not. we don't
 *                         want to change the state after queue event since
 *                         the state already changed.
 * Output:  RV_OK on success, error code - otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionChangeState(
           IN TransportConnection*                       pConn,
           IN RvSipTransportConnectionState              eState,
           IN RvSipTransportConnectionStateChangedReason eReason,
           IN RvBool                                     bSendToQueue,
           IN RvBool                                     bChangeInternalState);

/************************************************************************************
 * TransportConnectionHashRemove
 * ----------------------------------------------------------------------------------
 * General: Remove a given connection from the hash
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionHashRemove(IN TransportConnection    *pConn);


/******************************************************************************
 * TransportConnectionNotifyStatus
 * ----------------------------------------------------------------------------
 * General: Notifies connection owners of the connection new status.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn        - Pointer to the relevant connection.
 *          hOwner       - Handle to the owner to be notified.
 *                         If  NULL, all owners will be notified.
 *          pMsgSendInfo - For MSG_WAS_SENT status - information about message
 *          eConnStatus  - The connection status.
 *          bSendToQueue - pass the event through the TPQ (transport Processing
 *                         Queue). All transport events must come from the TPQ.
 * Output:  none
 *****************************************************************************/
void RVCALLCONV TransportConnectionNotifyStatus(
                       IN TransportConnection                *pConn,
                       IN RvSipTransportConnectionOwnerHandle hOwner,
                       IN SipTransportSendInfo*               pMsgSendInfo,
                       IN RvSipTransportConnectionStatus      eConnStatus,
                       IN RvBool                              bSendToQueue);

/************************************************************************************
 * TransportConnectionFreeConnPage
 * ----------------------------------------------------------------------------------
 * General: Free the connection page
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportConnectionFreeConnPage(IN TransportConnection    *pConn);

/******************************************************************************
* TransportConnectionApplyServerConnReuseThreadSafe
* -----------------------------------------------------------------------------
* General: applies the connection-reuse procedure on the connection:
*          1. check that the top via contain an 'alias' parameter.
*          2. authorize connection alias.
*          3. insert the connection to the hash.
*          Locks connection Event Lock before & unlocks it after the procedure.
* Return Value: void
* -----------------------------------------------------------------------------
* Arguments:
*      pTransportMgr - handle to the Transport Manager object.
*      hConn         - handle to the connection.
*      hAppConn      - handle stored by the application in the connection.
*      hMsg          - the received message object
* Output: void
******************************************************************************/
RvStatus TransportConnectionApplyServerConnReuseThreadSafe(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvSipMsgHandle                    hMsg);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
#if (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * TransportConnectionGetTlsStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given tls state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransportConnectionGetTlsStateName (
                                IN  RvSipTransportConnectionTlsState  eTlsState);

/***************************************************************************
 * TransportConnectionGetSecurityNotificationName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given security status
 * Return Value: The string with the status name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eStatus - The status as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransportConnectionGetSecurityNotificationName (
                                IN  RvSipTransportConnectionTlsStatus  eStatus);

#else
#define TransportConnectionGetTlsStateName(_st) "-"
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#else /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
#define TransportConnectionGetTlsStateName(_st) "-"
#endif
#define CONNECTION_OBJECT_NAME_STR              "Connection"

/***************************************************************************
 * TransportConnectionGetAddress
 * ------------------------------------------------------------------------
 * General: retrieves local or remote address from a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr   - pointer to the transport manager
 *            pConn       - the connection that its address we seek
 *          bIsRemote   - get the remote or local address of the connection
 * Output:  pAddress    - A previously allocated buffer to where the local address
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - a pointer to the variable to hold the port
 *          peAddressType - ipv6/ipv4
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionGetAddress(
                                        IN  TransportConnection    *pConn,
                                        IN  RvBool             bIsRemote,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *pPort,
#if defined(UPDATED_BY_SPIRENT)
                                        OUT RvChar             *if_name,
					OUT RvUint16           *vdevblock,
#endif
                                        OUT RvSipTransportAddressType *peAddressType);

/***************************************************************************
 * TransportConnectionHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a connection into the hash table.
 *          The key is generated from the connection information and the
 *          connection handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to insert to the hash table
 *               RV_OK - connection handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionHashInsert(
                               IN RvSipTransportConnectionHandle hConn,
                               IN RvBool                         bByAlias);

/******************************************************************************
 * TransportConnectionRegisterSelectEvents
 * ----------------------------------------------------------------------------
 * General: This function is used to wrap Core Select Functions.
 *          For First calls the select FD will be constructed, and first select
 *          call will be placed.
 *          For calls (other than first) that have events, a select update call
 *          will be made.
 *          For calls with no events, select usage will be stoped:
 *              Fd will be destructed
 *              A call to select remove will be placed
 *
 * Return Value: RV_OK - completes succesfully.
 *               RV_ERROR_UNKNOWN - failed in one of como core operations
 *
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn      - Pointer to the relevant connection.
 *          bAddClose  - should the close event be called on
 *          bFirstCall - is this the first time we call for the socket?
 *                       if it is the first timem we will construct fd and 
 *                       add it to the select FDs
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionRegisterSelectEvents(
                                            IN TransportConnection  *pConn,
                                            IN RvBool               bAddClose,
                                            IN RvBool               bFirstCall);

/******************************************************************************
 * TransportConnectionDecUsageCounter
 * ----------------------------------------------------------------------------
 * General: Decrements connection usage counter. In case usage counter is 0
 *          and connection is marked as 'deleted' this function actually
 *          deletes the connection.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDecUsageCounter(
                                            IN TransportConnection    *pConn);

/******************************************************************************
 * TransportConnectionDecUsageCounterThreadSafe
 * ----------------------------------------------------------------------------
 * General: Calls TransportConnectionDecUsageCounter() in thread safe way:
 *          locks the Event Lock of the connection before call and
 *          unlocks it after it.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the Transport Manager object.
 *          pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDecUsageCounterThreadSafe(
                                        IN TransportMgr*        pTransportMgr,
                                        IN TransportConnection* pConn);

/******************************************************************************
 * TransportConnectionIncUsageCounter
 * ----------------------------------------------------------------------------
 * General: Increments connection usage counter. In case connection
 *          is marked as 'deleted' this function returns RV_ERROR_UNKNOWN.
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionIncUsageCounter(
                                            IN TransportConnection    *pConn);

/******************************************************************************
 * TransportConnectionOpen
 * ----------------------------------------------------------------------------
 * General: opens the connection - creates a socket, binds it, listens/connects
 *          to the peer socket.
 * Return Value: RV_OK on success
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pConn           - Pointer to the connection.
 *         bConnect        - If RV_FALSE, the socket will not be connected.
 *                           In this case TransportTCPConnect should be called.
 *                           This paramater is actual for client connections.
 *         clientLocalPort - Port, to which the connection socket will be bound
 *                           This parameter is actual for client connections.
 *                           Listening connections use port of their Local
 *                           Address objects.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionOpen(
                                    IN TransportConnection* pConn,
                                    IN RvBool               bConnect,
                                    IN RvUint16             clientLocalPort);

/******************************************************************************
 * TransportConnectionOpen
 * ----------------------------------------------------------------------------
 * General: close the connection - moves it to CLOSING state, stop listening
 *          for incoming messages, removes it from hash.
 * Return Value: RV_OK on success
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - Pointer to the connection.
 *          bCloseTlsAsTcp  - If RV_TRUE, the TLS state machine is not affected
 *                            by the closing.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionClose(
                                       IN TransportConnection* pConn,
                                       IN RvBool               bCloseTlsAsTcp);

/***************************************************************************
* TransportConnectionHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:    newHashElement  - a new hash element
*           oldHashElement  - an element from the hash
***************************************************************************/
RvBool TransportConnectionHashCompare(IN void *newHashElement,
                                      IN void *oldHashElement);

/***************************************************************************
 * TransportConnectionAddToConnectedNoOwnerList
 * ------------------------------------------------------------------------
 * General: Add connection to the not-in-use connections list.
 *
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionAddToConnectedNoOwnerList(
                           IN  TransportConnection*  pConn);

/***************************************************************************
 * TransportConnectionRemoveFromConnectedNoOwnerList
 * ------------------------------------------------------------------------
 * General: remove the connection from the not-in-use list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Output:     pConn - pointer to the connection.
 ***************************************************************************/
void RVCALLCONV TransportConnectionRemoveFromConnectedNoOwnerList(
                               TransportConnection   *pConn);

/***************************************************************************
 * TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe
 * ------------------------------------------------------------------------
 * General: calls TransportConnectionRemoveFromConnectedNoOwnerList in thread
 *          safe way: locks the connection Event Lock before and unlocks it
 *          after the call.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to the Transport Manager object.
 *          pConn         - pointer to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe(
                                        IN TransportMgr*        pTransportMgr,
                                        IN TransportConnection* pConn);

/***************************************************************************
 * TransportConnectionHashFindByAlias
 * ------------------------------------------------------------------------
 * General: find a connection in the hash table according to given information.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr -    Handle of the transport manager
 *          eTransport    -    The requested transport (Tcp/Tls/Sctp)
 *          hLocalAddr    -    Handle to the requested local address
 *          pDestAddr     -    pointer to the requested destination address
 * Output:  phConn -           Handle to the connection found in the hash or NULL
 *                             if the connection was not found.
 ***************************************************************************/
void RVCALLCONV TransportConnectionHashFind(
                         IN TransportMgr                   *pTransportMgr,
                         IN RvSipTransport                 eTransport,
                         IN RvSipTransportLocalAddrHandle  hLocalAddress,
                         IN SipTransportAddress            *pDestAddress,
                         IN RPOOL_Ptr                      *pAlias,
                         OUT RvSipTransportConnectionHandle *phConn);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * TransportConnectionLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks connection according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionLockAPI(
                                   IN  TransportConnection*   pConn);

/************************************************************************************
 * TransportConnectionUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: unLocks connection according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pConn - pointer to the connection
***********************************************************************************/
void RVCALLCONV TransportConnectionUnLockAPI(IN  TransportConnection*   pConn);

/************************************************************************************
 * TransportConnectionLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks connection according to Event processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
RvStatus RVCALLCONV TransportConnectionLockEvent(IN  TransportConnection*   pConn);

/************************************************************************************
 * TransportConnectionUnLockEvent
 * ----------------------------------------------------------------------------------
 * General: UnLocks connection according to EVENT processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV TransportConnectionUnLockEvent(IN  TransportConnection*   pConn);


#else
#define TransportConnectionLockAPI(a) (RV_OK)
#define TransportConnectionUnLockAPI(a)
#define TransportConnectionLockEvent(a) (RV_OK)
#define TransportConnectionUnLockEvent(a)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
 * TransportConnectionRemoveOwnerMsgs
 * ------------------------------------------------------------------------
 * General: remove all the pending messages of a specific owner that
 *          are about to be send, from the connection send list.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn  - Pointer to the Connection object.
 *          hOwner - Handle to the connection owner.
 ***************************************************************************/
void RVCALLCONV TransportConnectionRemoveOwnerMsgs(
                      IN TransportConnection*                pConn,
                      IN RvSipTransportConnectionOwnerHandle hOwner);

/******************************************************************************
 * TransportConnectionDisconnectOnNoOwners
 * ----------------------------------------------------------------------------
 * General: checks, if the connection has owners.
 *          If it does, there is nothing to be done. Just return.
 *          Otherwise - initiate disconnection.
 * Return Value: void
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn  - Pointer to the Connection object.
 *          hOwner - Handle to the connection owner.
 *          bForce - If RV_TRUE, the Connection will be disconnected.
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionDisconnectOnNoOwners(
                                               IN TransportConnection* pConn,
                                               IN RvBool               bForce);

/******************************************************************************
 * TransportConnectionAddWriteEventToTPQ
 * ----------------------------------------------------------------------------
 * General: allocates, initializes and tails the WRITE_EVENT event
 *          to the Transport Processing Queue (TPQ).
 *          This function is called on handling of write event raised by select
 *          The function assumes that the Connection object is locked already.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - The Connection object
 * Output:  ppEv  - The allocated event
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionAddWriteEventToTPQ(
                                    IN  TransportConnection* pConn,
                                    OUT void**               ppEv);

/******************************************************************************
 * TransportConnectionReceive
 * ----------------------------------------------------------------------------
 * General: reads data from socket, that serves bit-stream connection,
 *          such as TCP or TLS.
 *          The function accumulates received bytes until the whole SIP message
 *          was received. After this the message is parsed and passed to upper
 *          layers.
 *          This function is called on handling of read event raised by select.
 *          The function assumes that the Connection object is locked already.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - The Connection object
 * Output:
 *****************************************************************************/
RvStatus RVCALLCONV TransportConnectionReceive(IN  TransportConnection* pConn);

#ifdef __cplusplus
}
#endif


#endif /*TRANSPORT_CONNECTION_H*/


