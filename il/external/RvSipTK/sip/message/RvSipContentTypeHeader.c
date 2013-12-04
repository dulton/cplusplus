/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipContentTypeHeader.c                               *
 *                                                                            *
 * The file defines the methods of the Content-Type header object             *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Aug 2001                                              *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipContentTypeHeader.h"
#include "_SipContentTypeHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipBody.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ContentTypeHeaderClean(IN MsgContentTypeHeader*  pHeader,
								   IN RvBool                 bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipContentTypeHeaderConstructInBody
 * ------------------------------------------------------------------------
 * General: Constructs a Content-type header object inside a given message
 *          body object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBody    - Handle to the message body object.
 * output: hpHeader - Handle to the newly constructed Content-Type header
 *                    object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderConstructInBody(
                               IN  RvSipBodyHandle               hBody,
                               OUT RvSipContentTypeHeaderHandle* phHeader)

{

    Body*           pBody;
    RvStatus       stat;

    if(hBody == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pBody = (Body*)hBody;

    stat = RvSipContentTypeHeaderConstruct((RvSipMsgMgrHandle)pBody->pMsgMgr,
                                            pBody->hPool, pBody->hPage, phHeader);
    if(stat != RV_OK)
        return stat;

    /* attach the header in the msg */
    pBody->pContentType = (MsgContentTypeHeader *)*phHeader;

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Content-Type Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: phHeader - Handle to the newly constructed Content-Type header
 *                    object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderConstruct(
                                    IN  RvSipMsgMgrHandle             hMsgMgr,
                                    IN  HRPOOL                        hPool,
                                    IN  HPAGE                         hPage,
                                    OUT RvSipContentTypeHeaderHandle* phHeader)
{
    MsgContentTypeHeader*  pHeader;
    MsgMgr*                pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (NULL == phHeader) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    *phHeader = NULL;

    /* allocate the header */
    pHeader = (MsgContentTypeHeader*)SipMsgUtilsAllocAlign(
                                                  pMsgMgr,
                                                  hPool,
                                                  hPage,
                                                  sizeof(MsgContentTypeHeader),
                                                  RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipContentTypeHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr         = pMsgMgr;
    pHeader->hPage           = hPage;
    pHeader->hPool           = hPool;

    ContentTypeHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipContentTypeHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipContentTypeHeaderConstruct - Content-Type header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, phHeader=0x%p)",
              hMsgMgr, hPool, hPage, *phHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Content-Type header object to
 *          a destination Content-Type header object.
 *          You must construct the destination object before using this
 *          function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Content-Type header object.
 *    hSource      - Handle to the source Content-Type header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderCopy(
                             INOUT RvSipContentTypeHeaderHandle hDestination,
                             IN    RvSipContentTypeHeaderHandle hSource)
{
    MsgContentTypeHeader* source;
    MsgContentTypeHeader* dest;
	RvStatus              stat;
	

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgContentTypeHeader*)hSource;
    dest   = (MsgContentTypeHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* media type and sub type */
    dest->eMediaSubType = source->eMediaSubType;
    dest->eMediaType    = source->eMediaType;
    dest->bIsCompact    = source->bIsCompact;
	/* multipart/related parameters */ 
	dest->eTypeParamMediaType	 = source->eTypeParamMediaType;
	dest->eTypeParamMediaSubType = source->eTypeParamMediaSubType; 

    /* media type string */
    if  ((source->eMediaType == RVSIP_MEDIATYPE_OTHER) &&
         (source->strMediaType > UNDEFINED))
    {
        dest->strMediaType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                             dest->hPool,
                                                             dest->hPage,
                                                             source->hPool,
                                                             source->hPage,
                                                             source->strMediaType);
        if (dest->strMediaType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderCopy - Failed to copy media type. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pMediaType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                            dest->strMediaType);
#endif
    }
    else
    {
        dest->strMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMediaType = NULL;
#endif
    }

    /* media sub type string */
    if((source->eMediaSubType == RVSIP_MEDIASUBTYPE_OTHER) &&
       (source->strMediaSubType > UNDEFINED))
    {
        dest->strMediaSubType = MsgUtilsAllocCopyRpoolString(
                                                          source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strMediaSubType);
        if (dest->strMediaSubType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderCopy - Failed to copy media sub type. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pMediaSubType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                               dest->strMediaSubType);
#endif
    }
    else
    {
        dest->strMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMediaSubType = NULL;
#endif
    }

    /* boundary string */
    if(source->strBoundary > UNDEFINED)
    {
        dest->strBoundary = MsgUtilsAllocCopyRpoolString(
                                                          source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strBoundary);
        if (dest->strBoundary == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderCopy - Failed to copy boundary. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pBoundary = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strBoundary);
#endif
    }
    else
    {
        dest->strBoundary = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBoundary = NULL;
#endif
    }

    /* version string */
    if(source->strVersion > UNDEFINED)
    {
        dest->strVersion = MsgUtilsAllocCopyRpoolString(
                                                          source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strVersion);
        if (dest->strVersion == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderCopy - Failed to copy version. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pVersion = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strVersion);
#endif
    }
    else
    {
        dest->strVersion = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pVersion = NULL;
#endif
    }

    /* base string */
    if(source->strBase > UNDEFINED)
    {
        dest->strBase = MsgUtilsAllocCopyRpoolString(  source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strBase);
        if (dest->strBase == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderCopy - Failed to copy base. hDest 0x%p",
                      hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
            dest->pBase = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                       dest->strBase);
#endif
    }
    else
    {
        dest->strBase = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBase = NULL;
#endif
    }

	/*------------------------------*/
	/* multipart/related parameters */
	/*------------------------------*/
	/* type parameter's media type string */
    if  ((source->eTypeParamMediaType == RVSIP_MEDIATYPE_OTHER) &&
		(source->strTypeParamMediaType > UNDEFINED))
    {
        dest->strTypeParamMediaType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
			dest->hPool,
			dest->hPage,
			source->hPool,
			source->hPage,
			source->strTypeParamMediaType);
        if (dest->strTypeParamMediaType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
				"RvSipContentTypeHeaderCopy - Failed to copy media type of 'type' parameter. hDest 0x%p",
				hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
		dest->pTypeParamMediaType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
			dest->strTypeParamMediaType);
#endif
    }
    else
    {
        dest->strTypeParamMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTypeParamMediaType = NULL;
#endif
    }
	
    /* type parameter's media sub type string */
    if((source->eTypeParamMediaSubType == RVSIP_MEDIASUBTYPE_OTHER) &&
		(source->strTypeParamMediaSubType > UNDEFINED))
    {
        dest->strTypeParamMediaSubType = MsgUtilsAllocCopyRpoolString(
			source->pMsgMgr,
			dest->hPool,
			dest->hPage,
			source->hPool,
			source->hPage,
			source->strTypeParamMediaSubType);
        if (dest->strTypeParamMediaSubType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
				"RvSipContentTypeHeaderCopy - Failed to copy media sub type of 'type' parameter. hDest 0x%p",
				hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
		dest->pTypeParamMediaSubType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
			dest->strTypeParamMediaSubType);
#endif
    }
    else
    {
        dest->strTypeParamMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTypeParamMediaSubType = NULL;
#endif
    }
    /* Start Addr Spec */
    if (NULL != source->hStartAddrSpec)
    {
        stat = RvSipContentTypeHeaderSetStart(hDestination, source->hStartAddrSpec);
        if (stat != RV_OK)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContentTypeHeaderCopy - Failed to copy address spec. hDest 0x%p, stat %d",
                hDestination, stat));
            return stat;
        }
    }

    /* otherParams */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strOtherParams);
        if (dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContentTypeHeaderCopy - Failed to copy other param. stat %d",
                RV_ERROR_OUTOFRESOURCES));
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
                "RvSipContentTypeHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipContentTypeHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Content-Type header object to a textual Content-Type
 *          header. The textual header is placed on a page taken from a
 *          specified pool. In order to copy the textual header from the
 *          page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Content-Type header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderEncode(
                                  IN   RvSipContentTypeHeaderHandle hHeader,
                                  IN   HRPOOL                       hPool,
                                  OUT  HPAGE*                       hPage,
                                  OUT  RvUint32*                   length)
{
    RvStatus stat;
    MsgContentTypeHeader* pHeader;

    pHeader = (MsgContentTypeHeader*)hHeader;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0; /* so length will give the real length, and will not just add the
                    new length to the old one */

    if ((hPool == NULL) || (hHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentTypeHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentTypeHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *hPage, hPool, hHeader));
    }
    stat = ContentTypeHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, RV_FALSE,length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentTypeHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ContentTypeHeaderEncode fail",
                  *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * ContentTypeHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-Type header (as string) on it.
 *          Content-Type = "Content-Type:" type "/" subtype (;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Type header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderEncode(
                                  IN    RvSipContentTypeHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bInUrlHeaders,
                                  IN    RvBool                       bForceCompactForm,
                                  INOUT RvUint32*                    length)
{
    MsgContentTypeHeader*   pHeader;
    RvStatus               stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgContentTypeHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "ContentTypeHeaderEncode - Encoding Content-Type header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
              hHeader, hPool, hPage));

    /* set "Content-Type" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "c", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Content-Type", length);
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContentTypeHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
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
                "ContentTypeHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* set type "/" subtype */
    stat = MsgUtilsEncodeMediaType(pHeader->pMsgMgr, hPool, hPage, pHeader->eMediaType,
                                   pHeader->hPool, pHeader->hPage,
                                   pHeader->strMediaType, length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
                  stat));
        return stat;
    }
    /* / */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,  "/", length);
    stat = MsgUtilsEncodeMediaSubType(pHeader->pMsgMgr, hPool, hPage, pHeader->eMediaSubType,
                                      pHeader->hPool, pHeader->hPage,
                                      pHeader->strMediaSubType, length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
                  stat));
        return stat;
    }

    /* Content-Type parameters with ";" between them */
    /* boundary */
    if(pHeader->strBoundary > UNDEFINED)
    {
        /* set ";boundary=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                     MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "boundary", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        /* = */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* set pHeader->boundary */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBoundary,
                                    length);

    }
    /* ;version=" */
    if(pHeader->strVersion > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "version", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set pHeader->boundary */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strVersion,
                                    length);
    }
    /* ;base= */
    if(pHeader->strBase > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "base", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        /* set pHeader->boundary */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBase,
                                    length);
    }

	/*------------------------------*/
	/* multipart/related parameters */
	/*------------------------------*/
	/* ;type= type "/" subtype */
	if (pHeader->eTypeParamMediaType != RVSIP_MEDIATYPE_UNDEFINED)
	{
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "type", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		stat = MsgUtilsEncodeMediaType(pHeader->pMsgMgr, hPool, hPage, pHeader->eTypeParamMediaType,
			pHeader->hPool, pHeader->hPage,
			pHeader->strTypeParamMediaType, length);
		if (RV_OK != stat)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
				stat));
			return stat;
		}

        if (pHeader->eTypeParamMediaSubType != RVSIP_MEDIASUBTYPE_UNDEFINED)
	    {
		    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,  "/", length);
		    stat = MsgUtilsEncodeMediaSubType(pHeader->pMsgMgr, hPool, hPage, pHeader->eTypeParamMediaSubType,
			    pHeader->hPool, pHeader->hPage,
			    pHeader->strTypeParamMediaSubType, length);
		    if (RV_OK != stat)
		    {
			    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				    "ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
				    stat));
			    return stat;
		    }
	    }
        else
        {
            stat = RV_ERROR_ILLEGAL_ACTION;
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				    "ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
				    stat));
		    return stat;
        }
  	}
  
	/* ;start=" */
    if(pHeader->hStartAddrSpec != NULL)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
			MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "start", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
			MsgUtilsGetEqualChar(bInUrlHeaders), length);
		
        /* set pHeader->start */
        stat = AddrEncode(pHeader->hStartAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (RV_OK != stat)
		{
            return stat;
		}
	}

    /* other parameters */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set pHeader->strOtherParams */
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
                  "ContentTypeHeaderEncode: Failed to encode Content-Type header. stat is %d",
                  stat));
    }
    return stat;
}

/***************************************************************************
 * ContentTypeHeaderConsecutiveEncode
 * General: Accepts a consecutive buffer and encodes the Content-Type header
 *          as string on the buffer.
 *          Content-Type = "Content-Type:" type "/" subtype (;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Type header object.
 *        strBuffer - Buffer to fill with the encoded header.
 *        bufferLen - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderConsecutiveEncode(
                                     IN  RvSipContentTypeHeaderHandle hHeader,
                                     IN  RvChar*                     strBuffer,
                                     IN  RvUint                      bufferLen,
                                     OUT RvUint*                     actualLen)
{
    MsgContentTypeHeader*   pHeader;
    RvStatus               stat;
    RvUint                 strLen;
    RvBool                 bIsSufficient;
    RvChar                *tempBuffer;

    if((hHeader == NULL) || (NULL == strBuffer) || (NULL == actualLen))
        return RV_ERROR_BADPARAM;
    pHeader = (MsgContentTypeHeader*)hHeader;
    *actualLen = 0;
    bIsSufficient = RV_TRUE;
    tempBuffer = strBuffer;

    /* we don't set "Content-Type: " for consecutive encode */

    /* set type "/" subtype */
    /* set media type */
    stat = MsgUtilsEncodeConsecutiveMediaType(pHeader->pMsgMgr,
                                              pHeader->eMediaType,
                                              pHeader->hPool, pHeader->hPage,
                                              pHeader->strMediaType, bufferLen,
                                              &bIsSufficient,
                                              &tempBuffer, actualLen);
    if ((RV_OK != stat) &&
        (RV_ERROR_INSUFFICIENT_BUFFER != stat))
    {
        return stat;
    }

    /* set '/' */
    strLen = (RvUint)strlen("/");
    if ((strLen + *actualLen <= bufferLen) &&
        (RV_TRUE == bIsSufficient))
    {
        strncpy(tempBuffer, "/", strLen);
        tempBuffer += strLen;
    }
    else
    {
        bIsSufficient = RV_FALSE;
    }
    *actualLen += strLen;

    /* set media sub-type */
    stat = MsgUtilsEncodeConsecutiveMediaSubType(pHeader->pMsgMgr,
                                                 pHeader->eMediaSubType,
                                                 pHeader->hPool, pHeader->hPage,
                                                 pHeader->strMediaSubType, bufferLen,
                                                 &bIsSufficient,
                                                 &tempBuffer, actualLen);
    if ((RV_OK != stat) &&
        (RV_ERROR_INSUFFICIENT_BUFFER != stat))
    {
        return stat;
    }

    /* Content-Type parameters with ";" between them */
    /* boundary */
    if(pHeader->strBoundary > UNDEFINED)
    {
        /* set " ;boundary=" */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        strLen = (RvUint)strlen("; boundary=");
#else
        strLen = (RvUint)strlen(" ;boundary=");
#endif
/* SPIRENT_END */
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            strncpy(tempBuffer, "; boundary=", strLen);
#else
            strncpy(tempBuffer, " ;boundary=", strLen);
#endif
/* SPIRENT_END */
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;

        /* set pHeader->boundary */
        strLen = RPOOL_Strlen(pHeader->hPool, pHeader->hPage, pHeader->strBoundary);
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
            stat = RPOOL_CopyToExternal(pHeader->hPool, pHeader->hPage,
                                        pHeader->strBoundary, tempBuffer, strLen);
            if (RV_OK != stat)
            {
                return stat;
            }
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;

    }
    /* version */
    if(pHeader->strVersion > UNDEFINED)
    {
        /* set " ;version=" */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        strLen = (RvUint)strlen("; version=");
#else
        strLen = (RvUint)strlen(" ;version=");
#endif
/* SPIRENT_END */
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            strncpy(tempBuffer, "; version=", strLen);
#else
            strncpy(tempBuffer, " ;version=", strLen);
#endif
/* SPIRENT_END */
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;

        /* set pHeader->version */
        strLen = RPOOL_Strlen(pHeader->hPool, pHeader->hPage, pHeader->strVersion);
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
            stat = RPOOL_CopyToExternal(pHeader->hPool, pHeader->hPage,
                                        pHeader->strVersion, tempBuffer, strLen);
            if (RV_OK != stat)
            {
                return stat;
            }
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;
    }
    /* base */
    if(pHeader->strBase > UNDEFINED)
    {
        /* set " ;base=" */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        strLen = (RvUint)strlen("; base=");
#else
        strLen = (RvUint)strlen(" ;base=");
#endif
/* SPIRENT_END */
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            strncpy(tempBuffer, "; base=", strLen);
#else
            strncpy(tempBuffer, " ;base=", strLen);
#endif
/* SPIRENT_END */
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;

        /* set pHeader->base */
        strLen = RPOOL_Strlen(pHeader->hPool, pHeader->hPage, pHeader->strBase);
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
            stat = RPOOL_CopyToExternal(pHeader->hPool, pHeader->hPage,
                                        pHeader->strBase, tempBuffer, strLen);
            if (RV_OK != stat)
            {
                return stat;
            }
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;
    }

	/*------------------------------*/
	/* multipart/related parameters */
	/*------------------------------*/
	/* set ;type=type "/" subtype */
    /* set media type */
	if(pHeader->strTypeParamMediaType > UNDEFINED)
    {
        /* set " ;type=" */
        strLen = (RvUint)strlen(" ;type=");
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
            strncpy(tempBuffer, " ;type=", strLen);
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;
		
		stat = MsgUtilsEncodeConsecutiveMediaType(pHeader->pMsgMgr,
		pHeader->eTypeParamMediaType,
		pHeader->hPool, pHeader->hPage,
		pHeader->strTypeParamMediaType, bufferLen,
		&bIsSufficient,
		&tempBuffer, actualLen);
		if ((RV_OK != stat) &&
			(RV_ERROR_INSUFFICIENT_BUFFER != stat))
		{
			return stat;
		}
	}
	if(pHeader->strTypeParamMediaSubType > UNDEFINED)
	{
		/* set '/' */
		strLen = (RvUint)strlen("/");
		if ((strLen + *actualLen <= bufferLen) &&
			(RV_TRUE == bIsSufficient))
		{
			strncpy(tempBuffer, "/", strLen);
			tempBuffer += strLen;
		}
		else
		{
			bIsSufficient = RV_FALSE;
		}
		*actualLen += strLen;
		
		/* set media sub-type */
		stat = MsgUtilsEncodeConsecutiveMediaSubType(pHeader->pMsgMgr,
			pHeader->eTypeParamMediaSubType,
			pHeader->hPool, pHeader->hPage,
			pHeader->strTypeParamMediaSubType, bufferLen,
			&bIsSufficient,
			&tempBuffer, actualLen);
		if ((RV_OK != stat) &&
			(RV_ERROR_INSUFFICIENT_BUFFER != stat))
		{
			return stat;
		}
	}
	/* start */
	if (pHeader->hStartAddrSpec != NULL)
    {
		HPAGE    hTempPage;
				
		/* set " ;start=" */
		strLen = (RvUint)strlen(" ;start=");
		if ((strLen + *actualLen <= bufferLen) &&
			(RV_TRUE == bIsSufficient))
		{
			strncpy(tempBuffer, " ;start=", strLen);
			tempBuffer += strLen;
		}
		else
		{
			bIsSufficient = RV_FALSE;
		}
		*actualLen += strLen;

        /* Set the addr-spec */
		stat = RPOOL_GetPage(pHeader->hPool,0,&hTempPage);
		if (RV_OK != stat)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ContentTypeHeaderConsecutiveEncode: Failed to get temp. page for Content-Type header %p. stat is %d",
				pHeader,stat));
			return stat; 
		}
		strLen = 0;
        stat = AddrEncode(pHeader->hStartAddrSpec, pHeader->hPool, hTempPage, RV_FALSE, RV_TRUE, &strLen);
        if (RV_OK != stat)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"ContentTypeHeaderConsecutiveEncode: Failed to encode Content-Type header %p. stat is %d",
				pHeader,stat));
			RPOOL_FreePage(pHeader->hPool,hTempPage);
            return stat;
		}
		
		strLen = RPOOL_Strlen(pHeader->hPool, hTempPage, 0);
		if ((strLen + *actualLen <= bufferLen) &&
			(RV_TRUE == bIsSufficient))
		{
			stat = RPOOL_CopyToExternal(pHeader->hPool, hTempPage, 0, tempBuffer, strLen);
			if (RV_OK != stat)
			{
				RPOOL_FreePage(pHeader->hPool,hTempPage);
				return stat;
			}
			tempBuffer += strLen;
		}
		else
		{
			bIsSufficient = RV_FALSE;
		}
		*actualLen += strLen;
		RPOOL_FreePage(pHeader->hPool,hTempPage);
    }

    /* other parameters */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set " ;" */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        strLen = (RvUint)strlen("; ");
#else
        strLen = (RvUint)strlen(" ;");
#endif
/* SPIRENT_END */
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            strncpy(tempBuffer, "; ", strLen);
#else
            strncpy(tempBuffer, " ;", strLen);
#endif
/* SPIRENT_END */
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;

        /* set pHeader->base */
        strLen = RPOOL_Strlen(pHeader->hPool, pHeader->hPage, pHeader->strOtherParams);
        if ((strLen + *actualLen <= bufferLen) &&
            (RV_TRUE == bIsSufficient))
        {
            stat = RPOOL_CopyToExternal(pHeader->hPool, pHeader->hPage,
                                        pHeader->strOtherParams, tempBuffer, strLen);
            if (RV_OK != stat)
            {
                return stat;
            }
            tempBuffer += strLen;
        }
        else
        {
            bIsSufficient = RV_FALSE;
        }
        *actualLen += strLen;
    }

    return stat;
}

/***************************************************************************
 * RvSipContentTypeHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual ContentType header - for example:
 *         "Content-Type: multipart/mixed; boundart=unique"
 *         - into a Content-Type header object. All the textual fields are
 *         placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Content-Type header object.
 *    buffer    - Buffer containing a textual Content-Type header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderParse(
                                 IN    RvSipContentTypeHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer)
{
    MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)hHeader;
    RvStatus             rv;
     if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContentTypeHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_TYPE,
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
 * RvSipContentTypeHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ContentType header value into an ContentType
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContentTypeHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ContentType header object.
 *    buffer    - The buffer containing a textual ContentType header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderParseValue(
                                 IN    RvSipContentTypeHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer)
{
    MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)hHeader;
	RvBool                bIsCompact;
    RvStatus              rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    ContentTypeHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_TYPE,
                                        buffer,
                                        RV_TRUE,
                                         (void*)hHeader);

    
	/* restore is-compact indication */
	pHeader->bIsCompact = bIsCompact;
	
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
 * RvSipContentTypeHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ContentType header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderFix(
                                     IN RvSipContentTypeHeaderHandle hHeader,
                                     IN RvChar*                     pFixedBuffer)
{
    MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipContentTypeHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentTypeHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentTypeHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipContentTypeHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-Type header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
HRPOOL SipContentTypeHeaderGetPool(RvSipContentTypeHeaderHandle hContentType)
{
    return ((MsgContentTypeHeader*)hContentType)->hPool;
}

/***************************************************************************
 * SipContentTypeHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-Type header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
HPAGE SipContentTypeHeaderGetPage(RvSipContentTypeHeaderHandle hContentType)
{
    return ((MsgContentTypeHeader*)hContentType)->hPage;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Content-Type header fields are kept in a string format
 *          - for example, the Content-Type header parameter boundary. In order
 *          to get such a field from the Content-Type header object, your
 *          application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipContentTypeHeaderGetStringLength(
                             IN  RvSipContentTypeHeaderHandle     hHeader,
                             IN  RvSipContentTypeHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_CONTENT_TYPE_MEDIATYPE:
        {
            stringOffset = SipContentTypeHeaderGetMediaType(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_MEDIASUBTYPE:
        {
            stringOffset = SipContentTypeHeaderGetMediaSubType(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_BOUNDARY:
        {
            stringOffset = SipContentTypeHeaderGetBoundary(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_VERSION:
        {
            stringOffset = SipContentTypeHeaderGetVersion(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_BASE:
        {
            stringOffset = SipContentTypeHeaderGetBase(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_OTHER_PARAMS:
        {
            stringOffset = SipContentTypeHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_CONTENT_TYPE_BAD_SYNTAX:
        {
            stringOffset = SipContentTypeHeaderGetStrBadSyntax(hHeader);
            break;
        }
		case RVSIP_CONTENT_TYPE_TYPE_PARAM_MEDIATYPE:
		{
			stringOffset = SipContentTypeHeaderTypeParamGetMediaType(hHeader);
			break; 
		}
		case RVSIP_CONTENT_TYPE_TYPE_PARAM_MEDIASUBTYPE:
		{
			stringOffset = SipContentTypeHeaderTypeParamGetMediaSubType(hHeader);
			break; 
		}
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RvSipContentTypeHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipContentTypeHeaderGetPool(hHeader),
                             SipContentTypeHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * SipContentTypeHeaderGetMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetMediaType(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strMediaType;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetMediaType
 * ------------------------------------------------------------------------
 * General: Gets the media type enumeration value. If RVSIP_MEDIATYPE_OTHER
 *          is returned, you can receive the media type string using
 *          RvSipContentTypeHeaderGetStrMediaType().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaType RVCALLCONV RvSipContentTypeHeaderGetMediaType(
                                   IN RvSipContentTypeHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIATYPE_UNDEFINED;
    }
    return (((MsgContentTypeHeader*)hHeader)->eMediaType);
}

/***************************************************************************
 * RvSipContentTypeHeaderGetStrMediaType
 * ------------------------------------------------------------------------
 * General: Copies the media type string from the Content-Type header into
 *          a given buffer. Use this function when the media type enumeration
 *          returned by RvSipContentTypeHeaderGetMediaType() is RVSIP_MEDIATYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetStrMediaType(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetMediaType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderGetMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetMediaSubType(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strMediaSubType;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetMediaSubType
 * ------------------------------------------------------------------------
 * General: Gets the media sub type enumeration value. If
 *          RVSIP_MEDIASUBTYPE_OTHER is returned, you can receive the media
 *          sub type string using RvSipContentTypeHeaderGetMediaSubTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaSubType RVCALLCONV RvSipContentTypeHeaderGetMediaSubType(
                                   IN RvSipContentTypeHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    return (((MsgContentTypeHeader*)hHeader)->eMediaSubType);
}

/***************************************************************************
 * RvSipContentTypeHeaderGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: Copies the media sub type string from the Content-Type header
 *          into a given buffer. Use this function when the media sub type
 *          enumeration returned by RvSipContentTypeGetMediaSubType() is
 *          RVSIP_MEDIASUBTYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetStrMediaSubType(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetMediaSubType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetMediaType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strMediaType = newStr;
#ifdef SIP_DEBUG
    pHeader->pMediaType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                       pHeader->strMediaType);
#endif
    /* set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eMediaType = RVSIP_MEDIATYPE_UNDEFINED;
    }
    else
    {
        pHeader->eMediaType = RVSIP_MEDIATYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: Sets the media type field in the Content-Type header object.
 *          If the enumeration given by eMediaType is RVSIP_MEDIATYPE_OTHER,
 *          sets the media type string given by strMediaType in the
 *          Content-Type header object. Use RVSIP_MEDIATYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaType - The media type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIATYPE_UNDEFINED is supplied, the
 *               existing media type is removed from the header.
 *    strMediaType - The media type string to be set in the Content-Type header.
 *                (relevant when eMediaType is RVSIP_MEDIATYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetMediaType(
                                 IN  RvSipContentTypeHeaderHandle hHeader,
                                 IN  RvSipMediaType               eMediaType,
                                 IN  RvChar                     *strMediaType)
{
    MsgContentTypeHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentTypeHeader *)hHeader;

    pHeader->eMediaType = eMediaType;
    if (RVSIP_MEDIATYPE_OTHER == eMediaType)
    {
        stat = SipContentTypeHeaderSetMediaType(hHeader, strMediaType, NULL,
                                                NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIATYPE_UNDEFINED == eMediaType)
    {
        pHeader->strMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pMediaType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipContentTypeHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaSubType  - The media sub type string to be set - if Null,
 *                     the exist media sub type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetMediaSubType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaSubType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strMediaSubType = newStr;
#ifdef SIP_DEBUG
    pHeader->pMediaSubType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                          pHeader->strMediaSubType);
#endif

    /* Set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eMediaSubType = RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    else
    {
        pHeader->eMediaSubType = RVSIP_MEDIASUBTYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: Sets the media sub type field in the Content-Type header object.
 *          If the enumeration given by eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER,
 *          sets the media sub type string given by strMediaSubType in the
 *          Content-Type header object. Use RVSIP_MEDIASUBTYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaSubType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaSubType - The media sub type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIASUBTYPE_UNDEFINED is supplied, the
 *               existing media sub type is removed from the header.
 *    strMediaSubType - The media sub type string to be set in the Content-Type
 *               header. (relevant when eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetMediaSubType(
                            IN  RvSipContentTypeHeaderHandle hHeader,
                            IN  RvSipMediaSubType            eMediaSubType,
                            IN  RvChar                     *strMediaSubType)
{
    MsgContentTypeHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentTypeHeader *)hHeader;

    pHeader->eMediaSubType = eMediaSubType;
    if (RVSIP_MEDIASUBTYPE_OTHER == eMediaSubType)
    {
        stat = SipContentTypeHeaderSetMediaSubType(hHeader, strMediaSubType, NULL,
                                                   NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIASUBTYPE_UNDEFINED == eMediaSubType)
    {
        pHeader->strMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pMediaSubType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipContentTypeHeaderGetBoundary
 * ------------------------------------------------------------------------
 * General: This method retrieves the boundary field.
 * Return Value: Boundary value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetBoundary(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strBoundary;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetBoundary
 * ------------------------------------------------------------------------
 * General: Copies the boundary string from the Content-Type header
 *          into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetBoundary(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetBoundary(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderSetBoundary
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strBoundary in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strBoundary  - The boundary string to be set - if Null,
 *                           the exist boundary in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetBoundary(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strBoundary,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strBoundary,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBoundary = newStr;
#ifdef SIP_DEBUG
    pHeader->pBoundary = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                      pHeader->strBoundary);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderSetBoundary
 * ------------------------------------------------------------------------
 * General: Sets the boundary string in the Content-Type header object.
 *          The given string is copied to the Content-Type header.
 *          Note that the given boundary may be quoted as defined in RFC 2046.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    boundary    - The boundary string to be set in the Content-Type header.
 *                If NULL is supplied, the existing boudary is removed from
 *                the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetBoundary(
                                 IN   RvSipContentTypeHeaderHandle  hHeader,
                                 IN   RvChar                      *boundary)
{
    /* validity checking will be done in the internal function */
    return SipContentTypeHeaderSetBoundary(hHeader, boundary, NULL,
                                           NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipContentTypeHeaderGetVersion
 * ------------------------------------------------------------------------
 * General: This method retrieves the version field.
 * Return Value: Version value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetVersion(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strVersion;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetVersion
 * ------------------------------------------------------------------------
 * General: Copies the vesion string from the Content-Type header
 *          into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetVersion(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetVersion(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderSetVersion
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strVersion in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strVersion  - The version string to be set - if Null,
 *                           the exist version in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetVersion(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strVersion,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strVersion,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strVersion = newStr;
#ifdef SIP_DEBUG
    pHeader->pVersion = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                      pHeader->strVersion);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderSetVersion
 * ------------------------------------------------------------------------
 * General: Sets the version string in the Content-Type header object.
 *          The given string is copied to the Content-Type header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    version     - The version string to be set in the Content-Type header.
 *                If NULL is supplied, the existing version is removed from
 *                the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetVersion(
                                 IN   RvSipContentTypeHeaderHandle  hHeader,
                                 IN   RvChar                      *version)
{
    /* validity checking will be done in the internal function */
    return SipContentTypeHeaderSetVersion(hHeader, version, NULL,
                                           NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContentTypeHeaderGetBase
 * ------------------------------------------------------------------------
 * General: This method retrieves the base field.
 * Return Value: Version value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetBase(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strBase;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetBase
 * ------------------------------------------------------------------------
 * General: Copies the base string from the Content-Type header
 *          into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetBase(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetBase(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderSetBase
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strBase in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strBase  - The base string to be set - if Null,
 *                     the exist base in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetBase(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strBase,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strBase,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBase = newStr;
#ifdef SIP_DEBUG
    pHeader->pBase = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strBase);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderSetBase
 * ------------------------------------------------------------------------
 * General: Sets the base string in the Content-Type header object.
 *          The given string is copied to the Content-Type header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    base        - The base string to be set in the Content-Type header.
 *                If NULL is supplied, the existing base is removed from
 *                the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetBase(
                                 IN   RvSipContentTypeHeaderHandle  hHeader,
                                 IN   RvChar                      *base)
{
    /* validity checking will be done in the internal function */
    return SipContentTypeHeaderSetBase(hHeader, base, NULL,
                                       NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContentTypeHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetOtherParams(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other parameters string from the Content-Type header
 *          into a given buffer.
 *          Not all the Content-Type header parameters have separated fields
 *          in the Content-Type header object. Parameters with no specific
 *          fields are refered to as other params. They are kept in the object
 *          in one concatenated string in the form- "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetOtherParams(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the Content-Type header object
 *          strOtherParams - The OtherParams string to be set - if Null,
 *                     the exist strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetOtherParams(
                             IN    RvSipContentTypeHeaderHandle hHeader,
                             IN    RvChar *                    strOtherParams,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             IN    RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strOtherParams,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
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
 * RvSipContentTypeHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other parameters string in the Content-Type header object.
 *          The given string is copied to the Content-Type header.
 *          Not all the Content-Type header parameters have separated fields
 *          in the Content-Type header object. Parameters with no specific
 *          fields are refered to as other params. They are kept in the object
 *          in one concatenated string in the form- "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    otherParams - The other parameters string to be set in the Content-Type
 *                header. If NULL is supplied, the existing parametes string
 *                is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetOtherParams(
                                 IN   RvSipContentTypeHeaderHandle  hHeader,
                                 IN   RvChar                      *otherParams)
{
    /* validity checking will be done in the internal function */
    return SipContentTypeHeaderSetOtherParams(hHeader, otherParams, NULL,
                                              NULL_PAGE, UNDEFINED);
}

/***************************************************************************
* SipContentTypeHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
* hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipContentTypeHeaderGetStrBadSyntax(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipContentTypeHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ContentType header,
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
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetStrBadSyntax(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipContentTypeHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetStrBadSyntax(
                                  IN RvSipContentTypeHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgContentTypeHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgContentTypeHeader*)hHeader;

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
 * RvSipContentTypeHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal ContentType header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetStrBadSyntax(
                                  IN RvSipContentTypeHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipContentTypeHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipContentTypeHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Content-Type header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetCompactForm(
                                   IN RvSipContentTypeHeaderHandle hHeader,
                                   IN RvBool                      bIsCompact)
{
    MsgContentTypeHeader* pHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader = (MsgContentTypeHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipContentTypeHeaderSetCompactForm - Setting compact form of hHeader 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}


/***************************************************************************
 * RvSipContentTypeHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Content-Type header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetCompactForm(
                                   IN RvSipContentTypeHeaderHandle hHeader,
                                   IN RvBool                      *pbIsCompact)
{
    MsgContentTypeHeader* pHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    pHeader = (MsgContentTypeHeader*)hHeader;
    *pbIsCompact = pHeader->bIsCompact;
    return RV_OK;
	

}

/*-----------------------------------------------------------------------*/
/*                         MULTIPART/RELATED FUNCTIONS                   */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipContentTypeHeaderGetStart
 * ------------------------------------------------------------------------
 * General: Retrieve the Start parameter address
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderGetStart(
                                    IN  RvSipContentTypeHeaderHandle   hHeader,
                                    OUT RvSipAddressHandle			  *phStartAddr)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

	if (phStartAddr == NULL)
		return RV_ERROR_NULLPTR;

    *phStartAddr = ((MsgContentTypeHeader*)hHeader)->hStartAddrSpec; 

	return RV_OK;
}


/***************************************************************************
 * RvSipContentTypeHeaderSetStart
 * ------------------------------------------------------------------------
 * General: Sets the start address in the Content-Type header object.
 *          The given address is copied to the Content-Type header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    base        - The base string to be set in the Content-Type header.
 *                If NULL is supplied, the existing base is removed from
 *                the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderSetStart(
                                 IN  RvSipContentTypeHeaderHandle   hHeader,
                                 IN  RvSipAddressHandle				hStartAddr)
{
    MsgContentTypeHeader*   pHeader;
	RvStatus                retStatus;
	RvSipAddressType        currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress             *pAddr           = (MsgAddress*)hStartAddr;    

    if (hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader = (MsgContentTypeHeader*)hHeader;
    if(hStartAddr == NULL)
    {
		pHeader->hStartAddrSpec = NULL;
        return RV_OK;
    }
    
    if (NULL != pHeader->hStartAddrSpec)
    {
        currentAddrType = RvSipAddrGetAddrType(pHeader->hStartAddrSpec);
    }

    /* if no address object was allocated, we will construct it */
    if((pHeader->hStartAddrSpec == NULL) ||
       (currentAddrType != pAddr->eAddrType))
    {
        RvSipAddressHandle hSipAddr;

        retStatus = RvSipAddrConstructInContentTypeHeader(hHeader,
                                                     pAddr->eAddrType,
                                                    &hSipAddr);
		if (retStatus != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
	             "RvSipContentTypeHeaderSetStart - Unable to construct address in content-type header 0x%p rv=%d",
		         hHeader, retStatus));
			return retStatus;
		}

	}
	
    
	retStatus = RvSipAddrCopy(pHeader->hStartAddrSpec,hStartAddr); 
	
    return retStatus;
}

/***************************************************************************
 * SipContentTypeHeaderTypeParamGetMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field of the 'type' 
 *		    parameter
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderTypeParamGetMediaType(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strTypeParamMediaType;
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamGetMediaType
 * ------------------------------------------------------------------------
 * General: Gets the media type enumeration value of the 'type' parameter. 
 *			If RVSIP_MEDIATYPE_OTHER
 *          is returned, you can receive the media type string using
 *          RvSipContentTypeHeaderGetStrMediaType().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaType RVCALLCONV RvSipContentTypeHeaderTypeParamGetMediaType(
                                   IN RvSipContentTypeHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIATYPE_UNDEFINED;
    }
    return (((MsgContentTypeHeader*)hHeader)->eTypeParamMediaType);
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamGetStrMediaType
 * ------------------------------------------------------------------------
 * General: Copies the media type string from the 'type' parameter of the 
 *			Content-Type header into 
 *          a given buffer. Use this function when the media type enumeration
 *          returned by RvSipContentTypeHeaderGetMediaType() is RVSIP_MEDIATYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderTypeParamGetStrMediaType(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderTypeParamGetMediaType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderTypeParamGetMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field of the 'type' 
 *			parameter.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderTypeParamGetMediaSubType(
                                     IN RvSipContentTypeHeaderHandle hHeader)
{
    return ((MsgContentTypeHeader*)hHeader)->strTypeParamMediaSubType;
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamGetMediaSubType
 * ------------------------------------------------------------------------
 * General: Gets the media sub type enumeration value of the 'type' parameter. If
 *          RVSIP_MEDIASUBTYPE_OTHER is returned, you can receive the media
 *          sub type string using RvSipContentTypeHeaderGetMediaSubTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaSubType RVCALLCONV RvSipContentTypeHeaderTypeParamGetMediaSubType(
                                   IN RvSipContentTypeHeaderHandle   hHeader)
{
    if (NULL == hHeader)
    {
        return RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    return (((MsgContentTypeHeader*)hHeader)->eTypeParamMediaSubType);
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: Copies the media sub type string from the 'type' parameter of 
 *			the Content-Type header
 *          into a given buffer. Use this function when the media sub type
 *          enumeration returned by RvSipContentTypeGetMediaSubType() is
 *          RVSIP_MEDIASUBTYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderTypeParamGetStrMediaSubType(
                                    IN RvSipContentTypeHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContentTypeHeader*)hHeader)->hPool,
                                  ((MsgContentTypeHeader*)hHeader)->hPage,
                                  SipContentTypeHeaderTypeParamGetMediaSubType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContentTypeHeaderTypeParamSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          'type' parameter of the Content-Type header object. 
 *			The API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid un-needed coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderTypeParamSetMediaType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strTypeParamMediaType = newStr;
#ifdef SIP_DEBUG
    pHeader->pTypeParamMediaType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                       pHeader->strTypeParamMediaType);
#endif
    /* set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eTypeParamMediaType = RVSIP_MEDIATYPE_UNDEFINED;
    }
    else
    {
        pHeader->eTypeParamMediaType = RVSIP_MEDIATYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamSetMediaType
 * ------------------------------------------------------------------------
 * General: Sets the media type field in the
 *          'type' parameter of the the Content-Type header object.
 *          If the enumeration given by eMediaType is RVSIP_MEDIATYPE_OTHER,
 *          sets the media type string given by strMediaType in the
 *          Content-Type header object. Use RVSIP_MEDIATYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaType - The media type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIATYPE_UNDEFINED is supplied, the
 *               existing media type is removed from the header.
 *    strMediaType - The media type string to be set in the Content-Type header.
 *                (relevant when eMediaType is RVSIP_MEDIATYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderTypeParamSetMediaType(
                                 IN  RvSipContentTypeHeaderHandle hHeader,
                                 IN  RvSipMediaType               eMediaType,
                                 IN  RvChar                      *strMediaType)
{
    MsgContentTypeHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentTypeHeader *)hHeader;

    pHeader->eTypeParamMediaType = eMediaType;
    if (RVSIP_MEDIATYPE_OTHER == eMediaType)
    {
        stat = SipContentTypeHeaderTypeParamSetMediaType(
				hHeader, strMediaType, NULL,NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIATYPE_UNDEFINED == eMediaType)
    {
        pHeader->strTypeParamMediaType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pTypeParamMediaType = NULL;
#endif
    }
    return stat;
}

/***************************************************************************
 * SipContentTypeHeaderTypeParamSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          'type' parameter of the Content-Type header object. the API will 
 *			call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaSubType  - The media sub type string to be set - if Null,
 *                     the exist media sub type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderTypeParamSetMediaSubType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgContentTypeHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContentTypeHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMediaSubType,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strTypeParamMediaSubType = newStr;
#ifdef SIP_DEBUG
    pHeader->pTypeParamMediaSubType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                          pHeader->strTypeParamMediaSubType);
#endif

    /* Set appropriate enumeration */
    if (UNDEFINED == newStr)
    {
        pHeader->eTypeParamMediaSubType = RVSIP_MEDIASUBTYPE_UNDEFINED;
    }
    else
    {
        pHeader->eTypeParamMediaSubType = RVSIP_MEDIASUBTYPE_OTHER;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContentTypeHeaderTypeParamSetMediaSubType
 * ------------------------------------------------------------------------
 * General: Sets the media sub type field in the 'type' parameter of the 
 *			Content-Type header object.
 *          If the enumeration given by eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER,
 *          sets the media sub type string given by strMediaSubType in the
 *          Content-Type header object. Use RVSIP_MEDIASUBTYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaSubType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaSubType - The media sub type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIASUBTYPE_UNDEFINED is supplied, the
 *               existing media sub type is removed from the header.
 *    strMediaSubType - The media sub type string to be set in the Content-Type
 *               header. (relevant when eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentTypeHeaderTypeParamSetMediaSubType(
                            IN  RvSipContentTypeHeaderHandle hHeader,
                            IN  RvSipMediaSubType            eMediaSubType,
                            IN  RvChar                     *strMediaSubType)
{
    MsgContentTypeHeader *pHeader;
    RvStatus             stat = RV_OK;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgContentTypeHeader *)hHeader;

    pHeader->eTypeParamMediaSubType = eMediaSubType;
    if (RVSIP_MEDIASUBTYPE_OTHER == eMediaSubType)
    {
        stat = SipContentTypeHeaderTypeParamSetMediaSubType(
					hHeader, strMediaSubType, NULL,NULL_PAGE, UNDEFINED);
    }
    else if (RVSIP_MEDIASUBTYPE_UNDEFINED == eMediaSubType)
    {
        pHeader->strTypeParamMediaSubType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pTypeParamMediaSubType = NULL;
#endif
    }
    return stat;
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ContentTypeHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ContentTypeHeaderClean(IN MsgContentTypeHeader*  pHeader,
								   IN RvBool                 bCleanBS)
{
    pHeader->eMediaType                 = RVSIP_MEDIATYPE_UNDEFINED;
    pHeader->eMediaSubType              = RVSIP_MEDIASUBTYPE_UNDEFINED;
    pHeader->strBase                    = UNDEFINED;
    pHeader->strBoundary                = UNDEFINED;
    pHeader->strMediaSubType            = UNDEFINED;
    pHeader->strMediaType               = UNDEFINED;
    pHeader->strVersion                 = UNDEFINED;
    pHeader->strOtherParams             = UNDEFINED;
    pHeader->eHeaderType                = RVSIP_HEADERTYPE_CONTENT_TYPE;
    pHeader->eTypeParamMediaType        = RVSIP_MEDIATYPE_UNDEFINED;
    pHeader->eTypeParamMediaSubType     = RVSIP_MEDIASUBTYPE_UNDEFINED;
    pHeader->strTypeParamMediaType      = UNDEFINED;
    pHeader->strTypeParamMediaSubType   = UNDEFINED;
    pHeader->hStartAddrSpec             = NULL;
#ifdef SIP_DEBUG
    pHeader->pBase                  = NULL;
    pHeader->pBoundary              = NULL;
    pHeader->pMediaSubType          = NULL;
    pHeader->pMediaType             = NULL;
    pHeader->pVersion               = NULL;
    pHeader->pTypeParamMediaType    = NULL;
    pHeader->pTypeParamMediaSubType = NULL;
    pHeader->hStartAddrSpec  = NULL;
    pHeader->strTypeParamMediaType    = UNDEFINED;
    pHeader->strTypeParamMediaSubType = UNDEFINED;
#endif

    if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


