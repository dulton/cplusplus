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


#ifndef TRANSPORT_UDP_H
#define TRANSPORT_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransportMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/******************************************************************************
 * TransportUDPOpen
 * ----------------------------------------------------------------------------
 * General: creates UDP socket, binds it to the specified address and
 *          starts to listen on it (register it to READ event).
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the transport info structure
 *          pLocalAddress - local address, socket for which should be opened.
 *                          It contains IP and port for binding
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPOpen(IN TransportMgr*          pTransportMgr,
                                     IN TransportMgrLocalAddr* pLocalAddress);

/******************************************************************************
 * TransportUDPClose
 * ----------------------------------------------------------------------------
 * General: Closes UDP address - closes network socket, unregisters it from
 *          Select Engine, destructs FileDescriptor
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport manager
 *          pLocalAddr      - pointer to the address to be closed
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPClose(
                                    IN TransportMgr             *pTransportMgr,
                                    IN TransportMgrLocalAddr    *pLocalAddr);

#if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES)
/******************************************************************************
 * TransportUDPReopen
 * ----------------------------------------------------------------------------
 * General: closes, opens and binds socket to the same IP:Port.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the Transport manager
 *          pLocalAddr      - pointer to the address to be closed
 *****************************************************************************/
RvStatus RVCALLCONV TransportUDPReopen(
                            IN TransportMgr             *pTransportMgr,
                            IN TransportMgrLocalAddr    *pLocalAddr);
#endif /* #if (RV_REOPEN_SOCKET_ON_ERROR == RV_YES) */

/******************************************************************************
 * TransportUdpEventCallback
 * ----------------------------------------------------------------------------------
 * General:  Callback function that is called from the LI package whenever
 *           we are getting event from there.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * input   : selectEngine   - Events engine of this fd
 *           fd             - File descriptor that this event occured on
 *           selectEvent    - Event that happened
 *           error          - RV_TRUE if an error occured
 * Output:  RvStatus
 ***********************************************************************************/
void TransportUdpEventCallback(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error);

#ifdef __cplusplus
}
#endif

#endif /*TRANSPORT_UDP_H*/
