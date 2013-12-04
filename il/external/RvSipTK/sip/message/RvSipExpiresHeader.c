/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipExpiresHeader.c                           *
 *                                                                            *
 * The file defines the methods of the Expires header object.                 *
 * The Expires header functions enable you to construct, copy, encode, parse, *
 * access and change Expires Header parameters.                               *                                                *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Jan 2001                                              *
 ******************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipExpiresHeader.h"
#include "_SipExpiresHeader.h"
#include "MsgInterval.h"
#include "RvSipDateHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
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
static void ExpiresHeaderClean( IN MsgExpiresHeader* pHeader,
                                IN RvBool           bCleanBS);

static RvStatus RVCALLCONV ExpiresSetDeltaSeconds(
										   IN MsgExpiresHeader* pHeader,
										   IN RvInt32           deltaSeconds);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipExpiresHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Expires header object inside a given message object.
 *          The header is kept in the header list of the message. You can
 *          choose to insert the header either at the head or tail of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: phHeader - Handle to the newly constructed Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderConstructInMsg(
                                IN  RvSipMsgHandle            hSipMsg,
                                IN  RvBool                   pushHeaderAtHead,
                                OUT RvSipExpiresHeaderHandle* phHeader)
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

    stat = RvSipExpiresHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_EXPIRES,
                              &hListElem,
                              (void**)phHeader);
}

/***************************************************************************
 * RvSipExpiresConstructInContactHeader
 * ------------------------------------------------------------------------
 * General: Constructs an Expires object inside a given Contact header.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader - Handle to the Contact header that relates to this Expires object.
 * output: phExpires - Handle to the newly constructed expires object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresConstructInContactHeader(
                                       IN  RvSipContactHeaderHandle  hHeader,
                                       OUT RvSipExpiresHeaderHandle *phExpires)
{
    MsgContactHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phExpires == NULL)
        return RV_ERROR_NULLPTR;

    *phExpires = NULL;

    pHeader = (MsgContactHeader*)hHeader;

    /* creating the new expires object */
    stat = RvSipExpiresHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr, pHeader->hPool, pHeader->hPage, phExpires);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new expires parameter to the Contact header */
    pHeader->hExpiresParam = *phExpires;

    return RV_OK;
}

/***************************************************************************
 * RvSipExpiresHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Expires Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: phHeader - Handle to the newly constructed Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderConstruct(
                                        IN  RvSipMsgMgrHandle         hMsgMgr,
                                        IN  HRPOOL                    hPool,
                                        IN  HPAGE                     hPage,
                                        OUT RvSipExpiresHeaderHandle* phHeader)
{
    MsgExpiresHeader*   pHeader;
    MsgMgr*             pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pHeader = (MsgExpiresHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgExpiresHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipExpiresHeaderConstruct - Failed to construct expire header. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    if(IntervalConstruct(pMsgMgr, hPool, hPage, &pHeader->pInterval) != RV_OK)
        return RV_ERROR_OUTOFRESOURCES;

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    ExpiresHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipExpiresHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipExpiresHeaderConstruct - Expires header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *phHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipExpiresHeaderCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Expires header object to a
 *         destination Expires header object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Expires header object.
 *    pSource      - Handle to the source Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderCopy(
                                     INOUT RvSipExpiresHeaderHandle hDestination,
                                     IN    RvSipExpiresHeaderHandle hSource)
{
    MsgExpiresHeader*   source;
    MsgExpiresHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgExpiresHeader*)hSource;
    dest   = (MsgExpiresHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

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
                "RvSipExpiresHeaderCopy - failed in coping strBadSyntax."));
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

    return IntervalCopy(dest->pInterval, source->pInterval);
}

/***************************************************************************
 * RvSipExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Expires header object to a textual Expires header.
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
 * input: hHeader  - Handle to the Expires header object.
 *        hPool    - Handle to the specified memory pool.
 * output: phPage   - The memory page allocated to contain the encoded header.
 *         pLength  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderEncode(
                                      IN    RvSipExpiresHeaderHandle hHeader,
                                      IN    HRPOOL                   hPool,
                                      OUT   HPAGE*                   phPage,
                                      OUT   RvUint32*               pLength)
{
    RvStatus stat;
    MsgExpiresHeader* pHeader;

    pHeader = (MsgExpiresHeader*)hHeader;
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
                  "RvSipExpiresHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipExpiresHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *phPage, hPool, hHeader));
    }

    *pLength = 0; /* This way length will give the real length, and will not
                     just add the new length to the old one */
    stat = ExpiresHeaderEncode(hHeader, hPool, *phPage, RV_FALSE, RV_FALSE, pLength);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *phPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipExpiresHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ExpiresHeaderEncode fail",
                  *phPage, hPool));
    }
    return stat;
}


/***************************************************************************
 * ExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Expires header (as string) on it.
 *          format: "Expires: "
 *                  (SIP-date|delta-seconds)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the expires header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bIsParameter - True if the expires is a parameter of a Contact
 *                       header and False if this is an Expires header. The
 *                       encoding is done appropriately.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  ExpiresHeaderEncode(
                                  IN    RvSipExpiresHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                  bIsParameter,
                                  IN    RvBool                bInUrlHeaders,
                                  INOUT RvUint32*               pLength)
{
    MsgExpiresHeader*  pHeader;
    RvStatus          stat;

   if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    pHeader = (MsgExpiresHeader*) hHeader;

     RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "ExpiresHeaderEncode - Encoding Expires header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
               hHeader, hPool, hPage));

    if (RVSIP_EXPIRES_FORMAT_UNDEFINED == pHeader->pInterval->eFormat &&
        UNDEFINED == pHeader->strBadSyntax )
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_FALSE == bIsParameter)
    {
        /* put "Expires" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Expires", pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ExpiresHeaderEncode - Failed to encoding expires header. stat is %d",
                      stat));
            return stat;
        }
        /* encode ": " or "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        pLength);
    }
    else
    {
        /* put "expires= " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "expires=", pLength);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ExpiresHeaderEncode - Failed to encoding expires header. stat is %d",
                      stat));
            return stat;
        }
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
                "ExpiresHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }
    else
    {
        return IntervalEncode(pHeader->pInterval, hPool, hPage, bIsParameter, bInUrlHeaders, pLength);
    }


}

/***************************************************************************
 * RvSipExpiresHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual expires header-for example,
 *         "Expires: Thu, 01 Dec 2040 16:00:00 GMT"-into a Expires header
 *          object. All the textual fields are placed inside the object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_INVALID_HANDLE,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   -    A handle to the Expires header object.
 *    strBuffer    - Buffer containing a textual Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderParse(
                                     IN    RvSipExpiresHeaderHandle hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgExpiresHeader* pHeader = (MsgExpiresHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    ExpiresHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_EXPIRES,
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
 * RvSipExpiresHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Expires header value into an Expires header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipExpiresHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Expires header object.
 *    buffer    - The buffer containing a textual Expires header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderParseValue(
                                     IN    RvSipExpiresHeaderHandle hHeader,
                                     IN    RvChar*                 strBuffer)
{
    MsgExpiresHeader* pHeader = (MsgExpiresHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (strBuffer == NULL))
        return RV_ERROR_BADPARAM;

    ExpiresHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_EXPIRES,
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
 * RvSipExpiresHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Expires header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderFix(
                                     IN RvSipExpiresHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgExpiresHeader* pHeader = (MsgExpiresHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipExpiresHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipExpiresHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipExpiresHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * RvSipExpiresHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Expires header fields are kept in a string format.
 *          In order to get such a field from the Expires header object,
 *          your application should supply an adequate buffer to where the
 *          string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Expires header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipExpiresHeaderGetStringLength(
                                      IN  RvSipExpiresHeaderHandle     hHeader,
                                      IN  RvSipExpiresHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgExpiresHeader* pHeader = (MsgExpiresHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_EXPIRES_BAD_SYNTAX:
        {
            stringOffset = SipExpiresHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipExpiresHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}
/***************************************************************************
 * SipExpiresHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Expires object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipExpiresHeaderGetPool(RvSipExpiresHeaderHandle hHeader)
{
    return ((MsgExpiresHeader *)hHeader)->hPool;
}

/***************************************************************************
 * SipExpiresHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Expires object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipExpiresHeaderGetPage(RvSipExpiresHeaderHandle hHeader)
{
    return ((MsgExpiresHeader *)hHeader)->hPage;
}


/***************************************************************************
 * RvSipExpiresHeaderGetFormat
 * ------------------------------------------------------------------------
 * General: Gets the format of the Expires header: Undefined, Date or
 *          Delta-seconds.
 * Return Value: The format enumeration.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Expires header object.
 ***************************************************************************/
RVAPI RvSipExpiresFormat RVCALLCONV RvSipExpiresHeaderGetFormat(
                                    IN  RvSipExpiresHeaderHandle     hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_EXPIRES_FORMAT_UNDEFINED;
    }
    return ((MsgExpiresHeader *)hHeader)->pInterval->eFormat;
}


/***************************************************************************
 * RvSipExpiresHeaderGetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Gets the delta-seconds integer of the Expires header.
 *          If the delta-seconds integer is not set, UNDEFINED is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - pDeltaSeconds is NULL.
 *               RV_ERROR_NOT_FOUND - delta-seconds were not defined for this
 *                             expires object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Expires header object.
 *  Output: pDeltaSeconds - The delta-seconds integer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderGetDeltaSeconds(
                                  IN  RvSipExpiresHeaderHandle hHeader,
                                  OUT RvUint32               *pDeltaSeconds)
{
    MsgExpiresHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDeltaSeconds)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgExpiresHeader *)hHeader;
    if (RVSIP_EXPIRES_FORMAT_DELTA_SECONDS != pHeader->pInterval->eFormat)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *pDeltaSeconds = pHeader->pInterval->deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * RvSipExpiresHeaderSetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Sets the delta seconds integer of the Expires header. Changes
 *          the Expires format to delta-seconds. If the given delta-seconds
 *          is UNDEFINED, the delta-seconds of the Expires header is removed
 *          and the format is changed to UNDEFINED.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Expires header handle is invalid.
 *               RV_Invalid parameter - The delta-seconds integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Expires header object.
 *         deltaSeconds - The delta-seconds to be set to the Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderSetDeltaSeconds(
                                   IN  RvSipExpiresHeaderHandle hHeader,
                                   IN  RvInt32                deltaSeconds)
{
    MsgExpiresHeader *pExpires;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pExpires = (MsgExpiresHeader *)hHeader;
    return ExpiresSetDeltaSeconds(pExpires, deltaSeconds);
}


/***************************************************************************
 * RvSipExpiresHeaderGetDateHandle
 * ------------------------------------------------------------------------
 * General: Gets the date Handle to the Expires header.
 * Return Value: Returns the handle to the date header object, or NULL
 *               if the date header object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Expires header object.
 ***************************************************************************/
RVAPI RvSipDateHeaderHandle RVCALLCONV RvSipExpiresHeaderGetDateHandle(
                                        IN  RvSipExpiresHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return NULL;
    }
    if (RVSIP_EXPIRES_FORMAT_DATE != ((MsgExpiresHeader *)hHeader)->pInterval->eFormat)
    {
        return NULL;
    }
    return ((MsgExpiresHeader *)hHeader)->pInterval->hDate;
}


/***************************************************************************
 * RvSipExpiresHeaderSetDateHandle
 * ------------------------------------------------------------------------
 * General: Sets a new Date header in the Expires header object and changes
 *          the Expires format to date.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Expires header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Expires header object.
 *         hDate - The date handle to be set to the Expires header.
 *                 If the date handle is NULL, the existing date header
 *                 is removed from the expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderSetDateHandle(
                                       IN  RvSipExpiresHeaderHandle hHeader,
                                       IN  RvSipDateHeaderHandle    hDate)
{
    MsgExpiresHeader     *pExpires;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pExpires = (MsgExpiresHeader *)hHeader;

    return IntervalSetDate(pExpires->pInterval, hDate);

}

/***************************************************************************
 * SipExpiresHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipExpiresHeaderGetStrBadSyntax(
                                    IN RvSipExpiresHeaderHandle hHeader)
{
    return ((MsgExpiresHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipExpiresHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Expires header,
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
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderGetStrBadSyntax(
                                               IN RvSipExpiresHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgExpiresHeader*)hHeader)->hPool,
                                  ((MsgExpiresHeader*)hHeader)->hPage,
                                  SipExpiresHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipExpiresHeaderSetStrBadSyntax
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
RvStatus SipExpiresHeaderSetStrBadSyntax(
                                  IN RvSipExpiresHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgExpiresHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgExpiresHeader*)hHeader;

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
 * RvSipExpiresHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Expires header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipExpiresHeaderSetStrBadSyntax(
                                  IN RvSipExpiresHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipExpiresHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ExpiresHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ExpiresHeaderClean( IN MsgExpiresHeader* pHeader,
                                IN RvBool           bCleanBS)
{
    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }

    pHeader->eHeaderType      = RVSIP_HEADERTYPE_EXPIRES;

}

/***************************************************************************
 * ExpiresSetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Sets the delta seconds value to the delta seconds parameter. 
 *          There is different implementation depending on whether the 
 *          delta seconds value is signed or unsigned int
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pHeader - The Expires header
 *          deltaSeconds - The delta seconds value
 ***************************************************************************/
static RvStatus RVCALLCONV ExpiresSetDeltaSeconds(
										   IN MsgExpiresHeader* pHeader,
										   IN RvInt32           deltaSeconds)
{
#ifndef RV_SIP_UINT_DELTASECONDS
	if (deltaSeconds < 0)
	{
		/* This means that UNDFINED is set to the delta seconds value */
		pHeader->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_UNDEFINED;
		return RV_OK;
	}
#endif /* #ifndef RV_SIP_UINT_DELTASECONDS */ 

	pHeader->pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
	pHeader->pInterval->deltaSeconds = deltaSeconds;

	return RV_OK;
}

#endif /*#ifndef RV_SIP_LIGHT*/

