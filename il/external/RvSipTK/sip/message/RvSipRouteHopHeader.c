/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipRouteHopHeader.c                                  *
 *                                                                            *
 * The file defines the functions of the Route and Recorde Route objects      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change parameters.                                                         *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Sarit Mekler     April.2001                                            *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipRouteHopHeader.h"
#include "_SipRouteHopHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void RouteHopHeaderClean( IN MsgRouteHopHeader* pHeader,
                                 IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipRouteHopHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a RouteHop header object inside a given message object.
 *          The header is kept in the header list of the message. You can
 *          choose to insert the header either at the head or tail of the list.
 *          Use RvSipRouteHopHeaderSetHeaderType to determine whether the
 *          object represents a Route or a Record-Route header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader - Handle to the newly constructed Route header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           IN  RvBool                 pushHeaderAtHead,
                                           OUT RvSipRouteHopHeaderHandle* hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem= NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipRouteHopHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                         msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_ROUTE_HOP,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipRouteHopHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone RouteHop Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 *          Use RvSipRouteHopHeaderSetHeaderType to determine whether the
 *          object represents a Route or a Record-Route header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed RouteHop header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipRouteHopHeaderHandle* hHeader)
{
    MsgRouteHopHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgRouteHopHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                hPool,
                                                hPage,
                                                sizeof(MsgRouteHopHeader),
                                                RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRouteHopHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;
    RouteHopHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipRouteHopHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRouteHopHeaderConstruct - RouteHop header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
             hPool, hPage, *hHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source RouteHop header object to a destination
 *          RouteHop header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination RouteHop header object.
 *    hSource      - Handle to the source RouteHop header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderCopy(
                                         INOUT RvSipRouteHopHeaderHandle hDestination,
                                         IN    RvSipRouteHopHeaderHandle hSource)
{
    RvStatus         stat;
    MsgRouteHopHeader*   source;
    MsgRouteHopHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgRouteHopHeader*)hSource;
    dest   = (MsgRouteHopHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    /*header type*/
    dest->headerType = source->headerType;


    /* AddrSpec */
    stat = RvSipRouteHopHeaderSetAddrSpec(hDestination, source->hAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipRouteHopHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* display name */
    if(source->strDisplayName > UNDEFINED)
    {
        dest->strDisplayName = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                               dest->hPool,
                                                               dest->hPage,
                                                               source->hPool,
                                                               source->hPage,
                                                               source->strDisplayName);
        if (dest->strDisplayName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderCopy - Failed to copy displayName. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pDisplayName = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                              dest->strDisplayName);
#endif
    }
    else
    {
        dest->strDisplayName = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDisplayName = NULL;
#endif
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    /* otherParams */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                               dest->hPool,
                                                               dest->hPage,
                                                               source->hPool,
                                                               source->hPage,
                                                               source->strOtherParams);
        if (dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderCopy - Failed to copy other param. stat %d",
                stat));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOtherParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strOtherParams);
#endif
    }
    else
    {
        dest->strOtherParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOtherParams = NULL;
#endif
    }

    /* bad syntax */
    if(source->strBadSyntax > UNDEFINED)
    {
        dest->strBadSyntax = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strBadSyntax);
        if(dest->strBadSyntax == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderCopy - failed in coping strBadSyntax."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pBadSyntax = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strBadSyntax);
#endif
    }
    else
    {
        dest->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBadSyntax = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a RouteHop header object to a textual Route/Record-Route header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the RouteHop header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderEncode(
                                          IN    RvSipRouteHopHeaderHandle hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if ((hPool == NULL) || (pHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /*length will give the real length, and will not just add the
                   new length to the old one */
    stat = RouteHopHeaderEncode(hHeader, hPool, *hPage,RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderEncode - Failed. Free page 0x%p on pool 0x%p. RouteHop HeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * RouteHopHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            RouteHop header (as string) on it.
 *          Route Header =
 *          Record-Route = Record-Route: 1# ( name-addr *(;rr-param ))
 *          Route = Route:1# ( name-addr *(;rr-param ))
 *          Path =  Path:1# ( name-addr *(;rr-param ))
 *          Service-Route =  Service-Route: 1# ( name-addr *(;rr-param ))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the RouteHop header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderEncode(
                                IN    RvSipRouteHopHeaderHandle hHeader,
                                IN    HRPOOL                 hPool,
                                IN    HPAGE                  hPage,
                                IN    RvBool                 bInUrlHeaders,
                                INOUT RvUint32*              length)
{
    MsgRouteHopHeader*   pHeader;
    RvStatus         stat = RV_OK;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgRouteHopHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RouteHopHeaderEncode - Encoding RouteHop header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
              hHeader, hPool, hPage));

    /* set "Route: " or "Record-Route: or "Path: "  or "Service-Route: " */
    if(pHeader->headerType == RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER)
    {
        /* set "Record-Route" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Record-Route", length);
    }
    else if(pHeader->headerType == RVSIP_ROUTE_HOP_ROUTE_HEADER)
    {
        /* set "Route" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Route", length);
    }
    else if(pHeader->headerType == RVSIP_ROUTE_HOP_PATH_HEADER)
    {
        /* set "Path" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Path", length);
    }
    else if (pHeader->headerType == RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER)
    {
        /* set "Service-Route" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Service-Route", length);
    }
    else
    {
        RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RouteHopHeaderEncode - header 0x%pheader type was not set (route/record-route/path/service-route!!!.",
            hHeader));
        return RV_ERROR_BADPARAM;
    }


    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RouteHopHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }

    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    length);
    if (stat != RV_OK)
        return stat;

    /* bad - syntax */
    if(pHeader->strBadSyntax > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBadSyntax,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RouteHopHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* set [displayName]"<"spec ">"*/
    if(pHeader->strDisplayName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDisplayName,
                                    length);
        if (stat != RV_OK)
            return stat;
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    if (pHeader->hAddrSpec != NULL)
    {
        /* set spec */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
            return stat;
    }
    else
        return RV_ERROR_UNKNOWN;

    /* Route Params with ";" in the beginning */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        /* set pHeader->strOtherParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
        if (stat != RV_OK)
            return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Route or Record-Route header-for example,
 *        "Record-Route: <sip:UserB@there.com;maddr=ss1.wcom.com>"-into a
 *         Route header object. All the textual fields are placed inside
 *         the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Route header object.
 *    buffer    - Buffer containing a textual RouteHop header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderParse(
                                     IN    RvSipRouteHopHeaderHandle   hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RouteHopHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ROUTE,
                                        buffer,
                                        RV_FALSE,
                                        (void*)hHeader);;
    if(rv == RV_OK)
    {
        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
    }

    return rv;
}

/***************************************************************************
 * RvSipRouteHopHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual RouteHop header value into an RouteHop header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipRouteHopHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the RouteHop header object.
 *    buffer    - The buffer containing a textual RouteHop header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderParseValue(
                                     IN    RvSipRouteHopHeaderHandle  hHeader,
                                     IN    RvChar*                    buffer)
{
    MsgRouteHopHeader*    pHeader = (MsgRouteHopHeader*)hHeader;
	SipParseType          eParseType;
	RvStatus              rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

	switch(pHeader->headerType) {
	case RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER:
		{
			eParseType = SIP_PARSETYPE_RECORD_ROUTE;
			break;
		}
	case RVSIP_ROUTE_HOP_PATH_HEADER:
		{
			eParseType = SIP_PARSETYPE_PATH;
			break;
		}
	case RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER:
		{
			eParseType = SIP_PARSETYPE_SERVICE_ROUTE;
			break;
		}
	default:
		{
			eParseType = SIP_PARSETYPE_ROUTE;
			break;
		}
	}
	
    RouteHopHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        eParseType,
                                        buffer,
                                        RV_TRUE,
                                         (void*)hHeader);;
    if(rv == RV_OK)
    {
        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
    }

    return rv;
}

/***************************************************************************
 * RvSipRouteHopHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an RouteHop header with bad-syntax information.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as it was.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderFix(
                                     IN RvSipRouteHopHeaderHandle hHeader,
                                     IN RvChar*                  pFixedBuffer)
{
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipRouteHopHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRouteHopHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRouteHopHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipRouteHopHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RouteHop header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hRoute - The RouteHop header to take the page from.
 ***************************************************************************/
HRPOOL SipRouteHopHeaderGetPool(RvSipRouteHopHeaderHandle hRoute)
{
    return ((MsgRouteHopHeader*)hRoute)->hPool;
}

/***************************************************************************
 * SipRouteHopHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RouteHop header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hRoute - The RouteHop header to take the page from.
 ***************************************************************************/
HPAGE SipRouteHopHeaderGetPage(RvSipRouteHopHeaderHandle hRoute)
{
    return ((MsgRouteHopHeader*)hRoute)->hPage;
}

/***************************************************************************
 * RvSipRouteHopHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the RouteHop header fields are kept in a string format-for example, the
 *          RouteHop header display name. In order to get such a field from the RouteHop header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the RouteHop header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipRouteHopHeaderGetStringLength(
                                      IN  RvSipRouteHopHeaderHandle     hHeader,
                                      IN  RvSipRouteHopHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
        case RVSIP_ROUTE_HOP_DISPLAY_NAME:
        {
            stringOffset = SipRouteHopHeaderGetDisplayName(hHeader);
            break;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        case RVSIP_ROUTE_HOP_OTHER_PARAMS:
        {
            stringOffset = SipRouteHopHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_ROUTE_HOP_BAD_SYNTAX:
        {
            stringOffset = SipRouteHopHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipRouteHopHeaderGetPool(hHeader),
                             SipRouteHopHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipRouteHopHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RouteHop header object
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetDisplayName(IN RvSipRouteHopHeaderHandle hHeader)
{
    return ((MsgRouteHopHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipRouteHopHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the RouteHop header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderGetDisplayName(
                                               IN RvSipRouteHopHeaderHandle   hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgRouteHopHeader*)hHeader)->hPool,
                                  ((MsgRouteHopHeader*)hHeader)->hPage,
                                  SipRouteHopHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRouteHopHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          RouteHop Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the RouteHop header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipRouteHopHeaderSetDisplayName(IN    RvSipRouteHopHeaderHandle hHeader,
                                       IN    RvChar *              strDisplayName,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOffset)
{
    RvInt32           newStr;
    MsgRouteHopHeader* pHeader;
    RvStatus          retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgRouteHopHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strDisplayName,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strDisplayName = newStr;
#ifdef SIP_DEBUG
    pHeader->pDisplayName = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strDisplayName);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the RouteHop header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the header object.
 *    strDisplayName - The display name to be set in the RouteHop header.
 *                   If NULL is supplied, the existing
 *                   display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetDisplayName(
                                             IN    RvSipRouteHopHeaderHandle hHeader,
                                             IN    RvChar *              strDisplayName)
{
    /* validity checking will be done in the internal function */
    return SipRouteHopHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipRouteHopHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the RouteHop header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RouteHop header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipRouteHopHeaderGetAddrSpec
                                            (IN RvSipRouteHopHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgRouteHopHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipRouteHopHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec address object in the RouteHop header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the RouteHop header object.
 *  hAddrSpec - Handle to the Address Spec address object to be set in
 *              the RouteHop header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetAddrSpec
                                            (IN    RvSipRouteHopHeaderHandle hHeader,
                                             IN    RvSipAddressHandle     hAddrSpec)
{
    RvStatus           stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgRouteHopHeader  *pHeader         = (MsgRouteHopHeader*)hHeader;


    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hAddrSpec == NULL)
    {
        pHeader->hAddrSpec = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
               return RV_ERROR_BADPARAM;
        }
        if (NULL != pHeader->hAddrSpec)
        {
            currentAddrType = RvSipAddrGetAddrType(pHeader->hAddrSpec);
        }

        /* if no address object was allocated, we will construct it */
        if((pHeader->hAddrSpec == NULL) ||
           (currentAddrType != pAddr->eAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInRouteHopHeader(hHeader,
                                                      pAddr->eAddrType,
                                                      &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }
        return RvSipAddrCopy(pHeader->hAddrSpec,
                             hAddrSpec);
    }
}

/***************************************************************************
 * SipRouteHopHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams.
 * Return Value: Other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RouteHop header object
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetOtherParams(IN RvSipRouteHopHeaderHandle hHeader)
{
    return ((MsgRouteHopHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipRouteHopHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the RouteHop header other params field of the RouteHop header object into a
 *          given buffer.
 *          Not all the RouteHop header parameters have separated fields in the RouteHop header
 *          object. Parameters with no specific fields are refered to as other params. They
 *          are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the RouteHop header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderGetOtherParams(
                                               IN RvSipRouteHopHeaderHandle   hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)

{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgRouteHopHeader*)hHeader)->hPool,
                                  ((MsgRouteHopHeader*)hHeader)->hPage,
                                  SipRouteHopHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRouteHopHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          RouteHop Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the RouteHop header object
 *          strOtherParams - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipRouteHopHeaderSetOtherParams(  IN    RvSipRouteHopHeaderHandle hHeader,
                                         IN    RvChar *              strOtherParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strOtherOffset)
{
    RvInt32        newStr;
    MsgRouteHopHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgRouteHopHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOtherOffset, strOtherParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strOtherParams = newStr;
#ifdef SIP_DEBUG
    pHeader->pOtherParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strOtherParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the RouteHop header object.
 *          Not all the RouteHop header parameters have separated fields in the RouteHop header
 *          object. Parameters with no specific fields are refered to as other params. They
 *          are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the RouteHop header object.
 *    strOtherParams - The Other Params string to be set in the RouteHop header.
 *                   If NULL is supplied, the existing Other Params field is
 *                   removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetOtherParams(
                                             IN    RvSipRouteHopHeaderHandle hHeader,
                                             IN    RvChar *              strOtherParams)
{
    /* validity checking will be done in the internal function */
    return SipRouteHopHeaderSetOtherParams(hHeader, strOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipRouteHopHeaderGetHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the RouteHop Header object.
 * Return Value: Returns the RouteHop header type enumeration from the RouteHop
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RouteHop header object.
 ***************************************************************************/
RVAPI RvSipRouteHopHeaderType RVCALLCONV RvSipRouteHopHeaderGetHeaderType(
                                   IN RvSipRouteHopHeaderHandle hHeader)
{
    MsgRouteHopHeader* pHeader;

    if(hHeader == NULL)
        return RVSIP_ROUTE_HOP_UNDEFINED_HEADER;

    pHeader = (MsgRouteHopHeader*)hHeader;
    return pHeader->headerType;

}

/***************************************************************************
 * RvSipRouteHopHeaderSetHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the RouteHop Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the RouteHop header object.
 *    eHeaderType       - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetHeaderType(
                                 IN    RvSipRouteHopHeaderHandle hHeader,
                                 IN    RvSipRouteHopHeaderType   eHeaderType)
{
    MsgRouteHopHeader* pHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgRouteHopHeader*)hHeader;
    pHeader->headerType = eHeaderType;
    return RV_OK;
}

/***************************************************************************
 * SipRouteHopHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetStrBadSyntax(
                                    IN RvSipRouteHopHeaderHandle hHeader)
{
    return ((MsgRouteHopHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipRouteHopHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad RouteHop header,
 *          and you wish to see the header-value.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer with which to fill the bad syntax string.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the bad syntax + 1, to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderGetStrBadSyntax(
                                               IN RvSipRouteHopHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgRouteHopHeader*)hHeader)->hPool,
                                  ((MsgRouteHopHeader*)hHeader)->hPage,
                                  SipRouteHopHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRouteHopHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipRouteHopHeaderSetStrBadSyntax(
                                  IN RvSipRouteHopHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgRouteHopHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgRouteHopHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
                                  strBadSyntax, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strBadSyntax = newStrOffset;
#ifdef SIP_DEBUG
    header->pBadSyntax = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                      header->strBadSyntax);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipRouteHopHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal RouteHop header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetStrBadSyntax(
                                  IN RvSipRouteHopHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipRouteHopHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipRouteHopHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the RouteHop header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipRouteHopHeader - Handle to the RouteHop header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the RouteHop header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderGetRpoolString(
                             IN    RvSipRouteHopHeaderHandle      hSipRouteHopHeader,
                             IN    RvSipRouteHopHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hSipRouteHopHeader;
    RvInt32      requestedParamOffset;

    if(hSipRouteHopHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRouteHopHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_ROUTE_HOP_DISPLAY_NAME:
        requestedParamOffset = pHeader->strDisplayName;
        break;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    case RVSIP_ROUTE_HOP_OTHER_PARAMS:
        requestedParamOffset = pHeader->strOtherParams;
        break;
    case RVSIP_ROUTE_HOP_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRouteHopHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
    /* the parameter does not exit in the header - return UNDEFINED*/
    if(requestedParamOffset == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(
                                     pHeader->pMsgMgr,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pHeader->hPool,
                                     pHeader->hPage,
                                     requestedParamOffset);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRouteHopHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipRouteHopHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the RouteHop header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipRouteHopHeader - Handle to the RouteHop header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRouteHopHeaderSetRpoolString(
                             IN    RvSipRouteHopHeaderHandle      hSipRouteHopHeader,
                             IN    RvSipRouteHopHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr)
{
    MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)hSipRouteHopHeader;
    RvInt32*     pDestParamOffset;

    if(hSipRouteHopHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipRouteHopHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_ROUTE_HOP_DISPLAY_NAME:
        pDestParamOffset = &(pHeader->strDisplayName);
        break;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    case RVSIP_ROUTE_HOP_OTHER_PARAMS:
        pDestParamOffset = &(pHeader->strOtherParams);
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRouteHopHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

    *pDestParamOffset = MsgUtilsAllocCopyRpoolString(
                                     pHeader->pMsgMgr,
                                     pHeader->hPool,
                                     pHeader->hPage,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);

    if (*pDestParamOffset == UNDEFINED)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRouteHopHeaderSetRpoolString - Failed to copy the supplied string into the header page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RouteHopHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void RouteHopHeaderClean(IN MsgRouteHopHeader* pHeader,
                                IN RvBool            bCleanBS)
{
    pHeader->hAddrSpec      = NULL;
    pHeader->strOtherParams = UNDEFINED;
    pHeader->eHeaderType    = RVSIP_HEADERTYPE_ROUTE_HOP;
#ifndef RV_SIP_JSR32_SUPPORT
    pHeader->strDisplayName = UNDEFINED;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

#ifdef SIP_DEBUG
    pHeader->pOtherParams   = NULL;
#ifndef RV_SIP_JSR32_SUPPORT
    pHeader->pDisplayName   = NULL;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->headerType       = RVSIP_ROUTE_HOP_UNDEFINED_HEADER;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}


#ifdef __cplusplus
}
#endif

