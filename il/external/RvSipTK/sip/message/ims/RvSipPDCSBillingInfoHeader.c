/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPDCSBillingInfoHeader.c									*
 *																					*
 * The file defines the methods of the PDCSBillingInfo header object:				*
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

#include "RvSipPDCSBillingInfoHeader.h"
#include "_SipPDCSBillingInfoHeader.h"
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
static void PDCSBillingInfoHeaderClean(IN MsgPDCSBillingInfoHeader* pHeader,
										  IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSBillingInfo header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSBillingInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderConstructInMsg(
                                   IN  RvSipMsgHandle					 hSipMsg,
                                   IN  RvBool							 pushHeaderAtHead,
                                   OUT RvSipPDCSBillingInfoHeaderHandle* hHeader)
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

    stat = RvSipPDCSBillingInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_DCS_BILLING_INFO,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSBillingInfo Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSBillingInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderConstruct(
                               IN  RvSipMsgMgrHandle				 hMsgMgr,
                               IN  HRPOOL							 hPool,
                               IN  HPAGE							 hPage,
                               OUT RvSipPDCSBillingInfoHeaderHandle* hHeader)
{
    MsgPDCSBillingInfoHeader*   pHeader;
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

    pHeader = (MsgPDCSBillingInfoHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPDCSBillingInfoHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderConstruct - Failed to construct PDCSBillingInfo header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PDCSBillingInfoHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPDCSBillingInfoHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderConstruct - PDCSBillingInfo header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSBillingInfo header 
 *			object to a destination PDCSBillingInfo header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSBillingInfo header object.
 *    hSource      - Handle to the destination PDCSBillingInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderCopy(
                             INOUT RvSipPDCSBillingInfoHeaderHandle hDestination,
                             IN    RvSipPDCSBillingInfoHeaderHandle hSource)
{
    MsgPDCSBillingInfoHeader*   source;
    MsgPDCSBillingInfoHeader*   dest;
	RvStatus					stat;
	
    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPDCSBillingInfoHeader*)hSource;
    dest   = (MsgPDCSBillingInfoHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* strBillingCorrelationID */
	if(source->strBillingCorrelationID > UNDEFINED)
    {
        dest->strBillingCorrelationID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
													  dest->hPool,
													  dest->hPage,
													  source->hPool,
													  source->hPage,
													  source->strBillingCorrelationID);
        if(dest->strBillingCorrelationID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderCopy - Failed to copy strBillingCorrelationID"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pBillingCorrelationID */
#ifdef SIP_DEBUG
        dest->pBillingCorrelationID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strBillingCorrelationID);
#endif
    }
    else
    {
        dest->strBillingCorrelationID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBillingCorrelationID = NULL;
#endif
    }
	
	/* strFEID */
	if(source->strFEID > UNDEFINED)
    {
        dest->strFEID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strFEID);
        if(dest->strFEID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderCopy - Failed to copy strFEID"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pFEID */
#ifdef SIP_DEBUG
        dest->pFEID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strFEID);
#endif
    }
    else
    {
        dest->strFEID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pFEID = NULL;
#endif
    }
	
	/* strFEIDHost */
	if(source->strFEIDHost > UNDEFINED)
    {
        dest->strFEIDHost = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strFEIDHost);
        if(dest->strFEIDHost == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderCopy - Failed to copy strFEIDHost"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pFEIDHost */
#ifdef SIP_DEBUG
        dest->pFEIDHost = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strFEIDHost);
#endif
    }
    else
    {
        dest->strFEIDHost = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pFEIDHost = NULL;
#endif
    }

	/* strRKSGroupID */
	if(source->strRKSGroupID > UNDEFINED)
    {
        dest->strRKSGroupID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strRKSGroupID);
        if(dest->strRKSGroupID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderCopy - Failed to copy strRKSGroupID"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pRKSGroupID */
#ifdef SIP_DEBUG
        dest->pRKSGroupID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strRKSGroupID);
#endif
    }
    else
    {
        dest->strRKSGroupID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pRKSGroupID = NULL;
#endif
    }
	
	/* hChargeAddr */
    stat = RvSipPDCSBillingInfoHeaderSetAddrSpec(hDestination, source->hChargeAddr,
												 RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderCopy: Failed to copy hChargeAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

	/* hCallingAddr */
    stat = RvSipPDCSBillingInfoHeaderSetAddrSpec(hDestination, source->hCallingAddr,
												 RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderCopy: Failed to copy hCallingAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }
	
	/* hCalledAddr */
    stat = RvSipPDCSBillingInfoHeaderSetAddrSpec(hDestination, 
												 source->hCalledAddr,
												 RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED);

    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderCopy: Failed to copy hCalledAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

	/* hRoutingAddr */
    stat = RvSipPDCSBillingInfoHeaderSetAddrSpec(hDestination, 
												 source->hRoutingAddr,
												 RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderCopy: Failed to copy hRoutingAddr. hDest 0x%p, stat %d",
            hDestination, stat));
        return stat;
    }

	/* hLocRouteAddr */
    stat = RvSipPDCSBillingInfoHeaderSetAddrSpec(hDestination, 
												 source->hLocRouteAddr,
												 RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE);
    if(stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderCopy: Failed to copy hLocRouteAddr. hDest 0x%p, stat %d",
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
                      "RvSipPDCSBillingInfoHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPDCSBillingInfoHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPDCSBillingInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSBillingInfo header object to a textual PDCSBillingInfo header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSBillingInfo header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderEncode(
                                          IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPDCSBillingInfoHeader* pHeader;

    pHeader = (MsgPDCSBillingInfoHeader*)hHeader;

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
                "RvSipPDCSBillingInfoHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PDCSBillingInfoHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PDCSBillingInfoHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PDCSBillingInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-Billing-Info header (as string) on it.
 *          format: "P-DCS-Billing-Info: Billing-Correlation-ID "/" FEID *(SEMI Billing-Info-param)"
 *                  
 * Return Value: RV_OK					   - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-Billing-Info header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PDCSBillingInfoHeaderEncode(
                                  IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                  IN    HRPOOL							 hPool,
                                  IN    HPAGE							 hPage,
                                  IN    RvBool							 bInUrlHeaders,
                                  INOUT RvUint32*						 length)
{
    MsgPDCSBillingInfoHeader*	pHeader;
    RvStatus					stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PDCSBillingInfoHeaderEncode - Encoding PDCSBillingInfo header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-DCS-Billing-Info" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-DCS-Billing-Info", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSBillingInfoHeaderEncode - Failed to encoding PDCSBillingInfo header. stat is %d",
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
                "PDCSBillingInfoHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
    }

	/* BillingCorrelationID "/" FEID "@" FEDIHost */
	if( pHeader->strBillingCorrelationID > UNDEFINED &&
		pHeader->strFEID				 > UNDEFINED &&
		pHeader->strFEIDHost			 > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBillingCorrelationID,
                                    length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "/", length);

		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strFEID,
                                    length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetAtChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strFEIDHost,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode PDCSBillingInfoHeaderEncode header. stat is %d",
				stat));
			return stat;
		}
    }
    else
    {
        /* These are not optional parameters */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSBillingInfoHeaderEncode: Failed. BillingCorrelationID or FEID information missing. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
	
	/* RKSGroupID (insert ";" in the beginning) */
    if(pHeader->strRKSGroupID > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "rksgroup=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "rksgroup", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		/* set the RKSGroupID string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strRKSGroupID,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode RKSGroupID String. stat is %d",
				stat));
		}
    }
    
    /* charge (insert ";" in the beginning) */
    if(pHeader->hChargeAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "charge=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "charge", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the adder-spec */
        stat = AddrEncode(pHeader->hChargeAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode charge addr-spec. stat is %d",
				stat));
		}
    }

	/* calling (insert ";" in the beginning) */
    if(pHeader->hCallingAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "calling=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "calling", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the adder-spec */
        stat = AddrEncode(pHeader->hCallingAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode calling addr-spec. stat is %d",
				stat));
		}
    }

	/* called (insert ";" in the beginning) */
    if(pHeader->hCalledAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "called=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "called", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the adder-spec */
        stat = AddrEncode(pHeader->hCalledAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode called addr-spec. stat is %d",
				stat));
		}
    }

	/* routing (insert ";" in the beginning) */
    if(pHeader->hRoutingAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "routing=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "routing", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the adder-spec */
        stat = AddrEncode(pHeader->hRoutingAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode routing addr-spec. stat is %d",
				stat));
		}
    }

	/* locroute (insert ";" in the beginning) */
    if(pHeader->hLocRouteAddr != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "locroute=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "locroute", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		/* Set the adder-spec */
        stat = AddrEncode(pHeader->hLocRouteAddr, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
        
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSBillingInfoHeaderEncode - Failed to encode locroute addr-spec. stat is %d",
				stat));
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
            "PDCSBillingInfoHeaderEncode - Failed to encoding PDCSBillingInfo header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSBillingInfo header-for example,
 *          "PDCSBillingInfo:sip:172.20.5.3:5060"-into a PDCSBillingInfo header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSBillingInfo header object.
 *    buffer    - Buffer containing a textual PDCSBillingInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderParse(
                                 IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                 IN    RvChar*							buffer)
{
    MsgPDCSBillingInfoHeader*    pHeader = (MsgPDCSBillingInfoHeader*)hHeader;
	RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PDCSBillingInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_BILLING_INFO,
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
 * RvSipPDCSBillingInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSBillingInfo header value into an PDCSBillingInfo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSBillingInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSBillingInfo header object.
 *    buffer    - The buffer containing a textual PDCSBillingInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderParseValue(
                                 IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                 IN    RvChar*							buffer)
{
    MsgPDCSBillingInfoHeader*    pHeader = (MsgPDCSBillingInfoHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PDCSBillingInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_BILLING_INFO,
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
 * RvSipPDCSBillingInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSBillingInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderFix(
                                     IN RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPDCSBillingInfoHeader* pHeader = (MsgPDCSBillingInfoHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPDCSBillingInfoHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSBillingInfoHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSBillingInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The Header to take the Pool from.
 ***************************************************************************/
HRPOOL SipPDCSBillingInfoHeaderGetPool(RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSBillingInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The Header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSBillingInfoHeaderGetPage(RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSBillingInfo header fields are kept in a string format-for example, the
 *          PDCSBillingInfo header VNetworkSpec name. In order to get such a field from the PDCSBillingInfo header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the PDCSBillingInfo header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSBillingInfoHeaderGetStringLength(
                                      IN  RvSipPDCSBillingInfoHeaderHandle     hHeader,
                                      IN  RvSipPDCSBillingInfoHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPDCSBillingInfoHeader* pHeader = (MsgPDCSBillingInfoHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_DCS_BILLING_INFO_BILLING_CORRELATION_ID:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetStrBillingCorrelationID(hHeader);
            break;
        }
		case RVSIP_P_DCS_BILLING_INFO_FEID:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetStrFEID(hHeader);
            break;
        }
		case RVSIP_P_DCS_BILLING_INFO_FEID_HOST:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetStrFEIDHost(hHeader);
            break;
        }
		case RVSIP_P_DCS_BILLING_INFO_RKS_GROUP_ID:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetStrRKSGroupID(hHeader);
            break;
        }
        case RVSIP_P_DCS_BILLING_INFO_OTHER_PARAMS:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_DCS_BILLING_INFO_BAD_SYNTAX:
        {
            stringOffset = SipPDCSBillingInfoHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSBillingInfoHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPDCSBillingInfoHeaderGetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General:This method gets the BillingCorrelationID in the PDCSBillingInfo header object.
 * Return Value: BillingCorrelationID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrBillingCorrelationID(IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strBillingCorrelationID;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header BillingCorrelationID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrBillingCorrelationID(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetStrBillingCorrelationID(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BillingCorrelationID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pBillingCorrelationID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrBillingCorrelationID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pBillingCorrelationID,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32						newStr;
    MsgPDCSBillingInfoHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pBillingCorrelationID,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBillingCorrelationID = newStr;
#ifdef SIP_DEBUG
    pHeader->pBillingCorrelationID = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strBillingCorrelationID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General:Sets the BillingCorrelationID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strBillingCorrelationID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrBillingCorrelationID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pBillingCorrelationID)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetStrBillingCorrelationID(hHeader, pBillingCorrelationID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrFEID
 * ------------------------------------------------------------------------
 * General:This method gets the FEID in the PDCSBillingInfo header object.
 * Return Value: FEID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrFEID(
                                            IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strFEID;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrFEID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header FEID field of the PDCSBillingInfo
 *			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrFEID(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetStrFEID(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrFEID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FEID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pFEID - The FEID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing FEID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrFEID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pFEID,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32						newStr;
    MsgPDCSBillingInfoHeader*	pHeader;
    RvStatus					retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pFEID,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strFEID = newStr;
#ifdef SIP_DEBUG
    pHeader->pFEID = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strFEID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrFEID
 * ------------------------------------------------------------------------
 * General:Sets the FEID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrFEID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pFEID)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetStrFEID(hHeader, pFEID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrFEIDHost
 * ------------------------------------------------------------------------
 * General:This method gets the FEIDHost in the PDCSBillingInfo header object.
 * Return Value: FEIDHost offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrFEIDHost(IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strFEIDHost;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrFEIDHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header BillingCorrelationID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrFEIDHost(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetStrFEIDHost(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrFEIDHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FEIDHost in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pBillingCorrelationID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrFEIDHost(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pFEIDHost,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSBillingInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pFEIDHost,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strFEIDHost = newStr;
#ifdef SIP_DEBUG
    pHeader->pFEIDHost = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strFEIDHost);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrFEIDHost
 * ------------------------------------------------------------------------
 * General:Sets the FEIDHost field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strBillingCorrelationID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrFEIDHost(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pFEIDHost)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetStrFEIDHost(hHeader, pFEIDHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General:This method gets the RKSGroupID in the PDCSBillingInfo header object.
 * Return Value: BillingCorrelationID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrRKSGroupID(IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strRKSGroupID;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header RKSGroupID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrRKSGroupID(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetStrRKSGroupID(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BillingCorrelationID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pRKSGroupID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrRKSGroupID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pRKSGroupID,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSBillingInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pRKSGroupID,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strRKSGroupID = newStr;
#ifdef SIP_DEBUG
    pHeader->pRKSGroupID = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strRKSGroupID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General:Sets the BillingCorrelationID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strRKSGroupID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrRKSGroupID(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pRKSGroupID)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetStrRKSGroupID(hHeader, pRKSGroupID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PDCSBillingInfo header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSBillingInfo header object.
 *	  ePDCSBillingInfoAddrType - Enumeration for the field from which to get the address.	
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPDCSBillingInfoHeaderGetAddrSpec(
                        IN RvSipPDCSBillingInfoHeaderHandle hHeader,
						IN RvSipPDCSBillingInfoAddressType	ePDCSBillingInfoAddrType)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

	switch(ePDCSBillingInfoAddrType)
	{
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE:
		return ((MsgPDCSBillingInfoHeader*)hHeader)->hChargeAddr;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING:
		return ((MsgPDCSBillingInfoHeader*)hHeader)->hCallingAddr;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED:
		return ((MsgPDCSBillingInfoHeader*)hHeader)->hCalledAddr;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING:
		return ((MsgPDCSBillingInfoHeader*)hHeader)->hRoutingAddr;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE:
		return ((MsgPDCSBillingInfoHeader*)hHeader)->hLocRouteAddr;
	default:
		return NULL;
	}
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PDCSBillingInfo header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *               Address Spec is removed from the PDCSBillingInfo header.
 *	  ePDCSBillingInfoAddrType - Enumeration for the field in which to set the address.	
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetAddrSpec(
                        IN	RvSipPDCSBillingInfoHeaderHandle	hHeader,
                        IN	RvSipAddressHandle					hAddrSpec,
						IN	RvSipPDCSBillingInfoAddressType		ePDCSBillingInfoAddrType)
{
    RvStatus					stat;
    RvSipAddressType			currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress					*pAddr			= (MsgAddress*)hAddrSpec;
    MsgPDCSBillingInfoHeader	*pHeader	    = (MsgPDCSBillingInfoHeader*)hHeader;
	RvSipAddressHandle*			hAddr;
	
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	switch(ePDCSBillingInfoAddrType)
	{
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE:
		hAddr = &pHeader->hChargeAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING:
		hAddr = &pHeader->hCallingAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED:
		hAddr = &pHeader->hCalledAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING:
		hAddr = &pHeader->hRoutingAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE:
		hAddr = &pHeader->hLocRouteAddr;
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
            stat = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
                                                     pAddr->eAddrType, ePDCSBillingInfoAddrType,
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
 * SipPDCSBillingInfoHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PDCSBillingInfo header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetOtherParams(
                                            IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header other params field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          Not all the PDCSBillingInfo header parameters have separated fields in the PDCSBillingInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetOtherParams(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pOtherParams - The Other Params to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetOtherParams(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSBillingInfoHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSBillingInfoHeader*) hHeader;

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
 * RvSipPDCSBillingInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSBillingInfo header object.
 *         Not all the PDCSBillingInfo header parameters have separated fields in the PDCSBillingInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strOtherParams - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetOtherParams(
                                     IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrBadSyntax(
                                    IN RvSipPDCSBillingInfoHeaderHandle hHeader)
{
    return ((MsgPDCSBillingInfoHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSBillingInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrBadSyntax(
                                               IN RvSipPDCSBillingInfoHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSBillingInfoHeader*)hHeader)->hPool,
                                  ((MsgPDCSBillingInfoHeader*)hHeader)->hPage,
                                  SipPDCSBillingInfoHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrBadSyntax
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
RvStatus SipPDCSBillingInfoHeaderSetStrBadSyntax(
                                  IN RvSipPDCSBillingInfoHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPDCSBillingInfoHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPDCSBillingInfoHeader*)hHeader;

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
 * RvSipPDCSBillingInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSBillingInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrBadSyntax(
                                  IN RvSipPDCSBillingInfoHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSBillingInfoHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PDCSBillingInfoHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PDCSBillingInfoHeaderClean( IN MsgPDCSBillingInfoHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType				= RVSIP_HEADERTYPE_P_DCS_BILLING_INFO;
    pHeader->strOtherParams				= UNDEFINED;
	pHeader->strBillingCorrelationID	= UNDEFINED;
	pHeader->strFEID					= UNDEFINED;
	pHeader->strFEIDHost				= UNDEFINED;
	pHeader->strRKSGroupID				= UNDEFINED;
	pHeader->hChargeAddr				= NULL;
	pHeader->hCallingAddr				= NULL;
	pHeader->hCalledAddr				= NULL;
	pHeader->hRoutingAddr				= NULL;
	pHeader->hLocRouteAddr				= NULL;

#ifdef SIP_DEBUG
    pHeader->pOtherParams				= NULL;
	pHeader->pBillingCorrelationID		= NULL;
	pHeader->pFEID						= NULL;
	pHeader->pFEIDHost					= NULL;
	pHeader->pRKSGroupID				= NULL;
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

