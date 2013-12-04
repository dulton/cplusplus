 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPChargingFunctionAddressesHeader.c							*
 *																					*
 * The file defines the methods of the PChargingFunctionAddresses header object:    *
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

#include "RvSipPChargingFunctionAddressesHeader.h"
#include "_SipPChargingFunctionAddressesHeader.h"
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
static void PChargingFunctionAddressesHeaderClean(IN MsgPChargingFunctionAddressesHeader* pHeader,
										  IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingFunctionAddresses header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PChargingFunctionAddresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPChargingFunctionAddressesHeaderHandle* hHeader)
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

    stat = RvSipPChargingFunctionAddressesHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingFunctionAddresses Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PChargingFunctionAddresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPChargingFunctionAddressesHeaderHandle* hHeader)
{
    MsgPChargingFunctionAddressesHeader*   pHeader;
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

    pHeader = (MsgPChargingFunctionAddressesHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPChargingFunctionAddressesHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesHeaderConstruct - Failed to construct PChargingFunctionAddresses header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PChargingFunctionAddressesHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPChargingFunctionAddressesHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesHeaderConstruct - PChargingFunctionAddresses header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PChargingFunctionAddresses header 
 *			object to a destination PChargingFunctionAddresses header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingFunctionAddresses header object.
 *    hSource      - Handle to the destination PChargingFunctionAddresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderCopy(
                                         INOUT	RvSipPChargingFunctionAddressesHeaderHandle hDestination,
                                         IN		RvSipPChargingFunctionAddressesHeaderHandle hSource)
{
    MsgPChargingFunctionAddressesHeader*	source;
    MsgPChargingFunctionAddressesHeader*	dest;
	RvStatus								stat;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPChargingFunctionAddressesHeader*)hSource;
    dest   = (MsgPChargingFunctionAddressesHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

	/* Copy ccfList and ecfList */
	stat = RvSipPChargingFunctionAddressesHeaderSetCcfList(hDestination, source->ccfList);
	if(RV_OK != stat)
	{
		return stat;
	}

 	stat = RvSipPChargingFunctionAddressesHeaderSetEcfList(hDestination, source->ecfList);
	if(RV_OK != stat)
	{
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
                      "RvSipPChargingFunctionAddressesHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPChargingFunctionAddressesHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPChargingFunctionAddressesHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PChargingFunctionAddresses header object to a textual PChargingFunctionAddresses header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PChargingFunctionAddresses header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderEncode(
                                          IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPChargingFunctionAddressesHeader* pHeader;

    pHeader = (MsgPChargingFunctionAddressesHeader*)hHeader;

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
                "RvSipPChargingFunctionAddressesHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingFunctionAddressesHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PChargingFunctionAddressesHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingFunctionAddressesHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PChargingFunctionAddressesHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PChargingFunctionAddressesHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PChargingFunctionAddresses header (as string) on it.
 *          format: "P-Charging-Function-Addresses: "
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PChargingFunctionAddresses header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PChargingFunctionAddressesHeaderEncode(
                                  IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPChargingFunctionAddressesHeader*	pHeader;
    RvStatus								stat;
	RvBool									isFirstParam = RV_TRUE;	

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPChargingFunctionAddressesHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PChargingFunctionAddressesHeaderEncode - Encoding PChargingFunctionAddresses header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Charging-Function-Addresses" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Charging-Function-Addresses", length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PChargingFunctionAddressesHeaderEncode - Failed to encoding PChargingFunctionAddresses header. stat is %d",
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
                "PChargingFunctionAddressesHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

    /* Ccf */
	if(pHeader->ccfList != NULL)
	{
		RvBool bIsListEmpty;

		RvSipCommonListIsEmpty(pHeader->ccfList, &bIsListEmpty);
		if(RV_FALSE == bIsListEmpty)
		{
			stat = MsgUtilsEncodePChargingFunctionAddressesList(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->ccfList,
													pHeader->hPool,
													pHeader->hPage,
													bInUrlHeaders,
													RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF,
													length);
			if (stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingFunctionAddressesHeaderEncode - Failed to encode ccf. stat is %d",
				stat));
			}

			isFirstParam = RV_FALSE;
		}
	}
    
	/* Ecf */
	if(pHeader->ecfList != NULL)
	{
		RvBool bIsListEmpty;

		RvSipCommonListIsEmpty(pHeader->ecfList, &bIsListEmpty);
		if(RV_FALSE == bIsListEmpty)
		{
			/* set ";" in the beginning, unless it's the first param */
			if(!isFirstParam)
			{
				stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
					                                MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
			}
			
			stat = MsgUtilsEncodePChargingFunctionAddressesList(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->ecfList,
													pHeader->hPool,
													pHeader->hPage,
													bInUrlHeaders,
													RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF,
													length);
			if (stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingFunctionAddressesHeaderEncode - Failed to encode ecf. stat is %d",
				stat));
			}

			isFirstParam = RV_FALSE;
		}
	}
    
    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
         /* set ";" in the beginning, unless it's the first param */
		if(!isFirstParam)
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
				                                MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		}

        /* set OtherParms */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
		isFirstParam = RV_FALSE;
	    if (stat != RV_OK)
		{	
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingFunctionAddressesHeaderEncode - Failed to encoding PChargingFunctionAddresses header. stat is %d",
				stat));
		}
    }

	/* One Parameter at least must be present, if none were found: */
	if(isFirstParam == RV_TRUE)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PChargingFunctionAddressesHeaderEncode - Failed to encoding PChargingFunctionAddresses header. stat is %d",
				RV_ERROR_BADPARAM));
		return RV_ERROR_BADPARAM;
	}

    return stat;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingFunctionAddresses header-for example,
 *          "PChargingFunctionAddresses:sip:172.20.5.3:5060"-into a PChargingFunctionAddresses header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PChargingFunctionAddresses header object.
 *    buffer    - Buffer containing a textual PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderParse(
                                     IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPChargingFunctionAddressesHeader*    pHeader = (MsgPChargingFunctionAddressesHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PChargingFunctionAddressesHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES,
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
 * RvSipPChargingFunctionAddressesHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingFunctionAddresses header value into an PChargingFunctionAddresses header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPChargingFunctionAddressesHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PChargingFunctionAddresses header object.
 *    buffer    - The buffer containing a textual PChargingFunctionAddresses header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderParseValue(
                                     IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPChargingFunctionAddressesHeader*    pHeader = (MsgPChargingFunctionAddressesHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}


    PChargingFunctionAddressesHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES,
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
 * RvSipPChargingFunctionAddressesHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PChargingFunctionAddresses header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderFix(
                                     IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPChargingFunctionAddressesHeader* pHeader = (MsgPChargingFunctionAddressesHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPChargingFunctionAddressesHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PChargingFunctionAddresses object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPChargingFunctionAddressesHeaderGetPool(RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PChargingFunctionAddresses object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPChargingFunctionAddressesHeaderGetPage(RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingFunctionAddresses header fields are kept in a string format-for example, the
 *          PChargingFunctionAddresses header VNetworkSpec name. In order to get such a field from the PChargingFunctionAddresses header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PChargingFunctionAddresses header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetStringLength(
                                      IN  RvSipPChargingFunctionAddressesHeaderHandle     hHeader,
                                      IN  RvSipPChargingFunctionAddressesHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPChargingFunctionAddressesHeader* pHeader = (MsgPChargingFunctionAddressesHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_CHARGING_FUNCTION_ADDRESSES_OTHER_PARAMS:
        {
            stringOffset = SipPChargingFunctionAddressesHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_CHARGING_FUNCTION_ADDRESSES_BAD_SYNTAX:
        {
            stringOffset = SipPChargingFunctionAddressesHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPChargingFunctionAddressesHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * RvSipPChargingFunctionAddressesHeaderConstructCcfList
 * ------------------------------------------------------------------------
 * General: Constructs a RvSipCommonList object on the header's page, into 
 *			the given handle. Also placing the list in the CcfList parameter
 *			of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader	- Handle to the header object.
 * output: hList	- Handle to the constructed list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructCcfList(
                                       IN	RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*						hList)
{
	MsgPChargingFunctionAddressesHeader*	pHeader;

	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgPChargingFunctionAddressesHeader*) hHeader;
	
	RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, hList);
	
	if(hList == NULL)
	{
		return RV_ERROR_OUTOFRESOURCES;
	}
	
	pHeader->ccfList = *hList;
    return RV_OK;
}
/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetCcfList
 * ------------------------------------------------------------------------
 * General: Returns a handle of type RvSipCommonListHandle to the ccf list of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetCcfList(
                                               IN RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->ccfList;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetCcfList
 * ------------------------------------------------------------------------
 * General: Sets the ccfList in the PChargingFunctionAddresses header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingFunctionAddressesHeaderConstructCcfList(),
 *			and than RvSipPChargingFunctionAddressesListElementConstructInHeader() to construct each element.
 ** Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PChargingFunctionAddresses header object.
 *    hList   - Handle to the ccfList object. If NULL is supplied, the existing
 *              ccfList is removed from the PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetCcfList(
                                    IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle							hList)
{
    RvStatus										stat;
    MsgPChargingFunctionAddressesHeader				*pHeader	= (MsgPChargingFunctionAddressesHeader*)hHeader;
	RvInt32											safeCounter = 0;
	RvInt32											maxLoops    = 10000;
	RvInt											elementType;
	void*											element;
	RvSipCommonListElemHandle						hRelative = NULL;
	RvSipPChargingFunctionAddressesListElemHandle	hElement = NULL;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hList == NULL)
    {
        pHeader->ccfList = NULL;
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
			pHeader->ccfList = NULL;
		}
		else
		{
			/* Construct a new list */
			RvSipCommonListHandle hNewList;
			RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, &hNewList);
			
			if(hNewList == NULL)
			{
				return RV_ERROR_OUTOFRESOURCES;
			}

			pHeader->ccfList = hNewList;
			
			while ((RV_OK == stat) && (NULL != element))
			{
				stat = RvSipPChargingFunctionAddressesListElementConstructInHeader(hHeader, &hElement);
				stat = RvSipPChargingFunctionAddressesListElementCopy(hElement,
																(RvSipPChargingFunctionAddressesListElemHandle)element);
				stat = RvSipCommonListPushElem(pHeader->ccfList, RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF,
												(void*)hElement, RVSIP_LAST_ELEMENT, NULL, &phNewPos);
				
				if (RV_OK != stat)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingFunctionAddressesHeaderSetCcfList - Failed to copy ccfList. stat = %d",
							  stat));
					return stat;
				}
				
				stat = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
												&elementType, &element, &hRelative);

				safeCounter++;
				if (safeCounter > maxLoops)
				{
					RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingFunctionAddressesHeaderSetCcfList - Execption in loop. Too many rounds."));
					return RV_ERROR_UNKNOWN;
				}
			}
		}
    }

	return RV_OK;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstructEcfList
 * ------------------------------------------------------------------------
 * General: Constructs a RvSipCommonList object on the header's page, into 
 *			the given handle. Also placing the list in the EcfList parameter
 *			of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader	- Handle to the header object.
 * output: hList	- Handle to the constructed list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructEcfList(
                                       IN	RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*				hList)
{
	MsgPChargingFunctionAddressesHeader*	pHeader;

	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgPChargingFunctionAddressesHeader*) hHeader;
	
	RvSipCommonListConstructOnPage(pHeader->hPool,pHeader->hPage, (RV_LOG_Handle)pHeader->pMsgMgr->pLogMgr, hList);
	
	if(hList == NULL)
	{
		return RV_ERROR_OUTOFRESOURCES;
	}
	
	pHeader->ecfList = *hList;
    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetEcfList
 * ------------------------------------------------------------------------
 * General: Returns a handle of type RvSipCommonListHandle to the ecf list of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetEcfList(
                                               IN RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return NULL;
	}

    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->ecfList;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetEcfList
 * ------------------------------------------------------------------------
 * General: Sets the ecfList in the PChargingFunctionAddresses header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingFunctionAddressesHeaderConstructEcfList(),
 *			and than RvSipPChargingFunctionAddressesListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PChargingFunctionAddresses header object.
 *    hList - Handle to the ecfList object. If NULL is supplied, the existing
 *              ecfList is removed from the PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetEcfList(
                                    IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle							hList)
{
    RvStatus										stat;
    MsgPChargingFunctionAddressesHeader				*pHeader	= (MsgPChargingFunctionAddressesHeader*)hHeader;
	RvInt32								   		    safeCounter = 0;
	RvInt32 										maxLoops    = 10000;
	RvInt											elementType;
	void*											element;
	RvSipCommonListElemHandle						hRelative = NULL;
	RvSipPChargingFunctionAddressesListElemHandle	hElement = NULL;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hList == NULL)
    {
        pHeader->ecfList = NULL;
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
			pHeader->ccfList = NULL;
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
			
			pHeader->ecfList = hNewList;
			
			while ((RV_OK == stat) && (NULL != element))
			{
				stat = RvSipPChargingFunctionAddressesListElementConstructInHeader(hHeader, &hElement);
				stat = RvSipPChargingFunctionAddressesListElementCopy(hElement,
																(RvSipPChargingFunctionAddressesListElemHandle)element);
				stat = RvSipCommonListPushElem(pHeader->ecfList, RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF,
												(void*)hElement, RVSIP_LAST_ELEMENT, NULL, &phNewPos);
				
				if (RV_OK != stat)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingFunctionAddressesHeaderSetEcfList - Failed to copy ecfList. stat = %d",
							  stat));
					return stat;
				}
				
				stat = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
												&elementType, &element, &hRelative);

				safeCounter++;
				if (safeCounter > maxLoops)
				{
					RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							  "RvSipPChargingFunctionAddressesHeaderSetEcfList - Execption in loop. Too many rounds."));
					return RV_ERROR_UNKNOWN;
				}
			}
		}
    }

	return RV_OK;
}

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PChargingFunctionAddresses header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingFunctionAddresses header object..
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesHeaderGetOtherParams(
                                            IN RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PChargingFunctionAddresses header other params field of the PChargingFunctionAddresses header object into a
 *          given buffer.
 *          Not all the PChargingFunctionAddresses header parameters have separated fields in the PChargingFunctionAddresses
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingFunctionAddresses header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetOtherParams(
                                               IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingFunctionAddressesHeader*)hHeader)->hPool,
                                  ((MsgPChargingFunctionAddressesHeader*)hHeader)->hPage,
                                  SipPChargingFunctionAddressesHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipPChargingFunctionAddressesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PChargingFunctionAddressesHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingFunctionAddresses header object.
 *            pOtherParams - The Other Params to be set in the PChargingFunctionAddresses header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingFunctionAddressesHeaderSetOtherParams(
                                     IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPChargingFunctionAddressesHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPChargingFunctionAddressesHeader*) hHeader;

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
 * RvSipPChargingFunctionAddressesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PChargingFunctionAddresses header object.
 *         Not all the PChargingFunctionAddresses header parameters have separated fields in the PChargingFunctionAddresses
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingFunctionAddresses header object.
 *    strOtherParams - The extended parameters field to be set in the PChargingFunctionAddresses header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetOtherParams(
                                     IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingFunctionAddressesHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesHeaderGetStrBadSyntax(
                                    IN RvSipPChargingFunctionAddressesHeaderHandle hHeader)
{
    return ((MsgPChargingFunctionAddressesHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PChargingFunctionAddresses header,
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
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetStrBadSyntax(
                                               IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPChargingFunctionAddressesHeader*)hHeader)->hPool,
                                  ((MsgPChargingFunctionAddressesHeader*)hHeader)->hPage,
                                  SipPChargingFunctionAddressesHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderSetStrBadSyntax
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
RvStatus SipPChargingFunctionAddressesHeaderSetStrBadSyntax(
                                  IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPChargingFunctionAddressesHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPChargingFunctionAddressesHeader*)hHeader;

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
 * RvSipPChargingFunctionAddressesHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PChargingFunctionAddresses header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetStrBadSyntax(
                                  IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingFunctionAddressesHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        LIST FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementConstructInHeader
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingFunctionAddressesListElement object inside a given Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader			- Handle to the header object.
 * output: hElement			- Handle to the newly constructed PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementConstructInHeader(
                               IN	RvSipPChargingFunctionAddressesHeaderHandle		hHeader,
                               OUT	RvSipPChargingFunctionAddressesListElemHandle*	hElement)
{
	MsgPChargingFunctionAddressesHeader*	header;
	RvStatus								stat;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hElement == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hElement = NULL;

    header = (MsgPChargingFunctionAddressesHeader*)hHeader;

    stat = RvSipPChargingFunctionAddressesListElementConstruct((RvSipMsgMgrHandle)header->pMsgMgr, 
														header->hPool,
														header->hPage,
														hElement);
	return stat;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingFunctionAddressesListElement object. The element is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          element object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hElement - Handle to the newly constructed PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementConstruct(
							   IN  RvSipMsgMgrHandle								hMsgMgr,
							   IN  HRPOOL											hPool,
							   IN  HPAGE											hPage,
							   OUT RvSipPChargingFunctionAddressesListElemHandle*	hElement)
{
	MsgPChargingFunctionAddressesListElem*	pElement;
    MsgMgr*									pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hElement == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hElement = NULL;

    pElement = (MsgPChargingFunctionAddressesListElem*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                   hPool,
                                                   hPage,
                                                   sizeof(MsgPChargingFunctionAddressesListElem),
                                                   RV_TRUE);
    if(pElement == NULL)
    {
        *hElement = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesListElementConstruct - Failed to construct Element. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

	pElement->pMsgMgr	= pMsgMgr;
    pElement->hPage		= hPage;
    pElement->hPool		= hPool;
	
	pElement->strValue	= UNDEFINED;
	
#ifdef SIP_DEBUG
	pElement->pValue		= NULL;
#endif
	
   *hElement = (RvSipPChargingFunctionAddressesListElemHandle)pElement;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPChargingFunctionAddressesListElementConstruct - PChargingFunctionAddressesListElem object was constructed successfully. (hPool=0x%p, hPage=0x%p, hElement=0x%p)",
            hPool, hPage, *hElement));

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source PChargingFunctionAddressesListElement object to a 
 *		   destination PChargingFunctionAddressesListElement object.
 *         You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingFunctionAddressesListElement object.
 *    hSource      - Handle to the source PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementCopy(
								INOUT	RvSipPChargingFunctionAddressesListElemHandle	hDestination,
								IN		RvSipPChargingFunctionAddressesListElemHandle	hSource)
{
    MsgPChargingFunctionAddressesListElem* dest;
    MsgPChargingFunctionAddressesListElem* source;

    if((hDestination == NULL)||(hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    dest	= (MsgPChargingFunctionAddressesListElem*)hDestination;
    source	= (MsgPChargingFunctionAddressesListElem*)hSource;

	/* copy value */
    if(source->strValue > UNDEFINED)
    {
        dest->strValue = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strValue);
        if(dest->strValue == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipPChargingFunctionAddressesListElementCopy - Failed to copy strValue"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pValue = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strValue);
#endif
    }
    else
    {
        dest->strValue = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pValue= NULL;
#endif
    }

	return RV_OK;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingFunctionAddressesListElement fields are kept in a string
 *			format-for example, the value parameter. In order to get such a field from
 *			the element object, your application should supply an adequate buffer to 
 *			where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingFunctionAddresses header object.
 *  stringName	- Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingFunctionAddressesListElementGetStringLength(
                              IN  RvSipPChargingFunctionAddressesListElemHandle		hElement,
                              IN  RvSipPChargingFunctionAddressesListElementStringName stringName)
{
    MsgPChargingFunctionAddressesListElem*	pElement = (MsgPChargingFunctionAddressesListElem*)hElement;
	RvInt32									stringOffset;

	if(hElement == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_ELEMENT_VALUE:
        {
            stringOffset = SipPChargingFunctionAddressesListElementGetStrValue(hElement);
            break;
        }
		default:
        {
            RvLogExcep(pElement->pMsgMgr->pLogSrc,(pElement->pMsgMgr->pLogSrc,
                "RvSipPChargingFunctionAddressesListElementGetStringLength - Unknown stringName %d", stringName));
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
 * SipPChargingFunctionAddressesListElementGetStrValue
 * ------------------------------------------------------------------------
 * General:This method gets the value in the PChargingFunctionAddressesListElement object.
 * Return Value: value offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingFunctionAddressesListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesListElementGetStrValue(IN RvSipPChargingFunctionAddressesListElemHandle hElement)
{
    return ((MsgPChargingFunctionAddressesListElem*)hElement)->strValue;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementGetStrValue
 * ------------------------------------------------------------------------
 * General: Copies the PChargingFunctionAddressesListElement value field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingFunctionAddressesListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementGetStrValue(
                               IN  RvSipPChargingFunctionAddressesListElemHandle	hElement,
                               IN  RvChar*											strBuffer,
                               IN  RvUint											bufferLen,
                               OUT RvUint*											actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgPChargingFunctionAddressesListElem*)hElement)->hPool,
                                  ((MsgPChargingFunctionAddressesListElem*)hElement)->hPage,
                                  SipPChargingFunctionAddressesListElementGetStrValue(hElement),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPChargingFunctionAddressesListElementSetStrValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Value in the
 *          PChargingFunctionAddressesListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hElement - Handle of the PChargingFunctionAddressesListElement object.
 *            pValue - The Value to be set in the PChargingFunctionAddressesListElement.
 *                          If NULL, the existing Value string in the element will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingFunctionAddressesListElementSetStrValue(
                             IN    RvSipPChargingFunctionAddressesListElemHandle	hElement,
                             IN    RvChar *									pValue,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset)
{
    RvInt32							newStr;
    MsgPChargingFunctionAddressesListElem*	pElement;
    RvStatus						retStatus;

    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pElement = (MsgPChargingFunctionAddressesListElem*) hElement;

    retStatus = MsgUtilsSetString(pElement->pMsgMgr, hPool, hPage, strOffset, pValue,
                                  pElement->hPool, pElement->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pElement->strValue = newStr;
#ifdef SIP_DEBUG
    pElement->pValue = (RvChar *)RPOOL_GetPtr(pElement->hPool, pElement->hPage,
                                           pElement->strValue);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementSetStrValue
 * ------------------------------------------------------------------------
 * General:Sets the Value field in the PChargingFunctionAddressesListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingFunctionAddressesListElement object.
 *    pValue		- The value to be set in the PChargingFunctionAddressesListElement. If NULL is
 *				  supplied, the existing value field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementSetStrValue(
                             IN    RvSipPChargingFunctionAddressesListElemHandle	hElement,
                             IN    RvChar *									pValue)
{
    if(hElement == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPChargingFunctionAddressesListElementSetStrValue(hElement, pValue, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PChargingFunctionAddressesHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PChargingFunctionAddressesHeaderClean( IN MsgPChargingFunctionAddressesHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES;
    pHeader->strOtherParams		= UNDEFINED;
	pHeader->ccfList			= NULL;
	pHeader->ecfList			= NULL;

#ifdef SIP_DEBUG
    pHeader->pOtherParams		= NULL;
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

