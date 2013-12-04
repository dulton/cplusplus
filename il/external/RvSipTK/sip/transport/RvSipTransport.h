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
 *                              <RvSipTransport.h>
 *
 * The transport layer of the RADVISION SIP toolkit allows you to control sending
 * and receiving of messages over the network.
 * This file defines the API for the transport layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2002
 *********************************************************************************/


#ifndef RVSIP_TRANSPORT_H
#define RVSIP_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "RV_SIP_DEF.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransportDNS.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#include "rvnettypes.h"
    
/*-----------------------------------------------------------------------*/
/*                TRANSPORT MANAGER  API                                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipTransportMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for all transport events.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *            hAppTransportMgr - An application handle. This handle will be supplied
 *                        with some of transport callbacks
 *          pHandlers   - Pointer to structure containing application event
 *                        handler pointers.
 *          evHandlerStructSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetEvHandlers(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipAppTransportMgrHandle       hAppTransportMgr,
                        IN RvSipTransportMgrEvHandlers     *pHandlers,
                        IN RvUint32                        evHandlerStructSize);

/***************************************************************************
 * RvSipTransportMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the handle to the application transport manger object.
 *          You can also supply this handle when calling
 *          RvSipTransportMgrSetEvHandlers().
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the stack transport manager.
 *           phAppTransportMgr - The application transport manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetAppMgrHandle(
                                   IN RvSipTransportMgrHandle      hTransportMgr,
                                   IN RvSipAppTransportMgrHandle   phAppTransportMgr);

/***************************************************************************
 * RvSipTransportMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application transport manger object.
 *          You set this handle in the stack using the
 *          RvSipTransportMgrSetEvHandlers() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 * Output:     hAppTransportMgr - The application transport manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrGetAppMgrHandle(
                         IN RvSipTransportMgrHandle        hTransportMgr,
                         OUT RvSipAppTransportMgrHandle*   hAppTransportMgr);



/***************************************************************************
 * RvSipTransportMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transport
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr   - Handle to the transport manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance. In order to use this
 *                            pointer as a stack handle you need to cast it to
 *                            RvSipStackHandle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrGetStackInstance(
                                   IN RvSipTransportMgrHandle hTransportMgr,
                                   OUT  void*                *phStackInstance);

/***************************************************************************
 * RvSipTransportMgrSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for the entire stack that will be used
 *			by all objects. This function is equivalent to the outbound address
 *			that is being set in configuration except that now the address
 *			can be updated during the lifetime of the stack.
 *			
 *   		Note that unless there are specific reasons to use this function, it is 
 *			recommended to set this address only in the construction of the stack
 *			through the configuration from the following reasons:
 *			1. Setting the outbound address during the time that transactions
 *			   are alive and send messages might cause the address not to be
 *			   updated completely.
 *			2. Even if it is updated completely, changing the outbound address
 *			   while there is an active transaction that sends messages (using a transmitter)
 *			   will not change the destination. This means that if the first
 *			   message was sent to the old outbound proxy address, the retransmissions
 *			   will be sent to the same old address.
 *			3. This function allocates dynamic memory to hold the address.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr   - Handle to the transport manager.
 *            pOutboundAddr   - A valid pointer that holds the new outbound address.
 *            sizeOfCfg       - The size of the outbound proxy configuration structure.
 * Output:    None
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrSetOutboundAddress(
                                   IN RvSipTransportMgrHandle          hTransportMgr,
                                   IN RvSipTransportOutboundProxyCfg   *pOutboundAddr,
								   IN RvInt32                          sizeOfCfg);

/*-----------------------------------------------------------------------*/
/*                TRANSPORT MANAGER  UTILITY FUNCTIONS                   */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipTransportConvertStringToIp
 * ------------------------------------------------------------------------
 * General: Converts an ip from string format to binary format
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr     - Handle to the transport manager
 *          pszIpAsString    - a NULL terminated string representing an ip address
 *                             (d.d.d.d for ipv4, x:x:x:x:x:x:x:x for ipv6)
 *                             Note: do not use [] for ipv6 addresses.
 *             eAddressType     - represent the type of address ipv6/ipv4
 * Output:  pIpAsBinary      - the ip address represented in binary.
 *                             (16 bytes for ipv6, 4 bytes for ipv4)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertStringToIp (IN  RvSipTransportMgrHandle   hTransportMgr,
                                                            IN  RvChar*                  strIpAsString,
                                                            IN  RvSipTransportAddressType eAddressType,
                                                            OUT RvUint8*                  pIpAsBinary);

/***************************************************************************
 * RvSipTransportConvertIpToString
 * ------------------------------------------------------------------------
 * General: Converts an IP address from a binary format to a string format
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr     - Handle to the transport manager
 *          pIpAsBinary      - The ip address represented in a binary format.
 *                             (16 bytes for ipv6, 4 bytes for ipv4)
 *          stringLen        - the size of the buffer.
 *             eAddressType     - represent the type of address ipv6/ipv4
 * Output:  strIpAsString    - a NULL terminated string representing an ip address
 *                             (d.d.d.d for ipv4, x:x:x:x:x:x:x:x for ipv6)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertIpToString (IN  RvSipTransportMgrHandle   hTransportMgr,
                                                            IN  RvUint8*                  pIpAsBinary,
                                                            IN  RvSipTransportAddressType eAddressType,
                                                            IN  RvInt32                  stringLen,
                                                            OUT RvChar*                  strIpAsString);
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipTransportInjectMsg
 * ------------------------------------------------------------------------
 * General: This function enables the application to 'inject' a message to the stack.
 *          The message will be handled by the stack as if it was received from the
 *          network.
 *          The message may be given as a string or as a message object.
 *          You can optionally supply a local and remote addresses for this message.
 *          Suppling a local address is useful in the case of multihomed host when
 *          you want to simulate a message that is received from a specific network
 *          card. For request messages this will cause the response to be sent from
 *          the same card. If you supply a remote address this address will be set
 *          as the received parameter of the Via header for incoming requests. Responses
 *          will then be sent to this address.
 *          If you want to simulate a message that was received on a specific connection
 *          you can supply a connection handle. In that the local and remote addresses
 *          will be taken from the connection and the pAddressInfo will be ignored.
 *          Note: If you don't care about addresses you can supply NULL value in the
 *          pAddressInfo and the hConn parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr  - Handle to the transport manager.
 *          pMsgBuffer     - The 'injected' message in a string format.
 *          totalMsgLength - Total length of the massage given in pMsgBuffer.
 *          hMsg           - Handle to the 'injected' message, in a message object format.
 *            hConn          - The connection handle that the message is 'injected' to.
 *          pAddressInfo   - Structure contains the local address and remote address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportInjectMsg(
                        IN  RvSipTransportMgrHandle            hTransportMgr,
                        IN  RvChar                        *pMsgBuffer,
                        IN  RvUint32                       totalMsgLength,
                        IN  RvSipMsgHandle                  hMsg,
                        IN  RvSipTransportConnectionHandle  hConn,
                        IN  RvSipTransportMsgAddrCfg       *pAddressInfo);

#endif /*#ifndef RV_SIP_PRIMITIVES */

/*-----------------------------------------------------------------------*/
/*                TRANSPORT CONNECTION API                               */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransportMgrCreateConnection
 * ------------------------------------------------------------------------
 * General: Constructs a new un-initialized connection and attach the supplied
 *          owner to the connection. The owner's event handlers structure
 *          is saved together with the connection owner.
 *          The new connection assumes the IDLE state.
 *          Calling the RvSipTransportConnectionInit() function at this state
 *          will initialize the connection and will cause the connection to move
 *          to the READY state.
 *          NOTE: This function does not connect the connection. In order to
 *          connect the connection you must first initialize it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hOwner     - An handle to the connection owner.
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the Event handler structure
 * Output:  phConn     - handle to a newly creates connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrCreateConnection(
     IN  RvSipTransportMgrHandle                 hTransportMgr,
     IN  RvSipTransportConnectionOwnerHandle     hOwner,
     IN  RvSipTransportConnectionEvHandlers      *pEvHandlers,
     IN  RvInt32                                sizeofEvHandlers,
     OUT RvSipTransportConnectionHandle          *phConn);



/***************************************************************************
 * RvSipTransportConnectionInit
 * ------------------------------------------------------------------------
 * General: Initializes a connection with all needed configuration parameters
 *          found in the RvSipTransportConnectionCfg structure. You can call
 *          this function only on the IDLE state. This function will cause the
 *          connection to move to the READY state.
 *          The initialized connection is inserted to the connection hash
 *          and therefor can be used by any persistent stack object.
 *          Note: This function does not connect the connection. The connection
 *          will be connected when a stack object will use it for
 *          sending a message or if you will specifically call the
 *          RvSipTransportConnectionConnect() function. In both cases the connection
 *          will assume the TCP_CONNECTING and then the TCP_CONNECTED states.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection to be initialized.
 *          pCfg  - The configuration to use when initializing the connection.
 *          sizeofCfg - The size of the configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionInit(
                        IN RvSipTransportConnectionHandle      hConn,
                        IN RvSipTransportConnectionCfg        *pCfg,
                        IN RvInt32                             sizeofCfg);

/***************************************************************************
 * RvSipTransportConnectionConnect
 * ------------------------------------------------------------------------
 * General: Connects a Connection. You can call this function only on the
 *          READY state. Calling this function will
 *          cause the connection to move to the CONNECTING state. The connection
 *          will move to the CONNECTED state when an indication that the connection
 *          was successfully connected will be received from the network.
 *          Note:
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection to connect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionConnect(
                        IN RvSipTransportConnectionHandle hConn);

/***************************************************************************
 * RvSipTransportConnectionTerminate
 * ------------------------------------------------------------------------
 * General: The function behavior depends on the connection state.
 *          If the connection is in the TCP_CONNECTED or TLS_CONNECTED state the
 *          connection will start a normal disconnection process. TCP connections
 *          will move to the CLOSING state. TLS connections will move to the
 *          TLS_CLOSE_SEQUENSE_STARTED and then to the CLOSING state.
 *          For all other states the connection will close it internal socket if
 *          the socket was opened and will terminate. After termination the connection
 *          will assume the TERMINATED state.
 *          Note: If the connection has messages that it is about to send these
 *          messages will be lost. It is therefor not recommended to use this
 *          function. If you no longer need this connection call the
 *          RvSipTransportConnectionDetachOwner() function. The connection will be
 *          closed only when the last owner is detached. This means that if the
 *          connection is still beeing used by other stack objects it will not
 *          be closed until these objects will detach from it.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection to be terminated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTerminate(
                        IN  RvSipTransportConnectionHandle  hConn);

/***************************************************************************
 * RvSipTransportConnectionAttachOwner
 * ------------------------------------------------------------------------
 * General: Attach a new owner to the supplied connection together with a set
 *          of callback functions that will be used to notify this owner about
 *          connection events. You can use this function only if
 *          the connection is connected or in the process of being connected.
 *          You cannot attach an owner to a connection that started its
 *          disconnection process.
 *          Notes:
 *          Note: The connection will not disconnect as long as it has owners
 *          attached to it.
 *              Application can't attach owner to incoming connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to the connection.
 *          hOwner - The owner handle
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the event handler tructure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionAttachOwner(
            IN  RvSipTransportConnectionHandle            hConn,
            IN  RvSipTransportConnectionOwnerHandle       hOwner,
            IN  RvSipTransportConnectionEvHandlers        *pEvHandlers,
            IN  RvInt32                                  sizeofEvHandlers);


/***************************************************************************
 * RvSipTransportConnectionDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach an owner from the supplied connection.
 *          If the connection is left with no other owners it will be closed.
 *          If the same owner attached to a connection more then once the first
 *          matching owner will be removed.
 *          Note: After detaching from a connection you will stop getting
 *          connection events and you must not use the connection handle any more.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn - Handle to the connection.
 *          hOwner - Handle to the owner to detach from the connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionDetachOwner(
            IN  RvSipTransportConnectionHandle            hConn,
            IN  RvSipTransportConnectionOwnerHandle       hOwner);


/***************************************************************************
 * RvSipTransportConnectionEnable
 * ------------------------------------------------------------------------
 * General: Inserts a connection object to the hash so that persistent objects
 *          will be able to use it.
 *          Note: When ever a connection is initialized with the function
 *          RvSipTransportConnectionInit() it is automatically inserted to
 *          the hash.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionEnable(
                                        IN  RvSipTransportConnectionHandle hConn);

/***************************************************************************
 * RvSipTransportConnectionDisable
 * ------------------------------------------------------------------------
 * General: Removes a connection object from the hash so that persistent objects
 *          will not be able to use it. Objects that are already using the connection
 *          (are in the connection owners list) will continue to use the connection.
 *          however, other objects will not be able to use the connection as long
 *          as the connection is Disabled.
 *          Note: in order to insert the connection back to the hash use
 *          RvSipTransportConnectionEnable().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionDisable(
                                        IN  RvSipTransportConnectionHandle hConn);


/***************************************************************************
 * RvSipTransportConnectionIsEnabled
 * ------------------------------------------------------------------------
 * General: Returns whether the connection is enabled (in the hash) or not.
 *          An enabled connection is a connection that can be used by persistent
 *          objects.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 * Output   pbIsEnabled  - RV_TRUE if the connection is enabled. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionIsEnabled(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvBool             *pbIsEnabled);


/***************************************************************************
 * RvSipTransportConnectionGetCurrentState
 * ------------------------------------------------------------------------
 * General: Retrieves the connection current state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn      - Handle to the connection.
 * Output:  peState    - The connection current state
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetCurrentState(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionState  *peState);

/***************************************************************************
 * RvSipTransportConnectionIsTcpClient
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED.
 *          Use RvSipTransportConnectionGetType instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 * Output:  pbIsClient  - RV_TRUE if the connection is a TCP client. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionIsTcpClient(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvBool                         *pbIsClient);

/***************************************************************************
 * RvSipTransportConnectionGetType
 * ------------------------------------------------------------------------
 * General: Returns the type of the connection (Client, Server, Mutliserver).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn            - Handle to the connection.
 * Output:  peConnectionType - Type of the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetType(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionType    *peConnectionType);

/***************************************************************************
 * RvSipTransportConnectionGetNumOfOwners
 * ------------------------------------------------------------------------
 * General: Retrieves the number of owners currently attached to the connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 * Output:  pNumOfOwners    - Number of connection owners.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetNumOfOwners(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvInt32                *pNumOfOwners);

/***************************************************************************
 * RvSipTransportConnectionGetTransportType
 * ------------------------------------------------------------------------
 * General: Retrieves the connection transport (TCP/TLS)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - Handle to the connection.
 * Output:  peTransport - The connection transport
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetTransportType(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvSipTransport          *peTransport);

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 'pPort' is defined as a macro in Spirent's app.  So, rename it to 'port'
 * to avoid compile errors.
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipTransportConnectionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the local address of the connection. The local address
 *          includes the local IP, local Port and the address type (IPv4 or IPv6).
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *			hConn       - Handle to the connection.
 * Output:  pAddress    - A previously allocated buffer to where the local address 
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - The local port.
 *          peAddressType - The local address type IPV4/IPv6
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetLocalAddress(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *port,
#if defined(UPDATED_BY_SPIRENT)
                                        OUT RvChar             *if_name,
					OUT RvUint16           *vdevblock,
#endif
                                        OUT RvSipTransportAddressType *peAddressType);
#else /* !defined(UPDATED_BY_SPIRENT) */
/***************************************************************************
 * RvSipTransportConnectionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the local address of the connection. The local address
 *          includes the local IP, local Port and the address type (IPv4 or IPv6).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *            hConn       - Handle to the connection.
 * Output:  pAddress    - A previously allocated buffer to where the local address
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - The local port.
 *          peAddressType - The local address type IPV4/IPv6
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetLocalAddress(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *pPort,
                                        OUT RvSipTransportAddressType *peAddressType);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 'pPort' is defined as a macro in Spirent's app.  So, rename it to 'port'
 * to avoid compile errors.
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipTransportConnectionGetRemoteAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the remote address of the connection. The remote address
 *          includes the remote IP, remote Port and the remote address type 
 *         (IPv4 or IPv6).
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *			hConn       - Handle to the connection.
 * Output:  srtAddress    - A previously allocated buffer to where the remote address 
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - The remote port.
 *          peAddressType - The remote address type IPV4/IPv6
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetRemoteAddress(
										IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *port,
                                        OUT RvSipTransportAddressType *peAddressType);
#else /* !defined(UPDATED_BY_SPIRENT) */
/***************************************************************************
 * RvSipTransportConnectionGetRemoteAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the remote address of the connection. The remote address
 *          includes the remote IP, remote Port and the remote address type
 *         (IPv4 or IPv6).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *            hConn       - Handle to the connection.
 * Output:  srtAddress    - A previously allocated buffer to where the remote address
 *                        will be copied to.
 *                        The buffer should have a minimum size of 48.
 *          pPort       - The remote port.
 *          peAddressType - The remote address type IPV4/IPv6
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetRemoteAddress(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        OUT RvChar             *strAddress,
                                        OUT RvUint16           *pPort,
                                        OUT RvSipTransportAddressType *peAddressType);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipTransportConnectionGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transport
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClientMgr   - Handle to the register-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetStackInstance(
                                   IN  RvSipTransportConnectionHandle  hConn,
                                   OUT void                 **phStackInstance);

/***************************************************************************
 * RvSipTransportConnectionGetAppHandle
 * ------------------------------------------------------------------------
 * General: retrieves the connection's application handle
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn          - The connection handle
 * Output:  phAppHandle - The connection application handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetAppHandle(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT RvSipTransportConnectionAppHandle *phAppHandle);

/***************************************************************************
 * RvSipTransportConnectionSetAppHandle
 * ------------------------------------------------------------------------
 * General: sets the connection's application handle
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn         - The connection handle
 *      :   hAppHandle - The connection application handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetAppHandle(
                        IN  RvSipTransportConnectionHandle  hConn,
                        IN  RvSipTransportConnectionAppHandle hAppHandle);

/******************************************************************************
 * RvSipTransportConnectionGetSecOwner
 * ----------------------------------------------------------------------------
 * General: Retrieves the security owner of the connection.
 *          The security owner is a security-object which is implemented in
 *          the RADVISION IMS SIP Security module.
 *          The IMS application should use this function when working with
 *          the Security module. The application should call this function from
 *          the RvSipTransportMsgReceivedEv() callback for a message received
 *          on a connection. This is to verify the identity of the message
 *          sender against the identity of the user with which the security
 *          owner established the protection. Calling this function from
 *          the RvSipTransportMsgReceivedEv() callback prevents further message
 *          processing in case the check fails.
 *          If the function returns NULL in the ppSecOwner parameter,this means
 *          that the message was not protected by the IMS security-object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn      - The connection handle
 * Output:  ppSecOwner - The pointer to the Security Object,
 *                       owning the Connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetSecOwner(
                        IN  RvSipTransportConnectionHandle  hConn,
                        OUT void**                          ppSecOwner);

/*-----------------------------------------------------------------------*/
/*                TRANSPORT TLS API                                      */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransportConnectionGetCurrentTlsState
 * ------------------------------------------------------------------------
 * General: retrieves the connection current TLS state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 * Output:  peState    - The connection current TLS state
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetCurrentTlsState(
                        IN RvSipTransportConnectionHandle hConn,
                        OUT RvSipTransportConnectionTlsState* peState);

/***************************************************************************
 * RvSipTransportTlsEngineConstruct
 * ------------------------------------------------------------------------
 * General: Constructs a TLS engine.
 *          Tls Engine is an entity that holds together a number of characteristics
 *          related to TLS sessions. When making a TLS handshake you have to
 *          provide an engine. The handshake parameters will be derived from the
 *          engines parameters.
 *          For example, you can create a "TLS client" engine by calling
 *          RvSipTransportTlsEngineAddTrastedCA() after an engine has been
 *          constructed.
 *          Once an engine has been constructed it can be used to preform TLS
 *          handshakes. A handshake that uses an engine will 'inherit' it TLS
 *          characteristics, (e.g. TLS version)
 *
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          pTlsEngineCfg - a pointer to a configuration struct the holds data
 *          for the TLS engine.
 *          sizeofCfg - The size of the configuration structure
 * Output:  phTlsEngine     - a newly creates Tls engine.
 ***************************************************************************/
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineConstruct(
                        IN RvBool                           reqClientCert,
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineCfg*      pTlsEngineCfg,
                        IN RvInt32                         sizeofCfg,
                        OUT RvSipTransportTlsEngineHandle*  phTlsEngine);
#else
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineConstruct(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineCfg*      pTlsEngineCfg,
                        IN RvInt32                         sizeofCfg,
                        OUT RvSipTransportTlsEngineHandle*  phTlsEngine);
#endif

/***************************************************************************
 * RvSipTransportTlsEngineAddCertificateToChain
 * ------------------------------------------------------------------------
 * General: Adds a TLS certificate to chain of certificate. The engine holds
 *          a chain of certificates needed for its approval (usually ending
 *          with a self signed certificate).
 *          The engine will display the chain of certificates during handshakes
 *          in which it is required to present certificates.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 *          strCert       - The certificate encoded as ASN.1 string representation .
 *          certLen       - The length of the certificate
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineAddCertificateToChain(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        IN RvChar                          *strCert,
                        IN RvInt32                         certLen);

/***************************************************************************
 * RvSipTransportTlsEngineAddTrustedCA
 * ------------------------------------------------------------------------
 * General: Adds a trusted certificate authority to an engine.
 *          After that function is used the engine will approve all certificates
 *          issued by the CA.
 *          A CA (Certificate Authority) is an entity that issues certificates.
 *          Most TLS clients on the net trust one or more CAs and approve only
 *          certificates that were issued by those CAs. After adding a trusted CA
 *          to an engine you can use it as a "TLS client" engine and use that
 *          connection on handshakes in which you request the other side of the
 *          connection to display it certificates.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 *          strCert       - The certificate encoded as ASN.1 string representation .
 *          certLen       - The length of the certificate
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineAddTrustedCA(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        IN RvChar                          *strCert,
                        IN RvInt32                         certLen);

/***************************************************************************
 * RvSipTransportTlsEngineGetUnderlyingCtx
 * ------------------------------------------------------------------------
 * General: gets the TLS engine's underlying SSL_CTX* pointer. that pointer 
 *          can be used to aplly direct OpenSSL API on the ctx.
 *          Once an engine has been exposed, the application can set 
 *          different settings to it, but it is not allowed to interfere 
 *          with it's I/O operation's (e.g. accept a session with it)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A handle to Tls engine.
 * Output:  pUnderlyingCTX - the exposed SSL_CTX*
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineGetUnderlyingCtx(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine,
                        OUT void**                          pUnderlyingCTX);

/***************************************************************************
 * RvSipTransportConnectionTlsHandshake
 * ------------------------------------------------------------------------
 * General: starts TLS negotiation on a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start the handshake
 *        hEngine     - The TLS engine that will be associated with the connection.
 *                    The connection will "inherit" the engine's parameters.
 *        eHandshakeSide - the TLS handshake side that the
 *                    connection will play on the TLS handshake. Using
 *                    the default enumeration will set the handshake side to be
 *                    client for TCP clients and Server for TCP servers
 *        pfnVerifyCertEvHandler - callback to check certificates that arrived
 *                    during handshake.
 *                    Client handshake side: NULL means use the default callback - valid
 *                                           certificates will be approved and invalid
 *                                           certificates will be rejected, causing a handshake
 *                                           failure.
 *                                           The callback function supplied here overrides that default.
 *                    Server handshake side: NULL means no client certificates.
 *                                           The callback function supplied here will require
 *                                           client certificate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsHandshake(
                 IN RvSipTransportConnectionHandle         hConnection,
                 IN RvSipTransportTlsEngineHandle          hEngine,
                 IN RvSipTransportTlsHandshakeSide         eHandshakeSide,
                 IN RvSipTransportVerifyCertificateEv      pfnVerifyCertEvHandler);

#if RV_TLS_ENABLE_RENEGOTIATION
/***************************************************************************
 * RvSipTransportConnectionTlsRenegotiate
 * ------------------------------------------------------------------------
 * General: starts TLS renegotiation on a connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start renegotiation
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsRenegotiate(
                 IN RvSipTransportConnectionHandle  hConnection);

/***************************************************************************
 * RvSipTransportConnectionTlsRenegotiationInProgress
 * ------------------------------------------------------------------------
 * General: checks if the TLS connection is being renegotiated
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hConnection - connection on which to start renegotiation
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsRenegotiationInProgress(
                 IN RvSipTransportConnectionHandle  hConnection,
                 OUT RvBool*                        pbInProgress);
#endif /*RV_TLS_ENABLE_RENEGOTIATION*/

/***************************************************************************
 * RvSipTransportConnectionTlsGetEncodedCert
 * ------------------------------------------------------------------------
 * General: Retrieves a certificate from a connection.if the allocated buffer
 * is insufficient, the length of the buffer needed will be inserted in pCertLen.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConnection - connection on to get cert of
 *          pCertLen    - The allocated cert buffer len
 * OutPut:  pCertLen    - The real size of the certificate in case the buffer was not sufficient.
 *          strCert     - an allocated buffer to hold the certificate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsGetEncodedCert(
                         IN    RvSipTransportConnectionHandle         hConnection,
                         INOUT RvInt32                              *pCertLen,
                         OUT   RvChar                               *strCert);

/***************************************************************************
 * RvSipTransportConnectionTlsGetUnderlyingSsl
 * ------------------------------------------------------------------------
 * General: gets the TLS session's underlying SSL* pointer. that pointer 
 *          can be used to aplly direct OpenSSL API on the session.
 *          Once a session has been exposed, the application can set 
 *          different settings to it, but it is not allowed to interfere 
 *          with it's I/O operation's
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConnection - connection on to get cert of
 * OutPut:  pUnderlyingSSL    - the connection's underlying SSL session.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionTlsGetUnderlyingSsl(
                     IN    RvSipTransportConnectionHandle   hConnection,
                     OUT   void**                           pUnderlyingSSL);

/***************************************************************************
 * RvSipTransportTlsEncodeCert
 * ------------------------------------------------------------------------
 * General: Encodes a certificate to a buffer in DER(ASN.1) format.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCert - the certificate to encode
 *          pCertLen - the buffer length
 * Output:  strCert - the certificate encoded into Asn.1 format
 *       :  pCertLen    - The length of the certificate. (bytes)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEncodeCert(
                        IN    RvSipTransportTlsCertificate           hCert,
                        INOUT RvInt32                              *pCertLen,
                        OUT   RvChar                               *strCert);

/***************************************************************************
 * RvSipTransportTlsGetCertVerificationError
 * ------------------------------------------------------------------------
 * General: retrieves an error string in the the verification callback
 * Return Value: the error string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCert - the handle of the certificate
 * Output:  strError - the error string
 ***************************************************************************/
RVAPI RvChar* RVCALLCONV RvSipTransportTlsGetCertVerificationError(
                         IN    RvSipTransportTlsCertificate           hCert,
                         OUT   RvChar                              **strError);

/***************************************************************************
 * RvSipTransportTlsEngineCheckPrivateKey
 * ------------------------------------------------------------------------
 * General: Checks the consistency of a private key with the corresponding
 *          certificate loaded into the engine. If more than one key/certificate pair
 *          (RSA/DSA) is installed, the last item installed will be checked. If e.g.
 *          the last item was a RSA certificate or key, the RSA key/certificate
 *          pair will be checked.
 *          This is a utility function for the application to make sure the key and
 *          certificate were loaded correctly into the engine.
 * Return Value: RV_OK - the key and certificate match
 *               RV_ERROR_UNKNOWN - the key and certificate does not match
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - Handle to the transport manager.
 *          hTlsEngine    - A Tls engine.
 * Output:
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsEngineCheckPrivateKey(
                        IN RvSipTransportMgrHandle          hTransportMgr,
                        IN RvSipTransportTlsEngineHandle    hTlsEngine);

/***************************************************************************
 * RvSipTransportTlsGetSubjectAltDNS
 * ------------------------------------------------------------------------
 * General: The function retrieves a list of the subjectAltNames that are
 *          in a DNS format, from the TLS certificate.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection - The TLS connection.
 *          pBuffer    - A buffer allocated by application, to fill with the 
 *                       function output subjectAltNames list.
 *          pBufferLen - Pointer to an integer containing the size of buffer. 
 * Output:  pBuffer    - The buffer containing a zero-separated list of DNS names
 *          pBufferLen - Contains the requested buffer size (The real size that 
 *                       was used for the list - if there were enough resources,
 *                       or the expected buffer size - in case of out of resource case)
 *          NumOfNamesInList - Pointer to an integer containing the number of
 *                       SubjAltNames in the output list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportTlsGetSubjectAltDNS(
                        IN    RvSipTransportConnectionHandle   hConnection,
                        INOUT RvChar*                          pBuffer,
                        INOUT RvInt32*                         pBufferLen,
                        OUT   RvInt32*                         pNumOfNamesInList);

/*---------------FUNCTIONS FOR INTERNAL USAGE (not to be documented)------------*/

/***************************************************************************
 * RvSipTransportSendObjectEvent
 * ------------------------------------------------------------------------
 * General: Sends an object termination event through the event queue.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr - The transport manager handle.
 *          pObj          - Pointer to the object to be terminated.
 *          pEventInfo    - Pointer to an allocated uninitialized structure
 *                          used for queuing object events.
 *          reason        - event reason
 *          func          - event callback function - this function will be
 *                          called when the event will be pooped
 *                          from the event queue.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportSendObjectEvent(
                                    IN RvSipTransportMgrHandle          hTransportMgr,
                                    IN void*                            pObj,
                                    IN RvSipTransportObjEventInfo*      pEventInfo,
                                    IN RvInt32                          reason,
                                    IN RvSipTransportObjectEventHandler func);

/***************************************************************************
 * RvSipTransportGetNumOfIPv4LocalAddresses
 * ------------------------------------------------------------------------
 * General: Returns the number of local addresses that the stack listens to.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          pNumberOfAddresses - The number of local addresses for which the
 *                              stack listens.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetNumOfIPv4LocalAddresses(
                                     IN   RvSipTransportMgrHandle    hTransportMgr,
                                     OUT  RvUint32                 *pNumberOfAddresses);

/***************************************************************************
 * RvSipTransportGetIPv4LocalAddressByIndex
 * ------------------------------------------------------------------------
 * General: Retrieves the local address by index. Used when the stack was
 *          initialized with IPv4 local address of 0, and therefore listens
 *          on several distinct local addresses.
 *          To know how many local addresses are available by this function
 *          call RvSipTransportGetNumOfIPv4LocalAddresses. If for example this
 *          function returns 5 then you can call
 *          SipTransportGetIPv4LocalAddressByIndex with indexes going from
 *          0 to 4.
 *          Note: The IPv4 address requires 4-BYTEs of memory. This is the
 *          same as an unsigned int (RvUint32). This function requires
 *          pLocalAddr to be a pointer to a 4-BYTE allocated memory.
 *          It can also be a pointer to RvUint32 with an appropriate casting.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          index - The index for the local address to retrieve
 *          pLocalAddr - A pointer to a 4-BYTE memory space to be filled
 *                        with the selected local address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetIPv4LocalAddressByIndex(
                                     IN   RvSipTransportMgrHandle    hTransportMgr,
                                     IN   RvUint                   index,
                                     OUT  RvUint8                  *pLocalAddr);

/***************************************************************************
 * RvSipTransportGetIPv6LocalAddress
 * ------------------------------------------------------------------------
 * General: Retrieves the local address that was actually open for listening
 *          when the stack was initiated with local address 0:0:0:0:0:0:0:0.
 *          Note: The IPv6 address requires 16-BYTEs of memory. This function
 *          requires pLocalAddr to be a pointer to a 16-BYTE allocated memory.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          pLocalAddr  - A pointer to a 16-BYTE memory space to be filled
 *                        with the selected local address.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportGetIPv6LocalAddress(
                                     IN   RvSipTransportMgrHandle    hTransportMgr,
                                     OUT  RvUint8                  *pLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressAdd
 * ------------------------------------------------------------------------
 * General: add new local address,on which Stack will receive and send messages
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 *          eLocationInList - indication, where the new address should be placed
 *                            in the list of local addresses.
 *          hBaseLocalAddr  - An existing address in the list, before or after
 *                            which the new addresses can be added.
 *                            The parameter is meaningless, if eLocationInList
 *                            is not set to RVSIP_PREV_ELEMENT or
 *                            RVSIP_NEXT_ELEMENT.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle hTransportMgr,
                        IN  RvSipTransportAddr      *pAddressDetails,
                        IN  RvUint32                addrStructSize,
                        IN  RvSipListLocation       eLocationInList,
                        IN  RvSipTransportLocalAddrHandle hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle *phLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressRemove
 * ------------------------------------------------------------------------
 * General: remove the local address,on which Stack receives and sends messages
 *          The socket will be closed immediately.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the Transport Manager
 *          hLocalAddr      - handle to the address to be removed
 * Output : none
*****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressRemove(
                            IN  RvSipTransportMgrHandle         hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressFind
 * ------------------------------------------------------------------------
 * General: Finds the local address in the SIP  Stack that matches the details
 *          supplied by pAddressDetails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          addrStructSize  - size of the structure with details.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressFind(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr              *pAddressDetails,
                        IN  RvUint32                        addrStructSize,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: Returns the details of the local address, the handle of which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - handle of the Local Address.
 *          addrStructSize  - size of the structure with details.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        IN  RvUint32                        addrStructSize,
                        OUT RvSipTransportAddr              *pAddressDetails);

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetFirst
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
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetFirst(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransport                  eTransportType,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetNext
 * ------------------------------------------------------------------------
 * General: Gets the handle of the local address that is located in the list of
 *          local addresses next to the address whose handle is supplied
 *          as a parameter to the function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBaseLocalAddr  - The handle to the local address that is located
 *                            before the requested address.
 * Output : phLocalAddr     - pointer to the memory, where the handle of
 *                            the found address will be stored by the function.
 *                            NULL will be stored, if no matching address
 *                            was found.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetNext(
                        IN  RvSipTransportLocalAddrHandle   hBaseLocalAddr,
                        OUT RvSipTransportLocalAddrHandle   *phLocalAddr);

/******************************************************************************
 * RvSipTransportMgrLocalAddressSetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Sets the IP_TOS socket option when the value is in decimal form.
 *          Note that the option does not provide QoS functionality
 *          in operation systems that support a more powerful DSCP mechanism
 *          in place of the previous TOS byte mechanism.
 *          The function can be called any time during the address life cycle.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - handle to the local address to be updated.
 *          typeOfService   - number, to be set as a TOS byte value.
 * Output : none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetIpTosSockOption(
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN RvInt32                         typeOfService);

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Gets the value of the IP_TOS option that is set for the socket,
 *          which serves the specified local address.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - handle to the local address to be updated.
 * Output : pTypeOfService  - pointer to the memory, where the option value
 *                            will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetIpTosSockOption(
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN RvInt32                         *pTypeOfService);



#ifdef RV_SIP_JSR32_SUPPORT
/******************************************************************************
 * RvSipTransportMgrLocalAddressSetSentBy
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
 * Input:   hLocalAddr   - handle of the local address.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *          strSentByHost  - Null terminated sent-by host string.
 *          sentByPort - the sent by port, or UNDEFINED if the application does
 *                       not wish to set a port in the sent-by.
*****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetSentByString(
										   IN  RvSipTransportLocalAddrHandle   hLocalAddr,
										   IN  RvChar*                         strSentByHost,
										   IN  RvInt32                         sentByPort);

#endif /*#ifdef RV_SIP_JSR32_SUPPORT*/

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetConnection
 * ------------------------------------------------------------------------
 * General: Gets the handle of the listening Connection object,
 *          bound to the Local Address object of TCP/TLS/SCTP type.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - handle to the local address to be updated
 * Output : phConn     - pointer to the memory, where the handle will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetConnection(
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                            OUT RvSipTransportConnectionHandle  *phConn);

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns an handle application to the local address object.
 *          You set this handle in the stack using the
 *          RvSipTransportMgrLocalAddressSetAppHandle() function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 * Output : phAppLocalAddr - The application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetAppHandle(
                                IN  RvSipTransportLocalAddrHandle     hLocalAddr,
                                OUT RvSipTransportLocalAddrAppHandle *phAppLocalAddr); 

/******************************************************************************
 * RvSipTransportMgrLocalAddressSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets an handle application to the local address object.
 *          You get this handle in the stack using the
 *          RvSipTransportMgrLocalAddressGetAppHandle() function.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 *			hAppLocalAddr  - The set application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetAppHandle(
                                IN  RvSipTransportLocalAddrHandle     hLocalAddr,
                                IN  RvSipTransportLocalAddrAppHandle  hAppLocalAddr); 

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetSockAddrType
 * ------------------------------------------------------------------------
 * General: gets type of addresses (IPv4/IPv6), supported by the Local Address
 *          socket.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr     - Handle to the local address.
 *			hAppLocalAddr  - The set application transport manager handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetSockAddrType(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvSipTransportAddressType*     peSockAddrType); 

/******************************************************************************
 * RvSipTransportMgrLocalAddressGetSecOwner
 * ----------------------------------------------------------------------------
 * General: Retrieves the security owner of the connection.
 *          The security owner is a security-object which is implemented in
 *          the RADVISION IMS SIP Security module.
 *          The IMS application should use this function when working with
 *          the Security module. The application should call this function from
 *          the RvSipTransportMsgReceivedEv() callback for a message received
 *          on a Local Address. This is to verify the identity of the message
 *          sender against the identity of the user with which the security
 *          owner established the protection. Calling this function from
 *          the RvSipTransportMsgReceivedEv() callback prevents further message
 *          processing in case the check fails.
 *          If the function returns NULL in the ppSecOwner parameter,this means
 *          that the message was not protected by the IMS security-object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - The connection handle
 * Output:  ppSecOwner - The pointer to the Security Object,
 *                       owning the Connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetSecOwner(
                        IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                        OUT void**                         ppSecOwner);

/******************************************************************************
 * RvSipTransportConnectionSetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Sets the IP_TOS socket option for the socket serving the connection
 *          The option value is in decimal form.
 *          Note that the option does not provide QoS functionality
 *          in operation systems that support a more powerful DSCP mechanism
 *          in place of the previous TOS byte mechanism.
 *          The function can be called any time during the address life cycle.
 *
 * Return Value: RV_OK on success, error code otherwise (see RV_SIP_DEF.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hConn         - handle to the connection to be updated.
 *         typeOfService - number to be set as a TOS byte value.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetIpTosSockOption(
                            IN  RvSipTransportConnectionHandle  hConn,
                            IN  RvInt32                         typeOfService);


/******************************************************************************
 * RvSipTransportConnectionGetIpTosSockOption
 * ------------------------------------------------------------------------
 * General: Gets the value of the IP_TOS option that is set for the socket,
 *          which serves the specified connection.
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hConn           - handle to the connection to be updated.
 * Output: pTypeOfService  - pointer to the memory, where the option value
 *                           will be stored
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetIpTosSockOption(
                            IN  RvSipTransportConnectionHandle  hConn,
                            IN  RvInt32                         *pTypeOfService);

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransportConnectionSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message
 *          on the Connection, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hConn      - Handle to the Connection object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionSetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/******************************************************************************
 * RvSipTransportConnectionGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message
 *          on the Connection, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hConn      - Handle to the Connection object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

/***************************************************************************
 * RvSipTransportMgrLocalAddressCloseSocket
 * ------------------------------------------------------------------------
 * General: Closes the socket of the given Local Address object
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - handle to the Local Address object,
 *                       socket of which should be closed.
 * Output : none.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressCloseSocket(
                                IN RvSipTransportLocalAddrHandle   hLocalAddr);
/***************************************************************************
 * RvSipTransportConnectionEnableConnByAlias
 * ------------------------------------------------------------------------
 * General: This function enables a server connection for further reuse, by
 *          inserting it to the connections hash table. 
 *          When an incoming request has an alias parameter in it's top most
 *          via header, the callback RvSipTransportConnectionServerReuseEv()
 *          is called. in this callback application should authorize this
 *          connection, and if authorized, it should use this function to
 *          enable it for sending future requests.
 *          the connection is identified by an alias string, given in the 
 *          top most via header. 
 *          
 *          (The function is different from RvSipTransportConnectionEnable, 
 *          because it uses the alias name of the connection, and not the remote address)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionEnableConnByAlias(
                                        IN  RvSipTransportConnectionHandle hConn);

/***************************************************************************
 * RvSipTransportConnectionGetAlias
 * ------------------------------------------------------------------------
 * General: This function retrieves the connection alias string.
 *          The function always retrieves the alias string length. 
 *          If an allocated buffer is also given, and allocatedBufferLen>0, 
 *          the alias string will be copy to the buffer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn       - The connection handle
 *            allocatedBufferLen - length of the given allocated buffer.
 * Output:    pAliasLength - length of the alias string. (0 if not exists)
 *            pBuffer      - The buffer filled with the alias string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetAlias(
                                        IN  RvSipTransportConnectionHandle hConn,
                                        IN  RvInt32                         allocatedBufferLen,
                                        OUT RvInt32                        *pAliasLength,
                                        OUT RvChar                         *pBuffer);

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransportMgrLocalAddressGetContext
 * ----------------------------------------------------------------------------
 * General: The IMS application should use this function when working with
 *          the RADVISION IMS SIP Security-Agreement module. This function
 *          returns the context (sizeof(void*) bytes) that was set by
 *          the application into the security-agreement object which uses
 *          the local address for message protection. The context is considered
 *          to be an IMPI-identifier of the IMS client. Upon message reception,
 *          the application has to verify the identity of the message sender
 *          against the identity of the user, with which the security-agreement
 *          was performed. Therefore, the application should call
 *          the RvSipTransportMgrLocalAddressGetContext() function from
 *          the RvSipTransportMsgReceivedEv() callback to prevent message
 *          processing if the verification fails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - Handle to the local address.
 * Output : pContext   - The context.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressGetContext(
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                            OUT void**                          pContext);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransportConnectionGetContext
 * ------------------------------------------------------------------------
 * General: The IMS application should use this function when working with
 *          the RADVISION IMS SIP Security-Agreement module. This function
 *          returns the context (sizeof(void*) bytes) that was set by
 *          the application into the security-agreement object which uses
 *          the connection for message protection. The context is considered
 *          to be an IMPI-identifier of the IMS client. Upon message reception,
 *          the application has to verify the identity of the message sender
 *          against the identity of the user, with which the security-agreement
 *          was performed. Therefore, the application should call
 *          the RvSipTransportMgrLocalAddressGetContext() function from
 *          the RvSipTransportMsgReceivedEv() callback to prevent message
 *          processing if the verification fails.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn    - Handle to the connection.
 * Output : pContext - The context.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetContext(
                                IN  RvSipTransportConnectionHandle hConn,
                                OUT void**                         pContext);
#endif /*#ifdef RV_SIP_IMS_ON*/

/******************************************************************************
 * RvSipTransportAddr2RvAddress
 * ----------------------------------------------------------------------------
 * General: Converts RvSipTransportAddr structure to RvAddress structure.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAddressDetails - RvSipTransportAddr structure.
 * Output:  pRvAddress      - RvAddress to be filled.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConvertTransportAddr2RvAddress(
                                    IN  RvSipTransportAddr  *pAddressDetails,
                                    OUT RvAddress           *pRvAddress);

/******************************************************************************
 * RvSipTransportConvertRvAddress2TransportAddr
 * ----------------------------------------------------------------------------
 * General: converts RvAddress object structure to RvSipTransportAddr structure.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pRvAddress      - RvSipTransportAddr structure.
 * Output:  pTransportAddr  - RvSipTransportAddr to be filled.
 *****************************************************************************/
RVAPI void RVCALLCONV RvSipTransportConvertRvAddress2TransportAddr(
                                      IN  RvAddress          *pRvAddress,
                                      OUT RvSipTransportAddr *pTransportAddr);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * RvSipTransportPQPrintOORlist
 * ----------------------------------------------------------------------------------
 * General: prints objects handles from the OOR list to the log.
 *          this function may be called by the application in order to have a better
 *          view of the out of resource status.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Transport manager handler
 *          
 * Output:  none
 ***********************************************************************************/
RVAPI void RVCALLCONV RvSipTransportPrintOORlist(IN  RvSipTransportMgrHandle   hTransportMgr);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * RvSipTransportConnectionGetClientLocalPort
 * ------------------------------------------------------------------------
 * General: The function returns the  port used by the local socket after
 *          establishing the given client-side connection object. 
 *
 * NOTE: This function might return different port number than the API 
 *       RvSipTransportConnectionGetLocalAddress(), since this function
 *       returns the actual port, bound to the client-side connection socket. 
 *       This port number might be an ephemeral port number.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h, rverror.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to a client-side connection.
 *
 * Output : pPort - The local port of the given connection
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransportConnectionGetClientLocalPort(
                                IN  RvSipTransportConnectionHandle  hConn,
                                OUT RvUint16                       *pPort); 

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* TransportError handlers 
*/
#if defined(UPDATED_BY_SPIRENT)

RVAPI void RVCALLCONV RvSipTransportErrorSetEvHandlers(
                                    IN RvSipTransportErrorEvHandlers* hErrorEvHandlers);
RVAPI void RVCALLCONV RvSipTransportErrorRejectMsgEvHandler(
                                    IN  RvChar   *pEventTextMsg);

int RvSipUdpPortIsAvailable(RvSipTransportMgrHandle            transportMgr,
                         RvSipTransportAddr *pLocalAddr, 
                         int port);

RVAPI RvStatus RVCALLCONV RvSipTransportMgrLocalAddressSetSockSize(
								   IN RvSipTransportLocalAddrHandle   hLocalAddr,
								   IN RvUint32                        sockSize);

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RVSIP_TRANSPORT_H */

