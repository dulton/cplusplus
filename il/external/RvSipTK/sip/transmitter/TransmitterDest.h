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
 *                              <TransmitterDest.h>
 *
 *  The TransmitterDest.c file contains relevant for Address resolution
 *  
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                 January 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#ifndef TRANSMITTER_DEST_H
#define TRANSMITTER_DEST_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/
#define TRANSMITTER_DNS_REG_MAX_MATCHES 10

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipTransmitter.h"

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER DEST FUNCTIONS                         */
/*-----------------------------------------------------------------------*/

/*****************************************************************************
 * TransmitterDestDiscovery
 * ---------------------------------------------------------------------------
 * General: Identify the destination address.
 *          This function is a focal point for all nex hop decisions.
 *          it forks to :
 *          - discover the next hop host
 *          - convert TEL URLs to SIP URLS
 *          - find the next hop protocol (transport)
 *          - find the next hop ip and port
 *          - proceed with sending after resolution is complete
 *          - report on a msg sent failre if DNS resolution is not completed
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_BADPARAM
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx          - pointer to transmitter
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from a callback.
 * Output:  -
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterDestDiscovery(
                IN  Transmitter*         pTrx,
                IN RvBool                bReachedFromApp);

/***************************************************************************
 * TransmitterDestAllocateHostBuffer
 * ------------------------------------------------------------------------
 * General: allocates the buffer for the host member in the msg params.
 *          we need this parameter in a consecutive buffer for resolver operations
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx           - transmitter
 *        length         - size to allocate
 * Output:pStrHostName   - allocated consecutive buffer
 *        pHostNameOffset - The offset of the buffer on the transmitter page.
 *                          (may be NULL)
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterDestAllocateHostBuffer(
            IN  Transmitter*    pTrx,
            IN  RvUint          length,
            OUT RvChar**        pStrHostName,
            OUT RvInt32*        pHostNameOffset);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef TRANSMITTER_DEST_H */

