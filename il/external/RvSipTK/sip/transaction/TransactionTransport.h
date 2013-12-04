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
 *                              TransactionTransaport.h
 *
 *  This file provides a method for identifing a destination address in a message.
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20-Nov-2000
 *********************************************************************************/

#ifndef TRANSACTION_TRANSPORT_H
#define TRANSACTION_TRANSPORT_H


#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "_SipTransactionTypes.h"
#include "RvSipMsg.h"
#include "TransactionObject.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
* TransactionTransportSendMsg
* ------------------------------------------------------------------------
* General: Find the address to send the message within the message itself,
*          encode the message, and send it to this address.
* Return Value: RV_ERROR_NULLPTR - pTransc or hMsgToSend are NULL pointers.
*               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
*               RV_ERROR_UNKNOWN - a failure occured.
* ------------------------------------------------------------------------
* Arguments:
* Input: pTransc  - The transaction to send message with.
*        bResolveAddr - RV_TRUE if we want the transmitter to resolve the dest addr
*                       from the message, RV_FALSE if the transmitter should use the
*                       address it currently hold.
* Output: TransactionTransportSendMsg
***************************************************************************/
RvStatus TransactionTransportSendMsg(IN  Transaction    *pTransc,
                                     IN  RvBool          bResolveAddress);


/***************************************************************************
 * TransactionTransportRetransmitMessage
 * ------------------------------------------------------------------------
 * General: Retransmit the last message sent by the transaction, if exists.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to retransmit from.
 ***************************************************************************/
RvStatus TransactionTransportRetransmitMessage(IN Transaction *pTransc);



/***************************************************************************
* TransactionTransportSendOutofContextMsg
* ------------------------------------------------------------------------
* General: Send a given message that is not related to the transaction.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: TranscpMgr         - The transaction manager handle.
*        hMsgToSend         - The message to send.
*        bHandleTopVia      - Indicate whether to add top via to request or to
*                             remove top via from response.
*        strViaBranch       - via branch to add to the message befor sending
*                            (used ONLY on request and if bHandleTopVia = RV_TRUE)
*        pViaBranch         - the via branch as an rpool string.
*        hLocalAddr        - the local address handle to use.
*        strLocalAddress      - The local address to use when sending the message. If
*                                 NULL is given the deafult local address will be used.
*        localPort            - The local port to use when sending the message.
*        bIgnoreOutboundProxy - Indicate the proxy whether to ignore an
*                                 outbound proxy.
*        bSendToFirstRouteHeader  - Determines weather to send a request to to a loose router
*                             (to the first in the route list) or to a strict router
*                             (to the request URI). When the message sent is a response
*                             this parameter is ignored.
*        bCopyMsg           - mark if the transmitter should copy the message.
***************************************************************************/
RvStatus TransactionTransportSendOutofContextMsg(
                               IN    TransactionMgr                *pTranscMgr,
                               IN    RvSipMsgHandle                 hMsgToSend,
                               IN    RvBool                         bHandleTopVia,
                               IN    RvChar                        *strViaBranch,
                               IN    RPOOL_Ptr                     *pViaBranch,
                               IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                               IN    RvChar                        *strLocalAddress,
                               IN    RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
                               IN    RvChar                        *if_name,
			       IN    RvUint16                       vdevblock,
#endif
                               IN    RvBool                         bIgnoreOutboundProxy,
                               IN    RvBool                         bSendToFirstRouteHeader,
                               IN    RvBool                         bCopyMsg);

/***************************************************************************
 * TransactionTransportConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 *          The transaction will handle the received event.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hOwner  -   Handle to the connection owner.
 *          hConn   -   The connection handle
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTransportConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTransc,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason);

/***************************************************************************
 * TransactionTransportDetachAllConnections
 * ------------------------------------------------------------------------
 * General: Detaches the transaction from all connections.
 *          after calling this function the transaction will no longer receive TCP events
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - pointer to transaction
 ***************************************************************************/
void RVCALLCONV TransactionTransportDetachAllConnections(
                                    IN  Transaction         *pTransc);



/***************************************************************************
 * TransactionTransportTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Completes sending of the SIP message upon asynchronous DNS
 *    procedure finished callback
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     transmitter - the transmitter
 *            hContext    - the callback context, points to the transaction
 ***************************************************************************/
void RVCALLCONV TransactionTransportTrxStateChangeEv (
       IN  RvSipTransmitterHandle            hTrx,
       IN  RvSipAppTransmitterHandle         hAppTrx,
       IN  RvSipTransmitterState             eState,
       IN  RvSipTransmitterReason            eReason,
       IN  RvSipMsgHandle                    hMsg,
       IN  void*                             pExtraInfo);


/*****************************************************************************
 * TransactionTransportTrxOtherURLAddressFoundEv
 * ---------------------------------------------------------------------------
 * General: implementation to the other url found event handler of the transmitter.
 * Return Value: RvStatus - RvStatus
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx        - the transaction transmitter
 *          hAppTrx     - App handle. in this case always points to transaction
 *          hMsg        - Handle of the message that contains the Other
 *                        URL address.
 *          hAddress    - The other URL address handle to be "converted"
 *                        to SIP URL address .
 *
 * Output:  hSipURLAddress      - Handle to the constructed SIP URL address.
 *          pbAddressResolved   - Indication of a successful/failed address
 *                                resolving.
 *
 ****************************************************************************/
RvStatus RVCALLCONV TransactionTransportTrxOtherURLAddressFoundEv(
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    pbAddressResolved);



/***************************************************************************
 * TransactionTransportOutOfContextTrxStateChangeEv
 * ------------------------------------------------------------------------
 * General: Notifies the transaction of a Transmitter state change.
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    -    The sip stack Transmitter handle
 *            hAppTrx - The transaction handle.
 *            eState  -      The new Transmitter state
 *            eReason -     The reason for the state change.
 *            hMsg    -     When the state relates to the outgoing message, the
 *                          message is supplied.
 *            pExtraInfo - specific information for the new state.
 ***************************************************************************/
void RVCALLCONV TransactionTransportOutOfContextTrxStateChangeEv(
                                   IN  RvSipTransmitterHandle            hTrx,
                                   IN  RvSipAppTransmitterHandle         hAppTrx,
                                   IN  RvSipTransmitterState             eState,
                                   IN  RvSipTransmitterReason            eReason,
                                   IN  RvSipMsgHandle                    hMsg,
                                   IN  void*                             pExtraInfo);


/***************************************************************************
 * TransactionTransportOutOfContextTrxOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a message needs to be sent and
 *          the destination address is a URL type that is currently not
 *          supported by the stack. The URL has to be converted to a SIP URL
 *          for the message to be sent.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTrx           - The sip stack Transmitter handle
 *        hAppTrx        - The transaction manager.
 *        hMsg           - The message that includes the other URL address.
 *        hAddress       - Handle to unsupport address to be converted.
 * Output:hSipURLAddress - Handle to the SIP URL address - this is an empty
 *                         address object that the application should fill.
 *        pbAddressResolved -Indication wether the SIP URL address was filled.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTransportOutOfContextTrxOtherURLAddressFoundEv (
                     IN  RvSipTransmitterHandle     hTrx,
                     IN  RvSipAppTransmitterHandle  hAppTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool                    *pbAddressResolved);

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif  /*END OF: #ifndef TRANSACTION_TRANSPORT_H*/
