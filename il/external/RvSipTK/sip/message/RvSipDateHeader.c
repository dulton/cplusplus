/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipDateHeader.c                              *
 *                                                                            *
 * The file defines the functions  of the Date header object:                 *
 * The Date header functions enable you to construct, destruct, copy, encode, *
 * parse, access and change Date Header parameters.                           *                        *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Jan 2001                                              *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipDateHeader.h"
#include "RvSipExpiresHeader.h"
#include "_SipDateHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipMsgTypes.h"
#include <string.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus DateEncode(
                            IN    RvSipDateHeaderHandle    hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                 bInUrlHeaders,
                            INOUT RvUint32*               pLength);

/*-----------------------------------------------------------------------*/
/*                        MODULE FUNCTIONS                               */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void DateHeaderClean( IN MsgDateHeader* pHeader,
                             IN RvBool        bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RvSipDateHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Date header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail
 *          of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: phHeader - Handle of the newly constructed date header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderConstructInMsg(
                                IN  RvSipMsgHandle            hSipMsg,
                                IN  RvBool                   pushHeaderAtHead,
                                OUT RvSipDateHeaderHandle*    phHeader)
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

    stat = RvSipDateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, phHeader);
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
                              RVSIP_HEADERTYPE_DATE,
                              &hListElem,
                              (void**)phHeader);
}


/***************************************************************************
 * RvSipDateConstructInExpiresHeader
 * ------------------------------------------------------------------------
 * General: Constructs a Date header object in a given Expires header.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader - Handle to the Expires header that relates to this date.
 * output: phDate - Handle to the newly constructed date object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateConstructInExpiresHeader(
                                       IN  RvSipExpiresHeaderHandle  hHeader,
                                       OUT RvSipDateHeaderHandle    *phDate)
{
    MsgExpiresHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phDate == NULL)
        return RV_ERROR_NULLPTR;

    *phDate = NULL;

    pHeader = (MsgExpiresHeader*)hHeader;


    /* creating the new date object */
    stat = RvSipDateHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                                    pHeader->hPool, pHeader->hPage, phDate);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new date parameter to the Expires header */
    pHeader->pInterval->hDate = *phDate;
    pHeader->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DATE;

    return RV_OK;
}

/***************************************************************************
 * SipDateConstructInInterval
 * ------------------------------------------------------------------------
 * General: Constructs a Date header object in a given Interval object.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pInterval - Pointer to the Interval object that relates to this date.
 * output: phDate    - Handle to the newly constructed date object.
 ***************************************************************************/
RvStatus RVCALLCONV SipDateConstructInInterval(
                                       IN  MsgInterval*           pInterval,
                                       OUT RvSipDateHeaderHandle *phDate)
{
    RvStatus  stat;

    if (phDate == NULL || pInterval == NULL)
        return RV_ERROR_NULLPTR;

    *phDate = NULL;

    /* creating the new date object */
    stat = RvSipDateHeaderConstruct((RvSipMsgMgrHandle)pInterval->pMsgMgr,
                                    pInterval->hPool, pInterval->hPage, phDate);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new date parameter to the Expires header */
    pInterval->hDate = *phDate;
    pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DATE;

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipDateConstructInRetryAfterHeader
 * ------------------------------------------------------------------------
 * General: Constructs a Date header object in a given Retry-After header.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader - Handle to the RetryAfter header that relates to this date.
 * output: phDate - Handle to the newly constructed date object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateConstructInRetryAfterHeader(
                                       IN  RvSipRetryAfterHeaderHandle  hHeader,
                                       OUT RvSipDateHeaderHandle        *phDate)
{
    MsgRetryAfterHeader*   pHeader;
    RvStatus              stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phDate == NULL)
        return RV_ERROR_NULLPTR;

    *phDate = NULL;

    pHeader = (MsgRetryAfterHeader*)hHeader;

    /* creating the new date object */
    stat = RvSipDateHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                                     pHeader->hPool, pHeader->hPage, phDate);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new date parameter to the Expires header */
    pHeader->pInterval->hDate = *phDate;
    pHeader->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DATE;

    return RV_OK;
}
#endif /* #ifndef RV_SIP_PRIMITIVES */
/***************************************************************************
 * RvSipDateHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Date Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use
 *         hPage    - Handle of the memory page that the object will use
 * output: phHeader - Handle to the newly constructed date object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderConstruct(
                                        IN  RvSipMsgMgrHandle         hMsgMgr,
                                        IN  HRPOOL                    hPool,
                                        IN  HPAGE                     hPage,
                                        OUT RvSipDateHeaderHandle*    phHeader)
{
    MsgDateHeader*   pHeader;
    MsgMgr*          pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pHeader = (MsgDateHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgDateHeader),
                                                    RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipDateHeaderConstruct - Failed to construct date header. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    DateHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipDateHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipDateHeaderConstruct - Date header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              pMsgMgr, hPool, hPage, *phHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Date header object to a destination
 *         Date header object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Date header object.
 *    pSource      - Handle to the source Date header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderCopy(
                                     INOUT RvSipDateHeaderHandle hDestination,
                                     IN    RvSipDateHeaderHandle hSource)
{
    MsgDateHeader*   source;
    MsgDateHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgDateHeader*)hSource;
    dest   = (MsgDateHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->day      = source->day;
    dest->eMonth   = source->eMonth;
    dest->eWeekDay = source->eWeekDay;
    dest->hour     = source->hour;
    dest->minute   = source->minute;
    dest->second   = source->second;
    dest->year     = source->year;
    dest->zone     = source->zone;

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
                "RvSipDateHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipDateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Date header object to a textual Date header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
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
 * input: hHeader  - Handle of the Date header object.
 *        hPool    - Handle to the specified memory pool.
 * output: phPage   - The memory page allocated to contain the encoded header.
 *         pLength  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderEncode(
                                      IN    RvSipDateHeaderHandle    hHeader,
                                      IN    HRPOOL                   hPool,
                                      OUT   HPAGE*                   phPage,
                                      OUT   RvUint32*               pLength)
{
    RvStatus stat;
    MsgDateHeader* pHeader;

    pHeader = (MsgDateHeader*)hHeader;
    if(phPage == NULL || pLength == NULL)
        return RV_ERROR_NULLPTR;

    *phPage = NULL_PAGE;
       *pLength = 0;

    if((hHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, phPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipDateHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipDateHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *phPage, hPool, hHeader));
    }

    *pLength = 0; /* This way length will give the real length, and will not
                     just add the new length to the old one */
    stat = DateHeaderEncode(hHeader, hPool, *phPage, RV_FALSE, RV_FALSE, pLength);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *phPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipDateHeaderEncode - Failed. Free page 0x%p on pool 0x%p. DateHeaderEncode fail",
                  *phPage, hPool));
    }
    return stat;
}


/***************************************************************************
 * DateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Date header (as string) on it.
 *          format: "Date: " SIP-date
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the date header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bIsExpires - True if the date is a part of and Expires header.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderEncode (
                             IN    RvSipDateHeaderHandle    hHeader,
                             IN    HRPOOL                   hPool,
                             IN    HPAGE                    hPage,
                             IN    RvBool                  bIsExpires,
                             IN    RvBool                 bInUrlHeaders,
                             INOUT RvUint32*               pLength)
{
    MsgDateHeader*     pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    pHeader = (MsgDateHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "DateHeaderEncode - Encoding Date header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
               hHeader, hPool, hPage));

    if (RV_FALSE == bIsExpires)
    {
        /* put "Date" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Date", pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "DateHeaderEncode - Failed to encode date header. stat is %d",
                      stat));
            return stat;
        }
        /* encode ": " or "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        pLength);
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
                                    pLength);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "DateHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    stat = DateEncode(hHeader, hPool, hPage, bInUrlHeaders, pLength);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "DateHeaderEncode - Failed to encode date header. stat is %d",
                  stat));
        return stat;
    }
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Date header into a Date header object.
 *         All the textual fields are placed inside the object.
 *         You must construct a Date header before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_INVALID_HANDLE,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   -    A handle to the Date header object.
 *    strBuffer -    Buffer containing a textual Date header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderParse(
                                     IN    RvSipDateHeaderHandle    hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgDateHeader* pHeader = (MsgDateHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    DateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_DATE,
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
 * RvSipDateHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Date header value into an Date header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipDateHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Date header object.
 *    buffer    - The buffer containing a textual Date header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderParseValue(
                                     IN    RvSipDateHeaderHandle    hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgDateHeader* pHeader = (MsgDateHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    DateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_DATE,
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
 * RvSipDateHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Date header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipDateHeaderFix(
                                     IN RvSipDateHeaderHandle hHeader,
                                     IN RvChar*              pFixedBuffer)
{
    MsgDateHeader* pHeader = (MsgDateHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipDateHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipDateHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipDateHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipDateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Date object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipDateHeaderGetPool(RvSipDateHeaderHandle hHeader)
{
    return ((MsgDateHeader *)hHeader)->hPool;
}

/***************************************************************************
 * SipDateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Date object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipDateHeaderGetPage(RvSipDateHeaderHandle hHeader)
{
    return ((MsgDateHeader *)hHeader)->hPage;
}

/***************************************************************************
 * RvSipDateHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Date header fields are kept in a string format.
 *          In order to get such a field from the Date header object,
 *          your application should supply an adequate buffer to where the
 *          string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Date header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipDateHeaderGetStringLength(
                                      IN  RvSipDateHeaderHandle     hHeader,
                                      IN  RvSipDateHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgDateHeader* pHeader = (MsgDateHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_DATE_BAD_SYNTAX:
        {
            stringOffset = SipDateHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipDateHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}

/***************************************************************************
 * RvSipDateHeaderGetWeekDay
 * ------------------------------------------------------------------------
 * General: Gets the week-day enumeration. RVSIP_WEEKDAY_UNDEFINED is returned
 *          if the weekday is not set.
 * Return Value: The enumeration of the week day.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 ***************************************************************************/
RVAPI RvSipDateWeekDay RVCALLCONV RvSipDateHeaderGetWeekDay(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_WEEKDAY_UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->eWeekDay;
}


/***************************************************************************
 * RvSipDateHeaderSetWeekDay
 * ------------------------------------------------------------------------
 * General: Sets the week-day parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Expires header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 *         eWeekDay - The week-day enumeration.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetWeekDay(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvSipDateWeekDay         eWeekDay)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->eWeekDay = eWeekDay;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetDay
 * ------------------------------------------------------------------------
 * General: Gets the number for the day of the month in the given Date header.
 *
 * Return Value: Returns the day number. If the integer number for the day of the
 *               month is not set, UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 ***************************************************************************/
RVAPI RvInt8 RVCALLCONV RvSipDateHeaderGetDay(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->day;
}

/***************************************************************************
 * RvSipDateHeaderSetDay
 * ------------------------------------------------------------------------
 * General: Sets the number for the day of the month parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Expires header handle is invalid.
 *               RV_Invalid parameter - The day integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 *         day - The day number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetDay(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvInt8                  day)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (((0 > day) &&
         (UNDEFINED != day)) ||
         (100 <= day))
    {
        return RV_ERROR_BADPARAM;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->day = day;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetMonth
 * ------------------------------------------------------------------------
 * General: Gets the month enumeration of the given Date header.
 *          RVSIP_MONTH_UNDEFINED is returned if the month field is not set.
 * Return Value: The month enumeration.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 ***************************************************************************/
RVAPI RvSipDateMonth RVCALLCONV RvSipDateHeaderGetMonth(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MONTH_UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->eMonth;
}



/***************************************************************************
 * RvSipDateHeaderSetMonth
 * ------------------------------------------------------------------------
 * General: Sets the month parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         eMonth - The month enumeration.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetMonth(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvSipDateMonth           eMonth)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->eMonth = eMonth;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetYear
 * ------------------------------------------------------------------------
 * General: Get the year of the given Date header.
 *          If the year integer was not initialized UNDEFINED is returned.
 * Return Value: Returns the year number. If the year field is not set,
 *               UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipDateHeaderGetYear(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->year;
}


/***************************************************************************
 * RvSipDateHeaderSetYear
 * ------------------------------------------------------------------------
 * General: Sets the year number parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Expires header handle is invalid.
 *               RV_Invalid parameter - The year integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle of the Date header object.
 *         year - The year number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetYear(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvInt16                 year)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (((0 > year) &&
         (UNDEFINED != year)) ||
         (10000 <= year))
    {
        return RV_ERROR_BADPARAM;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->year = year;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetHour
 * ------------------------------------------------------------------------
 * General: Get the hour of the given Date header.
 * Return Value: Returns the hour number. If the hour field is not set,
 *               UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 ***************************************************************************/
RVAPI RvInt8 RVCALLCONV RvSipDateHeaderGetHour(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->hour;
}


/***************************************************************************
 * RvSipDateHeaderSetHour
 * ------------------------------------------------------------------------
 * General: Sets the hour number parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 *               RV_Invalid parameter - The hour integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         year - The hour number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetHour(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvInt8                  hour)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (((0 > hour) &&
         (UNDEFINED != hour)) ||
         (100 <= hour))
    {
        return RV_ERROR_BADPARAM;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->hour = hour;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetMinute
 * ------------------------------------------------------------------------
 * General: Gets the minute of the given Date header.
 * Return Value: Returns the minute number. If the minute field is not set,
 *               UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 ***************************************************************************/
RVAPI RvInt8 RVCALLCONV RvSipDateHeaderGetMinute(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->minute;
}


/***************************************************************************
 * RvSipDateHeaderSetMinute
 * ------------------------------------------------------------------------
 * General: Sets the minute number parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 *               RV_Invalid parameter - The minute integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         minute - The minute number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetMinute(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvInt8                  minute)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (((0 > minute) &&
         (UNDEFINED != minute)) ||
         (100 <= minute))
    {
        return RV_ERROR_BADPARAM;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->minute = minute;
    return RV_OK;
}


/***************************************************************************
 * RvSipDateHeaderGetSecond
 * ------------------------------------------------------------------------
 * General: Gets the second number of the given Date header.
 * Return Value: Returns the second number. If the second field is not set,
 *               UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 ***************************************************************************/
RVAPI RvInt8 RVCALLCONV RvSipDateHeaderGetSecond(
                                        IN  RvSipDateHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return UNDEFINED;
    }
    return ((MsgDateHeader *)hHeader)->second;
}


/***************************************************************************
 * RvSipDateHeaderSetSecond
 * ------------------------------------------------------------------------
 * General: Sets the second number parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 *               RV_Invalid parameter - The second integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         second - The second number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetSecond(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  RvInt8                  second)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (((0 > second) &&
         (UNDEFINED != second)) ||
         (100 <= second))
    {
        return RV_ERROR_BADPARAM;
    }
    pDate = (MsgDateHeader *)hHeader;
    pDate->second = second;
    return RV_OK;
}


/***************************************************************************
 * SipDateHeaderSetZone
 * ------------------------------------------------------------------------
 * General: Sets the zone parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 *               RV_Invalid parameter - The second integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         second - The second number.
 ***************************************************************************/
RvStatus RVCALLCONV SipDateHeaderSetZone(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  MsgDateTimeZone          zone)
{
    MsgDateHeader *pDate;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pDate = (MsgDateHeader *)hHeader;
    pDate->zone = zone;
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                          */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * DateEncode
 * ------------------------------------------------------------------------
 * General: Encodes the date structure of the given Date header.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the date header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
static RvStatus DateEncode(
                            IN    RvSipDateHeaderHandle    hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                 bInUrlHeaders,
                            INOUT RvUint32*               pLength)
{
    MsgDateHeader*  pHeader;
    RvStatus          stat;
    RvChar            strHelper[16];

    pHeader = (MsgDateHeader*)hHeader;

    if ((UNDEFINED == pHeader->day) ||
        (RVSIP_MONTH_UNDEFINED == pHeader->eMonth) ||
        (UNDEFINED == pHeader->hour) ||
        (UNDEFINED == pHeader->minute) ||
        (UNDEFINED == pHeader->second) ||
        (UNDEFINED == pHeader->year))
    {
        return RV_ERROR_BADPARAM;
    }

    /* Encode week day */
    if(RVSIP_WEEKDAY_UNDEFINED != pHeader->eWeekDay)
    {
        switch (pHeader->eWeekDay)
        {
        case RVSIP_WEEKDAY_SUN:
            strcpy(strHelper, "Sun");
            break;
        case RVSIP_WEEKDAY_MON:
            strcpy(strHelper, "Mon");
            break;
        case RVSIP_WEEKDAY_TUE:
            strcpy(strHelper, "Tue");
            break;
        case RVSIP_WEEKDAY_WED:
            strcpy(strHelper, "Wed");
            break;
        case RVSIP_WEEKDAY_THU:
            strcpy(strHelper, "Thu");
            break;
        case RVSIP_WEEKDAY_FRI:
            strcpy(strHelper, "Fri");
            break;
        case RVSIP_WEEKDAY_SAT:
            strcpy(strHelper, "Sat");
            break;
        default:
            return RV_ERROR_BADPARAM;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
        if (RV_OK != stat)
        {
            return stat;
        }
        /* set "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetCommaChar(bInUrlHeaders), pLength);
        if (RV_OK != stat)
        {
            return stat;
        }

        /* set space " " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
        if (RV_OK != stat)
        {
            return stat;
        }
    }

    /* Encode day */
    if (10 > pHeader->day)
    {
        MsgUtils_itoa(0, strHelper);
        MsgUtils_itoa(pHeader->day , strHelper+1);
    }
    else
    {
        MsgUtils_itoa(pHeader->day , strHelper);
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode month */
    /* space before */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
    switch (pHeader->eMonth)
    {
    case RVSIP_MONTH_JAN:
        strcpy(strHelper, "Jan");
        break;
    case RVSIP_MONTH_FEB:
        strcpy(strHelper, "Feb");
        break;
    case RVSIP_MONTH_MAR:
        strcpy(strHelper, "Mar");
        break;
    case RVSIP_MONTH_APR:
        strcpy(strHelper, "Apr");
        break;
    case RVSIP_MONTH_MAY:
        strcpy(strHelper, "May");
        break;
    case RVSIP_MONTH_JUN:
        strcpy(strHelper, "Jun");
        break;
    case RVSIP_MONTH_JUL:
        strcpy(strHelper, "Jul");
        break;
    case RVSIP_MONTH_AUG:
        strcpy(strHelper, "Aug");
        break;
    case RVSIP_MONTH_SEP:
        strcpy(strHelper, "Sep");
        break;
    case RVSIP_MONTH_OCT:
        strcpy(strHelper, "Oct");
        break;
    case RVSIP_MONTH_NOV:
        strcpy(strHelper, "Nov");
        break;
    case RVSIP_MONTH_DEC:
        strcpy(strHelper, "Dec");
        break;
    default:
        return RV_ERROR_BADPARAM;
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }
    /*space after */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);

    /* Encode year */
    MsgUtils_itoa(pHeader->year , strHelper);
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode ' ' */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode hour */
    if (10 > pHeader->hour)
    {
        MsgUtils_itoa(0, strHelper);
        MsgUtils_itoa(pHeader->hour , strHelper+1);
    }
    else
    {
        MsgUtils_itoa(pHeader->hour , strHelper);
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode ':' */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, ":", pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode minute */
    if (10 > pHeader->minute)
    {
        MsgUtils_itoa(0, strHelper);
        MsgUtils_itoa(pHeader->minute , strHelper+1);
    }
    else
    {
        MsgUtils_itoa(pHeader->minute , strHelper);
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode ':' */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, ":", pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode second */
    if (10 > pHeader->second)
    {
        MsgUtils_itoa(0, strHelper);
        MsgUtils_itoa(pHeader->second , strHelper+1);
    }
    else
    {
        MsgUtils_itoa(pHeader->second , strHelper);
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        return stat;
    }

    /* Encode the time zone (with space before it) */
    switch(pHeader->zone)
    {
    case MSG_DATE_TIME_ZONE_GMST:
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "GMST", pLength);
        break;
    case MSG_DATE_TIME_ZONE_UTC:
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "UTC", pLength);
        break;
    case MSG_DATE_TIME_ZONE_GMT:
    case MSG_DATE_TIME_ZONE_UNDEFINED:
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, MsgUtilsGetSpaceChar(bInUrlHeaders), pLength);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "GMT", pLength);
        break;
    }

    if (RV_OK != stat)
    {
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * SipDateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipDateHeaderGetStrBadSyntax(
                                    IN RvSipDateHeaderHandle hHeader)
{
    return ((MsgDateHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipDateHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Date header,
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
RVAPI RvStatus RVCALLCONV RvSipDateHeaderGetStrBadSyntax(
                                               IN RvSipDateHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgDateHeader*)hHeader)->hPool,
                                  ((MsgDateHeader*)hHeader)->hPage,
                                  SipDateHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipDateHeaderSetStrBadSyntax
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
RvStatus SipDateHeaderSetStrBadSyntax(
                                  IN RvSipDateHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgDateHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgDateHeader*)hHeader;

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
 * RvSipDateHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Date header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipDateHeaderSetStrBadSyntax(
                                  IN RvSipDateHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipDateHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * DateHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void DateHeaderClean( IN MsgDateHeader* pHeader,
                             IN RvBool        bCleanBS)
{
    pHeader->day              = UNDEFINED;
    pHeader->eMonth           = RVSIP_MONTH_UNDEFINED;
    pHeader->eWeekDay         = RVSIP_WEEKDAY_UNDEFINED;
    pHeader->hour             = UNDEFINED;
    pHeader->minute           = UNDEFINED;
    pHeader->second           = UNDEFINED;
    pHeader->year             = UNDEFINED;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_DATE;
    pHeader->zone             = MSG_DATE_TIME_ZONE_UNDEFINED;

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

