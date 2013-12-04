/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPProfileKeyHeader.c							      *
 *                                                                            *
 * The file defines the methods of the PProfileKey header object:             *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey            Mar.2007                                            *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPProfileKeyHeader.h"
#include "_SipPProfileKeyHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PProfileKeyHeaderClean(IN MsgPProfileKeyHeader* pHeader,
							   IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPProfileKeyHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PProfileKey header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PProfileKey header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPProfileKeyHeaderHandle* hHeader)
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

    stat = RvSipPProfileKeyHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_PROFILE_KEY,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPProfileKeyHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PProfileKey Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PProfileKey header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPProfileKeyHeaderHandle* hHeader)
{
    MsgPProfileKeyHeader*   pHeader;
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

    pHeader = (MsgPProfileKeyHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPProfileKeyHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPProfileKeyHeaderConstruct - Failed to construct PProfileKey header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PProfileKeyHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPProfileKeyHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPProfileKeyHeaderConstruct - PProfileKey header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPProfileKeyHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PProfileKey header object to a destination PProfileKey
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PProfileKey header object.
 *    hSource      - Handle to the destination PProfileKey header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderCopy(
                                         INOUT    RvSipPProfileKeyHeaderHandle hDestination,
                                         IN    RvSipPProfileKeyHeaderHandle hSource)
{
    RvStatus		stat;
    MsgPProfileKeyHeader*	source;
    MsgPProfileKeyHeader*	dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPProfileKeyHeader*)hSource;
    dest   = (MsgPProfileKeyHeader*)hDestination;

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
				"RvSipPProfileKeyHeaderCopy - Failed to copy displayName"));
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
    stat = RvSipPProfileKeyHeaderSetAddrSpec(hDestination, source->hAddrSpec);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPProfileKeyHeaderCopy: Failed to copy address spec. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }
	

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
					  "RvSipPProfileKeyHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPProfileKeyHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPProfileKeyHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PProfileKey header object to a textual PProfileKey header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PProfileKey header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderEncode(
                                          IN    RvSipPProfileKeyHeaderHandle hHeader,
                                          IN    HRPOOL                hPool,
                                          OUT   HPAGE*                hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgPProfileKeyHeader* pHeader;

    pHeader = (MsgPProfileKeyHeader*)hHeader;

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
                "RvSipPProfileKeyHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPProfileKeyHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PProfileKeyHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(RV_OK != stat)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPProfileKeyHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PProfileKeyHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PProfileKeyHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          header (as string) on it.
 *          example format: "P-Profile-Key: " 
 *							("*"|(1#((name-addr|addr-spec)
 *							*(";"generic-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PProfileKey header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PProfileKeyHeaderEncode(
                                  IN    RvSipPProfileKeyHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPProfileKeyHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPProfileKeyHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PProfileKeyHeaderEncode - Encoding PProfileKey header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "P-Profile-Key" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Profile-Key", length);
	
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PProfileKeyHeaderEncode - Failed to encode PProfileKey header. stat is %d",
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
                "PProfileKeyHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
            "PProfileKeyHeaderEncode: Failed. No hAddrSpec was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
    
    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
	{
		/* set ";" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set PProfileKeyParmas */
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
			"PProfileKeyHeaderEncode - Failed to encode PProfileKey header. stat is %d",
			stat));
	}

    return stat;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PProfileKey header-for example,
 *          "P-Profile-Key:sip:172.20.5.3:5060"-into a PProfileKey header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PProfileKey header object.
 *    buffer    - Buffer containing a textual PProfileKey header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderParse(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar*				 buffer)
{
    MsgPProfileKeyHeader*	pHeader = (MsgPProfileKeyHeader*)hHeader;
	RvStatus		rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PProfileKeyHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_PROFILE_KEY,
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
 * RvSipPProfileKeyHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PProfileKey header value into an PProfileKey header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPProfileKeyHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PProfileKey header object.
 *    buffer    - The buffer containing a textual PProfileKey header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderParseValue(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPProfileKeyHeader*    pHeader = (MsgPProfileKeyHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PProfileKeyHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_PROFILE_KEY,
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
 * RvSipPProfileKeyHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PProfileKey header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderFix(
                                     IN RvSipPProfileKeyHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPProfileKeyHeader*	pHeader = (MsgPProfileKeyHeader*)hHeader;
    RvStatus		rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPProfileKeyHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPProfileKeyHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPProfileKeyHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPProfileKeyHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PProfileKey object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPProfileKeyHeaderGetPool(RvSipPProfileKeyHeaderHandle hHeader)
{
    return ((MsgPProfileKeyHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPProfileKeyHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PProfileKey object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPProfileKeyHeaderGetPage(RvSipPProfileKeyHeaderHandle hHeader)
{
    return ((MsgPProfileKeyHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PProfileKey header fields are kept in a string format-for example, the
 *          PProfileKey header display name. In order to get such a field from the PProfileKey header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PProfileKey header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPProfileKeyHeaderGetStringLength(
                                      IN  RvSipPProfileKeyHeaderHandle     hHeader,
                                      IN  RvSipPProfileKeyHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPProfileKeyHeader* pHeader = (MsgPProfileKeyHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_PROFILE_KEY_DISPLAYNAME:
        {
            stringOffset = SipPProfileKeyHeaderGetDisplayName(hHeader);
            break;
        }
        case RVSIP_P_PROFILE_KEY_OTHER_PARAMS:
        {
            stringOffset = SipPProfileKeyHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_PROFILE_KEY_BAD_SYNTAX:
        {
            stringOffset = SipPProfileKeyHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPProfileKeyHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPProfileKeyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PProfileKey header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PProfileKey header object..
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetDisplayName(
									IN RvSipPProfileKeyHeaderHandle hHeader)
{
	return ((MsgPProfileKeyHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PProfileKey header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetDisplayName(
                                               IN RvSipPProfileKeyHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPProfileKeyHeader*)hHeader)->hPool,
                                  ((MsgPProfileKeyHeader*)hHeader)->hPage,
                                  SipPProfileKeyHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPProfileKeyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          PProfileKeyHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PProfileKey header object.
 *         pDisplayName - The display name to be set in the PProfileKey header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPProfileKeyHeaderSetDisplayName(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPProfileKeyHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPProfileKeyHeader*) hHeader;

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
 * RvSipPProfileKeyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PProfileKey header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the PProfileKey header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetDisplayName(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar*                 strDisplayName)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

     return SipPProfileKeyHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPProfileKeyHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PProfileKey header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PProfileKey header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPProfileKeyHeaderGetAddrSpec(
                                    IN RvSipPProfileKeyHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPProfileKeyHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PProfileKey header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PProfileKey header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the PProfileKey header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetAddrSpec(
                                        IN    RvSipPProfileKeyHeaderHandle hHeader,
                                        IN    RvSipAddressHandle       hAddrSpec)
{
    RvStatus			stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress			*pAddr           = (MsgAddress*)hAddrSpec;
    MsgPProfileKeyHeader		*pHeader         = (MsgPProfileKeyHeader*)hHeader;

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

            stat = RvSipAddrConstructInPProfileKeyHeader(hHeader,
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
 * SipPProfileKeyHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the PProfileKey Params in the PProfileKey header object.
 * Return Value: PProfileKey param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PProfileKey header object..
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetOtherParams(
									IN RvSipPProfileKeyHeaderHandle hHeader)
{
    return ((MsgPProfileKeyHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PProfileKey header other params field of the PProfileKey header object into a
 *          given buffer.
 *          Not all the PProfileKey header parameters have separated fields in the PProfileKey
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PProfileKey header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetOtherParams(
                                               IN RvSipPProfileKeyHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
	MsgPProfileKeyHeader   *pHeader         = (MsgPProfileKeyHeader*)hHeader;
	
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
								  SipPProfileKeyHeaderGetOtherParams(hHeader),
								  strBuffer,
								  bufferLen,
								  actualLen);
}

/***************************************************************************
 * SipPProfileKeyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PProfileKeyParam in the
 *          PProfileKeyHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PProfileKey header object.
 *            pOtherParams - The Other Params to be set in the PProfileKey header.
 *                          If NULL, the exist OtherParams string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPProfileKeyHeaderSetOtherParams(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32        newStr;
    MsgPProfileKeyHeader* pHeader;
    RvStatus       retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPProfileKeyHeader*) hHeader;

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
 * RvSipPProfileKeyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PProfileKey header object.
 *         Not all the PProfileKey header parameters have separated fields in the PProfileKey
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PProfileKey header object.
 *    strPProfileKeyParam - The extended parameters field to be set in the PProfileKey header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetOtherParams(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar *				 pPProfileKeyParam)
{
	return SipPProfileKeyHeaderSetOtherParams(hHeader, pPProfileKeyParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPProfileKeyHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetStrBadSyntax(
                                    IN RvSipPProfileKeyHeaderHandle hHeader)
{
    return ((MsgPProfileKeyHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPProfileKeyHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PProfileKey header,
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
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetStrBadSyntax(
                                               IN RvSipPProfileKeyHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPProfileKeyHeader*)hHeader)->hPool,
                                  ((MsgPProfileKeyHeader*)hHeader)->hPage,
                                  SipPProfileKeyHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPProfileKeyHeaderSetStrBadSyntax
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
RvStatus SipPProfileKeyHeaderSetStrBadSyntax(
                                  IN RvSipPProfileKeyHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPProfileKeyHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPProfileKeyHeader*)hHeader;

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
 * RvSipPProfileKeyHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PProfileKey header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetStrBadSyntax(
                                  IN RvSipPProfileKeyHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    return SipPProfileKeyHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PProfileKeyHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PProfileKeyHeaderClean( IN MsgPProfileKeyHeader* pHeader,
							    IN RvBool            bCleanBS)
{
    pHeader->hAddrSpec        = NULL;
    pHeader->strOtherParams   = UNDEFINED;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_P_PROFILE_KEY;
	pHeader->strDisplayName   = UNDEFINED;

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

