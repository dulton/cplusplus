/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************************
 *                  RvSipPDCSLAESHeader.c											*
 *																					*
 * The file defines the methods of the PDCSLAES header object:						*	
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

#include "RvSipPDCSLAESHeader.h"
#include "_SipPDCSLAESHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PDCSLAESHeaderClean(IN MsgPDCSLAESHeader* pHeader,
								IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSLAESHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSLAES header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSLAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderConstructInMsg(
                                       IN  RvSipMsgHandle				hSipMsg,
                                       IN  RvBool						pushHeaderAtHead,
                                       OUT RvSipPDCSLAESHeaderHandle*	hHeader)
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

    stat = RvSipPDCSLAESHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_DCS_LAES,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPDCSLAESHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSLAES Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSLAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderConstruct(
                                           IN  RvSipMsgMgrHandle			hMsgMgr,
                                           IN  HRPOOL						hPool,
                                           IN  HPAGE						hPage,
                                           OUT RvSipPDCSLAESHeaderHandle*	hHeader)
{
    MsgPDCSLAESHeader*   pHeader;
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

    pHeader = (MsgPDCSLAESHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPDCSLAESHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSLAESHeaderConstruct - Failed to construct PDCSLAES header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PDCSLAESHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPDCSLAESHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPDCSLAESHeaderConstruct - PDCSLAES header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSLAES header 
 *			object to a destination PDCSLAES header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSLAES header object.
 *    hSource      - Handle to the destination PDCSLAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderCopy(
                                         INOUT	RvSipPDCSLAESHeaderHandle hDestination,
                                         IN		RvSipPDCSLAESHeaderHandle hSource)
{
    MsgPDCSLAESHeader*   source;
    MsgPDCSLAESHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgPDCSLAESHeader*)hSource;
    dest   = (MsgPDCSLAESHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* strLaesSig */
	if(source->strLaesSigHost > UNDEFINED)
    {
        dest->strLaesSigHost = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
														  dest->hPool,
														  dest->hPage,
														  source->hPool,
														  source->hPage,
														  source->strLaesSigHost);
        if(dest->strLaesSigHost == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderCopy - Failed to copy strLaesSigHost"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pLaesSigHost */
#ifdef SIP_DEBUG
        dest->pLaesSigHost = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strLaesSigHost);
#endif
    }
    else
    {
        dest->strLaesSigHost = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pLaesSigHost = NULL;
#endif
    }
	
	/* laesSigPort */
	dest->laesSigPort = source->laesSigPort;
	
	/* strLaesContentHost */
	if(source->strLaesContentHost > UNDEFINED)
    {
        dest->strLaesContentHost = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strLaesContentHost);
        if(dest->strLaesContentHost == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderCopy - Failed to copy strLaesContentHost"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pLaesContentHost */
#ifdef SIP_DEBUG
        dest->pLaesContentHost = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strLaesContentHost);
#endif
    }
    else
    {
        dest->strLaesContentHost = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pLaesContentHost = NULL;
#endif
    }

	/* laesContentPort */
	dest->laesContentPort = source->laesContentPort;
	
	/* strLaesKey */
	if(source->strLaesKey > UNDEFINED)
    {
        dest->strLaesKey = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strLaesKey);
        if(dest->strLaesKey == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderCopy - Failed to copy strLaesKey"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pLaesKey */
#ifdef SIP_DEBUG
        dest->pLaesKey = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strLaesKey);
#endif
    }
    else
    {
        dest->strLaesKey = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pLaesKey = NULL;
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
                      "RvSipPDCSLAESHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPDCSLAESHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPDCSLAESHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSLAES header object to a textual PDCSLAES header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSLAES header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderEncode(
                              IN    RvSipPDCSLAESHeaderHandle	hHeader,
                              IN    HRPOOL						hPool,
                              OUT   HPAGE*						hPage,
                              OUT   RvUint32*					length)
{
    RvStatus			stat;
    MsgPDCSLAESHeader*	pHeader;

    pHeader = (MsgPDCSLAESHeader*)hHeader;

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
                "RvSipPDCSLAESHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PDCSLAESHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PDCSLAESHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PDCSLAESHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PDCSLAES header (as string) on it.
 *          format: "P-DCS-LAES: "
 * Return Value: RV_OK				- If succeeded.
 *      RV_ERROR_OUTOFRESOURCES		- If allocation failed (no resources)
 *      RV_ERROR_UNKNOWN			- In case of a failure.
 *      RV_ERROR_BADPARAM			- If hHeader or hPage are NULL or no method
 *										or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PDCSLAES header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PDCSLAESHeaderEncode(
                                  IN    RvSipPDCSLAESHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPDCSLAESHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgPDCSLAESHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PDCSLAESHeaderEncode - Encoding PDCSLAES header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-DCS-LAES" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-DCS-LAES", length);
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSLAESHeaderEncode - Failed to encoding PDCSLAES header. stat is %d",
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
                "PDCSLAESHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

	/* LaesSig */
	if(pHeader->strLaesSigHost > UNDEFINED)
    {
		stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strLaesSigHost,
                                    length);

		if(pHeader->laesSigPort > UNDEFINED)
		{
			RvChar strHelper[16];
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetColonChar(bInUrlHeaders), length);
			/* set the port string */
			MsgUtils_itoa(pHeader->laesSigPort, strHelper);
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
												strHelper, length);
		}

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSLAESHeaderEncode - Failed to encode PDCSLAESHeaderEncode header. stat is %d",
				stat));
			return stat;
		}
    }
    else
    {
        /* This is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PDCSLAESHeaderEncode: Failed. No LaesSig was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
	
    /* LaesContentHost (insert ";" in the beginning) */
    if(pHeader->strLaesContentHost > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		/* set "content=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "content", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		/* set the LaesContentHost string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strLaesContentHost,
                                    length);
		
		if(pHeader->laesContentPort > UNDEFINED)
		{
			RvChar strHelper[16];
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetColonChar(bInUrlHeaders), length);
			/* set the port string */
			MsgUtils_itoa(pHeader->laesContentPort, strHelper);
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
												strHelper, length);
		}
    }

	/* LaesKey (insert ";" in the beginning) */
    if(pHeader->strLaesKey > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSLAESHeaderEncode - Failed to encode Semicolon character. stat is %d",
				stat));
		}

		/* set "key=" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "key", length);

		
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSLAESHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the LaesKey string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strLaesKey,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"PDCSLAESHeaderEncode - Failed to encode LaesKey String. stat is %d",
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
            "PDCSLAESHeaderEncode - Failed to encoding PDCSLAES header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSLAES header-for example,
 *          "PDCSLAES:172.20.5.3:5060"-into a PDCSLAES header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSLAES header object.
 *    buffer    - Buffer containing a textual PDCSLAES header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderParse(
                             IN    RvSipPDCSLAESHeaderHandle	hHeader,
                             IN    RvChar*						buffer)
{
    MsgPDCSLAESHeader*	pHeader = (MsgPDCSLAESHeader*)hHeader;
	RvStatus			rv;
    
	if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PDCSLAESHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_LAES,
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
 * RvSipPDCSLAESHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSLAES header value into an PDCSLAES header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSLAESHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSLAES header object.
 *    buffer    - The buffer containing a textual PDCSLAES header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderParseValue(
                                 IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                 IN    RvChar*						buffer)
{
    MsgPDCSLAESHeader*	pHeader = (MsgPDCSLAESHeader*)hHeader;
	RvStatus			rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}


    PDCSLAESHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_DCS_LAES,
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
 * RvSipPDCSLAESHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSLAES header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderFix(
                                     IN RvSipPDCSLAESHeaderHandle	hHeader,
                                     IN RvChar*						pFixedBuffer)
{
    MsgPDCSLAESHeader*	pHeader = (MsgPDCSLAESHeader*)hHeader;
    RvStatus			rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPDCSLAESHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSLAESHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPDCSLAESHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPDCSLAESHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSLAES object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPDCSLAESHeaderGetPool(RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPDCSLAESHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSLAES object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPDCSLAESHeaderGetPage(RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSLAES header fields are kept in a string format-for example, the
 *          PDCSLAES header LaesSig name. In order to get such a field from the PDCSLAES header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSLAES header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSLAESHeaderGetStringLength(
                                      IN  RvSipPDCSLAESHeaderHandle     hHeader,
                                      IN  RvSipPDCSLAESHeaderStringName stringName)
{
    RvInt32				stringOffset;
    MsgPDCSLAESHeader*	pHeader = (MsgPDCSLAESHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_DCS_LAES_SIG_HOST:
        {
            stringOffset = SipPDCSLAESHeaderGetStrLaesSigHost(hHeader);
            break;
        }
		case RVSIP_P_DCS_LAES_CONTENT_HOST:
        {
            stringOffset = SipPDCSLAESHeaderGetStrLaesContentHost(hHeader);
            break;
        }
		case RVSIP_P_DCS_LAES_KEY:
        {
            stringOffset = SipPDCSLAESHeaderGetStrLaesKey(hHeader);
            break;
        }
        case RVSIP_P_DCS_LAES_OTHER_PARAMS:
        {
            stringOffset = SipPDCSLAESHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_DCS_LAES_BAD_SYNTAX:
        {
            stringOffset = SipPDCSLAESHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPDCSLAESHeaderGetStringLength - Unknown stringName %d", stringName));
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
 * SipPDCSLAESHeaderGetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General:This method gets the LaesSig in the PDCSLAES header object.
 * Return Value: LaesSig offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesSigHost(IN RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->strLaesSigHost;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesSig field of the PDCSLAES header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesSigHost(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSLAESHeader*)hHeader)->hPool,
                                  ((MsgPDCSLAESHeader*)hHeader)->hPage,
                                  SipPDCSLAESHeaderGetStrLaesSigHost(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesSig in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesSigHost - The LaesSig to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesSig string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesSigHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *					pLaesSigHost,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32					strOffset)
{
    RvInt32				newStr;
    MsgPDCSLAESHeader*	pHeader;
    RvStatus			retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSLAESHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pLaesSigHost,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strLaesSigHost = newStr;
#ifdef SIP_DEBUG
    pHeader->pLaesSigHost = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strLaesSigHost);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General:Sets the LaesSig field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strLaesSig - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesSigHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesSigHost)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSLAESHeaderSetStrLaesSigHost(hHeader, pLaesSigHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetLaesSigPort
 * ------------------------------------------------------------------------
 * General: Gets port parameter from the PDCSLAES header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSLAESHeaderGetLaesSigPort(
                                         IN RvSipPDCSLAESHeaderHandle hPDCSLAESHeader)
{
    if(hPDCSLAESHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgPDCSLAESHeader*)hPDCSLAESHeader)->laesSigPort;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderSetLaesSigPort
 * ------------------------------------------------------------------------
 * General:  Sets port parameter of the PDCSLAES header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 *    portNum         - The port number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetLaesSigPort(
                                         IN    RvSipPDCSLAESHeaderHandle hPDCSLAESHeader,
                                         IN    RvInt32					 portNum)
{
    if(hPDCSLAESHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgPDCSLAESHeader*)hPDCSLAESHeader)->laesSigPort = portNum;
    return RV_OK;
}

/***************************************************************************
 * SipPDCSLAESHeaderGetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General:This method gets the LaesContentHost in the PDCSLAES header object.
 * Return Value: LaesContentHost offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesContentHost(
                                            IN RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->strLaesContentHost;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesContentHost field of the PDCSLAES
			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesContentHost(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSLAESHeader*)hHeader)->hPool,
                                  ((MsgPDCSLAESHeader*)hHeader)->hPage,
                                  SipPDCSLAESHeaderGetStrLaesContentHost(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesContentHost in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesContentHost - The LaesContentHost to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesContentHost string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesContentHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesContentHost,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSLAESHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSLAESHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pLaesContentHost,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strLaesContentHost = newStr;
#ifdef SIP_DEBUG
    pHeader->pLaesContentHost = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strLaesContentHost);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General:Sets the LaesContentHost field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesContentHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesContentHost)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSLAESHeaderSetStrLaesContentHost(hHeader, pLaesContentHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetLaesContentPort
 * ------------------------------------------------------------------------
 * General: Gets LaesContentPort parameter from the PDCSLAES header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSLAESHeaderGetLaesContentPort(
                                         IN RvSipPDCSLAESHeaderHandle hPDCSLAESHeader)
{
    if(hPDCSLAESHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgPDCSLAESHeader*)hPDCSLAESHeader)->laesContentPort;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderSetLaesContentPort
 * ------------------------------------------------------------------------
 * General:  Sets LaesContentPort parameter of the PDCSLAES header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 *    portNum         - The port number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetLaesContentPort(
                                         IN    RvSipPDCSLAESHeaderHandle hPDCSLAESHeader,
                                         IN    RvInt32					 portNum)
{
    if(hPDCSLAESHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgPDCSLAESHeader*)hPDCSLAESHeader)->laesContentPort = portNum;
    return RV_OK;
}

/***************************************************************************
 * SipPDCSLAESHeaderGetStrLaesKey
 * ------------------------------------------------------------------------
 * General:This method gets the LaesKey in the PDCSLAES header object.
 * Return Value: LaesKey offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesKey(IN RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->strLaesKey;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesKey
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesSig field of the PDCSLAES header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesKey(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSLAESHeader*)hHeader)->hPool,
                                  ((MsgPDCSLAESHeader*)hHeader)->hPage,
                                  SipPDCSLAESHeaderGetStrLaesKey(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesKey in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesSigHost - The LaesSig to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesSig string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesKey(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesKey,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSLAESHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSLAESHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pLaesKey,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strLaesKey = newStr;
#ifdef SIP_DEBUG
    pHeader->pLaesKey = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strLaesKey);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesKey
 * ------------------------------------------------------------------------
 * General:Sets the LaesKey field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strLaesSig - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesKey(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesKey)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSLAESHeaderSetStrLaesKey(hHeader, pLaesKey, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPDCSLAESHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PDCSLAES header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetOtherParams(
                                        IN RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header other params field of the PDCSLAES header object into a
 *          given buffer.
 *          Not all the PDCSLAES header parameters have separated fields in the PDCSLAES
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetOtherParams(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSLAESHeader*)hHeader)->hPool,
                                  ((MsgPDCSLAESHeader*)hHeader)->hPage,
                                  SipPDCSLAESHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSLAESHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pOtherParams - The Other Params to be set in the PDCSLAES header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetOtherParams(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPDCSLAESHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPDCSLAESHeader*) hHeader;

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
 * RvSipPDCSLAESHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSLAES header object.
 *         Not all the PDCSLAES header parameters have separated fields in the PDCSLAES
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strOtherParams - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetOtherParams(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSLAESHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipPDCSLAESHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrBadSyntax(
                                    IN RvSipPDCSLAESHeaderHandle hHeader)
{
    return ((MsgPDCSLAESHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSLAES header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrBadSyntax(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPDCSLAESHeader*)hHeader)->hPool,
                                  ((MsgPDCSLAESHeader*)hHeader)->hPage,
                                  SipPDCSLAESHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPDCSLAESHeaderSetStrBadSyntax
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
RvStatus SipPDCSLAESHeaderSetStrBadSyntax(
                                  IN RvSipPDCSLAESHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPDCSLAESHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPDCSLAESHeader*)hHeader;

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
 * RvSipPDCSLAESHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSLAES header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrBadSyntax(
                                  IN RvSipPDCSLAESHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPDCSLAESHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PDCSLAESHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PDCSLAESHeaderClean( IN MsgPDCSLAESHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_DCS_LAES;
    pHeader->strOtherParams		= UNDEFINED;
	pHeader->strLaesSigHost		= UNDEFINED;
	pHeader->laesSigPort		= UNDEFINED;
	pHeader->strLaesContentHost	= UNDEFINED;
	pHeader->laesContentPort	= UNDEFINED;
	pHeader->strLaesKey			= UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pOtherParams		= NULL;
	pHeader->pLaesSigHost		= NULL;
	pHeader->pLaesContentHost		= NULL;
	pHeader->pLaesKey			= NULL;
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

