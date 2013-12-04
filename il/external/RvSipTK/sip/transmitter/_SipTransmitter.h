
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
 *                              <_SipTransmitter.h>
 *
 *  The _SipTransmitter.h file contains Internal API functions of the
 *  Transmitter layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                 January 2004
 *********************************************************************************/


#ifndef SIP_TRANSMITTER_H
#define SIP_TRANSMITTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitter.h"
#include "TransmitterObject.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER FUNCTIONS                             */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipTransmitterRetransmit
 * ------------------------------------------------------------------------
 * General: Retransmit the last message sent by the transmitter, if exists.
 *          The transmitter holds the last message it sent in encoded format,
 *          and also the address it sent this message to (after DNS).
 *          This function will send this encoded message to this address
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterRetransmit(
                          IN RvSipTransmitterHandle        hTrx);

/***************************************************************************
 * SipTransmitterMoveToMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Mark the transmitter that it should move to msg send failure
 *          when the state changed callback returns from the final dest
 *          resolved state.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 ***************************************************************************/
void RVCALLCONV SipTransmitterMoveToMsgSendFailure(
                          IN RvSipTransmitterHandle        hTrx);


/***************************************************************************
 * SipTransmitterGetDestAddr
 * ------------------------------------------------------------------------
 * General: Gets the destination address the transmitter will use.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            pDestAddr         - destination addr
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetDestAddr(
                          IN RvSipTransmitterHandle        hTrx,
                          IN SipTransportAddress*          pDestAddr);

/***************************************************************************
 * SipTransmitterSetTripleLock
 * ------------------------------------------------------------------------
 * General: sets the transmitter triple lock
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - the transmitter handle
 *           pTripleLock - pointer to a triple lock.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetTripleLock(
                          IN RvSipTransmitterHandle  hTrx,
                          IN SipTripleLock*          pTripleLock);

/***************************************************************************
 * SipTransmitterDestruct
 * ------------------------------------------------------------------------
 * General: Free the Transmitter resources and terminate the transmitter
 *          without sending it to the event queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter to destruct
 ***************************************************************************/
void RVCALLCONV SipTransmitterDestruct(
                          IN RvSipTransmitterHandle hTrx);

/***************************************************************************
 * SipTransmitterSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx  - Handle to the transmitter
 *          hConn - Handle to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetConnection(
                   IN  RvSipTransmitterHandle           hTrx,
                   IN  RvSipTransportConnectionHandle   hConn);

/***************************************************************************
 * SipTransmitterSetPersistency
 * ------------------------------------------------------------------------
 * General: Sets the persistency definition of the transmitter
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx - The Transmitter handle
 *          bIsPersistent - persistency definition
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetPersistency(
                           IN RvSipTransmitterHandle  hTrx,
                           IN RvBool                  bIsPersistent);


/***************************************************************************
 * SipTransmitterGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the transmitter persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 ***************************************************************************/
RvBool RVCALLCONV SipTransmitterGetPersistency(
                     IN  RvSipTransmitterHandle  hTrx);


/***************************************************************************
 * TransmitterDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from its connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - pointer to the Transmitter
 ***************************************************************************/
void SipTransmitterDetachFromConnection(IN  RvSipTransmitterHandle  hTrx);

/***************************************************************************
 * SipTransmitterDetachFromAres
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the ARES if it is in the middle of 
 *          resolution process (so no ARES events will be called).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - pointer to the Transmitter
 ***************************************************************************/
void SipTransmitterDetachFromAres(IN  RvSipTransmitterHandle  hTrx);


/***************************************************************************
 * SipTransmitterDontCopyMsg
 * ------------------------------------------------------------------------
 * General: Indicates not to copy the message of application transmitter.
 *          This function will be used by standalone transmitters that are
 *          created by the stack.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 ***************************************************************************/
void RVCALLCONV SipTransmitterDontCopyMsg(
                     IN  RvSipTransmitterHandle  hTrx);


/************************************************************************************
 * SipTransmitterCopy
 * ----------------------------------------------------------------------------------
 * General: Copies relevant information from the source transmitter
 *          to the destination transmitter
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hDestTrx      - Destination transmitter
 *          hSrcTrx       - Source transmitter
 *          bCopyDnsList - Indicates whether to copy the DNS list or not.
 *          
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterCopy(
                     IN  RvSipTransmitterHandle hDestTrx,
                     IN  RvSipTransmitterHandle hSrcTrx,
                     IN  RvBool                 bCopyDnsList);



                     
/***************************************************************************
 * SipTransmitterSetOutboundAddsStructure
 * ------------------------------------------------------------------------
 * General: Set the outbound address structure of the transmitter
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the transaction
 *            outboundAddr  - The outbound proxy to set.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundAddsStructure(
                            IN  RvSipTransmitterHandle           hTrx,
                            IN  SipTransportOutboundAddress *outboundAddr);
                     
                     
                     
/***************************************************************************
 * SipTransmitterGetOutboundAddsStructure
 * ------------------------------------------------------------------------
 * General: Get the outbound address structure of the transmitter
 * Return Value: outboundAddr.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the transaction
 ***************************************************************************/
SipTransportOutboundAddress* RVCALLCONV SipTransmitterGetOutboundAddsStructure(
                            IN  RvSipTransmitterHandle       hTrx);
                     
/***************************************************************************
 * SipTransmitterSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the transmitter will send outgoing requests.
 * The SIP Stack can be configured to listen to many local addresses. Each local
 * address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 * When the SIP Stack sends an outgoing request, the local address (from where
 * the request is sent) is chosen according to the characteristics of the remote
 * address. Both the local and remote addresses must have the same characteristics,
 * such as the same transport type and address type. If several configured local
 * addresses match the remote address characteristics, the first configured address
 * is taken.
 * You can use SipTransmitterSetLocalAddress() to force the transmitter to
 * choose a specific local address for a specific transport and address type. For
 * example, you can force the transmitter to use the second configured UDP/
 * IPv4 local address. If the transmitter sends a request to a UDP/IPv4 remote
 * address, it will use the local address that you set instead of the default first local
 * address.
 * Note: The localAddress string you provide for this function must match exactly
 * with the local address that was inserted in the configuration structure in the
 * initialization of the SIP Stack. If you configured the SIP Stack to listen to a 0.0.0.0
 * local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 *            localAddress - The local address to be set to this transmitter.
 *          localPort - The local port to be set to this transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetLocalAddress(
                            IN  RvSipTransmitterHandle         hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    );
                            
 /***************************************************************************
 * SipTransmitterGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transmitter will use to send outgoing
 *          messages. 
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType - The address type(ip or ip6).
 * Output:    localAddress - The local address this transmitter is using.
 *            localPort - The local port this transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT RvChar*                   localAddress,
                            OUT RvUint16*                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                *if_name
			    , OUT  RvUint16*              vdevblock
#endif
			    );

/***************************************************************************
 * SipTransmitterGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transmitter will use to send   
 *          the next message this function returns the actual address from 
 *          the 6 addresses that was used or going to be used.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 * Output:    eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType  - The address type(IP4 or IP6).
 *            localAddress - The local address this transmitter is using(must be longer than 48).
 *            localPort - The local port this transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetCurrentLocalAddress(
                            IN  RvSipTransmitterHandle     hTrx,
                            OUT RvSipTransport*            eTransportType,
                            OUT RvSipTransportAddressType* eAddressType,
                            OUT RvChar*                    localAddress,
                            OUT RvUint16*                  localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                 *if_name
			    , OUT  RvUint16*               vdevblock
#endif
			    );

#if defined(UPDATED_BY_SPIRENT)
RvSipTransportLocalAddrHandle SipTransmitterGetCurrentLocalAddressHandle(IN  RvSipTransmitterHandle     hTrx);
#endif

/***************************************************************************
 * SipTransmitterSetLocalAddressHandle
 * ------------------------------------------------------------------------
 * General: Sets a local address handle in the transmitter local addr structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            hLocalAddress - The local address handle.
 *           bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetLocalAddressHandle(
                            IN  RvSipTransmitterHandle        hTrx,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            IN  RvBool                        bSetAddrInUse);


/***************************************************************************
 * SipTransmitterSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address the transmitter will use as the request
 *          destination address. A requests sent
 *          by this transmitter will be sent according to the transmitter
 *          outbound address and not according to the Request-URI.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx         - Handle to the transmitter
 *            strOutboundAddrIp    - The outbound ip to be set to this
 *                                 transmitter.
 *          outboundPort         - The outbound port to be set to this
 *                                 transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundAddress(
                            IN  RvSipTransmitterHandle     hTrx,
                            IN  RvChar*                    strOutboundAddrIp,
                            IN  RvUint16                   outboundPort);


/***************************************************************************
 * SipTransmitterGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Get the outbound address the transmitter is using. If an
 *          outbound address was set to the transmitter it is returned. If
 *          no outbound address was set to the transmitter, but an outbound
 *          proxy was configured for the stack the configured address is
 *          returned. '\0' is returned no address was defined for
 *          the transmitter or the stack.
 *          NOTE: you must supply a valid consecutive buffer of size 20 for the
 *          strOutboundIp parameter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx      - Handle to the transmitter
 * Output:
 *            strOutboundIp   - A buffer with the outbound IP address the transmitter.
 *                               is using
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundAddress(
                            IN   RvSipTransmitterHandle    hTrx,
                            OUT  RvChar*                   strOutboundIp,
                            OUT  RvUint16*                 pOutboundPort);


/***************************************************************************
 * SipTransmitterSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy host name of the transmitter object.
 * The outbound host name will be resolved each time a request is sent to this host.
 * the request will be sent to the resolved IP address.
 * Note: To set a specific IP address, use SipTransmitterSetOutboundAddress().
 * If you configure a transmitter with both an outbound IP address and an
 * outbound host name, the transmitter will ignore the outbound host name and
 * will use the outbound IP address.
 * When using an outbound host all outgoing requests will be sent directly to the transmitter
 * outbound proxy host unless the application specifically ordered to ignore the outbound host.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx           - Handle to the transmitter
 *            strOutboundHostName    - The outbound proxy host to be set to this
 *                               transmitter.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               transmitter. If you set the port to zero it
 *                               will be determined using the DNS procedure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetOutboundHostName(
                            IN  RvSipTransmitterHandle  hTrx,
                            IN  RvChar*                 strOutboundHost,
                            IN  RvUint16                outboundPort);

/***************************************************************************
 * SipTransmitterGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy that the transmitter is using.
 * If an outbound host was set to the transmitter this host will be returned. If no
 * outbound host was set to the transmitter but an outbound host was configured
 * for the SIP Stack, the configured host is returned. If the transmitter is not
 * using an outbound host, '\0' is returned.
 * Note: You must supply a valid consecutive buffer to get the strOutboundHost host
 *       name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx      - Handle to the transmitter
 * Output:
 *          strOutboundHost   -  A buffer with the outbound host name
 *          pOutboundPort - The outbound host port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundHostName(
                            IN   RvSipTransmitterHandle  hTrx,
                            OUT  RvChar*                 strOutboundHostName,
                            OUT  RvUint16*               pOutboundPort);


/***************************************************************************
 * SipTransmitterSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the transmitter outbound proxy.
 * This transport will be used for the outbound proxy that you set using the
 * SipTransmitterSetOutboundAddress() function or the
 * SipTransmitterSetOutboundHostName() function. If you do not set an
 * outbound transport, the transport will be determined using the DNS procedures.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx           - Handle to the transmitter
 *          eOutboundTransport - The outbound transport to be set
 *                               to this transmitter.
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetOutboundTransport(
                            IN  RvSipTransmitterHandle           hTrx,
                            IN  RvSipTransport              eOutboundTransport);


/***************************************************************************
 * SipTransmitterGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of the outbound proxy that the transmitter is using.
 * If an outbound transport was set to the transmitter, this transport will be
 * returned. If no outbound transport was set to the transmitter but an outbound
 * proxy was configured for the SIP Stack, the configured transport is returned.
 * RVSIP_TRANSPORT_UNDEFINED is returned if the transmitter is not
 * using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx           - Handle to the transmitter
 * Output: eOutboundTransport - The outbound proxy transport the transmitter is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterGetOutboundTransport(
                            IN   RvSipTransmitterHandle    hTrx,
                            OUT  RvSipTransport      *eOutboundProxyTransport);

/************************************************************************************
 * SipTransmitterSendMessage
 * ----------------------------------------------------------------------------------
 * General: Send a message to the remote party. The remote address is calculated
 *          according to RFC 3261 that takes into account the existence of 
 *          outbound proxy, Route headers and loose routing rules.
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          hMsgToSend   - The message to send.
 *          eMsgType     - The type of the message (transaction-wise) to send
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterSendMessage (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN SipTransportMsgType       eMsgType);

/************************************************************************************
 * SipTransmitterCanContinue
 * ----------------------------------------------------------------------------------
 * General: checks if there is a valid DNS list in the transmitter and if that list 
 *          has at least 1 element that we can continue to
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmiter handle
 * Output:  none
 ***********************************************************************************/
RvBool RVCALLCONV SipTransmitterCanContinue (
              IN RvSipTransmitterHandle    hTrx);

/***************************************************************************
 * SipTransmitterDNSListReset
 * ------------------------------------------------------------------------
 * General: resets the transmitter DNS list. ie empty the list
 *
 * Return Value: RvStatus, RV_ERROR_UNKNOWN
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - the transmitter
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDNSListReset(
    IN  RvSipTransmitterHandle    hTrx);

/************************************************************************************
 * SipTransmitterSendMessageToPreDiscovered
 * ----------------------------------------------------------------------------------
 * General: Send a message to the remote party. if sending with this function, 
 *          Dns procedure will not start, but the "old" results from previous 
 *          DNS will be used
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          hMsgToSend   - The message to send.
 *          eMsgType     - The type of the message (transaction-wise) to send
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterSendMessageToPreDiscovered (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN SipTransportMsgType       eMsgType);


/************************************************************************************
 * SipTransmitterAddNewTopViaToMsg
 * ----------------------------------------------------------------------------------
 * General: Adds top via header to the message before address resolution and
 *          according to the transport of the request URI combined with the 
 *          available local addresses. This is a best shot via header. The via
 *          header will be fixed after the final destination will be resolved.
 *          Note: this function will use the branch found in the transmitter or
 *          will generate a new branch.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx      - Handle to the transmitter
 *          hMsg      - A message to add a top via to.
 *          hSourceVia- if NULL, use values extracted from transport,
 *                      o/w - copy this value to the new via header
 *          bIgnorTrxBranch - If the transmitter needs to add a branch it should create
 *                            a new branch and ignore its own existing branch.
 *                            this is good for ack on 2xx.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransmitterAddNewTopViaToMsg (
                     IN  RvSipTransmitterHandle          hTrx,
                     IN  RvSipMsgHandle                  hMsg,
                     IN  RvSipViaHeaderHandle            hSourceVia,
                     IN  RvBool                          bIgnorTrxBranch);


/***************************************************************************
 * SipTransmitterSetLocalAddressesStructure
 * ------------------------------------------------------------------------
 * General: Sets the local address structure to the transmitter.
 *          The address in use is updated according to the received
 *          structure.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  
 *          hTrx - Handle to the transmitter
 *         pLocalAddresses - The local address structure
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetLocalAddressesStructure (
                            IN  RvSipTransmitterHandle   hTrx,
                            IN  SipTransportObjLocalAddresses*   pLocalAddresses);

/***************************************************************************
 * SipTransmitterGetLocalAddressesStructure
 * ------------------------------------------------------------------------
 * General: Gets the local address structure to the transmitter
 *          The address in use is not defined in this stage.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  
 *          hTrx - Handle to the transmitter
 *         pLocalAddresses - The local address structure
 ***************************************************************************/
void RVCALLCONV SipTransmitterGetLocalAddressesStructure (
                            IN  RvSipTransmitterHandle   hTrx,
                            IN  SipTransportObjLocalAddresses*   pLocalAddresses);

/***************************************************************************
 * SipTransmitterGetAddrInUse
 * ------------------------------------------------------------------------
 * General: Gets the local-address-in-use from the transmitter
 * Return Value: A pointer to the local address in use
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTrx - Handle to the transmitter
 ***************************************************************************/
RvSipTransportLocalAddrHandle* RVCALLCONV SipTransmitterGetAddrInUse (
                            IN  RvSipTransmitterHandle           hTrx);

/***************************************************************************
 * SipTransmitterSetLocalAddressForAllTransports
 * ------------------------------------------------------------------------
 * General: gets a string local ip and port and set it to all handles that have
 *          a matching address.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 *            strLocalAddress - The local address string
 *            localPort - The local port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetLocalAddressForAllTransports(
                            IN  RvSipTransmitterHandle        hTrx,
                            IN  RvChar                        *strLocalAddress,
                            IN  RvUint16                       localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                      *if_name
			    , IN  RvUint16                     vdevblock
#endif
			    );

#ifdef RV_SIGCOMP_ON

/***************************************************************************
* SipTransmitterConvertMsgCompTypeToCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter outbound message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvSipCompType RVCALLCONV SipTransmitterConvertMsgCompTypeToCompType(
                             IN RvSipTransmitterMsgCompType eMsgCompType);

/***************************************************************************
* SipTransmitterGetOutboundMsgCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter outbound message compression type
* Return Value: The message compression type.
* ------------------------------------------------------------------------
* Arguments:
* input:  hTrx - Handle to the transmitter.
***************************************************************************/
RvSipTransmitterMsgCompType RVCALLCONV SipTransmitterGetOutboundMsgCompType(
                         IN  RvSipTransmitterHandle       hTrx);


/***************************************************************************
* SipTransmitterGetNextMsgCompType
* ------------------------------------------------------------------------
* General: Retrieves the transmitter next message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvSipTransmitterMsgCompType RVCALLCONV SipTransmitterGetNextMsgCompType(
                                            IN  RvSipTransmitterHandle  hTrx);
 
/***************************************************************************
* SipTransmitterSetNextMsgCompType
* ------------------------------------------------------------------------
* General: Set the transmitter next sent message compression type
* Return Value: The compression type
* ------------------------------------------------------------------------
* Arguments:
* input: hTrx - Handle to the transmitter.
***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetNextMsgCompType(
                      IN  RvSipTransmitterHandle      hTrx,
                      IN  RvSipTransmitterMsgCompType eNextMsgCompType);

/***************************************************************************
 * SipTransmitterMatchSentOutboundMsgToNextMsgType
 * ------------------------------------------------------------------------
 * General: The next retranmitted message type was set in either of
 *          the following cases:
 *          (1) A timeout on outgoing SigComp message was expired and the
 *              the application chose the next re-sent message type.
 *          (2) An incoming message was re-received (in case of original
 *              compressed message) and trigerred a retransmission process
 *              of the suitable response/ACK. The re-received message type
 *              effected the next retransmitted message type.
 *          The next retransmitted message type might change the sent
 *          outbound message page.
 * Return Value: RV_OK on successful completion.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - Handle to the transmitter.
 * Output: pbZeroRetrans - Indication if the retransmission counter has to
 *                         become zero.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMatchSentOutboundMsgToNextMsgType(
                                     IN  RvSipTransmitterHandle  hTrx,
                                     OUT RvBool                 *pbZeroRetrans);


/***************************************************************************
 * SipTransmitterDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a Transmitter.
 *          This means that even if the message indicates compression
 *          it will not be used.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx    - The transmitter handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDisableCompression(
                                IN  RvSipTransmitterHandle hTrx);
#endif /* RV_SIGCOMP_ON */


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransmitterChangeOwner
 * ------------------------------------------------------------------------
 * General: Changes the owner of a transmitter. (hAppTrx, and eventHandlers)
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - handle to the transmitter.
 *            hAppTrx     - new application handle to the transmitter.
 *            pEvHanders  - new event handlers structure for this transmitter.
 *            sizeofEvHandlers - size of the event handler structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterChangeOwner(
                   IN  RvSipTransmitterHandle      hTrx,
                   IN  RvSipAppTransmitterHandle   hAppTrx,
                   IN  RvSipTransmitterEvHandlers* pEvHandlers,
                   IN  RvInt32                     sizeofEvHandlers);

/***************************************************************************
 * SipTransmitterDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach the owner of a transmitter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - handle to the transmitter.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterDetachOwner(
                   IN  RvSipTransmitterHandle      hTrx);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransmitterSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Transmitter object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - Handle to the Transmitter object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransmitterSetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/***************************************************************************
 * SipTransmitterGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Transmitter object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - Handle to the Transmitter object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransmitterGetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransmitterSetCompartment
 * ------------------------------------------------------------------------
 * General: Sets an handle to a SigComp Compartment within a Transmitter
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - Handle to the Transmitter object.
 *          hCompartment - Handle to the set Compartment.
 *
 * Output:  None.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterSetCompartment(
                    IN  RvSipTransmitterHandle  hTrx,
                    IN  RvSipCompartmentHandle  hCompartment);

/***************************************************************************
 * SipTransmitterGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves an handle to a SigComp Compartment from a Transmitter
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx          - Handle to the Transmitter object.
 *
 * Output:  phCompartment - Handle to the retrieved SigComp Compartment.
 ***************************************************************************/
void RVCALLCONV SipTransmitterGetCompartment(
                         IN  RvSipTransmitterHandle  hTrx,
                         OUT RvSipCompartmentHandle *phCompartment);
#endif /* #ifdef RV_SIGCOMP_ON */ 

#if defined(UPDATED_BY_SPIRENT)
RvStatus SipTransmitterGetLocalAddr(Transmitter *trx,RvSipTransportAddr *addressDetails);
#endif

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_TRANSMITTER_H */



