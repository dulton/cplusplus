/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptLanguageHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the AcceptLanguage header object:          *
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

#include "RvSipAcceptLanguageHeader.h"
#include "_SipAcceptLanguageHeader.h"
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
static void AcceptLanguageHeaderClean(IN MsgAcceptLanguageHeader* pHeader,
                                      IN RvBool                   bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptLanguageHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept-Language header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept-Language 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptLanguageHeaderHandle      *hHeader)
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

    stat = RvSipAcceptLanguageHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_ACCEPT_LANGUAGE,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipAcceptLanguageHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept-Language Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept-Language header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptLanguageHeaderHandle *hHeader)
{
    MsgAcceptLanguageHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAcceptLanguageHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgAcceptLanguageHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAcceptLanguageHeaderConstruct - Allocation failed. hPool 0x%p, hPage %d",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
	
    AcceptLanguageHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAcceptLanguageHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAcceptLanguageHeaderConstruct - AcceptLanguage header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept-Language header object to a destination 
 *          AcceptLanguage header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AcceptLanguage header object.
 *    hSource      - Handle to the source AcceptLanguage header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderCopy(
                                     IN    RvSipAcceptLanguageHeaderHandle hDestination,
                                     IN    RvSipAcceptLanguageHeaderHandle hSource)
{
    MsgAcceptLanguageHeader* source, *dest;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAcceptLanguageHeader*)hSource;
    dest   = (MsgAcceptLanguageHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;
	
	/*  */
	if(source->strLanguage > UNDEFINED)
	{
		dest->strLanguage = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strLanguage);
		if (dest->strLanguage == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAcceptLanguageHeaderCopy: Failed to copy strLanguages. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pLanguage = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strLanguage);
#endif
	}
	else
	{
		dest->strLanguage = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pLanguage = NULL;
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
						"RvSipAcceptLanguageHeaderCopy: Failed to copy strQVal. hDest 0x%p",
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
						"RvSipAcceptLanguageHeaderCopy: Failed to copy other params. hDest 0x%p",
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
						"RvSipAcceptLanguageHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipAcceptLanguageHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept-Language header object to a textual Accept-Language header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept-Language header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderEncode(
                                          IN    RvSipAcceptLanguageHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgAcceptLanguageHeader* pHeader;

    pHeader = (MsgAcceptLanguageHeader*)hHeader;
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
                "RvSipAcceptLanguageHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptLanguageHeaderEncode - Got new page %d on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AcceptLanguageHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptLanguageHeaderEncode - Failed. Free page %d on pool 0x%p. AcceptLanguageHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AcceptLanguageHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept-Language header (as string) on it.
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
RvStatus RVCALLCONV AcceptLanguageHeaderEncode(
                                      IN    RvSipAcceptLanguageHeaderHandle     hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length)
{
    MsgAcceptLanguageHeader*  pHeader = (MsgAcceptLanguageHeader*)hHeader;
    RvStatus				  stat;
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AcceptLanguageHeaderEncode - Language AcceptLanguage header. hHeader 0x%p, hPool 0x%p, hPage %d",
             hHeader, hPool, hPage));

	/* encode "Accept-Language" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Accept-Language", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
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
			      "AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
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
                "AcceptLanguageHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage %d",
                stat, hPool, hPage));
        }
        return stat;
    }

	/* encode Language */
    if(pHeader->strLanguage > UNDEFINED)
    {
        /* set pHeader->strLanguages */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strLanguage,
                                    length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptLanguageHeaderEncode - Failed to encode Accept-Language header. RvStatus is %d, hPool 0x%p, hPage %d",
					stat, hPool, hPage));
			return stat;
		}
    }
	else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptLanguageHeaderEncode - Failed to encode Accept-Language header - missing mandatory Language. RvStatus is %d, hHeader %d",
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
						"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "q", length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
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
						"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptLanguageHeaderEncode - Failed to encode AcceptLanguage header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipAcceptLanguageHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AcceptLanguage header into a AcceptLanguage header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AcceptLanguage header object.
 *    buffer    - Buffer containing a textual AcceptLanguage header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderParse(
                                     IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgAcceptLanguageHeader* pHeader = (MsgAcceptLanguageHeader*)hHeader;
    RvStatus                 rv;

    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptLanguageHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT_LANGUAGE,
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
 * RvSipAcceptLanguageHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept-Language header value into an Accept-Language header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptLanguageHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept-Language header object.
 *    buffer    - The buffer containing a textual AcceptLanguage header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderParseValue(
                                     IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN RvChar*                          buffer)
{
    MsgAcceptLanguageHeader* pHeader = (MsgAcceptLanguageHeader*)hHeader;
    RvStatus                 rv;
	
    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptLanguageHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT_LANGUAGE,
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
 * RvSipAcceptLanguageHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AcceptLanguage header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderFix(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgAcceptLanguageHeader* pHeader = (MsgAcceptLanguageHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAcceptLanguageHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptLanguageHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptLanguageHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAcceptLanguageHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AcceptLanguage object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptLanguageHeaderGetPool(RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAcceptLanguageHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AcceptLanguage object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptLanguageHeaderGetPage(RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AcceptLanguage header fields are kept in a string format, for
 *          example, the AcceptLanguage header YYY string. In order to get such a field
 *          from the AcceptLanguage header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AcceptLanguage header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptLanguageHeaderGetStringLength(
                                      IN  RvSipAcceptLanguageHeaderHandle     hHeader,
                                      IN  RvSipAcceptLanguageHeaderStringName eStringName)
{
    RvInt32                  stringOffset = UNDEFINED;
    MsgAcceptLanguageHeader* pHeader      = (MsgAcceptLanguageHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
        case RVSIP_ACCEPT_LANGUAGE_LANGUAGE:
        {
            stringOffset = SipAcceptLanguageHeaderGetLanguage(hHeader);
            break;
        }
		case RVSIP_ACCEPT_LANGUAGE_QVAL:
		{
			stringOffset = SipAcceptLanguageHeaderGetQVal(hHeader);
			break;
		}
		case RVSIP_ACCEPT_LANGUAGE_OTHER_PARAMS:
		{
			stringOffset = SipAcceptLanguageHeaderGetOtherParams(hHeader);
			break;
		}
		case RVSIP_ACCEPT_LANGUAGE_BAD_SYNTAX:
		{
			stringOffset = SipAcceptLanguageHeaderGetStrBadSyntax(hHeader);
			break;
		}
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipAcceptLanguageHeaderGetStringLength - Unknown stringName %d", eStringName));
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
 * SipAcceptLanguageHeaderGetLanguage
 * ------------------------------------------------------------------------
 * General:This method gets the Language string from the Accept-Language header.
 * Return Value: Language offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Language header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetLanguage(IN RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->strLanguage;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetLanguage
 * ------------------------------------------------------------------------
 * General: Copies the Language string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Language header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetLanguage(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptLanguageHeader*)hHeader)->hPool,
                                  ((MsgAcceptLanguageHeader*)hHeader)->hPage,
                                  SipAcceptLanguageHeaderGetLanguage(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptLanguageHeaderSetLanguage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Language string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pLanguage      - The Language string to be set in the AcceptLanguage header - If
 *                      NULL, the exist Language string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetLanguage(IN  RvSipAcceptLanguageHeaderHandle   hHeader,
										  IN  RvChar                           *pLanguage,
										  IN  HRPOOL                            hPool,
										  IN  HPAGE                             hPage,
										  IN  RvInt32                           strOffset)
{
    RvInt32                      newStr;
    MsgAcceptLanguageHeader      *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptLanguageHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pLanguage,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	pHeader->strLanguage  = newStr;
#ifdef SIP_DEBUG
    pHeader->pLanguage = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                  pHeader->strLanguage);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderSetLanguage
 * ------------------------------------------------------------------------
 * General:Sets the Language string in the Accept-Language header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptLanguage header object.
 *    pLanguage    - The Language string to be set in the Accept-Language header. 
 *                 If NULL is supplied, the existing Language is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetLanguage(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                           *pLanguage)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptLanguageHeaderSetLanguage(hHeader, pLanguage, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptLanguageHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept-Language header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Language header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetQVal(IN RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->strQVal;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Language header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetQVal(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptLanguageHeader*)hHeader)->hPool,
                                  ((MsgAcceptLanguageHeader*)hHeader)->hPage,
                                  SipAcceptLanguageHeaderGetQVal(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptLanguageHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pQVal        - The QVal string to be set in the AcceptLanguage header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetQVal(IN  RvSipAcceptLanguageHeaderHandle   hHeader,
										IN  RvChar                           *pQVal,
										IN  HRPOOL                            hPool,
										IN  HPAGE                             hPage,
										IN  RvInt32                           strOffset)
{
    RvInt32                      newStr;
    MsgAcceptLanguageHeader      *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptLanguageHeader*)hHeader;

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
 * RvSipAcceptLanguageHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept-Language header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept-Language header object.
 *    pQVal      - The QVal string to be set in the Accept-Language header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetQVal(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                           *pQVal)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptLanguageHeaderSetQVal(hHeader, pQVal, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptLanguageHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the AcceptLanguage header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptLanguage header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetOtherParams(IN RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AcceptLanguage header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetOtherParams(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptLanguageHeader*)hHeader)->hPool,
                                  ((MsgAcceptLanguageHeader*)hHeader)->hPage,
                                  SipAcceptLanguageHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptLanguageHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pOtherParams - The other-params to be set in the AcceptLanguage header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetOtherParams(
							    IN  RvSipAcceptLanguageHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgAcceptLanguageHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptLanguageHeader*)hHeader;

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
 * RvSipAcceptLanguageHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the AcceptLanguage header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptLanguage header object.
 *    pText      - The other-params string to be set in the AcceptLanguage header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetOtherParams(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptLanguageHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptLanguageHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptLanguage header object.
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetStrBadSyntax(IN  RvSipAcceptLanguageHeaderHandle hHeader)
{
    return ((MsgAcceptLanguageHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AcceptLanguage header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAcceptLanguageHeader*)hHeader)->hPool,
                                  ((MsgAcceptLanguageHeader*)hHeader)->hPage,
                                  SipAcceptLanguageHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptLanguageHeaderSetStrBadSyntax
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
RvStatus SipAcceptLanguageHeaderSetStrBadSyntax(
                                  IN RvSipAcceptLanguageHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgAcceptLanguageHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAcceptLanguageHeader*)hHeader;

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
 * RvSipAcceptLanguageHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AcceptLanguage header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetStrBadSyntax(
                                  IN RvSipAcceptLanguageHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptLanguageHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AcceptLanguageHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AcceptLanguageHeaderClean( 
								      IN MsgAcceptLanguageHeader* pHeader,
                                      IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_ACCEPT_LANGUAGE;
	
	pHeader->strLanguage      = UNDEFINED;
	pHeader->strQVal        = UNDEFINED;
	pHeader->strOtherParams = UNDEFINED;
	
	
#ifdef SIP_DEBUG
	pHeader->pLanguage        = NULL;
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
