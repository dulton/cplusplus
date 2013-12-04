/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReplyToHeader.c                                   *
 *                                                                            *
 * The file defines the methods of the ReplyTo header object:                 *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Mar 2005                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

#include "RvSipReplyToHeader.h"
#include "_SipReplyToHeader.h"
#include "RvSipAddress.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "ParserProcess.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"


#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ReplyToHeaderClean(IN MsgReplyToHeader* pHeader,
                              IN RvBool           bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReplyToHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an ReplyTo header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed ReplyTo 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipReplyToHeaderHandle             *hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                    stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipReplyToHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                      msg->hPool,
                                      msg->hPage,
                                      hHeader);

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
                              RVSIP_HEADERTYPE_REPLY_TO,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipReplyToHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone ReplyTo Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed ReplyTo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipReplyToHeaderHandle *hHeader)
{
    MsgReplyToHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgReplyToHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgReplyToHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReplyToHeaderConstruct - Allocation failed. hPool 0x%p, hPage %d",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    ReplyToHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipReplyToHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReplyToHeaderConstruct - ReplyTo header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipReplyToHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source ReplyTo header object to a destination 
 *          ReplyTo header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination ReplyTo header object.
 *    hSource      - Handle to the source ReplyTo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderCopy(
                                     IN    RvSipReplyToHeaderHandle hDestination,
                                     IN    RvSipReplyToHeaderHandle hSource)
{
    MsgReplyToHeader* source, *dest;
	RvStatus          rv;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgReplyToHeader*)hSource;
    dest   = (MsgReplyToHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;

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
        if (dest->strDisplayName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipReplyToHeaderCopy - Failed to copy displayName. hDest 0x%p",
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
	
	/* Address spec */
	rv = RvSipReplyToHeaderSetAddrSpec(hDestination, source->hAddrSpec);
	if (rv != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
			"RvSipReplyToHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
			hDestination, rv));
        return rv;
    }

	/* other-params */
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
						"RvSipReplyToHeaderCopy: Failed to copy other params. hDest 0x%p",
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
		dest->strBadSyntax = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strBadSyntax);
		if (dest->strBadSyntax == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipReplyToHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
						hDestination));
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
 * RvSipReplyToHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an ReplyTo header object to a textual ReplyTo header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the ReplyTo header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderEncode(
                                          IN    RvSipReplyToHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgReplyToHeader* pHeader;

    pHeader = (MsgReplyToHeader*)hHeader;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplyToHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplyToHeaderEncode - Got new page %d on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ReplyToHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplyToHeaderEncode - Failed. Free page %d on pool 0x%p. ReplyToHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ReplyToHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          ReplyTo header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderEncode(
                                      IN    RvSipReplyToHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length)
{
    MsgReplyToHeader*  pHeader = (MsgReplyToHeader*)hHeader;
    RvStatus          stat;
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ReplyToHeaderEncode - Encoding ReplyTo header. hHeader 0x%p, hPool 0x%p, hPage %d",
             hHeader, hPool, hPage));

	/* encode "Reply-To" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Reply-To", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"ReplyToHeaderEncode - Failed to encode Reply-To header. RvStatus is %d, hPool 0x%p, hPage %d",
					stat, hPool, hPage));
		return stat;
	}
	/* encode ":" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
										length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			      "ReplyToHeaderEncode - Failed to encode ReplyTo header. RvStatus is %d, hPool 0x%p, hPage %d",
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
                "ReplyToHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage %d",
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
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"ReplyToHeaderEncode - Failed to encode ReplyTo header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

	if (pHeader->hAddrSpec != NULL)
    {
        /* encode address-spec */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"ReplyToHeaderEncode - Failed to encode address spec in Info header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
            return stat;
		}
    }
    else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"ReplyToHeaderEncode - Failed to encode Reply-To header - Missing mandatory address spec. hHeader=%p",
					pHeader));
        return RV_ERROR_UNKNOWN;
	}

    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReplyToHeaderEncode - Failed to encode ReplyTo header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}

        /* set pHeader->strOtherParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReplyToHeaderEncode - Failed to encode ReplyTo header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipReplyToHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReplyTo header into a ReplyTo header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the ReplyTo header object.
 *    buffer    - Buffer containing a textual ReplyTo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderParse(
                                     IN RvSipReplyToHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgReplyToHeader* pHeader = (MsgReplyToHeader*)hHeader;
    RvStatus          rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    ReplyToHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REPLY_TO,
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
 * RvSipReplyToHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReplyTo header value into an ReplyTo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReplyToHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ReplyTo header object.
 *    buffer    - The buffer containing a textual ReplyTo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderParseValue(
                                     IN RvSipReplyToHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgReplyToHeader* pHeader = (MsgReplyToHeader*)hHeader;
    RvStatus          rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    ReplyToHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REPLY_TO,
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
 * RvSipReplyToHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ReplyTo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderFix(
                                     IN RvSipReplyToHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgReplyToHeader* pHeader = (MsgReplyToHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipReplyToHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipReplyToHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipReplyToHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipReplyToHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the ReplyTo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipReplyToHeaderGetPool(RvSipReplyToHeaderHandle hHeader)
{
    return ((MsgReplyToHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipReplyToHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the ReplyTo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipReplyToHeaderGetPage(RvSipReplyToHeaderHandle hHeader)
{
    return ((MsgReplyToHeader*)hHeader)->hPage;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReplyToHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the ReplyTo header fields are kept in a string format, for
 *          example, the ReplyTo header YYY string. In order to get such a field
 *          from the ReplyTo header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the ReplyTo header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReplyToHeaderGetStringLength(
                                      IN  RvSipReplyToHeaderHandle     hHeader,
                                      IN  RvSipReplyToHeaderStringName eStringName)
{
    RvInt32          stringOffset = UNDEFINED;
    MsgReplyToHeader* pHeader      = (MsgReplyToHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
	case RVSIP_REPLY_TO_DISPLAY_NAME:
		{
			stringOffset = SipReplyToHeaderGetDisplayName(hHeader);
            break;
		}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */		
    case RVSIP_REPLY_TO_OTHER_PARAMS:
        {
            stringOffset = SipReplyToHeaderGetOtherParams(hHeader);
            break;
        }
	case RVSIP_REPLY_TO_BAD_SYNTAX:
		{
			stringOffset = SipReplyToHeaderGetStrBadSyntax(hHeader);
			break;
		}
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipReplyToHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
    else
        return 0;
}

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReplyToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetDisplayName(IN RvSipReplyToHeaderHandle hHeader)
{
    return ((MsgReplyToHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipReplyToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the ReplyTo header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetDisplayName(
                                               IN RvSipReplyToHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReplyToHeader*)hHeader)->hPool,
                                  ((MsgReplyToHeader*)hHeader)->hPage,
                                  SipReplyToHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplyToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          ReplyToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the ReplyTo header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipReplyToHeaderSetDisplayName(IN   RvSipReplyToHeaderHandle hHeader,
                                        IN   RvChar *                 strDisplayName,
                                        IN   HRPOOL                   hPool,
                                        IN   HPAGE                    hPage,
                                        IN   RvInt32                  strOffset)
{
    RvInt32           newStr;
    MsgReplyToHeader* pHeader;
    RvStatus          retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplyToHeader*)hHeader;

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
 * RvSipReplyToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the ReplyTo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the header object.
 *    strDisplayName - The display name to be set in the ReplyTo header. If NULL is supplied, the existing
 *                   display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetDisplayName
                                            (IN    RvSipReplyToHeaderHandle hHeader,
                                             IN    RvChar *              strDisplayName)
{
    /* validity checking will be done in the internal function */
    return SipReplyToHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipReplyToHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address-Spec field is held in the Reply-To header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address object, or NULL if the Address
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Reply-To header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipReplyToHeaderGetAddrSpec
                                            (IN RvSipReplyToHeaderHandle hHeader)
{
	if(hHeader == NULL)
        return NULL;
	
    return ((MsgReplyToHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipReplyToHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address-Spec object in the Reply-To header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Reply-To header object.
 *    hAddrSpec - Handle to the Address-Spec object to be set in the 
 *                Reply-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetAddrSpec
                                            (IN    RvSipReplyToHeaderHandle  hHeader,
                                             IN    RvSipAddressHandle        hAddrSpec)
{
	RvStatus            stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgReplyToHeader   *pHeader         = (MsgReplyToHeader*)hHeader;

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
            RvSipAddressHandle hAddr;

            stat = RvSipAddrConstructInReplyToHeader(hHeader,
                                                     pAddr->eAddrType,
                                                     &hAddr);
            if(stat != RV_OK)
                return stat;
        }
        return RvSipAddrCopy(pHeader->hAddrSpec,
                             hAddrSpec);
    }
}

/***************************************************************************
 * SipReplyToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the ReplyTo header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object..
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetOtherParams(IN RvSipReplyToHeaderHandle hHeader)
{
    return ((MsgReplyToHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipReplyToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the ReplyTo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the ReplyTo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetOtherParams(
                                       IN RvSipReplyToHeaderHandle  hHeader,
                                       IN RvChar*                   strBuffer,
                                       IN RvUint                    bufferLen,
                                       OUT RvUint*                  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReplyToHeader*)hHeader)->hPool,
                                  ((MsgReplyToHeader*)hHeader)->hPage,
                                  SipReplyToHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplyToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          ReplyTo Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the ReplyTo header object.
 *       pOtherParams - The other-params to be set in the ReplyTo header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReplyToHeaderSetOtherParams(
							    IN  RvSipReplyToHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                     hPool,
                                IN  HPAGE                      hPage,
                                IN  RvInt32                    strOffset)
{
    RvInt32                      newStr;
    MsgReplyToHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplyToHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOtherParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strOtherParams  = newStr;
#ifdef SIP_DEBUG
    pHeader->pOtherParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                   pHeader->strOtherParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReplyToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the ReplyTo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the ReplyTo header object.
 *    pText      - The other-params string to be set in the ReplyTo header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetOtherParams(
                                     IN RvSipReplyToHeaderHandle    hHeader,
                                     IN RvChar                     *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReplyToHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReplyToHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object.
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetStrBadSyntax(IN  RvSipReplyToHeaderHandle hHeader)
{
    return ((MsgReplyToHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipReplyToHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ReplyTo header,
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetStrBadSyntax(
                                     IN  RvSipReplyToHeaderHandle             hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgReplyToHeader*)hHeader)->hPool,
                                  ((MsgReplyToHeader*)hHeader)->hPage,
                                  SipReplyToHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplyToHeaderSetStrBadSyntax
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
RvStatus SipReplyToHeaderSetStrBadSyntax(
                                  IN RvSipReplyToHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgReplyToHeader*   header;
    RvInt32             newStrOffset;
    RvStatus            retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgReplyToHeader*)hHeader;

	if (hPool == header->hPool && hPage == header->hPage)
	{
		/* no need to copy, only to update the offset */
		header->strBadSyntax = strBadSyntaxOffset;
	}
	else
	{
		retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
			                          strBadSyntax, header->hPool, header->hPage, &newStrOffset);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}
		header->strBadSyntax = newStrOffset;
	}

#ifdef SIP_DEBUG
    header->pBadSyntax = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
												header->strBadSyntax);
#endif
    
	return RV_OK;
}

/***************************************************************************
 * RvSipReplyToHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          ReplyTo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetStrBadSyntax(
                                  IN RvSipReplyToHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReplyToHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ReplyToHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ReplyToHeaderClean(IN MsgReplyToHeader*            pHeader,
                               IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_REPLY_TO;
	
	pHeader->hAddrSpec      = NULL;

	pHeader->strOtherParams = UNDEFINED;
		
#ifdef SIP_DEBUG
	pHeader->pOtherParams   = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}



#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
