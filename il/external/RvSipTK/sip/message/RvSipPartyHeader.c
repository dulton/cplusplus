/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipPartyHeader.c                                     *
 *                                                                            *
 * The file defines the methods of the Party header object                    *
 * (for To/From headers):                                                     *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
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
#ifndef RV_SIP_LIGHT

#include "RvSipPartyHeader.h"
#include "_SipPartyHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PartyHeaderClean(IN MsgPartyHeader* pHeader,
							 IN RvBool          bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipToHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a To Party header object inside a given message object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 * output: hHeader - Handle to the newly constructed To Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipToHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           OUT RvSipPartyHeaderHandle* hHeader)
{

    MsgMessage*       msg;
    RvStatus         stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipPartyHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    stat = SipPartyHeaderSetType(*hHeader, RVSIP_HEADERTYPE_TO);
    if(stat != RV_OK)
        return stat;

    /* attach the header in the msg */
    msg->hToHeader = *hHeader;

    return RV_OK;
}

/***************************************************************************
 * RvSipFromHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a From Party header object inside a given message object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 * output: hHeader - Handle to the newly constructed From Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipFromHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           OUT RvSipPartyHeaderHandle* hHeader)
{
    MsgMessage*   msg;
    RvStatus     stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipPartyHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    stat = SipPartyHeaderSetType(*hHeader, RVSIP_HEADERTYPE_FROM);
    if(stat != RV_OK)
        return stat;

    /* attach the header in the msg */
    msg->hFromHeader = *hHeader;

    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Party Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderConstruct(
                                           IN  RvSipMsgMgrHandle       hMsgMgr,
                                           IN  HRPOOL                  hPool,
                                           IN  HPAGE                   hPage,
                                           OUT RvSipPartyHeaderHandle* hHeader)
{
    MsgPartyHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgPartyHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgPartyHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPartyHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }


    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;
    PartyHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPartyHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPartyHeaderConstruct - Party header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Party header object to a destination Party header
 *          object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Party header object.
 *    hSource      - Handle to the source Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderCopy(
                                        INOUT RvSipPartyHeaderHandle hDestination,
                                        IN    RvSipPartyHeaderHandle hSource)
{
    RvStatus       stat;
    MsgPartyHeader* source;
    MsgPartyHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgPartyHeader*)hSource;
    dest   = (MsgPartyHeader*)hDestination;

    dest->bIsCompact = source->bIsCompact;
    dest->eHeaderType = source->eHeaderType;
    /* AddrSpec */
    stat = RvSipPartyHeaderSetAddrSpec(hDestination, source->hAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPartyHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
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
        /*stat = RvSipPartyHeaderSetDisplayName(hDestination,
                                              source->strDisplayName);*/
        if (dest->strDisplayName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderCopy - Failed to copy displayName. hDest 0x%p",
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
        /*stat = RvSipPartyHeaderSetOtherParams(hDestination,
                                          source->strOtherParams);*/
        if (dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderCopy - Failed to copy other param. stat %d",
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

    /* tag */
    if(source->strTag > UNDEFINED)
    {
        dest->strTag = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                    dest->hPool,
                                                    dest->hPage,
                                                    source->hPool,
                                                    source->hPage,
                                                    source->strTag);
        /*stat = RvSipPartyHeaderSetTag(hDestination, source->strTag);*/
        if (dest->strTag == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderCopy - Failed to copy tag param. stat %d",
                stat));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pTag = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                  dest->strTag);
#endif
    }
    else
    {
        dest->strTag = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTag = NULL;
#endif
    }

#ifdef RFC_2543_COMPLIANT
    dest->bEmptyTag = source->bEmptyTag;
#endif /*#ifdef RFC_2543_COMPLIANT*/

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
                "RvSipPartyHeaderCopy - failed in coping strBadSyntax."));
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
 * SipPartyHeaderConstructAndCopy
 * ------------------------------------------------------------------------
 * General: Constructs a party header and fill it with information from another 
 *          party header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Party header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipPartyHeaderConstructAndCopy(
                                           IN  RvSipMsgMgrHandle       hMsgMgr,
                                           IN  HRPOOL                  hPool,
                                           IN  HPAGE                   hPage,
                                           IN  RvSipPartyHeaderHandle  hSource,
                                           IN  RvSipHeaderType         eType,
                                           OUT RvSipPartyHeaderHandle* phNewHeader)
{
    RvStatus rv = RV_OK;

    if(hPage == NULL || hSource == NULL || phNewHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(*phNewHeader == NULL)
    {
        rv = RvSipPartyHeaderConstruct(hMsgMgr, hPool, hPage, phNewHeader);
        if(RV_OK != rv)
        {
            return rv;
        }
    }
    rv = SipPartyHeaderSetType(*phNewHeader, eType);
    if(RV_OK != rv)
    {
        return rv;
    }
    rv = RvSipPartyHeaderCopy(*phNewHeader, hSource);
    if(RV_OK != rv)
    {
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * RvSipPartyHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Party header object to a textual Party header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the contact header object.
 *        hPool    - Handle to the specified memory pool.
 *        bIsTo     - Indicates whether a Header is a To header-RV_TRUE- or From header-
 *                   RV_FALSE.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderEncode(
                                          IN    RvSipPartyHeaderHandle hHeader,
                                          IN    HRPOOL                 hPool,
                                          IN    RvBool                bIsTo,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;

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
                "RvSipPartyHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PartyHeaderEncode(hHeader, hPool, *hPage, bIsTo, RV_FALSE, RV_FALSE,length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PartyHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PartyHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            party header (as string) on it.
 *          partyHeader = ("From: "|"To: ") (name-addr|addr-spec)
 *                                          *(";"party-param)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the party header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderEncode(
                            IN    RvSipPartyHeaderHandle hHeader,
                            IN    HRPOOL                 hPool,
                            IN    HPAGE                  hPage,
                            IN    RvBool                bIsTo,
                            IN    RvBool                bInUrlHeaders,
                            IN    RvBool                bForceCompactForm,
                            INOUT RvUint32*             length)
{
    MsgPartyHeader*   pHeader;
    RvStatus         stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgPartyHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PartyHeaderEncode - Encoding Party header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "From" or "To" */
    if(bIsTo == RV_TRUE)
    {
        if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "t", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "To", length);
        }
    }
    else
    {
        /* set "From" */
        if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "f", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "From", length);
        }
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PartyHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "PartyHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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

    /* partyParams with ";" in the beginning */
    if(pHeader->strTag > UNDEFINED)
    {
        /* set ";tag=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "tag", length);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        /* set pHeader->strTag */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strTag,
                                    length);
        if (stat != RV_OK)
            return stat;
    }

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
            "PartyHeaderEncode: Failed to encode Party header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPartyHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Party header-for example, "From:
 *         <sip:charlie@caller.com>;tag=5"-into a Party header object. All the textual
 *         fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Party header object.
 *  bIsTo      - Indicates whether a Header is a To header-RV_TRUE- or From header-
 *              RV_FALSE.
 *    buffer    - Buffer containing a textual Party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderParse(
                                     IN    RvSipPartyHeaderHandle   hHeader,
                                     IN    RvBool                  bIsTo,
                                     IN    RvChar*                 buffer)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;
    RvStatus             rv;
    SipParseType   eType;

    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    PartyHeaderClean(pHeader, RV_FALSE);

    eType = (bIsTo == RV_TRUE)?SIP_PARSETYPE_TO:SIP_PARSETYPE_FROM;
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        eType,
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
 * RvSipPartyHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Party header value into an Party header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPartyHeaderParse() function to parse strings
 *          that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Party header object.
 *	  bIsTo      - Indicates whether a Header is a To header-RV_TRUE- or From header-
 *                 RV_FALSE.
 *    buffer    - The buffer containing a textual Party header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderParseValue(
                                     IN    RvSipPartyHeaderHandle   hHeader,
                                     IN    RvBool                  bIsTo,
                                     IN    RvChar*                 buffer)
{
    MsgPartyHeader*  pHeader = (MsgPartyHeader*)hHeader;
	RvBool           bIsCompact;
    RvStatus         rv;
    SipParseType     eType;

    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    PartyHeaderClean(pHeader, RV_FALSE);

    eType = (bIsTo == RV_TRUE)?SIP_PARSETYPE_TO:SIP_PARSETYPE_FROM;
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        eType,
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
 * RvSipPartyHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Party header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderFix(
                                     IN RvSipPartyHeaderHandle hHeader,
                                     IN RvBool                bIsTo,
                                     IN RvChar*               pFixedBuffer)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipPartyHeaderParseValue(hHeader, bIsTo, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPartyHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPartyHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPartyHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the party header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hParty - The party header to take the page from.
 ***************************************************************************/
HRPOOL SipPartyHeaderGetPool(RvSipPartyHeaderHandle hParty)
{
    return ((MsgPartyHeader*)hParty)->hPool;
}

/***************************************************************************
 * SipPartyHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the party header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hParty - The party header to take the page from.
 ***************************************************************************/
HPAGE SipPartyHeaderGetPage(RvSipPartyHeaderHandle hParty)
{
    return ((MsgPartyHeader*)hParty)->hPage;
}

/***************************************************************************
 * RvSipPartyHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Party header fields are kept in a string format-for example, the
 *          Party header display name. In order to get such a field from the Party header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Party header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPartyHeaderGetStringLength(
                                      IN  RvSipPartyHeaderHandle     hHeader,
                                      IN  RvSipPartyHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
        case RVSIP_PARTY_DISPLAY_NAME:
        {
            stringOffset = SipPartyHeaderGetDisplayName(hHeader);
            break;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        case RVSIP_PARTY_TAG:
        {
            stringOffset = SipPartyHeaderGetTag(hHeader);
            break;
        }
        case RVSIP_PARTY_OTHER_PARAMS:
        {
            stringOffset = SipPartyHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_PARTY_BAD_SYNTAX:
        {
            stringOffset = SipPartyHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipPartyHeaderGetPool(hHeader),
                             SipPartyHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}
/***************************************************************************
 * SipPartyHeaderGetTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetTag(IN RvSipPartyHeaderHandle hHeader)
{
    if(NULL == hHeader)
        return UNDEFINED;
    return ((MsgPartyHeader*)hHeader)->strTag;
}

/***************************************************************************
 * RvSipPartyHeaderGetTag
 * ------------------------------------------------------------------------
 * General: Copies the Tag parameter of the Party header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
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
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetTag(
                                               IN RvSipPartyHeaderHandle   hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPartyHeader*)hHeader)->hPool,
                                  ((MsgPartyHeader*)hHeader)->hPage,
                                  SipPartyHeaderGetTag(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

#ifdef RFC_2543_COMPLIANT
/***************************************************************************
 * SipPartyHeaderSetEmptyTag
 * ------------------------------------------------------------------------
 * General: Sets the empty tag boolean to TRUE
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 ***************************************************************************/
RvStatus SipPartyHeaderSetEmptyTag(IN RvSipPartyHeaderHandle hHeader)
{
    MsgPartyHeader* pHeader;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader  = (MsgPartyHeader *)hHeader;
    
    pHeader->bEmptyTag = RV_TRUE;

    return RV_OK;
}

/***************************************************************************
 * SipPartyHeaderGetEmptyTag
 * ------------------------------------------------------------------------
 * General: Gets the empty tag boolean.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 ***************************************************************************/
RvBool SipPartyHeaderGetEmptyTag(IN RvSipPartyHeaderHandle hHeader)
{
    if(hHeader == NULL)
    {
        return RV_FALSE;
    }
    return ((MsgPartyHeader *)hHeader)->bEmptyTag;
}
#endif /*#ifdef RFC_2543_COMPLIANT*/
/***************************************************************************
 * SipPartyHeaderSetTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strTag in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the party header object
 *            strTag  - The tag string to be set - if Null, the exist tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetTag(IN    RvSipPartyHeaderHandle hHeader,
                               IN    RvChar *              strTag,
                               IN    HRPOOL                 hPool,
                               IN    HPAGE                  hPage,
                               IN    RvInt32               strTagOffset)
{
    RvInt32        newStr;
    MsgPartyHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgPartyHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strTagOffset, strTag,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strTag = newStr;
#ifdef SIP_DEBUG
    pHeader->pTag = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strTag);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderSetTag
 * ------------------------------------------------------------------------
 * General: Sets the tag field in the Party header object
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the party header object.
 *    strTag  - The Tag field to be set in the Party header object. If NULL is supplied, the
 *            existing tag field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetTag
                                        (IN    RvSipPartyHeaderHandle hHeader,
                                         IN    RvChar *              strTag)
{
    /* validity checking will be done in the internal function */
    return SipPartyHeaderSetTag(hHeader, strTag, NULL, NULL_PAGE, UNDEFINED);
}


#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipPartyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetDisplayName(IN RvSipPartyHeaderHandle hHeader)
{
    return ((MsgPartyHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipPartyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the Party header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetDisplayName(
                                               IN RvSipPartyHeaderHandle hHeader,
                                               IN RvChar*               strBuffer,
                                               IN RvUint                bufferLen,
                                               OUT RvUint*              actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgPartyHeader*)hHeader)->hPool,
                                  ((MsgPartyHeader*)hHeader)->hPage,
                                  SipPartyHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPartyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the party header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetDisplayName(IN    RvSipPartyHeaderHandle hHeader,
                                       IN    RvChar *              strDisplayName,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOffset)
{
    RvInt32        newStr;
    MsgPartyHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgPartyHeader*)hHeader;

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
 * RvSipPartyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the Party header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the header object.
 *    strDisplayName - The display name to be set in the Party header. If NULL is supplied, the existing
 *                   display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetDisplayName
                                            (IN    RvSipPartyHeaderHandle hHeader,
                                             IN    RvChar *              strDisplayName)
{
    /* validity checking will be done in the internal function */
    return SipPartyHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipPartyHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the Party header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Party header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPartyHeaderGetAddrSpec
                                            (IN RvSipPartyHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgPartyHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipPartyHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec address object in the Party header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Party header object.
 *  hAddrSpec - Handle to the Address Spec address object to be set in the Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetAddrSpec
                                            (IN    RvSipPartyHeaderHandle hHeader,
                                             IN    RvSipAddressHandle     hAddrSpec)
{
    RvStatus           stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgPartyHeader     *pHeader         = (MsgPartyHeader*)hHeader;

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

            stat = RvSipAddrConstructInPartyHeader(hHeader,
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
 * SipPartyHeaderCorrectUrl
 * ------------------------------------------------------------------------
 * General: The SIP-URL of the party header MUST NOT contain the "transport-param",
 *          "maddr-param", "ttl-param", or "headers" elements.
 *          this function removes these parameters.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Party header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipPartyHeaderCorrectUrl(IN RvSipPartyHeaderHandle hHeader)
{
    MsgPartyHeader     *pHeader         = (MsgPartyHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pHeader->hAddrSpec == NULL)
        return RV_ERROR_INVALID_HANDLE;


    /*if this is not a Sip URL - don't fix header*/
    if(RvSipAddrGetAddrType(pHeader->hAddrSpec) != RVSIP_ADDRTYPE_URL)
    {
        /* nothing to do */
        return RV_OK;
    }
    /* Delete the Maddr parameter of the Sip-Url */
    RvSipAddrUrlSetMaddrParam(pHeader->hAddrSpec, NULL);
    /* Delete the Transport parameter of the Sip-Url */
    RvSipAddrUrlSetTransport(pHeader->hAddrSpec, RVSIP_TRANSPORT_UNDEFINED, NULL);
    /* Delete the Headers parameter of the Sip-Url */
    RvSipAddrUrlSetHeaders(pHeader->hAddrSpec, NULL);
    /* Delete the Ttl parameter of the Sip-Url */
    RvSipAddrUrlSetTtlNum(pHeader->hAddrSpec, UNDEFINED);
    /* Delete the port parameter of the Sip-Url */
    RvSipAddrUrlSetPortNum(pHeader->hAddrSpec, UNDEFINED);
    /* Delete the method parameter of the Sip-Url */
    RvSipAddrUrlSetMethod(pHeader->hAddrSpec,RVSIP_METHOD_UNDEFINED, NULL);
    /* Delete the lr parameter of the Sip-Url */
    RvSipAddrUrlSetLrParam(pHeader->hAddrSpec, RV_FALSE);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /* Delete the comp=sigcomp parameter of the Sip-Url */
	RvSipAddrUrlSetCompParam(pHeader->hAddrSpec, RVSIP_COMP_UNDEFINED,NULL);
	RvSipAddrUrlSetSigCompIdParam(pHeader->hAddrSpec, NULL);
#endif
/* SPIRENT_END */
    return RV_OK;
}
/***************************************************************************
 * SipPartyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams.
 * Return Value: Other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetOtherParams(IN RvSipPartyHeaderHandle hHeader)
{
    return ((MsgPartyHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPartyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Party header other params field of the Party header object into a
 *          given buffer.
 *          Not all the Party header parameters have separated fields in the Party header
 *          object. Parameters with no specific fields are refered to as other params. They
 *          are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Party header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetOtherParams(
                                               IN RvSipPartyHeaderHandle   hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPartyHeader*)hHeader)->hPool,
                                  ((MsgPartyHeader*)hHeader)->hPage,
                                  SipPartyHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPartyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the party header object
 *          strOtherParams - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetOtherParams(  IN    RvSipPartyHeaderHandle hHeader,
                                         IN    RvChar *              strOtherParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strOtherOffset)
{
    RvInt32        newStr;
    MsgPartyHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgPartyHeader*)hHeader;

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
 * RvSipPartyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the Party header object.
 *          Not all the Party header parameters have separated fields in the Party header
 *          object. Parameters with no specific fields are refered to as other params. They
 *          are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the party header object.
 *    strOtherParams - The Other Params string to be set in the Party header. If NULL is supplied, the
 *                   existing Other Params field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetOtherParams
                                    (IN    RvSipPartyHeaderHandle hHeader,
                                     IN    RvChar *              strOtherParams)
{
    /* validity checking will be done in the internal function */
    return SipPartyHeaderSetOtherParams(hHeader, strOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipPartyIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two party header objects. Party header fields are considered equal if
 *         their URIs match and their header parameters match in name and value.
 *         Parameters names and token parameter values are compared ignoring case while
 *         quoted-string parameter values are case-sensitive.
 *         The tag comparison is performed if both header fields have a tag value.
 * Return Value: Returns RV_TRUE if the party header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the party header object.
 *    hOtherHeader - Handle to the party header object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPartyIsEqual(const RvSipPartyHeaderHandle hHeader,
                                           const RvSipPartyHeaderHandle hOtherHeader)
{
    MsgPartyHeader*   first;
    MsgPartyHeader*   other;

    if((hHeader == NULL) || (hOtherHeader == NULL))
        return RV_FALSE;
	
	if (hHeader == hOtherHeader)
	{
		/* this is the same object */
		return RV_TRUE;
	}

    first = (MsgPartyHeader*)hHeader;
    other = (MsgPartyHeader*)hOtherHeader;

	if (first->strBadSyntax != UNDEFINED || other->strBadSyntax != UNDEFINED)
	{
		/* bad syntax string is uncomparable */
		return RV_FALSE;
	}
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    if(RvSipAddrUrlCompare(first->hAddrSpec, other->hAddrSpec) != 0)
#else /* !defined(UPDATED_BY_SPIRENT) */

    if(RvSipAddrUrlIsEqual(first->hAddrSpec, other->hAddrSpec) == RV_FALSE)
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "RvSipPartyIsEqual - Header 0x%p and header 0x%p are not equal. addrSpec is not equal",
                hHeader, hOtherHeader));

        return RV_FALSE;
    }

    /* the tags will be check, only if both headers contain tags */
    if((first->strTag > UNDEFINED) && (other->strTag > UNDEFINED))
    {
        /*if(strcmp(first->strTag, other->strTag) != 0)*/

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT) 
        if(SPIRENT_RPOOL_Stricmp(first->hPool, first->hPage, first->strTag,
                        other->hPool, other->hPage, other->strTag) != 0)
#else /* !defined(UPDATED_BY_SPIRENT) */
        if(RPOOL_Stricmp(first->hPool, first->hPage, first->strTag,
                        other->hPool, other->hPage, other->strTag) == RV_FALSE)
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                    "RvSipPartyIsEqual - Header 0x%p and header 0x%p are not equal. tag is not equal!!!",
                    hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }

    RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
            "RvSipPartyIsEqual - Header 0x%p and header 0x%p are equal",
            hHeader, hOtherHeader));

    return RV_TRUE;

}

/***************************************************************************
 * SipPartyHeaderSetType
 * ------------------------------------------------------------------------
 * General: Sets the type of a party header:
 *          RVSIP_HEADERTYPE_TO / RVSIP_HEADERTYPE_FROM
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 *  type           - The type to be set in the header object (to / from)
 ***************************************************************************/
RvStatus SipPartyHeaderSetType(IN RvSipPartyHeaderHandle hHeader,
                                IN RvSipHeaderType        type)
{
    MsgPartyHeader* pHeader;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader  = (MsgPartyHeader *)hHeader;
    if(type != RVSIP_HEADERTYPE_TO && type != RVSIP_HEADERTYPE_FROM)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipPartyHeaderSetType - type is not valid"));
        return RV_ERROR_BADPARAM;
    }

    pHeader->eHeaderType = type;

    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderSetType
 * ------------------------------------------------------------------------
 * General: Sets the type of a party header:
 *          RVSIP_HEADERTYPE_TO or RVSIP_HEADERTYPE_FROM
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 *  eType          - The type of the header object (to / from)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetType(IN RvSipPartyHeaderHandle hHeader,
                                                   IN RvSipHeaderType        eType)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader *)hHeader;

    if(pHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader->eHeaderType = eType;

    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderGetType
 * ------------------------------------------------------------------------
 * General: returns the tpye of a party header:
 *          RVSIP_HEADERTYPE_TO or RVSIP_HEADERTYPE_FROM
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 *  peType         - The type of the header object (to / from)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetType(IN RvSipPartyHeaderHandle hHeader,
                                                   OUT RvSipHeaderType*      peType)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader *)hHeader;

    if(pHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peType = pHeader->eHeaderType;

    return RV_OK;
}
/***************************************************************************
 * SipPartyHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPartyHeaderGetStrBadSyntax(
                                    IN RvSipPartyHeaderHandle hHeader)
{
    return ((MsgPartyHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPartyHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Party header,
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
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetStrBadSyntax(
                                               IN RvSipPartyHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPartyHeader*)hHeader)->hPool,
                                  ((MsgPartyHeader*)hHeader)->hPage,
                                  SipPartyHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPartyHeaderSetStrBadSyntax
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
RvStatus SipPartyHeaderSetStrBadSyntax(
                                  IN RvSipPartyHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPartyHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgPartyHeader*)hHeader;

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
 * RvSipPartyHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Party header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetStrBadSyntax(
                                  IN RvSipPartyHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipPartyHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipPartyHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Party header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipPartyHeader - Handle to the Party header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Party header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetRpoolString(
                             IN    RvSipPartyHeaderHandle      hSipPartyHeader,
                             IN    RvSipPartyHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hSipPartyHeader;
    RvInt32      requestedParamOffset;

    if(hSipPartyHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipPartyHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }


    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_PARTY_DISPLAY_NAME:
        requestedParamOffset = pHeader->strDisplayName;
        break;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    case RVSIP_PARTY_TAG:
        requestedParamOffset = pHeader->strTag;
        break;
    case RVSIP_PARTY_OTHER_PARAMS:
        requestedParamOffset = pHeader->strOtherParams;
        break;
    case RVSIP_PARTY_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipPartyHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipPartyHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipPartyHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Party header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipPartyHeader - Handle to the Party header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetRpoolString(
                             IN    RvSipPartyHeaderHandle      hSipPartyHeader,
                             IN    RvSipPartyHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hSipPartyHeader;
    RvInt32*     pDestParamOffset;

    if(hSipPartyHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipPartyHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }


    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_PARTY_DISPLAY_NAME:
        pDestParamOffset = &(pHeader->strDisplayName);
        break;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    case RVSIP_PARTY_TAG:
        pDestParamOffset = &(pHeader->strTag);
        break;
    case RVSIP_PARTY_OTHER_PARAMS:
        pDestParamOffset = &(pHeader->strOtherParams);
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPartyHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
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
                  "RvSipPartyHeaderSetRpoolString - Failed to copy the supplied string into the header page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

#endif /*#ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * RvSipPartyHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Party header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderSetCompactForm(
                                   IN    RvSipPartyHeaderHandle hHeader,
                                   IN    RvBool                bIsCompact)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipPartyHeaderSetCompactForm - Setting compact form of Header 0x%p to %d",
             hHeader, bIsCompact));
    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}

/***************************************************************************
 * RvSipPartyHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Party header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPartyHeaderGetCompactForm(
                                   IN    RvSipPartyHeaderHandle hHeader,
                                   IN    RvBool                *pbIsCompact)
{
    MsgPartyHeader* pHeader = (MsgPartyHeader*)hHeader;
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
 * PartyHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanIsCompact - Clean the compact indication or not.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PartyHeaderClean(IN MsgPartyHeader* pHeader,
							 IN RvBool          bCleanBS)
{
    pHeader->hAddrSpec      = NULL;
    pHeader->strOtherParams = UNDEFINED;
    pHeader->strTag         = UNDEFINED;
#ifdef RFC_2543_COMPLIANT
    pHeader->bEmptyTag = RV_FALSE;
#endif /*#ifdef RFC_2543_COMPLIANT*/
#ifndef RV_SIP_JSR32_SUPPORT
	pHeader->strDisplayName = UNDEFINED;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

#ifdef SIP_DEBUG
    pHeader->pOtherParams   = NULL;
    pHeader->pTag           = NULL;
#ifndef RV_SIP_JSR32_SUPPORT
	pHeader->pDisplayName   = NULL;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
#endif

	if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->eHeaderType      = RVSIP_HEADERTYPE_UNDEFINED;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipPartyHeaderCorrectUrl
 * ------------------------------------------------------------------------
 * General: The SIP-URL of the party header MUST NOT contain the “transport-param”,
 *          “maddr-param”, “ttl-param”, or “headers” elements.
 *          this function removes these parameters.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Party header object.
 ***************************************************************************/
RvStatus RVCALLCONV RvSipPartyHeaderCorrectUrl(IN RvSipPartyHeaderHandle hHeader)
{
   return SipPartyHeaderCorrectUrl(hHeader);
}
#endif
/* SPIRENT_END */

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

