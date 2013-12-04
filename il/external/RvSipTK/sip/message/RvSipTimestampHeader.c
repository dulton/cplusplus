/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipTimestampHeader.c                                 *
 *                                                                            *
 * The file defines the methods of the Timestamp header object:               *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author                Date                                            *
 *     ------              ------------                                       *
 *    Galit Edri Domani     May 2005                                          *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT


#include "RvSipTimestampHeader.h"
#include "_SipTimestampHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"


#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void TimestampHeaderClean(IN MsgTimestampHeader* pHeader,
                                 IN RvBool              bCleanBS);


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTimestampHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Timestamp Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderConstruct(
                                         IN  RvSipMsgMgrHandle           hMsgMgr,
                                         IN  HRPOOL                      hPool,
                                         IN  HPAGE                       hPage,
                                         OUT RvSipTimestampHeaderHandle *hHeader)
{

    MsgTimestampHeader*   pHeader;
    MsgMgr*               pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgTimestampHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                         hPool,
                                                         hPage,
                                                         sizeof(MsgTimestampHeader),
                                                         RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipTimestampHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    TimestampHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipTimestampHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipTimestampHeaderConstruct - Timestamp header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;

}

/***************************************************************************
 * RvSipTimestampHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Timestamp header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Timestamp 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipTimestampHeaderHandle             *hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                    stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;


/*	RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderConstruct(
		IN  RvSipMsgMgrHandle        hMsgMgr,
		IN  HRPOOL                   hPool,
		IN  HPAGE                    hPage,
		OUT RvSipTimestampHeaderHandle *hHeader)
*/
    stat = RvSipTimestampHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                            msg->hPool,
                                                            msg->hPage,
                                                            hHeader);
    if(stat != RV_OK)
    {
        return stat;
    }

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_TIMESTAMP,
                              &hListElem,
                              (void**)hHeader);
}



/***************************************************************************
 * RvSipTimestampHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Timestamp header object to a destination 
 *          Timestamp header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Timestamp header object.
 *    hSource      - Handle to the source Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderCopy(
                                     IN    RvSipTimestampHeaderHandle hDestination,
                                     IN    RvSipTimestampHeaderHandle hSource)
{
    MsgTimestampHeader* source, *dest;
    
    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgTimestampHeader*)hSource;
    dest   = (MsgTimestampHeader*)hDestination;

	/* header type */

    dest->eHeaderType = source->eHeaderType;
	
	/* timestampVal & delayVal */

	dest->timestampVal.integerPart = source->timestampVal.integerPart;
	dest->timestampVal.decimalPart = source->timestampVal.decimalPart;

	dest->delayVal.integerPart = source->delayVal.integerPart;
	dest->delayVal.decimalPart = source->delayVal.decimalPart;
	
	

	/* bad syntax */
	if(source->strBadSyntax > UNDEFINED)
	{
		dest->strBadSyntax = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strBadSyntax);
		if (dest->strBadSyntax == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipTimestampHeaderCopy: Failed to copy bad syntax. hDest 0x%p",
						hDestination));
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
 * RvSipTimestampHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Timestamp header object to a textual Timestamp header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Timestamp header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderEncode(
                                          IN    RvSipTimestampHeaderHandle hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length)
{
    RvStatus         stat;
    MsgTimestampHeader* pHeader;

    pHeader = (MsgTimestampHeader*)hHeader;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipTimestampHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipTimestampHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = TimestampHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipTimestampHeaderEncode - Failed. Free page 0x%p on pool 0x%p. TimestampHeaderEncode fail",
                *hPage, hPool));
		
    }
	return stat;
    
}

/***************************************************************************
 * TimestampHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Timestamp header (as string) on it.
 * The Timestamp header looks like: "Timestamp : int32.int32  int32.int32"
 * or could look like :              "Timestamp: int32.int32       .int32"
 *
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 *                            in TimestampHeader  should be "TRUE".
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV TimestampHeaderEncode(
                                      IN    RvSipTimestampHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length)
{    	
    MsgTimestampHeader*  pHeader = (MsgTimestampHeader*)hHeader;
    RvStatus          stat;
	RvChar            strHelper[16];
	
    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
	
    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
		"TimestampHeaderEncode - Encoding Timestamp header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
		hHeader, hPool, hPage));
	

	/* The Timestamp header looks like: "Timestamp : int32.int32  int32.int32"
	   or could look like :              "Timestamp: int32.int32       .int32"
	   or could look like :              "Timestamp: int32.int32"
    */
	
	/* encode "Timestamp" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
		                                hPool, 
										hPage, 
										"Timestamp", 
										length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"TimestampHeaderEncode - Failed to encode Timestamp string.RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}


	
	
	/* encode ":" -(bInUrlHeaders should be TRUE)*/
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
		                                hPool, 
										hPage,
		                                MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
		                                length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"TimestampHeaderEncode - Failed to encode Timestamp seperator. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}

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
                "TimestampHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			
        }
		return stat;
        
    }

	/* timestampVal string:"intPart.decPart" */
	/*timestampVal.integerPart is not an optional parameter*/
	if(pHeader->timestampVal.integerPart > UNDEFINED)   
	{
		/*timestampVal.integerPart*/
		MsgUtils_itoa(pHeader->timestampVal.integerPart, strHelper);

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
			                                hPool, 
											hPage,
											strHelper, 
											length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"TimestampHeaderEncode - Failed to encode timestampVal->integerPart.RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}

		/*if the decimalPart exist (optional parameter),encode the ".decimalPart"
		  otherwise the timestame string will be "int32"*/
		if(pHeader->timestampVal.decimalPart > UNDEFINED)
		{
			/*encode the "." */
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
				hPool, 
				hPage, 
				".", 
				length);
			if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"TimestampHeaderEncode - Failed to encode dotChar in timestampVal->integerPart. RvStatus is %d, hPool 0x%p, hPage 0x%p",
					stat, hPool, hPage));
				return stat;
			}
			/*timestampVal.decimalPart*/
			MsgUtils_itoa(pHeader->timestampVal.decimalPart, strHelper);
			
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
				hPool, 
				hPage,
				strHelper, 
				length);
			if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"TimestampHeaderEncode - Failed to encode Timestamp decimalPart. RvStatus is %d, hPool 0x%p, hPage 0x%p",
					stat, hPool, hPage));
				return stat;
			}	
					
		}
	} 

	/*else if(pHeader->timestampVal.integerPart <= UNDEFINED)*/
	else 
	{
		/* this is not optional */
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"TimestampHeaderEncode Failed. pHeader->timestampVal.integerPart is UNDEFINED. cannot encode. not optional parameter"));
		
		return RV_ERROR_BADPARAM;
	}
	

	/* put 2 spaces between timestampVal to delayVal */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
		                                hPool,
										hPage,
		                                MsgUtilsGetSpaceChar(bInUrlHeaders), 
										length);
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
		                                hPool,
		                                hPage,
	 	                                MsgUtilsGetSpaceChar(bInUrlHeaders), 
		                                length);
	
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"TimestampHeaderEncode - Failed to encode Timestamp.dec  SpaceChar. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}	
		

	/* encode delayVal*/
	/*in case the delayVal.integerPart exist (optional parameter)*/
	if (pHeader->delayVal.integerPart > UNDEFINED)
	{
		MsgUtils_itoa(pHeader->delayVal.integerPart, strHelper);
		
		/*convert delayVal.integerPart to string*/
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
			hPool, 
			hPage,
			strHelper, 
			length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"TimestampHeaderEncode - Failed to encode delay.integerPart.RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
		/*if the delayVal.decimalPart exist - add ".decimalPart"to the Delay string.
		  if delayVal.decimalPart does not exist- the delay string will look like: "int32" (intPart) 
		*/

	}
    /*in case the delayVal.decimalPart exist (optional parameter too)*/
	if (pHeader->delayVal.decimalPart > UNDEFINED) 
	{
		/*encode the "."*/
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
			hPool, 
			hPage, 
			".", 
			length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"TimestampHeaderEncode - Failed to encode delayVal.intPart-dotChar.RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
		
		/*encode delayVal.decimalPart*/
		MsgUtils_itoa(pHeader->delayVal.decimalPart, strHelper);
		/*convert delayVal.decimalPart to string*/
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, 
			hPool, 
			hPage,
			strHelper, 
			length);
		
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"TimestampHeaderEncode - Failed to encode delayVal-decimalPart.RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}
	return stat;
			    
}


/***************************************************************************
 * RvSipTimestampHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Timestamp header into a Timestamp header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Timestamp header object.
 *    buffer    - Buffer containing a textual Timestamp header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderParse(
                                     IN RvSipTimestampHeaderHandle  hHeader,
                                     IN RvChar*                     buffer)
{
    MsgTimestampHeader* pHeader = (MsgTimestampHeader*)hHeader;
    RvStatus             rv;
    
	if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    TimestampHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_TIMESTAMP,
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

 
	return RV_OK;
}


/***************************************************************************
 * RvSipTimestampHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Timestamp header value into an Timestamp header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipTimestampHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Timestamp header object.
 *    buffer    - The buffer containing a textual Timestamp header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderParseValue(
                                     IN RvSipTimestampHeaderHandle  hHeader,
                                     IN RvChar*                     buffer)
{
    MsgTimestampHeader* pHeader = (MsgTimestampHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    TimestampHeaderClean(pHeader, RV_FALSE);

	/*the 5'th parameter should be RV_TRUE if the buffer is only the value of the header*/
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_TIMESTAMP,
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
 * RvSipTimestampHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Timestamp header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderFix(
                                     IN RvSipTimestampHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer)
{
    MsgTimestampHeader* pHeader = (MsgTimestampHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipTimestampHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipTimestampHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				   "RvSipTimestampHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipTimestampHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Timestamp object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipTimestampHeaderGetPool(RvSipTimestampHeaderHandle hHeader)
{
    return ((MsgTimestampHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipTimestampHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Timestamp object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipTimestampHeaderGetPage(RvSipTimestampHeaderHandle hHeader)
{
    return ((MsgTimestampHeader*)hHeader)->hPage;
}


/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/


/***************************************************************************
 * RvSipTimestampHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Timestamp header fields are kept in a string format, for
 *          example, the Timestamp header YYY string. In order to get such a field
 *          from the Timestamp header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Timestamp header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipTimestampHeaderGetStringLength(
                                      IN  RvSipTimestampHeaderHandle     hHeader,
                                      IN  RvSipTimestampHeaderStringName eStringName)
{
    RvInt32             stringOffset = UNDEFINED;
    MsgTimestampHeader* pHeader      = (MsgTimestampHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

	switch (eStringName)
    {
	case RVSIP_TIMESTAMP_BAD_SYNTAX:
		{
			stringOffset = SipTimestampHeaderGetStrBadSyntax(hHeader);
			break;
		}       
	default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					   "RvSipTimestampHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
* RvSipTimestampHeaderSetTimestampInt
* ------------------------------------------------------------------------
* General: Gets the YYY value from the Timestamp Header object.
* Return Value: Returns the YYY value.
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle to the Timestamp header object.
*    newTimestamp- The new value to be set in the object.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetTimestampVal(
															  IN RvSipTimestampHeaderHandle hHeader,
															  IN RvInt32         timestampIntPart,
															  IN RvInt32         timestampDecPart)
{
	MsgTimestampHeader *pheader = (MsgTimestampHeader*)hHeader;
	
	if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	/* timestampIntPart is not  an optional parameter. but it can be set to -1
	   and in this case the encode function will fail */
	if (timestampIntPart <= UNDEFINED ) 
	{

		pheader->timestampVal.integerPart = UNDEFINED;
	}
	else  
	{
		pheader->timestampVal.integerPart = timestampIntPart;
	}
		
	
	if (timestampDecPart > UNDEFINED) 
	{
		pheader->timestampVal.decimalPart = timestampDecPart;
	}
	else
	{
		pheader->timestampVal.decimalPart = UNDEFINED;

	}


	
	return RV_OK;
	
	
}

/***************************************************************************
 * RvSipTimestampHeaderGetTimestampVal
 * ------------------------------------------------------------------------
 * General: Gets the YYY value from the Timestamp Header object.
 * Return Value: Returns the YYY value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Timestamp header object.
 *    timestampNum  - the struct that is filled with the timestamp value
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetTimestampVal(
													  IN RvSipTimestampHeaderHandle hHeader,
													  OUT RvInt32         *timestampIntPart,
													  OUT RvInt32         *timestampDecPart)
{
	MsgTimestampHeader *pheader = (MsgTimestampHeader*)hHeader;

	if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	if (NULL == hHeader)
    {
		*timestampIntPart = UNDEFINED;
		*timestampDecPart = UNDEFINED;
    }
	else  
	{
		*timestampIntPart = pheader->timestampVal.integerPart;
		*timestampDecPart = pheader->timestampVal.decimalPart;		
	}
	
	return RV_OK;
}
/***************************************************************************
 * RvSipTimestampHeaderGetDelayVal
 * ------------------------------------------------------------------------
 * General: Sets the YYY value in the Timestamp Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Timestamp header object.
 *    retDelay - the struct that is filled with the delay value
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetDelayVal(
												  IN RvSipTimestampHeaderHandle hHeader,
												  OUT RvInt32         *delayIntPart,
												  OUT RvInt32         *delayDecPart)
{
	MsgTimestampHeader *pheader = (MsgTimestampHeader*)hHeader;
	
	if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	if (NULL == hHeader)
    {
		*delayIntPart= UNDEFINED;
		*delayDecPart= UNDEFINED;
		
    }
	else
	{
		*delayIntPart = pheader->delayVal.integerPart;
		*delayDecPart = pheader->delayVal.decimalPart;
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipTimestampHeaderSetDelayVal
 * ------------------------------------------------------------------------
 * General: Gets the YYY value from the Timestamp Header object.
 * Return Value: Returns the YYY value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Timestamp header object.
 *    newDelay- The new value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetDelayVal(
														  IN RvSipTimestampHeaderHandle hHeader,
														  IN RvInt32         delayIntPart,
														  IN RvInt32         delayDecPart)
{
	MsgTimestampHeader *pheader = (MsgTimestampHeader*)hHeader;
	
	if(hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	/* delayVal is an optional parameter and it can get any value from application.
	   if it is an illegal value it won't be displayed*/
	
	if (delayIntPart <= UNDEFINED)
	{
		pheader->delayVal.integerPart = UNDEFINED;
	}
	else
	{
		pheader->delayVal.integerPart = delayIntPart;
	}
	
	if (delayDecPart <= UNDEFINED)
	{
		pheader->delayVal.decimalPart = UNDEFINED;
	}
	else
	{
		pheader->delayVal.decimalPart = delayDecPart;
		
	}
	
	
	return RV_OK;
}

/***************************************************************************
 * RvSipTimestampHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Timestamp header,
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
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetStrBadSyntax(
                                     IN  RvSipTimestampHeaderHandle  hHeader,
                                     IN  RvChar*					 strBuffer,
                                     IN  RvUint						 bufferLen,
                                     OUT RvUint*					 actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgTimestampHeader*)hHeader)->hPool,
                                  ((MsgTimestampHeader*)hHeader)->hPage,
                                  SipTimestampHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipTimestampHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the timestamp header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipTimestampHeaderSetStrBadSyntax(
                                  IN RvSipTimestampHeaderHandle          hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgTimestampHeader*   header;
    RvInt32               newStrOffset;
    RvStatus              retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgTimestampHeader*)hHeader;

	if (hPool == header->hPool && hPage == header->hPage)
	{
		/* no need to copy, only to update the offset */
		header->strBadSyntax = strBadSyntaxOffset;
	}
	else
	{
		retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
			                          strBadSyntax, header->hPool, header->hPage, &newStrOffset);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}
		header->strBadSyntax = newStrOffset;
	}

#ifdef SIP_DEBUG
    header->pBadSyntax = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
												header->strBadSyntax);
#endif
    
	return RV_OK;
}

/***************************************************************************
 * SipTimestampHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Timestamp header object.
 ***************************************************************************/
RvInt32 SipTimestampHeaderGetStrBadSyntax(IN  RvSipTimestampHeaderHandle hHeader)
{
    return ((MsgTimestampHeader*)hHeader)->strBadSyntax;
}
/***************************************************************************
 * RvSipTimestampHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Timestamp header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetStrBadSyntax(
                                  IN RvSipTimestampHeaderHandle hHeader,
                                  IN RvChar*					strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipTimestampHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipTimestampHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Timestamp header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTimestampHeader  - Handle to the Timestamp header object.
 *           eStringName       - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside the rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Timestamp header object.
 ***************************************************************************/
/*
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetRpoolString(
                             IN    RvSipTimestampHeaderHandle      hTimestampHeader,
                             IN    RvSipTimestampHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                      *pRpoolPtr)
{
    MsgTimestampHeader* pHeader = (MsgTimestampHeader*)hTimestampHeader;
    RvInt32             requestedParamOffset;

    if(hTimestampHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"RvSipTimestampHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_TIMESTAMP_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipTimestampHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

  if(requestedParamOffset == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(
                                     pHeader->pMsgMgr,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pHeader->hPool,
                                     pHeader->hPage,
                                     requestedParamOffset);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipTimestampHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}
*/
/***************************************************************************
 * RvSipTimestampHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Timestamp header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Timestamp header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                           new string is located.
 ***************************************************************************/
/*
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetRpoolString(
                             IN RvSipTimestampHeaderHandle       hHeader,
                             IN RvSipTimestampHeaderStringName   eStringName,
                             IN RPOOL_Ptr                  *pRpoolPtr)
{
    MsgTimestampHeader* pHeader;

    pHeader = (MsgTimestampHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipTimestampHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
     
	case RVSIP_TIMESTAMP_BAD_SYNTAX:
        return SipTimestampHeaderSetStrBadSyntax(hHeader,
											     NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipTimestampHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

*/
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TimestampHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void TimestampHeaderClean( 
								      IN MsgTimestampHeader* pHeader,
                                      IN RvBool              bCleanBS)
{
	pHeader->eHeaderType = RVSIP_HEADERTYPE_TIMESTAMP;
	


	pHeader->timestampVal.integerPart  = UNDEFINED;
	pHeader->timestampVal.decimalPart  = UNDEFINED;
	pHeader->delayVal.integerPart      = UNDEFINED;
	pHeader->delayVal.decimalPart      = UNDEFINED;
	
	
    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}



#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
