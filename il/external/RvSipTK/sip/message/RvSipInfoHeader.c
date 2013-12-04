/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipInfoHeader.c                                      *
 *                                                                            *
 * The file defines the methods of the Info header object:                    *
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

#include "RvSipInfoHeader.h"
#include "_SipInfoHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "ParserProcess.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"
#include "RvSipAddress.h"


#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void InfoHeaderClean(IN MsgInfoHeader* pHeader,
                            IN RvBool           bCleanBS);

static RvStatus RVCALLCONV EncodeInfoEnumAndString(
										IN  MsgInfoHeader*         pHeader,
										IN  HRPOOL                   hPool,
										IN  HPAGE                    hPage,
										OUT RvUint32*                length);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Info header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 *          Use RvSipInfoHeaderSetHeaderType to determine whether the
 *          object represents an Alert-Info, a Call-Info or an Error-Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Info 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipInfoHeaderHandle               *hHeader)
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

    stat = RvSipInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_INFO,
                              &hListElem,
                              (void**)hHeader);
}


/***************************************************************************
 * RvSipInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Info Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 *          Use RvSipInfoHeaderSetHeaderType to determine whether the
 *          object represents an Alert-Info, a Call-Info or an Error-Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipInfoHeaderHandle *hHeader)
{
    MsgInfoHeader*   pHeader;
    MsgMgr*            pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgInfoHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                      hPool,
                                                      hPage,
                                                      sizeof(MsgInfoHeader),
                                                      RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipInfoHeaderConstruct - Allocation failed. hPool 0x%p, hPage %d",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    InfoHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipInfoHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipInfoHeaderConstruct - Info header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Info header object to a destination 
 *          Info header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Info header object.
 *    hSource      - Handle to the source Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderCopy(
                                     IN    RvSipInfoHeaderHandle hDestination,
                                     IN    RvSipInfoHeaderHandle hSource)
{
    MsgInfoHeader* source, *dest;
	RvStatus       rv;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgInfoHeader*)hSource;
    dest   = (MsgInfoHeader*)hDestination;

	/* header type */
    dest->eHeaderType   = source->eHeaderType;
	/* Info type */
	dest->eInfoType     = source->eInfoType;

	/* Address spec */
	rv = RvSipInfoHeaderSetAddrSpec(hDestination, source->hAddrSpec);
	if (rv != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
				   "RvSipInfoHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
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
						"RvSipInfoHeaderCopy: Failed to copy other params. hDest 0x%p",
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
						"RvSipInfoHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
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
 * RvSipInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Info header object to a textual Info header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Info header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderEncode(
                                          IN    RvSipInfoHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgInfoHeader* pHeader;

    pHeader = (MsgInfoHeader*)hHeader;
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
                "RvSipInfoHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipInfoHeaderEncode - Got new page %d on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = InfoHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipInfoHeaderEncode - Failed. Free page %d on pool 0x%p. InfoHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * InfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Info header (as string) on it.
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
RvStatus RVCALLCONV InfoHeaderEncode(
                                      IN    RvSipInfoHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length)
{
    MsgInfoHeader*    pHeader = (MsgInfoHeader*)hHeader;
    RvStatus          stat;
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "InfoHeaderEncode - Encoding Info header. hHeader 0x%p, hPool 0x%p, hPage %d",
             hHeader, hPool, hPage));

	/* encode header name */
	switch (pHeader->eInfoType)
	{
	case RVSIP_INFO_ALERT_INFO_HEADER:
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Alert-Info", length);
			break;
		}
	case RVSIP_INFO_CALL_INFO_HEADER:
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Call-Info", length);
			break;
		}
	case RVSIP_INFO_ERROR_INFO_HEADER:
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Error-Info", length);
			break;
		}
	default:
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"InfoHeaderEncode - Failed to encode Info header - Undefined header type. header=%p",
					pHeader));
		return RV_ERROR_UNKNOWN;
	}
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"InfoHeaderEncode - Failed to encode Info header. RvStatus is %d, hPool 0x%p, hPage %d",
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
			      "InfoHeaderEncode - Failed to encode Info header. RvStatus is %d, hPool 0x%p, hPage %d",
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
                "InfoHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage %d",
                stat, hPool, hPage));
        }
        return stat;
    }

	/* Addr Spec */
	
	if (pHeader->hAddrSpec != NULL)
    {
        /* encode <address-spec> */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"InfoHeaderEncode - Failed to encode address spec in Info header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
            return stat;
		}
    }
    else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"InfoHeaderEncode - Failed to encode Info header - Missing mandatory address spec. hHeader=%p",
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
				"InfoHeaderEncode - Failed to encode Info header. RvStatus is %d, hPool 0x%p, hPage %d",
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
				"InfoHeaderEncode - Failed to encode Info header. RvStatus is %d, hPool 0x%p, hPage %d",
				stat, hPool, hPage));
			return stat;
		}
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Info header into a Info header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Info header object.
 *    buffer    - Buffer containing a textual Info header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderParse(
                                     IN RvSipInfoHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgInfoHeader*       pHeader = (MsgInfoHeader*)hHeader;
    RvStatus             rv;
	SipParseType         eType;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    InfoHeaderClean(pHeader, RV_FALSE);

	if (pHeader->eInfoType == RVSIP_INFO_ALERT_INFO_HEADER)
	{
		eType = SIP_PARSETYPE_ALERT_INFO;
	}
	else if (pHeader->eInfoType == RVSIP_INFO_CALL_INFO_HEADER)
	{
		eType = SIP_PARSETYPE_CALL_INFO;
	}
	else
	{
		/* The parser is not interested in the specific type but rather in the
		knowledge that this is one of the Info headers. Therefore, if no type
		was set, a default (error-info) will be taken. */
		eType = SIP_PARSETYPE_ERROR_INFO;
	}
	
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
 * RvSipInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Info header value into an Info header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Info header object.
 *    buffer    - The buffer containing a textual Info header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderParseValue(
                                     IN RvSipInfoHeaderHandle  hHeader,
                                     IN RvChar*                  buffer)
{
    MsgInfoHeader*       pHeader = (MsgInfoHeader*)hHeader;
    RvStatus             rv;
	SipParseType         eType;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;
	
    if (pHeader->eInfoType == RVSIP_INFO_ALERT_INFO_HEADER)
	{
		eType = SIP_PARSETYPE_ALERT_INFO;
	}
	else if (pHeader->eInfoType == RVSIP_INFO_CALL_INFO_HEADER)
	{
		eType = SIP_PARSETYPE_CALL_INFO;
	}
	else
	{
		eType = SIP_PARSETYPE_ERROR_INFO;
	}
	
    InfoHeaderClean(pHeader, RV_FALSE);
	
	rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
										eType,
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
 * RvSipInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Info header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderFix(
                                     IN RvSipInfoHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgInfoHeader* pHeader = (MsgInfoHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipInfoHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipInfoHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipInfoHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Info header fields are kept in a string format, for example, the
 *          CSeq other-params. In order to get such a field from the Info header object,
 *          your application should supply an adequate buffer to where the string will be
 *          copied.
 *          This function provides you with the length of the string to enable you to
 *          allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Info header object.
 *    stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipInfoHeaderGetStringLength(
                                      IN  RvSipInfoHeaderHandle     hHeader,
                                      IN  RvSipInfoHeaderStringName stringName)
{
    RvInt32        string;
    MsgInfoHeader* pHeader = (MsgInfoHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (stringName)
    {
        case RVSIP_INFO_OTHER_PARAMS:
        {
            string = SipInfoHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_INFO_BAD_SYNTAX:
        {
            string = SipInfoHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"RvSipInfoHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(string > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, string)+1);
    else
        return 0;
}

/***************************************************************************
 * SipInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Info object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipInfoHeaderGetPool(RvSipInfoHeaderHandle hHeader)
{
    return ((MsgInfoHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Info object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipInfoHeaderGetPage(RvSipInfoHeaderHandle hHeader)
{
    return ((MsgInfoHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipInfoHeaderGetHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Info Header object.
 * Return Value: Returns the Info header type enumeration from the Info
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Info header object.
 ***************************************************************************/
RVAPI RvSipInfoHeaderType  RVCALLCONV RvSipInfoHeaderGetHeaderType(
											IN RvSipInfoHeaderHandle hHeader)
{
	MsgInfoHeader* pHeader;
	
    if(hHeader == NULL)
        return RVSIP_INFO_UNDEFINED_HEADER;
	
    pHeader = (MsgInfoHeader*)hHeader;
    return pHeader->eInfoType;
}

/***************************************************************************
 * RvSipInfoHeaderSetHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Info Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hInfoHeader       - Handle to the Info header object.
 *    eHeaderType       - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetHeaderType(
                                 IN    RvSipInfoHeaderHandle hHeader,
                                 IN    RvSipInfoHeaderType   eHeaderType)
{
    MsgInfoHeader* pHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgInfoHeader*)hHeader;
    pHeader->eInfoType = eHeaderType;
    return RV_OK;
}

/***************************************************************************
 * RvSipInfoHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address-Spec field is held in the Info header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address object, or NULL if the Address
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Info header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipInfoHeaderGetAddrSpec
                                            (IN RvSipInfoHeaderHandle hHeader)
{
	if(hHeader == NULL)
        return NULL;
	
    return ((MsgInfoHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipInfoHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address-Spec object in the Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Info header object.
 *    hAddrSpec - Handle to the Address-Spec object to be set in the 
 *                Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetAddrSpec
                                            (IN    RvSipInfoHeaderHandle  hHeader,
                                             IN    RvSipAddressHandle     hAddrSpec)
{
	RvStatus            stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgInfoHeader      *pHeader         = (MsgInfoHeader*)hHeader;

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

            stat = RvSipAddrConstructInInfoHeader(hHeader,
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
 * SipInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Info header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Info header object..
 ***************************************************************************/
RvInt32 SipInfoHeaderGetOtherParams(IN RvSipInfoHeaderHandle hHeader)
{
    return ((MsgInfoHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Info header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Info header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderGetOtherParams(
                                       IN RvSipInfoHeaderHandle  hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgInfoHeader*)hHeader)->hPool,
                                  ((MsgInfoHeader*)hHeader)->hPage,
                                  SipInfoHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Info Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Info header object.
 *       pOtherParams - The other-params to be set in the Info header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipInfoHeaderSetOtherParams(
							    IN  RvSipInfoHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset)
{
    RvInt32                      newStr;
    MsgInfoHeader             *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgInfoHeader*)hHeader;

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
 * RvSipInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Info header object.
 *    pText      - The other-params string to be set in the Info header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetOtherParams(
                                     IN RvSipInfoHeaderHandle   hHeader,
                                     IN RvChar                   *pText)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipInfoHeaderSetOtherParams(hHeader, pText, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Info header object.
 ***************************************************************************/
RvInt32 SipInfoHeaderGetStrBadSyntax(IN  RvSipInfoHeaderHandle hHeader)
{
    return ((MsgInfoHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Info header,
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
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderGetStrBadSyntax(
                                     IN  RvSipInfoHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgInfoHeader*)hHeader)->hPool,
                                  ((MsgInfoHeader*)hHeader)->hPage,
                                  SipInfoHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipInfoHeaderSetStrBadSyntax
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
RvStatus SipInfoHeaderSetStrBadSyntax(
                                  IN RvSipInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgInfoHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgInfoHeader*)hHeader;

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
 * RvSipInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetStrBadSyntax(
                                  IN RvSipInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipInfoHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * InfoHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void InfoHeaderClean( 
								      IN MsgInfoHeader* pHeader,
                                      IN RvBool                       bCleanBS)
{
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_INFO;
	pHeader->eInfoType      = RVSIP_INFO_UNDEFINED_HEADER;
	
	pHeader->hAddrSpec   = NULL;
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
