
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReferredByHeader.c                                *
 *                                                                            *
 * The file defines the methods of the Referred-By header object              *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Apr.2001                                              *
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
#include "RvSipReferredByHeader.h"
#include "_SipReferredByHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ReferredByHeaderClean(IN MsgReferredByHeader*   pHeader,
								  IN RvBool                 bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipReferredByHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Referred-By header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Referred-By header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderConstructInMsg(
                                       IN  RvSipMsgHandle               hSipMsg,
                                       IN  RvBool                       pushHeaderAtHead,
                                       OUT RvSipReferredByHeaderHandle* hHeader)
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

    stat = RvSipReferredByHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_REFERRED_BY,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipReferredByHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Referred-By Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Referred-By header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderConstruct(
                                        IN  RvSipMsgMgrHandle            hMsgMgr,
                                        IN  HRPOOL                       hPool,
                                        IN  HPAGE                        hPage,
                                        OUT RvSipReferredByHeaderHandle* hHeader)
{
    MsgReferredByHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgReferredByHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgReferredByHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReferredByHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }


    pHeader->pMsgMgr                = pMsgMgr;
    pHeader->hPage                  = hPage;
    pHeader->hPool                  = hPool;

    ReferredByHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipReferredByHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReferredByHeaderConstruct - Referred-By header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipReferredByHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Referred-By header object to a
 *          destination Referred-By header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Referred-By header object.
 *    hSource      - Handle to the source Referred-By header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderCopy(
                                     INOUT RvSipReferredByHeaderHandle hDestination,
                                     IN    RvSipReferredByHeaderHandle hSource)
{

    RvStatus            stat;
    MsgReferredByHeader* source;
    MsgReferredByHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgReferredByHeader*)hSource;
    dest   = (MsgReferredByHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    dest->bIsCompact = source->bIsCompact;
    /* referrer-url */

    /* address-spec */
    stat = RvSipReferredByHeaderSetAddrSpec(hDestination,
                                            source->hReferrerAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipReferredByHeaderCopy: Failed to copy referrer-url. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* display name */
    if(source->strReferrerDisplayName > UNDEFINED)
    {
        dest->strReferrerDisplayName = MsgUtilsAllocCopyRpoolString(
                                                            source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strReferrerDisplayName);
        if (dest->strReferrerDisplayName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderCopy - Failed to copy displayName. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pReferrerDisplayName = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                                      dest->strReferrerDisplayName);
#endif
    }
    else
    {
        dest->strReferrerDisplayName = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pReferrerDisplayName = NULL;
#endif
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    /* cid param */
    if(source->strCidParam > UNDEFINED)
    {
        dest->strCidParam = MsgUtilsAllocCopyRpoolString(
                                                    source->pMsgMgr,
                                                    dest->hPool,
                                                    dest->hPage,
                                                    source->hPool,
                                                    source->hPage,
                                                    source->strCidParam);
        if (dest->strCidParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderCopy - Failed to copy cid param. stat %d",
                stat));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCidParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                                  dest->strCidParam);
#endif
    }
    else
    {
        dest->strCidParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCidParam = NULL;
#endif
    }
    /* other params */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(
                                                    source->pMsgMgr,
                                                    dest->hPool,
                                                    dest->hPage,
                                                    source->hPool,
                                                    source->hPage,
                                                    source->strOtherParams);
        if (dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderCopy - Failed to copy other params. stat %d",
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
                "RvSipReferredByHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipReferredByHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Referred-By header object to a textual Referred-By header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Referred-By header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderEncode(
                                       IN    RvSipReferredByHeaderHandle hHeader,
                                       IN    HRPOOL                      hPool,
                                       OUT   HPAGE*                      hPage,
                                       OUT   RvUint32*                   length)
{
    RvStatus stat;
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;

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
                "RvSipReferredByHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ReferredByHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ReferredByHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ReferredByHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Referred-By header (as string) on it.
 *          referred-By Header = ("Referred-By" | "b") ":" (name-addr|addr-spec)
 *                                          (referenced-url |
 *                                           (referenced-url ";" ref-signature) |
 *                                           (ref-signature ";" referenced-url))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Referred-By header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output:length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReferredByHeaderEncode(
                                 IN    RvSipReferredByHeaderHandle hHeader,
                                 IN    HRPOOL                      hPool,
                                 IN    HPAGE                       hPage,
                                 IN    RvBool                      bInUrlHeaders,
                                 IN    RvBool                      bForceCompactForm,
                                 INOUT RvUint32*                   length)
{
    MsgReferredByHeader*   pHeader;
    RvStatus              stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgReferredByHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ReferredByHeaderEncode - Encoding Referred-By header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "Referred-By" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "b", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Referred-By", length);
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReferredByHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "ReferredByHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* set referrer-url [displayName]"<"spec ">"*/
    if(pHeader->strReferrerDisplayName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strReferrerDisplayName,
                                    length);
        if (stat != RV_OK)
            return stat;
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    if (pHeader->hReferrerAddrSpec != NULL)
    {
        /* set spec */
        stat = AddrEncode(pHeader->hReferrerAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
            return stat;
    }
    else
        return RV_ERROR_UNKNOWN;

    /* set cid param with " ;" at the begining*/
    if(pHeader->strCidParam > UNDEFINED)
    {
        /* set " ;cid=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "cid", length);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        /* set pHeader->strSignatureParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCidParam,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReferredByHeaderEncode: Failed to encode cid param of Referred-By header. stat is %d",
                stat));
        }
    }

    /* set generic params with " ;" at the begining*/
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set " ;" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        /* set pHeader->strSignatureParams */
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
            "ReferredByHeaderEncode: Failed to encode other params of Referred-By header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipReferredByHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Referred-By header-for example, "Referred-by:
 *         <sip:charlie@caller.com>;ref=<sip:me@com>"-into a Referred-By
 *         header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Referred-By header object.
 *    buffer    - Buffer containing a textual ReferredBy header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderParse(
                                     IN    RvSipReferredByHeaderHandle hHeader,
                                     IN    RvChar                     *buffer)
{
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ReferredByHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REFERRED_BY,
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
 * RvSipReferredByHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReferredBy header value into an ReferredBy
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReferredByHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ReferredBy header object.
 *    buffer    - The buffer containing a textual ReferredBy header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderParseValue(
                                     IN    RvSipReferredByHeaderHandle hHeader,
                                     IN    RvChar                     *buffer)
{
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;
	RvBool               bIsCompact;
    RvStatus             rv;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    ReferredByHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REFERRED_BY,
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
 * RvSipReferredByHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ReferredBy header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderFix(
                                     IN RvSipReferredByHeaderHandle hHeader,
                                     IN RvChar                     *pFixedBuffer)
{
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipReferredByHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReferredByHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReferredByHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipReferredByHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Referred-By header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReferredBy - The Refered-By header to take the page from.
 ***************************************************************************/
HRPOOL SipReferredByHeaderGetPool(RvSipReferredByHeaderHandle hReferredBy)
{
    return ((MsgReferredByHeader*)hReferredBy)->hPool;
}

/***************************************************************************
 * SipReferredByHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Referred-By header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReferred-By - The Referred-By header to take the page from.
 ***************************************************************************/
HPAGE SipReferredByHeaderGetPage(RvSipReferredByHeaderHandle hReferredBy)
{
    return ((MsgReferredByHeader*)hReferredBy)->hPage;
}

/***************************************************************************
 * RvSipReferredByHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Referred-By header fields are kept in a string
 *          format-for example, the referrer display name. In order to get
 *          such a field from the Referred-By header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Referred-By header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReferredByHeaderGetStringLength(
                                 IN  RvSipReferredByHeaderHandle     hHeader,
                                 IN  RvSipReferredByHeaderStringName stringName)
{
    RvInt32             stringOffset;
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
        case RVSIP_REFERRED_BY_DISP_NAME:
        {
            stringOffset = SipReferredByHeaderGetReferrerDispName(hHeader);
            break;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        case RVSIP_REFERRED_BY_OTHER_PARAMS:
        {
            stringOffset = SipReferredByHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_REFERRED_BY_BAD_SYNTAX:
        {
            stringOffset = SipReferredByHeaderGetStrBadSyntax(hHeader);
            break;
        }
        case RVSIP_REFERRED_BY_CID_PARAM:
        {
            stringOffset = SipReferredByHeaderGetCidParam(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReferredByHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipReferredByHeaderGetPool(hHeader),
                             SipReferredByHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReferredByHeaderGetReferrerDispName
 * ------------------------------------------------------------------------
 * General: This function retrieves the referrer display-name field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetReferrerDispName(
                                       IN RvSipReferredByHeaderHandle hHeader)
{
    return ((MsgReferredByHeader*)hHeader)->strReferrerDisplayName;
}

/***************************************************************************
 * RvSipReferredByHeaderGetDispName
 * ------------------------------------------------------------------------
 * General: Copies the referrer display name parameter of the Referred-By
 *          header object into a given buffer. If the bufferLen is adequate,
 *          the function copies the requested parameter into strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderGetDispName(
                                        IN RvSipReferredByHeaderHandle hHeader,
                                        IN RvChar*                     strBuffer,
                                        IN RvUint                      bufferLen,
                                        OUT RvUint*                    actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferredByHeader*)hHeader)->hPool,
                                  ((MsgReferredByHeader*)hHeader)->hPage,
                                  SipReferredByHeaderGetReferrerDispName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipReferredByHeaderSetReferrerDispName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strReferrerDisplayName
 *          in the Referred-By Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Referred-by header object
 *            strDispName  - The display-name string to be set - if Null, the exist
 *                         display-name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strStringOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetReferrerDispName(
                               IN    RvSipReferredByHeaderHandle hHeader,
                               IN    RvChar                     *strDispName,
                               IN    HRPOOL                      hPool,
                               IN    HPAGE                       hPage,
                               IN    RvInt32                     strStringOffset)
{
    RvInt32             newStr;
    MsgReferredByHeader* pHeader;
    RvStatus            retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferredByHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strStringOffset, strDispName,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strReferrerDisplayName = newStr;
#ifdef SIP_DEBUG
    if (newStr > UNDEFINED)
    {
        pHeader->pReferrerDisplayName = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                     pHeader->strReferrerDisplayName);
    }
    else
    {
        pHeader->pReferrerDisplayName = NULL;
    }
#endif

    return RV_OK;
}


/***************************************************************************
 * RvSipReferredByHeaderSetDispName
 * ------------------------------------------------------------------------
 * General: Sets the referred display name field in the Referred-By header
 *          object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Referred-By header object.
 *    strDispName  - The display name field to be set in the Referred-By header
 *                 object. If NULL is supplied, the existing display name is
 *                 removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetDispName(
                                    IN RvSipReferredByHeaderHandle hHeader,
                                    IN RvChar                     *strDispName)
{
    /* validity checking will be done in the internal function */
    return SipReferredByHeaderSetReferrerDispName(hHeader, strDispName, NULL,
                                                  NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipReferredByHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: Returns the referrer-url address spec.
 * Return Value: Returns a handle to the Address Spec object, or NULL
 *               if this Address Spec object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Referred-By header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipReferredByHeaderGetAddrSpec(
                                    IN RvSipReferredByHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgReferredByHeader*)hHeader)->hReferrerAddrSpec;
}

/***************************************************************************
 * RvSipReferredByHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the referrer-url address spec in the Referred-By header
 *          object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Referred-by header object.
 *  hAddrSpec - Handle to the Address Spec address object to be set in the
 *              Referred-By header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetAddrSpec(
                                    IN RvSipReferredByHeaderHandle hHeader,
                                    IN RvSipAddressHandle          hAddrSpec)
{
    RvStatus              stat;
    RvSipAddressType       currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress            *pAddr           = (MsgAddress*)hAddrSpec;
    MsgReferredByHeader   *pHeader         = (MsgReferredByHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hAddrSpec == NULL)
    {
        pHeader->hReferrerAddrSpec = NULL;
        return RV_OK;
    }
    else
    {
        /*first check if the given address was constructed on the header page
        and if so attach it*/
        if( SipAddrGetPool(hAddrSpec) == pHeader->hPool &&
            SipAddrGetPage(hAddrSpec) == pHeader->hPage)
        {
            pHeader->hReferrerAddrSpec = hAddrSpec;
            return RV_OK;
        }

        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
            return RV_ERROR_BADPARAM;
        }
        if (NULL != pHeader->hReferrerAddrSpec)
        {
            currentAddrType = RvSipAddrGetAddrType(pHeader->hReferrerAddrSpec);
        }

        /* if no address object was allocated, we will construct it */
        if((pHeader->hReferrerAddrSpec == NULL) ||
            (currentAddrType != pAddr->eAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInReferredByHeader(
                                                    hHeader,
                                                    pAddr->eAddrType,
                                                    &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }

        return RvSipAddrCopy(pHeader->hReferrerAddrSpec,
                             hAddrSpec);
    }
}

/***************************************************************************
 * SipReferredByHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the other params.
 * Return Value: other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetOtherParams(
                                        IN RvSipReferredByHeaderHandle hHeader)
{
    return ((MsgReferredByHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipReferredByHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Referred-By header other params field of the
 *          Referred-By header object into a given buffer.
 *          Not all the Referred-By header parameters have separated fields
 *          in the Referred-By header object.
 *          Parameters with no specific fields are refered to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value...". This list includes also the signature
 *          parameters.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Referred-By header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderGetOtherParams(
                                         IN  RvSipReferredByHeaderHandle  hHeader,
                                         IN  RvChar*                      strBuffer,
                                         IN  RvUint                       bufferLen,
                                         OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferredByHeader*)hHeader)->hPool,
                                  ((MsgReferredByHeader*)hHeader)->hPage,
                                  SipReferredByHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReferredByHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other params in the
 *          Referred-By Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 *         Note that signature params are kept in the object in one concatenated
 *         string in the form-"name=value;name=value..." (the string you set
 *         should follow these instructions).
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the Referred-By header object
 *          strOtherParams - The Referred-By other Params string to be set
 *                           - if Null, the exist other Params in the object
 *                           will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the params string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetOtherParams(
                                IN RvSipReferredByHeaderHandle hHeader,
                                IN RvChar                     *strOtherParams,
                                IN HRPOOL                      hPool,
                                IN HPAGE                       hPage,
                                IN RvInt32                     strOffset)
{
    RvInt32             newStr;
    MsgReferredByHeader* pHeader;
    RvStatus            retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferredByHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strOtherParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    pHeader->strOtherParams = newStr;
#ifdef SIP_DEBUG
    if (newStr > UNDEFINED)
    {
        pHeader->pOtherParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                             pHeader->strOtherParams);
    }
    else
    {
        pHeader->pOtherParams = NULL;
    }
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReferredByHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the Referred-By header object.
 *          Not all the Referred-By header parameters have separated fields
 *          in the Referred-By header object. Parameters with no specific
 *          fields are refered to as other params. They are kept in the
 *          object in one concatenated string in the form- "name=value;
 *          name=value...". This list includes also the signature parameters.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the Referred-By header object.
 *    strOtherParams - The other Params string to be set in the Referred-By
 *                   header. If NULL is supplied, the
 *                   existing other params field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetOtherParams(
                              IN RvSipReferredByHeaderHandle hHeader,
                              IN RvChar                     *strOtherParams)
{
    /* validity checking will be done in the internal function */
    return SipReferredByHeaderSetOtherParams(hHeader, strOtherParams,
                                             NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReferredByHeaderGetCidParam
 * ------------------------------------------------------------------------
 * General: This function retrieves the cid parameter.
 * Return Value: string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetCidParam(
                                       IN RvSipReferredByHeaderHandle hHeader)
{
    return ((MsgReferredByHeader*)hHeader)->strCidParam;
}

/***************************************************************************
 * RvSipReferredByHeaderGetCidParam
 * ------------------------------------------------------------------------
 * General: Copies the referrer cid parameter of the Referred-By
 *          header object into a given buffer. If the bufferLen is adequate,
 *          the function copies the requested parameter into strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderGetCidParam(
                                        IN RvSipReferredByHeaderHandle hHeader,
                                        IN RvChar                     *strBuffer,
                                        IN RvUint                      bufferLen,
                                        OUT RvUint                    *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferredByHeader*)hHeader)->hPool,
                                  ((MsgReferredByHeader*)hHeader)->hPage,
                                  SipReferredByHeaderGetCidParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipReferredByHeaderSetCidParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strCidParam
 *          in the Referred-By Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Referred-by header object
 *            strCidParam  - The cid-param string to be set - if Null, the exist
 *                         cid-param in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strStringOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetCidParam(
                               IN    RvSipReferredByHeaderHandle hHeader,
                               IN    RvChar                     *strCidParam,
                               IN    HRPOOL                      hPool,
                               IN    HPAGE                       hPage,
                               IN    RvInt32                     strStringOffset)
{
    RvInt32             newStr;
    MsgReferredByHeader* pHeader;
    RvStatus            retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferredByHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strStringOffset, strCidParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCidParam = newStr;
#ifdef SIP_DEBUG
    if (newStr > UNDEFINED)
    {
        pHeader->pCidParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                     pHeader->strCidParam);
    }
    else
    {
        pHeader->pCidParam = NULL;
    }
#endif

    return RV_OK;
}


/***************************************************************************
 * RvSipReferredByHeaderSetCidParam
 * ------------------------------------------------------------------------
 * General: Sets the cid parameter in the Referred-By header
 *          object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the Referred-By header object.
 *    strCidParam  - The cid parameter to be set in the Referred-By header
 *                 object. If NULL is supplied, the existing cid param is
 *                 removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetCidParam(
                                    IN RvSipReferredByHeaderHandle hHeader,
                                    IN RvChar                     *strCidParam)
{
    /* validity checking will be done in the internal function */
    return SipReferredByHeaderSetCidParam(hHeader, strCidParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReferredByHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetStrBadSyntax(
                                    IN RvSipReferredByHeaderHandle hHeader)
{
    return ((MsgReferredByHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipReferredByHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ReferredBy header,
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
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderGetStrBadSyntax(
                                               IN RvSipReferredByHeaderHandle hHeader,
                                               IN RvChar                     *strBuffer,
                                               IN RvUint                      bufferLen,
                                               OUT RvUint                    *actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReferredByHeader*)hHeader)->hPool,
                                  ((MsgReferredByHeader*)hHeader)->hPage,
                                  SipReferredByHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReferredByHeaderSetStrBadSyntax
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
RvStatus SipReferredByHeaderSetStrBadSyntax(
                                  IN RvSipReferredByHeaderHandle hHeader,
                                  IN RvChar                     *strBadSyntax,
                                  IN HRPOOL                      hPool,
                                  IN HPAGE                       hPage,
                                  IN RvInt32                     strBadSyntaxOffset)
{
    MsgReferredByHeader *header;
    RvInt32              newStrOffset;
    RvStatus             retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgReferredByHeader*)hHeader;

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
 * RvSipReferredByHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal ReferredBy header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetStrBadSyntax(
                                  IN RvSipReferredByHeaderHandle hHeader,
                                  IN RvChar                     *strBadSyntax)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipReferredByHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipReferredByHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the ReferredBy header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderSetCompactForm(
                                   IN    RvSipReferredByHeaderHandle hHeader,
                                   IN    RvBool                      bIsCompact)
{
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipReferredByHeaderSetCompactForm - Setting compact form of Header 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;

}

/***************************************************************************
 * RvSipReferredByHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the ReferredBy header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReferredByHeaderGetCompactForm(
                                   IN    RvSipReferredByHeaderHandle hHeader,
                                   IN    RvBool                      *pbIsCompact)
{
    MsgReferredByHeader* pHeader = (MsgReferredByHeader*)hHeader;
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
 * ReferredByHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ReferredByHeaderClean(IN MsgReferredByHeader*   pHeader,
								  IN RvBool                 bCleanBS)
{
    pHeader->hReferrerAddrSpec      = NULL;
    pHeader->strOtherParams         = UNDEFINED;
    pHeader->eHeaderType            = RVSIP_HEADERTYPE_REFERRED_BY;
    pHeader->strCidParam            = UNDEFINED;
#ifndef RV_SIP_JSR32_SUPPORT
	pHeader->strReferrerDisplayName = UNDEFINED;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

#ifdef SIP_DEBUG
    pHeader->pOtherParams         = NULL;
    pHeader->pCidParam            = NULL;
#ifndef RV_SIP_JSR32_SUPPORT
    pHeader->pReferrerDisplayName = NULL;
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

#endif /*#ifndef RV_SIP_PRIMITIVES*/
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef __cplusplus
}
#endif

