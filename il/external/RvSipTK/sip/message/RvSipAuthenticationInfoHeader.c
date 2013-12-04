/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAuthenticationInfoHeader.c                        *
 *                                                                            *
 * The file defines the methods of the AuthenticationInfo header object:      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Mar 2005                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "RvSipAuthenticationInfoHeader.h"
#include "_SipAuthenticationInfoHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "AddrUrl.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"


#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AuthenticationInfoHeaderClean(
							         IN MsgAuthenticationInfoHeader* pHeader,
                                     IN RvBool                       bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAuthenticationInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an AuthenticationInfo header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed AuthenticationInfo
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAuthenticationInfoHeaderHandle *hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                    stat;

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

    stat = RvSipAuthenticationInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                  msg->hPool,
                                                  msg->hPage,
                                                  hHeader);

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
                              RVSIP_HEADERTYPE_AUTHENTICATION_INFO,
                              &hListElem,
                              (void**)hHeader);
}





/***************************************************************************
 * RvSipAuthenticationInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone AuthenticationInfo Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderConstruct(
                                         IN  RvSipMsgMgrHandle                    hMsgMgr,
                                         IN  HRPOOL                               hPool,
                                         IN  HPAGE                                hPage,
                                         OUT RvSipAuthenticationInfoHeaderHandle *hHeader)
{
    MsgAuthenticationInfoHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    pHeader = (MsgAuthenticationInfoHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                             hPool,
                                                             hPage,
                                                             sizeof(MsgAuthenticationInfoHeader),
                                                             RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthenticationInfoHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    AuthenticationInfoHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAuthenticationInfoHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthenticationInfoHeaderConstruct - AuthenticationInfo header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies the informative field from a source AuthenticationInfo header
 *          to a destination AuthenticationInfo header. The informative field is
 *          the field indicated by the RvSipAuthInfoType enumeration.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AuthenticationInfo header object.
 *    hSource      - Handle to the source AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderCopy(
                                     IN    RvSipAuthenticationInfoHeaderHandle hDestination,
                                     IN    RvSipAuthenticationInfoHeaderHandle hSource)
{
    MsgAuthenticationInfoHeader* source, *dest;

    if ((hDestination == NULL)||(hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgAuthenticationInfoHeader*)hSource;
    dest   = (MsgAuthenticationInfoHeader*)hDestination;

	/* header types */
    dest->eHeaderType = source->eHeaderType;

	/* nonce-count */
	dest->nonceCount = source->nonceCount;
	
	/* message qop */
	dest->eMsgQop = source->eMsgQop;
	if((source->strMsgQop > UNDEFINED) && (dest->eMsgQop == RVSIP_AUTH_QOP_OTHER))
	{
		dest->strMsgQop = MsgUtilsAllocCopyRpoolString(
													source->pMsgMgr,
													dest->hPool,
													dest->hPage,
													source->hPool,
													source->hPage,
													source->strMsgQop);
		if (dest->strMsgQop == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAuthenticationInfoHeaderCopy: Failed to copy qop option. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pMsgQop = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strMsgQop);
#endif
	}
	else
	{
		dest->strMsgQop = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pMsgQop = NULL;
#endif
	}

	/* next-nonce */
	if(source->strNextNonce > UNDEFINED)
	{
		dest->strNextNonce = MsgUtilsAllocCopyRpoolString(
												source->pMsgMgr,
												dest->hPool,
												dest->hPage,
												source->hPool,
												source->hPage,
												source->strNextNonce);
		if (dest->strNextNonce == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAuthenticationInfoHeaderCopy: Failed to copy next-nonce. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pNextNonce = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												  dest->strNextNonce);
#endif
	}
	else
	{
		dest->strNextNonce = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pNextNonce = NULL;
#endif
	}

	/* cnonce */
	if(source->strCNonce > UNDEFINED)
	{
		dest->strCNonce = MsgUtilsAllocCopyRpoolString(
												source->pMsgMgr,
												dest->hPool,
												dest->hPage,
												source->hPool,
												source->hPage,
												source->strCNonce);
		if (dest->strCNonce == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAuthenticationInfoHeaderCopy: Failed to copy cnonce. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pCNonce = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
											   dest->strCNonce);
#endif
	}
	else
	{
		dest->strCNonce = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pCNonce = NULL;
#endif
	}

	/* response-auth */
	if(source->strRspAuth > UNDEFINED)
	{
		dest->strRspAuth = MsgUtilsAllocCopyRpoolString(
												source->pMsgMgr,
												dest->hPool,
												dest->hPage,
												source->hPool,
												source->hPage,
												source->strRspAuth);
		if (dest->strRspAuth == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAuthenticationInfoHeaderCopy: Failed to copy response-auth. hDest 0x%p",
						hDestination));
			return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pRspAuth = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
												dest->strRspAuth);
#endif
	}
	else
	{
		dest->strRspAuth = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pRspAuth = NULL;
#endif
	}

	/* bad-syntax */
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
						"RvSipAuthenticationInfoHeaderCopy: Failed to copy bad-syntax. hDest 0x%p",
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
 * RvSipAuthenticationInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an AuthenticationInfo header object to a textual AuthenticationInfo header. The
 *          textual header is placed on a page taken from a specified pool. In order to copy
 *          the textual header from the page to a consecutive buffer, use
 *          RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the AuthenticationInfo header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderEncode(
                                          IN    RvSipAuthenticationInfoHeaderHandle  hHeader,
                                          IN    HRPOOL                               hPool,
                                          OUT   HPAGE*                               hPage,
                                          OUT   RvUint32*                            length)
{
    RvStatus stat;
    MsgAuthenticationInfoHeader* pHeader;

    pHeader = (MsgAuthenticationInfoHeader*)hHeader;
    if(hPage == NULL || length == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationInfoHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationInfoHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
					new length to the old one */
    stat = AuthenticationInfoHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationInfoHeaderEncode - Failed. Free page 0x%p on pool 0x%p. AuthenticationInfoHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AuthenticationInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          AuthenticationInfo header (as string) on it.
 *          Only the field indicated by eAuthInfoType is encoded
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
  *                   if so, reserved characters should be encoded in escaped
  *                   form, and '=' should be set after header name instead of ':'.
  * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderEncode(
                                      IN    RvSipAuthenticationInfoHeaderHandle  hHeader,
                                      IN    HRPOOL                               hPool,
                                      IN    HPAGE                                hPage,
                                      IN    RvBool                               bInUrlHeaders,
                                      INOUT RvUint32*                            length)
{
    MsgAuthenticationInfoHeader*  pHeader = (MsgAuthenticationInfoHeader*)hHeader;
    RvStatus                      stat;
	RvChar                        tempNc[9];
	RvBool						  bFirstParam = RV_TRUE;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AuthenticationInfoHeaderEncode - Encoding AuthenticationInfo header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

	/* encode "Authentication-Info" */
	stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Authentication-Info", length);
	if(stat != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
			stat, hPool, hPage));
		return stat;
	}

	/* encode ":" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
										length);

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
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
	}

	/* nonce-count */
	if(pHeader->nonceCount > UNDEFINED)
	{
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "nc", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
	
		RvSprintf(tempNc, "%08x", pHeader->nonceCount);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, tempNc, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}

		bFirstParam = RV_FALSE;
	}

	/* message qop */
	if(pHeader->eMsgQop != RVSIP_AUTH_QOP_UNDEFINED)
	{
		if (RV_FALSE == bFirstParam) 
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetCommaChar(bInUrlHeaders),length);
		}

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "qop", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode Authentication Info header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders), length);


		if (pHeader->eMsgQop == RVSIP_AUTH_QOP_OTHER)
		{
			if (pHeader->strMsgQop != UNDEFINED)
			{
				stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
										hPool,
										hPage,
										pHeader->hPool,
										pHeader->hPage,
										pHeader->strMsgQop,
										length);
				if(stat != RV_OK)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							"AuthenticationInfoHeaderEncode - Failed to encode Authentication Info header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
							stat, hPool, hPage));
					return stat;
				}
			}
			else
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header - qop option is undefined. RvStatus is %d, hPool 0x%p, hPage 0x%p",
							stat, hPool, hPage));
				return RV_ERROR_UNKNOWN;
			}
		}
		else
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
												MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
			if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							"AuthenticationInfoHeaderEncode - Failed to encode Authentication Info header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
							stat, hPool, hPage));
				return stat;
			}
			stat = MsgUtilsEncodeQopOptions(pHeader->pMsgMgr, hPool, hPage, pHeader->eMsgQop, length);
			if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
							"AuthenticationInfoHeaderEncode - Failed to encode Authentication Info header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
							stat, hPool, hPage));
				return stat;
			}
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
												MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
			if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
				return stat;
			}
		}

		bFirstParam = RV_FALSE;
	}
	
	/* next-nonce */
	if(pHeader->strNextNonce > UNDEFINED)
	{
		if (RV_FALSE == bFirstParam) 
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetCommaChar(bInUrlHeaders),length);
		}

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "nextnonce", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		
		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strNextNonce,
									length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}

		bFirstParam = RV_FALSE;
	}

	/* cnonce */
	if(pHeader->strCNonce > UNDEFINED)
	{
		if (RV_FALSE == bFirstParam) 
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetCommaChar(bInUrlHeaders),length);
		}

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "cnonce", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
	
		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strCNonce,
									length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}

		bFirstParam = RV_FALSE;
	}
		
	/* response-auth */
	if(pHeader->strRspAuth > UNDEFINED)
	{
		if (RV_FALSE == bFirstParam) 
		{
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetCommaChar(bInUrlHeaders),length);
		}

		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "rspauth", length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
		
		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strRspAuth,
									length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
						stat, hPool, hPage));
			return stat;
		}
	}

	/* Make sure at least one parameter was encoded */
	if (RV_TRUE == bFirstParam) 
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"AuthenticationInfoHeaderEncode - Failed to encode AuthenticationInfo header. At least one parameter must be set for the header to be encoded."));
		return RV_ERROR_UNKNOWN;
	}
	
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Authentication-Info header into an
 *          Authentication-Info header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AuthenticationInfo header object.
 *    buffer    - Buffer containing a textual AuthenticationInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderParse(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar*                              buffer)
{
    MsgAuthenticationInfoHeader* pHeader = (MsgAuthenticationInfoHeader*)hHeader;
    RvStatus					 rv;
    
	if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    AuthenticationInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_AUTHENTICATION_INFO,
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
 * RvSipAuthenticationInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AuthenticationInfo header value into an AuthenticationInfo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAuthenticationInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the AuthenticationInfo header object.
 *    buffer    - The buffer containing a textual AuthenticationInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderParseValue(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar*                              buffer)
{
    MsgAuthenticationInfoHeader*	pHeader = (MsgAuthenticationInfoHeader*)hHeader;
    RvStatus						rv;

    if(hHeader == NULL || (buffer == NULL))
	{
		return RV_ERROR_BADPARAM;
	}

    AuthenticationInfoHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_AUTHENTICATION_INFO,
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
 * RvSipAuthenticationInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AuthenticationInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderFix(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar*                             pFixedBuffer)
{
    MsgAuthenticationInfoHeader*	pHeader = (MsgAuthenticationInfoHeader*)hHeader;
    RvStatus						rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipAuthenticationInfoHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthenticationInfoHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthenticationInfoHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAuthenticationInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AuthenticationInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthenticationInfoHeaderGetPool(RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAuthenticationInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AuthenticationInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthenticationInfoHeaderGetPage(RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AuthenticationInfo header fields are kept in a string format, for
 *          example, the AuthenticationInfo header next-nonce string. In order to get such a field
 *          from the AuthenticationInfo header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AuthenticationInfo header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAuthenticationInfoHeaderGetStringLength(
                                      IN  RvSipAuthenticationInfoHeaderHandle     hHeader,
                                      IN  RvSipAuthenticationInfoHeaderStringName eStringName)
{
    RvInt32                      stringOffset = UNDEFINED;
    MsgAuthenticationInfoHeader* pHeader      = (MsgAuthenticationInfoHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

	switch (eStringName)
    {
        case RVSIP_AUTH_INFO_NEXT_NONCE:
        {
            stringOffset = SipAuthenticationInfoHeaderGetNextNonce(hHeader);
            break;
        }
        case RVSIP_AUTH_INFO_CNONCE:
        {
            stringOffset = SipAuthenticationInfoHeaderGetCNonce(hHeader);
            break;
        }
        case RVSIP_AUTH_INFO_RSP_AUTH:
        {
            stringOffset = SipAuthenticationInfoHeaderGetResponseAuth(hHeader);
            break;
        }
        case RVSIP_AUTH_INFO_MSG_QOP:
        {
            stringOffset = SipAuthenticationInfoHeaderGetStrQop(hHeader);
            break;
        }
        case RVSIP_AUTH_INFO_BAD_SYNTAX:
        {
            stringOffset = SipAuthenticationInfoHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
					"RvSipAuthenticationInfoHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
	
    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * SipAuthenticationInfoHeaderGetNextNonce
 * ------------------------------------------------------------------------
 * General:This method gets the next-nonce string from the AuthenticationInfo header.
 * Return Value: next-nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetNextNonce(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->strNextNonce;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetNextNonce
 * ------------------------------------------------------------------------
 * General: Copies the next-nonce string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetNextNonce(
                                       IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                       IN RvChar*                              strBuffer,
                                       IN RvUint                               bufferLen,
                                       OUT RvUint*                             actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationInfoHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationInfoHeader*)hHeader)->hPage,
                                  SipAuthenticationInfoHeaderGetNextNonce(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderSetNextNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the next-nonce string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AuthenticationInfo header object.
 *       pNextNonce   - The next-nonce to be set in the AuthenticationInfo header - If
 *                      NULL, the exist next-nonce string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetNextNonce(
                                     IN  RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN  RvChar                              *pNextNonce,
                                     IN  HRPOOL                               hPool,
                                     IN  HPAGE                                hPage,
                                     IN  RvInt32                              strOffset)
{
    RvInt32                      newStr;
    MsgAuthenticationInfoHeader *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationInfoHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pNextNonce,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    pHeader->strNextNonce  = newStr;
#ifdef SIP_DEBUG
    pHeader->pNextNonce = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
								                pHeader->strNextNonce);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetNextNonce
 * ------------------------------------------------------------------------
 * General:Sets the next-nonce string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AuthenticationInfo header object.
 *    pNextNonce - The next-nonce string to be set in the AuthenticationInfo header.
 *                 If NULL is supplied, the existing next-nonce is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetNextNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar                              *pNextNonce)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationInfoHeaderSetNextNonce(hHeader, pNextNonce, NULL, NULL_PAGE, UNDEFINED);
}



/***************************************************************************
 * SipAuthenticationInfoHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General:This method gets the CNonce string from the AuthenticationInfo header.
 * Return Value: CNonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetCNonce(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->strCNonce;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General: Copies the cnonce string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetCNonce(
                                       IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                       IN RvChar*							   strBuffer,
                                       IN RvUint							   bufferLen,
                                       OUT RvUint*							   actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationInfoHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationInfoHeader*)hHeader)->hPage,
                                  SipAuthenticationInfoHeaderGetCNonce(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the cnonce string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader   - Handle of the AuthenticationInfo header object.
 *       pCNonce   - The cnonce to be set in the AuthenticationInfo header - If
 *                   NULL, the exist cnonce string in the header will be removed.
 *       strOffset - Offset of a string on the page  (if relevant).
 *       hPool     - The pool on which the string lays (if relevant).
 *       hPage     - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetCNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar                              *pCNonce,
                                     IN HRPOOL                               hPool,
                                     IN HPAGE                                hPage,
                                     IN RvInt32                              strOffset)
{
    RvInt32                      newStr;
    MsgAuthenticationInfoHeader *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationInfoHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pCNonce,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    pHeader->strCNonce = newStr;
#ifdef SIP_DEBUG
    pHeader->pCNonce = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
											  pHeader->strCNonce);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General:Sets the cnonce string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the AuthenticationInfo header object.
 *    pCNonce  - The cnonce string to be set in the AuthenticationInfo header. If a NULL value is
 *             supplied, the existing cnonce string in the header object is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetCNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar *                            pCNonce)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationInfoHeaderSetCNonce(hHeader, pCNonce, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthenticationInfoHeaderGetResponseAuth
 * ------------------------------------------------------------------------
 * General:This method gets the response-auth string from the AuthenticationInfo header.
 * Return Value: response-auth offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetResponseAuth(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->strRspAuth;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetResponseAuth
 * ------------------------------------------------------------------------
 * General: Copies the response-auth string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetResponseAuth(
                                        IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                        IN RvChar*                             strBuffer,
                                        IN RvUint                              bufferLen,
                                        OUT RvUint*                            actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationInfoHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationInfoHeader*)hHeader)->hPage,
                                  SipAuthenticationInfoHeaderGetResponseAuth(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderSetResponseAuth
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the response-auth string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader        - Handle of the AuthenticationInfo header object.
 *       pResponseAuth  - The response-auth to be set in the AuthenticationInfo header - If
 *                        NULL, the exist response-auth string in the header will be removed.
 *       strOffset      - Offset of a string on the page  (if relevant).
 *       hPool			- The pool on which the string lays (if relevant).
 *       hPage			- The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetResponseAuth(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar                             *pResponseAuth,
                                     IN HRPOOL                              hPool,
                                     IN HPAGE                               hPage,
                                     IN RvInt32                             strOffset)
{
    RvInt32                newStr;
    MsgAuthenticationInfoHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationInfoHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pResponseAuth,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    pHeader->strRspAuth = newStr;
#ifdef SIP_DEBUG
    pHeader->pRspAuth = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
							                  pHeader->strRspAuth);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetResponseAuth
 * ------------------------------------------------------------------------
 * General:Sets the response-auth string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the AuthenticationInfo header object.
 *    pResponseAuth  - The opaque string to be set in the AuthenticationInfo header. If a NULL value is
 *                     supplied, the existing response-auth string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetResponseAuth(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar *                            pResponseAuth)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationInfoHeaderSetResponseAuth(hHeader, pResponseAuth, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetNonceCount
 * ------------------------------------------------------------------------
 * General: Gets the nonce count value from the AuthenticationInfo Header object.
 * Return Value: Returns the nonce count value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAuthenticationInfoHeaderGetNonceCount(
                                    IN RvSipAuthenticationInfoHeaderHandle hSipAuthInfoHeader)
{
    if(hSipAuthInfoHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgAuthenticationInfoHeader*)hSipAuthInfoHeader)->nonceCount;
}


/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetNonceCount
 * ------------------------------------------------------------------------
 * General: Sets the nonce count value in the AuthenticationInfo Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthInfoHeader	- Handle to the AuthenticationInfo header object.
 *    nonceCount			- The nonce count value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetNonceCount(
                                 IN    RvSipAuthenticationInfoHeaderHandle hSipAuthInfoHeader,
                                 IN    RvInt32                             nonceCount)
{
	MsgAuthenticationInfoHeader *pHeader = (MsgAuthenticationInfoHeader*)hSipAuthInfoHeader;

    if(hSipAuthInfoHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    pHeader->nonceCount = nonceCount;
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetQopOption
 * ------------------------------------------------------------------------
 * General: Gets the Qop option enumeration from the AuthenticationInfo object.
 * Return Value: The qop type from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Returns the Qop option enumeration from the object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipAuthenticationInfoHeaderGetQopOption(
                                      IN  RvSipAuthenticationInfoHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_AUTH_QOP_UNDEFINED;
	}

    return ((MsgAuthenticationInfoHeader*)hHeader)->eMsgQop;
}

/***************************************************************************
 * SipAuthenticationInfoHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General:This method gets the qop string from the authentication-info header.
 * Return Value: qop or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetStrQop(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->strMsgQop;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General: Copies the Qop string value of the Authentication-Info object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication-Info header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetStrQop(
                                       IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                       IN RvChar*                             strBuffer,
                                       IN RvUint                              bufferLen,
                                       OUT RvUint*                            actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationInfoHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationInfoHeader*)hHeader)->hPage,
                                  SipAuthenticationInfoHeaderGetStrQop(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderSetStrQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the qop string in the
 *          Authentication-Info Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Authentication-Info header object.
 *       eQop     - The qop option enumeration value to be set in the object.
 *       pQop    - The qop to be set in the Authentication-Info header - If
 *                 NULL, the exist qop string in the header will be removed.
 *       offset  - Offset of a string on the page (if relevant).
 *       hPool   - The pool on which the string lays (if relevant).
 *       hPage   - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetStrQop(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvSipAuthQopOption                  eQop,
									 IN RvChar                             *pQop,
                                     IN HRPOOL                              hPool,
                                     IN HPAGE                               hPage,
                                     IN RvInt32                             offset)
{
    RvInt32                      newStr;
    MsgAuthenticationInfoHeader *pHeader;
    RvStatus                     retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationInfoHeader*)hHeader;

	if (eQop == RVSIP_AUTH_QOP_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pQop, pHeader->hPool,
									  pHeader->hPage, &newStr);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}


		pHeader->strMsgQop     = newStr;
#ifdef SIP_DEBUG
		pHeader->pMsgQop = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
												  pHeader->strMsgQop);
#endif
	}
	else
	{
		pHeader->strMsgQop     = UNDEFINED;
#ifdef SIP_DEBUG
		pHeader->pMsgQop       = NULL;
#endif
	}

	pHeader->eMsgQop       = eQop;

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetQopOption
 * ------------------------------------------------------------------------
 * General:Sets the Qop string in the Authentication-Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication-Info header object.
 *    eQop     - The qop option enumeration value to be set in the object.
 *    strQop   - You can use this parameter only if the eQop parameter is set to
 *               RVSIP_AUTH_QOP_OTHER. In this case you can supply the qop
 *               option as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetQopOption(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvSipAuthQopOption                  eQop,
									 IN RvChar *                            strQop)
{
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	if (eQop == RVSIP_AUTH_QOP_AUTH_AND_AUTHINT)
	{
		return RV_ERROR_BADPARAM;
	}

	return SipAuthenticationInfoHeaderSetStrQop(hHeader, eQop, strQop, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object.
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetStrBadSyntax(
                                            IN  RvSipAuthenticationInfoHeaderHandle hHeader)
{
    return ((MsgAuthenticationInfoHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AuthenticationInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetStrBadSyntax(
                                     IN  RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationInfoHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationInfoHeader*)hHeader)->hPage,
                                  SipAuthenticationInfoHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationInfoHeaderSetStrBadSyntax
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
RvStatus SipAuthenticationInfoHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset)
{
    MsgAuthenticationInfoHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgAuthenticationInfoHeader*)hHeader;

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
 * RvSipAuthenticationInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AuthenticationInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationInfoHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the AuthenticationInfo header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 *           eStringName    - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetRpoolString(
                             IN    RvSipAuthenticationInfoHeaderHandle      hSipAuthInfoHeader,
                             IN    RvSipAuthenticationInfoHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgAuthenticationInfoHeader* pHeader = (MsgAuthenticationInfoHeader*)hSipAuthInfoHeader;
    RvInt32                      requestedParamOffset;

    if(hSipAuthInfoHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthenticationInfoHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

	switch(eStringName)
    {
    case RVSIP_AUTH_INFO_NEXT_NONCE:
        requestedParamOffset = pHeader->strNextNonce;
        break;
    case RVSIP_AUTH_INFO_CNONCE:
        requestedParamOffset = pHeader->strCNonce;
        break;
    case RVSIP_AUTH_INFO_RSP_AUTH:
        requestedParamOffset = pHeader->strRspAuth;
        break;
    case RVSIP_AUTH_INFO_MSG_QOP:
        requestedParamOffset = pHeader->strMsgQop;
        break;
    case RVSIP_AUTH_INFO_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthenticationInfoHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
    /* the parameter does not exit in the header - return UNDEFINED*/
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
                  "RvSipAuthenticationInfoHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the AuthenticationInfo header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetRpoolString(
                             IN RvSipAuthenticationInfoHeaderHandle      hSipAuthInfoHeader,
                             IN RvSipAuthenticationInfoHeaderStringName  eStringName,
                             IN RPOOL_Ptr                                *pRpoolPtr)
{
    MsgAuthenticationInfoHeader* pHeader;

    pHeader = (MsgAuthenticationInfoHeader*)hSipAuthInfoHeader;
    if(hSipAuthInfoHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipAuthenticationInfoHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_AUTH_INFO_NEXT_NONCE:
        return SipAuthenticationInfoHeaderSetNextNonce(
											   hSipAuthInfoHeader,
											   NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTH_INFO_CNONCE:
        return SipAuthenticationInfoHeaderSetCNonce(
												 hSipAuthInfoHeader,
												 NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);
    case RVSIP_AUTH_INFO_RSP_AUTH:
        return SipAuthenticationInfoHeaderSetResponseAuth(
											  hSipAuthInfoHeader,
											  NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    case RVSIP_AUTH_INFO_MSG_QOP:
        return SipAuthenticationInfoHeaderSetStrQop(
											  hSipAuthInfoHeader,
											  RVSIP_AUTH_QOP_OTHER,
											  NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
	case RVSIP_AUTH_INFO_BAD_SYNTAX:
        return SipAuthenticationInfoHeaderSetStrBadSyntax(
											  hSipAuthInfoHeader,
											  NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationInfoHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AuthenticationInfoHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *    bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AuthenticationInfoHeaderClean(
								      IN MsgAuthenticationInfoHeader* pHeader,
                                      IN RvBool                       bCleanBS)
{
	pHeader->nonceCount     = UNDEFINED;
	pHeader->strNextNonce   = UNDEFINED;
	pHeader->strCNonce      = UNDEFINED;
	pHeader->strRspAuth     = UNDEFINED;
	pHeader->strMsgQop      = UNDEFINED;
	pHeader->eMsgQop        = RVSIP_AUTH_QOP_UNDEFINED;
	pHeader->eHeaderType    = RVSIP_HEADERTYPE_AUTHENTICATION_INFO;

#ifdef SIP_DEBUG
	pHeader->pMsgQop        = NULL;
	pHeader->pNextNonce     = NULL;
	pHeader->pCNonce        = NULL;
	pHeader->pRspAuth       = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_AUTH_ON */

