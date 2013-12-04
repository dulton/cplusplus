/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                      RvSipAllowEventsHeader.c                               *
 *                                                                            *
 * The file defines the methods of the Allow-events header object:             *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             May 2002                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES

#include "RvSipAllowEventsHeader.h"
#include "_SipAllowEventsHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AllowEventsHeaderClean( IN MsgAllowEventsHeader* pHeader,
								    IN RvBool                bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAllowEventsHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Allow Header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of the header list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be
 *                            pushed to the head of the list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Allow-events header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderConstructInMsg(
                                    IN  RvSipMsgHandle                hSipMsg,
                                    IN  RvBool                        pushHeaderAtHead,
                                    OUT RvSipAllowEventsHeaderHandle *hHeader)
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

    stat = RvSipAllowEventsHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);

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
                              RVSIP_HEADERTYPE_ALLOW_EVENTS,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipAllowEventsHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Allow-events header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle of the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Allow-events header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderConstruct(
                                           IN  RvSipMsgMgrHandle             hMsgMgr,
                                           IN  HRPOOL                        hPool,
                                           IN  HPAGE                         hPage,
                                           OUT RvSipAllowEventsHeaderHandle *hHeader)
{
    MsgAllowEventsHeader     *pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hPage == NULL_PAGE) || (hPool == NULL) || (hMsgMgr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAllowEventsHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgAllowEventsHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAllowEventsHeaderConstruct - Fail. out of resources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;

    AllowEventsHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAllowEventsHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAllowEventsHeaderConstruct - Allow-Events header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipAllowEventsHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Allow-events header object to a destination Allow
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Allow-events header object.
 *    hSource      - Handle to the source Allow-events header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderCopy(
                                        INOUT RvSipAllowEventsHeaderHandle hDestination,
                                        IN    RvSipAllowEventsHeaderHandle hSource)
{
    MsgAllowEventsHeader *source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAllowEventsHeader*)hSource;
    dest   = (MsgAllowEventsHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->bIsCompact  = source->bIsCompact;

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
                "RvSipAllowEventsHeaderCopy - failed in coping strEventPackage."));
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
                "RvSipAllowEventsHeaderCopy - failed in coping strEventTemplate."));
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
                "RvSipAllowEventsHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipAllowEventsHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Allow-events header object to a textual Allow-events header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Allow-events header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderEncode(
                                          IN    RvSipAllowEventsHeaderHandle hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgMgr* pMsgMgr;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pMsgMgr = ((MsgAllowEventsHeader*)hHeader)->pMsgMgr;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowEventsHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowEventsHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AllowEventsHeaderEncode(hHeader, hPool, *hPage, RV_TRUE, RV_FALSE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowEventsHeaderEncode - Failed. Free page 0x%p on pool 0x%p. AllowEventsHeaderEncode failed",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AllowEventsHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Allow-events header (as string) on it.
 *          format: "Allow-Events: " event-package.event-template
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Allow-events header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstInMessage - RV_TRUE if this parameter is the first Allow_Events header
 *                    between the Allow-Events headers in the headers list,
 *                    RV_FALSE otherwise.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AllowEventsHeaderEncode(
                                  IN    RvSipAllowEventsHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bFirstInMessage,
                                  IN    RvBool                       bInUrlHeaders,
                                  IN    RvBool                       bForceCompactForm,
                                  INOUT RvUint32                    *length)
{
    MsgAllowEventsHeader* pHeader;
    RvStatus             stat = RV_OK;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgAllowEventsHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AllowEventsHeaderEncode - Encoding Allow-Events header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    if(bFirstInMessage == RV_TRUE)
    {
        /* encode "Allow-Events: " */
        if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "u", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Allow-Events", length);
        }
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                   MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders), length);

    }
    else
    {
        /* encode "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, ",", length);
    }

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
                "AllowEventsHeaderEncode - Failed to encode bad-syntax string in Allow-events header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    if(pHeader->strEventPackage == UNDEFINED)
    {
        /*mandatory parameter was not set - return illegal action*/
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AllowEventsHeaderEncode - illegal action, eventPackage was not set (mandatory parameter)."));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* event-package */
    stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                pHeader->hPool, pHeader->hPage,
                                pHeader->strEventPackage, length);

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AllowEventsHeaderEncode - Failed to encode eventPackage in Allow-events header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }

    /* event-template */
    if(pHeader->strEventTemplate > UNDEFINED)
    {
        /* encode "." */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,".", length);

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, hPage,
                                    pHeader->hPool, pHeader->hPage,
                                    pHeader->strEventTemplate, length);

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AllowEventsHeaderEncode - Failed to encode eventTemplate in Allow-events header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipAllowEventsHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Allow-events header into an Allow-events header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Allow-events header object.
 *    buffer    - Buffer containing a textual Allow-events header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderParse(
                                     IN RvSipAllowEventsHeaderHandle hHeader,
                                     IN RvChar*                      buffer)
{
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    AllowEventsHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                         SIP_PARSETYPE_ALLOW_EVENTS,
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
 * RvSipAllowEventsHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AllowEvents header value into an AllowEvents
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAllowEventsHeaderParse() function to parse strings
 *          that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the AllowEvents header object.
 *    buffer    - The buffer containing a textual AllowEvents header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderParseValue(
                                     IN RvSipAllowEventsHeaderHandle hHeader,
                                     IN RvChar                      *buffer)
{
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;
	RvBool                bIsCompact;
    RvStatus              rv;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

	/* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;

    AllowEventsHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ALLOW_EVENTS,
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
 * RvSipAllowEventsHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AllowEvents header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderFix(
                                     IN RvSipAllowEventsHeaderHandle hHeader,
                                     IN RvChar*                      pFixedBuffer)
{
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAllowEventsHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAllowEventsHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAllowEventsHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;

}

/***************************************************************************
 * SipAllowEventsHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Allow object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAllowEventsHeaderGetPool(RvSipAllowEventsHeaderHandle hHeader)
{
    return ((MsgAllowEventsHeader*)hHeader)->hPool;

}

/***************************************************************************
 * SipAllowEventsHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Allow object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAllowEventsHeaderGetPage(RvSipAllowEventsHeaderHandle hHeader)
{
    return ((MsgAllowEventsHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAllowEventsHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Allow-events header fields are kept in a string format.
 *          In order to get such a field from the Allow-events header object, your
 *          application should supply an adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to
 *          allocator an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Allow-events header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAllowEventsHeaderGetStringLength(
                                      IN  RvSipAllowEventsHeaderHandle     hHeader,
                                      IN  RvSipAllowEventsHeaderStringName stringName)
{
    RvInt32        stringOffset;
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_ALLOW_EVENTS_EVENT_PACKAGE:
        {
            stringOffset = SipAllowEventsHeaderGetEventPackage(hHeader);
            break;
        }
        case RVSIP_ALLOW_EVENTS_EVENT_TEMPLATE:
        {
            stringOffset = SipAllowEventsHeaderGetEventTemplate(hHeader);
            break;
        }
        case RVSIP_ALLOW_EVENTS_BAD_SYNTAX:
        {
            stringOffset = SipAllowEventsHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAllowEventsHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * RvSipAllowEventsHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Allow-Events header into
 *          a given page from a specified pool. The copied string is not
 *          consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAllowEventsHeader - Handle to the Allow-Events header object.
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
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderGetRpoolString(
                             IN    RvSipAllowEventsHeaderHandle      hSipAllowEventsHeader,
                             IN    RvSipAllowEventsHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                         *pRpoolPtr)
{
    MsgAllowEventsHeader* pHeader;
    RvInt32              requestedParamOffset;

    if(hSipAllowEventsHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgAllowEventsHeader*)hSipAllowEventsHeader;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAllowEventsHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_ALLOW_EVENTS_EVENT_PACKAGE:
        requestedParamOffset = pHeader->strEventPackage;
        break;
    case RVSIP_ALLOW_EVENTS_EVENT_TEMPLATE:
        requestedParamOffset = pHeader->strEventTemplate;
        break;
    case RVSIP_ALLOW_EVENTS_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAllowEventsHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipAllowEventsHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipAllowEventsHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Allow-Events
 *          header object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAllowEventsHeader - Handle to the Allow-Events header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderSetRpoolString(
                             IN    RvSipAllowEventsHeaderHandle      hSipAllowEventsHeader,
                             IN    RvSipAllowEventsHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                         *pRpoolPtr)
{
    MsgAllowEventsHeader* pHeader;

    pHeader = (MsgAllowEventsHeader*)hSipAllowEventsHeader;
    if(hSipAllowEventsHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipAllowEventsHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_ALLOW_EVENTS_EVENT_PACKAGE:
        return SipAllowEventsHeaderSetEventPackage(hSipAllowEventsHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_ALLOW_EVENTS_EVENT_TEMPLATE:
        return SipAllowEventsHeaderSetEventTemplate(hSipAllowEventsHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAllowEventsHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}


/***************************************************************************
 * SipAllowEventsHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General: This method retrieves the event-package string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetEventPackage(IN  RvSipAllowEventsHeaderHandle hHeader)
{
    return ((MsgAllowEventsHeader*)hHeader)->strEventPackage;

}

/***************************************************************************
 * RvSipAllowEventsHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General:Copies the event-package string from the Allow-events header object into a
 *         given buffer. If the bufferLen is adequate, the function copies
 *         the requested parameter into strBuffer. Otherwise, the function
 *         returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *         buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Allow-events header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderGetEventPackage(
                                IN  RvSipAllowEventsHeaderHandle hHeader,
                                IN  RvChar                      *strBuffer,
                                IN  RvUint                       bufferLen,
                                OUT RvUint                      *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAllowEventsHeader*)hHeader)->hPool,
                                  ((MsgAllowEventsHeader*)hHeader)->hPage,
                                  SipAllowEventsHeaderGetEventPackage(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);

}


/***************************************************************************
 * SipAllowEventsHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the event-package string in the
 *          AllowEvents Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strEventPackage- Text string giving the event-package to be set in the object.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strEventPackageOffset - Offset of the event-package string (if relevant).
 ***************************************************************************/
RvStatus SipAllowEventsHeaderSetEventPackage(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar*                      strEventPackage,
                                  IN HRPOOL                       hPool,
                                  IN HPAGE                        hPage,
                                  IN RvInt32                      strEventPackageOffset)
{
    MsgAllowEventsHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAllowEventsHeader*)hHeader;

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
 * RvSipAllowEventsHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: Sets the Event-package in the Allow-events header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Allow-events header object.
 *  strEvent - The event-package string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderSetEventPackage(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar                      *strEventPackage)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAllowEventsHeaderSetEventPackage(hHeader, strEventPackage, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * SipAllowEventsHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General: This method retrieves the event-template string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetEventTemplate(IN  RvSipAllowEventsHeaderHandle hHeader)
{
    return ((MsgAllowEventsHeader*)hHeader)->strEventTemplate;

}

/***************************************************************************
 * RvSipAllowEventsHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General:Copies the event-template string from the Allow-events header object into a
 *         given buffer. If the bufferLen is adequate, the function copies
 *         the requested parameter into strBuffer. Otherwise, the function
 *         returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *         buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Allow-events header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderGetEventTemplate(
                                        IN  RvSipAllowEventsHeaderHandle hHeader,
                                        IN  RvChar                      *strBuffer,
                                        IN  RvUint                       bufferLen,
                                        OUT RvUint                      *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAllowEventsHeader*)hHeader)->hPool,
                                  ((MsgAllowEventsHeader*)hHeader)->hPage,
                                  SipAllowEventsHeaderGetEventTemplate(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);

}


/***************************************************************************
 * SipAllowEventsHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the event-template string in the
 *          AllowEvents Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strEventTemplate - Text string giving the event-template to be set in the object.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strEventTemplateOffset - Offset of the event-template string (if relevant).
 ***************************************************************************/
RvStatus SipAllowEventsHeaderSetEventTemplate(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar                      *strEventTemplate,
                                  IN HRPOOL                       hPool,
                                  IN HPAGE                        hPage,
                                  IN RvInt32                      strEventTemplateOffset)
{
    MsgAllowEventsHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAllowEventsHeader*)hHeader;

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
 * RvSipAllowEventsHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: Sets the Event-template in the Allow-events header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Allow-events header object.
 *  strEvent - The event-template string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderSetEventTemplate(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar*                     strEventTemplate)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAllowEventsHeaderSetEventTemplate(hHeader, strEventTemplate, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * SipAllowEventsHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetStrBadSyntax(IN  RvSipAllowEventsHeaderHandle hHeader)
{
    return ((MsgAllowEventsHeader*)hHeader)->strBadSyntax;

}

/***************************************************************************
 * RvSipAllowEventsHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AllowEvents header,
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
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderGetStrBadSyntax(
                                         IN  RvSipAllowEventsHeaderHandle hHeader,
                                         IN  RvChar*                      strBuffer,
                                         IN  RvUint                       bufferLen,
                                         OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAllowEventsHeader*)hHeader)->hPool,
                                  ((MsgAllowEventsHeader*)hHeader)->hPage,
                                  SipAllowEventsHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);

}

/***************************************************************************
 * SipAllowEventsHeaderSetStrBadSyntax
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
RvStatus SipAllowEventsHeaderSetStrBadSyntax(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar                      *strBadSyntax,
                                  IN HRPOOL                       hPool,
                                  IN HPAGE                        hPage,
                                  IN RvInt32                      strBadSyntaxOffset)
{
    MsgAllowEventsHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAllowEventsHeader*)hHeader;

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
 * RvSipAllowEventsHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal AllowEvents header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderSetStrBadSyntax(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar                      *strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAllowEventsHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}


/***************************************************************************
 * RvSipAllowEventsHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Allow-Events header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderSetCompactForm(
                                   IN    RvSipAllowEventsHeaderHandle hHeader,
                                   IN    RvBool                      bIsCompact)
{
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipAllowEventsHeaderSetCompactForm - Setting compact form of hHeader 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}

/***************************************************************************
 * RvSipAllowEventsHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Allow-Events header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowEventsHeaderGetCompactForm(
                                   IN    RvSipAllowEventsHeaderHandle hHeader,
                                   IN    RvBool                      *pbIsCompact)
{
    MsgAllowEventsHeader* pHeader = (MsgAllowEventsHeader*)hHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *pbIsCompact = pHeader->bIsCompact;
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AllowEventsHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AllowEventsHeaderClean( IN MsgAllowEventsHeader* pHeader,
								    IN RvBool                bCleanBS)
{
    pHeader->strEventPackage  = UNDEFINED;
    pHeader->strEventTemplate = UNDEFINED;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_ALLOW_EVENTS;

#ifdef SIP_DEBUG
    pHeader->pEventPackage    = NULL;
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


#endif /* RV_SIP_PRIMITIVES */
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef __cplusplus
}
#endif

