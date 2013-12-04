/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipWarningHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the Warning header object:                  *
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

#include "RvSipWarningHeader.h"
#include "_SipWarningHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"


#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void WarningHeaderClean(IN MsgWarningHeader* pHeader,
                               IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipWarningHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Warning header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Warning
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipWarningHeaderHandle             *hHeader)
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

    stat = RvSipWarningHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_WARNING,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipWarningHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Warning Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipWarningHeaderHandle *hHeader)
{
    MsgWarningHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgWarningHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgWarningHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipWarningHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    WarningHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipWarningHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipWarningHeaderConstruct - Warning header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipWarningHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Warning header object to a destination
 *          Warning header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Warning header object.
 *    hSource      - Handle to the source Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderCopy( 
									 IN    RvSipWarningHeaderHandle hDestination,
                                     IN    RvSipWarningHeaderHandle hSource)
{
    MsgWarningHeader* source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgWarningHeader*)hSource;
    dest   = (MsgWarningHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;

	/* warn-code */
	dest->warnCode = source->warnCode;
	
	/* warn-agent */
	if(source->strWarnAgent > UNDEFINED)
	{
		dest->strWarnAgent = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strWarnAgent);
		if (dest->strWarnAgent == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipWarningHeaderCopy: Failed to copy strWarnAgent. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pWarnAgent = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												  dest->strWarnAgent);
#endif
	}
	else
	{
		dest->strWarnAgent = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pWarnAgent = NULL;
#endif
	}

	/* warn-text */
	if(source->strWarnText > UNDEFINED)
	{
		dest->strWarnText = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strWarnText);
		if (dest->strWarnText == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipWarningHeaderCopy: Failed to copy strWarnText. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pWarnText = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												   dest->strWarnText);
#endif
	}
	else
	{
		dest->strWarnText = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pWarnText = NULL;
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
						"RvSipWarningHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipWarningHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Warning header object to a textual Warning header. The
 *          textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Warning header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderEncode(
                                          IN    RvSipWarningHeaderHandle   hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgWarningHeader* pHeader;

    pHeader = (MsgWarningHeader*)hHeader;
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
                "RvSipWarningHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipWarningHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = WarningHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipWarningHeaderEncode - Failed. Free page 0x%p on pool 0x%p. WarningHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * WarningHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Warning header (as string) on it.
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
RvStatus RVCALLCONV WarningHeaderEncode(
                                      IN    RvSipWarningHeaderHandle    hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length)
{
    MsgWarningHeader*  pHeader = (MsgWarningHeader*)hHeader;
    RvStatus          stat;
	RvChar            strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "WarningHeaderEncode - Encoding Warning header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

	/* encode "Warning" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Warning", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
			      "WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "WarningHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

	if (pHeader->warnCode == UNDEFINED ||
		(pHeader->strWarnAgent == UNDEFINED) ||
		pHeader->strWarnText == UNDEFINED)
	{
		/* mandatory field is missing */
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"WarningHeaderEncode - Failed to encode Warning header - mandatory field is missing. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return RV_ERROR_BADPARAM;
	}

	/* warn-code */
	/* set warn-code value */
	MsgUtils_itoa(pHeader->warnCode, strHelper);
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										strHelper, length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}
	/* set space */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetSpaceChar(bInUrlHeaders), length);
    if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}

	/* warn agent */
	stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                hPool,
                                hPage,
                                pHeader->hPool,
                                pHeader->hPage,
                                pHeader->strWarnAgent,
                                length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
					stat, hPool, hPage));
		return stat;
	}
	
	/* set space */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetSpaceChar(bInUrlHeaders), length);
    if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}

	/* warn-text */
    stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                hPool,
                                hPage,
                                pHeader->hPool,
                                pHeader->hPage,
                                pHeader->strWarnText,
                                length);
    if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"WarningHeaderEncode - Failed to encode Warning header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
		return stat;
	}

    return RV_OK;
}


/***************************************************************************
 * RvSipWarningHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Warning header into a Warning header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Warning header object.
 *    buffer    - Buffer containing a textual Warning header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderParse(
                                     IN RvSipWarningHeaderHandle  hHeader,
                                     IN RvChar*                   buffer)
{
    MsgWarningHeader* pHeader = (MsgWarningHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    WarningHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_WARNING,
                                        buffer,
                                        RV_FALSE,
                                         (void*)hHeader);;
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
 * RvSipWarningHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Warning header value into an Warning header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipWarningHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Warning header object.
 *    buffer    - The buffer containing a textual Warning header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderParseValue(
                                     IN RvSipWarningHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgWarningHeader* pHeader = (MsgWarningHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    WarningHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_WARNING,
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
 * RvSipWarningHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Warning header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderFix(
                                     IN RvSipWarningHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgWarningHeader* pHeader = (MsgWarningHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipWarningHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipWarningHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipWarningHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipWarningHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Warning object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipWarningHeaderGetPool(RvSipWarningHeaderHandle hHeader)
{
    return ((MsgWarningHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipWarningHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Warning object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipWarningHeaderGetPage(RvSipWarningHeaderHandle hHeader)
{
    return ((MsgWarningHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipWarningHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Warning header fields are kept in a string format, for
 *          example, the Warning header warn-text string. In order to get such a field
 *          from the Warning header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Warning header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipWarningHeaderGetStringLength(
                                      IN  RvSipWarningHeaderHandle     hHeader,
                                      IN  RvSipWarningHeaderStringName eStringName)
{
    RvInt32          stringOffset = UNDEFINED;
    MsgWarningHeader* pHeader      = (MsgWarningHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
        case RVSIP_WARNING_WARN_AGENT:
        {
            stringOffset = SipWarningHeaderGetWarnAgent(hHeader);
            break;
        }
		case RVSIP_WARNING_WARN_TEXT:
        {
            stringOffset = SipWarningHeaderGetWarnText(hHeader);
            break;
        }
        case RVSIP_WARNING_BAD_SYNTAX:
        {
            stringOffset = SipWarningHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipWarningHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
    else
        return 0;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipWarningHeaderGetWarnCode
 * ------------------------------------------------------------------------
 * General: Gets the warn-code value from the Warning Header object.
 * Return Value: Returns the warn-code value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Warning header object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipWarningHeaderGetWarnCode(
                                    IN RvSipWarningHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgWarningHeader*)hHeader)->warnCode;
}


/***************************************************************************
 * RvSipWarningHeaderSetWarnCode
 * ------------------------------------------------------------------------
 * General: Sets the warn-code value in the Warning Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Warning header object.
 *    warnCode          - The warn-code value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnCode(
                                 IN    RvSipWarningHeaderHandle hHeader,
                                 IN    RvInt16                  warnCode)
{
	MsgWarningHeader *pHeader = (MsgWarningHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

	pHeader->warnCode = warnCode;
    return RV_OK;
}

/***************************************************************************
 * SipWarningHeaderGetWarnAgent
 * ------------------------------------------------------------------------
 * General:This method gets the warn-agent string from the Warning header.
 * Return Value: host offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object..
 ***************************************************************************/
RvInt32 SipWarningHeaderGetWarnAgent(IN RvSipWarningHeaderHandle hHeader)
{
    return ((MsgWarningHeader*)hHeader)->strWarnAgent;
}

/***************************************************************************
 * RvSipWarningHeaderGetWarnAgent
 * ------------------------------------------------------------------------
 * General: Copies the warn-agent string from the Warning header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Warning header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetWarnAgent(
                                       IN RvSipWarningHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgWarningHeader*)hHeader)->hPool,
                                  ((MsgWarningHeader*)hHeader)->hPage,
                                  SipWarningHeaderGetWarnAgent(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipWarningHeaderSetWarnAgent
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the warn-agent string in
 *          the Warning Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Warning header object.
 *       pHost        - The host string to be set in the Warning header - If
 *                      NULL, the existing host string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipWarningHeaderSetWarnAgent(IN  RvSipWarningHeaderHandle   hHeader,
                                      IN  RvChar                    *pHost,
                                      IN  HRPOOL                     hPool,
                                      IN  HPAGE                      hPage,
                                      IN  RvInt32                    strOffset)
{
    RvInt32                      newStr;
    MsgWarningHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgWarningHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pHost,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strWarnAgent  = newStr;
#ifdef SIP_DEBUG
    pHeader->pWarnAgent = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                  pHeader->strWarnAgent);
#endif
	
    return RV_OK;
}

/***************************************************************************
 * RvSipWarningHeaderSetWarnAgent
 * ------------------------------------------------------------------------
 * General:Sets the warn-agent string in the Warning header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Warning header object.
 *    pHost      - The host string to be set in the Warning header.
 *                 If NULL is supplied, the existing host is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnAgent(
                                     IN RvSipWarningHeaderHandle   hHeader,
                                     IN RvChar                    *pHost)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipWarningHeaderSetWarnAgent(hHeader, pHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipWarningHeaderGetWarnText
 * ------------------------------------------------------------------------
 * General:This method gets the warn-text string from the Warning header.
 * Return Value: warn-text offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object..
 ***************************************************************************/
RvInt32 SipWarningHeaderGetWarnText(IN RvSipWarningHeaderHandle hHeader)
{
    return ((MsgWarningHeader*)hHeader)->strWarnText;
}

/***************************************************************************
 * RvSipWarningHeaderGetWarnText
 * ------------------------------------------------------------------------
 * General: Copies the warn-text string from the Warning header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Warning header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetWarnText(
                                       IN RvSipWarningHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgWarningHeader*)hHeader)->hPool,
                                  ((MsgWarningHeader*)hHeader)->hPage,
                                  SipWarningHeaderGetWarnText(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipWarningHeaderSetWarnText
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the warn-text string
 *          in the Warning Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Warning header object.
 *       pWarnText    - The warn-text string to be set in the Warning header - If
 *                      NULL, the existing warn-text string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipWarningHeaderSetWarnText(IN  RvSipWarningHeaderHandle   hHeader,
                                     IN  RvChar                    *pWarnText,
                                     IN  HRPOOL                     hPool,
                                     IN  HPAGE                      hPage,
                                     IN  RvInt32                    strOffset)
{
    RvInt32                      newStr;
    MsgWarningHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgWarningHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pWarnText,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strWarnText  = newStr;
#ifdef SIP_DEBUG
    pHeader->pWarnText = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
												pHeader->strWarnText);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipWarningHeaderSetWarnText
 * ------------------------------------------------------------------------
 * General:Sets the warn-text string in the Warning header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Warning header object.
 *    pWarnText  - The warn-text string to be set in the Warning header.
 *                 If NULL is supplied, the existing warn-text is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnText(
                                     IN RvSipWarningHeaderHandle   hHeader,
                                     IN RvChar                    *pWarnText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipWarningHeaderSetWarnText(hHeader, pWarnText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipWarningHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object.
 ***************************************************************************/
RvInt32 SipWarningHeaderGetStrBadSyntax(IN  RvSipWarningHeaderHandle hHeader)
{
    return ((MsgWarningHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipWarningHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Warning header,
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
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetStrBadSyntax(
                                     IN  RvSipWarningHeaderHandle             hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgWarningHeader*)hHeader)->hPool,
                                  ((MsgWarningHeader*)hHeader)->hPage,
                                  SipWarningHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipWarningHeaderSetStrBadSyntax
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
RvStatus SipWarningHeaderSetStrBadSyntax(
                                  IN RvSipWarningHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgWarningHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgWarningHeader*)hHeader;

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
 * RvSipWarningHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Warning header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetStrBadSyntax(
                                  IN RvSipWarningHeaderHandle          hHeader,
                                  IN RvChar*						   strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipWarningHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipWarningHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Warning header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hWarningHeader  - Handle to the Warning header object.
 *           eStringName - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetRpoolString(
                             IN    RvSipWarningHeaderHandle      hWarningHeader,
                             IN    RvSipWarningHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgWarningHeader* pHeader = (MsgWarningHeader*)hWarningHeader;
    RvInt32                      requestedParamOffset;

    if(hWarningHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipWarningHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_WARNING_WARN_AGENT:
		requestedParamOffset = pHeader->strWarnAgent;
        break;
    case RVSIP_WARNING_WARN_TEXT:
        requestedParamOffset = pHeader->strWarnText;
        break;
    case RVSIP_WARNING_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipWarningHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipWarningHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipWarningHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Warning header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Warning header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                           new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetRpoolString(
                             IN RvSipWarningHeaderHandle       hHeader,
                             IN RvSipWarningHeaderStringName   eStringName,
                             IN RPOOL_Ptr                     *pRpoolPtr)
{
    MsgWarningHeader* pHeader;

    pHeader = (MsgWarningHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipWarningHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_WARNING_WARN_AGENT:
        return SipWarningHeaderSetWarnAgent(
									  hHeader,
									  NULL,
                                      pRpoolPtr->hPool,
                                      pRpoolPtr->hPage,
                                      pRpoolPtr->offset);
    case RVSIP_WARNING_WARN_TEXT:
        return SipWarningHeaderSetWarnText(hHeader,
										   NULL,
                                           pRpoolPtr->hPool,
                                           pRpoolPtr->hPage,
                                           pRpoolPtr->offset);
    case RVSIP_WARNING_BAD_SYNTAX:
        return SipWarningHeaderSetStrBadSyntax(hHeader,
											  NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipWarningHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * WarningHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void WarningHeaderClean(IN MsgWarningHeader*    pHeader,
                               IN RvBool               bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_WARNING;

	pHeader->warnCode          = UNDEFINED;
	pHeader->strWarnAgent      = UNDEFINED;
	pHeader->strWarnText       = UNDEFINED;


#ifdef SIP_DEBUG
	pHeader->pWarnAgent			= NULL;
	pHeader->pWarnText			= NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
