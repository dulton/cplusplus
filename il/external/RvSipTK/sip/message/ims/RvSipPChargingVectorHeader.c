/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPChargingVectorHeader.c									*
 *																					*
 * The file defines the methods of the PChargingVector header object:				*
 * construct, destruct, copy, encode, parse and the ability to access and			*
 * change it's parameters.															*
 *																					*
 *      Author           Date														*
 *     ------           ------------												*
 *      Mickey           Nov.2005													*
 ************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPChargingVectorHeader.h"
#include "_SipPChargingVectorHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include "RvSipCommonList.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PChargingVectorHeaderClean(IN MsgPChargingVectorHeader* pHeader,
										  IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingVectorHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingVector header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PChargingVector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPChargingVectorHeaderHandle* hHeader)
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

    stat = RvSipPChargingVectorHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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

    /* Attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_P_CHARGING_VECTOR,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPChargingVectorHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingVector Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PChargingVector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstruct(
                                   IN  RvSipMsgMgrHandle					hMsgMgr,
                                   IN  HRPOOL								hPool,
                                   IN  HPAGE								hPage,
                                   OUT RvSipPChargingVectorHeaderHandle*	hHeader)
{
    MsgPChargingVectorHeader*   pHeader;
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

    pHeader = (MsgPChargingVectorHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPChargingVectorHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingVectorHeaderConstruct - Failed to construct PChargingVector header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PChargingVectorHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPChargingVectorHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingVectorHeaderConstruct - PChargingVector header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PChargingVector header 
 *			object to a destination PChargingVector header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingVector header object.
 *    hSource      - Handle to the destination PChargingVector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderCopy(
                                         INOUT	RvSipPChargingVectorHeaderHandle hDestination,
                                         IN		RvSipPChargingVectorHeaderHandle hSource)
{											
	MsgPChargingVectorHeader*	source;
    MsgPChargingVectorHeader*	dest;
	RvStatus					stat;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPChargingVectorHeader*)hSource;
    dest   = (MsgPChargingVectorHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* strIcidValue */
	if(source->strIcidValue > UNDEFINED)
    {
        dest->strIcidValue = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
													  dest->hPool,
													  dest->hPage,
													  source->hPool,
													  source->hPage,
													  source->strIcidValue);
        if(dest->strIcidValue == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strIcidValue"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pIcidValue */
#ifdef SIP_DEBUG
        dest->pIcidValue = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strIcidValue);
#endif
    }
    else
    {
        dest->strIcidValue = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIcidValue = NULL;
#endif
    }
	
	/* strIcidGenAddr */
	if(source->strIcidGenAddr > UNDEFINED)
    {
        dest->strIcidGenAddr = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strIcidGenAddr);
        if(dest->strIcidGenAddr == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strIcidGenAddr"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pIcidGenAddr */
#ifdef SIP_DEBUG
        dest->pIcidGenAddr = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strIcidGenAddr);
#endif
    }
    else
    {
        dest->strIcidGenAddr = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIcidGenAddr = NULL;
#endif
    }
	
	/* strOrigIoi */
	if(source->strOrigIoi > UNDEFINED)
    {
        dest->strOrigIoi = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strOrigIoi);
        if(dest->strOrigIoi == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strOrigIoi"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pOrigIoi */
#ifdef SIP_DEBUG
        dest->pOrigIoi = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strOrigIoi);
#endif
    }
    else
    {
        dest->strOrigIoi = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOrigIoi = NULL;
#endif
    }

	/* strTermIoi */
	if(source->strTermIoi > UNDEFINED)
    {
        dest->strTermIoi = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strTermIoi);
        if(dest->strTermIoi == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strTermIoi"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pTermIoi */
#ifdef SIP_DEBUG
        dest->pTermIoi = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strTermIoi);
#endif
    }
    else
    {
        dest->strTermIoi = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTermIoi = NULL;
#endif
    }
	
	/* strGgsn */
	if(source->strGgsn > UNDEFINED)
    {
        dest->strGgsn = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strGgsn);
        if(dest->strGgsn == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strGgsn"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pGgsn */
#ifdef SIP_DEBUG
        dest->pGgsn = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strGgsn);
#endif
    }
    else
    {
        dest->strGgsn = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pGgsn = NULL;
#endif
    }

	/* strGprsAuthToken */
	if(source->strGprsAuthToken > UNDEFINED)
    {
        dest->strGprsAuthToken = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strGprsAuthToken);
        if(dest->strGprsAuthToken == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strGprsAuthToken"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pGprsAuthToken */
#ifdef SIP_DEBUG
        dest->pGprsAuthToken = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strGprsAuthToken);
#endif
    }
    else
    {
        dest->strGprsAuthToken = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pGprsAuthToken = NULL;
#endif
    }

	/* pdpInfoList */
    stat = RvSipPChargingVectorHeaderSetPdpInfoList(hDestination, source->pdpInfoList);
	if(RV_OK != stat)
	{
		return stat;
	}
	
	/* strBras */
	if(source->strBras > UNDEFINED)
    {
        dest->strBras = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strBras);
        if(dest->strBras == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strBras"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pBras */
#ifdef SIP_DEBUG
        dest->pBras = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strBras);
#endif
    }
    else
    {
        dest->strBras = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBras = NULL;
#endif
    }

	/* strXdslAuthToken */
	if(source->strXdslAuthToken > UNDEFINED)
    {
        dest->strXdslAuthToken = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strXdslAuthToken);
        if(dest->strXdslAuthToken == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strXdslAuthToken"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pXdslAuthToken */
#ifdef SIP_DEBUG
        dest->pXdslAuthToken = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strXdslAuthToken);
#endif
    }
    else
    {
        dest->strXdslAuthToken = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pXdslAuthToken = NULL;
#endif
    }
	
	/* dslBearerInfoList */
	stat = RvSipPChargingVectorHeaderSetDslBearerInfoList(hDestination, source->dslBearerInfoList);
	if(RV_OK != stat)
	{
		return stat;
	}

	/* bIWlanChargingInfo */
	dest->bIWlanChargingInfo = source->bIWlanChargingInfo;

	/* bPacketcableChargingInfo */
	dest->bPacketcableChargingInfo = source->bPacketcableChargingInfo;
	
	/* strBCid */
	if(source->strBCid > UNDEFINED)
    {
        dest->strBCid = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strBCid);
        if(dest->strBCid == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderCopy - Failed to copy strBCid"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pBCid */
#ifdef SIP_DEBUG
        dest->pBCid = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strBCid);
#endif
    }
    else
    {
        dest->strBCid = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBCid = NULL;
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
                      "RvSipPChargingVectorHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPChargingVectorHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPChargingVectorHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PChargingVector header object to a textual PChargingVector header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PChargingVector header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderEncode(
                                          IN    RvSipPChargingVectorHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPChargingVectorHeader* pHeader;

    pHeader = (MsgPChargingVectorHeader*)hHeader;

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
                "RvSipPChargingVectorHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PChargingVectorHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PChargingVectorHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PChargingVectorHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PChargingVector header (as string) on it.
 *          format: "P-Charging-Vector: "
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PChargingVector header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PChargingVectorHeaderEncode(
                                  IN    RvSipPChargingVectorHeaderHandle hHeader,
                                  IN    HRPOOL							hPool,
                                  IN    HPAGE							hPage,
                                  IN    RvBool							bInUrlHeaders,
                                  INOUT RvUint32*						length)
{
    MsgPChargingVectorHeader*	pHeader;
    RvStatus					stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PChargingVectorHeaderEncode - Encoding PChargingVector header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Charging-Vector" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Charging-Vector", length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PChargingVectorHeaderEncode - Failed to encoding PChargingVector header. stat is %d",
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
                "PChargingVectorHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

	/* IcidValue */
	if(pHeader->strIcidValue > UNDEFINED)
    {
		/* set "icid-value=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "icid-value", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}
		
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strIcidValue,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encoding PChargingVectorHeaderEncode header. stat is %d",
				stat));
			return stat;
		}
    }
    else
    {
        /* this is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PChargingVectorHeaderEncode: Failed. No IcidValue was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
	
    /* IcidGenAddr (insert ";" in the beginning) */
    if(pHeader->strIcidGenAddr > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "icid-generated-at=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "icid-generated-at", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the IcidGenAddr string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strIcidGenAddr,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode IcidGenAddr String. stat is %d",
				stat));
		}
    }

	/* OrigIoi (insert ";" in the beginning) */
    if(pHeader->strOrigIoi > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "orig-ioi=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "orig-ioi", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the OrigIoi string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOrigIoi,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode OrigIoi String. stat is %d",
				stat));
		}
    }

	/* TermIoi (insert ";" in the beginning) */
    if(pHeader->strTermIoi > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "term-ioi=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "term-ioi", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the TermIoi string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strTermIoi,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode TermIoi String. stat is %d",
				stat));
		}
    }
    
	/* gprs-charging-info (insert ";" in the beginning) */
    if(pHeader->strGgsn > UNDEFINED && pHeader->strGprsAuthToken > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "ggsn=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "ggsn", length);

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set the Ggsn string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strGgsn,
                                    length);
		
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "auth-token=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "auth-token", length);

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set the auth-token string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strGprsAuthToken,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode gprs-charging-info. stat is %d",
				stat));
		}

		if(pHeader->pdpInfoList != NULL)
		{
			RvBool bIsListEmpty;

			RvSipCommonListIsEmpty(pHeader->pdpInfoList, &bIsListEmpty);
			if(RV_FALSE == bIsListEmpty)
			{
				/* set ";" in the beginning */
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

				/* set "pdp-info=" */
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													"pdp-info", length);

				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetEqualChar(bInUrlHeaders), length);
				
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);

				stat = MsgUtilsEncodePChargingVectorInfoList(pHeader->pMsgMgr,
														hPool,
														hPage,
														pHeader->pdpInfoList,
														pHeader->hPool,
														pHeader->hPage,
														bInUrlHeaders,
														RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_PDP,
														length);
				if (stat != RV_OK)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"PChargingVectorHeaderEncode - Failed to encode gprs-charging-info. stat is %d",
					stat));
				}
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
																MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
			}
		}
    }

	/* xDSL-bearer-info (insert ";" in the beginning) */
    if(pHeader->strBras > UNDEFINED && pHeader->strXdslAuthToken > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "bras=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "bras", length);

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set the bras string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBras,
                                    length);
		
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "auth-token=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "auth-token", length);

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set the auth-token string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strXdslAuthToken,
                                    length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingVectorHeaderEncode - Failed to encode xdsl-bearer-info. stat is %d",
				stat));
		}

		if(pHeader->dslBearerInfoList != NULL)
		{
			RvBool bIsListEmpty;

			RvSipCommonListIsEmpty(pHeader->dslBearerInfoList, &bIsListEmpty);
			if(RV_FALSE == bIsListEmpty)
			{
				/* set ";" in the beginning */
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

				/* set "dsl-bearer-info=" */
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													"dsl-bearer-info", length);

				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetEqualChar(bInUrlHeaders), length);
				
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);

				stat = MsgUtilsEncodePChargingVectorInfoList(
												pHeader->pMsgMgr,
												hPool,
												hPage,
												pHeader->dslBearerInfoList,
												pHeader->hPool,
												pHeader->hPage,
												bInUrlHeaders,
												RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_DSL_BEARER,
												length);
				if (stat != RV_OK)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"PChargingVectorHeaderEncode - Failed to encode xdsl-charging-info. stat is %d",
					stat));
				}
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
													MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
			}
		}
    }
	
	/* i-wlan-charging-info */
    if(pHeader->bIWlanChargingInfo == RV_TRUE)
	{
		/* set ";" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											"pdg", length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"PChargingVectorHeaderEncode - Failed to encode i-wlan-charging-info. stat is %d",
			stat));
		}
	}

	/* packetcable-charging-info */
	if(pHeader->bPacketcableChargingInfo == RV_TRUE)
	{
		/* set ";" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											"packetcable-multimedia", length);
		if (stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"PChargingVectorHeaderEncode - Failed to encode packetcable-charging-info. stat is %d",
			stat));
		}

        /* BCid (insert ";" in the beginning) */
        if(pHeader->strBCid > UNDEFINED)
        {
            /* set ";" in the beginning */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		    if (stat != RV_OK)
		    {
			    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				    "PChargingVectorHeaderEncode - Failed to encode Semicolon character. stat is %d",
				    stat));
		    }

		    /* set "bcid=" */
		    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                "bcid", length);

		    
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders), length);

		    if (stat != RV_OK)
		    {
			    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				    "PChargingVectorHeaderEncode - Failed to encode Equal character. stat is %d",
				    stat));
		    }

            /* set the BCid string */
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                        hPool,
                                        hPage,
                                        pHeader->hPool,
                                        pHeader->hPage,
                                        pHeader->strBCid,
                                        length);
		    if (stat != RV_OK)
		    {
			    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				    "PChargingVectorHeaderEncode - Failed to encode BCid String. stat is %d",
				    stat));
		    }
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

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PChargingVectorHeaderEncode - Failed to encoding PChargingVector header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingVector header-for example,
 *          "PChargingVector:sip:172.20.5.3:5060"-into a PChargingVector header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PChargingVector header object.
 *    buffer    - Buffer containing a textual PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderParse(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPChargingVectorHeader*	pHeader = (MsgPChargingVectorHeader*)hHeader;
	RvStatus					rv;
    
	if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PChargingVectorHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_CHARGING_VECTOR,
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
 * RvSipPChargingVectorHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingVector header value into an PChargingVector header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPChargingVectorHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PChargingVector header object.
 *    buffer    - The buffer containing a textual PChargingVector header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderParseValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPChargingVectorHeader*    pHeader = (MsgPChargingVectorHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}


    PChargingVectorHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_CHARGING_VECTOR,
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
 * RvSipPChargingVectorHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PChargingVector header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderFix(
                                     IN RvSipPChargingVectorHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPChargingVectorHeader* pHeader = (MsgPChargingVectorHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPChargingVectorHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPChargingVectorHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPChargingVectorHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPChargingVectorHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PChargingVector object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPChargingVectorHeaderGetPool(RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPChargingVectorHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PChargingVector object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPChargingVectorHeaderGetPage(RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingVector header fields are kept in a string format-for example, the
 *          PChargingVector header VNetworkSpec name. In order to get such a field from the PChargingVector header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PChargingVector header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingVectorHeaderGetStringLength(
                                      IN  RvSipPChargingVectorHeaderHandle     hHeader,
                                      IN  RvSipPChargingVectorHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPChargingVectorHeader* pHeader = (MsgPChargingVectorHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_CHARGING_VECTOR_ICID_VALUE:
        {
            stringOffset = SipPChargingVectorHeaderGetStrIcidValue(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_ICID_GEN_ADDR:
        {
            stringOffset = SipPChargingVectorHeaderGetStrIcidGenAddr(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_ORIG_IOI:
        {
            stringOffset = SipPChargingVectorHeaderGetStrOrigIoi(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_TERM_IOI:
        {
            stringOffset = SipPChargingVectorHeaderGetStrTermIoi(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_GGSN:
        {
            stringOffset = SipPChargingVectorHeaderGetStrGgsn(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_GPRS_AUTH_TOKEN:
        {
            stringOffset = SipPChargingVectorHeaderGetStrGprsAuthToken(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_BRAS:
        {
            stringOffset = SipPChargingVectorHeaderGetStrBras(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_XDSL_AUTH_TOKEN:
        {
            stringOffset = SipPChargingVectorHeaderGetStrXdslAuthToken(hHeader);
            break;
        }
        case RVSIP_P_CHARGING_VECTOR_BCID:
        {
            stringOffset = SipPChargingVectorHeaderGetStrBCid(hHeader);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_OTHER_PARAMS:
        {
            stringOffset = SipPChargingVectorHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_CHARGING_VECTOR_BAD_SYNTAX:
        {
            stringOffset = SipPChargingVectorHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPChargingVectorHeaderGetStrIcidValue
 * ------------------------------------------------------------------------
 * General:This method gets the IcidValue in the PChargingVector header object.
 * Return Value: IcidValue offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrIcidValue(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strIcidValue;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrIcidValue
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header IcidValue field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrIcidValue(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrIcidValue(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrIcidValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IcidValue in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pIcidValue - The IcidValue to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrIcidValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidValue,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pIcidValue,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
	
    pHeader->strIcidValue = newStr;
#ifdef SIP_DEBUG
    pHeader->pIcidValue = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strIcidValue);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrIcidValue
 * ------------------------------------------------------------------------
 * General:Sets the IcidValue field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strIcidValue - The IcidValue field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing IcidValue field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrIcidValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidValue)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrIcidValue(hHeader, pIcidValue, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General:This method gets the IcidGenAddr in the PChargingVector header object.
 * Return Value: IcidGenAddr offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrIcidGenAddr(
                                            IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strIcidGenAddr;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header IcidGenAddr field of the PChargingVector
			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrIcidGenAddr(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrIcidGenAddr(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IcidGenAddr in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pIcidGenAddr - The IcidGenAddr to be set in the PChargingVector header.
 *                          If NULL, the existing IcidGenAddr string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrIcidGenAddr(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidGenAddr,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pIcidGenAddr,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strIcidGenAddr = newStr;
#ifdef SIP_DEBUG
    pHeader->pIcidGenAddr = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strIcidGenAddr);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General:Sets the IcidGenAddr field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strUtranCellId3gpp - The IcidGenAddr field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing IcidGenAddr field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrIcidGenAddr(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidGenAddr)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrIcidGenAddr(hHeader, pIcidGenAddr, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrOrigIoi
 * ------------------------------------------------------------------------
 * General:This method gets the OrigIoi in the PChargingVector header object.
 * Return Value: OrigIoi offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrOrigIoi(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strOrigIoi;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrOrigIoi
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header OrigIoi field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrOrigIoi(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrOrigIoi(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrOrigIoi
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OrigIoi in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pOrigIoi - The OrigIoi to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrOrigIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOrigIoi,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOrigIoi,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strOrigIoi = newStr;
#ifdef SIP_DEBUG
    pHeader->pOrigIoi = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strOrigIoi);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrOrigIoi
 * ------------------------------------------------------------------------
 * General:Sets the OrigIoi field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strIcidValue - The OrigIoi field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing OrigIoi field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrOrigIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOrigIoi)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrOrigIoi(hHeader, pOrigIoi, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrTermIoi
 * ------------------------------------------------------------------------
 * General:This method gets the TermIoi in the PChargingVector header object.
 * Return Value: IcidValue offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrTermIoi(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strTermIoi;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrTermIoi
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header TermIoi field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrTermIoi(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrTermIoi(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrTermIoi
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the TermIoi in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pTermIoi - The TermIoi to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrTermIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pTermIoi,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pTermIoi,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strTermIoi = newStr;
#ifdef SIP_DEBUG
    pHeader->pTermIoi = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strTermIoi);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrTermIoi
 * ------------------------------------------------------------------------
 * General:Sets the TermIoi field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strTermIoi - The TermIoi field to be set in the PChargingVector header. If NULL is
 *                    supplied, the TermIoi parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrTermIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pTermIoi)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrTermIoi(hHeader, pTermIoi, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrGgsn
 * ------------------------------------------------------------------------
 * General:This method gets the Ggsn in the PChargingVector header object.
 * Return Value: Ggsn offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrGgsn(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strGgsn;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrGgsn
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header Ggsn field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrGgsn(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrGgsn(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrGgsn
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Ggsn in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pGgsn - The Ggsn to be set in the PChargingVector header. If NULL,
 *                           the existing Ggsn string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrGgsn(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pGgsn,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32						newStr;
    MsgPChargingVectorHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pGgsn,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strGgsn = newStr;
#ifdef SIP_DEBUG
    pHeader->pGgsn = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strGgsn);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrGgsn
 * ------------------------------------------------------------------------
 * General:Sets the Ggsn field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGgsn	- The Ggsn field to be set in the PChargingVector header. If NULL is
 *					supplied, the existing Ggsn field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrGgsn(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGgsn)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrGgsn(hHeader, pGgsn, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General:This method gets the GprsAuthToken in the PChargingVector header object.
 * Return Value: GprsAuthToken offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrGprsAuthToken(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strGprsAuthToken;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header GprsAuthToken field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrGprsAuthToken(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrGprsAuthToken(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the GprsAuthToken in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pGprsAuthToken - The GprsAuthToken to be set in the PChargingVector header. If NULL,
 *                           the existing GprsAuthToken string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrGprsAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGprsAuthToken,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset)
{
    RvInt32						newStr;
    MsgPChargingVectorHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pGprsAuthToken,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strGprsAuthToken = newStr;
#ifdef SIP_DEBUG
    pHeader->pGprsAuthToken = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strGprsAuthToken);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General:Sets the GprsAuthToken field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGprsAuthToken - The GprsAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the GprsAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrGprsAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGprsAuthToken)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrGprsAuthToken(hHeader, pGprsAuthToken, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrBras
 * ------------------------------------------------------------------------
 * General:This method gets the Bras in the PChargingVector header object.
 * Return Value: Bras offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBras(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strBras;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBras
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header Bras field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBras(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrBras(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBras
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Bras in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pBras - The Bras to be set in the PChargingVector header. If NULL,
 *				    the existing Bras string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrBras(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pBras,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset)
{
    RvInt32						newStr;
    MsgPChargingVectorHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pBras,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBras = newStr;
#ifdef SIP_DEBUG
    pHeader->pBras = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strBras);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrBras
 * ------------------------------------------------------------------------
 * General:Sets the Bras field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGprsAuthToken - The GprsAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the GprsAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBras(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pBras)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrBras(hHeader, pBras, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General:This method gets the XdslAuthToken in the PChargingVector header object.
 * Return Value: XdslAuthToken offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrXdslAuthToken(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strXdslAuthToken;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header XdslAuthToken field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrXdslAuthToken(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrXdslAuthToken(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Bras in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pXdslAuthToken - The Bras to be set in the PChargingVector header. If NULL,
 *				    the existing Bras string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrXdslAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pXdslAuthToken,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset)
{
    RvInt32						newStr;
    MsgPChargingVectorHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pXdslAuthToken,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strXdslAuthToken = newStr;
#ifdef SIP_DEBUG
    pHeader->pXdslAuthToken = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
													pHeader->strXdslAuthToken);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General:Sets the XdslAuthToken field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strXdslAuthToken - The XdslAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the XdslAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrXdslAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pXdslAuthToken)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrXdslAuthToken(hHeader, pXdslAuthToken, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetIWlanChargingInfo
 * ------------------------------------------------------------------------
 * General: Returns value IWlanChargingInfo parameter in the header.
 * Return Value: Returns IWlanChargingInfo boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorHeaderGetIWlanChargingInfo(
                                               IN RvSipPChargingVectorHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RV_FALSE;
	}

    return ((MsgPChargingVectorHeader*)hHeader)->bIWlanChargingInfo;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetIWlanChargingInfo
 * ------------------------------------------------------------------------
 * General:Sets the IWlanChargingInfo parameter in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    bIWlanChargingInfo - The IWlanChargingInfo to be set in the PChargingVector header. If RV_FALSE is supplied, the
 *                 parameter ("pdg") is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetIWlanChargingInfo(
                     IN    RvSipPChargingVectorHeaderHandle	hHeader,
					 IN	   RvBool							bIWlanChargingInfoType)
{
    MsgPChargingVectorHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

	pHeader->bIWlanChargingInfo = bIWlanChargingInfoType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetPacketcableChargingInfo
 * ------------------------------------------------------------------------
 * General: Returns value PacketcableChargingInfo parameter in the header.
 * Return Value: Returns PacketcableChargingInfo boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorHeaderGetPacketcableChargingInfo(
										IN RvSipPChargingVectorHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RV_FALSE;
	}

    return ((MsgPChargingVectorHeader*)hHeader)->bPacketcableChargingInfo;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetPacketcableChargingInfo
 * ------------------------------------------------------------------------
 * General:Sets the PacketcableChargingInfo parameter in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader                  - Handle to the header object.
 *    bPacketcableChargingInfo - The PacketcableChargingInfo to be set in the PChargingVector header. If RV_FALSE is supplied, the
 *                 parameter ("packetcable-multimedia") is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetPacketcableChargingInfo(
                     IN    RvSipPChargingVectorHeaderHandle	hHeader,
					 IN	   RvBool							bPacketcableChargingInfoType)
{
    MsgPChargingVectorHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

	pHeader->bPacketcableChargingInfo = bPacketcableChargingInfoType;
	
    return RV_OK;
}

/***************************************************************************
 * SipPChargingVectorHeaderGetStrBCid
 * ------------------------------------------------------------------------
 * General:This method gets the BCid in the PChargingVector header object.
 * Return Value: BCid offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBCid(IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strBCid;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBCid
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header BCid field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBCid(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrBCid(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBCid
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BCid in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pBCid - The BCid to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrBCid(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                         pBCid,
                                     IN    HRPOOL                           hPool,
                                     IN    HPAGE                            hPage,
                                     IN    RvInt32                          strOffset)
{
    RvInt32                   newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus                  retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pBCid,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBCid = newStr;
#ifdef SIP_DEBUG
    pHeader->pBCid = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strBCid);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrBCid
 * ------------------------------------------------------------------------
 * General:Sets the BCid field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    pBCid		- The BCid field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing BCid field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBCid(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pBCid)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrBCid(hHeader, pBCid, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPChargingVectorHeaderConstructPdpInfoList
 * ------------------------------------------------------------------------
 * General: Returns the PdpInfoList of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructPdpInfoList(
                                       IN	RvSipPChargingVectorHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*				hList)
{
	MsgPChargingVectorHeader*	pHeader;

	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgPChargingVectorHeader*) hHeader;
	
	RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, hList);
	
	if(hList == NULL)
	{
		return RV_ERROR_OUTOFRESOURCES;
	}
	
	pHeader->pdpInfoList = *hList;
    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetPdpInfoList
 * ------------------------------------------------------------------------
 * General: Returns the PdpInfoList of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingVectorHeaderGetPdpInfoList(
                                               IN RvSipPChargingVectorHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPChargingVectorHeader*)hHeader)->pdpInfoList;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetPdpInfoList
 * ------------------------------------------------------------------------
 * General: Sets the PdpInfoList in the PChargingVector header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingVectorHeaderConstructPdpInfoList(),
 *			and than RvSipPChargingVectorInfoListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PChargingVector header object.
 *    hList - Handle to the PdpInfoList object. If NULL is supplied, the existing
 *              PdpInfoList is removed from the PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetPdpInfoList(
                                    IN    RvSipPChargingVectorHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle				hList)
{
    RvStatus					stat;
    MsgPChargingVectorHeader   *pHeader	= (MsgPChargingVectorHeader*)hHeader;
	RvInt						safeCounter = 0;
	RvInt						maxLoops    = 10000;
	RvInt						elementType;
	void*						element;
	RvSipCommonListElemHandle	hRelative = NULL;
	RvSipPChargingVectorInfoListElemHandle	hPdpInfoListElement = NULL;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hList == NULL)
    {
        pHeader->pdpInfoList = NULL;
        return RV_OK;
    }
    else
    {
		RvSipCommonListElemHandle  phNewPos;

		/* Copy the list */
		stat = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
									&elementType, &element, &hRelative);
		
		if(element == NULL)
		{
			pHeader->pdpInfoList = NULL;
		}
		else
		{
			/* Construct a new list */
			RvSipCommonListHandle	hNewList;
			RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, &hNewList);
			
			if(hNewList == NULL)
			{
				return RV_ERROR_OUTOFRESOURCES;
			}

			pHeader->pdpInfoList = hNewList;

			while ((RV_OK == stat) && (NULL != element))
			{
				stat = RvSipPChargingVectorInfoListElementConstructInHeader(hHeader, &hPdpInfoListElement);
				stat = RvSipPChargingVectorInfoListElementCopy(hPdpInfoListElement,
																(RvSipPChargingVectorInfoListElemHandle)element);
				stat = RvSipCommonListPushElem(pHeader->pdpInfoList, RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_TYPE_INFO,
												(void*)hPdpInfoListElement, RVSIP_LAST_ELEMENT, NULL, &phNewPos);
				
				if (RV_OK != stat)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingVectorHeaderSetPdpInfoList - Failed to copy pdpInfoList. stat = %d",
							  stat));
					return stat;
				}
				
				stat = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
												&elementType, &element, &hRelative);

				safeCounter++;
				if (safeCounter > maxLoops)
				{
					RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingVectorHeaderSetPdpInfoList - Execption in loop. Too many rounds."));
					return RV_ERROR_UNKNOWN;
				}
			}
		}
    }

	return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderConstructDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Constructs a RvSipCommonList object on the header's page, into 
 *			the given handle. Also placing the list in the DslBearerInfoList parameter
 *			of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader	- Handle to the header object.
 * output: hList	- Handle to the constructed list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructDslBearerInfoList(
                                       IN	RvSipPChargingVectorHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*				hList)
{
	MsgPChargingVectorHeader*	pHeader;

	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgPChargingVectorHeader*) hHeader;
	
	RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, hList);
	
	if(hList == NULL)
	{
		return RV_ERROR_OUTOFRESOURCES;
	}
	
	pHeader->dslBearerInfoList = *hList;
    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Returns the dslBearerInfoList of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingVectorHeaderGetDslBearerInfoList(
                                               IN RvSipPChargingVectorHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPChargingVectorHeader*)hHeader)->dslBearerInfoList;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderSetDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Sets the DslBearerInfoList in the PChargingVector header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingVectorHeaderConstructDslBearerInfoList(),
 *			and than RvSipPChargingVectorInfoListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PChargingVector header object.
 *    hList	  - Handle to the DslBearerInfoList object. If NULL is supplied, the existing
 *              DslBearerInfoList is removed from the PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetDslBearerInfoList(
                                    IN    RvSipPChargingVectorHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle				hList)
{
    RvStatus					stat;
    MsgPChargingVectorHeader   *pHeader	= (MsgPChargingVectorHeader*)hHeader;
	RvInt						safeCounter = 0;
	RvInt						maxLoops    = 10000;
	RvInt						elementType;
	void*						element;
	RvSipCommonListElemHandle	hRelative = NULL;
	RvSipPChargingVectorInfoListElemHandle	hDslBearerInfoListElement = NULL;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hList == NULL)
    {
        pHeader->dslBearerInfoList = NULL;
        return RV_OK;
    }
    else
    {
		RvSipCommonListElemHandle  phNewPos;
		
		/* Copy the list */
		stat = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
									&elementType, &element, &hRelative);
		
        if(element == NULL)
		{
			pHeader->dslBearerInfoList = NULL;
		}
		else
		{
			/* Construct a new list */
			RvSipCommonListHandle	hNewList;
			RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, &hNewList);
			
			if(hNewList == NULL)
			{
				return RV_ERROR_OUTOFRESOURCES;
			}
			
			pHeader->dslBearerInfoList = hNewList;
			
			while ((RV_OK == stat) && (NULL != element))
			{
				stat = RvSipPChargingVectorInfoListElementConstructInHeader(hHeader, &hDslBearerInfoListElement);
				stat = RvSipPChargingVectorInfoListElementCopy(hDslBearerInfoListElement,
																(RvSipPChargingVectorInfoListElemHandle)element);
				stat = RvSipCommonListPushElem(pHeader->dslBearerInfoList, RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_TYPE_INFO,
												(void*)hDslBearerInfoListElement, RVSIP_LAST_ELEMENT, NULL, &phNewPos);
				
				if (RV_OK != stat)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingVectorHeaderSetDslBearerInfoList - Failed to copy dslBearerInfoListElement. stat = %d",
							  stat));
					return stat;
				}
				
				stat = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
												&elementType, &element, &hRelative);

				safeCounter++;
				if (safeCounter > maxLoops)
				{
					RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingVectorHeaderSetDslBearerInfoList - Execption in loop. Too many rounds."));
					return RV_ERROR_UNKNOWN;
				}
			}
		}
    }

	return RV_OK;
}

/***************************************************************************
 * SipPChargingVectorHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PChargingVector header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetOtherParams(
                                            IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header other params field of the PChargingVector header object into a
 *          given buffer.
 *          Not all the PChargingVector header parameters have separated fields in the PChargingVector
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetOtherParams(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipPChargingVectorHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pOtherParams - The Other Params to be set in the PChargingVector header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetOtherParams(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingVectorHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingVectorHeader*) hHeader;

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
 * RvSipPChargingVectorHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PChargingVector header object.
 *         Not all the PChargingVector header parameters have separated fields in the PChargingVector
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strOtherParams - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetOtherParams(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPChargingVectorHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBadSyntax(
                                    IN RvSipPChargingVectorHeaderHandle hHeader)
{
    return ((MsgPChargingVectorHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PChargingVector header,
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
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBadSyntax(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorHeader*)hHeader)->hPool,
                                  ((MsgPChargingVectorHeader*)hHeader)->hPage,
                                  SipPChargingVectorHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBadSyntax
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
RvStatus SipPChargingVectorHeaderSetStrBadSyntax(
                                  IN RvSipPChargingVectorHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPChargingVectorHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPChargingVectorHeader*)hHeader;

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
 * RvSipPChargingVectorHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PChargingVector header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBadSyntax(
                                  IN RvSipPChargingVectorHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        LIST FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingVectorInfoListElementConstructInHeader
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingVectorInfoListElement object inside a given Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader			- Handle to the header object.
 * output: hElement			- Handle to the newly constructed PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementConstructInHeader(
                               IN	RvSipPChargingVectorHeaderHandle		hHeader,
                               OUT	RvSipPChargingVectorInfoListElemHandle*	hElement)
{
	MsgPChargingVectorHeader*   header;
	RvStatus					stat;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hElement == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hElement = NULL;

    header = (MsgPChargingVectorHeader*)hHeader;

    stat = RvSipPChargingVectorInfoListElementConstruct((RvSipMsgMgrHandle)header->pMsgMgr, 
														header->hPool,
														header->hPage,
														hElement);
	return stat;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingVectorInfoListElement object. The element is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          element object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hElement - Handle to the newly constructed PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementConstruct(
							   IN  RvSipMsgMgrHandle						hMsgMgr,
							   IN  HRPOOL									hPool,
							   IN  HPAGE									hPage,
							   OUT RvSipPChargingVectorInfoListElemHandle*	hElement)
{
	MsgPChargingVectorInfoListElem*	pElement;
    MsgMgr*							pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hElement == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hElement = NULL;

    pElement = (MsgPChargingVectorInfoListElem*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                   hPool,
                                                   hPage,
                                                   sizeof(MsgPChargingVectorInfoListElem),
                                                   RV_TRUE);
    if(pElement == NULL)
    {
        *hElement = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingVectorInfoListElementConstruct - Failed to construct Element. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

	pElement->pMsgMgr	= pMsgMgr;
    pElement->hPage		= hPage;
    pElement->hPool		= hPool;
	
    pElement->bSig		= RV_FALSE;
	pElement->nItem		= UNDEFINED;
	pElement->strCid	= UNDEFINED;
	pElement->strFlowID	= UNDEFINED;
	
#ifdef SIP_DEBUG
	pElement->pCid		= NULL;
	pElement->pFlowID	= NULL;
#endif
	
   *hElement = (RvSipPChargingVectorInfoListElemHandle)pElement;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingVectorInfoListElementConstruct - PChargingVectorInfoListElem object was constructed successfully. (hPool=0x%p, hPage=0x%p, hElement=0x%p)",
            hPool, hPage, *hElement));

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source PChargingVectorInfoListElement object to a 
 *		   destination PChargingVectorInfoListElement object.
 *         You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingVectorInfoListElement object.
 *    hSource      - Handle to the source PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementCopy(
								INOUT	RvSipPChargingVectorInfoListElemHandle	hDestination,
								IN		RvSipPChargingVectorInfoListElemHandle	hSource)
{
    MsgPChargingVectorInfoListElem* dest;
    MsgPChargingVectorInfoListElem* source;

    if((hDestination == NULL)||(hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    dest	= (MsgPChargingVectorInfoListElem*)hDestination;
    source	= (MsgPChargingVectorInfoListElem*)hSource;

	/* copy item */
	dest->nItem		= source->nItem;

	/* copy sig */
	dest->bSig		= source->bSig;

	/* copy cid */
    if(source->strCid > UNDEFINED)
    {
        dest->strCid = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strCid);
        if(dest->strCid == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipPChargingVectorInfoListElementCopy - Failed to copy strCid"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCid = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strCid);
#endif
    }
    else
    {
        dest->strCid = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCid= NULL;
#endif
    }

	/* copy flow-id */
    if(source->strFlowID > UNDEFINED)
    {
        dest->strFlowID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
														dest->hPool,
														dest->hPage,
														source->hPool,
														source->hPage,
														source->strFlowID);
        if(dest->strFlowID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipPChargingVectorInfoListElementCopy - Failed to copy strFlowID"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pFlowID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strFlowID);
#endif
    }
    else
    {
        dest->strFlowID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pFlowID= NULL;
#endif
    }

	return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingVectorInfoListElement fields are kept in a string
 *			format-for example, the cid parameter. In order to get such a field from
 *			the element object, your application should supply an adequate buffer to 
 *			where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *  stringName	- Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingVectorInfoListElementGetStringLength(
                              IN  RvSipPChargingVectorInfoListElemHandle		hElement,
                              IN  RvSipPChargingVectorInfoListElementStringName stringName)
{
    MsgPChargingVectorInfoListElem*	pElement = (MsgPChargingVectorInfoListElem*)hElement;
	RvInt32							stringOffset;

	if(hElement == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_CID:
        {
            stringOffset = SipPChargingVectorInfoListElementGetStrCid(hElement);
            break;
        }
		case RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_FLOW_ID:
        {
            stringOffset = SipPChargingVectorInfoListElementGetStrFlowID(hElement);
            break;
        }
        default:
        {
            RvLogExcep(pElement->pMsgMgr->pLogSrc,(pElement->pMsgMgr->pLogSrc,
                "RvSipPChargingVectorInfoListElementGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
	
    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pElement->hPool, pElement->hPage, stringOffset)+1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * SipPChargingVectorInfoListElementGetStrCid
 * ------------------------------------------------------------------------
 * General:This method gets the cid in the PChargingVectorInfoListElement object.
 * Return Value: cid offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingVectorInfoListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorInfoListElementGetStrCid(IN RvSipPChargingVectorInfoListElemHandle hElement)
{
    return ((MsgPChargingVectorInfoListElem*)hElement)->strCid;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStrCid
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVectorInfoListElement cid field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingVectorInfoListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementGetStrCid(
                               IN  RvSipPChargingVectorInfoListElemHandle	hElement,
                               IN  RvChar*									strBuffer,
                               IN  RvUint									bufferLen,
                               OUT RvUint*									actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorInfoListElem*)hElement)->hPool,
                                  ((MsgPChargingVectorInfoListElem*)hElement)->hPage,
                                  SipPChargingVectorInfoListElementGetStrCid(hElement),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorInfoListElementSetStrCid
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Cid in the
 *          PChargingVectorInfoListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hElement - Handle of the PChargingVectorInfoListElement object.
 *            pCid - The Cid to be set in the PChargingVectorInfoListElement.
 *                          If NULL, the existing Cid string in the element will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorInfoListElementSetStrCid(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pCid,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset)
{
    RvInt32							newStr;
    MsgPChargingVectorInfoListElem*	pElement;
    RvStatus						retStatus;

    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pElement = (MsgPChargingVectorInfoListElem*) hElement;

    retStatus = MsgUtilsSetString(pElement->pMsgMgr, hPool, hPage, strOffset, pCid,
                                  pElement->hPool, pElement->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pElement->strCid = newStr;
#ifdef SIP_DEBUG
    pElement->pCid = (RvChar *)RPOOL_GetPtr(pElement->hPool, pElement->hPage,
                                           pElement->strCid);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetStrCid
 * ------------------------------------------------------------------------
 * General:Sets the Cid field in the PChargingVectorInfoListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    pCid		- The cid to be set in the PChargingVectorInfoListElement. If NULL is
 *				  supplied, the existing cid field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetStrCid(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pCid)
{
    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorInfoListElementSetStrCid(hElement, pCid, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPChargingVectorInfoListElementGetStrFlowID
 * ------------------------------------------------------------------------
 * General:This method gets the FlowID in the PChargingVectorInfoListElement object.
 * Return Value: FlowID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingVectorInfoListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorInfoListElementGetStrFlowID(IN RvSipPChargingVectorInfoListElemHandle hElement)
{
    return ((MsgPChargingVectorInfoListElem*)hElement)->strFlowID;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStrFlowID
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVectorInfoListElement FlowID field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingVectorInfoListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementGetStrFlowID(
                               IN  RvSipPChargingVectorInfoListElemHandle	hElement,
                               IN  RvChar*									strBuffer,
                               IN  RvUint									bufferLen,
                               OUT RvUint*									actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgPChargingVectorInfoListElem*)hElement)->hPool,
                                  ((MsgPChargingVectorInfoListElem*)hElement)->hPage,
                                  SipPChargingVectorInfoListElementGetStrFlowID(hElement),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingVectorInfoListElementSetStrFlowID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FlowID in the
 *          PChargingVectorInfoListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hElement - Handle of the PChargingVectorInfoListElement object.
 *			pFlowID	 - The FlowID to be set in the PChargingVectorInfoListElement.
 *						If NULL, the existing FlowID string in the element will
 *						be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorInfoListElementSetStrFlowID(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pFlowID,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset)
{
    RvInt32							newStr;
    MsgPChargingVectorInfoListElem*	pElement;
    RvStatus						retStatus;

    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pElement = (MsgPChargingVectorInfoListElem*) hElement;

    retStatus = MsgUtilsSetString(pElement->pMsgMgr, hPool, hPage, strOffset, pFlowID,
                                  pElement->hPool, pElement->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pElement->strFlowID = newStr;
#ifdef SIP_DEBUG
    pElement->pFlowID = (RvChar *)RPOOL_GetPtr(pElement->hPool, pElement->hPage,
												pElement->strFlowID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetStrFlowID
 * ------------------------------------------------------------------------
 * General:Sets the FlowID field in the PChargingVectorInfoListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    pFlowID	- The FlowID to be set in the PChargingVectorInfoListElement. If NULL is
 *				  supplied, the existing FlowID field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetStrFlowID(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pFlowID)
{
    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingVectorInfoListElementSetStrFlowID(hElement, pFlowID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetItem
 * ------------------------------------------------------------------------
 * General: Gets Item parameter from the PChargingVectorInfoListElement object.
 * Return Value: Returns the Item number, or UNDEFINED if the Item number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle to the PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPChargingVectorInfoListElementGetItem(
                                         IN RvSipPChargingVectorInfoListElemHandle hElement)
{
    if(hElement == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgPChargingVectorInfoListElem*)hElement)->nItem;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetItem
 * ------------------------------------------------------------------------
 * General:  Sets Item parameter of the PChargingVectorInfoListElement object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    item		- The item number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetItem(
                                 IN    RvSipPChargingVectorInfoListElemHandle hElement,
                                 IN    RvInt32								  item)
{
    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgPChargingVectorInfoListElem*)hElement)->nItem = item;
    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetSig
 * ------------------------------------------------------------------------
 * General: Gets Sig parameter from the PChargingVectorInfoListElement object.
 * Return Value: Returns the Sig bool.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle to the PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorInfoListElementGetSig(
                                         IN RvSipPChargingVectorInfoListElemHandle hElement)
{
    if(hElement == NULL)
	{
        return RV_FALSE;
	}

    return ((MsgPChargingVectorInfoListElem*)hElement)->bSig;
}

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetSig
 * ------------------------------------------------------------------------
 * General:  Sets Sig parameter of the PChargingVectorInfoListElement object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    sig		- The item number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetSig(
                                 IN    RvSipPChargingVectorInfoListElemHandle hElement,
                                 IN    RvBool								  sig)
{
    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgPChargingVectorInfoListElem*)hElement)->bSig = sig;
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PChargingVectorHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PChargingVectorHeaderClean( IN MsgPChargingVectorHeader*	pHeader,
										IN RvBool						bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_CHARGING_VECTOR;
    pHeader->strOtherParams		= UNDEFINED;
	pHeader->strIcidValue		= UNDEFINED;
	pHeader->strIcidGenAddr		= UNDEFINED;
	pHeader->strOrigIoi			= UNDEFINED;
	pHeader->strTermIoi			= UNDEFINED;
	
	pHeader->strGgsn			= UNDEFINED;
	pHeader->strGprsAuthToken	= UNDEFINED;
	pHeader->pdpInfoList		= NULL;
	
	pHeader->strBras			= UNDEFINED;
	pHeader->strXdslAuthToken	= UNDEFINED;
	pHeader->dslBearerInfoList	= NULL;
	
	pHeader->bIWlanChargingInfo	= RV_FALSE;
	
	pHeader->bPacketcableChargingInfo	= RV_FALSE;
	pHeader->strBCid			= UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pOtherParams		= NULL;
	pHeader->pIcidValue			= NULL;
	pHeader->pIcidGenAddr		= NULL;
	pHeader->pOrigIoi			= NULL;
	pHeader->pTermIoi			= NULL;
	pHeader->pGgsn				= NULL;
	pHeader->pGprsAuthToken		= NULL;
	pHeader->pBras				= NULL;
	pHeader->pXdslAuthToken		= NULL;
	pHeader->pBCid				= NULL;
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

