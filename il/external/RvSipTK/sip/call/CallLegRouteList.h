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
 *                              <sipCallLegRouteList.h>
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef CALL_LEG_ROUTE_LIST_H
#define CALL_LEG_ROUTE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "CallLegObject.h"
#include "RvSipMsg.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegRouteListInitialize
 * ------------------------------------------------------------------------
 * General:  Loop over Record-Route headers found in the message and insert
 *           them into route list. The route-list is built in the same
 *           order that the Record-Route headers are found in the received
 *           message, whether the message is a request or reponse.
 *           The necessary reversal of Route headers on the UAC side
 *           is done when preparing outgoing messages.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListInitialize(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * CallLegRouteListProvInitialize
 * ------------------------------------------------------------------------
 * General:  Loop over Record-Route headers found in the reliable provisional
 *           message and insert them into the route list. The route list
 *           built from reliable provisional responses are valid only until
 *           a prack is sent for that provisional. The list is not built on
 *           the call-leg page but on a temporary page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListProvInitialize(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);


/***************************************************************************
 * CallLegRecordRouteListInitialize
 * ------------------------------------------------------------------------
 * General:  Loop over Record-Route headers found in the message and insert
 *           them into the record route list. The record route list is built
 *           in the same of the Record-Route headers that are found in the
 *           received message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
/*RvStatus CallLegRecordRouteListInitialize(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg);*/

/***************************************************************************
 * CallLegRouteListAddToMessage
 * ------------------------------------------------------------------------
 * General: adds route list to requests and record route list to responses
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListAddToMessage (IN  CallLeg                 *pCallLeg,
                                        IN  RvSipTranscHandle       hTransc,
                                        IN  RvSipMsgHandle          hMsg);
/***************************************************************************
 * CallLegRouteListGetRequestURI
 * ------------------------------------------------------------------------
 * General: When we have a route list the request URI is taken from the list.
 *          The end of the list for client requests and the head of the list
 *          for server requests
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg
 * Output:  phAddress         - The actual contact to use.
 *          bSendToFirstRouteHeader - If the next hop in the path is a loose router this
 *                              parameter will be set to RV_TRUE, otherwise to RV_FALSE.
 ***************************************************************************/
RvStatus CallLegRouteListGetRequestURI(IN  CallLeg*           pCallLeg,
                                        OUT RvSipAddressHandle *phAddress,
                                        OUT RvBool            *bSendToFirstRouteHeader);

/***************************************************************************
 * CallLegRouteListIsEmpty
 * ------------------------------------------------------------------------
 * General: Checks if the call-leg route list is empty. It is empty if it
 *          is NULL or if there are no elements on the list.
 * Return Value: RV_TRUE if the list is empty, RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvBool CallLegRouteListIsEmpty(IN  CallLeg      *pCallLeg);


/***************************************************************************
 * CallLegRouteListProvDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the route list built from provisional response.
 * responses in the call-leg.
 * Return Value: RV_TRUE if the list is empty, RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
void CallLegRouteListProvDestruct(IN  CallLeg      *pCallLeg);

/***************************************************************************
 * CallLegRouteListAddTo2xxAckMessage
 * ------------------------------------------------------------------------
 * General: adds route list to ACK request that is sent with no transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	pCallLeg - Pointer to the call-leg
 *          hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListAddTo2xxAckMessage(IN  CallLeg        *pCallLeg,
                                                 IN  RvUint16       responseCode,
			        							 IN  RvSipMsgHandle  hMsg);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * CallLegRouteListReset
 * ------------------------------------------------------------------------
 * General: Reset tail route list entry.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            routeValue - Value of route header to be added to the route list
 ***************************************************************************/
RvStatus CallLegRouteListReset (IN  CallLeg                 *pCallLeg,
                                IN  RvChar                  *routeValue);

/***************************************************************************
 * CallLegRouteListAdd
 * ------------------------------------------------------------------------
 * General: Given route value, insert the route header to the head of the 
 *          CallLeg's route list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            listPage - RPOOL Allocation to use
 *            hList    - Handle to the list of routes.
 *            routeValue - Value of route header to be added to the route list
 ***************************************************************************/
RvStatus CallLegRouteListAdd   (IN  CallLeg                 *pCallLeg,
                                IN  HPAGE                   listPage,
                                IN  RLIST_HANDLE            hList,
                                IN  RvChar                  *routeValue);

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#endif /* RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_ROUTE_LIST_H */

