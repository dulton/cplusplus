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
 *                              <CallLegRouteList.c>
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Dec 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                          */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegRouteList.h"
#include "RvSipRouteHopHeader.h"
#include "_SipRouteHopHeader.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include "_SipSubs.h"
#include "RvSipCSeqHeader.h"
#include "_SipCommonUtils.h"


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus ListInitialize(
                                 IN  CallLeg                 *pCallLeg,
                                 IN  RvSipMsgHandle          hMsg,
                                 IN  HPAGE                   listPage,
                                 OUT RLIST_HANDLE            *hList);

static RvStatus ListAddElement(
                                 IN  CallLeg                 *pCallLeg,
                                 IN RvSipRouteHopHeaderHandle hRecordRoute,
                                  IN  HPAGE                   listPage,
                                 IN RLIST_HANDLE              hList);

static RvStatus RouteListAddToRequest(
                                  IN  CallLeg         *pCallLeg,
                                  IN RvSipMsgHandle  hMsg);

static RvStatus RecordRouteListAddToResponse(
                                  IN  CallLeg                 *pCallLeg,
                                   IN  RvSipMsgHandle          hMsg);

static RvStatus RouteListAddToClientRequest(
                                  IN  CallLeg         *pCallLeg,
                                  IN  RvSipMsgHandle  hMsg);

static RvStatus RouteListAddToServerRequest(
                                  IN  CallLeg         *pCallLeg,
                                  IN  RvSipMsgHandle  hMsg);

static RvStatus RouteListAppendRemoteContact(
                                  IN  CallLeg         *pCallLeg,
                                  IN  RvSipMsgHandle  hMsg);



static RvBool IsLooseRouter(IN RvSipRouteHopHeaderHandle hRouteHopHeader);

static RvStatus AddRouteListToRequestMsgIfNeed(
                                         IN  CallLeg                 *pCallLeg,
                                         IN  RvSipTranscHandle       hTransc,
                                         IN  RvSipMsgHandle          hMsg);

static RvStatus AddRouteListToResponseMsgIfNeeded(
                                          IN  CallLeg                 *pCallLeg,
                                          IN  RvSipTranscHandle       hTransc,
                                          IN  RvSipMsgHandle          hMsg);

#ifdef RV_SIP_SUBS_ON
static RvStatus IsInitialSubscription(IN  RvSipTranscHandle  hTransc,
                                      OUT RvBool            *pbInitSubs);
#endif /* #ifdef RV_SIP_SUBS_ON */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
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
                        IN  RvSipMsgHandle          hMsg)
{
    RvStatus                    rv;

    /*at this point we can destruct the provisional route list if we have one*/
    CallLegRouteListProvDestruct(pCallLeg);

    rv = ListInitialize(pCallLeg,hMsg,pCallLeg->hPage,&(pCallLeg->hRouteList));
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegRouteListInitialize - Failed for Call-leg 0x%p (rv=%d)",
                  pCallLeg,rv));
        return rv;
    }
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegRouteListInitialize - Call-leg 0x%p - Route list initialized",
              pCallLeg));

    return RV_OK;
}


/***************************************************************************
 * CallLegRouteListProvInitialize
 * ------------------------------------------------------------------------
 * General:  Loop over Record-Route headers found in the provisional
 *           message and insert them into the route list. The route list
 *           built from provisional responses are valid only until
 *           a 2xx is recevied. The list is not built on
 *           the call-leg page but on a temporary page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListProvInitialize(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)
{
    RvStatus           rv;
    /* if we already have a route list we should not change it until 2xx is received*/
    if(pCallLeg->hRouteList != NULL)
    {
        return RV_OK;
    }
    /*build a new route list*/
    /*get a new page for building the record route list*/
    rv = RPOOL_GetPage(pCallLeg->pMgr->hGeneralPool,0,&(pCallLeg->hListPage));
    if(rv != RV_OK)
    {
        pCallLeg->hListPage = NULL_PAGE;
        return RV_ERROR_OUTOFRESOURCES;
    }
    /*initialize the list on the given page*/
    rv = ListInitialize(pCallLeg,hMsg,pCallLeg->hListPage,
                        &(pCallLeg->hRouteList));

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegRouteListProvInitialize - Failed for Call-leg 0x%p (rv=%d)",
                  pCallLeg,rv));

        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool,pCallLeg->hListPage);
        pCallLeg->hListPage = NULL_PAGE;
        return rv;
    }
    /*if there was no need to initialize a list free the page*/
    if(pCallLeg->hRouteList == NULL)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool,pCallLeg->hListPage);
        pCallLeg->hListPage = NULL_PAGE;
    }

    return RV_OK;
}

/***************************************************************************
 * CallLegRouteListProvDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the route list built from provisional responses
 * responses in the call-leg.
 * Return Value: RV_TRUE if the list is empty, RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
void CallLegRouteListProvDestruct(IN  CallLeg      *pCallLeg)
{

    if(pCallLeg->hRouteList != NULL)
    {
          RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegRouteListProvDestruct - Call-leg 0x%p, Destructing route list",pCallLeg));
          pCallLeg->hRouteList = NULL;
    }
    if(pCallLeg->hListPage != NULL_PAGE)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool,pCallLeg->hListPage);
        pCallLeg->hListPage = NULL_PAGE;

    }
}


/***************************************************************************
 * CallLegRouteListAddToMessage
 * ------------------------------------------------------------------------
 * General: adds route list to requests and record route list to responses
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *          hTransc - handle to the transaction.
 *            hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListAddToMessage(IN  CallLeg                 *pCallLeg,
                                      IN  RvSipTranscHandle       hTransc,
                                      IN  RvSipMsgHandle          hMsg)
{

    RvSipMsgType eMsgType   = RvSipMsgGetMsgType(hMsg);
    RvStatus rv;

    /*---------------------------------------------------------
        Add route set to requests
      ---------------------------------------------------------*/
    if (eMsgType == RVSIP_MSG_REQUEST)
    {
        rv = AddRouteListToRequestMsgIfNeed(pCallLeg,hTransc,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRouteListAddToMessage - AddRouteListToRequestMsgIfNeed failed for Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }
    else
    {
        rv = AddRouteListToResponseMsgIfNeeded(pCallLeg,hTransc,hMsg);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRouteListAddToMessage - AddRouteListToResponseMsgIfNeeded failed for Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }

    return RV_OK;
}

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
                                        OUT RvBool            *bSendToFirstRouteHeader)
{

    RLIST_ITEM_HANDLE       pItem            = NULL;
    RvSipRouteHopHeaderHandle *phListRoute   = NULL;

    /*if this is a client request then the end of the route list is the
      new request URI
      if this is a server request then the request URI is the remote contact
      or from together with the maddr of the begining of the route list*/
    if(pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_OUTGOING)
    {
        RLIST_GetTail(NULL, pCallLeg->hRouteList,&pItem);

    }
    else
    {
        RLIST_GetHead(NULL, pCallLeg->hRouteList,&pItem);
    }

    phListRoute = (RvSipRouteHopHeaderHandle*)pItem;

    /*get the address spec from the route hop*/
    *phAddress = RvSipRouteHopHeaderGetAddrSpec(*phListRoute);

    if(RvSipAddrUrlGetLrParam(*phAddress) == RV_FALSE)
    /* The next route in the route list is a strict route */
    {
        /* remove the headers parameter field that are not allowed in a Request-UR I*/
        RvSipAddrUrlSetHeaders(*phAddress,NULL);
        *bSendToFirstRouteHeader = RV_FALSE;
        return RV_OK;
    }
    else /* The next route in the route list is a loose route */
    {
        *bSendToFirstRouteHeader = RV_TRUE;
        return CallLegGetRemoteTargetURI(pCallLeg, phAddress);
    }
}

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
RvBool CallLegRouteListIsEmpty(IN  CallLeg      *pCallLeg)
{

    RLIST_ITEM_HANDLE  listItem = NULL;

    if(pCallLeg->hRouteList == NULL)
    {
        return RV_TRUE;
    }
    RLIST_GetHead(NULL, pCallLeg->hRouteList,&listItem);
    if(listItem == NULL)
    {
        return RV_TRUE;
    }
    return RV_FALSE;

}

/***************************************************************************
 * CallLegRouteListAddTo2xxAckMessage
 * ------------------------------------------------------------------------
 * General: adds route list to ACK request that is sent with no transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Pointer to the call-leg
 *          hMsg - Handle to the message.
 ***************************************************************************/
RvStatus CallLegRouteListAddTo2xxAckMessage(IN  CallLeg        *pCallLeg,
                                                 IN  RvUint16       responseCode,
                                                 IN  RvSipMsgHandle  hMsg)
{
    RvStatus rv;

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        if(responseCode >= 300)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRouteListAddTo2xxAckMessage - Call-leg 0x%p, will not add route list to ACK msg 0x%p",
                pCallLeg, hMsg));
            return RV_OK;
        }
    }
    rv = RouteListAddToRequest(pCallLeg,hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegRouteListAddTo2xxAckMessage - Failed for Call-leg 0x%p",pCallLeg));
    }
    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ListInitialize
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

static RvStatus ListInitialize(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg,
                        IN  HPAGE                   listPage,
                        OUT RLIST_HANDLE            *hList)
{

    RvSipRouteHopHeaderHandle    hRouteHop;
    RvSipHeaderListElemHandle    listElem;
    RvSipRouteHopHeaderType      routeHopType;
    RvStatus                    rv;

    /*get the first record route header*/
    hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                                    hMsg,
                                                    RVSIP_HEADERTYPE_ROUTE_HOP,
                                                    RVSIP_FIRST_HEADER,
                                                    RVSIP_MSG_HEADERS_OPTION_ALL,
                                                    &listElem);
    while (hRouteHop != NULL)
    {
        routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);
        /*if the route hop is a record route, add its address spec to the
        route address list*/
        if(routeHopType == RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER)
        {

            /*--------------------------------------
               Construct the Route list if needed
            -----------------------------------------*/
            if(*hList == NULL)
            {
                *hList = RLIST_RPoolListConstruct(pCallLeg->pMgr->hGeneralPool,
                                                  listPage, sizeof(void*),
                                                  pCallLeg->pMgr->pLogSrc);
                if(*hList == NULL)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "ListInitialize - Failed for Call-leg 0x%p, Failed to construct a new list",pCallLeg));
                    return RV_ERROR_UNKNOWN;
                }
            }
            /*--------------------------------------
                add the address to the list
            ----------------------------------------*/
            rv = ListAddElement(pCallLeg,hRouteHop,listPage,*hList);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "ListInitialize - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
                /*destruct the route list*/
                *hList = NULL;
                return rv;
            }
        }

        /* get the next route hop*/
        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                                     hMsg,
                                                     RVSIP_HEADERTYPE_ROUTE_HOP,
                                                     RVSIP_NEXT_HEADER,
                                                     RVSIP_MSG_HEADERS_OPTION_ALL,
                                                     &listElem);
    }
    return RV_OK;

}

/***************************************************************************
 * ListAddElement
 * ------------------------------------------------------------------------
 * General: Takes the address from a record route header and insert it to
 *          the end of a given route list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hRecordRoute - record route header to add to the list
 ***************************************************************************/
static RvStatus ListAddElement(IN  CallLeg                 *pCallLeg,
                                IN RvSipRouteHopHeaderHandle hRecordRoute,
                                IN  HPAGE                    listPage,
                                IN RLIST_HANDLE              hList)
{


    RvSipRouteHopHeaderHandle hNewRouteHop;
    RvSipRouteHopHeaderHandle *phNewRouteHop;
    RLIST_ITEM_HANDLE  listItem;
    RvStatus          rv;



    /*add the Route hop to the list - first copy the header to the given page
      and then insert the new handle to the list*/
    rv = RvSipRouteHopHeaderConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool,
                                      listPage, &hNewRouteHop);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "ListAddElement - Failed for Call-leg 0x%p, Failed to construct new RouteHop on page 0x%p",
                  pCallLeg,listPage));
        return rv;
    }

    rv = RvSipRouteHopHeaderCopy(hNewRouteHop, hRecordRoute);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "ListAddElement - Failed for Call-leg 0x%p, Failed to copy new RouteHop header",pCallLeg));
        return rv;
    }

    rv = RLIST_InsertTail(NULL, hList, (RLIST_ITEM_HANDLE *)&listItem);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "ListAddElement - Failed for Call-leg 0x%p, Failed to get new list element",pCallLeg));
        return rv;
    }
    phNewRouteHop = (RvSipRouteHopHeaderHandle*)listItem;
    *phNewRouteHop = hNewRouteHop;
    return RV_OK;
}

/***************************************************************************
 * RecordRouteListAddToResponse
 * ------------------------------------------------------------------------
 * General: Take the route addresses from the route list and build a record
 *          route list in the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static RvStatus RecordRouteListAddToResponse (IN  CallLeg                 *pCallLeg,
                                          IN  RvSipMsgHandle          hMsg)
{

    RvSipRouteHopHeaderHandle  hRecordRoute = NULL;
    RvSipRouteHopHeaderHandle  *phRouteHop  = NULL;
    RLIST_ITEM_HANDLE       pItem        = NULL;     /*current item*/
    RLIST_ITEM_HANDLE       pNextItem    = NULL;     /*next item*/
    RvStatus               rv;

    /*check if the list is empty*/
    if(CallLegRouteListIsEmpty(pCallLeg))
    {
        return RV_OK;
    }

    /*get the first address*/
    RLIST_GetHead(NULL, pCallLeg->hRouteList,&pItem);
    phRouteHop = (RvSipRouteHopHeaderHandle*)pItem;

    while(phRouteHop != NULL)
    {
        /*set a new record route header in the message*/
        rv = RvSipRouteHopHeaderConstructInMsg(hMsg,RV_FALSE,&hRecordRoute);
        if(rv != RV_OK)
        {
            return rv;
        }

        rv = RvSipRouteHopHeaderCopy(hRecordRoute,*phRouteHop);
        if(rv != RV_OK)
        {
            return rv;
        }
        RvSipRouteHopHeaderSetHeaderType(hRecordRoute,RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER);

        /*get the next address*/
        RLIST_GetNext(NULL, pCallLeg->hRouteList, pItem, &pNextItem);

        pItem = pNextItem;
        phRouteHop = (RvSipRouteHopHeaderHandle*)pItem;


    }
    return RV_OK;
}

/***************************************************************************
 * RouteListAddToRequest
 * ------------------------------------------------------------------------
 * General:
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static RvStatus RouteListAddToRequest (IN  CallLeg         *pCallLeg,
                                       IN  RvSipMsgHandle  hMsg)
{
    /*check if the list is empty*/
    if(CallLegRouteListIsEmpty(pCallLeg))
    {
        return RV_OK;
    }
    if(pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_OUTGOING)
    {
        return RouteListAddToClientRequest(pCallLeg,hMsg);
    }
    else
    {
        return RouteListAddToServerRequest(pCallLeg,hMsg);
    }

}

/***************************************************************************
 * RouteListAddToClientRequest
 * ------------------------------------------------------------------------
 * General: Loop over the route list and add the Route headers to the request
 *          message in reverse order.
 *          The first item on the route list becomes
 *          the Request-URI and the rest become part of the message route
 *            headers.
 *          This method prepares only the messages Route header list, hence the
 *          first item in the route list is skipped. The first item is accessed
 *          from the call-leg when creating a UAC transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static RvStatus RouteListAddToClientRequest (IN  CallLeg         *pCallLeg,
                                              IN  RvSipMsgHandle  hMsg)
{
    RvSipRouteHopHeaderHandle hRoute;
    RvSipRouteHopHeaderHandle *phListRoute= NULL;
    RLIST_ITEM_HANDLE      pItem          = NULL;     /*current item*/
    RLIST_ITEM_HANDLE      pPrevItem      = NULL;     /*prev item*/
    RvStatus              rv;
    RvBool                bIsLooseRoute;

    /*the last record route is ignored and will be used as the requrst uri*/
    RLIST_GetTail(NULL, pCallLeg->hRouteList,&pItem);

    phListRoute = (RvSipRouteHopHeaderHandle*)pItem;

    bIsLooseRoute = IsLooseRouter(*phListRoute);

    if(bIsLooseRoute == RV_FALSE)
    {
        /*this is the first address that will be inserted into the request list*/
        RLIST_GetPrev(NULL, pCallLeg->hRouteList, pItem, &pPrevItem);

        phListRoute = (RvSipRouteHopHeaderHandle*)pPrevItem;
        pItem = pPrevItem;
    }

    while(phListRoute != NULL)
    {
        /*set a new Route header in the message*/
        rv = RvSipRouteHopHeaderConstructInMsg(hMsg,RV_FALSE,&hRoute);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = RvSipRouteHopHeaderCopy(hRoute,*phListRoute);
        if(rv != RV_OK)
        {
            return rv;
        }
        RvSipRouteHopHeaderSetHeaderType(hRoute,RVSIP_ROUTE_HOP_ROUTE_HEADER);

        /*get the next address*/
        RLIST_GetPrev(NULL,
                      pCallLeg->hRouteList, pItem, &pPrevItem);

        pItem = pPrevItem;
        phListRoute = (RvSipRouteHopHeaderHandle*)pItem;
    }

    if(bIsLooseRoute == RV_FALSE)
    {
        /*add the remote contact to the end of the list*/
        return RouteListAppendRemoteContact(pCallLeg,hMsg);
    }
    else
    {
        return RV_OK;
    }
}

/***************************************************************************
 * RouteListAddToServerRequest
 * ------------------------------------------------------------------------
 * General:  Loop over route list and add the Route headers to the request
 *           message. Then add the remote contact as the last (bottom)
 *           Route header.
 *           The first item on the route list becomes the Request-URI and
 *           the rest become part of the message routes headers.
 *           This method prepares only the messages Route header list,
 *           hence the first item in the route list is skipped. The first item
 *           is accessed from the call-leg when creating a UAC transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static RvStatus RouteListAddToServerRequest (IN  CallLeg         *pCallLeg,
                                              IN  RvSipMsgHandle  hMsg)
{
    RvSipRouteHopHeaderHandle hRoute      = NULL;
    RvSipRouteHopHeaderHandle *phListRoute = NULL; /*header taken from the list*/
    RLIST_ITEM_HANDLE         pItem       = NULL; /*current item*/
    RLIST_ITEM_HANDLE         pNextItem   = NULL; /*next item*/
    RvStatus                 rv;
    RvBool                bIsLooseRoute;

    /*the first record route is ignored and will be used as the requrst uri*/
    RLIST_GetHead(NULL,
                  pCallLeg->hRouteList,&pItem);

    phListRoute = (RvSipRouteHopHeaderHandle*)pItem;

    bIsLooseRoute = IsLooseRouter(*phListRoute);

    if(bIsLooseRoute == RV_FALSE)
    {
        /*this is the first address that will be inserted into the request list*/
        RLIST_GetNext(NULL,
                      pCallLeg->hRouteList, pItem, &pNextItem);

        phListRoute = (RvSipRouteHopHeaderHandle*)pNextItem;
        pItem = pNextItem;
    }

    while(phListRoute != NULL)
    {
        /*set a new Route header in the message*/
        rv = RvSipRouteHopHeaderConstructInMsg(hMsg,RV_FALSE,&hRoute);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = RvSipRouteHopHeaderCopy(hRoute,*phListRoute);
        if(rv != RV_OK)
        {
            return rv;
        }
        RvSipRouteHopHeaderSetHeaderType(hRoute,RVSIP_ROUTE_HOP_ROUTE_HEADER);

        /*get the next address*/
        RLIST_GetNext(NULL,
                      pCallLeg->hRouteList, pItem, &pNextItem);

        pItem = pNextItem;
        phListRoute = (RvSipRouteHopHeaderHandle*)pItem;
    }
    if(bIsLooseRoute == RV_FALSE)
    {
        /*add the remote contact to the end of the list*/
        return RouteListAppendRemoteContact(pCallLeg,hMsg);
    }
    else
    {
        return RV_OK;
    }
}





/***************************************************************************
 * RouteListAppendRemoteContact
 * ------------------------------------------------------------------------
 * General: If there is a remote contact it should be added to end of the
 *          route list for both client and server requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
static RvStatus RouteListAppendRemoteContact(IN  CallLeg         *pCallLeg,
                                              IN  RvSipMsgHandle  hMsg)
{
    RvStatus              rv;
    RvSipRouteHopHeaderHandle hRoute      = NULL;
    RvSipAddressHandle hContact = NULL;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    // Attempt to get request URI, if hRemoteContact does not exists, get it from FROM or TO header. 
    // The purpose is to ensure that the first INVITE with a none "lr" Route header has the 
    // request URI in the Route list. 
    rv = CallLegGetRemoteTargetURI( pCallLeg, &hContact );
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RouteListAppendRemoteContact -Call-leg 0x%p, Failed to CallLegGetRemoteTargetURI",pCallLeg));
        return rv;
    }

#else
    /*if there is no remote contact then we are done*/
    if(pCallLeg->hRemoteContact == NULL)
    {
        return RV_OK;
    }
    else
    {
        hContact = pCallLeg->hRemoteContact;
    }
#endif
/* SPIRENT_END */

    /*construct a new Route header at the end of the message list*/
    rv = RvSipRouteHopHeaderConstructInMsg(hMsg,RV_FALSE,&hRoute);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RouteListAppendRemoteContact -Call-leg 0x%p, Failed to construct Route header",pCallLeg));
        return rv;
    }

    /*set the header type to Route*/
    rv = RvSipRouteHopHeaderSetHeaderType(hRoute,RVSIP_ROUTE_HOP_ROUTE_HEADER);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RouteListAppendRemoteContact -Call-leg 0x%p, Failed to set Route header type",pCallLeg));
        return rv;
    }

    /*set the Route header address spec to the remote contact*/
    rv = RvSipRouteHopHeaderSetAddrSpec(hRoute,hContact);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RouteListAppendRemoteContact - Call-leg 0x%p, Failed to set Route header address",pCallLeg));
        return rv;
    }

    return RV_OK;
}


/***************************************************************************
 * IsLooseRouter
 * ------------------------------------------------------------------------
 * General: Checks weather the route hop is a loose router, meaning, weather the
 *          address in the header contains the lr parameter or not.
 * Return Value: RV_TRUE if the header contains the lr parameter, RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRouteHopHeader - The route header to check if it is a loose route.
 ***************************************************************************/
static RvBool IsLooseRouter(IN RvSipRouteHopHeaderHandle hRouteHopHeader)
{
    RvSipAddressHandle phAddress;

    phAddress = NULL;

    /*get the address spec from the route hop*/
    phAddress = RvSipRouteHopHeaderGetAddrSpec(hRouteHopHeader);

    return RvSipAddrUrlGetLrParam(phAddress);
}


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
                                IN  RvChar                  *routeValue)
{
    RvStatus rv = RV_OK;
    RLIST_ITEM_HANDLE  listItem;
    RLIST_GetTail(NULL, pCallLeg->hRouteList, &listItem);
    if (listItem) {
        RvSipRouteHopHeaderHandle *phNewRouteHop = (RvSipRouteHopHeaderHandle*)listItem;
        if(strstr(routeValue,"Route:")||strstr(routeValue,"route:")||strstr(routeValue,"ROUTE:")) {
          rv = RvSipRouteHopHeaderParse( *phNewRouteHop, routeValue );
        } else {
          rv = RvSipRouteHopHeaderParseValue( *phNewRouteHop, routeValue );
        }
    }
    return rv;
}

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
                                IN  RvChar                  *routeValue)
{
    RvSipRouteHopHeaderHandle hNewRouteHop;
    RvSipRouteHopHeaderHandle *phNewRouteHop;
    RLIST_ITEM_HANDLE  listItem;
    RvStatus          rv;

    /*add the Route hop to the list - first create the header to the given page
      and then insert the new handle to the list*/
    rv = RvSipRouteHopHeaderConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool,
                                      listPage, &hNewRouteHop);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "%s - Failed for Call-leg 0x%p, Failed to construct new RouteHop on page %d",
                  __FUNCTION__, pCallLeg,listPage));
        return rv;
    }

    if(strstr(routeValue,"Route:")||strstr(routeValue,"route:")||strstr(routeValue,"ROUTE:")) {
      rv = RvSipRouteHopHeaderParse( hNewRouteHop, routeValue );
    } else {
      rv = RvSipRouteHopHeaderParseValue( hNewRouteHop, routeValue );
    }

    if(rv != RV_Success)
    {
      RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "%s - Failed in RvSipRouteHopHeaderParseValue for Call-leg 0x%p",
                 __FUNCTION__,pCallLeg));
      return rv;
    }

    rv = RLIST_InsertHead(NULL, hList, (RLIST_ITEM_HANDLE *)&listItem);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "%s - Failed for Call-leg 0x%p, Failed to get new list element",__FUNCTION__,pCallLeg));
        return rv;
    }
    phNewRouteHop = (RvSipRouteHopHeaderHandle*)listItem;
    *phNewRouteHop = hNewRouteHop;
    return RV_OK;
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * AddRouteListToRequestMsgIfNeed
 * ------------------------------------------------------------------------
 * General: Adds a RouteList to CallLeg sent request
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg
 *        hTransc  - handle to the transaction.
 *        hMsg     - Handle to the message.
 ***************************************************************************/
static RvStatus AddRouteListToRequestMsgIfNeed(
                                         IN  CallLeg                 *pCallLeg,
                                         IN  RvSipTranscHandle       hTransc,
                                         IN  RvSipMsgHandle          hMsg)
{
    RvStatus              rv;
    /* The order of the following definitions is important due */
    /* to its assignments - DO NOT CHANGE */
    RvSipCSeqHeaderHandle hCSeq      = RvSipMsgGetCSeqHeader(hMsg);
    RvSipMethodType       eMsgMethod = RvSipCSeqHeaderGetMethodType(hCSeq);

    /* The RouteList is NOT added to ACK(after rejected INVITE) or CANCEL */
    /* outgoing requests, which are sent in not-connected state           */
    switch (eMsgMethod)
    {
    case RVSIP_METHOD_ACK:
        {
            RvUint16 transcResponse;
            SipTransactionGetResponseCode(hTransc,&transcResponse);
            /* If the transc response is >= 300 we continue to the next code */
            /* block check, otherwise we can move on */
            if(transcResponse < 300)
            {
                break;
            }
        }
        /* If the transc response is >= 300 we continue to the next code block */
    case RVSIP_METHOD_CANCEL:
        if (pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AddRouteListToRequestMsgIfNeed - Call-leg 0x%p, will not add route list to ACK or CANCEL msg 0x%p of initial invite transaction 0x%p",
                pCallLeg,hMsg,hTransc));
            return RV_OK;
        }
    default:
        break;
    }

    rv = RouteListAddToRequest(pCallLeg,hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "AddRouteListToRequestMsgIfNeed - Failed for Call-leg 0x%p",pCallLeg));
    }

    return RV_OK;
}

/***************************************************************************
 * AddRouteListToResponseMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: Adds a RouteList to CallLeg sent response
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg
 *        hTransc  - handle to the transaction.
 *        hMsg     - Handle to the message.
 ***************************************************************************/
static RvStatus AddRouteListToResponseMsgIfNeeded(
                                          IN  CallLeg                 *pCallLeg,
                                          IN  RvSipTranscHandle       hTransc,
                                          IN  RvSipMsgHandle          hMsg)
{
    RvStatus             rv;
    RvInt16              responseCode = RvSipMsgGetStatusCode(hMsg);
    SipTransactionMethod eMethod;
    CallLegTranscMemory *pMemory;

    SipTransactionGetMethod(hTransc,&eMethod);

    /*-----------------------------------------------------------
       Add Record Route set to 1xx and 2xx responses of an early
       dialog for responses of INVITE, REFER and SUBSCRIBE. These are
       the only methods that can establish a dialog. The subscribe should
       be created with the subscription API other wise it cannot establish
       a dialog.
      ----------------------------------------------------------*/
    if(responseCode > 100 &&
       responseCode < 300 &&
       pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        switch (eMethod)
        {
#ifdef RV_SIP_SUBS_ON
        case SIP_TRANSACTION_METHOD_SUBSCRIBE:
            {
                RvBool bInitial;

                /*verify that this is the initial subscription and not a refresh*/
                rv = IsInitialSubscription(hTransc,&bInitial);
                if (rv != RV_OK || bInitial == RV_FALSE)
                {
                    return rv;
                }
            }
            /* Continue to the next block */
        case SIP_TRANSACTION_METHOD_REFER:
#endif /* #ifdef RV_SIP_SUBS_ON */
        case SIP_TRANSACTION_METHOD_INVITE:
            rv =  RecordRouteListAddToResponse(pCallLeg,hMsg);
            if(rv != RV_OK)
            {
                return rv;
            }
            break;
        default:
            break;
        }

        return RV_OK;
    }

    /* Destruct the list if we respond with reject (responseCode <= 100||responseCode >= 300)*/
    /* The context of the transaction determines weather the Reject response is
    because the message was an incoming INVITE transaction inside an INVITE transaction
    (when the call is in the OFFERING state. If this is the case then the record route
    list is not destructed here. */

    pMemory = (CallLegTranscMemory*)SipTransactionGetContext(hTransc);
    if(pMemory!= NULL &&
       pMemory->dontDestructRouteList == RV_TRUE)
    {
        return RV_OK;
    }
    /* Don't destruct the route list if the method is not invite */
    if(eMethod          != SIP_TRANSACTION_METHOD_INVITE  ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ||
       responseCode     < 200)
    {
        return RV_OK;
    }
    pCallLeg->hRouteList = NULL;

    return RV_OK;
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * IsInitialSubscription
 * ------------------------------------------------------------------------
 * General:  is the initial subscription and not a refresh
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg
 *        hTransc  - handle to the transaction.
 *        hMsg     - Handle to the message.
 ***************************************************************************/
static RvStatus IsInitialSubscription(IN  RvSipTranscHandle  hTransc,
                                      OUT RvBool            *pbInitSubs)
{
    void*          pSubsInfo  = NULL;
    RvSipSubsState eSubsState = RVSIP_SUBS_STATE_UNDEFINED;

    *pbInitSubs = RV_TRUE;
    pSubsInfo = SipTransactionGetSubsInfo(hTransc);
    if(pSubsInfo == NULL)
    {
        *pbInitSubs = RV_FALSE;
        return RV_OK;
    }
    eSubsState = SipSubsGetCurrState((RvSipSubsHandle)pSubsInfo);
    if(eSubsState != RVSIP_SUBS_STATE_SUBS_RCVD)
    {
        *pbInitSubs = RV_FALSE;
        return RV_OK;
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /*RV_SIP_PRIMITIVES */

