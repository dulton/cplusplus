/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipContentDispositionHeader.h                                     *
 *                                                                            *
 * The file defines the methods of the Content-Diposition header object       *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Aug 2001                                              *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipContentDispositionHeader.h"
#include "_SipContentDispositionHeader.h"
#include "RvSipBodyPart.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ContentDispositionHeaderClean(
                                    IN MsgContentDispositionHeader* pHeader,
                                    IN RvBool                      bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipContentDispositionHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Content-Disposition header object inside a given
 *          message object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the list-RV_TRUE
 *                            -or to the tail-RV_FALSE.
 * output: hHeader - Handle to the newly constructed Content-Disposition header
 *                   object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderConstructInMsg(
                        IN  RvSipMsgHandle                       hSipMsg,
                        IN  RvBool                              pushHeaderAtHead,
                        OUT RvSipContentDispositionHeaderHandle* hHeader)
{

    MsgMessage*               msg;
    RvSipHeaderListElemHandle hListElem = NULL;
    RvSipHeadersLocation      location;
    RvStatus                 stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipContentDispositionHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                  msg->hPool, msg->hPage,
                                                  hHeader);
    if (RV_OK != stat)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_CONTENT_DISPOSITION,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipContentDispositionHeaderConstructInBodyPart
 * ------------------------------------------------------------------------
 * General: Constructs a Content-Disposition header object inside a given
 *          message body part object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBodyPart - Handle to the message body part object.
 * output: hHeader - Handle to the newly constructed Content-Disposition header
 *                   object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderConstructInBodyPart(
                        IN  RvSipBodyPartHandle                  hBodyPart,
                        OUT RvSipContentDispositionHeaderHandle* hHeader)
{

    BodyPart*              pBodyPart;
    RvStatus                 stat;

    if(hBodyPart == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pBodyPart = (BodyPart*)hBodyPart;

    stat = RvSipContentDispositionHeaderConstruct((RvSipMsgMgrHandle)pBodyPart->pMsgMgr,
                                                  pBodyPart->hPool,
                                                  pBodyPart->hPage, hHeader);
    if (RV_OK != stat)
        return stat;

    /* attach the header in the body part */
    pBodyPart->pContentDisposition = (MsgContentDispositionHeader *)(*hHeader);

    return RV_OK;
}

/***************************************************************************
 * RvSipContentDispositionHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Content-Disposition Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Content-Disposition header
 *                   object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderConstruct(
                              IN  RvSipMsgMgrHandle                    hMsgMgr,
                              IN  HRPOOL                               hPool,
                              IN  HPAGE                                hPage,
                              OUT RvSipContentDispositionHeaderHandle* hHeader)
{
    MsgContentDispositionHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (NULL == hHeader) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgContentDispositionHeader*)
                        SipMsgUtilsAllocAlign(pMsgMgr,
                                              hPool,
                                              hPage,
                                              sizeof(MsgContentDispositionHeader),
                                              RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipContentDispositionHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr         = pMsgMgr;
    pHeader->hPage           = hPage;
    pHeader->hPool           = hPool;
    ContentDispositionHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipContentDispositionHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipContentDispositionHeaderConstruct - Content-Disposition header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *hHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipContentDispositionHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Content-Disposition header object
 *          to a destination Content-Disposition header object.
 *          You must construct the destination object before using this
 *          function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Content-Disposition header object.
 *    hSource      - Handle to the source Content-Disposition header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderCopy(
                        INOUT RvSipContentDispositionHeaderHandle hDestination,
                        IN    RvSipContentDispositionHeaderHandle hSource)
{
    MsgContentDispositionHeader* source;
    MsgContentDispositionHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgContentDispositionHeader*)hSource;
    dest   = (MsgContentDispositionHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* disposition type and handling enumerations */
    dest->eHandling  = source->eHandling;
    dest->eType      = source->eType;

    /* Type string */
    if  ((source->eType == RVSIP_DISPOSITIONTYPE_OTHER) &&
         (source->strType > UNDEFINED))
    {
        dest->strType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                        dest->hPool,
                                                        dest->hPage,
                                                        source->hPool,
                                                        source->hPage,
                                                        source->strType);
        if (dest->strType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentDispositionHeaderCopy - Failed to copy disposition type. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                       dest->strType);
#endif
    }
    else
    {
        dest->strType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pType = NULL;
#endif
    }

    /* Handling string */
    if((source->eHandling == RVSIP_DISPOSITIONHANDLING_OTHER) &&
       (source->strHandling > UNDEFINED))
    {
        dest->strHandling = MsgUtilsAllocCopyRpoolString(
                                                        source->pMsgMgr,
                                                        dest->hPool,
                                                        dest->hPage,
                                                        source->hPool,
                                                        source->hPage,
                                                        source->strHandling);
        if (dest->strHandling == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentDispositionHeaderCopy - Failed to copy handling parameter. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pHandling = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strHandling);
#endif
    }
    else
    {
        dest->strHandling = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pHandling = NULL;
#endif
    }

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
                      "RvSipContentDispositionHeaderCopy - Failed to copy other param. stat %d",
                      RV_ERROR_OUTOFRESOURCES));
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
                "RvSipContentDispositionHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipContentDispositionHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Content-Disposition header object to a textual
 *          Content-Disposition header. The textual header is placed on a page
 *          taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Content-Disposition header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderEncode(
                             IN    RvSipContentDispositionHeaderHandle hHeader,
                             IN    HRPOOL                              hPool,
                             OUT   HPAGE*                              hPage,
                             OUT   RvUint32*                          length)
{
    RvStatus stat;
    MsgContentDispositionHeader* pHeader;

    pHeader = (MsgContentDispositionHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0; /* so length will give the real length, and will not just add the
                    new length to the old one */

    if ((hPool == NULL) || (hHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentDispositionHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentDispositionHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *hPage, hPool, hHeader));
    }

    stat = ContentDispositionHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentDispositionHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ContentDispositionHeaderEncode fail",
                  *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * ContentDispositionHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-Disposition header (as string) on it.
 *          Content-Disposition =
 *          "Content-Disposition:" disposition-type *(hanling-parameter)
 *                                                  *(;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Disposition header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentDispositionHeaderEncode(
                             IN    RvSipContentDispositionHeaderHandle hHeader,
                             IN    HRPOOL                              hPool,
                             IN    HPAGE                               hPage,
                             IN    RvBool                              bInUrlHeaders,
                             INOUT RvUint32*                           length)
{
    MsgContentDispositionHeader*   pHeader;
    RvStatus                      stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgContentDispositionHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "ContentDispositionHeaderEncode - Encoding Content-Disposition header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
              hHeader, hPool, hPage));

    /* set "Content-Disposition" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Content-Disposition", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContentDispositionHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);

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
                "ContentDispositionHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* set disposition-type */
    stat = MsgUtilsEncodeDispositionType(
                                   pHeader->pMsgMgr,
                                   hPool, hPage, pHeader->eType,
                                   pHeader->hPool, pHeader->hPage,
                                   pHeader->strType, length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "ContentDispositionHeaderEncode: Failed to encode Content-Disposition header. stat is %d",
                  stat));
        return stat;
    }

    /* Content-Disposition parameters with ";" between them */
    /* set handling */
    if(RVSIP_DISPOSITIONHANDLING_UNDEFINED != pHeader->eHandling)
    {
        /* set ";handling=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "handling", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        /* = */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeDispositionHandling(
                                   pHeader->pMsgMgr,
                                   hPool, hPage, pHeader->eHandling,
                                   pHeader->hPool, pHeader->hPage,
                                   pHeader->strHandling, length);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContentDispositionHeaderEncode: Failed to encode Content-Disposition header. stat is %d",
                      stat));
            return stat;
        }
    }

    /* other parameters */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set pHeader->strOtherParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "ContentDispositionHeaderEncode: Failed to encode Content-Disposition header. stat is %d",
                  stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipContentDispositionHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Content-Disposition header - for example:
 *         "Content-Disposition: signal; handling=optional"
 *         - into a Content-Disposition header object. All the textual
 *         fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Content-Disposition header object.
 *    buffer    - Buffer containing a textual Content-Disposition header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderParse(
                           IN    RvSipContentDispositionHeaderHandle  hHeader,
                           IN    RvChar*                             buffer)
{
    MsgContentDispositionHeader* pHeader = (MsgContentDispositionHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContentDispositionHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_DISPOSITION,
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
 * RvSipContentDispositionHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ContentDisposition header value into an
 *          ContentDisposition header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value
 *          part as a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContentDispositionHeaderParse() function to
 *          parse strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ContentDisposition header object.
 *    buffer    - The buffer containing a textual ContentDisposition header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderParseValue(
                           IN    RvSipContentDispositionHeaderHandle  hHeader,
                           IN    RvChar*                             buffer)
{
    MsgContentDispositionHeader* pHeader = (MsgContentDispositionHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContentDispositionHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_DISPOSITION,
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
 * RvSipContentDispositionHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ContentDisposition header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderFix(
                                     IN RvSipContentDispositionHeaderHandle hHeader,
                                     IN RvChar*                            pFixedBuffer)
{
    MsgContentDispositionHeader* pHeader = (MsgContentDispositionHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipContentDispositionHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentDispositionHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentDispositionHeaderFix(hHeader=0x%p) - Failure in parsing header value",
            hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipContentDispositionHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-Disposition header
 *          object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentDisposition - The Content-Disposition header to take t
 *            he page from.
 ***************************************************************************/
HRPOOL SipContentDispositionHeaderGetPool(
                    RvSipContentDispositionHeaderHandle hContentDisposition)
{
    return ((MsgContentDispositionHeader*)hContentDisposition)->hPool;
}

/***************************************************************************
 * SipContentDispositionHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-Disposition header
 *          object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentDisposition - The Content-Disposition header to take
 *            the page from.
 ***************************************************************************/
HPAGE SipContentDispositionHeaderGetPage(
                    RvSipContentDispositionHeaderHandle hContentDisposition)
{
    return ((MsgContentDispositionHeader*)hContentDisposition)->hPage;
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Content-Disposition header fields are kept in a string
 *          format - for example, the Content-Disposition header parameter
 *          handling may be in a string format in case it's value is not
 *          enumerated by RvSipDispositionHandling.
 *          In order to get such a field from the Content-Disposition header
 *          object, your application should supply an adequate buffer to where
 *          the string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Disposition header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipContentDispositionHeaderGetStringLength(
                      IN  RvSipContentDispositionHeaderHandle     hHeader,
                      IN  RvSipContentDispositionHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgContentDispositionHeader* pHeader;

    pHeader = (MsgContentDispositionHeader*)hHeader;
    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_CONTENT_DISPOSITION_TYPE:
        {
            stringOffset = SipContentDispositionHeaderGetType(hHeader);
            break;
        }
        case RVSIP_CONTENT_DISPOSITION_HANDLING:
        {
            stringOffset = SipContentDispositionHeaderGetHandling(hHeader);
            break;
        }
        case RVSIP_CONTENT_DISPOSITION_OTHER_PARAMS:
        {
            stringOffset = SipContentDispositionHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_CONTENT_DISPOSITION_BAD_SYNTAX:
        {
            stringOffset = SipContentDispositionHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RvSipContentDispositionHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipContentDispositionHeaderGetPool(hHeader),
                             SipContentDispositionHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * SipContentDispositionHeaderGetType
 * ------------------------------------------------------------------------
 * General: This method retrieves the disposition type field.
 * Return Value: Disposition type string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetType(
                              IN RvSipContentDispositionHeaderHandle hHeader)
{
    return ((MsgContentDispositionHeader*)hHeader)->strType;
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetType
 * ------------------------------------------------------------------------
 * General: Gets the disposition type enumeration value.
 *          If RVSIP_DISPOSITIONTYPE_OTHER is returned, you can receive the
 *          disposition type string using
 *          RvSipContentDispositionHeaderGetTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipDispositionType RVCALLCONV RvSipContentDispositionHeaderGetType(
                            IN RvSipContentDispositionHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_DISPOSITIONTYPE_UNDEFINED;
    }
    return (((MsgContentDispositionHeader*)hHeader)->eType);
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetStrType
 * ------------------------------------------------------------------------
 * General: Copies the disposition type string from the Content-Disposition
 *          header into a given buffer. Use this function when the disposition
 *          type enumeration returned by RvSipContentDispositionGetType()
 *          is RVSIP_DISPOSITIONTYPE_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderGetStrType(
                              IN RvSipContentDispositionHeaderHandle hHeader,
                              IN RvChar*                            strBuffer,
                              IN RvUint                             bufferLen,
                              OUT RvUint*                           actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(
                            ((MsgContentDispositionHeader*)hHeader)->hPool,
                            ((MsgContentDispositionHeader*)hHeader)->hPage,
                            SipContentDispositionHeaderGetType(hHeader),
                            strBuffer,
                            bufferLen,
                            actualLen);
}

/***************************************************************************
 * SipContentDispositionHeaderGetHandling
 * ------------------------------------------------------------------------
 * General: This method retrieves the handling field.
 * Return Value: Handling string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetHandling(
                             IN RvSipContentDispositionHeaderHandle hHeader)
{
    return ((MsgContentDispositionHeader*)hHeader)->strHandling;
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetHandling
 * ------------------------------------------------------------------------
 * General: Gets the handling enumeration value. If
 *          RVSIP_DISPOSITIONHANDLING_OTHER is returned, you can receive the
 *          handling string using RvSipContentDispositionHeaderGetHandlingStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipDispositionHandling RVCALLCONV
                            RvSipContentDispositionHeaderGetHandling(
                               IN RvSipContentDispositionHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_DISPOSITIONHANDLING_UNDEFINED;
    }
    return (((MsgContentDispositionHeader*)hHeader)->eHandling);
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetStrHandling
 * ------------------------------------------------------------------------
 * General: Copies the handling string from the Content-Disposition header
 *          into a given buffer. Use this function when the handling
 *          enumeration returned by RvSipContentDispositionGetHandling() is
 *          RVSIP_DISPOSITIONHANDLING_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderGetStrHandling(
                            IN RvSipContentDispositionHeaderHandle   hHeader,
                            IN RvChar*                              strBuffer,
                            IN RvUint                               bufferLen,
                            OUT RvUint*                             actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(
                            ((MsgContentDispositionHeader*)hHeader)->hPool,
                            ((MsgContentDispositionHeader*)hHeader)->hPage,
                            SipContentDispositionHeaderGetHandling(hHeader),
                            strBuffer,
                            bufferLen,
                            actualLen);
}

/***************************************************************************
 * SipContentDispositionHeaderSetType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strType in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Disposition header object
 *            strType - The disposition type string to be set - if Null, the
 *                    exist disposition type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetType(
                             IN  RvSipContentDispositionHeaderHandle hHeader,
                             IN  RvChar *                           strType,
                             IN  HRPOOL                              hPool,
                             IN  HPAGE                               hPage,
                             IN  RvInt32                            strOffset)
{
    RvInt32                     newStr;
    MsgContentDispositionHeader* pHeader;
    RvStatus                    retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentDispositionHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strType = newStr;
#ifdef SIP_DEBUG
    pHeader->pType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strType);
#endif
    /* set appropriate enumeration  */
    if (UNDEFINED == newStr)
    {
        pHeader->eType = RVSIP_DISPOSITIONTYPE_UNDEFINED;
    }
    else
    {
        pHeader->eType = RVSIP_DISPOSITIONTYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentDispositionHeaderSetType
 * ------------------------------------------------------------------------
 * General: Sets the disposition type field in the Content-Disposition header
 *          object. If the enumeration given by eDispType is
 *          RVSIP_DISPOSITIONTYPE_OTHER, sets the disposition type string
 *          given by strDispType in the Content-Type header object. Use
 *          RVSIP_DISPOSITIONTYPE_OTHER when the disposition type you wish to
 *          set to the Content-Type does not have a matching enumeration value
 *          in the RvSipDispositionType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Disposition header object.
 *    eDispType  - The disposition type enumeration to be set in the
 *               Content-Type header object. If RVSIP_DISPOSITIONTYPE_UNDEFINED
 *               is supplied, the existing disposition type is removed from the
 *               header.
 *    strDispType - The disposition type string to be set in the Content-Type
 *                header. (relevant when eDispType is RVSIP_DISPOSITIONTYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderSetType(
                    IN  RvSipContentDispositionHeaderHandle hHeader,
                    IN  RvSipDispositionType                eDispType,
                    IN  RvChar                            *strDispType)
{
    MsgContentDispositionHeader *pHeader;
    RvStatus                    stat = RV_OK;
    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentDispositionHeader *)hHeader;

    pHeader->eType = eDispType;
    if (RVSIP_DISPOSITIONTYPE_OTHER == eDispType)
    {
        stat = SipContentDispositionHeaderSetType(hHeader, strDispType, NULL,
                                                  NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_DISPOSITIONTYPE_UNDEFINED == eDispType)
    {
        pHeader->strType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipContentDispositionHeaderSetHandling
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strHandling in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Disposition header object
 *            strHandling  - The handling string to be set - if Null,
 *                     the exist handling type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetHandling(
                          IN  RvSipContentDispositionHeaderHandle hHeader,
                          IN  RvChar *                           strHandling,
                          IN  HRPOOL                              hPool,
                          IN  HPAGE                               hPage,
                          IN  RvInt32                            strOffset)
{
    RvInt32                     newStr;
    MsgContentDispositionHeader* pHeader;
    RvStatus                    retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentDispositionHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strHandling,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strHandling = newStr;
#ifdef SIP_DEBUG
    pHeader->pHandling = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                      pHeader->strHandling);
#endif
    /* set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eHandling = RVSIP_DISPOSITIONHANDLING_UNDEFINED;
    }
    else
    {
        pHeader->eHandling = RVSIP_DISPOSITIONHANDLING_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentDispositionHeaderSetHandling
 * ------------------------------------------------------------------------
 * General: Sets the handling field in the Content-Disposition header
 *          object. If the enumeration given by eHandling is
 *          RVSIP_DISPOSITIONHANDLING_OTHER, sets the handling string
 *          given by strHandling in the Content-Type header object. Use
 *          RVSIP_DISPOSITIONHANDLING_OTHER when the disposition type you wish
 *          to set to the Content-Type does not have a matching enumeration
 *          value in the RvSipDispositionHandling enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Disposition header object.
 *    eHandling  - The handling enumeration to be set in the Content-Type header
 *               object. If RVSIP_DISPOSITIONHANDLING_UNDEFINED is supplied,
 *               the existing handling is removed from the header.
 *    strHandling - The handling string to be set in the Content-Type header.
 *                (relevant when eHandling is RVSIP_DISPOSITIONHANDLING_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderSetHandling(
                        IN  RvSipContentDispositionHeaderHandle hHeader,
                        IN  RvSipDispositionHandling            eHandling,
                        IN  RvChar                            *strHandling)
{
    MsgContentDispositionHeader *pHeader;
    RvStatus                    stat = RV_OK;
    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentDispositionHeader *)hHeader;

    pHeader->eHandling = eHandling;
    if (RVSIP_DISPOSITIONHANDLING_OTHER == eHandling)
    {
        stat = SipContentDispositionHeaderSetHandling(hHeader, strHandling, NULL,
                                                      NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_DISPOSITIONHANDLING_UNDEFINED == eHandling)
    {
        pHeader->strHandling = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pHandling = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipContentDispositionHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetOtherParams(
                            IN RvSipContentDispositionHeaderHandle hHeader)
{
    return ((MsgContentDispositionHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other parameters string from the Content-Disposition
 *          header into a given buffer.
 *          Not all the Content-Disposition header parameters have separated
 *          fields in the Content-Disposition header object. Parameters with no
 *          specific fields are refered to as other params. They are kept in
 *          the object in one concatenated string in the form-
 *          "name=value;name=value..."
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
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderGetOtherParams(
                            IN RvSipContentDispositionHeaderHandle   hHeader,
                            IN RvChar*                              strBuffer,
                            IN RvUint                               bufferLen,
                            OUT RvUint*                             actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentDispositionHeader*)hHeader)->hPool,
                                  ((MsgContentDispositionHeader*)hHeader)->hPage,
                                  SipContentDispositionHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentDispositionHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the Content-Disposition header object
 *          strOtherParams - The OtherParams string to be set - if Null,
 *                     the exist strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetOtherParams(
                      IN    RvSipContentDispositionHeaderHandle hHeader,
                      IN    RvChar *                           strOtherParams,
                      IN    HRPOOL                              hPool,
                      IN    HPAGE                               hPage,
                      IN    RvInt32                            strOffset)
{
    RvInt32                     newStr;
    MsgContentDispositionHeader* pHeader;
    RvStatus                    retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentDispositionHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strOtherParams,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
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
 * RvSipContentDispositionHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other parameters string in the Content-Disposition header
 *          object.
 *          Not all the Content-Disposition header parameters have separated
 *          fields in the Content-Disposition header object. Parameters with no
 *          specific fields are refered to as other params. They are kept in
 *          the object in one concatenated string in the form-
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    otherParams - The other parameters string to be set in the
 *                Content-Disposition header. If NULL is supplied, the existing
 *                other parameters string is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderSetOtherParams(
                          IN   RvSipContentDispositionHeaderHandle  hHeader,
                          IN   RvChar                             *otherParams)
{
    /* validity checking will be done in the internal function */
    return SipContentDispositionHeaderSetOtherParams(
                                              hHeader, otherParams, NULL,
                                              NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContentDispositionHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetStrBadSyntax(
                            IN RvSipContentDispositionHeaderHandle hHeader)
{
    return ((MsgContentDispositionHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipContentDispositionHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ContentDisposition header,
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
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderGetStrBadSyntax(
                            IN RvSipContentDispositionHeaderHandle   hHeader,
                            IN RvChar*                              strBuffer,
                            IN RvUint                               bufferLen,
                            OUT RvUint*                             actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentDispositionHeader*)hHeader)->hPool,
                                  ((MsgContentDispositionHeader*)hHeader)->hPage,
                                  SipContentDispositionHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentDispositionHeaderSetStrBadSyntax
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
RvStatus SipContentDispositionHeaderSetStrBadSyntax(
                                  IN RvSipContentDispositionHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgContentDispositionHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgContentDispositionHeader*)hHeader;

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
 * RvSipContentDispositionHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal ContentDisposition header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentDispositionHeaderSetStrBadSyntax(
                                  IN RvSipContentDispositionHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipContentDispositionHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ContentDispositionHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ContentDispositionHeaderClean( IN MsgContentDispositionHeader* pHeader,
                                           IN RvBool                      bCleanBS)
{
    pHeader->eHandling       = RVSIP_DISPOSITIONHANDLING_UNDEFINED;
    pHeader->eType           = RVSIP_DISPOSITIONTYPE_UNDEFINED;
    pHeader->strHandling     = UNDEFINED;
    pHeader->strOtherParams  = UNDEFINED;
    pHeader->strType         = UNDEFINED;
    pHeader->eHeaderType     = RVSIP_HEADERTYPE_CONTENT_DISPOSITION;

#ifdef SIP_DEBUG
    pHeader->pHandling       = NULL;
    pHeader->pOtherParams    = NULL;
    pHeader->pType           = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif



