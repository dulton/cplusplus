/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPServedUserHeader.c							      *
 *                                                                            *
 * The file defines the methods of the PServedUser header object:             *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey            Jan.2008                                            *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPServedUserHeader.h"
#include "_SipPServedUserHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PServedUserHeaderClean(IN MsgPServedUserHeader* pHeader,
							   IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPServedUserHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PServedUser header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PServedUser header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPServedUserHeaderHandle* hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

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

    stat = RvSipPServedUserHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(RV_OK != stat)
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
                              RVSIP_HEADERTYPE_P_SERVED_USER,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPServedUserHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PServedUser Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PServedUser header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPServedUserHeaderHandle* hHeader)
{
    MsgPServedUserHeader*   pHeader;
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

    pHeader = (MsgPServedUserHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPServedUserHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPServedUserHeaderConstruct - Failed to construct PServedUser header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PServedUserHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPServedUserHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPServedUserHeaderConstruct - PServedUser header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPServedUserHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PServedUser header object to a destination PServedUser
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PServedUser header object.
 *    hSource      - Handle to the destination PServedUser header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderCopy(
                                         INOUT    RvSipPServedUserHeaderHandle hDestination,
                                         IN    RvSipPServedUserHeaderHandle hSource)
{
    RvStatus		stat;
    MsgPServedUserHeader*	source;
    MsgPServedUserHeader*	dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPServedUserHeader*)hSource;
    dest   = (MsgPServedUserHeader*)hDestination;

    dest->eHeaderType  = source->eHeaderType;

    /* strDisplayName */
	if(source->strDisplayName > UNDEFINED)
	{
		dest->strDisplayName = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
															  dest->hPool,
															  dest->hPage,
															  source->hPool,
															  source->hPage,
															  source->strDisplayName);
		if(dest->strDisplayName == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
				"RvSipPServedUserHeaderCopy - Failed to copy displayName"));
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
	
    /* AddrSpec */
    stat = RvSipPServedUserHeaderSetAddrSpec(hDestination, source->hAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPServedUserHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }
	
    /* eSessionCaseType */
    dest->eSessionCaseType = source->eSessionCaseType;

    /* eRegistrationStateType */
    dest->eRegistrationStateType = source->eRegistrationStateType;
    
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
					  "RvSipPServedUserHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPServedUserHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPServedUserHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PServedUser header object to a textual PServedUser header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PServedUser header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderEncode(
                                          IN    RvSipPServedUserHeaderHandle hHeader,
                                          IN    HRPOOL                hPool,
                                          OUT   HPAGE*                hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgPServedUserHeader* pHeader;

    pHeader = (MsgPServedUserHeader*)hHeader;

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
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPServedUserHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPServedUserHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PServedUserHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(RV_OK != stat)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPServedUserHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PServedUserHeaderEncode fail",
                *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * PServedUserHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          header (as string) on it.
 *          example format: "P-Served-User: " 
 *							("*"|(1#((name-addr|addr-spec)
 *							*(";"generic-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PServedUser header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PServedUserHeaderEncode(
                                  IN    RvSipPServedUserHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPServedUserHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPServedUserHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PServedUserHeaderEncode - Encoding PServedUser header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "P-Served-User" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Served-User", length);
	
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PServedUserHeaderEncode - Failed to encode PServedUser header. stat is %d",
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
        if(RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "PServedUserHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* display name */

    /* [displayNeme]"<"addr-spec">" */

    if(pHeader->strDisplayName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDisplayName,
                                    length);
    }

    if (pHeader->hAddrSpec != NULL)
    {
        /* Set the adder-spec */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (RV_OK != stat)
		{
            return stat;
		}
    }
    else
    {
        /* This is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PServedUserHeaderEncode: Failed. No hAddrSpec was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }

    /* eSessionCaseType */
	if(pHeader->eSessionCaseType > RVSIP_SESSION_CASE_TYPE_UNDEFINED)
    {
        /* set ";sescase=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "sescase", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeSessionCaseType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eSessionCaseType,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PServedUserHeaderEncode - Failed to encode SessionCase Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}
    
    /* eRegistrationStateType */
	if(pHeader->eRegistrationStateType > RVSIP_SESSION_CASE_TYPE_UNDEFINED)
    {
        /* set ";regstate=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "regstate", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeRegistrationStateType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eRegistrationStateType,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PServedUserHeaderEncode - Failed to encode RegistrationState Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}

    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
	{
		/* set ";" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set PServedUserParmas */
		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strOtherParams,
									length);
	}

	if (RV_OK != stat)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"PServedUserHeaderEncode - Failed to encode PServedUser header. stat is %d",
			stat));
	}

    return stat;
}

/***************************************************************************
 * RvSipPServedUserHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PServedUser header-for example,
 *          "P-Served-User:sip:172.20.5.3:5060"-into a PServedUser header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PServedUser header object.
 *    buffer    - Buffer containing a textual PServedUser header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderParse(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar*				 buffer)
{
    MsgPServedUserHeader*	pHeader = (MsgPServedUserHeader*)hHeader;
	RvStatus		rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PServedUserHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_SERVED_USER,
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
 * RvSipPServedUserHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PServedUser header value into an PServedUser header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPServedUserHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PServedUser header object.
 *    buffer    - The buffer containing a textual PServedUser header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderParseValue(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPServedUserHeader*    pHeader = (MsgPServedUserHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PServedUserHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_SERVED_USER,
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
 * RvSipPServedUserHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PServedUser header with bad-syntax information.
 *          A SIP header has the following grammar:
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
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderFix(
                                     IN RvSipPServedUserHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPServedUserHeader*	pHeader = (MsgPServedUserHeader*)hHeader;
    RvStatus		rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPServedUserHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPServedUserHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPServedUserHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPServedUserHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PServedUser object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPServedUserHeaderGetPool(RvSipPServedUserHeaderHandle hHeader)
{
    return ((MsgPServedUserHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPServedUserHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PServedUser object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPServedUserHeaderGetPage(RvSipPServedUserHeaderHandle hHeader)
{
    return ((MsgPServedUserHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PServedUser header fields are kept in a string format-for example, the
 *          PServedUser header display name. In order to get such a field from the PServedUser header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PServedUser header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPServedUserHeaderGetStringLength(
                                      IN  RvSipPServedUserHeaderHandle     hHeader,
                                      IN  RvSipPServedUserHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPServedUserHeader* pHeader = (MsgPServedUserHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_SERVED_USER_DISPLAYNAME:
        {
            stringOffset = SipPServedUserHeaderGetDisplayName(hHeader);
            break;
        }
        case RVSIP_P_SERVED_USER_OTHER_PARAMS:
        {
            stringOffset = SipPServedUserHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_SERVED_USER_BAD_SYNTAX:
        {
            stringOffset = SipPServedUserHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPServedUserHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPServedUserHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PServedUser header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PServedUser header object..
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetDisplayName(
									IN RvSipPServedUserHeaderHandle hHeader)
{
	return ((MsgPServedUserHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PServedUser header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetDisplayName(
                                               IN RvSipPServedUserHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPServedUserHeader*)hHeader)->hPool,
                                  ((MsgPServedUserHeader*)hHeader)->hPage,
                                  SipPServedUserHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPServedUserHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          PServedUserHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PServedUser header object.
 *         pDisplayName - The display name to be set in the PServedUser header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPServedUserHeaderSetDisplayName(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPServedUserHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPServedUserHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pDisplayName,
                                  pHeader->hPool, pHeader->hPage, &newStr);
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
 * RvSipPServedUserHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the PServedUser header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetDisplayName(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar*                 strDisplayName)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

     return SipPServedUserHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPServedUserHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PServedUser header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PServedUser header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPServedUserHeaderGetAddrSpec(
                                    IN RvSipPServedUserHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPServedUserHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipPServedUserHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PServedUser header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the PServedUser header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetAddrSpec(
                                        IN    RvSipPServedUserHeaderHandle hHeader,
                                        IN    RvSipAddressHandle       hAddrSpec)
{
    RvStatus			stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress			*pAddr           = (MsgAddress*)hAddrSpec;
    MsgPServedUserHeader		*pHeader         = (MsgPServedUserHeader*)hHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

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

            stat = RvSipAddrConstructInPServedUserHeader(hHeader,
												   pAddr->eAddrType,
												   &hSipAddr);
            if(RV_OK != stat)
                return stat;
        }
        stat = RvSipAddrCopy(pHeader->hAddrSpec,
                            hAddrSpec);
		return stat;
    }
}

/***************************************************************************
 * RvSipPServedUserHeaderSetSessionCaseType
 * ------------------------------------------------------------------------
 * General:Sets the SessionCase Type in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eSessionCaseType - The SessionCase Type to be set in the PServedUser header. If UNDEFINED is supplied, the
 *                 existing SessionCase Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetSessionCaseType(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
									 IN	   RvSipPServedUserSessionCaseType	eSessionCaseType)
{
    MsgPServedUserHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPServedUserHeader*) hHeader;

	pHeader->eSessionCaseType = eSessionCaseType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetSessionCaseType
 * ------------------------------------------------------------------------
 * General: Returns value SessionCase Type in the header.
 * Return Value: Returns RvSipPServedUserSessionCaseType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipPServedUserSessionCaseType RVCALLCONV RvSipPServedUserHeaderGetSessionCaseType(
                                               IN RvSipPServedUserHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SESSION_CASE_TYPE_UNDEFINED;
	}

    return ((MsgPServedUserHeader*)hHeader)->eSessionCaseType;
}

/***************************************************************************
 * RvSipPServedUserHeaderSetRegistrationStateType
 * ------------------------------------------------------------------------
 * General:Sets the RegistrationState Type in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eRegistrationStateType - The RegistrationState Type to be set in the PServedUser header. If UNDEFINED is supplied, the
 *                 existing RegistrationState Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetRegistrationStateType(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
									 IN	   RvSipPServedUserRegistrationStateType	eRegistrationStateType)
{
    MsgPServedUserHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPServedUserHeader*) hHeader;

	pHeader->eRegistrationStateType = eRegistrationStateType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetRegistrationStateType
 * ------------------------------------------------------------------------
 * General: Returns value RegistrationState Type in the header.
 * Return Value: Returns RvSipPServedUserRegistrationStateType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipPServedUserRegistrationStateType RVCALLCONV RvSipPServedUserHeaderGetRegistrationStateType(
                                               IN RvSipPServedUserHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SESSION_CASE_TYPE_UNDEFINED;
	}

    return ((MsgPServedUserHeader*)hHeader)->eRegistrationStateType;
}

/***************************************************************************
 * SipPServedUserHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the PServedUser Params in the PServedUser header object.
 * Return Value: PServedUser param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PServedUser header object..
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetOtherParams(
									IN RvSipPServedUserHeaderHandle hHeader)
{
    return ((MsgPServedUserHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PServedUser header other params field of the PServedUser header object into a
 *          given buffer.
 *          Not all the PServedUser header parameters have separated fields in the PServedUser
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PServedUser header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetOtherParams(
                                               IN RvSipPServedUserHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
	MsgPServedUserHeader   *pHeader         = (MsgPServedUserHeader*)hHeader;
	
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    return MsgUtilsFillUserBuffer(pHeader->hPool,
								  pHeader->hPage,
								  SipPServedUserHeaderGetOtherParams(hHeader),
								  strBuffer,
								  bufferLen,
								  actualLen);
}

/***************************************************************************
 * SipPServedUserHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PServedUserParam in the
 *          PServedUserHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PServedUser header object.
 *            pOtherParams - The Other Params to be set in the PServedUser header.
 *                          If NULL, the exist OtherParams string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPServedUserHeaderSetOtherParams(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32        newStr;
    MsgPServedUserHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPServedUserHeader*) hHeader;

	retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOtherParams,
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
 * RvSipPServedUserHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PServedUser header object.
 *         Not all the PServedUser header parameters have separated fields in the PServedUser
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PServedUser header object.
 *    strPServedUserParam - The extended parameters field to be set in the PServedUser header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetOtherParams(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar *				 pPServedUserParam)
{
	return SipPServedUserHeaderSetOtherParams(hHeader, pPServedUserParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPServedUserHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetStrBadSyntax(
                                    IN RvSipPServedUserHeaderHandle hHeader)
{
    return ((MsgPServedUserHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPServedUserHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad PServedUser header,
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
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetStrBadSyntax(
                                               IN RvSipPServedUserHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPServedUserHeader*)hHeader)->hPool,
                                  ((MsgPServedUserHeader*)hHeader)->hPage,
                                  SipPServedUserHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPServedUserHeaderSetStrBadSyntax
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
RvStatus SipPServedUserHeaderSetStrBadSyntax(
                                  IN RvSipPServedUserHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPServedUserHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPServedUserHeader*)hHeader;

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
 * RvSipPServedUserHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PServedUser header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetStrBadSyntax(
                                  IN RvSipPServedUserHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    return SipPServedUserHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PServedUserHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PServedUserHeaderClean( IN MsgPServedUserHeader* pHeader,
							        IN RvBool            bCleanBS)
{
    pHeader->hAddrSpec              = NULL;
    pHeader->strOtherParams         = UNDEFINED;
    pHeader->eHeaderType            = RVSIP_HEADERTYPE_P_SERVED_USER;
	pHeader->strDisplayName         = UNDEFINED;
    pHeader->eSessionCaseType       = RVSIP_SESSION_CASE_TYPE_UNDEFINED;
    pHeader->eRegistrationStateType = RVSIP_REGISTRATION_STATE_TYPE_UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pOtherParams     = NULL;
	pHeader->pDisplayName     = NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

