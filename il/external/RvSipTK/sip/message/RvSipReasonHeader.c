/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReasonHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the Reason header object:                  *
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

#include "RvSipReasonHeader.h"
#include "_SipReasonHeader.h"
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
static void ReasonHeaderClean(IN MsgReasonHeader* pHeader,
                              IN RvBool           bCleanBS);

static RvStatus RVCALLCONV EncodeReasonProtocol(
										IN  MsgReasonHeader*         pHeader,
										IN  HRPOOL                   hPool,
										IN  HPAGE                    hPage,
										OUT RvUint32*                length);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReasonHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Reason header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Reason
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipReasonHeaderHandle             *hHeader)
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

    stat = RvSipReasonHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_REASON,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipReasonHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Reason Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipReasonHeaderHandle *hHeader)
{
    MsgReasonHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgReasonHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgReasonHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReasonHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    ReasonHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipReasonHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReasonHeaderConstruct - Reason header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipReasonHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Reason header object to a destination
 *          Reason header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Reason header object.
 *    hSource      - Handle to the source Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderCopy(
                                     IN    RvSipReasonHeaderHandle hDestination,
                                     IN    RvSipReasonHeaderHandle hSource)
{
    MsgReasonHeader* source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgReasonHeader*)hSource;
    dest   = (MsgReasonHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;

	/* protocol-cause */
	dest->protocolCause = source->protocolCause;

	/* protocol */
	dest->eProtocol = source->eProtocol;
	if((source->strProtocol > UNDEFINED) && (dest->eProtocol == RVSIP_REASON_PROTOCOL_OTHER))
	{
		dest->strProtocol = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strProtocol);
		if (dest->strProtocol == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipReasonHeaderCopy: Failed to copy protocol. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pProtocol = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strProtocol);
#endif
	}
	else
	{
		dest->strProtocol = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pProtocol = NULL;
#endif
	}

	/* reason text */
	if(source->strReasonText > UNDEFINED)
	{
		dest->strReasonText = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strReasonText);
		if (dest->strReasonText == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipReasonHeaderCopy: Failed to copy reson-text. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pReasonText = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												   dest->strReasonText);
#endif
	}
	else
	{
		dest->strReasonText = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pReasonText = NULL;
#endif
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
						"RvSipReasonHeaderCopy: Failed to copy other params. hDest 0x%p",
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
						"RvSipReasonHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipReasonHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Reason header object to a textual Reason header. The
 *          textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Reason header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderEncode(
                                          IN    RvSipReasonHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgReasonHeader* pHeader;

    pHeader = (MsgReasonHeader*)hHeader;
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
                "RvSipReasonHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReasonHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ReasonHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReasonHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ReasonHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ReasonHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Reason header (as string) on it.
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
RvStatus RVCALLCONV ReasonHeaderEncode(
                                      IN    RvSipReasonHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length)
{
    MsgReasonHeader*  pHeader = (MsgReasonHeader*)hHeader;
    RvStatus          stat;
	RvChar            strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ReasonHeaderEncode - Encoding Reason header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

	/* encode "Reason" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Reason", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
			      "ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "ReasonHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

	/* encode protocol */
	stat = EncodeReasonProtocol(pHeader, hPool, hPage, length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				  "ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			      stat, hPool, hPage));
		return stat;
	}

	/* protocol cause */
	if(pHeader->protocolCause > UNDEFINED)
	{
		/* set ";cause=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "cause", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
			MsgUtilsGetEqualChar(bInUrlHeaders),length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
		MsgUtils_itoa(pHeader->protocolCause , strHelper);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}

	/* encode reason-text */
    if(pHeader->strReasonText > UNDEFINED)
    {
        /* set ";text=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "text", length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}

        /* set pHeader->strReasonText */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strReasonText,
                                    length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
    }

    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
				"ReasonHeaderEncode - Failed to encode Reason header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipReasonHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Reason header into a Reason header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Reason header object.
 *    buffer    - Buffer containing a textual Reason header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderParse(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgReasonHeader* pHeader = (MsgReasonHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ReasonHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REASON,
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
 * RvSipReasonHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Reason header value into an Reason header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReasonHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Reason header object.
 *    buffer    - The buffer containing a textual Reason header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderParseValue(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgReasonHeader* pHeader = (MsgReasonHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ReasonHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REASON,
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
 * RvSipReasonHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Reason header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderFix(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgReasonHeader* pHeader = (MsgReasonHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipReasonHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipReasonHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipReasonHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipReasonHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Reason object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipReasonHeaderGetPool(RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipReasonHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Reason object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipReasonHeaderGetPage(RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipReasonHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Reason header fields are kept in a string format, for
 *          example, the Reason header next-nonce string. In order to get such a field
 *          from the Reason header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Reason header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReasonHeaderGetStringLength(
                                      IN  RvSipReasonHeaderHandle     hHeader,
                                      IN  RvSipReasonHeaderStringName eStringName)
{
    RvInt32          stringOffset = UNDEFINED;
    MsgReasonHeader* pHeader      = (MsgReasonHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
        case RVSIP_REASON_PROTOCOL:
        {
            stringOffset = SipReasonHeaderGetStrProtocol(hHeader);
            break;
        }
        case RVSIP_REASON_TEXT:
        {
            stringOffset = SipReasonHeaderGetText(hHeader);
            break;
        }
        case RVSIP_REASON_OTHER_PARAMS:
        {
            stringOffset = SipReasonHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_REASON_BAD_SYNTAX:
        {
            stringOffset = SipReasonHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipReasonHeaderGetStringLength - Unknown stringName %d", eStringName));
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
 * SipReasonHeaderGetText
 * ------------------------------------------------------------------------
 * General:This method gets the reason-text string from the Reason header.
 * Return Value: reason-text offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object..
 ***************************************************************************/
RvInt32 SipReasonHeaderGetText(IN RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->strReasonText;
}

/***************************************************************************
 * RvSipReasonHeaderGetText
 * ------------------------------------------------------------------------
 * General: Copies the reason-text string from the Reason header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetText(
                                       IN RvSipReasonHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReasonHeader*)hHeader)->hPool,
                                  ((MsgReasonHeader*)hHeader)->hPage,
                                  SipReasonHeaderGetText(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReasonHeaderSetText
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the reason-text string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Reason header object.
 *       pText        - The reason text to be set in the Reason header - If
 *                      NULL, the exist reason text string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetText(IN  RvSipReasonHeaderHandle   hHeader,
                                IN  RvChar                    *pText,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgReasonHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReasonHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pText,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strReasonText  = newStr;
#ifdef SIP_DEBUG
    pHeader->pReasonText = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                  pHeader->strReasonText);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReasonHeaderSetText
 * ------------------------------------------------------------------------
 * General:Sets the reason-text string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Reason header object.
 *    pText      - The reason-text string to be set in the Reason header.
 *                 If NULL is supplied, the existing reason-text is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetText(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReasonHeaderSetText(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipReasonHeaderGetCause
 * ------------------------------------------------------------------------
 * General: Gets the protocol-cause value from the Reason Header object.
 * Return Value: Returns the protocol-cause value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Reason header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipReasonHeaderGetCause(
                                    IN RvSipReasonHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgReasonHeader*)hHeader)->protocolCause;
}


/***************************************************************************
 * RvSipReasonHeaderSetCause
 * ------------------------------------------------------------------------
 * General: Sets the protocol-cause value in the Reason Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Reason header object.
 *    cause             - The protocol-cause value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetCause(
                                 IN    RvSipReasonHeaderHandle hHeader,
                                 IN    RvInt32                 cause)
{
	MsgReasonHeader *pHeader = (MsgReasonHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

	pHeader->protocolCause = cause;
    return RV_OK;
}

/***************************************************************************
 * RvSipReasonHeaderGetProtocol
 * ------------------------------------------------------------------------
 * General: Gets the protocol enumeration from the Reason object.
 * Return Value: The protocol from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Returns the protocol enumeration from the object.
 ***************************************************************************/
RVAPI RvSipReasonProtocolType  RVCALLCONV RvSipReasonHeaderGetProtocol(
                                      IN  RvSipReasonHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_REASON_PROTOCOL_UNDEFINED;

    return ((MsgReasonHeader*)hHeader)->eProtocol;
}

/***************************************************************************
 * SipReasonHeaderGetStrProtocol
 * ------------------------------------------------------------------------
 * General:This method gets the protocol string from the Reason header.
 * Return Value: protocol or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object.
 ***************************************************************************/
RvInt32 SipReasonHeaderGetStrProtocol(IN RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->strProtocol;
}

/***************************************************************************
 * RvSipReasonHeaderGetStrProtocol
 * ------------------------------------------------------------------------
 * General: Copies the protocol string value of the Reason object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a
 *                     NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetStrProtocol(
                                       IN RvSipReasonHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReasonHeader*)hHeader)->hPool,
                                  ((MsgReasonHeader*)hHeader)->hPage,
                                  SipReasonHeaderGetStrProtocol(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReasonHeaderSetProtocol
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the protocol string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader   - Handle of the Reason header object.
 *       eProtocol - The protocol enumeration value to be set in the object.
 *       pProtocol - The protocol to be set in the Reason header - If
 *                   NULL, the exist protocol string in the header will be removed.
 *       offset    - Offset of a string on the page (if relevant).
 *       hPool     - The pool on which the string lays (if relevant).
 *       hPage     - The page on which the stringlays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetProtocol(IN RvSipReasonHeaderHandle  hHeader,
                                    IN RvSipReasonProtocolType  eProtocol,
									IN RvChar                  *pProtocol,
                                    IN HRPOOL                   hPool,
                                    IN HPAGE                    hPage,
                                    IN RvInt32                  offset)
{
    RvInt32                      newStr;
    MsgReasonHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReasonHeader*)hHeader;

	pHeader->eProtocol       = eProtocol;

	if (eProtocol == RVSIP_REASON_PROTOCOL_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage,
									  offset, pProtocol,
									  pHeader->hPool,pHeader->hPage,
									  &newStr);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}


		pHeader->strProtocol     = newStr;
#ifdef SIP_DEBUG
		pHeader->pProtocol = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
												    pHeader->strProtocol);
#endif
	}
	else
	{
		pHeader->strProtocol = UNDEFINED;
#ifdef SIP_DEBUG
		pHeader->pProtocol   = NULL;
#endif
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipReasonHeaderSetProtocol
 * ------------------------------------------------------------------------
 * General:Sets the protocol string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Reason header object.
 *    eProtocol   - The protocol enumeration value to be set in the object.
 *    strProtocol - You can use this parametere only if the eProtocol parameter is set to
 *                RVSIP_REASON_PROTOCOL_OTHER. In this case you can supply the
 *                protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetProtocol(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvSipReasonProtocolType  eProtocol,
									 IN RvChar *                 strProtocol)
{
	if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

	return SipReasonHeaderSetProtocol(hHeader, eProtocol, strProtocol, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReasonHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Reason header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object..
 ***************************************************************************/
RvInt32 SipReasonHeaderGetOtherParams(IN RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipReasonHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Reason header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetOtherParams(
                                       IN RvSipReasonHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReasonHeader*)hHeader)->hPool,
                                  ((MsgReasonHeader*)hHeader)->hPage,
                                  SipReasonHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReasonHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Reason header object.
 *       pOtherParams - The other-params to be set in the Reason header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetOtherParams(
							    IN  RvSipReasonHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgReasonHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReasonHeader*)hHeader;

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
 * RvSipReasonHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Reason header object.
 *    pText      - The other-params string to be set in the Reason header.
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetOtherParams(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReasonHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReasonHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object.
 ***************************************************************************/
RvInt32 SipReasonHeaderGetStrBadSyntax(IN  RvSipReasonHeaderHandle hHeader)
{
    return ((MsgReasonHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipReasonHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Reason header,
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
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetStrBadSyntax(
                                     IN  RvSipReasonHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgReasonHeader*)hHeader)->hPool,
                                  ((MsgReasonHeader*)hHeader)->hPage,
                                  SipReasonHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReasonHeaderSetStrBadSyntax
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
RvStatus SipReasonHeaderSetStrBadSyntax(
                                  IN RvSipReasonHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgReasonHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgReasonHeader*)hHeader;

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
 * RvSipReasonHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Reason header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetStrBadSyntax(
                                  IN RvSipReasonHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReasonHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipReasonHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Reason header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hReasonHeader - Handle to the Reason header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetRpoolString(
                             IN    RvSipReasonHeaderHandle      hReasonHeader,
                             IN    RvSipReasonHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgReasonHeader* pHeader = (MsgReasonHeader*)hReasonHeader;
    RvInt32                      requestedParamOffset;

    if(hReasonHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipReasonHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_REASON_PROTOCOL:
        requestedParamOffset = pHeader->strProtocol;
        break;
    case RVSIP_REASON_TEXT:
        requestedParamOffset = pHeader->strReasonText;
        break;
    case RVSIP_REASON_OTHER_PARAMS:
        requestedParamOffset = pHeader->strOtherParams;
        break;
    case RVSIP_REASON_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipReasonHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipReasonHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipReasonHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Reason header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Reason header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetRpoolString(
                             IN RvSipReasonHeaderHandle       hHeader,
                             IN RvSipReasonHeaderStringName   eStringName,
                             IN RPOOL_Ptr                    *pRpoolPtr)
{
    MsgReasonHeader* pHeader;

    pHeader = (MsgReasonHeader*)hHeader;
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
                 "RvSipReasonHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_REASON_PROTOCOL:
        return SipReasonHeaderSetProtocol(hHeader,
										  RVSIP_REASON_PROTOCOL_OTHER,
										  NULL,
                                          pRpoolPtr->hPool,
                                          pRpoolPtr->hPage,
                                          pRpoolPtr->offset);
    case RVSIP_REASON_TEXT:
        return SipReasonHeaderSetText(hHeader,
									  NULL,
                                      pRpoolPtr->hPool,
                                      pRpoolPtr->hPage,
                                      pRpoolPtr->offset);
    case RVSIP_REASON_OTHER_PARAMS:
        return SipReasonHeaderSetOtherParams(hHeader,
											 NULL,
                                             pRpoolPtr->hPool,
                                             pRpoolPtr->hPage,
                                             pRpoolPtr->offset);
    case RVSIP_REASON_BAD_SYNTAX:
        return SipReasonHeaderSetStrBadSyntax(hHeader,
											  NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReasonHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ReasonHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ReasonHeaderClean(
								      IN MsgReasonHeader* pHeader,
                                      IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_REASON;

	pHeader->protocolCause  = UNDEFINED;
	pHeader->strOtherParams = UNDEFINED;
	pHeader->strReasonText  = UNDEFINED;
	pHeader->strProtocol    = UNDEFINED;
	pHeader->eProtocol      = RVSIP_REASON_PROTOCOL_UNDEFINED;

#ifdef SIP_DEBUG
	pHeader->pProtocol      = NULL;
	pHeader->pReasonText    = NULL;
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

/***************************************************************************
 * EncodeReasonProtocol
 * ------------------------------------------------------------------------
 * General: The function adds the string of the reson protocol to the page,
 *          without '/0'.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hPage      - Handle of the memory page for encoded buffer.
 * Output: encoded    - Pointer to the beginning of the encoded string in the page.
 *         length     - The length that was given + the new encoded string length.
 ***************************************************************************/
static RvStatus RVCALLCONV EncodeReasonProtocol(
                                   IN  MsgReasonHeader*         pHeader,
                                   IN  HRPOOL                   hPool,
                                   IN  HPAGE                    hPage,
                                   OUT RvUint32*                length)
{
    RvStatus status;
	MsgMgr*  pMsgMgr = pHeader->pMsgMgr;

    switch(pHeader->eProtocol)
    {
        case RVSIP_REASON_PROTOCOL_SIP:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "SIP", length);
            return status;
        }
        case RVSIP_REASON_PROTOCOL_Q_850:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "Q.850", length);
            return status;
        }
        case RVSIP_REASON_PROTOCOL_OTHER:
        {
			status = MsgUtilsEncodeString(pMsgMgr,
										  hPool,
										  hPage,
										  pHeader->hPool,
										  pHeader->hPage,
										  pHeader->strProtocol,
										  length);
			return status;
        }
        case RVSIP_REASON_PROTOCOL_UNDEFINED:
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                       "EncodeReasonProtocol - Undefined protocol"));
            return RV_ERROR_BADPARAM;
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                       "EncodeReasonProtocol - Unknown eProtocol: %d", pHeader->eProtocol));
            return RV_ERROR_BADPARAM;
        }
    }
}
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */


