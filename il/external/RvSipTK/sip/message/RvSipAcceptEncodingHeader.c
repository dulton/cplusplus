/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptEncodingHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the AcceptEncoding header object:          *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Aug 2005                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

#include "RvSipAcceptEncodingHeader.h"
#include "_SipAcceptEncodingHeader.h"
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
static void AcceptEncodingHeaderClean(IN MsgAcceptEncodingHeader* pHeader,
                                      IN RvBool                   bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptEncodingHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept-Encoding header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept-Encoding 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptEncodingHeaderHandle      *hHeader)
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

    stat = RvSipAcceptEncodingHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_ACCEPT_ENCODING,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipAcceptEncodingHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept-Encoding Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept-Encoding header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptEncodingHeaderHandle *hHeader)
{
    MsgAcceptEncodingHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAcceptEncodingHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgAcceptEncodingHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAcceptEncodingHeaderConstruct - Allocation failed. hPool 0x%p, hPage %d",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
	
    AcceptEncodingHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAcceptEncodingHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAcceptEncodingHeaderConstruct - AcceptEncoding header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept-Encoding header object to a destination 
 *          AcceptEncoding header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AcceptEncoding header object.
 *    hSource      - Handle to the source AcceptEncoding header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderCopy(
									 IN    RvSipAcceptEncodingHeaderHandle hDestination,
                                     IN    RvSipAcceptEncodingHeaderHandle hSource)
{
    MsgAcceptEncodingHeader* source, *dest;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAcceptEncodingHeader*)hSource;
    dest   = (MsgAcceptEncodingHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;
	
	/* coding */
	if(source->strCoding > UNDEFINED)
	{
		dest->strCoding = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strCoding);
		if (dest->strCoding == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAcceptEncodingHeaderCopy: Failed to copy strCodings. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pCoding = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strCoding);
#endif
	}
	else
	{
		dest->strCoding = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pCoding = NULL;
#endif
	}

	/* q-value */
	if(source->strQVal > UNDEFINED)
	{
		dest->strQVal = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strQVal);
		if (dest->strQVal == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAcceptEncodingHeaderCopy: Failed to copy strQVal. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pQVal = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strQVal);
#endif
	}
	else
	{
		dest->strQVal = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pQVal = NULL;
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
						"RvSipAcceptEncodingHeaderCopy: Failed to copy other params. hDest 0x%p",
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
						"RvSipAcceptEncodingHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipAcceptEncodingHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept-Encoding header object to a textual Accept-Encoding header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept-Encoding header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderEncode(
                                          IN    RvSipAcceptEncodingHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgAcceptEncodingHeader* pHeader;

    pHeader = (MsgAcceptEncodingHeader*)hHeader;
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
                "RvSipAcceptEncodingHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptEncodingHeaderEncode - Got new page %d on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AcceptEncodingHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptEncodingHeaderEncode - Failed. Free page %d on pool 0x%p. AcceptEncodingHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AcceptEncodingHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept-Encoding header (as string) on it.
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
RvStatus RVCALLCONV AcceptEncodingHeaderEncode(
                                      IN    RvSipAcceptEncodingHeaderHandle     hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length)
{
    MsgAcceptEncodingHeader*  pHeader = (MsgAcceptEncodingHeader*)hHeader;
    RvStatus				  stat;
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AcceptEncodingHeaderEncode - Encoding AcceptEncoding header. hHeader 0x%p, hPool 0x%p, hPage %d",
             hHeader, hPool, hPage));

	/* encode "Accept-Encoding" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Accept-Encoding", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
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
			      "AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
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
                "AcceptEncodingHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage %d",
                stat, hPool, hPage));
        }
        return stat;
    }

	/* encode coding */
    if(pHeader->strCoding > UNDEFINED)
    {
        /* set pHeader->strCodings */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCoding,
                                    length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptEncodingHeaderEncode - Failed to encode Accept-Encoding header. RvStatus is %d, hPool 0x%p, hPage %d",
					stat, hPool, hPage));
			return stat;
		}
    }
	else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptEncodingHeaderEncode - Failed to encode Accept-Encoding header - missing mandatory coding. RvStatus is %d, hHeader %d",
					stat, pHeader));
		return RV_ERROR_UNKNOWN;
	}


	/* encode q-val */
    if(pHeader->strQVal > UNDEFINED)
    {
        /* set ";q=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "q", length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
		
        /* set pHeader->strQVal */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strQVal,
									length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptEncodingHeaderEncode - Failed to encode AcceptEncoding header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipAcceptEncodingHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AcceptEncoding header into a AcceptEncoding header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AcceptEncoding header object.
 *    buffer    - Buffer containing a textual AcceptEncoding header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderParse(
                                     IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgAcceptEncodingHeader* pHeader = (MsgAcceptEncodingHeader*)hHeader;
    RvStatus                 rv;

    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptEncodingHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT_ENCODING,
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
 * RvSipAcceptEncodingHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept-Encoding header value into an Accept-Encoding header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptEncodingHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept-Encoding header object.
 *    buffer    - The buffer containing a textual AcceptEncoding header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderParseValue(
                                     IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN RvChar*                          buffer)
{
    MsgAcceptEncodingHeader* pHeader = (MsgAcceptEncodingHeader*)hHeader;
    RvStatus                 rv;
	
    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptEncodingHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT_ENCODING,
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
 * RvSipAcceptEncodingHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AcceptEncoding header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderFix(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgAcceptEncodingHeader* pHeader = (MsgAcceptEncodingHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAcceptEncodingHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptEncodingHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptEncodingHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAcceptEncodingHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AcceptEncoding object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptEncodingHeaderGetPool(RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAcceptEncodingHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AcceptEncoding object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptEncodingHeaderGetPage(RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AcceptEncoding header fields are kept in a string format, for
 *          example, the AcceptEncoding header YYY string. In order to get such a field
 *          from the AcceptEncoding header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AcceptEncoding header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptEncodingHeaderGetStringLength(
                                      IN  RvSipAcceptEncodingHeaderHandle     hHeader,
                                      IN  RvSipAcceptEncodingHeaderStringName eStringName)
{
    RvInt32                  stringOffset = UNDEFINED;
    MsgAcceptEncodingHeader* pHeader      = (MsgAcceptEncodingHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
        case RVSIP_ACCEPT_ENCODING_CODING:
        {
            stringOffset = SipAcceptEncodingHeaderGetCoding(hHeader);
            break;
        }
		case RVSIP_ACCEPT_ENCODING_QVAL:
		{
			stringOffset = SipAcceptEncodingHeaderGetQVal(hHeader);
			break;
		}
		case RVSIP_ACCEPT_ENCODING_OTHER_PARAMS:
		{
			stringOffset = SipAcceptEncodingHeaderGetOtherParams(hHeader);
			break;
		}
		case RVSIP_ACCEPT_ENCODING_BAD_SYNTAX:
		{
			stringOffset = SipAcceptEncodingHeaderGetStrBadSyntax(hHeader);
			break;
		}
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipAcceptEncodingHeaderGetStringLength - Unknown stringName %d", eStringName));
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
 * SipAcceptEncodingHeaderGetCoding
 * ------------------------------------------------------------------------
 * General:This method gets the Coding string from the Accept-Encoding header.
 * Return Value: Coding offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Encoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetCoding(IN RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->strCoding;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetCoding
 * ------------------------------------------------------------------------
 * General: Copies the Coding string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Encoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetCoding(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAcceptEncodingHeader*)hHeader)->hPool,
                                  ((MsgAcceptEncodingHeader*)hHeader)->hPage,
                                  SipAcceptEncodingHeaderGetCoding(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptEncodingHeaderSetCoding
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Coding string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pCoding      - The Coding string to be set in the AcceptEncoding header - If
 *                      NULL, the exist Coding string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetCoding(IN  RvSipAcceptEncodingHeaderHandle   hHeader,
										  IN  RvChar                           *pCoding,
										  IN  HRPOOL                            hPool,
										  IN  HPAGE                             hPage,
										  IN  RvInt32                           strOffset)
{
    RvInt32                      newStr;
    MsgAcceptEncodingHeader      *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptEncodingHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pCoding,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strCoding  = newStr;
#ifdef SIP_DEBUG
    pHeader->pCoding = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                  pHeader->strCoding);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetCoding
 * ------------------------------------------------------------------------
 * General:Sets the Coding string in the Accept-Encoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptEncoding header object.
 *    pCoding    - The Coding string to be set in the Accept-Encoding header. 
 *                 If NULL is supplied, the existing Coding is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetCoding(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                           *pCoding)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptEncodingHeaderSetCoding(hHeader, pCoding, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptEncodingHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept-Encoding header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Encoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetQVal(IN RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->strQVal;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Encoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetQVal(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAcceptEncodingHeader*)hHeader)->hPool,
                                  ((MsgAcceptEncodingHeader*)hHeader)->hPage,
                                  SipAcceptEncodingHeaderGetQVal(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptEncodingHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pQVal        - The QVal string to be set in the AcceptEncoding header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetQVal(IN  RvSipAcceptEncodingHeaderHandle   hHeader,
										IN  RvChar                           *pQVal,
										IN  HRPOOL                            hPool,
										IN  HPAGE                             hPage,
										IN  RvInt32                           strOffset)
{
    RvInt32                      newStr;
    MsgAcceptEncodingHeader      *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptEncodingHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pQVal,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strQVal  = newStr;
#ifdef SIP_DEBUG
    pHeader->pQVal = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                  pHeader->strQVal);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept-Encoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept-Encoding header object.
 *    pQVal      - The QVal string to be set in the Accept-Encoding header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetQVal(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                           *pQVal)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptEncodingHeaderSetQVal(hHeader, pQVal, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptEncodingHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the AcceptEncoding header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptEncoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetOtherParams(IN RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AcceptEncoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetOtherParams(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptEncodingHeader*)hHeader)->hPool,
                                  ((MsgAcceptEncodingHeader*)hHeader)->hPage,
                                  SipAcceptEncodingHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptEncodingHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pOtherParams - The other-params to be set in the AcceptEncoding header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetOtherParams(
							    IN  RvSipAcceptEncodingHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgAcceptEncodingHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptEncodingHeader*)hHeader;

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
 * RvSipAcceptEncodingHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the AcceptEncoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptEncoding header object.
 *    pText      - The other-params string to be set in the AcceptEncoding header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetOtherParams(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptEncodingHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptEncodingHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptEncoding header object.
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetStrBadSyntax(IN  RvSipAcceptEncodingHeaderHandle hHeader)
{
    return ((MsgAcceptEncodingHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AcceptEncoding header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAcceptEncodingHeader*)hHeader)->hPool,
                                  ((MsgAcceptEncodingHeader*)hHeader)->hPage,
                                  SipAcceptEncodingHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptEncodingHeaderSetStrBadSyntax
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
RvStatus SipAcceptEncodingHeaderSetStrBadSyntax(
                                  IN RvSipAcceptEncodingHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgAcceptEncodingHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAcceptEncodingHeader*)hHeader;

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
 * RvSipAcceptEncodingHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AcceptEncoding header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetStrBadSyntax(
                                  IN RvSipAcceptEncodingHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptEncodingHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AcceptEncodingHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AcceptEncodingHeaderClean( 
								      IN MsgAcceptEncodingHeader* pHeader,
                                      IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_ACCEPT_ENCODING;
	
	pHeader->strCoding      = UNDEFINED;
	pHeader->strQVal        = UNDEFINED;
	pHeader->strOtherParams = UNDEFINED;
	
	
#ifdef SIP_DEBUG
	pHeader->pCoding        = NULL;
	pHeader->pQVal          = NULL;
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
