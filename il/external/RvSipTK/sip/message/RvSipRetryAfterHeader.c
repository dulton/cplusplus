/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipRetryAfterHeader.c                        *
 *                                                                            *
 * The file defines the methods of the Retry-After header object.             *
 * The Retry-After header functions enable you to construct, copy, encode,    *
 * parse, access and change Retry-After Header parameters.                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------------     ------------                                          *
 *     Ofra Wachsamn    December 2001                                         *
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipDateHeader.h"
#include "RvSipRetryAfterHeader.h"
#include "_SipRetryAfterHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "MsgInterval.h"
#include "RvSipMsg.h"
#include "RvSipMsgTypes.h"
#include <string.h>
#include <stdlib.h>


/*-----------------------------------------------------------------------*/
/*                        MODULE FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void RetryAfterHeaderClean( IN MsgRetryAfterHeader* pHeader,
                                    IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipRetryAfterHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Retry-After header object inside a given message object.
 *          The header is kept in the header list of the message. You can
 *          choose to insert the header either at the head or tail of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: phHeader - Handle to the newly constructed retry-after header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderConstructInMsg(
                                IN  RvSipMsgHandle            hSipMsg,
                                IN  RvBool                   pushHeaderAtHead,
                                OUT RvSipRetryAfterHeaderHandle* phHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipRetryAfterHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                           msg->hPool, msg->hPage, phHeader);
    if(stat != RV_OK)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*phHeader,
                              RVSIP_HEADERTYPE_RETRY_AFTER,
                              &hListElem,
                              (void**)phHeader);
}



/***************************************************************************
 * RvSipRetryAfterHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Retry-After Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool -   Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: phHeader - Handle to the newly constructed Retry-After header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderConstruct(
                                        IN  RvSipMsgMgrHandle         hMsgMgr,
                                        IN  HRPOOL                    hPool,
                                        IN  HPAGE                     hPage,
                                        OUT RvSipRetryAfterHeaderHandle* phHeader)
{
    MsgRetryAfterHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pHeader = (MsgRetryAfterHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgRetryAfterHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipRetryAfterHeaderConstruct - Failed to construct retry-After header. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    if(IntervalConstruct(pMsgMgr, hPool, hPage, &pHeader->pInterval)!= RV_OK)
        return RV_ERROR_OUTOFRESOURCES;

    pHeader->pMsgMgr             = pMsgMgr;
    pHeader->hPage               = hPage;
    pHeader->hPool               = hPool;
    RetryAfterHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipRetryAfterHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipRetryAfterHeaderConstruct - Retry-After header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *phHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipRetryAfterHeaderCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source retry-after header object to a
 *         destination retry-after header object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Retry-After header object.
 *    pSource      - Handle to the source Retry-After header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderCopy(
                                     INOUT RvSipRetryAfterHeaderHandle hDestination,
                                     IN    RvSipRetryAfterHeaderHandle hSource)
{
    MsgRetryAfterHeader*   source;
    MsgRetryAfterHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgRetryAfterHeader*)hSource;
    dest   = (MsgRetryAfterHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    if(IntervalCopy(dest->pInterval, source->pInterval)!=RV_OK)
        return RV_ERROR_UNKNOWN;

    dest->duration = source->duration;

    /* strRetryAfterParams */
    if(source->strRetryAfterParams > UNDEFINED)
    {
        dest->strRetryAfterParams = MsgUtilsAllocCopyRpoolString(
                                                            source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strRetryAfterParams);
       if (dest->strRetryAfterParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipRetryAfterHeaderCopy - Failed to copy strRetryAfterParams. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pRetryAfterParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                                   dest->strRetryAfterParams);
#endif
    }
    else
    {
        dest->strRetryAfterParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pRetryAfterParams = NULL;
#endif
    }

    /* strComment */
    if(source->strComment > UNDEFINED)
    {
        dest->strComment = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strComment);
       if (dest->strComment == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipRetryAfterHeaderCopy - Failed to copy strComment. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pComment = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strComment);
#endif
    }
    else
    {
        dest->strComment = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pComment = NULL;
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
                "RvSipRetryAfterHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipRetryAfterHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Retry-After header object to a textual Retry-After header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage().
 *          The allocated page must be freed only if this function returns
 *          RV_OK.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength or phPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader  - Handle to the Retry-After header object.
 *         hPool    - Handle to the specified memory pool.
 * output: phPage   - The memory page allocated to contain the encoded header.
 *         pLength  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderEncode(
                                      IN    RvSipRetryAfterHeaderHandle hHeader,
                                      IN    HRPOOL                   hPool,
                                      OUT   HPAGE*                   phPage,
                                      OUT   RvUint32*               pLength)
{
    RvStatus stat;
    MsgRetryAfterHeader* pHeader = (MsgRetryAfterHeader*)hHeader;

    if(phPage == NULL || pLength == NULL)
        return RV_ERROR_NULLPTR;

    *phPage = NULL_PAGE;
       *pLength = 0;

    if((pHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, phPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRetryAfterHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRetryAfterHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *phPage, hPool, hHeader));
    }

    *pLength = 0; /* This way length will give the real length, and will not
                     just add the new length to the old one */
    stat = RetryAfterHeaderEncode(hHeader, hPool, *phPage, RV_FALSE, pLength);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *phPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipRetryAfterHeaderEncode - Failed. Free page 0x%p on pool 0x%p. RetryAfterHeaderEncode fail",
                  *phPage, hPool));
    }
    return stat;
}


/***************************************************************************
 * RetryAfterHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Retry-After header (as string) on it.
 *          format: "Retry-After: "
 *                  (SIP-date|delta-seconds)[comment]*(;retry-param)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the retry after header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV RetryAfterHeaderEncode(
                                  IN    RvSipRetryAfterHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                bInUrlHeaders,
                                  INOUT RvUint32*               pLength)
{
    MsgRetryAfterHeader*  pHeader;
    RvStatus             stat;
    RvChar               strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    pHeader = (MsgRetryAfterHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "RetryAfterHeaderEncode - Encoding Retry-After header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
               hHeader, hPool, hPage));

    /* put "Retry-After" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Retry-After", pLength);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RetryAfterHeaderEncode - Failed to encoding Retry-After header. stat is %d",
                  stat));
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    pLength);


    /* bad - syntax */
    if(pHeader->strBadSyntax > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBadSyntax,
                                    pLength);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RetryAfterHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    if (RVSIP_EXPIRES_FORMAT_UNDEFINED == pHeader->pInterval->eFormat)
    {
        return RV_ERROR_BADPARAM;
    }

    if(IntervalEncode(pHeader->pInterval, hPool, hPage, RV_FALSE, bInUrlHeaders, pLength)!= RV_OK)
        return RV_ERROR_OUTOFRESOURCES;

    /* comment */
    if(pHeader->strComment > UNDEFINED)
    {
        /* "(" - comment need to be in () */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "(", pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RetryAfterHeaderEncode - Failed to encoding Retry-After header. stat is %d",
                      stat));
            return stat;
        }
        /* comment */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strComment,
                                    pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RetryAfterHeaderEncode - Failed to encoding Retry-After header. stat is %d",
                      stat));
            return stat;
        }
        /* ")" - comment need to be in () */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,")" , pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RetryAfterHeaderEncode - Failed to encoding Retry-After header. stat is %d",
                      stat));
            return stat;
        }
    }

    /* duration */
    if(pHeader->duration > UNDEFINED)
    {
        /* set ";duration=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "duration", pLength);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;

        MsgUtils_itoa(pHeader->duration, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
        if (stat != RV_OK)
            return stat;
    }

    /* ;retry-params */
    if(pHeader->strRetryAfterParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;

        /* set pHeader->strOtherParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strRetryAfterParams,
                                    pLength);
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipRetryAfterHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual expires header-for example,
 *         "RetryAfter: Thu, 01 Dec 2040 16:00:00 GMT"-into a RetryAfter header
 *          object. All the textual fields are placed inside the object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_INVALID_HANDLE,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   -    A handle to the Retry-After header object.
 *    strBuffer    - Buffer containing a textual Retry-After header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderParse(
                                     IN    RvSipRetryAfterHeaderHandle hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgRetryAfterHeader* pHeader = (MsgRetryAfterHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    RetryAfterHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RETRY_AFTER,
                                        strBuffer,
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
 * RvSipRetryAfterHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual RetryAfter header value into an RetryAfter
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value
 *          part as a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipRetryAfterHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the RetryAfter header object.
 *    buffer    - The buffer containing a textual RetryAfter header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderParseValue(
                                     IN    RvSipRetryAfterHeaderHandle hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgRetryAfterHeader* pHeader = (MsgRetryAfterHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    RetryAfterHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RETRY_AFTER,
                                        strBuffer,
                                        RV_TRUE,
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
 * RvSipRetryAfterHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an RetryAfter header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderFix(
                                     IN RvSipRetryAfterHeaderHandle hHeader,
                                     IN RvChar*                    pFixedBuffer)
{
    MsgRetryAfterHeader* pHeader = (MsgRetryAfterHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipRetryAfterHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRetryAfterHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRetryAfterHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipRetryAfterHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Retry-After object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipRetryAfterHeaderGetPool(RvSipRetryAfterHeaderHandle hHeader)
{
    return ((MsgRetryAfterHeader *)hHeader)->hPool;
}

/***************************************************************************
 * SipRetryAfterHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Retry-After object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipRetryAfterHeaderGetPage(RvSipRetryAfterHeaderHandle hHeader)
{
    return ((MsgRetryAfterHeader *)hHeader)->hPage;
}

/***************************************************************************
 * RvSipRetryAfterHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Retry-After header fields are kept in a string
 *          format - for example, the comment. In order to get
 *          such a field from the Retry-After header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Retry-After header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipRetryAfterHeaderGetStringLength(
                                 IN  RvSipRetryAfterHeaderHandle     hHeader,
                                 IN  RvSipRetryAfterHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgRetryAfterHeader* pHeader = (MsgRetryAfterHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_RETRY_AFTER_COMMENT:
        {
            stringOffset = SipRetryAfterHeaderGetStrComment(hHeader);
            break;
        }
        case RVSIP_RETRY_AFTER_PARAMS:
        {
            stringOffset = SipRetryAfterHeaderGetRetryAfterParams(hHeader);
            break;
        }
        case RVSIP_RETRY_AFTER_BAD_SYNTAX:
        {
            stringOffset = SipRetryAfterHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRetryAfterHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipRetryAfterHeaderGetPool(hHeader),
                             SipRetryAfterHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}
/***************************************************************************
 * RvSipRetryAfterHeaderGetFormat
 * ------------------------------------------------------------------------
 * General: Gets the format of the Retry-After header: Undefined, Date or
 *          Delta-seconds.
 * Return Value: The format enumeration.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 ***************************************************************************/
RVAPI RvSipExpiresFormat RVCALLCONV RvSipRetryAfterHeaderGetFormat(
                                    IN  RvSipRetryAfterHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_EXPIRES_FORMAT_UNDEFINED;
    }
    return ((MsgRetryAfterHeader*)hHeader)->pInterval->eFormat;
}


/***************************************************************************
 * RvSipRetryAfterHeaderGetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Gets the delta-seconds integer of the Retry-After header.
 *          If the delta-seconds integer is not set, UNDEFINED is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - pDeltaSeconds is NULL.
 *               RV_ERROR_NOT_FOUND - delta-seconds were not defined for this
 *                             expires object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Retry-After header object.
 *  Output: pDeltaSeconds - The delta-seconds integer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderGetDeltaSeconds(
                                  IN  RvSipRetryAfterHeaderHandle hHeader,
                                  OUT RvUint32                   *pDeltaSeconds)
{
    MsgRetryAfterHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDeltaSeconds)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgRetryAfterHeader *)hHeader;
    if (RVSIP_EXPIRES_FORMAT_DELTA_SECONDS != pHeader->pInterval->eFormat)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *pDeltaSeconds = pHeader->pInterval->deltaSeconds;
    return RV_OK;
}

/***************************************************************************
 * RvSipRetryAfterHeaderSetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Sets the delta seconds integer of the Retry-After header. Changes
 *          the Retry-After format to delta-seconds. If the given delta-seconds
 *          is UNDEFINED, the delta-seconds of the Retry-After header is removed
 *          and the format is changed to UNDEFINED.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Retry-After header handle is invalid.
 *               RV_Invalid parameter - The delta-seconds integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 *         deltaSeconds - The delta-seconds to be set to the Retry-After header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetDeltaSeconds(
                                   IN  RvSipRetryAfterHeaderHandle hHeader,
                                   IN  RvInt32                deltaSeconds)
{
    MsgRetryAfterHeader *pRetryAfter;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRetryAfter = (MsgRetryAfterHeader *)hHeader;
    if (UNDEFINED == deltaSeconds)
    {
        pRetryAfter->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_UNDEFINED;
    }
    else
    {
        pRetryAfter->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
    }
    pRetryAfter->pInterval->deltaSeconds = deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * RvSipRetryAfterHeaderGetDateHandle
 * ------------------------------------------------------------------------
 * General: Gets the date Handle to the Retry-After header.
 * Return Value: Returns the handle to the date header object, or NULL
 *               if the date header object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 ***************************************************************************/
RVAPI RvSipDateHeaderHandle RVCALLCONV RvSipRetryAfterHeaderGetDateHandle(
                                        IN  RvSipRetryAfterHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return NULL;
    }
    if (RVSIP_EXPIRES_FORMAT_DATE != ((MsgRetryAfterHeader *)hHeader)->pInterval->eFormat)
    {
        return NULL;
    }
    return ((MsgRetryAfterHeader *)hHeader)->pInterval->hDate;
}


/***************************************************************************
 * RvSipRetryAfterHeaderSetDate
 * ------------------------------------------------------------------------
 * General: Sets a new Date header in the Retry-After header object and changes
 *          the Retry-After format to date. (The function allocates a date header,
 *          and copy the given hDate object to it).
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Retry-After header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 *         hDate - The date handle to be set to the Retry-After header.
 *                 If the date handle is NULL, the existing date header
 *                 is removed from the expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetDate(
                                       IN  RvSipRetryAfterHeaderHandle hHeader,
                                       IN  RvSipDateHeaderHandle    hDate)
{
    MsgRetryAfterHeader     *pRetryAfter;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRetryAfter = (MsgRetryAfterHeader *)hHeader;

    return IntervalSetDate(pRetryAfter->pInterval, hDate);
}

/***************************************************************************
 * RvSipRetryAfterHeaderGetDuration
 * ------------------------------------------------------------------------
 * General: Gets the duration integer of the Retry-After header.
 *          If the duration integer is not set, UNDEFINED is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - pDuration is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Retry-After header object.
 *  Output: pDuration - The duration integer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderGetDuration(
                                  IN  RvSipRetryAfterHeaderHandle hHeader,
                                  OUT RvInt32                   *pDuration)
{
    MsgRetryAfterHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDuration)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgRetryAfterHeader *)hHeader;

    *pDuration = pHeader->duration;
    return RV_OK;
}

/***************************************************************************
 * RvSipRetryAfterHeaderSetDuration
 * ------------------------------------------------------------------------
 * General: Sets the duration integer of the Retry-After header. If the given duration
 *          is UNDEFINED, the duration of the Retry-After header is removed.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Retry-After header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 *         duration - The duration to be set to the Retry-After header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetDuration(
                                   IN  RvSipRetryAfterHeaderHandle hHeader,
                                   IN  RvInt32                    duration)
{
    MsgRetryAfterHeader *pRetryAfter;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pRetryAfter = (MsgRetryAfterHeader *)hHeader;

    pRetryAfter->duration = duration;
    return RV_OK;
}

/***************************************************************************
 * SipRetryAfterHeaderGetStrComment
 * ------------------------------------------------------------------------
 * General: This method retrieves the strComment.
 * Return Value: strComment offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RetryAfter header object
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetStrComment(IN RvSipRetryAfterHeaderHandle hHeader)
{
    return ((MsgRetryAfterHeader*)hHeader)->strComment;
}

/***************************************************************************
 * RvSipRetryAfterHeaderGetstrComment
 * ------------------------------------------------------------------------
 * General: Copies the strComment field of the RetryAfter header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the RetryAfter header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value
 *                     at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderGetStrComment(
                                               IN RvSipRetryAfterHeaderHandle   hHeader,
                                               IN RvChar*                      strBuffer,
                                               IN RvUint                       bufferLen,
                                               OUT RvUint*                     actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgRetryAfterHeader*)hHeader)->hPool,
                                  ((MsgRetryAfterHeader*)hHeader)->hPage,
                                  SipRetryAfterHeaderGetStrComment(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRetryAfterHeaderSetStrComment
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strComment in the
 *          RetryAfterHeader object. The API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader             - Handle of the RetryAfter header object
 *          strComment          - The strComment string to be set - if Null,
 *                                the exist strComment in the object will be removed.
 *          hPool               - The pool on which the string lays (if relevant).
 *          hPage               - The page on which the string lays (if relevant).
 *          strCommentOffset - the offset of the strComment string.
 ***************************************************************************/
RvStatus SipRetryAfterHeaderSetStrComment(
                                         IN    RvSipRetryAfterHeaderHandle hHeader,
                                         IN    RvChar *              strComment,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strCommentOffset)
{
    RvInt32        newStr;
    MsgRetryAfterHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgRetryAfterHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strCommentOffset, strComment,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strComment = newStr;
#ifdef SIP_DEBUG
    pHeader->pComment = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage, pHeader->strComment);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipRetryAfterHeaderSetStrComment
 * ------------------------------------------------------------------------
 * General: Sets the strComment field in the RetryAfter header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the RetryAfter header object.
 *    strComment - The strComment string to be set in the RetryAfter header.
 *               If NULL is supplied, the existing strComment field
 *               is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetStrComment(
                                    IN    RvSipRetryAfterHeaderHandle hHeader,
                                    IN    RvChar *                   strComment)
{
    /* validity checking will be done in the internal function */
    return SipRetryAfterHeaderSetStrComment(  hHeader,
                                              strComment,
                                              NULL,
                                              NULL_PAGE,
                                              UNDEFINED);
}

/***************************************************************************
 * SipRetryAfterHeaderGetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the RetryAfterParams.
 * Return Value: RetryAfterParams offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RetryAfter header object
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetRetryAfterParams(IN RvSipRetryAfterHeaderHandle hHeader)
{
    return ((MsgRetryAfterHeader*)hHeader)->strRetryAfterParams;
}

/***************************************************************************
 * RvSipRetryAfterHeaderGetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: Copies the RetryAfterParams field of the RetryAfter header object into a
 *          given buffer.
 *          Not all the RetryAfter header parameters have separated fields in the RetryAfter
 *          header object. Parameters with no specific fields are refered to as RetryAfterParams.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the RetryAfter header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderGetRetryAfterParams(
                                               IN RvSipRetryAfterHeaderHandle   hHeader,
                                               IN RvChar*                      strBuffer,
                                               IN RvUint                       bufferLen,
                                               OUT RvUint*                     actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgRetryAfterHeader*)hHeader)->hPool,
                                  ((MsgRetryAfterHeader*)hHeader)->hPage,
                                  SipRetryAfterHeaderGetRetryAfterParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRetryAfterHeaderSetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strRetryAfterParams in the
 *          RetryAfterHeader object. The API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader             - Handle of the RetryAfter header object
 *          strRetryAfterParams - The RetryAfterParams string to be set - if Null,
 *                                the exist strRetryAfterParams in the object will be removed.
 *          hPool               - The pool on which the string lays (if relevant).
 *          hPage               - The page on which the string lays (if relevant).
 *          strRetryAfterOffset - the offset of the RetryAfter string.
 ***************************************************************************/
RvStatus SipRetryAfterHeaderSetRetryAfterParams(
                                         IN    RvSipRetryAfterHeaderHandle hHeader,
                                         IN    RvChar *              strRetryAfterParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strRetryAfterOffset)
{
    RvInt32        newStr;
    MsgRetryAfterHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgRetryAfterHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strRetryAfterOffset, strRetryAfterParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strRetryAfterParams = newStr;
#ifdef SIP_DEBUG
    pHeader->pRetryAfterParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                              pHeader->strRetryAfterParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipRetryAfterHeaderSetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: Sets the RetryAfterParams field in the RetryAfter header object.
 *          Not all the RetryAfter header parameters have separated fields in the
 *          RetryAfter header object. Parameters with no specific fields are
 *          refered to as RetryAfterParams. They are kept in the object in one
 *          concatenated string in the form- "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader             - Handle to the RetryAfter header object.
 *    strRetryAfterParams - The RetryAfter Params string to be set in the RetryAfter header.
 *                        If NULL is supplied, the existing RetryAfter Params field
 *                        is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetRetryAfterParams(
                                    IN    RvSipRetryAfterHeaderHandle hHeader,
                                    IN    RvChar *                   strRetryAfterParams)
{
    /* validity checking will be done in the internal function */
    return SipRetryAfterHeaderSetRetryAfterParams(hHeader,
                                                  strRetryAfterParams,
                                                  NULL,
                                                  NULL_PAGE,
                                                  UNDEFINED);
}

/***************************************************************************
 * SipRetryAfterHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetStrBadSyntax(
                                    IN RvSipRetryAfterHeaderHandle hHeader)
{
    return ((MsgRetryAfterHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipRetryAfterHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad RetryAfter header,
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
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderGetStrBadSyntax(
                                               IN RvSipRetryAfterHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgRetryAfterHeader*)hHeader)->hPool,
                                  ((MsgRetryAfterHeader*)hHeader)->hPage,
                                  SipRetryAfterHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRetryAfterHeaderSetStrBadSyntax
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
RvStatus SipRetryAfterHeaderSetStrBadSyntax(
                                  IN RvSipRetryAfterHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgRetryAfterHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgRetryAfterHeader*)hHeader;

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
 * RvSipRetryAfterHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal RetryAfter header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRetryAfterHeaderSetStrBadSyntax(
                                  IN RvSipRetryAfterHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipRetryAfterHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RetryAfterHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void RetryAfterHeaderClean(IN MsgRetryAfterHeader* pHeader,
                                  IN RvBool               bCleanBS)
{
    pHeader->duration            = -1;
    pHeader->strRetryAfterParams = UNDEFINED;
    pHeader->strComment          = UNDEFINED;
    pHeader->eHeaderType         = RVSIP_HEADERTYPE_RETRY_AFTER;

#ifdef SIP_DEBUG
    pHeader->pComment            = NULL;
    pHeader->pRetryAfterParams   = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax    = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax      = NULL;
#endif
    }
}
#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif

