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
 *                              <RvSipTransmitter.h>
 *
 * This file contains all API function of the transmitter module.
 * The Transmitter functions of the RADVISION SIP stack enable you to create and
 * manage Transmitter objects, and use them to send sip messages.
 * Each transmitter can be used for sending a single sip message.
 * The transmitter performs the address resolution according to the message
 * object and other parameters such as the transmitter outbound address.
 *
 * The Transmitter Manager API
 * ------------------------
 * The Transmitter manager is used mainly for creating
 * new Transmitters.
 *
 * The Transmitter API
 * -----------------
 * A Transmitter object is used for sending a single SIP message (request
 * or response). It will usually be used by proxies to send out of context
 * messages such as ACK or responses that do not match any transaction.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                January 2004
 *********************************************************************************/


#ifndef RV_SIP_TRANSMITTER_H
#define RV_SIP_TRANSMITTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipMsgTypes.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransportDNSTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecurityTypes.h"
#endif
    


/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER MANAGER API FUNCTIONS                 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipTransmitterMgrCreateTransmitter
 * ------------------------------------------------------------------------
 * General: Creates a new Transmitter and exchanges handles with the
 *          application.  The new Transmitter assumes the IDLE state.
 *          The new transmitter can be used for sending only one message.
 *          The message can be a sip request a sip response or a non sip message.
 *          After creating the transmitter you can set different parameters
 *          to the transmitter with the transmitter Set functions. You can
 *          then use RvSipTransmitterSendMessage to send sip messages to 
 *          the remote party. In that case the remote address is calculated 
 *          from the message object along with the transmitter parameters.
 *          You can also use RvSipTransmitterSendBuffer() to send non sip
 *          messages. In that case you also need to supply the destination
 *          address.
 *          When creating a transmitter you should supply the transmitter
 *          event handlers in order to be notified of transmitter events.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr - Handle to the Transmitter manager
 *            hAppTrx - Application handle to the newly created Transmitter.
 *            pEvHanders  - Event handlers structure for this transmitter.
 *            sizeofEvHandlers - The size of the Event handler structure
 * Output:    phTrx   -  Handle to the newly created Transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrCreateTransmitter(
                   IN  RvSipTransmitterMgrHandle   hTrxMgr,
                   IN  RvSipAppTransmitterHandle   hAppTrx,
                   IN  RvSipTransmitterEvHandlers* pEvHandlers,
                   IN  RvInt32                     sizeofEvHandlers,
                   OUT RvSipTransmitterHandle*     phTrx);

/***************************************************************************
 * RvSipTransmitterMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own Transmitter manager handle.
 *          You can use the RvSipTransmitterMgrSetAppMgrHandle function to
 *          save this handle in the stack Transmitter manager object.
 *          At any given time you can get this handle by calling the 
 *          RvSipTransmitterMgrGetAppMgrHandle() function.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr            - Handle to the stack Transmitter manager.
 *            pAppTransmitterMgr - The application Transmitter manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetAppMgrHandle(
                          IN RvSipTransmitterMgrHandle hTrxMgr,
                          IN void*                     pAppTransmitterMgr);


/***************************************************************************
 * RvSipTransmitterMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application Transmitter manger object.
 *          You set this handle in the stack using the
 *          RvSipTransmitterMgrSetAppMgrHandle() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTrxMgr            - Handle to the stack Transmitter manager.
 * Output:     pAppTransmitterMgr - The application Transmitter manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetAppMgrHandle(
                                   IN RvSipTransmitterMgrHandle hTrxMgr,
                                   OUT void**               pAppTransmitterMgr);
/***************************************************************************
 * RvSipTransmitterMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this Transmitter
 *          manager belongs to. The stack handle is returned as a void pointer.
 *          In order to use it you should cast it to the RvSipStackHandle type.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTrxMgr     - Handle to the stack Transmitter manager.
 * Output:     phStackInstance - The stack handle as a void pointer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetStackInstance(
                      IN   RvSipTransmitterMgrHandle   hTrxMgr,
                      OUT  void*                      *phStackInstance);


/************************************************************************************
 * RvSipTransmitterMgrSetDnsServers
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS servers. please use the RvSipResolverMgrSetDnsServers()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - The transmitter manager
 *          pDnsServers      - A list of addresses of DNS servers to set to the stack
 *          numOfDnsServers  - The number of DNS servers in the list
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetDnsServers(
                              IN  RvSipTransmitterMgrHandle hTrxMgr,
                              IN  RvSipTransportAddr*       pDnsServers,
                              IN  RvInt32                   numOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

/************************************************************************************
 * RvSipTransmitterMgrGetDnsServers
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS servers. please use the RvSipResolverMgrGetDnsServers()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetDnsServers(
                              IN     RvSipTransmitterMgrHandle hTrxMgr,
                              INOUT  RvInt32*                  pNumOfDnsServers,
                              OUT    RvSipTransportAddr*       pDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

/************************************************************************************
 * RvSipTransmitterMgrSetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS Domains. please use the RvSipResolverMgrSetDnsDomains()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetDnsDomains(
                      IN  RvSipTransmitterMgrHandle   hTrxMgr,
                      IN  RvChar**                    pDomainList,
                      IN  RvInt32                     numOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

/************************************************************************************
 * RvSipTransmitterMgrGetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS Domains. please use the RvSipResolverMgrGetDnsDomains()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 * Output:  numOfDomains     - The size of the pDomainList array, will reteurn the number
 *                             of domains set to the stack
 *          pDomainList      - A list of DNS domains. (an array of char pointers).
 *                             that array will be filled with pointers to DNS domains.
 *                             The list of DNS domain is part of the stack memory, if
 *                             the application wishes to manipulate that list, It must copy
 *                             the strings to a different memory space.
 *                             The size of this array must be no smaller than numOfDomains
 *          
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetDnsDomains(
                      IN     RvSipTransmitterMgrHandle   hTrxMgr,
                      INOUT  RvInt32*                    pNumOfDomains,
                      OUT    RvChar**                    pDomainList
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

/*-----------------------------------------------------------------------
       T R A N S M I T T E R    C O N T R O L   A P I
 ------------------------------------------------------------------------*/
/************************************************************************************
 * RvSipTransmitterSendMessage
 * ----------------------------------------------------------------------------------
 * General: Sends a SIP message to the remote party. The application should supply
 *          the message object that it wishes the transmitter to send.
 *          In order to send the message the transmitter has to resolve the destination
 *          address. The transmitter first moves to the RESOLVING_ADDR
 *          state and starts the address resolution process.
 *          The transmitter calculates the remote address of the message
 *          according to RFC 3261 and RFC 3263 and takes into account the existence of
 *          outbound proxy, Route headers and loose routing rules.
 *          Once address resolution is completed the transmitter moves
 *          to the FINAL_DEST_RESOLVED state. This state is the last chance for
 *          the application to modify the Via header. The transmitter will then 
 *          move to the READY_FOR_SENDING state and will try to send the message. 
 *          If the message was sent successfully the transmitter moves to the MSG_SENT 
 *          state. If the transmitter fails to send the message it will move to 
 *          the MSG_SEND_FAILURE state.
 *          The RvSipTransmitterSendMessage() function can be called in two states:
 *          The IDLS state for initial sending and the MSG_SEND_FAILURE state in order
 *          to send the message to the next address in the DNS list in case the previos
 *          address failed.
 *          
 *          Note1: The DNS procedure is a-synchronic.
 *          Note2: If you wish the transmitter to fix the top via header of the message
 *          object according to the remote party address and transport types you should
 *          first call the RvSipTransmitterSetFixViaFlag() function. Otherwise the
 *          transmitter will only fix the via transport and will add the rport parameter
 *          in case it was configured to the stack. The send-by parameter will remain
 *          untouched.
 *          
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmiter handle
 *          hMsgToSend     - The message to send.
 *          bHandleTopVia  - Indicates whether the transmitter should
 *                           add a top Via header to request messages and
 *                           remove the top Via from response messages.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendMessage (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN RvBool                    bHandleTopVia);

/************************************************************************************
 * RvSipTransmitterRetransmitMessage
 * ----------------------------------------------------------------------------------
 * General: Retransmits the transmitter internal message.
 *          After the transmitter has successfully sent its initial SIP message, the
 *          application can instruct the transmitter to retransmit the message to the
 *          same destination. The transmitter will send its internal encoded message.
 *          Note: If there is no encoded message in the transmitter, RV_OK will be returned
 *          and the transmitter will not send any message.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterRetransmitMessage (
                                  IN RvSipTransmitterHandle    hTrx);

#ifdef RV_SIP_JSR32_SUPPORT
/************************************************************************************
 * RvSipTransmitterSendMessageToDestAddr
 * ----------------------------------------------------------------------------------
 * General: Sends a message to the destination address exists in the transmitter.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendMessageToDestAddr (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend);

#endif /*#ifdef RV_SIP_JSR32_SUPPORT*/

/************************************************************************************
 * RvSipTransmitterSendBuffer 
 * ----------------------------------------------------------------------------------
 * General: Sends a buffer to a specified destination.
 *          Using this function the application can send non sip messages to a
 *          desirable destination. Before calling this function the application
 *          has to set the destination address to the transmitter using the 
 *          RvSipTransmitterSetDestAddress() API. After the message is sent
 *          successfully the transmitter assumes the MSG_SENT
 *          state. If the transmitter fails to send the message it will assume
 *          the MSG_SEND_FAILURE state.
 *          In both cases it is the application's responsibility to terminate
 *          the transmitter.
 *          Note: If this function returns with an error, the transmitter will not move
 *          to the MSG_SEND_FAILURE state.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          strBuff      - The buffer to send.
 *          buffSize     - The buffer size
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendBuffer (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvChar*                   strBuff,
                                  IN RvInt32                   buffSize);


/************************************************************************************
 * RvSipTransmitterHoldSending
 * ----------------------------------------------------------------------------------
 * General: Holds all sending activity of the transmitter object and moves the transmitter
 *          to the ON_HOLD state.
 *          After address resolution is completed and before the message is sent
 *          the transmitter moved to the FINAL_DEST_RESOLVED state. At this
 *          state the application can hold the message sending by calling
 *          RvSipTransmitterHoldSending(). If the application wishes it can 
 *          change the remote address at this point using the 
 *          RvSipTransmitterSetDestAddress() function and manipulate the rest of
 *          the DNS list using the transport layer API.
 *          If you wish the transmitter to use the next element is the list you
 *          can use the RvSipTransmitterSetDestAddress() function to reset the
 *          current destination address. The transmitter will then repeat the
 *          address resolution process before sending the message.
 *          It is the responsibility of the application to resume
 *          the sending of the message using RvSipTransmitterResumeSending().
 *          
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - The transmiter handle
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterHoldSending (
                                  IN RvSipTransmitterHandle    hTrx);


/************************************************************************************
 * RvSipTransmitterResumeSending
 * ----------------------------------------------------------------------------------
 * General: Resumes sending activities of the transmitter. This function can be called only
 *          in the ON_HOLD state of the transmitter. When this function is called
 *          the transmitter first checks that a destination address exists.
 *          If so, the transmitter moves to the READY_FOR_SENDING state and then
 *          sends the message to this address. 
 *          If there is not destination address (the user reset the address
 *          by calling RvSipTransmitterSetDestAddress() with NULL values), the transmitter
 *          returns to the RESOLVING_ADDR state and to the address resolution process.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx        - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterResumeSending(
                             IN RvSipTransmitterHandle    hTrx);

/***************************************************************************
 * RvSipTransmitterTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a Transmitter and free all transmitter allocated
 *          resources. The transmitter will assume the TERMINATED
 *          state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - The transmitter handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterTerminate (
                                            IN  RvSipTransmitterHandle   hTrx);
/*-----------------------------------------------------------------------
          T R N S M I T T E R:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipTransmitterSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the transmitter will send 
 *          outgoing messages.
 *          The SIP Stack can be configured to listen to many local addresses. Each local
 *          address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 *          When the SIP Stack sends an outgoing message, the local address (from where
 *          the message is sent) is chosen according to the characteristics of the remote
 *          address. Both the local and remote addresses must have the same characteristics
 *          (transport type and address type). If several configured local addresses match
 *          the remote address characteristics, the first configured address is taken. You can
 *          use RvSipTransmitterSetLocalAddress() to force the transmitter to choose a specific
 *          local address for a specific transport and address type. For example, you can
 *          force the transmitter to use the second configured UDP/ IPv4 local address. If the
 *          transmitter sends a message to a UDP/IPv4 remote address, it will use the local
 *          address that you set instead of the default first local address.
 *          Note:   The localAddress string you provide to this function must match exactly
 *                  with the local address that was inserted in the configuration structure in the
 *                  initialization of the SIP Stack. If you configured the SIP Stack 
 *                  to listen to a 0.0.0.0
 *                  local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx            - Handle to the Transmitter
 *            eTransportType  - The transport type(UDP, TCP,TLS).
 *            eAddressType    - The address type(IPv4 or IPv6).
 *            strLocalAddress - The local address to be set to this Transmitter.
 *            localPort       - The local port to be set to this Transmitter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar*                   strLocalAddress,
                            IN  RvUint16                  localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar*                 if_name
			    , IN  RvUint16                vdevblock
#endif
			    );


/***************************************************************************
 * RvSipTransmitterGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address which the transmitter will use to send 
 *          outgoing messages to a destination that listens to a specific transport 
 *          and address type.
 *          This is the address the user set using the RvSipTransmitterSetLocalAddress()
 *          function. If no address was set, the function will return the default, first
 *          configured local address of the requested transport and address type.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx            - Handle to the Transmitter
 *          eTransportType  - The transport type (UDP, TCP, TLS).
 *          eAddressType    - The address type (IPv4 or IPv6).
 * Output:  strLocalAddress - The local address this Transmitter is using.
 *          pLocalPort      - The local port this Transmitter is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT RvChar*                   strLocalAddress,
                            OUT RvUint16*                 pLocalPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar*                if_name
			    , OUT  RvUint16*              vdevblock
#endif
			    );



/***************************************************************************
 * RvSipTransmitterSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the Transmitter object. 
 *          All detailes are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          Request sent by this object will use the outbound detail specifications
 *          as a remote address. The Request-URI will be ignored.
 *          However, the outbound proxy will be ignored if the message contains
 *          a Route header or if the RvSipTransmitterSetIgnoreOutboundProxyFlag() function
 *          was called.
 *
 *          Note If you specify both the IP address and a host name in the configuration
 *          structure, either of them will be set BUT the IP address will be used.
 *          If you do not specify port or transport, both will be determined according 
 *          to the DNS procedures specified in RFC 3263. 
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx           - Handle to the Transmitter.
 *            pOutboundCfg   - Pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetOutboundDetails(
                            IN  RvSipTransmitterHandle         hTrx,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                        cfgStructSize);

/***************************************************************************
 * RvSipTransmitterSetSkipViaProcessingFlag
 * ------------------------------------------------------------------------
 * General: Indicates that the transmitter should not do any via header
 *          processing even if it belongs to an internal stack object that
 *          is currently sending the message.
 *          This function is for internal use only. It is not documented.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            bSkipViaProcessing - RV_TRUE if the application wishes that the transmitter
 *                       will skip the via processing. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSkipViaProcessingFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bSkipViaProcessing);


/***************************************************************************
 * RvSipTransmitterGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all outbound proxy details that the Transmitter object uses. 
 *          The details are placed in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          If the outbound details were not set to the specific 
 *          Register-Client object but outbound proxy was defined to the stack 
 *          on initialization, the stack parameters will be returned.
 *          If the Transmitter is not using an outbound address NULL/UNDEFINED
 *          values are returned.
 *          NOTE: You must supply a valid consecutive buffer in the 
 *                RvSipTransportOutboundProxyCfg structure to get the
 *                outbound strings (host name and ip address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - Handle to the Transmitter.
 *          sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetOutboundDetails(
                            IN  RvSipTransmitterHandle         hTrx,
                            IN  RvInt32                        cfgStructSize,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);


/***************************************************************************
 * RvSipTransmitterSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the transmitter persistency definition at runtime.
 *          This function receives a Boolean value that indicates whether or not the
 *          application wishes this transmitter to be persistent. 
 *          A persistent transmitter object will try to locate a suitable connection 
 *          in the connection hash before opening a new connection. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx - The Transmitter handle
 *          bIsPersistent - Determines whether the Transmitter will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetPersistency(
                           IN RvSipTransmitterHandle  hTrx,
                           IN RvBool                  bIsPersistent);

/***************************************************************************
 * RvSipTransmitterGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the transmitter persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 * Output:   pbIsPersistent - The transmitter persistency definition. 
 *                            RV_TRUE indicates that the transmitter is
 *                            persistent. Otherwise, RV_FALSE.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransmitterGetPersistency(
                     IN  RvSipTransmitterHandle  hTrx,
                     OUT RvBool*                 pbIsPersistent);

/***************************************************************************
 * RvSipTransmitterSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the transmitter.
 *          The transmitter object will hold this connection in its internal database. 
 *          When sending a message the transmitter will use the connection only if 
 *          it fits the local and remote addresses. 
 *          Otherwise, the transmitter will either locate
 *          a suitable connection in the connection hash or create a new connection. The
 *          transmitter object will informed its owner when using a different
 *          connection then the one that was supplied.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx  - Handle to the transmitter
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetConnection(
                   IN  RvSipTransmitterHandle           hTrx,
                   IN  RvSipTransportConnectionHandle   hConn);


/***************************************************************************
 * RvSipTransmitterGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the Transmitter.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetConnection(
                            IN  RvSipTransmitterHandle          hTrx,
                            OUT RvSipTransportConnectionHandle *phConn);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV  RvSipTransmitterAddNewTopViaToMsg(IN  RvSipTransmitterHandle          hTrx, 
                                                             IN RvSipMsgHandle hMsg);
#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipTransmitterSetFixViaFlag
 * ------------------------------------------------------------------------
 * General: Indicates that the transmitter should update the sent-by parameter
 *          of the top Via header before sending the message. The sent-by should be
 *          updated according to the local address that the request will be sent
 *          from. This address is determined according to the remote address transport 
 *          and address types. In many cases the application
 *          does not know in advance the remote party IP, transport and address
 *          types and therefor cannot know which local address will be used.
 *          In this case the application might want the transmitter
 *          to update the top Via automatically and therefore should call the
 *          RvSipTransmitterSetFixViaFlag() function.
 *          The default value of the fixVia parameter is RV_FALSE and it should remain
 *          RV_FALSE if the application updates the top Via by
 *          itself. 
 *          Note: Regardless of the value of this parameter the transmitter
 *          will update the transport and rport parameter of the top Via header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            bFixVia - RV_TRUE if the application wishes that the transmitter
 *                      will update the sent-by in the top Via header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetFixViaFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bFixVia);


/***************************************************************************
 * RvSipTransmitterSetKeepMsgFlag
 * ------------------------------------------------------------------------
 * General: Indicates that the transmitter should not destruct the message
 *          immediately after encoding is completed.
 *          Before the transmitter sends a message it first encodes the message
 *          object to a buffer and then sends the buffer to the remote party.
 *          After encoding is completed, the transmitter destructs the message
 *          object.
 *          In case of send failure the transmitter moves to the MSG_SEND_FAILURE
 *          state. In this state the application can re-send the message object
 *          to the next address in the DNS list using RvSipTransmitterSendMessage().
 *          The application can instruct the transmitter not to destruct the 
 *          message after encoding by setting the KeepMsg flag to RV_TRUE and
 *          in this case it can supply NULL as a message handle to the
 *          RvSipTransmitterSendMessage() function.
 *          The message will be destructed only on termination of the transmitter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -  Handle to the Transmitter.
 *            bKeepMsg - RV_TRUE if the application wishes the
 *                       message to be destructed on terminateion only.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetKeepMsgFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bKeepMsg);


/***************************************************************************
 * RvSipTransmitterSetForceOutboundAddrFlag
 * ------------------------------------------------------------------------
 * General: Sets the force outbound addr flag. This flag forces the transmitter
 *          to send request messages to the outbound address regardless of the
 *          message content.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *     	    bForceOutboundAddr - The flag value to set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetForceOutboundAddrFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bForceOutboundAddr);
/***************************************************************************
 * RvSipTransmitterSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the Transmitter application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            hAppTrx - A new application handle to the Transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetAppHandle (
                                      IN  RvSipTransmitterHandle       hTrx,
                                      IN  RvSipAppTransmitterHandle    hAppTrx);

/***************************************************************************
 * RvSipTransmitterGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this Transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTrx      - Handle to the Transmitter.
 * Output:     phAppTrx  - The application handle of the Transmitter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetAppHandle (
                                      IN    RvSipTransmitterHandle        hTrx,
                                      OUT   RvSipAppTransmitterHandle     *phAppTrx);

/***************************************************************************
 * RvSipTransmitterGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the Transmitter current state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - The Transmitter handle.
 * Output:    peState - The Transmitter current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCurrentState (
                                      IN  RvSipTransmitterHandle   hTrx,
                                      OUT RvSipTransmitterState*   peState);

/***************************************************************************
 * RvSipTransmitterGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle of the stack instance to which this Transmitter
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - Handle to the Transmitter object.
 * Output:    phStackInstance - A void pointer which will be updated with the
 *                             handle of the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetStackInstance(
                                   IN   RvSipTransmitterHandle   hTrx,
                                   OUT  void**                   phStackInstance);
/***************************************************************************
 * RvSipTransmitterSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the Transmitter to a SigComp Compartment.
 *          The transmitter will use this compartment in the message compression
 *          process.
 *          Note: The message will be compresses only if the remote URI
 *                includes the comp=sigcomp parameter.
 *          
 * Return Value: RvStatus
 *
 * NOTE: Function deprecated
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx         - The Transmitter handle.
 *            hCompartment - Handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetCompartment(
                            IN RvSipTransmitterHandle     hTrx,
                            IN RvSipCompartmentHandle hCompartment);

/***************************************************************************
 * RvSipTransmitterDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a Transmitter.
 *          This means that even if the message indicates compression
 *          it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx    - The transmitter handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterDisableCompression(
                                IN  RvSipTransmitterHandle hTrx);


/***************************************************************************
 * RvSipTransmitterGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the SigComp Compartment associated with the Transmitter.
 * Return Value: RvStatus
 *
 * NOTE: Function deprecated
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - The Transmitter handle.
 *         phCompartment - The handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCompartment(
                            IN  RvSipTransmitterHandle      hTrx,
                            OUT RvSipCompartmentHandle *phCompartment);



/***************************************************************************
 * RvSipTransmitterDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves the DNS list object from the transmitter object.
 *          Each transmitter object holds a DNS list object. The DNS list
 *          is updated in the address resolution process. The application
 *          can manipulate the list using the DNS list API.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx      - The Transmitter handle
 * Output     phDnsList - Handle to the DNS list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterDNSGetList(
                                      IN RvSipTransmitterHandle        hTrx,
                                      OUT RvSipTransportDNSListHandle* phDnsList);

/***************************************************************************
 * RvSipTransmitterGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address the Transmitter will use to send   
 *          the message. This address is calculated according to the
 *          remote party transport and address types.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx          - Handle to the transmitter.
 * Output:    eTransporType - The transport type(UDP, TCP, TLS).
 *            eAddressType  - The address type(IPv4 or IPv6).
 *            localAddress  - The local address this transmitter is using
 *            localPort     - The local port this transmitter is using.
 ***************************************************************************/
  RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCurrentLocalAddress(
								   IN  RvSipTransmitterHandle     hTrx,
								   OUT RvSipTransport*            eTransportType,
								   OUT RvSipTransportAddressType* eAddressType,
								   OUT RvChar*                    localAddress,
								   OUT RvUint16*                  localPort
#if defined(UPDATED_BY_SPIRENT)
								   , OUT  RvChar*                 if_name
								   , OUT  RvUint16*               vdevblock
#endif
								   );
#if defined(UPDATED_BY_SPIRENT)
  RvSipTransportLocalAddrHandle RvSipTransmitterGetCurrentLocalAddressHandle(IN  RvSipTransmitterHandle     hTrx);
#endif

/***************************************************************************
 * RvSipTransmitterGetDestAddress
 * ------------------------------------------------------------------------
 * General: Return the destination address the transmitter will use.
 *          When the transmitter assumes the FINAL_DEST_RESOLVED state
 *          the application can get the destination address using the
 *          RvSipTransmitterGetDestAddress() function.
 *          In order to change this address use the RvSipTransmitterSetDestAddress()
 *          function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            addrStructSize    - The size of the pDestAddr struct.
 *            optionsStructSize - The size of the pOptions struct.
 * Output     pDestAddr         - The destination address the transmitter will use.
 *            pOptions          - Advanced instructions for the message sending such
 *                                as using compression
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetDestAddress(
                           IN  RvSipTransmitterHandle      hTrx,
                           IN  RvInt32                     addrStructSize,
                           IN  RvInt32                     optionsStructSize,
                           OUT RvSipTransportAddr*         pDestAddr,
                           OUT RvSipTransmitterExtOptions* pOptions);

/***************************************************************************
 * RvSipTransmitterSetDestAddress
 * ------------------------------------------------------------------------
 * General: Sets the destination address the transmitter will use.
 *          Use this function when you want the transmitter to use
 *          a specific address regardless of the message content.
 *          If NULL is set as the dest address, the stack will continue 
 *          the resolution process according to the DNS list.
 *          
 *          NOTE: Sending the transmitter for another "round" with the 
 *                DNS can take time as DNS requests and responses are exchanged
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            addrStructSize    - The size of the pDestAddr structure.
 *            optionsStructSize - The size of the pOptions structure.
 *            pDestAddr         - The destination address the transmitter will use.
 *                                if set to NULL, the transmitter will 
 *                                continue the DNS procedure as if the dest ip was 
 *                                never retrieved.
 *            pOptions          - Advanced instructions for the message sending such
 *                                as using compression
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetDestAddress(
                          IN RvSipTransmitterHandle        hTrx,
                          IN RvSipTransportAddr*           pDestAddr,
                          IN RvInt32                       addrStructSize,
                          IN RvSipTransmitterExtOptions*   pOptions,
                          IN RvInt32                       optionsStructSize);

/***************************************************************************
 * RvSipTransmitterSetIgnoreOutboundProxyFlag
 * ------------------------------------------------------------------------
 * General:  Instructs the Transmitter to ignore its outbound proxy when sending
 *           requests.
 *           In some cases the application will want the transmitter to 
 *           ignore its outbound proxy event if it is configured to use one.
 *           An example is when the request uri was calculated from a Route
 *           header that was found in the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx                  - The Transmitter handle.
 *          bIgnoreOutboundProxy  - RV_TRUE if you wish the Transmitter to ignored
 *                                  its configured outbound proxy, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetIgnoreOutboundProxyFlag(
                                 IN  RvSipTransmitterHandle hTrx,
                                 IN  RvBool                 bIgnoreOutboundProxy);

/***************************************************************************
 * RvSipTransmitterSetUseFirstRouteFlag
 * ------------------------------------------------------------------------
 * General: Indicates that a message should be sent according the first Route header
 *          and not according to the request uri. This will be the case when
 *          the next hop is a loose router.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx      - The Transmitter handle.
 *            bSendToFirstRoute - RV_TRUE if the message should be sent to the
 *                                first route. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetUseFirstRouteFlag(
                                  IN RvSipTransmitterHandle hTrx,
                                  IN RvBool                 bSendToFirstRoute);

/***************************************************************************
 * RvSipTransmitterSetViaBranch
 * ------------------------------------------------------------------------
 * General: Sets the branch parameter to the transmitter. 
 *          The transmitter will add the branch to the top Via header 
 *          of outgoing requests. If the message already has a branch it
 *          will be replaced with the branch set by this function.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - The Transmitter handle.
 *         strViaBranch  - The Via branch to add to the top via header.
 *                         This parameter is ignored for response messages.
 *         pRpoolViaBranch - The branch supplied on a page. You should set this
 *                         parameter to NULL if the branch was supplied as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetViaBranch(
                                  IN RvSipTransmitterHandle hTrx,
                                  IN RvChar*                strViaBranch,
                                  IN RPOOL_Ptr*             pRpoolViaBranch);


#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransmitterSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets SCTP transport parameters to the transmitter object.
 *          All possible SCTP parameters are located in the 
 *          RvSipTransportSctpMsgSendingParams structure and will be used 
 *          only if the transmitter will use SCTP as the destination transport.
 *          Note: This function is optional. If specific SCTP parameters are
 *          not set, the default SCTP parameters will be used.
 *          Note: Calling this function does not affect the chosen transport.
 *          The transport will be chosen in the address resolution process.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTrx       - Handle to the Transmitter object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the structure that contains all SCTP parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/******************************************************************************
 * RvSipTransmitterGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets SCTP parameters that are used by the transmitter
 *          while sending a message over SCTP.
 *          Note: Only SCTP parameters that were set with the 
 *          RvSipTransmitterSetSctpMsgSendingParams() will be returned when
 *          calling this function.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTrx         - Handle to the Transmitter object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams      - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransmitterSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transmitter.
 *          As a result of this operation, all messages, sent by this
 *          Transmitter, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTrx - Handle to the Transmitter object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSecObj(
                                        IN  RvSipTransmitterHandle    hTrx,
                                        IN  RvSipSecObjHandle         hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransmitterGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Transmitter.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTrx     - Handle to the Transmitter object.
 *  Output: phSecObj - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetSecObj(
                                    IN  RvSipTransmitterHandle    hTrx,
                                    OUT RvSipSecObjHandle*        phSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * RvSipTransmitterMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets the application event handlers for the transmitter manager
 *          object. 
 *          The application has to supply a structure that contains
 *          function pointers to the transmitter manager events that it
 *          wishes to listen to.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr - A handle to the Transmitter manager.
 *          pEvHandlers  - A pointer to the structure containing application 
 *                       event handler pointers.
 *          evHandlerStructSize - The size of the pEvHandlers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetEvHandlers(
                        IN RvSipTransmitterMgrHandle        hTrxMgr,
                        IN RvSipTransmitterMgrEvHandlers      *pEvHandlers,
                        IN RvUint32                        evHandlerStructSize);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RvSipTransmitterUpdateViaByFinalLocalAddress(IN RvSipTransmitterHandle pTrx,
                                                      RvSipMsgHandle hMsgToSend,
                                                      RvSipTransport eTransportType);
#endif
/* SPIRENT END */
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_TRANSMITTER_H */

