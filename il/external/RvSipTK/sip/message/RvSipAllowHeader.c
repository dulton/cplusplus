/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           RvSipAllowHeader.c                               *
 *                                                                            *
 * The file defines the methods of the Allow header object:                   *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipAllowHeader.h"
#include "_SipAllowHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include <string.h>



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AllowHeaderClean( IN MsgAllowHeader* pHeader,
                                    IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAllowHeaderConstructInMsg
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
 * output: hHeader          - Handle to the newly constructed Allow header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           IN  RvBool                 pushHeaderAtHead,
                                           OUT RvSipAllowHeaderHandle* hHeader)
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

    stat = RvSipAllowHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);

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
                              RVSIP_HEADERTYPE_ALLOW,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipAllowHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Allow Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle of the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Allow header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderConstruct(
                                           IN  RvSipMsgMgrHandle       hMsgMgr,
                                           IN  HRPOOL                  hPool,
                                           IN  HPAGE                   hPage,
                                           OUT RvSipAllowHeaderHandle* hHeader)
{
    MsgAllowHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hPage == NULL_PAGE) || (hPool == NULL) || (hMsgMgr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAllowHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgAllowHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAllowHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr     = pMsgMgr;
    pHeader->hPage       = hPage;
    pHeader->hPool       = hPool;
    AllowHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAllowHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAllowHeaderConstruct - Allow header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipAllowHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Allow header object to a destination Allow
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Allow header object.
 *    hSource      - Handle to the source Allow header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderCopy(
                                         INOUT RvSipAllowHeaderHandle hDestination,
                                         IN    RvSipAllowHeaderHandle hSource)
{
    MsgAllowHeader *source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAllowHeader*)hSource;
    dest = (MsgAllowHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->eMethod = source->eMethod;
    if((dest->eMethod == RVSIP_METHOD_OTHER) && (source->strMethod > UNDEFINED))
    {
        dest->strMethod = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                        dest->hPool,
                                                        dest->hPage,
                                                        source->hPool,
                                                        source->hPage,
                                                        source->strMethod);
        if(dest->strMethod == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAllowHeaderCopy - fail in coping strMethod."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMethod = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                     dest->strMethod);
#endif
    }
    else
    {
        dest->strMethod = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMethod = NULL;
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
                "RvSipAllowHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipAllowHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Allow header object to a textual Allow header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Allow header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderEncode(
                                          IN    RvSipAllowHeaderHandle hHeader,
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

    pMsgMgr = ((MsgAllowHeader*)hHeader)->pMsgMgr;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AllowHeaderEncode(hHeader, hPool, *hPage, RV_TRUE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAllowHeaderEncode - Failed. Free page 0x%p on pool 0x%p. AllowHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AllowHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Allow header (as string) on it.
 *          format: "Allow: " Method
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the allow header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstAllowInMessage - RV_TRUE if this parameter is the first Allow header
 *                                 between the Allow headers in the headers list,
 *                                 RV_FALSE otherwise.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV  AllowHeaderEncode(
                              IN    RvSipAllowHeaderHandle hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                   bFirstAllowInMessage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length)
{
    MsgAllowHeader* pHeader;
    RvStatus         stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgAllowHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AllowHeaderEncode - Encoding Allow header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    if(bFirstAllowInMessage == RV_TRUE)
    {
        /* encode "Allow:" or "Allow=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Allow", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AllowHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);

    }
    else
    {
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
                "AllowHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* method */
    stat = MsgUtilsEncodeMethodType(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->eMethod,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strMethod,
                                    length);

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AllowHeaderEncode - Failed to encode Allow header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    else
        return RV_OK;
}


/***************************************************************************
 * RvSipAllowHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Allow header into an Allow header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Allow header object.
 *    buffer    - Buffer containing a textual Allow header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderParse(
                                     IN RvSipAllowHeaderHandle hHeader,
                                     IN RvChar*               buffer)
{
    MsgAllowHeader* pHeader = (MsgAllowHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;

    AllowHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ALLOW,
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
 * RvSipAllowHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Allow header value into an Allow header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAllowHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Allow header object.
 *    buffer    - The buffer containing a textual Allow header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderParseValue(
                                     IN RvSipAllowHeaderHandle hHeader,
                                     IN RvChar*               buffer)
{
    MsgAllowHeader* pHeader = (MsgAllowHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;

    AllowHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ALLOW,
                                        buffer,
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
 * RvSipAllowHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Allow header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderFix(
                                     IN RvSipAllowHeaderHandle hHeader,
                                     IN RvChar*               pFixedBuffer)
{
    MsgAllowHeader* pHeader = (MsgAllowHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAllowHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAllowHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAllowHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAllowHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Allow object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAllowHeaderGetPool(RvSipAllowHeaderHandle hHeader)
{
    return ((MsgAllowHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAllowHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Allow object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAllowHeaderGetPage(RvSipAllowHeaderHandle hHeader)
{
    return ((MsgAllowHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAllowHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Allow header fields are kept in a string format-for example, the
 *          method string. In order to get such a field from the Allow header object, your
 *          application should supply an adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to
 *          allocator an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Allow header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAllowHeaderGetStringLength(
                                      IN  RvSipAllowHeaderHandle     hHeader,
                                      IN  RvSipAllowHeaderStringName stringName)
{
    RvInt32        stringOffset;
    MsgAllowHeader* pHeader = (MsgAllowHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_ALLOW_METHOD:
            stringOffset = SipAllowHeaderGetStrMethodType(hHeader);
            break;
        case RVSIP_ALLOW_BAD_SYNTAX:
            stringOffset = SipAllowHeaderGetStrBadSyntax(hHeader);
            break;
        default:
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAllowHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool,
                            pHeader->hPage,
                            stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * RvSipAllowHeaderGetMethodType
 * ------------------------------------------------------------------------
 * General: Gets the method type enumeration from the Allow header object.
 * Return Value: Returns the method type enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Allow header object.
 ***************************************************************************/
RVAPI RvSipMethodType RVCALLCONV RvSipAllowHeaderGetMethodType(
                                      IN  RvSipAllowHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_METHOD_UNDEFINED;

    return ((MsgAllowHeader*)hHeader)->eMethod;
}

/***************************************************************************
 * SipAllowHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type string value from the
 *          Allow object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Allow header object.
 ***************************************************************************/
RvInt32 SipAllowHeaderGetStrMethodType(IN  RvSipAllowHeaderHandle hHeader)
{
    return ((MsgAllowHeader*)hHeader)->strMethod;
}

/***************************************************************************
 * RvSipAllowHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General:Copies the method type string from the Allow header object into a given buffer.
 *         Use this function if RvSipAllowHeaderGetMethodType() returns
 *         RVSIP_METHOD_OTHER.
 *         If the bufferLen is adequate, the function copies the requested parameter into
 *         strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *         contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Allow header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderGetStrMethodType(
                                               IN  RvSipAllowHeaderHandle hHeader,
                                               IN  RvChar*               strBuffer,
                                               IN  RvUint                bufferLen,
                                               OUT RvUint*               actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAllowHeader*)hHeader)->hPool,
                                  ((MsgAllowHeader*)hHeader)->hPage,
                                  SipAllowHeaderGetStrMethodType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipAllowHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMethodType in the
 *          AllowHeader object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader - Handle of the Allow header object.
 *        eMethodType - The method type to be set in the object.
 *          strMethodType - text string giving the method type to be set in the object.
 *        strMethodOffset - Offset of the method string (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAllowHeaderSetMethodType(
                                  IN RvSipAllowHeaderHandle hHeader,
                                  IN RvSipMethodType        eMethodType,
                                  IN RvChar*               strMethodType,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strMethodOffset)
{
    MsgAllowHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAllowHeader*)hHeader;

    header->eMethod = eMethodType;

    if(eMethodType == RVSIP_METHOD_OTHER)
    {
        retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strMethodOffset,
                                      strMethodType, header->hPool,
                                      header->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        header->strMethod = newStrOffset;
#ifdef SIP_DEBUG
        header->pMethod = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                       header->strMethod);
#endif
    }
    else
    {
        header->strMethod = UNDEFINED;
#ifdef SIP_DEBUG
        header->pMethod = NULL;
#endif
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipAllowHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General: Sets the method type in the Allow header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Allow header object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - You can use this parameter only if the eMethodType parameter is set to
 *                  RVSIP_METHOD_OTHER. In this case, you can supply the method as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderSetMethodType(
                                          IN    RvSipAllowHeaderHandle hHeader,
                                          IN    RvSipMethodType        eMethodType,
                                          IN    RvChar*               strMethodType)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAllowHeaderSetMethodType(hHeader, eMethodType, strMethodType, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * SipAllowHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the header object.
 ***************************************************************************/
RvInt32 SipAllowHeaderGetStrBadSyntax(IN  RvSipAllowHeaderHandle hHeader)
{
    return ((MsgAllowHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAllowHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Allow header,
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
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderGetStrBadSyntax(
                                               IN  RvSipAllowHeaderHandle hHeader,
                                               IN  RvChar*               strBuffer,
                                               IN  RvUint                bufferLen,
                                               OUT RvUint*               actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAllowHeader*)hHeader)->hPool,
                                  ((MsgAllowHeader*)hHeader)->hPage,
                                  SipAllowHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAllowHeaderSetStrBadSyntax
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
RvStatus SipAllowHeaderSetStrBadSyntax(
                                  IN RvSipAllowHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgAllowHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAllowHeader*)hHeader;

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
 * RvSipAllowHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Allow header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAllowHeaderSetStrBadSyntax(
                                  IN RvSipAllowHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAllowHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AllowHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AllowHeaderClean( IN MsgAllowHeader* pHeader,
                              IN RvBool         bCleanBS)
{
    pHeader->strMethod   = UNDEFINED;
    pHeader->eMethod     = RVSIP_METHOD_UNDEFINED;
    pHeader->eHeaderType = RVSIP_HEADERTYPE_ALLOW;

#ifdef SIP_DEBUG
    pHeader->pMethod    = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

