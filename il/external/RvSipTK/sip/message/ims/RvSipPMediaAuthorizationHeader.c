/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPMediaAuthorizationHeader.c								*
 *																					*
 * The file defines the methods of the PMediaAuthorization header object:			*
 * construct, destruct, copy, encode, parse and the ability to access and			*
 * change it's parameters.															*
 *																					*
 *      Author           Date														*
 *     ------           ------------												*
 *      Mickey           Dec.2005													*
 ************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPMediaAuthorizationHeader.h"
#include "_SipPMediaAuthorizationHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PMediaAuthorizationHeaderClean(IN MsgPMediaAuthorizationHeader* pHeader,
										   IN RvBool						bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PMediaAuthorization header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PMediaAuthorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPMediaAuthorizationHeaderHandle* hHeader)
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

    stat = RvSipPMediaAuthorizationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PMediaAuthorization Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PMediaAuthorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPMediaAuthorizationHeaderHandle* hHeader)
{
    MsgPMediaAuthorizationHeader*   pHeader;
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

    pHeader = (MsgPMediaAuthorizationHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPMediaAuthorizationHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPMediaAuthorizationHeaderConstruct - Failed to construct PMediaAuthorization header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PMediaAuthorizationHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPMediaAuthorizationHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPMediaAuthorizationHeaderConstruct - PMediaAuthorization header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PMediaAuthorization header 
 *			object to a destination PMediaAuthorization header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PMediaAuthorization header object.
 *    hSource      - Handle to the destination PMediaAuthorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderCopy(
                                         INOUT    RvSipPMediaAuthorizationHeaderHandle hDestination,
                                         IN    RvSipPMediaAuthorizationHeaderHandle hSource)
{
    MsgPMediaAuthorizationHeader*   source;
    MsgPMediaAuthorizationHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPMediaAuthorizationHeader*)hSource;
    dest   = (MsgPMediaAuthorizationHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* strToken */
	if(source->strToken > UNDEFINED)
    {
        dest->strToken = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
													  dest->hPool,
													  dest->hPage,
													  source->hPool,
													  source->hPage,
													  source->strToken);
        if(dest->strToken == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPMediaAuthorizationHeaderCopy - Failed to copy strToken"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pToken */
#ifdef SIP_DEBUG
        dest->pToken = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strToken);
#endif
    }
    else
    {
        dest->strToken = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pToken = NULL;
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
                "RvSipPMediaAuthorizationHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPMediaAuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PMediaAuthorization header object to a textual PMediaAuthorization header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PMediaAuthorization header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderEncode(
                                          IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPMediaAuthorizationHeader* pHeader;

    pHeader = (MsgPMediaAuthorizationHeader*)hHeader;

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
                "RvSipPMediaAuthorizationHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPMediaAuthorizationHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PMediaAuthorizationHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPMediaAuthorizationHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PMediaAuthorizationHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PMediaAuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PMediaAuthorization header (as string) on it.
 *          format: "P-Media-Authorization: "
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PMediaAuthorization header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PMediaAuthorizationHeaderEncode(
                                  IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPMediaAuthorizationHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPMediaAuthorizationHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PMediaAuthorizationHeaderEncode - Encoding PMediaAuthorization header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Media-Authorization" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Media-Authorization", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PMediaAuthorizationHeaderEncode - Failed to encoding PMediaAuthorization header. stat is %d",
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
                "PMediaAuthorizationHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

	/* Token */
	if(pHeader->strToken > UNDEFINED)
    {

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strToken,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PMediaAuthorizationHeaderEncode - Failed to encoding PMediaAuthorizationHeaderEncode header. stat is %d",
				stat));
			return stat;
		}
    }
    else
    {
        /* this is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PMediaAuthorizationHeaderEncode: Failed. No Token was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
	
    return stat;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PMediaAuthorization header into a PMediaAuthorization
 *			header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PMediaAuthorization header object.
 *    buffer    - Buffer containing a textual PMediaAuthorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderParse(
                                     IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPMediaAuthorizationHeader*	pHeader = (MsgPMediaAuthorizationHeader*)hHeader;
	RvStatus						rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PMediaAuthorizationHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_MEDIA_AUTHORIZATION,
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
 * RvSipPMediaAuthorizationHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PMediaAuthorization header value into a PMediaAuthorization header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPMediaAuthorizationHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PMediaAuthorization header object.
 *    buffer    - The buffer containing a textual PMediaAuthorization header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderParseValue(
                                     IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPMediaAuthorizationHeader*	pHeader = (MsgPMediaAuthorizationHeader*)hHeader;
	RvStatus						rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PMediaAuthorizationHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_MEDIA_AUTHORIZATION,
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
 * RvSipPMediaAuthorizationHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PMediaAuthorization header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderFix(
                                     IN RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPMediaAuthorizationHeader*	pHeader = (MsgPMediaAuthorizationHeader*)hHeader;
    RvStatus						rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPMediaAuthorizationHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPMediaAuthorizationHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPMediaAuthorizationHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PMediaAuthorization object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPMediaAuthorizationHeaderGetPool(RvSipPMediaAuthorizationHeaderHandle hHeader)
{
    return ((MsgPMediaAuthorizationHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PMediaAuthorization object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPMediaAuthorizationHeaderGetPage(RvSipPMediaAuthorizationHeaderHandle hHeader)
{
    return ((MsgPMediaAuthorizationHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PMediaAuthorization header fields are kept in a string format-for example, the
 *          PMediaAuthorization header VNetworkSpec name. In order to get such a field from the PMediaAuthorization header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PMediaAuthorization header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPMediaAuthorizationHeaderGetStringLength(
                                      IN  RvSipPMediaAuthorizationHeaderHandle     hHeader,
                                      IN  RvSipPMediaAuthorizationHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPMediaAuthorizationHeader* pHeader = (MsgPMediaAuthorizationHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_MEDIA_AUTHORIZATION_TOKEN:
        {
            stringOffset = SipPMediaAuthorizationHeaderGetToken(hHeader);
            break;
        }
        case RVSIP_P_MEDIA_AUTHORIZATION_BAD_SYNTAX:
        {
            stringOffset = SipPMediaAuthorizationHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPMediaAuthorizationHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPMediaAuthorizationHeaderGetToken
 * ------------------------------------------------------------------------
 * General:This method gets the Token in the PMediaAuthorization header object.
 * Return Value: Token offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PMediaAuthorization header object..
 ***************************************************************************/
RvInt32 SipPMediaAuthorizationHeaderGetToken(IN RvSipPMediaAuthorizationHeaderHandle hHeader)
{
    return ((MsgPMediaAuthorizationHeader*)hHeader)->strToken;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderGetToken
 * ------------------------------------------------------------------------
 * General: Copies the PMediaAuthorization header Token field of the PMediaAuthorization header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PMediaAuthorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderGetToken(
                                               IN RvSipPMediaAuthorizationHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPMediaAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgPMediaAuthorizationHeader*)hHeader)->hPage,
                                  SipPMediaAuthorizationHeaderGetToken(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPMediaAuthorizationHeaderSetToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Token in the
 *          PMediaAuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PMediaAuthorization header object.
 *            pToken - The Token to be set in the PMediaAuthorization header.
 *                          If NULL, the existing Token string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPMediaAuthorizationHeaderSetToken(
                                     IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                pToken,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32							newStr;
    MsgPMediaAuthorizationHeader*	pHeader;
    RvStatus						retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPMediaAuthorizationHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pToken,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strToken = newStr;
#ifdef SIP_DEBUG
    pHeader->pToken = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strToken);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderSetToken
 * ------------------------------------------------------------------------
 * General:Sets the Token field in the PMediaAuthorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the PMediaAuthorization header object.
 *    strToken - The extended parameters field to be set in the PMediaAuthorization header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderSetToken(
                                     IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                pToken)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPMediaAuthorizationHeaderSetToken(hHeader, pToken, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPMediaAuthorizationHeaderGetStrBadSyntax(
                                    IN RvSipPMediaAuthorizationHeaderHandle hHeader)
{
    return ((MsgPMediaAuthorizationHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPMediaAuthorizationHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PMediaAuthorization header,
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
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderGetStrBadSyntax(
                                               IN RvSipPMediaAuthorizationHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPMediaAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgPMediaAuthorizationHeader*)hHeader)->hPage,
                                  SipPMediaAuthorizationHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPMediaAuthorizationHeaderSetStrBadSyntax
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
RvStatus SipPMediaAuthorizationHeaderSetStrBadSyntax(
                                  IN RvSipPMediaAuthorizationHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPMediaAuthorizationHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPMediaAuthorizationHeader*)hHeader;

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
 * RvSipPMediaAuthorizationHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PMediaAuthorization header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPMediaAuthorizationHeaderSetStrBadSyntax(
                                  IN RvSipPMediaAuthorizationHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPMediaAuthorizationHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PMediaAuthorizationHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PMediaAuthorizationHeaderClean( 
								IN MsgPMediaAuthorizationHeader* pHeader,
							    IN RvBool						 bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION;
	pHeader->strToken			= UNDEFINED;

#ifdef SIP_DEBUG
	pHeader->pToken				= NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax	= UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax		= NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

