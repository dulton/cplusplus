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
 *                              SipMsgBuilder.h
 *
 *  This file provides headers for the message builder object of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20-Nov-2000
 *********************************************************************************/

#ifndef TRANSPORT_MSG_BUILDER_H
#define TRANSPORT_MSG_BUILDER_H


#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "_SipCommonUtils.h"
#include "AdsRpool.h"
#include "TransportConnection.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/******************************************************************************
 * TransportMsgBuilderParseUdpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseUdpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from UDP
 *          socket and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseUdpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          hLocalAddr    - local address where the message was sent.
 *          recvFrom      - address from where the message was sent.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseUdpMsg(
                            IN TransportMgr*                 pTransportMgr,
                            IN RvChar*                       pBuffer,
                            IN RvInt32                       totalMsgSize,
                            IN RvSipMsgHandle                hInjectedMsg,
                            IN RvSipTransportLocalAddrHandle hLocalAddr,
                            IN SipTransportAddress*          recvFrom);

/******************************************************************************
 * TransportMsgBuilderParseTcpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseTcpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from TCP/TLS
 *          connection and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseTcpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          pConn         - pointer to the connection.
 *          hAppConn      - application handle stored in connection.
 *          sipMsgSize    - SIP part length of the message (without SDP).
 *          sdpLength     - SDP part length of the message.
 *          localAddr     - local address where the message was sent.
 *          recvFrom      - address from where the message was sent.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          pSigCompInfo  - Transport SigComp information related to the message.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseTcpMsg(
                            IN TransportMgr*                 pTransportMgr,
                            IN RvChar*                       pBuffer,
                            IN RvInt32                       totalMsgSize,
                            IN TransportConnection*          pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvInt32                       sipMsgSize,
                            IN RvUint32                      sdpLength,
                            IN RvSipTransportLocalAddrHandle hLocalAddr,
                            IN SipTransportAddress*          recvFrom,
                            IN RvSipMsgHandle                hInjectedMsg,
                            IN SipTransportSigCompMsgInfo*   pSigCompInfo);

/******************************************************************************
 * TransportMsgBuilderParseSctpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseSctpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from UDP
 *          socket and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseSctpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pConn         - pointer to the connection.
 *          hAppConn      - handle stored by the application in the connection.
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          hLocalAddr    - local address where the message was sent.
 *          recvFrom      - address from where the message was sent.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseSctpMsg(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvChar*                           pBuffer,
                            IN RvInt32                           totalMsgSize,
                            IN RvSipMsgHandle                    hInjectedMsg,
                            IN RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN SipTransportAddress*              recvFrom);

/******************************************************************************
 * TransportMsgBuilderAccumulateMsg
 * ----------------------------------------------------------------------------
 * General:   The TransportMsgBuilderAccumulateMsg goes through the buffer
 *            that stores data, received on bit-stream connection, and tries
 *            to recognize SIP messages in it (if more then 1).
 *            This function calls the prepare function to remove crlfs,
 *            and calls the set function which sets the body.
 *            The prepare function returns number of bytes actually was read
 *            from the buffer.
 *            Each time when single SIP message boundaries are discovered,
 *            MESSAGE_RECEIVED event is created and sent to the TPQ.
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf        - the buffer that was received from the network
 *          bufSize     - the buffer size.
 *          conn           - the connection
 *          pLocalAddr  - The local address to which the message was received
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderAccumulateMsg(
                            IN RvChar                       *pBuf,
                            IN RvUint32                      bufSize,
                            IN TransportConnection          *conn,
                            IN RvSipTransportLocalAddrHandle hLocalAddr);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * TransportMsgBuilderPrintMsg
 * ----------------------------------------------------------------------------------
 * General: log print function. used only for printing the buffer to the log.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport instance
 *          pBuf             - the buffer that was received from the network.
 *          msgDirection     - The direction of the sent message.
 *          bAdditional      - Indication if the request to print a message is in
 *                             addition to the general flow (for example before
 *                             compression of after decompression)
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportMsgBuilderPrintMsg(
                     IN TransportMgr                     *pTransportMgr,
                     IN RvChar                           *pBuf,
                     IN SipTransportMgrBuilderPrintMsgDir msgDirection,
                     IN RvBool                            bAdditional);
#else
#define TransportMsgBuilderPrintMsg(_t,_p,_s,_e)
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/




#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _SIP_MSG_BUILDER_H*/
