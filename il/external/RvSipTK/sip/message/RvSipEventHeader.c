/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                      RvSipEventHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the Event header object:                   *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             May 2002                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"


#ifndef RV_SIP_PRIMITIVES

#include "RvSipEventHeader.h"
#include "_SipEventHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void EventHeaderClean(IN MsgEventHeader* pHeader,
							 IN RvBool          bCleanBS);


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipEventHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Event Header object inside a given message
 *          object. The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE),
 *                            or to the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Event Header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderConstructInMsg(
                               IN  RvSipMsgHandle          hSipMsg,
                               IN  RvBool                 pushHeaderAtHead,
                               OUT RvSipEventHeaderHandle* hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipEventHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);

    if(stat != RV_OK)
    {
        return stat;
    }

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_EVENT,
                              &hListElem,
                              (void**)hHeader);

}

/***************************************************************************
 * RvSipEventHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Event Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Event Header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderConstruct(
                                IN  RvSipMsgMgrHandle       hMsgMgr,
                                IN  HRPOOL                  hPool,
                                IN  HPAGE                   hPage,
                                OUT RvSipEventHeaderHandle* hHeader)
{
    MsgEventHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hPage == NULL_PAGE) || (hPool == NULL) || (hMsgMgr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgEventHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgEventHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipEventHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    EventHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipEventHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipEventHeaderConstruct - Event header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;

}

/***************************************************************************
 * RvSipEventHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Event Header object to a
 *          destination Event Header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Event Header object.
 *    hSource      - Handle to the source Event Header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderCopy(
                                INOUT  RvSipEventHeaderHandle hDestination,
                                IN     RvSipEventHeaderHandle hSource)
{
    MsgEventHeader *source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgEventHeader*)hSource;
    dest   = (MsgEventHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->bIsCompact  = source->bIsCompact;
    /*dest->eventId     = source->eventId;*/

    /* event package */
    if(source->strEventPackage > UNDEFINED)
    {
        dest->strEventPackage = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                                dest->hPool,
                                                                dest->hPage,
                                                                source->hPool,
                                                                source->hPage,
                                                                source->strEventPackage);
        if(dest->strEventPackage == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipEventHeaderCopy - failed in coping strEventPackage."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pEventPackage = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strEventPackage);
#endif
    }
    else
    {
        dest->strEventPackage = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pEventPackage = NULL;
#endif
    }

    /* event template */
    if(source->strEventTemplate > UNDEFINED)
    {
        dest->strEventTemplate = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                                dest->hPool,
                                                                dest->hPage,
                                                                source->hPool,
                                                                source->hPage,
                                                                source->strEventTemplate);
        if(dest->strEventTemplate == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipEventHeaderCopy - failed in coping strEventTemplate."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pEventTemplate = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                            dest->strEventTemplate);
#endif
    }
    else
    {
        dest->strEventTemplate = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pEventTemplate = NULL;
#endif
    }

    /* event id */
    if(source->strEventId > UNDEFINED)
    {
        dest->strEventId = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strEventId);
        if(dest->strEventId == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipEventHeaderCopy - failed in coping strEventId."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pEventId = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strEventId);
#endif
    }
    else
    {
        dest->strEventId = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pEventId = NULL;
#endif
    }

    /* event param */
    if(source->strEventParams > UNDEFINED)
    {
        dest->strEventParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                                dest->hPool,
                                                                dest->hPage,
                                                                source->hPool,
                                                                source->hPage,
                                                                source->strEventParams);
        if(dest->strEventParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipEventHeaderCopy - failed in coping strEventParams."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pEventParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strEventParams);
#endif
    }
    else
    {
        dest->strEventParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pEventParams = NULL;
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
                "RvSipEventHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipEventHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Event Header object to a textual Event header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Event Header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderEncode(
                                IN    RvSipEventHeaderHandle hHeader,
                                IN    HRPOOL                 hPool,
                                OUT   HPAGE*                 hPage,
                                OUT   RvUint32*              length)
{
    RvStatus stat;
    MsgMgr* pMsgMgr;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pMsgMgr = ((MsgEventHeader*)hHeader)->pMsgMgr;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipEventHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipEventHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = EventHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, RV_FALSE,length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipEventHeaderEncode - Failed. Free page 0x%p on pool 0x%p. EventHeaderEncode failed",
                *hPage, hPool));
    }
    return stat;
}

 /***************************************************************************
 * EventHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Event header (as string) on it.
 *          format: "Event /o :event-package.event-template;id=x;event-param"
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Event header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV EventHeaderEncode(
                            IN    RvSipEventHeaderHandle hHeader,
                            IN    HRPOOL                 hPool,
                            IN    HPAGE                  hPage,
                            IN    RvBool                 bInUrlHeaders,
                            IN    RvBool                 bForceCompactForm,
                            INOUT RvUint32*              length)
{
    MsgEventHeader* pHeader;
    RvStatus       stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgEventHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "EventHeaderEncode - Encoding Event header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* encode "Event" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "o", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Event", length);
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "EventHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }

    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "EventHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }

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
                "EventHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* event package - mandatory */
    if(pHeader->strEventPackage > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                    pHeader->hPool, pHeader->hPage,
                                    pHeader->strEventPackage, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event package. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "EventHeaderEncode - encoding header 0x%p failed! strEventPackage is missing. manatory parameter!",
             hHeader));
        return RV_ERROR_UNKNOWN;
    }

    /* event template */
    if(pHeader->strEventTemplate > UNDEFINED)
    {
        /* encode '.' */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, ".", length);

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                    pHeader->hPool, pHeader->hPage,
                                    pHeader->strEventTemplate, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event template. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* event id */
    if(pHeader->strEventId > UNDEFINED)
    {
        /* encode ';id=' */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "id", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event id. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                    pHeader->hPool, pHeader->hPage,
                                    pHeader->strEventId, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event id. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* event param */
    if(pHeader->strEventParams > UNDEFINED)
    {
        /* encode ';' */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event id. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                    pHeader->hPool, pHeader->hPage,
                                    pHeader->strEventParams, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "EventHeaderEncode - Failed to encode Event params. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    return stat;
}

/***************************************************************************
 * RvSipEventHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Event Header-for example,
 *         "Event : event-package.event-template;id=5;event-param=param-val" -
 *          into an Event Header object.
 *         All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Event Header object.
 *    buffer    - Buffer containing a textual Event Header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderParse(
                                 IN  RvSipEventHeaderHandle  hHeader,
                                 IN  RvChar*                 buffer)
{
    MsgEventHeader* pHeader = (MsgEventHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    EventHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_EVENT,
                                        buffer,
                                        RV_FALSE,
                                         (void*)hHeader);
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
 * RvSipEventHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Event header value into an Event header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipEventHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Event header object.
 *    buffer    - The buffer containing a textual Event header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderParseValue(
                                 IN  RvSipEventHeaderHandle  hHeader,
                                 IN  RvChar*                 buffer)
{
    MsgEventHeader*      pHeader = (MsgEventHeader*)hHeader;
	RvBool               bIsCompact;
    RvStatus             rv;
	
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    EventHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_EVENT,
                                        buffer,
                                        RV_TRUE,
                                         (void*)hHeader);
    
	/* restore is-compact indication */
	pHeader->bIsCompact = bIsCompact;
	
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
 * RvSipEventHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Event header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipEventHeaderFix(
                                     IN RvSipEventHeaderHandle hHeader,
                                     IN RvChar*                pFixedBuffer)
{
    MsgEventHeader* pHeader = (MsgEventHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipEventHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipEventHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipEventHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;

}

/***************************************************************************
 * RvSipEventHeaderIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two event header objects.
 *         event headers considered equal if the event-type and event id parameter,
 *         are equal (case sesitive).
 *         The event-type portion of the "Event" header is compared byte-by-byte,
 *         and the "id" parameter token (if present) is compared byte-by-byte.
 *         An "Event" header containing an "id" parameter never matches an "Event"
 *         header without an "id" parameter.
 *         No other parameters are considered when performing a comparison.
 * Return Value: Returns RV_TRUE if the party header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - A handle to the Event Header object.
 *    hOtherHeader - Handle to the Event header object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipEventHeaderIsEqual(
                                 IN  const RvSipEventHeaderHandle  hHeader,
                                 IN  const RvSipEventHeaderHandle  hOtherHeader)
{
    MsgEventHeader*   first = (MsgEventHeader*)hHeader;
    MsgEventHeader*   other = (MsgEventHeader*)hOtherHeader;

    if((hHeader == NULL) || (hOtherHeader == NULL))
        return RV_FALSE;
	
	if (hHeader == hOtherHeader)
	{
		/* this is the same object */
		return RV_TRUE;
	}
	if (first->strBadSyntax != UNDEFINED || other->strBadSyntax != UNDEFINED)
	{
		/* bad syntax string is uncomparable */
		return RV_FALSE;
	}

    /* event type is made from event package and event template */
    /* event package */
    if(first->strEventPackage > UNDEFINED)
    {
        if(other->strEventPackage > UNDEFINED)
        {
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
            if(SPIRENT_RPOOL_Strcmp(first->hPool, first->hPage, first->strEventPackage,
                            other->hPool, other->hPage, other->strEventPackage) != 0)
#else
            if(RPOOL_Strcmp(first->hPool, first->hPage, first->strEventPackage,
                            other->hPool, other->hPage, other->strEventPackage) == RV_FALSE)
#endif
/* SPIRENT_END */
            {
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                        "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eventPackage is not equal!!!",
                        hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eventPackage is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if (other->strEventPackage > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eventPackage is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
    }

    /* event template */
    if(first->strEventTemplate > UNDEFINED)
    {
        if(other->strEventTemplate > UNDEFINED)
        {
            if(RPOOL_Strcmp(first->hPool, first->hPage, first->strEventTemplate,
                            other->hPool, other->hPage, other->strEventTemplate) == RV_FALSE)
            {
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                        "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventTemplate is not equal!!!",
                        hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventTemplate is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if (other->strEventTemplate > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventTemplate is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
    }
    /* id */
    if(first->strEventId > UNDEFINED)
    {
        if(other->strEventId > UNDEFINED)
        {
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
            if(SPIRENT_RPOOL_Strcmp(first->hPool, first->hPage, first->strEventId,
                other->hPool, other->hPage, other->strEventId) != 0)
#else
            if(RPOOL_Strcmp(first->hPool, first->hPage, first->strEventId,
                other->hPool, other->hPage, other->strEventId) == RV_FALSE)
#endif
/* SPIRENT_END */
            {
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                    "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventId is not equal!!!",
                    hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventId is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if (other->strEventId > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
            "RvSipEventHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strEventId is not equal",
            hHeader, hOtherHeader));
        return RV_FALSE;
    }

    return RV_TRUE;

}

/***************************************************************************
 * SipEventHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Event header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipEventHeaderGetPool(RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipEventHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Event header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipEventHeaderGetPage(RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipEventHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Event Header fields are kept in a string format,
 *          for example, the Event Header other params. In order to get
 *          such a field from the Event Header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to
 *          enable you to allocate an appropriate buffer size before calling
 *          the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Event Header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipEventHeaderGetStringLength(
                                IN  RvSipEventHeaderHandle     hHeader,
                                IN  RvSipEventHeaderStringName stringName)
{
    RvInt32        stringOffset;
    MsgEventHeader* pHeader = (MsgEventHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_EVENT_PACKAGE:
        {
            stringOffset = SipEventHeaderGetEventPackage(hHeader);
            break;
        }
        case RVSIP_EVENT_TEMPLATE:
        {
            stringOffset = SipEventHeaderGetEventTemplate(hHeader);
            break;
        }
        case RVSIP_EVENT_PARAM:
        {
            stringOffset = SipEventHeaderGetEventParam(hHeader);
            break;
        }
        case RVSIP_EVENT_ID:
        {
            stringOffset = SipEventHeaderGetEventId(hHeader);
            break;
        }
        case RVSIP_EVENT_BAD_SYNTAX:
        {
            stringOffset = SipEventHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipEventHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool,
                            pHeader->hPage,
                            stringOffset) + 1);
    else
        return 0;

}


/***************************************************************************
 * RvSipEventHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Event header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipEventHeader - Handle to the Event header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetRpoolString(
                             IN    RvSipEventHeaderHandle      hSipEventHeader,
                             IN    RvSipEventHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                   *pRpoolPtr)
{
    MsgEventHeader* pHeader;
    RvInt32        requestedParamOffset;

    if(hSipEventHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgEventHeader*)hSipEventHeader;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipEventHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_EVENT_PACKAGE:
        requestedParamOffset = pHeader->strEventPackage;
        break;
    case RVSIP_EVENT_TEMPLATE:
        requestedParamOffset = pHeader->strEventTemplate;
        break;
    case RVSIP_EVENT_ID:
        requestedParamOffset = pHeader->strEventId;
        break;
    case RVSIP_EVENT_PARAM:
        requestedParamOffset = pHeader->strEventParams;
        break;
    case RVSIP_EVENT_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipEventHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipEventHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipEventHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Event header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipEventHeader - Handle to the Event header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetRpoolString(
                             IN    RvSipEventHeaderHandle      hSipEventHeader,
                             IN    RvSipEventHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                   *pRpoolPtr)
{
    MsgEventHeader* pHeader;

    pHeader = (MsgEventHeader*)hSipEventHeader;
    if(hSipEventHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipEventHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_EVENT_PACKAGE:
        return SipEventHeaderSetEventPackage(hSipEventHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_EVENT_TEMPLATE:
        return SipEventHeaderSetEventTemplate(hSipEventHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_EVENT_ID:
        return SipEventHeaderSetEventId(hSipEventHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_EVENT_PARAM:
        return SipEventHeaderSetEventParam(hSipEventHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipEventHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
}

/***************************************************************************
 * SipEventHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General: This method retrieves the event string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventPackage(IN  RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->strEventPackage;
}

/***************************************************************************
 * RvSipEventHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General: Copies the event-package string from the Event Header into
 *          a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetEventPackage(
                            IN RvSipEventHeaderHandle   hHeader,
                            IN RvChar*                 strBuffer,
                            IN RvUint                  bufferLen,
                            OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                                  ((MsgEventHeader*)hHeader)->hPage,
                                  SipEventHeaderGetEventPackage(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipEventHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventPackage in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventPackage       - Text string giving the event to be set in the object.
 *        strEventPackageOffset - Offset of the event-package string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventPackage(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventPackage,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventPackageOffset)
{
    MsgEventHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgEventHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strEventPackageOffset,
                                  strEventPackage, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strEventPackage = newStrOffset;
#ifdef SIP_DEBUG
    header->pEventPackage = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                         header->strEventPackage);
#endif

    return RV_OK;
}


/***************************************************************************
 * RvSipEventHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: Sets the event-package field in the Event Header object.
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Event Header object.
 *    strPackage - The event-package string to be set in the Event Header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetEventPackage(
                             IN  RvSipEventHeaderHandle hHeader,
                             IN  RvChar                *strPackage)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipEventHeaderSetEventPackage(hHeader, strPackage, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipEventHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General: This method retrieves the event template string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventTemplate(IN  RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->strEventTemplate;
}

/***************************************************************************
 * RvSipEventHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General: Copies the event-template string from the Event Header into
 *          a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetEventTemplate(
                            IN RvSipEventHeaderHandle   hHeader,
                            IN RvChar*                 strBuffer,
                            IN RvUint                  bufferLen,
                            OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                                  ((MsgEventHeader*)hHeader)->hPage,
                                  SipEventHeaderGetEventTemplate(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipEventHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventTemplate in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventTemplate       - Text string giving the event to be set in the object.
 *        strEventTemplateOffset - Offset of the event-template string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventTemplate(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventTemplate,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventTemplateOffset)
{
    MsgEventHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgEventHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strEventTemplateOffset,
                                   strEventTemplate, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strEventTemplate = newStrOffset;
#ifdef SIP_DEBUG
    header->pEventTemplate = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                             header->strEventTemplate);
#endif


    return RV_OK;
}

/***************************************************************************
 * RvSipEventHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: Sets the event-template field in the Event Header object.
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Event Header object.
 *    strTemplate - The event-template string to be set in the Event Header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetEventTemplate(
                             IN  RvSipEventHeaderHandle hHeader,
                             IN  RvChar                *strTemplate)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipEventHeaderSetEventTemplate(hHeader, strTemplate, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipEventHeaderGetEventParam
 * ------------------------------------------------------------------------
 * General: This method retrieves the event string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventParam(IN  RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->strEventParams;
}


/***************************************************************************
 * RvSipEventHeaderGetEventParam
 * ------------------------------------------------------------------------
 * General: Copies the event-param string from the Event Header into
 *          a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetEventParam(
                            IN RvSipEventHeaderHandle   hHeader,
                            IN RvChar*                 strBuffer,
                            IN RvUint                  bufferLen,
                            OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                                  ((MsgEventHeader*)hHeader)->hPage,
                                  SipEventHeaderGetEventParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipEventHeaderSetEventParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventParam in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventParam         - Text string giving the event to be set in the object.
 *        strEventParamOffset   - Offset of the event-param string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventParam(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventParam,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventParamOffset)
{
    MsgEventHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgEventHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strEventParamOffset,
                                  strEventParam, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strEventParams = newStrOffset;
#ifdef SIP_DEBUG
    header->pEventParams = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                         header->strEventParams);
#endif

    return RV_OK;
}


/***************************************************************************
 * RvSipEventHeaderSetEventParam
 * ------------------------------------------------------------------------
 * General: Sets the event-param field in the Event Header object.
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Event Header object.
 *    strEventParam - The event-param string to be set in the Event Header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetEventParam(
                             IN  RvSipEventHeaderHandle hHeader,
                             IN  RvChar                *strEventParam)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipEventHeaderSetEventParam(hHeader, strEventParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipEventHeaderGetEventId
 * ------------------------------------------------------------------------
 * General: This method retrieves the event id string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventId(IN  RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->strEventId;
}


/***************************************************************************
 * RvSipEventHeaderGetEventId
 * ------------------------------------------------------------------------
 * General: Copies the event-id string from the Event Header into
 *          a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetEventId(
                            IN RvSipEventHeaderHandle   hHeader,
                            IN RvChar                  *strBuffer,
                            IN RvUint                   bufferLen,
                            OUT RvUint                 *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                                  ((MsgEventHeader*)hHeader)->hPage,
                                  SipEventHeaderGetEventId(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipEventHeaderSetEventId
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventId in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle to the Event header object.
 *        strEventId       - Text string giving the event id to be set in the object.
 *        strEventIdOffset - Offset of the event-id string (if relevant).
 *        hPool            - The pool on which the string lays (if relevant).
 *        hPage            - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventId(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventId,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventIdOffset)
{
    MsgEventHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgEventHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strEventIdOffset,
                                   strEventId, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strEventId = newStrOffset;
#ifdef SIP_DEBUG
    header->pEventId = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage, header->strEventId);
#endif

    return RV_OK;
}


/***************************************************************************
 * RvSipEventHeaderSetEventId
 * ------------------------------------------------------------------------
 * General: Sets the event-id field in the Event Header object.
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Event Header object.
 *    strId      - The event-id string to be set in the Event Header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetEventId(
                             IN  RvSipEventHeaderHandle hHeader,
                             IN  RvChar                *strId)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipEventHeaderSetEventId(hHeader, strId, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
* SipEventHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipEventHeaderGetStrBadSyntax(IN  RvSipEventHeaderHandle hHeader)
{
    return ((MsgEventHeader*)hHeader)->strBadSyntax;
}


/***************************************************************************
 * RvSipEventHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Event header,
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
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetStrBadSyntax(
                            IN RvSipEventHeaderHandle   hHeader,
                            IN RvChar*                 strBuffer,
                            IN RvUint                  bufferLen,
                            OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                                  ((MsgEventHeader*)hHeader)->hPage,
                                  SipEventHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipEventHeaderSetStrBadSyntax
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
RvStatus SipEventHeaderSetStrBadSyntax(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgEventHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgEventHeader*)hHeader;

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
 * RvSipEventHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Event header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetStrBadSyntax(
                                  IN RvSipEventHeaderHandle  hHeader,
                                  IN RvChar                 *strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipEventHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipEventHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Event header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderSetCompactForm(
                                   IN    RvSipEventHeaderHandle hHeader,
                                   IN    RvBool                bIsCompact)
{
    MsgEventHeader* pHeader = (MsgEventHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipEventHeaderSetCompactForm - Setting compact form of hHeader 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}


/***************************************************************************
 * RvSipEventHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Event header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipEventHeaderGetCompactForm(
                                   IN    RvSipEventHeaderHandle hHeader,
                                   IN    RvBool                *pbIsCompact)
{
    MsgEventHeader* pHeader = (MsgEventHeader*)hHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    *pbIsCompact = pHeader->bIsCompact;
    return RV_OK;

}

/***************************************************************************
 * SipEventHeaderIsReferEventPackage
 * ------------------------------------------------------------------------
 * General: Is the header package is refer
 * Return Value: RV_TRUE of refer. RV_FALSE if not.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 ***************************************************************************/
RvBool RVCALLCONV SipEventHeaderIsReferEventPackage(
                                  IN RvSipEventHeaderHandle hHeader)
{
    RvChar packageBuff[7];
    RvUint32 len = 7;
    RvStatus rv;

    if(hHeader == NULL)
        return RV_FALSE;

    rv = MsgUtilsFillUserBuffer(((MsgEventHeader*)hHeader)->hPool,
                          ((MsgEventHeader*)hHeader)->hPage,
                          SipEventHeaderGetEventPackage(hHeader),
                          packageBuff,
                          len,
                          &len);
    if(rv != RV_OK)
    {
        return RV_FALSE;
    }

    return SipCommonStricmp("refer", packageBuff);
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * EventHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void EventHeaderClean(IN MsgEventHeader* pHeader,
							 IN RvBool          bCleanBS)
{
    pHeader->strEventId       = UNDEFINED;
    pHeader->strEventPackage  = UNDEFINED;
    pHeader->strEventParams   = UNDEFINED;
    pHeader->strEventTemplate = UNDEFINED;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_EVENT;

#ifdef SIP_DEBUG
    pHeader->pEventId         = NULL;
    pHeader->pEventPackage    = NULL;
    pHeader->pEventParams     = NULL;
    pHeader->pEventTemplate   = NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*RV_SIP_PRIMITIVES*/

