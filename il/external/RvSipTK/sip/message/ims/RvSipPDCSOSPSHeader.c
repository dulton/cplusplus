/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPDCSOSPSHeader.c									  *
 *                                                                            *
 * The file defines the methods of the PDCSOSPS header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ----------                                            *
 *      Mickey           Jan.2006                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

#include "RvSipPDCSOSPSHeader.h"
#include "_SipPDCSOSPSHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PDCSOSPSHeaderClean(IN MsgPDCSOSPSHeader* pHeader,
								IN RvBool             bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSOSPSHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSOSPS header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderConstructInMsg(
                               IN  RvSipMsgHandle				hSipMsg,
                               IN  RvBool						pushHeaderAtHead,
                               OUT RvSipPDCSOSPSHeaderHandle*	hHeader)
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

    stat = RvSipPDCSOSPSHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_DCS_OSPS,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSOSPS Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderConstruct(
                                   IN  RvSipMsgMgrHandle          hMsgMgr,
                                   IN  HRPOOL                     hPool,
                                   IN  HPAGE                      hPage,
                                   OUT RvSipPDCSOSPSHeaderHandle* hHeader)
{
    MsgPDCSOSPSHeader*   pHeader;
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

    pHeader = (MsgPDCSOSPSHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPDCSOSPSHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSOSPSHeaderConstruct - Failed to construct PDCSOSPS header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PDCSOSPSHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPDCSOSPSHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSOSPSHeaderConstruct - PDCSOSPS header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSOSPS header object to a destination PDCSOSPS
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSOSPS header object.
 *    hSource      - Handle to the destination PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderCopy(
                                 INOUT	RvSipPDCSOSPSHeaderHandle hDestination,
                                 IN		RvSipPDCSOSPSHeaderHandle hSource)
{
    MsgPDCSOSPSHeader*   source;
    MsgPDCSOSPSHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPDCSOSPSHeader*)hSource;
    dest   = (MsgPDCSOSPSHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* OSPS Tag */
	dest->eOSPSTag = source->eOSPSTag;
    if(source->strOSPSTag > UNDEFINED)
    {
        dest->strOSPSTag = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strOSPSTag);
        if(dest->strOSPSTag == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSOSPSHeaderCopy - Failed to copy OSPSTag"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pOSPSTag */
#ifdef SIP_DEBUG
        dest->pOSPSTag = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strOSPSTag);
#endif
    }
    else
    {
        dest->strOSPSTag = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOSPSTag = NULL;
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
                "RvSipPDCSOSPSHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPDCSOSPSHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSOSPS header object to a textual PDCSOSPS header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSOSPS header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderEncode(
                              IN    RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN    HRPOOL						hPool,
                              OUT   HPAGE*						hPage,
                              OUT   RvUint32*					length)
{
    RvStatus stat;
    MsgPDCSOSPSHeader* pHeader;

    pHeader = (MsgPDCSOSPSHeader*)hHeader;

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
                "RvSipPDCSOSPSHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSOSPSHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PDCSOSPSHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSOSPSHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PDCSOSPSHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PDCSOSPSHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PDCSOSPS header (as string) on it.
 *          format: "P-DCS-OSPS: OSPS-Tag"
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PDCSOSPS header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PDCSOSPSHeaderEncode(
                          IN    RvSipPDCSOSPSHeaderHandle	hHeader,
                          IN    HRPOOL						hPool,
                          IN    HPAGE						hPage,
                          IN    RvBool						bInUrlHeaders,
                          INOUT RvUint32*					length)
{
    MsgPDCSOSPSHeader*  pHeader;
    RvStatus			stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPDCSOSPSHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PDCSOSPSHeaderEncode - Encoding PDCSOSPS header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-DCS-OSPS" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-DCS-OSPS", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSOSPSHeaderEncode - Failed to encoding PDCSOSPS header. stat is %d",
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
                "PDCSOSPSHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

    /* OSPS Tag */
	stat = MsgUtilsEncodeOSPSTag(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->eOSPSTag,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strOSPSTag,
									length);
	if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSOSPSHeaderEncode - Failed to encode OSPS Tag string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
		return stat;
    }

    return stat;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSOSPS header-for example,
 *          "P-DCS-OSPS: BLV"-into a PDCSOSPS header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSOSPS header object.
 *    buffer    - Buffer containing a textual PDCSOSPS header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderParse(
							 IN    RvSipPDCSOSPSHeaderHandle hHeader,
							 IN    RvChar*					 buffer)
{
    MsgPDCSOSPSHeader*	pHeader = (MsgPDCSOSPSHeader*)hHeader;
	RvStatus			rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PDCSOSPSHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_OSPS,
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
 * RvSipPDCSOSPSHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSOSPS header value into an PDCSOSPS header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSOSPSHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSOSPS header object.
 *    buffer    - The buffer containing a textual PDCSOSPS header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderParseValue(
                                 IN    RvSipPDCSOSPSHeaderHandle hHeader,
                                 IN    RvChar*					 buffer)
{
    MsgPDCSOSPSHeader*	pHeader = (MsgPDCSOSPSHeader*)hHeader;
	RvStatus			rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}


    PDCSOSPSHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_OSPS,
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
 * RvSipPDCSOSPSHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSOSPS header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderFix(
                             IN RvSipPDCSOSPSHeaderHandle	hHeader,
                             IN RvChar*						pFixedBuffer)
{
    MsgPDCSOSPSHeader*	pHeader = (MsgPDCSOSPSHeader*)hHeader;
    RvStatus			rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPDCSOSPSHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSOSPSHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSOSPSHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPDCSOSPSHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSOSPS object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPDCSOSPSHeaderGetPool(RvSipPDCSOSPSHeaderHandle hHeader)
{
    return ((MsgPDCSOSPSHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPDCSOSPSHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSOSPS object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPDCSOSPSHeaderGetPage(RvSipPDCSOSPSHeaderHandle hHeader)
{
    return ((MsgPDCSOSPSHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSOSPS header fields are kept in a string format-for example, the
 *          PDCSOSPS header VNetworkSpec name. In order to get such a field from the PDCSOSPS header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSOSPS header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSOSPSHeaderGetStringLength(
                              IN  RvSipPDCSOSPSHeaderHandle     hHeader,
                              IN  RvSipPDCSOSPSHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPDCSOSPSHeader* pHeader = (MsgPDCSOSPSHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_DCS_OSPS_TAG:
        {
            stringOffset = SipPDCSOSPSHeaderGetStrOSPSTag(hHeader);
            break;
        }
        case RVSIP_P_DCS_OSPS_BAD_SYNTAX:
        {
            stringOffset = SipPDCSOSPSHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSOSPSHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPDCSOSPSHeaderGetStrOSPSTag
 * ------------------------------------------------------------------------
 * General:This method gets the StrOSPSTag embedded in the PDCSOSPS header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSOSPS header object..
 ***************************************************************************/
RvInt32 SipPDCSOSPSHeaderGetStrOSPSTag(
                                    IN RvSipPDCSOSPSHeaderHandle hHeader)
{
    return ((MsgPDCSOSPSHeader*)hHeader)->strOSPSTag;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStrOSPSTag
 * ------------------------------------------------------------------------
 * General: Copies the StrOSPSTag from the PDCSOSPS header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderGetStrOSPSTag(
                                               IN RvSipPDCSOSPSHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSOSPSHeader*)hHeader)->hPool,
                                  ((MsgPDCSOSPSHeader*)hHeader)->hPage,
                                  SipPDCSOSPSHeaderGetStrOSPSTag(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSOSPSHeaderSetOSPSTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pOSPSTag in the
 *          PDCSOSPSHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PDCSOSPS header object.
 *         pOSPSTag - The OSPSTag Type to be set in the PDCSOSPS header - If
 *                NULL, the existing OSPSTag Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSOSPSHeaderSetOSPSTag(
                         IN    RvSipPDCSOSPSHeaderHandle	hHeader,
						 IN	   RvSipOSPSTagType				eOSPSTag,
                         IN    RvChar*						pOSPSTag,
                         IN    HRPOOL						hPool,
                         IN    HPAGE						hPage,
                         IN    RvInt32						strOffset)
{
    RvInt32				newStr;
    MsgPDCSOSPSHeader*	pHeader;
    RvStatus			retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSOSPSHeader*) hHeader;

	pHeader->eOSPSTag = eOSPSTag;

	if(eOSPSTag == RVSIP_P_DCS_OSPS_TAG_TYPE_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOSPSTag,
									  pHeader->hPool, pHeader->hPage, &newStr);
		if(RV_OK != retStatus)
		{
			return retStatus;
		}
		pHeader->strOSPSTag = newStr;
#ifdef SIP_DEBUG
	    pHeader->pOSPSTag = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strOSPSTag);
#endif
    }
	else
	{
		pHeader->strOSPSTag = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pOSPSTag = NULL;
#endif
	}
    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderSetOSPSTag
 * ------------------------------------------------------------------------
 * General:Sets the OSPSTag Type in the PDCSOSPS header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strOSPSTag - The OSPSTag Type to be set in the PDCSOSPS header. If NULL is supplied, the
 *                 existing OSPSTag Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderSetOSPSTag(
                             IN    RvSipPDCSOSPSHeaderHandle	hHeader,
							 IN	   RvSipOSPSTagType				eOSPSTag,
                             IN    RvChar*						strOSPSTag)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

     return SipPDCSOSPSHeaderSetOSPSTag(hHeader, eOSPSTag, strOSPSTag, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetOSPSTag
 * ------------------------------------------------------------------------
 * General: Copies the StrOSPSTag from the PDCSOSPS header into a given buffer.
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
RVAPI RvSipOSPSTagType RVCALLCONV RvSipPDCSOSPSHeaderGetOSPSTag(
									IN RvSipPDCSOSPSHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED;
	}

    return ((MsgPDCSOSPSHeader*)hHeader)->eOSPSTag;
}

/***************************************************************************
 * SipPDCSOSPSHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSOSPSHeaderGetStrBadSyntax(
								IN RvSipPDCSOSPSHeaderHandle hHeader)
{
    return ((MsgPDCSOSPSHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSOSPS header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderGetStrBadSyntax(
                                       IN RvSipPDCSOSPSHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSOSPSHeader*)hHeader)->hPool,
                                  ((MsgPDCSOSPSHeader*)hHeader)->hPage,
                                  SipPDCSOSPSHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSOSPSHeaderSetStrBadSyntax
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
RvStatus SipPDCSOSPSHeaderSetStrBadSyntax(
                              IN RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN RvChar*					strBadSyntax,
                              IN HRPOOL						hPool,
                              IN HPAGE						hPage,
                              IN RvInt32					strBadSyntaxOffset)
{
    MsgPDCSOSPSHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPDCSOSPSHeader*)hHeader;

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
 * RvSipPDCSOSPSHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSOSPS header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderSetStrBadSyntax(
                              IN RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN RvChar*					strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSOSPSHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PDCSOSPSHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PDCSOSPSHeaderClean( IN MsgPDCSOSPSHeader*	pHeader,
							  	 IN RvBool				bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_DCS_OSPS;
	pHeader->eOSPSTag			= RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED;
	pHeader->strOSPSTag			= UNDEFINED;

#ifdef SIP_DEBUG
	pHeader->pOSPSTag			= NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

