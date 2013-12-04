/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPVisitedNetworkIDHeader.c						          *
 *                                                                            *
 * The file defines the methods of the PVisitedNetworkID header object:          *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey            Nov.2005                                            *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPVisitedNetworkIDHeader.h"
#include "_SipPVisitedNetworkIDHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PVisitedNetworkIDHeaderClean(IN MsgPVisitedNetworkIDHeader* pHeader,
										 IN RvBool						bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PVisitedNetworkID header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PVisitedNetworkID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPVisitedNetworkIDHeaderHandle* hHeader)
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

    stat = RvSipPVisitedNetworkIDHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PVisitedNetworkID Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PVisitedNetworkID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPVisitedNetworkIDHeaderHandle* hHeader)
{
    MsgPVisitedNetworkIDHeader*   pHeader;
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

    pHeader = (MsgPVisitedNetworkIDHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPVisitedNetworkIDHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPVisitedNetworkIDHeaderConstruct - Failed to construct PVisitedNetworkID header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PVisitedNetworkIDHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPVisitedNetworkIDHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPVisitedNetworkIDHeaderConstruct - PVisitedNetworkID header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PVisitedNetworkID header object to a destination PVisitedNetworkID
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PVisitedNetworkID header object.
 *    hSource      - Handle to the destination PVisitedNetworkID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderCopy(
                                         INOUT    RvSipPVisitedNetworkIDHeaderHandle hDestination,
                                         IN    RvSipPVisitedNetworkIDHeaderHandle hSource)
{
    MsgPVisitedNetworkIDHeader*   source;
    MsgPVisitedNetworkIDHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPVisitedNetworkIDHeader*)hSource;
    dest   = (MsgPVisitedNetworkIDHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* strVNetworkSpec */
    if(source->strVNetworkSpec > UNDEFINED)
    {
        dest->strVNetworkSpec = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strVNetworkSpec);
        if(dest->strVNetworkSpec == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPVisitedNetworkIDHeaderCopy - Failed to copy VNetworkSpec"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pVNetworkSpec = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strVNetworkSpec);
#endif
    }
    else
    {
        dest->strVNetworkSpec = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pVNetworkSpec = NULL;
#endif
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
                      "RvSipPVisitedNetworkIDHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPVisitedNetworkIDHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPVisitedNetworkIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PVisitedNetworkID header object to a textual PVisitedNetworkID header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PVisitedNetworkID header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderEncode(
                                          IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPVisitedNetworkIDHeader* pHeader;

    pHeader = (MsgPVisitedNetworkIDHeader*)hHeader;

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
                "RvSipPVisitedNetworkIDHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPVisitedNetworkIDHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PVisitedNetworkIDHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPVisitedNetworkIDHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PVisitedNetworkIDHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PVisitedNetworkIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PVisitedNetworkID header (as string) on it.
 *          format: "P-Visited-Network-ID: "
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PVisitedNetworkID header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PVisitedNetworkIDHeaderEncode(
                                  IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPVisitedNetworkIDHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPVisitedNetworkIDHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PVisitedNetworkIDHeaderEncode - Encoding PVisitedNetworkID header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Visited-Network-ID" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Visited-Network-ID", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PVisitedNetworkIDHeaderEncode - Failed to encoding PVisitedNetworkID header. stat is %d",
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
                "PVisitedNetworkIDHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* VNetworkSpec */
    if(pHeader->strVNetworkSpec > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strVNetworkSpec,
                                    length);
    }

    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set PVisitedNetworkIDParmas */
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
            "PVisitedNetworkIDHeaderEncode - Failed to encoding PVisitedNetworkID header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PVisitedNetworkID header-for example,
 *          "PVisitedNetworkID:sip:172.20.5.3:5060"-into a PVisitedNetworkID header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PVisitedNetworkID header object.
 *    buffer    - Buffer containing a textual PVisitedNetworkID header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderParse(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPVisitedNetworkIDHeader*	pHeader = (MsgPVisitedNetworkIDHeader*)hHeader;
	RvStatus					rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PVisitedNetworkIDHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_VISITED_NETWORK_ID,
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
 * RvSipPVisitedNetworkIDHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PVisitedNetworkID header value into an PVisitedNetworkID header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPVisitedNetworkIDHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PVisitedNetworkID header object.
 *    buffer    - The buffer containing a textual PVisitedNetworkID header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderParseValue(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPVisitedNetworkIDHeader*	pHeader = (MsgPVisitedNetworkIDHeader*)hHeader;
	RvStatus					rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PVisitedNetworkIDHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_VISITED_NETWORK_ID,
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
 * RvSipPVisitedNetworkIDHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PVisitedNetworkID header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderFix(
                                     IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPVisitedNetworkIDHeader* pHeader = (MsgPVisitedNetworkIDHeader*)hHeader;
    RvStatus					rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPVisitedNetworkIDHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPVisitedNetworkIDHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPVisitedNetworkIDHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PVisitedNetworkID object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPVisitedNetworkIDHeaderGetPool(RvSipPVisitedNetworkIDHeaderHandle hHeader)
{
    return ((MsgPVisitedNetworkIDHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PVisitedNetworkID object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPVisitedNetworkIDHeaderGetPage(RvSipPVisitedNetworkIDHeaderHandle hHeader)
{
    return ((MsgPVisitedNetworkIDHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PVisitedNetworkID header fields are kept in a string format-for example, the
 *          PVisitedNetworkID header VNetworkSpec name. In order to get such a field from the PVisitedNetworkID header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PVisitedNetworkID header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPVisitedNetworkIDHeaderGetStringLength(
                                      IN  RvSipPVisitedNetworkIDHeaderHandle     hHeader,
                                      IN  RvSipPVisitedNetworkIDHeaderStringName stringName)
{
    RvInt32						stringOffset;
    MsgPVisitedNetworkIDHeader* pHeader = (MsgPVisitedNetworkIDHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_VISITED_NETWORK_ID_VNETWORK_SPEC:
        {
            stringOffset = SipPVisitedNetworkIDHeaderGetVNetworkSpec(hHeader);
            break;
        }
        case RVSIP_P_VISITED_NETWORK_ID_OTHER_PARAMS:
        {
            stringOffset = SipPVisitedNetworkIDHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_VISITED_NETWORK_ID_BAD_SYNTAX:
        {
            stringOffset = SipPVisitedNetworkIDHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPVisitedNetworkIDHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPVisitedNetworkIDHeaderGetVNetworkSpec
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PVisitedNetworkID header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PVisitedNetworkID header object..
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetVNetworkSpec(
                                    IN RvSipPVisitedNetworkIDHeaderHandle hHeader)
{
    return ((MsgPVisitedNetworkIDHeader*)hHeader)->strVNetworkSpec;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetVNetworkSpec
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PVisitedNetworkID header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetVNetworkSpec(
                                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPVisitedNetworkIDHeader*)hHeader)->hPool,
                                  ((MsgPVisitedNetworkIDHeader*)hHeader)->hPage,
                                  SipPVisitedNetworkIDHeaderGetVNetworkSpec(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetVNetworkSpec
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pVNetworkSpec in the
 *          PVisitedNetworkIDHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PVisitedNetworkID header object.
 *         pVNetworkSpec - The display name to be set in the PVisitedNetworkID header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPVisitedNetworkIDHeaderSetVNetworkSpec(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar *                pVNetworkSpec,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPVisitedNetworkIDHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPVisitedNetworkIDHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pVNetworkSpec,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strVNetworkSpec = newStr;
#ifdef SIP_DEBUG
    pHeader->pVNetworkSpec = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strVNetworkSpec);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderSetVNetworkSpec
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PVisitedNetworkID header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strVNetworkSpec - The display name to be set in the PVisitedNetworkID header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetVNetworkSpec(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar*                 strVNetworkSpec)
{
    if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

     return SipPVisitedNetworkIDHeaderSetVNetworkSpec(hHeader, strVNetworkSpec, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PVisitedNetworkID header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PVisitedNetworkID header object..
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetOtherParams(
                                            IN RvSipPVisitedNetworkIDHeaderHandle hHeader)
{
    return ((MsgPVisitedNetworkIDHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PVisitedNetworkID header other params field of the PVisitedNetworkID header object into a
 *          given buffer.
 *          Not all the PVisitedNetworkID header parameters have separated fields in the PVisitedNetworkID
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PVisitedNetworkID header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetOtherParams(
                                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPVisitedNetworkIDHeader*)hHeader)->hPool,
                                  ((MsgPVisitedNetworkIDHeader*)hHeader)->hPage,
                                  SipPVisitedNetworkIDHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PVisitedNetworkIDParam in the
 *          PVisitedNetworkIDHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PVisitedNetworkID header object.
 *            pPVisitedNetworkIDParam - The Other Params to be set in the PVisitedNetworkID header.
 *                          If NULL, the exist OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPVisitedNetworkIDHeaderSetOtherParams(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPVisitedNetworkIDHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPVisitedNetworkIDHeader*) hHeader;

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
 * RvSipPVisitedNetworkIDHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PVisitedNetworkID header object.
 *         Not all the PVisitedNetworkID header parameters have separated fields in the PVisitedNetworkID
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PVisitedNetworkID header object.
 *    strPVisitedNetworkIDParam - The extended parameters field to be set in the PVisitedNetworkID header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetOtherParams(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar *                pPVisitedNetworkIDParam)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPVisitedNetworkIDHeaderSetOtherParams(hHeader, pPVisitedNetworkIDParam, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetStrBadSyntax(
                                    IN RvSipPVisitedNetworkIDHeaderHandle hHeader)
{
    return ((MsgPVisitedNetworkIDHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PVisitedNetworkID header,
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetStrBadSyntax(
                                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPVisitedNetworkIDHeader*)hHeader)->hPool,
                                  ((MsgPVisitedNetworkIDHeader*)hHeader)->hPage,
                                  SipPVisitedNetworkIDHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetStrBadSyntax
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
RvStatus SipPVisitedNetworkIDHeaderSetStrBadSyntax(
                                  IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPVisitedNetworkIDHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPVisitedNetworkIDHeader*)hHeader;

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
 * RvSipPVisitedNetworkIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PVisitedNetworkID header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetStrBadSyntax(
                                  IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPVisitedNetworkIDHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PVisitedNetworkIDHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PVisitedNetworkIDHeaderClean( IN MsgPVisitedNetworkIDHeader* pHeader,
							    IN RvBool            bCleanBS)
{
    pHeader->strOtherParams   = UNDEFINED;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID;
	pHeader->strVNetworkSpec   = UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pOtherParams     = NULL;
	pHeader->pVNetworkSpec     = NULL;
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

