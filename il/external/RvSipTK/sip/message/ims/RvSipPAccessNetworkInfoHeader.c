/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPAccessNetworkInfoHeader.c						      *
 *                                                                            *
 * The file defines the methods of the PAccessNetworkInfo header object:      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ----------                                            *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPAccessNetworkInfoHeader.h"
#include "_SipPAccessNetworkInfoHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PAccessNetworkInfoHeaderClean(IN MsgPAccessNetworkInfoHeader* pHeader,
										  IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PAccessNetworkInfo header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PAccessNetworkInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPAccessNetworkInfoHeaderHandle* hHeader)
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

    stat = RvSipPAccessNetworkInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PAccessNetworkInfo Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PAccessNetworkInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPAccessNetworkInfoHeaderHandle* hHeader)
{
    MsgPAccessNetworkInfoHeader*   pHeader;
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

    pHeader = (MsgPAccessNetworkInfoHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPAccessNetworkInfoHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPAccessNetworkInfoHeaderConstruct - Failed to construct PAccessNetworkInfo header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PAccessNetworkInfoHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPAccessNetworkInfoHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPAccessNetworkInfoHeaderConstruct - PAccessNetworkInfo header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PAccessNetworkInfo header object to a destination PAccessNetworkInfo
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PAccessNetworkInfo header object.
 *    hSource      - Handle to the destination PAccessNetworkInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderCopy(
                                         INOUT    RvSipPAccessNetworkInfoHeaderHandle hDestination,
                                         IN    RvSipPAccessNetworkInfoHeaderHandle hSource)
{
    MsgPAccessNetworkInfoHeader*   source;
    MsgPAccessNetworkInfoHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
    source = (MsgPAccessNetworkInfoHeader*)hSource;
    dest   = (MsgPAccessNetworkInfoHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* Access Type */
	dest->eAccessType = source->eAccessType;
    if(source->strAccessType > UNDEFINED)
    {
        dest->strAccessType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAccessType);
        if(dest->strAccessType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy AccessType"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pAccessType */
#ifdef SIP_DEBUG
        dest->pAccessType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strAccessType);
#endif
    }
    else
    {
        dest->strAccessType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAccessType = NULL;
#endif
    }

    /* bNetworkProvided */
	dest->bNetworkProvided = source->bNetworkProvided;

	/* strCgi3gpp */
	if(source->strCgi3gpp > UNDEFINED)
    {
        dest->strCgi3gpp = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strCgi3gpp);
        if(dest->strCgi3gpp == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy strCgi3gpp"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pCgi3gpp */
#ifdef SIP_DEBUG
        dest->pCgi3gpp = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strCgi3gpp);
#endif
    }
    else
    {
        dest->strCgi3gpp = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCgi3gpp = NULL;
#endif
    }
	
	/* strUtranCellId3gpp */
	if(source->strUtranCellId3gpp > UNDEFINED)
    {
        dest->strUtranCellId3gpp = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
																dest->hPool,
																dest->hPage,
																source->hPool,
																source->hPage,
																source->strUtranCellId3gpp);
        if(dest->strUtranCellId3gpp == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy strUtranCellId3gpp"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pUtranCellId3gpp */
#ifdef SIP_DEBUG
        dest->pUtranCellId3gpp = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
														dest->strUtranCellId3gpp);
#endif
    }
    else
    {
        dest->strUtranCellId3gpp = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pUtranCellId3gpp = NULL;
#endif
    }
	
	/* strIWlanNodeID */
	if(source->strIWlanNodeID > UNDEFINED)
    {
        dest->strIWlanNodeID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
																dest->hPool,
																dest->hPage,
																source->hPool,
																source->hPage,
																source->strIWlanNodeID);
        if(dest->strIWlanNodeID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy strIWlanNodeID"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pIWlanNodeID */
#ifdef SIP_DEBUG
        dest->pIWlanNodeID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
														dest->strIWlanNodeID);
#endif
    }
    else
    {
        dest->strIWlanNodeID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIWlanNodeID = NULL;
#endif
    }

	/* DslLocation */
	if(source->strDslLocation > UNDEFINED)
    {
        dest->strDslLocation = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strDslLocation);
        if(dest->strDslLocation == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy strDslLocation"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the DslLocation */
#ifdef SIP_DEBUG
        dest->pDslLocation = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
													dest->strDslLocation);
#endif
    }
    else
    {
        dest->strDslLocation = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDslLocation = NULL;
#endif
    }

	/* strCi3gpp2 */
	if(source->strCi3gpp2 > UNDEFINED)
    {
        dest->strCi3gpp2 = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                        dest->hPool,
                                                        dest->hPage,
                                                        source->hPool,
                                                        source->hPage,
                                                        source->strCi3gpp2);
        if(dest->strCi3gpp2 == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderCopy - Failed to copy strCi3gpp2"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pCi3gpp2 */
#ifdef SIP_DEBUG
        dest->pCi3gpp2 = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strCi3gpp2);
#endif
    }
    else
    {
        dest->strCi3gpp2 = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCi3gpp2 = NULL;
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
                      "RvSipPAccessNetworkInfoHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPAccessNetworkInfoHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPAccessNetworkInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PAccessNetworkInfo header object to a textual PAccessNetworkInfo header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PAccessNetworkInfo header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderEncode(
                                          IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPAccessNetworkInfoHeader* pHeader;

    pHeader = (MsgPAccessNetworkInfoHeader*)hHeader;

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
                "RvSipPAccessNetworkInfoHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PAccessNetworkInfoHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PAccessNetworkInfoHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PAccessNetworkInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PAccessNetworkInfo header (as string) on it.
 *          format: "P-Access-Network-Info: IEEE-802.11a"
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PAccessNetworkInfo header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PAccessNetworkInfoHeaderEncode(
                                  IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPAccessNetworkInfoHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}
	
    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PAccessNetworkInfoHeaderEncode - Encoding PAccessNetworkInfo header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Access-Network-Info" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Access-Network-Info", length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PAccessNetworkInfoHeaderEncode - Failed to encoding PAccessNetworkInfo header. stat is %d",
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
                "PAccessNetworkInfoHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

    /* Access Type */
	stat = MsgUtilsEncodeAccessType(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->eAccessType,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strAccessType,
									length);
	if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PAccessNetworkInfoHeaderEncode - Failed to encode Access Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
		return stat;
    }

    /* network-provided */
    if(pHeader->bNetworkProvided == RV_TRUE)
	{
		/* set ";" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											"network-provided", length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"PAccessNetworkInfoHeaderEncode - Failed to encode network-provided. stat is %d",
			stat));
		}
	}
    
	/* Cgi-3gpp (insert ";" in the beginning) */
    if(pHeader->strCgi3gpp > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "cgi-3gpp=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "cgi-3gpp", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the Cgi-3gpp string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCgi3gpp,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Cgi-3gpp String. stat is %d",
				stat));
		}
    }

	/* Utran-Cell-Id-3gpp (insert ";" in the beginning) */
    if(pHeader->strUtranCellId3gpp > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "utran-cell-id-3gpp=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "utran-cell-id-3gpp", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the utran-cell-id-3gpp string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strUtranCellId3gpp,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode utran-cell-id-3gpp String. stat is %d",
				stat));
		}
    }
	
	/* i-wlan-node-id (insert ";" in the beginning) */
    if(pHeader->strIWlanNodeID > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "i-wlan-node-id=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "i-wlan-node-id", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the i-wlan-node-id string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strIWlanNodeID,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode i-wlan-node-id String. stat is %d",
				stat));
		}
    }

	/* dsl-location (insert ";" in the beginning) */
    if(pHeader->strDslLocation > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "dsl-location=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "dsl-location", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		/* set the dsl-location string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDslLocation,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode dsl-location String. stat is %d",
				stat));
		}
    }
    
	/* ci-3gpp2 (insert ";" in the beginning) */
    if(pHeader->strCi3gpp2 > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "utran-cell-id-3gpp=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "ci-3gpp2", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the ci-3gpp2 string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCi3gpp2,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PAccessNetworkInfoHeaderEncode - Failed to encode ci-3gpp2 String. stat is %d",
				stat));
		}
    }

    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set OtherParmas */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
    }

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PAccessNetworkInfoHeaderEncode - Failed to encoding PAccessNetworkInfo header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAccessNetworkInfo header-for example,
 *          "PAccessNetworkInfo:IEEE802.11a"-into a PAccessNetworkInfo header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PAccessNetworkInfo header object.
 *    buffer    - Buffer containing a textual PAccessNetworkInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderParse(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                     IN    RvChar*								buffer)
{
    MsgPAccessNetworkInfoHeader*	pHeader = (MsgPAccessNetworkInfoHeader*)hHeader;
	RvStatus						rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    PAccessNetworkInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_ACCESS_NETWORK_INFO,
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
 * RvSipPAccessNetworkInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAccessNetworkInfo header value into an PAccessNetworkInfo header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPAccessNetworkInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PAccessNetworkInfo header object.
 *    buffer    - The buffer containing a textual PAccessNetworkInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderParseValue(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPAccessNetworkInfoHeader*    pHeader = (MsgPAccessNetworkInfoHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PAccessNetworkInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_ACCESS_NETWORK_INFO,
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
 * RvSipPAccessNetworkInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PAccessNetworkInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderFix(
                                     IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPAccessNetworkInfoHeader* pHeader = (MsgPAccessNetworkInfoHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPAccessNetworkInfoHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPAccessNetworkInfoHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPAccessNetworkInfoHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PAccessNetworkInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPAccessNetworkInfoHeaderGetPool(RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PAccessNetworkInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPAccessNetworkInfoHeaderGetPage(RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PAccessNetworkInfo header fields are kept in a string format-for example, the
 *          PAccessNetworkInfo header VNetworkSpec name. In order to get such a field from the PAccessNetworkInfo header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PAccessNetworkInfo header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStringLength(
                                      IN  RvSipPAccessNetworkInfoHeaderHandle     hHeader,
                                      IN  RvSipPAccessNetworkInfoHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPAccessNetworkInfoHeader* pHeader = (MsgPAccessNetworkInfoHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_ACCESS_NETWORK_INFO_ACCESS_TYPE:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrAccessType(hHeader);
            break;
        }
		case RVSIP_P_ACCESS_NETWORK_INFO_CGI_3GPP:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrCgi3gpp(hHeader);
            break;
        }
		case RVSIP_P_ACCESS_NETWORK_INFO_UTRAN_CELL_ID_3GPP:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(hHeader);
            break;
        }
		case RVSIP_P_ACCESS_NETWORK_INFO_I_WLAN_NODE_ID:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrIWlanNodeID(hHeader);
            break;
        }
        case RVSIP_P_ACCESS_NETWORK_INFO_DSL_LOCATION:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrDslLocation(hHeader);
            break;
        }
		case RVSIP_P_ACCESS_NETWORK_INFO_CI_3GPP2:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrCi3gpp2(hHeader);
            break;
        }
		case RVSIP_P_ACCESS_NETWORK_INFO_OTHER_PARAMS:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_ACCESS_NETWORK_INFO_BAD_SYNTAX:
        {
            stringOffset = SipPAccessNetworkInfoHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAccessNetworkInfoHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPAccessNetworkInfoHeaderGetStrAccessType
 * ------------------------------------------------------------------------
 * General:This method gets the StrAccessType embedded in the PAccessNetworkInfo header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrAccessType(
                                    IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strAccessType;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrAccessType
 * ------------------------------------------------------------------------
 * General: Copies the StrAccessType from the PAccessNetworkInfo header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrAccessType(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrAccessType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetAccessType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pAccessType in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PAccessNetworkInfo header object.
 *         pAccessType - The Access Type to be set in the PAccessNetworkInfo header - If
 *                NULL, the existing Access Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetAccessType(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
							 IN	   RvSipPAccessNetworkInfoAccessType	eAccessType,
                             IN    RvChar *								pAccessType,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset)
{
    RvInt32							newStr;
    MsgPAccessNetworkInfoHeader*	pHeader;
    RvStatus						retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

	pHeader->eAccessType = eAccessType;

	if(eAccessType == RVSIP_ACCESS_TYPE_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pAccessType,
									  pHeader->hPool, pHeader->hPage, &newStr);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}
		pHeader->strAccessType = newStr;
#ifdef SIP_DEBUG
	    pHeader->pAccessType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strAccessType);
#endif
    }
	else
	{
		pHeader->strAccessType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pAccessType = NULL;
#endif
	}
    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetAccessType
 * ------------------------------------------------------------------------
 * General:Sets the Access Type in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strAccessType - The Access Type to be set in the PAccessNetworkInfo header. If NULL is supplied, the
 *                 existing Access Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetAccessType(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
									 IN	   RvSipPAccessNetworkInfoAccessType eAccessType,
                                     IN    RvChar*                 strAccessType)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    
	return SipPAccessNetworkInfoHeaderSetAccessType(hHeader, eAccessType, strAccessType, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetAccessType
 * ------------------------------------------------------------------------
 * General: Returns the enumeration value for the access type.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 * output:RvSipPAccessNetworkInfoAccessType - enumeration for the access type.
 ***************************************************************************/
RVAPI RvSipPAccessNetworkInfoAccessType RVCALLCONV RvSipPAccessNetworkInfoHeaderGetAccessType(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_ACCESS_TYPE_UNDEFINED;
	}

    return ((MsgPAccessNetworkInfoHeader*)hHeader)->eAccessType;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetNetworkProvided
 * ------------------------------------------------------------------------
 * General: Returns value NetworkProvided parameter in the header.
 * Return Value: Returns NetworkProvided boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPAccessNetworkInfoHeaderGetNetworkProvided(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RV_FALSE;
	}

    return ((MsgPAccessNetworkInfoHeader*)hHeader)->bNetworkProvided;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetNetworkProvided
 * ------------------------------------------------------------------------
 * General:Sets the NetworkProvided parameter in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader          - Handle to the header object.
 *    bNetworkProvided - The NetworkProvided to be set in the PAccessNetworkInfo header. 
 *                       If RV_FALSE is supplied, the parameter is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetNetworkProvided(
                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
					 IN	   RvBool                               bNetworkProvided)
{
    MsgPAccessNetworkInfoHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;
	pHeader->bNetworkProvided = bNetworkProvided;

    return RV_OK;
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General:This method gets the Cgi3gpp in the PAccessNetworkInfo header object.
 * Return Value: Cgi3gpp offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrCgi3gpp(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strCgi3gpp;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header Cgi3gpp field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrCgi3gpp(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrCgi3gpp(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Cgi3gpp in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pCgi3gpp - The Cgi3gpp to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing Cgi3gpp string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrCgi3gpp(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pCgi3gpp,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPAccessNetworkInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pCgi3gpp,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCgi3gpp = newStr;
#ifdef SIP_DEBUG
    pHeader->pCgi3gpp = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strCgi3gpp);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General:Sets the Cgi3gpp field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strCgi3gpp - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrCgi3gpp(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pCgi3gpp)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrCgi3gpp(hHeader, pCgi3gpp, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General:This method gets the UtranCellId3gpp in the PAccessNetworkInfo header object.
 * Return Value: UtranCellId3gpp offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strUtranCellId3gpp;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header UtranCellId3gpp field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the UtranCellId3gpp in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pUtranCellId3gpp - The UtranCellId3gpp to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing UtranCellId3gpp string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pUtranCellId3gpp,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset)
{
    RvInt32          newStr;
    MsgPAccessNetworkInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pUtranCellId3gpp,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strUtranCellId3gpp = newStr;
#ifdef SIP_DEBUG
    pHeader->pUtranCellId3gpp = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strUtranCellId3gpp);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General:Sets the UtranCellId3gpp field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pUtranCellId3gpp)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(hHeader, pUtranCellId3gpp, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General:This method gets the IWlanNodeID in the PAccessNetworkInfo header object.
 * Return Value: IWlanNodeID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrIWlanNodeID(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strIWlanNodeID;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header IWlanNodeID field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrIWlanNodeID(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrIWlanNodeID(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IWlanNodeID in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pIWlanNodeID - The IWlanNodeID to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing IWlanNodeID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrIWlanNodeID(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pIWlanNodeID,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset)
{
    RvInt32          newStr;
    MsgPAccessNetworkInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pIWlanNodeID,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strIWlanNodeID = newStr;
#ifdef SIP_DEBUG
    pHeader->pIWlanNodeID = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strIWlanNodeID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General:Sets the IWlanNodeID field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strIWlanNodeID - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrIWlanNodeID(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pIWlanNodeID)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrIWlanNodeID(hHeader, pIWlanNodeID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrDslLocation
 * ------------------------------------------------------------------------
 * General:This method gets the DslLocation in the PAccessNetworkInfo header object.
 * Return Value: DslLocation offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrDslLocation(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strDslLocation;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrDslLocation
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header DslLocation field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrDslLocation(
                                       IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                       IN RvChar*								strBuffer,
                                       IN RvUint								bufferLen,
                                       OUT RvUint*								actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrDslLocation(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrDslLocation
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DslLocation in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pDslLocation - The DslLocation to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing DslLocation string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrDslLocation(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pDslLocation,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset)
{
    RvInt32							newStr;
    MsgPAccessNetworkInfoHeader*	pHeader;
    RvStatus						retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pDslLocation,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strDslLocation = newStr;
#ifdef SIP_DEBUG
    pHeader->pDslLocation = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strDslLocation);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrDslLocation
 * ------------------------------------------------------------------------
 * General:Sets the DslLocation field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strDslLocation - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrDslLocation(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pDslLocation)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrDslLocation(hHeader, pDslLocation, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General:This method gets the Ci3gpp2 in the PAccessNetworkInfo header object.
 * Return Value: Ci3gpp2 offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrCi3gpp2(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strCi3gpp2;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header Ci3gpp2 field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrCi3gpp2(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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
	
    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrCi3gpp2(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Ci3gpp2 in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pCi3gpp2 - The Ci3gpp2 to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing Ci3gpp2 string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrCi3gpp2(
                         IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                         IN    RvChar *								pCi3gpp2,
                         IN    HRPOOL								hPool,
                         IN    HPAGE								hPage,
                         IN    RvInt32								strOffset)
{
    RvInt32							newStr;
    MsgPAccessNetworkInfoHeader*	pHeader;
    RvStatus						retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pCi3gpp2,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCi3gpp2 = newStr;
#ifdef SIP_DEBUG
    pHeader->pCi3gpp2 = (RvChar *)RPOOL_GetPtr(	pHeader->hPool, pHeader->hPage,
												pHeader->strCi3gpp2);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General:Sets the Ci3gpp2 field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strCi3gpp2 - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrCi3gpp2(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pCi3gpp2)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrCi3gpp2(hHeader, pCi3gpp2, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PAccessNetworkInfo header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetOtherParams(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header other params field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetOtherParams(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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
	
    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pOtherParams - The Other Params to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetOtherParams(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPAccessNetworkInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAccessNetworkInfoHeader*) hHeader;

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
 * RvSipPAccessNetworkInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strOtherParams - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetOtherParams(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrBadSyntax(
                                    IN RvSipPAccessNetworkInfoHeaderHandle hHeader)
{
    return ((MsgPAccessNetworkInfoHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PAccessNetworkInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrBadSyntax(
                                               IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
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
	
    return MsgUtilsFillUserBuffer(((MsgPAccessNetworkInfoHeader*)hHeader)->hPool,
                                  ((MsgPAccessNetworkInfoHeader*)hHeader)->hPage,
                                  SipPAccessNetworkInfoHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrBadSyntax
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
RvStatus SipPAccessNetworkInfoHeaderSetStrBadSyntax(
                                  IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPAccessNetworkInfoHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPAccessNetworkInfoHeader*)hHeader;

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
 * RvSipPAccessNetworkInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PAccessNetworkInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrBadSyntax(
                                  IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAccessNetworkInfoHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PAccessNetworkInfoHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PAccessNetworkInfoHeaderClean( IN MsgPAccessNetworkInfoHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO;
    pHeader->strOtherParams		= UNDEFINED;
	pHeader->strAccessType		= UNDEFINED;
	pHeader->strCgi3gpp			= UNDEFINED;
	pHeader->strUtranCellId3gpp	= UNDEFINED;
	pHeader->strIWlanNodeID		= UNDEFINED;
	pHeader->strDslLocation		= UNDEFINED;
	pHeader->strCi3gpp2			= UNDEFINED;
    pHeader->bNetworkProvided   = RV_FALSE;

#ifdef SIP_DEBUG
    pHeader->pOtherParams		= NULL;
	pHeader->pAccessType		= NULL;
	pHeader->pCgi3gpp			= NULL;
	pHeader->pUtranCellId3gpp	= NULL;
	pHeader->pIWlanNodeID		= NULL;
	pHeader->pDslLocation		= NULL;
	pHeader->pCi3gpp2			= NULL;
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

