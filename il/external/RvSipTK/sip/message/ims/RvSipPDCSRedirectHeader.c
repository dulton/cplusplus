/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPDCSRedirectHeader.c										*
 *																					*
 * The file defines the methods of the PDCSRedirect header object:					*
 * construct, destruct, copy, encode, parse and the ability to access and			*
 * change it's parameters.															*
 *																					*
 *      Author           Date														*
 *     ------           ------------												*
 *      Mickey           Jan.2006													*
 ************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

#include "RvSipPDCSRedirectHeader.h"
#include "_SipPDCSRedirectHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PDCSRedirectHeaderClean(IN MsgPDCSRedirectHeader*	pHeader,
									IN RvBool					bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSRedirectHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSRedirect header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderConstructInMsg(
                                   IN  RvSipMsgHandle					 hSipMsg,
                                   IN  RvBool							 pushHeaderAtHead,
                                   OUT RvSipPDCSRedirectHeaderHandle* hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus					stat;

    if(hSipMsg == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipPDCSRedirectHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
	{
        return stat;
	}

    if(pushHeaderAtHead == RV_TRUE)
	{
        location = RVSIP_FIRST_HEADER;
	}
    else
	{
        location = RVSIP_LAST_HEADER;
	}

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_P_DCS_REDIRECT,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSRedirect Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderConstruct(
                               IN  RvSipMsgMgrHandle				hMsgMgr,
                               IN  HRPOOL							hPool,
                               IN  HPAGE							hPage,
                               OUT RvSipPDCSRedirectHeaderHandle*	hHeader)
{
    MsgPDCSRedirectHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    pHeader = (MsgPDCSRedirectHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPDCSRedirectHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderConstruct - Failed to construct PDCSRedirect header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PDCSRedirectHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPDCSRedirectHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderConstruct - PDCSRedirect header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSRedirect header 
 *			object to a destination PDCSRedirect header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSRedirect header object.
 *    hSource      - Handle to the destination PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderCopy(
                             INOUT RvSipPDCSRedirectHeaderHandle hDestination,
                             IN    RvSipPDCSRedirectHeaderHandle hSource)
{
    MsgPDCSRedirectHeader*	source;
    MsgPDCSRedirectHeader*	dest;
	RvStatus				stat;
	
    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPDCSRedirectHeader*)hSource;
    dest   = (MsgPDCSRedirectHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

	/* hCalledIDAddr */
    stat = RvSipPDCSRedirectHeaderSetAddrSpec(hDestination, source->hCalledIDAddr,
												 RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderCopy: Failed to copy hCalledIDAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }
	
	/* hRedirectorAddr */
    stat = RvSipPDCSRedirectHeaderSetAddrSpec(hDestination, source->hRedirectorAddr,
												 RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderCopy: Failed to copy hRedirectorAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }
	
	/* nRedirCount */
	dest->nRedirCount = source->nRedirCount;
	
    /* strOtherParams */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strOtherParams);
        if(dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipPDCSRedirectHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPDCSRedirectHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPDCSRedirectHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSRedirect header object to a textual PDCSRedirect header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSRedirect header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderEncode(
                                          IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                          IN    HRPOOL							hPool,
                                          OUT   HPAGE*							hPage,
                                          OUT   RvUint32*               		length)
{
    RvStatus stat;
    MsgPDCSRedirectHeader* pHeader;

    pHeader = (MsgPDCSRedirectHeader*)hHeader;

    if(hPage == NULL || length == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL) || (hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSRedirectHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSRedirectHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PDCSRedirectHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSRedirectHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PDCSRedirectHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PDCSRedirectHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-Redirect header (as string) on it.
 *          format: "P-DCS-Redirect: Called-ID *(SEMI redir-params)"
 *                  
 * Return Value: RV_OK					   - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-Redirect header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PDCSRedirectHeaderEncode(
                                  IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                  IN    HRPOOL							hPool,
                                  IN    HPAGE							hPage,
                                  IN    RvBool							bInUrlHeaders,
                                  INOUT RvUint32*						length)
{
    MsgPDCSRedirectHeader*	pHeader;
    RvStatus				stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPDCSRedirectHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PDCSRedirectHeaderEncode - Encoding PDCSRedirect header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-DCS-Redirect" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-DCS-Redirect", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSRedirectHeaderEncode - Failed to encoding PDCSRedirect header. stat is %d",
            stat));
        return stat;
    }

    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);
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
                "PDCSRedirectHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
    }
    
    /* Called-ID */
    if(pHeader->hCalledIDAddr != NULL)
    {
        /* set " in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);

		/* Set the addr-spec */
        stat = AddrEncode(pHeader->hCalledIDAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSRedirectHeaderEncode - Failed to encode Called-ID addr-spec. stat is %d",
				stat));
		}
    }
	else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSRedirectHeaderEncode - Missing obligatory Called-ID Parameter."));
        return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Redirector (insert ";" in the beginning) */
    if(pHeader->hRedirectorAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "redirector-uri", length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the addr-spec */
        stat = AddrEncode(pHeader->hRedirectorAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSRedirectHeaderEncode - Failed to encode Redirector addr-spec. stat is %d",
				stat));
			return stat;
		}
    }

	if(pHeader->nRedirCount > UNDEFINED)
	{
		RvChar strHelper[16];
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "count", length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		
		/* set the count string */
		MsgUtils_itoa(pHeader->nRedirCount, strHelper);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											strHelper, length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSRedirectHeaderEncode - Failed to encode PDCSRedirect header. stat is %d",
				stat));
			return stat;
		}
	}

	/* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set OtherParms */
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
            "PDCSRedirectHeaderEncode - Failed to encoding PDCSRedirect header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSRedirect header-for example,
 *          "PDCSRedirect:sip:172.20.5.3:5060"-into a PDCSRedirect header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSRedirect header object.
 *    buffer    - Buffer containing a textual PDCSRedirect header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderParse(
                                 IN    RvSipPDCSRedirectHeaderHandle hHeader,
                                 IN    RvChar*							buffer)
{
    MsgPDCSRedirectHeader*	pHeader = (MsgPDCSRedirectHeader*)hHeader;
	RvStatus				rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PDCSRedirectHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_REDIRECT,
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
 * RvSipPDCSRedirectHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSRedirect header value into an PDCSRedirect header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSRedirectHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSRedirect header object.
 *    buffer    - The buffer containing a textual PDCSRedirect header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderParseValue(
                                 IN    RvSipPDCSRedirectHeaderHandle hHeader,
                                 IN    RvChar*							buffer)
{
    MsgPDCSRedirectHeader*	pHeader = (MsgPDCSRedirectHeader*)hHeader;
	RvStatus				rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}


    PDCSRedirectHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_REDIRECT,
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
 * RvSipPDCSRedirectHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSRedirect header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderFix(
                                     IN RvSipPDCSRedirectHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPDCSRedirectHeader*	pHeader = (MsgPDCSRedirectHeader*)hHeader;
    RvStatus				rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPDCSRedirectHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSRedirectHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPDCSRedirectHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSRedirect object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The Header to take the Pool from.
 ***************************************************************************/
HRPOOL SipPDCSRedirectHeaderGetPool(RvSipPDCSRedirectHeaderHandle hHeader)
{
    return ((MsgPDCSRedirectHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPDCSRedirectHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSRedirect object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The Header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSRedirectHeaderGetPage(RvSipPDCSRedirectHeaderHandle hHeader)
{
    return ((MsgPDCSRedirectHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSRedirect header fields are kept in a string format-for example, the
 *          PDCSRedirect header VNetworkSpec name. In order to get such a field from the PDCSRedirect header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the PDCSRedirect header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSRedirectHeaderGetStringLength(
                                      IN  RvSipPDCSRedirectHeaderHandle     hHeader,
                                      IN  RvSipPDCSRedirectHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPDCSRedirectHeader* pHeader = (MsgPDCSRedirectHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_DCS_REDIRECT_OTHER_PARAMS:
        {
            stringOffset = SipPDCSRedirectHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_DCS_REDIRECT_BAD_SYNTAX:
        {
            stringOffset = SipPDCSRedirectHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSRedirectHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }

    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PDCSRedirect header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSRedirect header object.
 *	  ePDCSRedirectAddrType - Enumeration for the field from which to get the address.	
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPDCSRedirectHeaderGetAddrSpec(
                        IN RvSipPDCSRedirectHeaderHandle hHeader,
						IN RvSipPDCSRedirectAddressType	ePDCSRedirectAddrType)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

	switch(ePDCSRedirectAddrType)
	{
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID:
		return ((MsgPDCSRedirectHeader*)hHeader)->hCalledIDAddr;
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR:
		return ((MsgPDCSRedirectHeader*)hHeader)->hRedirectorAddr;
	default:
		return NULL;
	}
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PDCSRedirect header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PDCSRedirect header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *               Address Spec is removed from the PDCSRedirect header.
 *	  ePDCSRedirectAddrType - Enumeration for the field in which to set the address.	
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetAddrSpec(
                        IN	RvSipPDCSRedirectHeaderHandle	hHeader,
                        IN	RvSipAddressHandle				hAddrSpec,
						IN	RvSipPDCSRedirectAddressType	ePDCSRedirectAddrType)
{
    RvStatus				stat;
    RvSipAddressType		currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress				*pAddr			= (MsgAddress*)hAddrSpec;
    MsgPDCSRedirectHeader	*pHeader	    = (MsgPDCSRedirectHeader*)hHeader;
	RvSipAddressHandle		*hAddr;
	
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	switch(ePDCSRedirectAddrType)
	{
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID:
		hAddr = &pHeader->hCalledIDAddr;
		break;
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR:
		hAddr = &pHeader->hRedirectorAddr;
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
    
	if(hAddrSpec == NULL)
    {
		*hAddr = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
			return RV_ERROR_BADPARAM;
        }
        
		if(NULL != *hAddr)
		{
			currentAddrType = RvSipAddrGetAddrType(*hAddr);
		}

        /* if no address object was allocated, we will construct it */
        if((*hAddr == NULL) ||
           (currentAddrType != pAddr->eAddrType))
        {
            stat = RvSipAddrConstructInPDCSRedirectHeader(hHeader,
                                                     pAddr->eAddrType, ePDCSRedirectAddrType,
                                                     hAddr);
            if(RV_OK != stat)
			{
                return stat;
			}
        }

        stat = RvSipAddrCopy(*hAddr, hAddrSpec);
		return stat;
    }
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetCount
 * ------------------------------------------------------------------------
 * General: Gets the Count parameter from the PDCSRedirect header object.
 * Return Value: Returns the Count number, or UNDEFINED if the Count number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSRedirectHeaderGetCount(
                                         IN RvSipPDCSRedirectHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgPDCSRedirectHeader*)hHeader)->nRedirCount;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetCount
 * ------------------------------------------------------------------------
 * General:  Sets Count parameter of the PDCSRedirect header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PDCSRedirect header object.
 *    count		- The Count number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetCount(
                                         IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                         IN    RvInt32							count)
{
    if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgPDCSRedirectHeader*)hHeader)->nRedirCount = count;
    return RV_OK;
}

/***************************************************************************
 * SipPDCSRedirectHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PDCSRedirect header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSRedirect header object..
 ***************************************************************************/
RvInt32 SipPDCSRedirectHeaderGetOtherParams(
                                            IN RvSipPDCSRedirectHeaderHandle hHeader)
{
    return ((MsgPDCSRedirectHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSRedirect header other params field of the PDCSRedirect header object into a
 *          given buffer.
 *          Not all the PDCSRedirect header parameters have separated fields in the PDCSRedirect
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSRedirect header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderGetOtherParams(
                                               IN RvSipPDCSRedirectHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgPDCSRedirectHeader*)hHeader)->hPool,
                                  ((MsgPDCSRedirectHeader*)hHeader)->hPage,
                                  SipPDCSRedirectHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSRedirectHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSRedirectHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSRedirect header object.
 *            pOtherParams - The Other Params to be set in the PDCSRedirect header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSRedirectHeaderSetOtherParams(
                                     IN    RvSipPDCSRedirectHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32					newStr;
    MsgPDCSRedirectHeader*	pHeader;
    RvStatus				retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSRedirectHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOtherParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
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
 * RvSipPDCSRedirectHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSRedirect header object.
 *         Not all the PDCSRedirect header parameters have separated fields in the PDCSRedirect
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSRedirect header object.
 *    strOtherParams - The extended parameters field to be set in the PDCSRedirect header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetOtherParams(
                                     IN    RvSipPDCSRedirectHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSRedirectHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSRedirectHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSRedirectHeaderGetStrBadSyntax(
                                    IN RvSipPDCSRedirectHeaderHandle hHeader)
{
    return ((MsgPDCSRedirectHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSRedirect header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderGetStrBadSyntax(
                                               IN RvSipPDCSRedirectHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
	if(actualLen == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(strBuffer == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    return MsgUtilsFillUserBuffer(((MsgPDCSRedirectHeader*)hHeader)->hPool,
                                  ((MsgPDCSRedirectHeader*)hHeader)->hPage,
                                  SipPDCSRedirectHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSRedirectHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipPDCSRedirectHeaderSetStrBadSyntax(
                                  IN RvSipPDCSRedirectHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPDCSRedirectHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPDCSRedirectHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
                                  strBadSyntax, header->hPool, header->hPage, &newStrOffset);
    if(RV_OK != retStatus)
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
 * RvSipPDCSRedirectHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSRedirect header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetStrBadSyntax(
                                  IN RvSipPDCSRedirectHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSRedirectHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PDCSRedirectHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PDCSRedirectHeaderClean( IN MsgPDCSRedirectHeader*	pHeader,
									 IN RvBool					bCleanBS)
{
	pHeader->eHeaderType				= RVSIP_HEADERTYPE_P_DCS_REDIRECT;
    pHeader->strOtherParams				= UNDEFINED;
	pHeader->nRedirCount				= UNDEFINED;
	pHeader->hCalledIDAddr				= NULL;
	pHeader->hRedirectorAddr			= NULL;

#ifdef SIP_DEBUG
    pHeader->pOtherParams				= NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax	= UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax		= NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

