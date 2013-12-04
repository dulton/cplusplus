/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptHeader.c                                    *
 *                                                                            *
 * The file defines the methods of the Accept header object:                  *
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

#include "RvSipAcceptHeader.h"
#include "_SipAcceptHeader.h"
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
static void AcceptHeaderClean(IN MsgAcceptHeader*         pHeader,
                              IN RvBool                   bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptHeaderHandle             *hHeader)
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

    stat = RvSipAcceptHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_ACCEPT,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipAcceptHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptHeaderHandle         *hHeader)
{
    MsgAcceptHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAcceptHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgAcceptHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
					"RvSipAcceptHeaderConstruct - Allocation failed. hPool 0x%p, hPage %d",
					hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
	
    AcceptHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAcceptHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAcceptHeaderConstruct - Accept header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept header object to a destination 
 *          Accept header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Accept header object.
 *    hSource      - Handle to the source Accept header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderCopy(
									 IN    RvSipAcceptHeaderHandle hDestination,
                                     IN    RvSipAcceptHeaderHandle hSource)
{
    MsgAcceptHeader* source, *dest;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAcceptHeader*)hSource;
    dest   = (MsgAcceptHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;
	
	/* media type and sub type */
    dest->eMediaSubType = source->eMediaSubType;
    dest->eMediaType    = source->eMediaType;
    
    /* media type string */
    if  ((source->eMediaType == RVSIP_MEDIATYPE_OTHER) &&
         (source->strMediaType > UNDEFINED))
    {
        dest->strMediaType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                             dest->hPool,
                                                             dest->hPage,
                                                             source->hPool,
                                                             source->hPage,
                                                             source->strMediaType);
        if (dest->strMediaType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipAcceptHeaderCopy - Failed to copy media type. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pMediaType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                            dest->strMediaType);
#endif
    }
    else
    {
        dest->strMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMediaType = NULL;
#endif
    }

    /* media sub type string */
    if((source->eMediaSubType == RVSIP_MEDIASUBTYPE_OTHER) &&
       (source->strMediaSubType > UNDEFINED))
    {
        dest->strMediaSubType = MsgUtilsAllocCopyRpoolString(
                                                          source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strMediaSubType);
        if (dest->strMediaSubType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipAcceptHeaderCopy - Failed to copy media sub type. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pMediaSubType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                               dest->strMediaSubType);
#endif
    }
    else
    {
        dest->strMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMediaSubType = NULL;
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
						"RvSipAcceptHeaderCopy: Failed to copy strQVal. hDest 0x%p",
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
						"RvSipAcceptHeaderCopy: Failed to copy other params. hDest 0x%p",
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
						"RvSipAcceptHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipAcceptHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept header object to a textual Accept header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderEncode(
                                          IN    RvSipAcceptHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgAcceptHeader* pHeader;

    pHeader = (MsgAcceptHeader*)hHeader;
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
                "RvSipAcceptHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptHeaderEncode - Got new page %d on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AcceptHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAcceptHeaderEncode - Failed. Free page %d on pool 0x%p. AcceptHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AcceptHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept header (as string) on it.
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
RvStatus RVCALLCONV AcceptHeaderEncode(
                                      IN    RvSipAcceptHeaderHandle     hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length)
{
    MsgAcceptHeader*  pHeader = (MsgAcceptHeader*)hHeader;
    RvStatus				  stat;
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AcceptHeaderEncode - Encoding Accept header. hHeader 0x%p, hPool 0x%p, hPage %d",
             hHeader, hPool, hPage));

	/* encode "Accept" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Accept", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
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
			      "AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
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
                "AcceptHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage %d",
                stat, hPool, hPage));
        }
        return stat;
    }

	/* set type "/" subtype */
    stat = MsgUtilsEncodeMediaType(pHeader->pMsgMgr, hPool, hPage, pHeader->eMediaType,
                                   pHeader->hPool, pHeader->hPage,
                                   pHeader->strMediaType, length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "AcceptHeaderEncode: Failed to encode Accept header. stat is %d",
                  stat));
        return stat;
    }
    /* / */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,  "/", length);
    stat = MsgUtilsEncodeMediaSubType(pHeader->pMsgMgr, hPool, hPage, pHeader->eMediaSubType,
                                      pHeader->hPool, pHeader->hPage,
                                      pHeader->strMediaSubType, length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "AcceptHeaderEncode: Failed to encode Accept header. stat is %d",
                  stat));
        return stat;
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
						"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "q", length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
						stat, hPool, hPage));
			return stat;
		}
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
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
						"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"AcceptHeaderEncode - Failed to encode Accept header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipAcceptHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept header into a Accept header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Accept header object.
 *    buffer    - Buffer containing a textual Accept header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderParse(
                                     IN RvSipAcceptHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgAcceptHeader* pHeader = (MsgAcceptHeader*)hHeader;
    RvStatus                 rv;

    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT,
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
 * RvSipAcceptHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept header value into an Accept header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept header object.
 *    buffer    - The buffer containing a textual Accept header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderParseValue(
                                     IN RvSipAcceptHeaderHandle  hHeader,
                                     IN RvChar*                          buffer)
{
    MsgAcceptHeader* pHeader = (MsgAcceptHeader*)hHeader;
    RvStatus                 rv;
	
    if(hHeader == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    AcceptHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_ACCEPT,
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
 * RvSipAcceptHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Accept header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderFix(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgAcceptHeader* pHeader = (MsgAcceptHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAcceptHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipAcceptHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAcceptHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Accept object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptHeaderGetPool(RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAcceptHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Accept object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptHeaderGetPage(RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->hPage;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Accept header fields are kept in a string format, for
 *          example, the Accept header YYY string. In order to get such a field
 *          from the Accept header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Accept header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptHeaderGetStringLength(
                                      IN  RvSipAcceptHeaderHandle     hHeader,
                                      IN  RvSipAcceptHeaderStringName eStringName)
{
    RvInt32                  stringOffset = UNDEFINED;
    MsgAcceptHeader* pHeader      = (MsgAcceptHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
        case RVSIP_ACCEPT_MEDIATYPE:
        {
            stringOffset = SipAcceptHeaderGetStrMediaType(hHeader);
            break;
        }
		case RVSIP_ACCEPT_MEDIASUBTYPE:
		{
            stringOffset = SipAcceptHeaderGetStrMediaSubType(hHeader);
            break;
        }
		case RVSIP_ACCEPT_QVAL:
		{
			stringOffset = SipAcceptHeaderGetQVal(hHeader);
			break;
		}
		case RVSIP_ACCEPT_OTHER_PARAMS:
		{
			stringOffset = SipAcceptHeaderGetOtherParams(hHeader);
			break;
		}
		case RVSIP_ACCEPT_BAD_SYNTAX:
		{
			stringOffset = SipAcceptHeaderGetStrBadSyntax(hHeader);
			break;
		}
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipAcceptHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * RvSipAcceptHeaderGetMediaType
 * ------------------------------------------------------------------------
 * General: Gets the media type enumeration value. If RVSIP_MEDIATYPE_OTHER
 *          is returned, you can receive the media type string using
 *          RvSipAcceptHeaderGetMediaTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaType RVCALLCONV RvSipAcceptHeaderGetMediaType(
                                   IN RvSipAcceptHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIATYPE_UNDEFINED;
    }
    return (((MsgAcceptHeader*)hHeader)->eMediaType);
}

/***************************************************************************
 * SipAcceptHeaderGetStrMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrMediaType(
                                     IN RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->strMediaType;
}

/***************************************************************************
 * RvSipAcceptHeaderGetStrMediaType
 * ------------------------------------------------------------------------
 * General: Copies the media type string from the Content-Type header into
 *          a given buffer. Use this function when the media type enumeration
 *          returned by RvSipAcceptGetMediaType() is RVSIP_MEDIATYPE_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrMediaType(
                                    IN RvSipAcceptHeaderHandle       hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAcceptHeader*)hHeader)->hPool,
                                  ((MsgAcceptHeader*)hHeader)->hPage,
                                  SipAcceptHeaderGetStrMediaType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - Handle of the Content-Type header object
 *          strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool     - The pool on which the string lays (if relevant).
 *          hPage     - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipAcceptHeaderSetMediaType(
                               IN  RvSipAcceptHeaderHandle     hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                      hPool,
                               IN  HPAGE                       hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgAcceptHeader*     pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strMediaType = newStr;
#ifdef SIP_DEBUG
    pHeader->pMediaType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strMediaType);
#endif
    /* set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eMediaType = RVSIP_MEDIATYPE_UNDEFINED;
    }
    else
    {
        pHeader->eMediaType = RVSIP_MEDIATYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: Sets the media type field in the Content-Type header object.
 *          If the enumeration given by eMediaType is RVSIP_MEDIATYPE_OTHER,
 *          sets the media type string given by strMediaType in the
 *          Content-Type header object. Use RVSIP_MEDIATYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaType - The media type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIATYPE_UNDEFINED is supplied, the
 *               existing media type is removed from the header.
 *    strMediaType - The media type string to be set in the Content-Type header.
 *                (relevant when eMediaType is RVSIP_MEDIATYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetMediaType(
                                 IN  RvSipAcceptHeaderHandle hHeader,
                                 IN  RvSipMediaType               eMediaType,
                                 IN  RvChar                     *strMediaType)
{
    MsgAcceptHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgAcceptHeader *)hHeader;

    pHeader->eMediaType = eMediaType;
    if (RVSIP_MEDIATYPE_OTHER == eMediaType)
    {
        stat = SipAcceptHeaderSetMediaType(hHeader, strMediaType, NULL,
                                           NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIATYPE_UNDEFINED == eMediaType)
    {
        pHeader->strMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pMediaType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * RvSipAcceptHeaderGetMediaSubType
 * ------------------------------------------------------------------------
 * General: Gets the media sub type enumeration value. If
 *          RVSIP_MEDIASUBTYPE_OTHER is returned, you can receive the media
 *          sub type string using RvSipAcceptHeaderGetMediaSubTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaSubType RVCALLCONV RvSipAcceptHeaderGetMediaSubType(
                                   IN RvSipAcceptHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    return (((MsgAcceptHeader*)hHeader)->eMediaSubType);
}

/***************************************************************************
 * SipAcceptHeaderGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrMediaSubType(
                                     IN RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->strMediaSubType;
}

/***************************************************************************
 * RvSipAcceptHeaderGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: Copies the media sub type string from the Content-Type header
 *          into a given buffer. Use this function when the media sub type
 *          enumeration returned by RvSipAcceptGetMediaSubType() is
 *          RVSIP_MEDIASUBTYPE_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrMediaSubType(
                                    IN RvSipAcceptHeaderHandle       hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAcceptHeader*)hHeader)->hPool,
                                  ((MsgAcceptHeader*)hHeader)->hPage,
                                  SipAcceptHeaderGetStrMediaSubType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaSubType  - The media sub type string to be set - if Null,
 *                     the exist media sub type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipAcceptHeaderSetMediaSubType(
                               IN  RvSipAcceptHeaderHandle     hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                      hPool,
                               IN  HPAGE                       hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgAcceptHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaSubType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strMediaSubType = newStr;
#ifdef SIP_DEBUG
    pHeader->pMediaSubType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                          pHeader->strMediaSubType);
#endif

    /* Set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eMediaSubType = RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    else
    {
        pHeader->eMediaSubType = RVSIP_MEDIASUBTYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAcceptHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: Sets the media sub type field in the Content-Type header object.
 *          If the enumeration given by eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER,
 *          sets the media sub type string given by strMediaSubType in the
 *          Content-Type header object. Use RVSIP_MEDIASUBTYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaSubType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaSubType - The media sub type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIASUBTYPE_UNDEFINED is supplied, the
 *               existing media sub type is removed from the header.
 *    strMediaSubType - The media sub type string to be set in the Content-Type
 *               header. (relevant when eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetMediaSubType(
                            IN  RvSipAcceptHeaderHandle      hHeader,
                            IN  RvSipMediaSubType            eMediaSubType,
                            IN  RvChar                      *strMediaSubType)
{
    MsgAcceptHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgAcceptHeader *)hHeader;

    pHeader->eMediaSubType = eMediaSubType;
    if (RVSIP_MEDIASUBTYPE_OTHER == eMediaSubType)
    {
        stat = SipAcceptHeaderSetMediaSubType(hHeader, strMediaSubType, NULL,
                                                   NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIASUBTYPE_UNDEFINED == eMediaSubType)
    {
        pHeader->strMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pMediaSubType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipAcceptHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object..
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetQVal(IN RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->strQVal;
}

/***************************************************************************
 * RvSipAcceptHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the Accept header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetQVal(
                                       IN RvSipAcceptHeaderHandle          hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptHeader*)hHeader)->hPool,
                                  ((MsgAcceptHeader*)hHeader)->hPage,
                                  SipAcceptHeaderGetQVal(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          Accept Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Accept header object.
 *       pQVal        - The QVal string to be set in the Accept header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptHeaderSetQVal(IN  RvSipAcceptHeaderHandle   hHeader,
								IN  RvChar                    *pQVal,
								IN  HRPOOL                    hPool,
								IN  HPAGE                     hPage,
								IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgAcceptHeader      *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptHeader*)hHeader;

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
 * RvSipAcceptHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept header object.
 *    pQVal      - The QVal string to be set in the Accept header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetQVal(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar                    *pQVal)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptHeaderSetQVal(hHeader, pQVal, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Accept header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object..
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetOtherParams(IN RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipAcceptHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Accept header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetOtherParams(
                                       IN RvSipAcceptHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgAcceptHeader*)hHeader)->hPool,
                                  ((MsgAcceptHeader*)hHeader)->hPage,
                                  SipAcceptHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Accept Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Accept header object.
 *       pOtherParams - The other-params to be set in the Accept header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptHeaderSetOtherParams(
							    IN  RvSipAcceptHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgAcceptHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAcceptHeader*)hHeader;

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
 * RvSipAcceptHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Accept header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept header object.
 *    pText      - The other-params string to be set in the Accept header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetOtherParams(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAcceptHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object.
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrBadSyntax(IN  RvSipAcceptHeaderHandle hHeader)
{
    return ((MsgAcceptHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAcceptHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Accept header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptHeaderHandle    hHeader,
                                     IN  RvChar*					strBuffer,
                                     IN  RvUint						bufferLen,
                                     OUT RvUint*					actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAcceptHeader*)hHeader)->hPool,
                                  ((MsgAcceptHeader*)hHeader)->hPage,
                                  SipAcceptHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAcceptHeaderSetStrBadSyntax
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
RvStatus SipAcceptHeaderSetStrBadSyntax(
                                  IN RvSipAcceptHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgAcceptHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAcceptHeader*)hHeader;

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
 * RvSipAcceptHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Accept header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetStrBadSyntax(
                                  IN RvSipAcceptHeaderHandle        hHeader,
                                  IN RvChar*						strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAcceptHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AcceptHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AcceptHeaderClean(IN MsgAcceptHeader*             pHeader,
                              IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType     = RVSIP_HEADERTYPE_ACCEPT_ENCODING;
	
	pHeader->eMediaType      = RVSIP_MEDIATYPE_UNDEFINED;
	pHeader->eMediaSubType   = RVSIP_MEDIASUBTYPE_UNDEFINED;

	pHeader->strMediaType    = UNDEFINED;
	pHeader->strMediaSubType = UNDEFINED;
	pHeader->strQVal         = UNDEFINED;
	pHeader->strOtherParams  = UNDEFINED;
	
	
#ifdef SIP_DEBUG
	pHeader->pMediaType      = NULL;
	pHeader->pMediaSubType   = NULL;
	pHeader->pQVal           = NULL;
	pHeader->pOtherParams    = NULL;
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
