
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReferToHeader.c                                   *
 *                                                                            *
 * The file defines the methods of the Refer-To header object:                *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Apr.2001                                             *
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

#include "RvSipReplacesHeader.h"
#include "RvSipReferToHeader.h"
#include "_SipReferToHeader.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipAddress.h"
#include "_SipReplacesHeader.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus UpdateReplacesHeader(IN RvSipReferToHeaderHandle   hHeader);

static void ReferToHeaderClean(IN MsgReferToHeader*    pHeader,
                               IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReferToHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Refer-To header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Refer-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipReferToHeaderHandle* hHeader)
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

    stat = RvSipReferToHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_REFER_TO,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipReferToHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Refer-To Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Refer-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderConstruct(
                                        IN  RvSipMsgMgrHandle         hMsgMgr,
                                        IN  HRPOOL                    hPool,
                                        IN  HPAGE                     hPage,
                                        OUT RvSipReferToHeaderHandle* hHeader)
{
    MsgReferToHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgReferToHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                  hPool,
                                                  hPage,
                                                  sizeof(MsgReferToHeader),
                                                  RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReferToHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr         = pMsgMgr;
    pHeader->hPage           = hPage;
    pHeader->hPool           = hPool;
    ReferToHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipReferToHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReferToHeaderConstruct - Refer-To header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipReferToHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Refer-To header object to a
 *          destination Refer-To header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Refer-To header object.
 *    hSource      - Handle to the source Refer-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderCopy(
                                    INOUT  RvSipReferToHeaderHandle hDestination,
                                    IN     RvSipReferToHeaderHandle hSource)
{
    RvStatus         stat;
    MsgReferToHeader* source;
    MsgReferToHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgReferToHeader*)hSource;
    dest   = (MsgReferToHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->bIsCompact = source->bIsCompact;

    /* copy AddrSpec */
    stat = RvSipReferToHeaderSetAddrSpec(hDestination, source->hAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipReferToHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* copy display name */
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
                      "RvSipReferToHeaderCopy - Failed to copy displayName. hDest 0x%p",
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

    /* copy replaces header */
    if(source->hReplacesHeader != NULL)
    {
        stat = RvSipReferToHeaderSetReplacesHeader(hDestination, source->hReplacesHeader);
        if(stat != RV_OK)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReferToHeaderCopy: Failed to copy replaces header. hDest 0x%p, stat %d",
                hDestination, stat));
            return stat;
        }
    }
    else
    {
        dest->hReplacesHeader = NULL;
    }

    /* copy other param */
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
                "RvSipReferToHeaderCopy - Failed to copy other params. hDest 0x%p",
                hDestination));
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
                "RvSipReferToHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipReferToHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the ReferTo header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetCompactForm(
                                   IN    RvSipReferToHeaderHandle hHeader,
                                   IN    RvBool                  bIsCompact)
{
    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipReferToHeaderSetCompactForm - Setting compact form of Header 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}


/***************************************************************************
 * RvSipReferToHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the ReferTo header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderGetCompactForm(
                                   IN    RvSipReferToHeaderHandle hHeader,
                                   IN    RvBool                *pbIsCompact)
{
    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	*pbIsCompact = pHeader->bIsCompact;
	return RV_OK;


}



/***************************************************************************
 * RvSipReferToHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Refer-To header object to a textual Refer-To header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Refer-To header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderEncode(
                                        IN    RvSipReferToHeaderHandle hHeader,
                                        IN    HRPOOL                   hPool,
                                        OUT   HPAGE*                   hPage,
                                        OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;

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
                "RvSipReferToHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferToHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ReferToHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, RV_FALSE,length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferToHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ReferToHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ReferToHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Refer-To header (as string) on it.
 *          Refer-To Header = ("Refer-To" | "r") ":" URL
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Refer-To header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderEncode(
                            IN    RvSipReferToHeaderHandle hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                   bInUrlHeaders,
                            IN    RvBool                   bForceCompactForm,
                            INOUT RvUint32*                length)
{
    MsgReferToHeader*   pHeader;
    RvStatus           stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgReferToHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ReferToHeaderEncode - Encoding Refer-To header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "Refer-To" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "r", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Refer-To", length);
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReferToHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "ReferToHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
        stat = AddrEncodeWithReplaces(pHeader->hAddrSpec, pHeader->hReplacesHeader, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
            return stat;
    }
    else
    {
        return RV_ERROR_UNKNOWN;
    }

    /* other parmams */
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
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReferToHeaderEncode: Failed to encode Refer-To header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipReferToHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Refer-To header-for example,
 *         "Refer-To: sip:charlie@caller.com"-into a Refer-To header object.
 *         All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Refer-To header object.
 *    buffer    - Buffer containing a textual Refer-To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderParse(
                                     IN  RvSipReferToHeaderHandle  hHeader,
                                     IN  RvChar                   *buffer)
{
    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ReferToHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REFER_TO,
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
 * RvSipReferToHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReferTo header value into an ReferTo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value
 *          part as a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReferToHeaderParse() function to parse strings
 *          that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ReferTo header object.
 *    buffer    - The buffer containing a textual ReferTo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderParseValue(
                                     IN  RvSipReferToHeaderHandle  hHeader,
                                     IN  RvChar                   *buffer)
{
    MsgReferToHeader*    pHeader = (MsgReferToHeader*)hHeader;
	RvBool               bIsCompact;
    RvStatus             rv;
	
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    ReferToHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REFER_TO,
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
 * RvSipReferToHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ReferTo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderFix(
                                     IN RvSipReferToHeaderHandle hHeader,
                                     IN RvChar                  *pFixedBuffer)
{
    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipReferToHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReferToHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReferToHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipReferToHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Refer-To header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReferTo - The Refer-To header to take the page from.
 ***************************************************************************/
HRPOOL SipReferToHeaderGetPool(RvSipReferToHeaderHandle hReferTo)
{
    return ((MsgReferToHeader*)hReferTo)->hPool;
}

/***************************************************************************
 * SipReferToHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Refer-To header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReferTo - The Refer-To header to take the page from.
 ***************************************************************************/
HPAGE SipReferToHeaderGetPage(RvSipReferToHeaderHandle hReferTo)
{
    return ((MsgReferToHeader*)hReferTo)->hPage;
}

/***************************************************************************
 * RvSipReferToHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Refer-To header fields are kept in a string format-
 *          for example, the Refer-To header display name. In order to get
 *          such a field from the Refer-To header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to
 *          enable you to allocate an appropriate buffer size before calling
 *          the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Refer-To header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReferToHeaderGetStringLength(
                                IN  RvSipReferToHeaderHandle     hHeader,
                                IN  RvSipReferToHeaderStringName stringName)
{
    RvInt32 stringOffset;

    MsgReferToHeader* pHeader = (MsgReferToHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
        case RVSIP_REFER_TO_DISPLAY_NAME:
        {
            stringOffset = SipReferToHeaderGetDisplayName(hHeader);
            break;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        case RVSIP_REFER_TO_OTHER_PARAMS:
        {
            stringOffset = SipReferToHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_REFER_TO_BAD_SYNTAX:
        {
            stringOffset = SipReferToHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferToHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipReferToHeaderGetPool(hHeader),
                             SipReferToHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReferToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Refer-To header object
 ***************************************************************************/
RvInt32 SipReferToHeaderGetDisplayName(IN RvSipReferToHeaderHandle hHeader)
{
    return ((MsgReferToHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipReferToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the Refer-To header into a given buffer.
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
 * output:actualLen - The length of the requested parameter, + 1 to include a
 *                    NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderGetDisplayName(
                                              IN RvSipReferToHeaderHandle hHeader,
                                              IN RvChar*                  strBuffer,
                                              IN RvUint                   bufferLen,
                                              OUT RvUint*                 actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferToHeader*)hHeader)->hPool,
                                  ((MsgReferToHeader*)hHeader)->hPage,
                                  SipReferToHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReferToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          ReferToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Refer-To header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipReferToHeaderSetDisplayName(
                                       IN    RvSipReferToHeaderHandle hHeader,
                                       IN    RvChar *                strDisplayName,
                                       IN    HRPOOL                   hPool,
                                       IN    HPAGE                    hPage,
                                       IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgReferToHeader* pHeader;
    RvStatus         retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferToHeader*)hHeader;

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
 * RvSipReferToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the ReferTo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the header object.
 *    strDisplayName - The display name to be set in the Refer-To header. If NULL
 *                   is supplied, the existing display name is removed from the
 *                   header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetDisplayName(
                                         IN    RvSipReferToHeaderHandle hHeader,
                                         IN    RvChar *                strDisplayName)
{
    /* validity checking will be done in the internal function */
    return SipReferToHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipReferToHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the Refer-To header object as
 *          an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Refer-To header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipReferToHeaderGetAddrSpec(
                                            IN RvSipReferToHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgReferToHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipReferToHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec address object in the Refer-To header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Refer-To header object.
 *  hAddrSpec - Handle to the Address Spec address object to be set in the
 *              Refer-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetAddrSpec(
                                         IN  RvSipReferToHeaderHandle hHeader,
                                         IN  RvSipAddressHandle       hAddrSpec)
{
    RvStatus           stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgReferToHeader   *pHeader         = (MsgReferToHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hAddrSpec == NULL)
    {
        pHeader->hAddrSpec = NULL;
        return RV_OK;
    }
    else
    {
        /*first check if the given address was constructed on the header page
        and if so attach it*/
        if( SipAddrGetPool(hAddrSpec) == pHeader->hPool &&
            SipAddrGetPage(hAddrSpec) == pHeader->hPage)
        {
            pHeader->hAddrSpec = hAddrSpec;
            return RV_OK;
        }

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
           (pAddr->eAddrType != currentAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInReferToHeader(hHeader,
                                                     pAddr->eAddrType,
                                                     &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }

        stat = RvSipAddrCopy(pHeader->hAddrSpec,
                             hAddrSpec);

        return stat;
    }
}

/***************************************************************************
 * SipReferToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the header OtherParams.
 * Return Value: Offset of other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReferTo header object
 ***************************************************************************/
RvInt32 SipReferToHeaderGetOtherParams(IN RvSipReferToHeaderHandle hHeader)
{
    return ((MsgReferToHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipReferToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other params field of the ReferTo header
 *          object into a given buffer.
 *          generic-parameters of refer-to header are refered to as other params.
 *          It is kept in the object in one concatenated string in the form -
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the ReferTo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a
 *                     NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderGetOtherParams(
                                               IN RvSipReferToHeaderHandle hHeader,
                                               IN RvChar*                  strBuffer,
                                               IN RvUint                   bufferLen,
                                               OUT RvUint*                 actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferToHeader*)hHeader)->hPool,
                                  ((MsgReferToHeader*)hHeader)->hPage,
                                  SipReferToHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReferToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          ReferToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the ReferTo header object
 *          strOtherParams - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool          - The pool on which the string lays (if relevant).
 *          hPage          - The page on which the string lays (if relevant).
 *          strOffset      - the offset of the other-params string.
 ***************************************************************************/
RvStatus SipReferToHeaderSetOtherParams(IN    RvSipReferToHeaderHandle hHeader,
                                        IN    RvChar *                 strOtherParams,
                                        IN    HRPOOL                   hPool,
                                        IN    HPAGE                    hPage,
                                        IN    RvInt32                  strOtherOffset)
{
    RvInt32        newStr;
    MsgReferToHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferToHeader*)hHeader;

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
 * RvSipReferToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the ReferTo header object.
 *          generic-parameters of refer-to header are refered to as other params.
 *          It is kept in the object in one concatenated string in the form -
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the ReferTo header object.
 *    strOtherParams - The Other Params string to be set in the ReferTo header.
 *                   If NULL is supplied, the existing Other Params field is
 *                   removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetOtherParams
                                    (IN    RvSipReferToHeaderHandle  hHeader,
                                     IN    RvChar                   *strOtherParams)
{
    /* validity checking will be done in the internal function */
    return SipReferToHeaderSetOtherParams(hHeader, strOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipReferToHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReferToHeaderGetStrBadSyntax(
                                    IN RvSipReferToHeaderHandle hHeader)
{
    return ((MsgReferToHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipReferToHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ReferTo header,
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
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderGetStrBadSyntax(
                                               IN  RvSipReferToHeaderHandle hHeader,
                                               IN  RvChar*                  strBuffer,
                                               IN  RvUint                   bufferLen,
                                               OUT RvUint*                  actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferToHeader*)hHeader)->hPool,
                                  ((MsgReferToHeader*)hHeader)->hPage,
                                  SipReferToHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReferToHeaderSetStrBadSyntax
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
RvStatus SipReferToHeaderSetStrBadSyntax(
                                  IN RvSipReferToHeaderHandle hHeader,
                                  IN RvChar                  *strBadSyntax,
                                  IN HRPOOL                   hPool,
                                  IN HPAGE                    hPage,
                                  IN RvInt32                  strBadSyntaxOffset)
{
    MsgReferToHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgReferToHeader*)hHeader;

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
 * RvSipReferToHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal ReferTo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetStrBadSyntax(
                                  IN RvSipReferToHeaderHandle  hHeader,
                                  IN RvChar                   *strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReferToHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipReferToHeaderGetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Gets the replaces header from the Refer-To header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader            - Handle to the Refer-To header object.
 *  Output: hReplacesHeader - Pointer to the handle of the returned Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderGetReplacesHeader(
                                            IN  RvSipReferToHeaderHandle   hHeader,
                                            OUT RvSipReplacesHeaderHandle *hReplacesHeader)
{
    RvStatus rv = RV_OK;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(((MsgReferToHeader *)hHeader)->hReplacesHeader == NULL)
    {
        rv = UpdateReplacesHeader(hHeader);
        if(rv != RV_OK)
        {
            return rv;
        }
    }
    *hReplacesHeader = ((MsgReferToHeader *)hHeader)->hReplacesHeader;

    return RV_OK;
}

/***************************************************************************
 * RvSipReferToHeaderSetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Sets the replaces header in the Refer-To header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 *  hReplacesHeader - Handle to the Replaces header to be set in the
 *                      Refer-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferToHeaderSetReplacesHeader(
                               IN  RvSipReferToHeaderHandle  hHeader,
                               IN  RvSipReplacesHeaderHandle hReplacesHeader)
{
    MsgReferToHeader *pReferToHeader;
    RvSipReplacesHeaderHandle hNewReplacesHeader;
    RvStatus         rv = RV_OK;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pReferToHeader = (MsgReferToHeader *)hHeader;

    rv = SipReferToRemoveOldReplacesFromHeadersList(hHeader);
    if(rv != RV_OK)
    {
        return rv;
    }
    if(hReplacesHeader == NULL)
    {
        pReferToHeader->hReplacesHeader = NULL;
        return RV_OK;
    }
    /*first check if the given address was constructed on the header page
      and if so attach it*/
    if(SipReplacesHeaderGetPool(hReplacesHeader) == pReferToHeader->hPool &&
       SipReplacesHeaderGetPage(hReplacesHeader) == pReferToHeader->hPage)
    {
        pReferToHeader->hReplacesHeader = hReplacesHeader;
        return RV_OK;
    }

    /* if there is no Replaces header object in the Refer-To, we will allocate one */
    if(pReferToHeader->hReplacesHeader == NULL)
    {
        if(RvSipReplacesHeaderConstructInReferToHeader(hHeader, &hNewReplacesHeader) != RV_OK)
        {
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        hNewReplacesHeader = pReferToHeader->hReplacesHeader;
    }
    return RvSipReplacesHeaderCopy(hNewReplacesHeader, hReplacesHeader);
}

/***************************************************************************
 * SipReferToHeaderGetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Gets the replaces header from the Refer-To header object.
 *          This is an internal function, the function just returns the
 *          replaces header, without checking the url headers parameter,
 *
 * Return Value: Returns hReplacesHeader.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader            - Handle to the Refer-To header object.
 ***************************************************************************/
RvSipReplacesHeaderHandle RVCALLCONV SipReferToHeaderGetReplacesHeader(
                                    IN  RvSipReferToHeaderHandle   hHeader)
{
    return ((MsgReferToHeader *)hHeader)->hReplacesHeader;
}
/***************************************************************************
 * SipReferToHeaderSetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Sets the replaces header in the Refer-To header object.
 *          This is an internal function, the function just copy the
 *          replaces header, without checking the url headers parameter,
 *          whoever uses this function, must remove the old replaces from
 *          the url headers parameter.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 *  hReplacesHeader - Handle to the Replaces header to be set in the
 *                      Refer-To header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipReferToHeaderSetReplacesHeader(
                               IN  RvSipReferToHeaderHandle  hHeader,
                               IN  RvSipReplacesHeaderHandle hReplacesHeader)
{
    MsgReferToHeader *pReferToHeader;
    RvSipReplacesHeaderHandle hNewReplacesHeader;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pReferToHeader = (MsgReferToHeader *)hHeader;

    if(hReplacesHeader == NULL)
    {
        pReferToHeader->hReplacesHeader = NULL;
        return RV_OK;
    }
    /*first check if the given address was constructed on the header page
      and if so attach it*/
    if(SipReplacesHeaderGetPool(hReplacesHeader) == pReferToHeader->hPool &&
       SipReplacesHeaderGetPage(hReplacesHeader) == pReferToHeader->hPage)
    {
        pReferToHeader->hReplacesHeader = hReplacesHeader;
        return RV_OK;
    }

    /* if there is no Replaces header object in the Refer-To, we will allocate one */
    if(pReferToHeader->hReplacesHeader == NULL)
    {
        if(SipReplacesHeaderConstructInReferToHeader(hHeader, &hNewReplacesHeader) != RV_OK)
        {
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        hNewReplacesHeader = pReferToHeader->hReplacesHeader;
    }
    return RvSipReplacesHeaderCopy(hNewReplacesHeader, hReplacesHeader);
}

/***************************************************************************
 * UpdateReplacesHeader
 * ------------------------------------------------------------------------
 * General: Update the Replaces header in the Refer-To header from the
 *          headers in the addr spec.
 * Return Value: Returns RV_OK if updated correctly, or no replaces
 *              header exists in the refer-to url.
 *              else - Failure when updating.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 ***************************************************************************/
static RvStatus UpdateReplacesHeader(IN RvSipReferToHeaderHandle   hHeader)
{
    MsgReferToHeader   *pHeader;
    MsgAddress *pAddr;
    MsgAddrUrl *pUrl;
    RvStatus   rv = RV_OK;
    RvInt32    headerOffset;
    RvInt32    length;
    RvInt32    safeCounter = 0;
    RvBool     findChar = RV_TRUE;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgReferToHeader *)hHeader;

    if(pHeader->hAddrSpec == NULL)
    {
        return RV_OK;
    }

   pAddr = (MsgAddress*)pHeader->hAddrSpec;
   if(pAddr->uAddrBody.hUrl == NULL)
   {
       return RV_OK;
   }
   pUrl = (MsgAddrUrl *)pAddr->uAddrBody.hUrl;
   if(pUrl->strHeaders == UNDEFINED)
   {
       return RV_OK;
   }


   headerOffset = pUrl->strHeaders;
   while(findChar == RV_TRUE && safeCounter < 1000)
   {
       if(RPOOL_CmpiPrefixToExternal(pAddr->hPool, pAddr->hPage, headerOffset, "Replaces=") == RV_TRUE)
       {
            if(pHeader->hReplacesHeader == NULL)
            {
                rv = RvSipReplacesHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr, pHeader->hPool, pHeader->hPage,
                                                        &(pHeader->hReplacesHeader));
                if(rv != RV_OK)
                {
                    return rv;
                }
            }
            rv = SipReplacesHeaderParseFromAddrSpec(pAddr->hPool,
                                                    pAddr->hPage,
                                                    headerOffset,
                                                    &(pHeader->hReplacesHeader));
            if(rv != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "UpdateReplacesHeader - SipReplacesHeaderParseFromAddrSpec failed - Replaces header syntax in the Refer-To header is not valid"));
            }
            return rv;
       }
       length = RPOOL_Strlen(pAddr->hPool, pAddr->hPage, headerOffset);
       if(length <= 0)
       {
           return RV_OK;
       }
       findChar = MsgUtilsFindCharInPage(pAddr->hPool, pAddr->hPage, headerOffset,
                                        length, '&', &headerOffset);
       safeCounter++;

   }
   return RV_OK;
}

/***************************************************************************
 * SipReferToRemoveOldReplacesFromHeadersList
 * ------------------------------------------------------------------------
 * General: This function is called from the 'ReferToSetReplaces' function. It checks if the
 *          headers in the address spec has Replaces header and removes the Replaces header from
 *          the address spec if it exists.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 ***************************************************************************/
RvStatus SipReferToRemoveOldReplacesFromHeadersList(IN RvSipReferToHeaderHandle  hHeader)
{
    MsgAddress *pAddr;
    MsgAddrUrl *pUrl;
    RvStatus   rv = RV_OK;
    RvInt32    headerOffset, length = 0;
    RvBool     findChar = RV_TRUE;
    RvBool     replaesOnlyHeader = RV_TRUE;
    RvInt32    safeCounter = 0;
    MsgReferToHeader *pHeader = (MsgReferToHeader *)hHeader;

    if(pHeader->hAddrSpec == NULL)
    {
        return RV_OK;
    }

   pAddr = (MsgAddress*)pHeader->hAddrSpec;
   if(pAddr->uAddrBody.hUrl == NULL)
   {
       return RV_OK;
   }
   pUrl = (MsgAddrUrl *)pAddr->uAddrBody.hUrl;
   if(pUrl->strHeaders == UNDEFINED)
   {
       return RV_OK;
   }
   headerOffset = pUrl->strHeaders;
   while(findChar == RV_TRUE && safeCounter < 1000)
   {
       if(RPOOL_CmpiPrefixToExternal(pAddr->hPool, pAddr->hPage, headerOffset, "Replaces=") == RV_TRUE)
       {
            RvInt32 newOffset, size;
            findChar = MsgUtilsFindCharInPage(pAddr->hPool, pAddr->hPage, headerOffset,
                                        length, '&', &newOffset);
            if(findChar == RV_FALSE)
            {
                if(replaesOnlyHeader == RV_TRUE)
                {
                    pUrl->strHeaders = UNDEFINED;
                }
                else
                {
                    rv = RPOOL_CopyFromExternal(pAddr->hPool, pAddr->hPage, headerOffset-1, "\0", 1);
                }
                return rv;
            }
            else
            {
                size = RPOOL_Strlen(pAddr->hPool, pAddr->hPage, newOffset);
                if(rv != RV_OK)
                {
                    return rv;
                }
                return RPOOL_CopyFrom(pAddr->hPool, pAddr->hPage, headerOffset,
                                        pAddr->hPool, pAddr->hPage, newOffset, size);
            }
       }
       length = RPOOL_Strlen(pAddr->hPool, pAddr->hPage, headerOffset);
       if(length <= 0)
       {
           return RV_OK;
       }
       findChar = MsgUtilsFindCharInPage(pAddr->hPool, pAddr->hPage, headerOffset,
                                        length, '&', &headerOffset);
       replaesOnlyHeader = RV_FALSE;
       safeCounter++;

   }
   return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ReferToHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ReferToHeaderClean(IN MsgReferToHeader*    pHeader,
                               IN RvBool               bCleanBS)
{
    pHeader->hAddrSpec       = NULL;
    pHeader->strOtherParams  = UNDEFINED;
    pHeader->eHeaderType     = RVSIP_HEADERTYPE_REFER_TO;
    pHeader->hReplacesHeader = NULL;
#ifndef RV_SIP_JSR32_SUPPORT
	pHeader->strDisplayName  = UNDEFINED;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

#ifdef SIP_DEBUG
    pHeader->pOtherParams    = NULL;
#ifndef RV_SIP_JSR32_SUPPORT
    pHeader->pDisplayName    = NULL;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
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

#endif /* #ifndef RV_SIP_PRIMITIVES*/
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef __cplusplus
}
#endif


