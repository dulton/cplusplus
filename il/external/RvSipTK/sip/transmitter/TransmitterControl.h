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
 *                              <TransmitterControl.h>
 *
 * This file contains all transmitter message sending activities.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos               January 2004
 *********************************************************************************/

#ifndef TRANSMITTER_TRANSPORT_H
#define TRANSMITTER_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipTransportTypes.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER TRANSPORT FUNCTIONS                   */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransmitterControlStartMessageSending
 * ------------------------------------------------------------------------
 * General: Send the message object held by the transmitter to the
 *          remote party. The function performes address resolution if 
 *          needed notifies the application that address was resolved and
 *          call the TransmitterControlContinueMessageSending that will
 *          do the actual sending.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx - handle to the transmitter
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlStartMessageSending(
                                        IN Transmitter*  pTrx);

/***************************************************************************
 * TransmitterControlContinueMessageSending
 * ------------------------------------------------------------------------
 * General: Send the message after address resolution was completed.
 *          If the dest address has been reset, go for another round with 
 *          the resolution process
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - handle to the transmitter.
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from the dns callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlContinueMessageSending(
                                        IN Transmitter*      pTrx,
                                        IN RvBool            bReachedFromApp);

/***************************************************************************
 * TransmitterControlSend
 * ------------------------------------------------------------------------
 * General: Send the message after address resolution was completed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - handle to the transmitter.
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from the dns callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlSend(
                                        IN Transmitter*      pTrx,
                                        IN RvBool            bReachedFromApp);

/***************************************************************************
 * TransmitterControlSendBuffer
 * ------------------------------------------------------------------------
 * General: Sends a buffer to the destination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx     - handle to the transmitter.
 *          strBuff  - The buffer to send.
 *          buffSize - The buffer size.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlSendBuffer(
                                        IN Transmitter*      pTrx,
										IN RvChar*           strBuff,
										IN RvInt32           buffSize);


/***************************************************************************
 * TransmitterControlConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   -   The connection handle
 *          hTrx    -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTrx,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason);

/***************************************************************************
 * TransmitterControlConnStatusEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection status was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   -   The connection handle
 *          hTrx    -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          pInfo   -   For future usage.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlConnStatusEv(
                    IN  RvSipTransportConnectionHandle        hConn,
                    IN  RvSipTransportConnectionOwnerHandle   hTrx,
                    IN RvSipTransportConnectionStatus         eStatus,
                    IN  void*                                 msgInfo);
/***************************************************************************
 * TransmitterControlTransmitMessage
 * ------------------------------------------------------------------------
 * General: transmit the message sent by the transaction, over tcp or udp
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx - The transaction to retransmit from.
 *        strHostName  - The host name for TLS post connection assertion.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterControlTransmitMessage(IN Transmitter *pTrx);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef TRANSMITTER_TRANSPORT_H*/



