/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                   RvSipViaHeader.c                                         *
 *                                                                            *
 * The file defines the methods of the Via header object.                     *
 * The Via header functions enable you to construct, copy, encode, parse,     *
 * Via header objects and access Via header object parameters.                *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipViaHeader.h"
#include "_SipViaHeader.h"
#include "RvSipMsg.h"
#include "MsgUtils.h"
#include "MsgTypes.h"
#include <string.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ViaHeaderClean(IN MsgViaHeader* pHeader,
						   IN RvBool        bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipViaHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Via Header header object inside a given message
 *          object. The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail
 *          of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the list-RV_TRUE
 *                            -or to the tail-RV_FALSE.
 * output: hHeader - Handle to the newly constructed Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderConstructInMsg(
                                           IN  RvSipMsgHandle        hSipMsg,
                                           IN  RvBool               pushHeaderAtHead,
                                           OUT RvSipViaHeaderHandle* hHeader)
{
    MsgMessage*               msg;
    RvSipHeaderListElemHandle hListElem= NULL;
    RvSipHeadersLocation      location;
    RvStatus                  stat;


    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipViaHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if (RV_OK != stat)
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
                              RVSIP_HEADERTYPE_VIA,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipViaHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Via header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderConstruct(
                                           IN  RvSipMsgMgrHandle     hMsgMgr,
                                           IN  HRPOOL                hPool,
                                           IN  HPAGE                 hPage,
                                           OUT RvSipViaHeaderHandle* hHeader)
{
    MsgViaHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgViaHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                   hPool,
                                                   hPage,
                                                   sizeof(MsgViaHeader),
                                                   RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipViaHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr            = pMsgMgr;
    pHeader->hPage              = hPage;
    pHeader->hPool              = hPool;
    ViaHeaderClean(pHeader, RV_TRUE);
    pHeader->bIsCompact       = RV_FALSE;
    pHeader->strHost          = UNDEFINED;
    pHeader->eTransport       = RVSIP_TRANSPORT_UNDEFINED;
    pHeader->strTransport     = UNDEFINED;
    pHeader->strBadSyntax     = UNDEFINED;
    pHeader->portNum          = UNDEFINED;
    pHeader->eComp            = RVSIP_COMP_UNDEFINED;
    pHeader->strCompParam     = UNDEFINED;
	pHeader->strSigCompIdParam = UNDEFINED;
#ifdef SIP_DEBUG
    pHeader->pBadSyntax       = NULL;
    pHeader->pHost            = NULL;
    pHeader->pTransport       = NULL;
    pHeader->pCompParam       = NULL;
	pHeader->pSigCompIdParam  = NULL;
#endif /* SIP_DEBUG */

    *hHeader = (RvSipViaHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipViaHeaderConstruct - Via header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipViaHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Via header object to a
 *          destination Via header object.
 *          You must construct the destination object before using this
 *          function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Via header object.
 *    pSource      - Handle to the source Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderCopy(
                                          INOUT RvSipViaHeaderHandle hDestination,
                                          IN    RvSipViaHeaderHandle hSource)
{
    MsgViaHeader* source;
    MsgViaHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgViaHeader*)hSource;
    dest   = (MsgViaHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;
    /* eTransport */
    dest->eTransport = source->eTransport;
    dest->bIsCompact = source->bIsCompact;
    dest->bIsHidden  = source->bIsHidden;
    dest->bAlias     = source->bAlias;

    if((source->eTransport == RVSIP_TRANSPORT_OTHER) && (source->strTransport > UNDEFINED))
    {
        dest->strTransport = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strTransport);
        if(dest->strTransport == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy transport. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pTransport = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strTransport);
#endif
    }
    else
    {
        dest->strTransport = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTransport = NULL;
#endif
    }

    /* ttl */
    dest->ttlNum  = source->ttlNum;

    /* portNum */
    dest->portNum = source->portNum;

    /* strHost */
    /*stat = RvSipViaHeaderSetHost(hDestination, source->strHost);*/
    if(source->strHost > UNDEFINED)
    {
        dest->strHost = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                        dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strHost);
        if(dest->strHost == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - didn't manage to copy strHost. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pHost = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strHost);
#endif
    }
    else
    {
        dest->strHost = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pHost = NULL;
#endif
    }

    /* strMaddrParam */
    /*stat = RvSipViaHeaderSetMaddrParam(hDestination, source->strMaddrParam);*/
    if(source->strMaddrParam > UNDEFINED)
    {
        dest->strMaddrParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strMaddrParam);
        if(dest->strMaddrParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy strMaddrParam. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMaddrParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strMaddrParam);
#endif
    }
    else
    {
        dest->strMaddrParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMaddrParam = NULL;
#endif
    }

    /* strReceivedParam */
    /*stat = RvSipViaHeaderSetReceivedParam(hDestination, source->strReceivedParam);*/
    if(source->strReceivedParam > UNDEFINED)
    {
        dest->strReceivedParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                                 dest->hPool,
                                                                  dest->hPage,
                                                                  source->hPool,
                                                                  source->hPage,
                                                                  source->strReceivedParam);
        if(dest->strReceivedParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy strReceivedParam. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pReceivedParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                              dest->strReceivedParam);
#endif
    }
    else
    {
        dest->strReceivedParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pReceivedParam = NULL;
#endif
    }

    /* rportParam & bRportExists */
    dest->rportParam   = source->rportParam;
    dest->bRportExists = source->bRportExists;

    /* strBranchParam */
    /*stat = RvSipViaHeaderSetBranchParam(hDestination, source->strBranchParam);*/
    if(source->strBranchParam > UNDEFINED)
    {
        dest->strBranchParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                               dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strBranchParam);
        if(dest->strBranchParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy strBranchParam. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pBranchParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                              dest->strBranchParam);
#endif
    }
    else
    {
        dest->strBranchParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBranchParam = NULL;
#endif
    }

    /* comp */
    dest->eComp = source->eComp;

    if((source->eComp == RVSIP_COMP_OTHER) && (source->strCompParam > UNDEFINED))
    {
        dest->strCompParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strCompParam);
        if(dest->strCompParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy comp. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCompParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                           dest->strCompParam);
#endif
    }
    else
    {
        dest->strCompParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCompParam = NULL;
#endif
    }

	/* sigcomp-id */
	if (source->strSigCompIdParam > UNDEFINED)
	{
		dest->strSigCompIdParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
																dest->hPool,
																dest->hPage,
																source->hPool,
																source->hPage,
																source->strSigCompIdParam);
		if (dest->strSigCompIdParam == UNDEFINED)
		{
			RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - didn't manage to copy strSigCompId. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pSigCompIdParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
													dest->strSigCompIdParam);
#endif
	}
	else
	{
		dest->strSigCompIdParam = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pSigCompIdParam = NULL;
#endif
	}
	
    /* strViaParams */
    /*stat = RvSipViaHeaderSetOtherParams(hDestination, source->strViaParams);*/
    if(source->strViaParams > UNDEFINED)
    {
        dest->strViaParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                             dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strViaParams);
        if(dest->strViaParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipViaHeaderCopy - Failed to copy strViaParams. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pViaParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                              dest->strViaParams);
#endif
    }
    else
    {
        dest->strViaParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pViaParams = NULL;
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
                "RvSipViaHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipViaHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Via header object to a textual Via header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader  - Handle to the Via header object.
 *         hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderEncode(
                                          IN    RvSipViaHeaderHandle   hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgViaHeader* pHeader;

    pHeader = (MsgViaHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if ((hPool == NULL) || (hHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipViaHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipViaHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ViaHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipViaHeaderEncode - Failed to encode Via header. Free page 0x%p on pool 0x%p. ViaHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ViaHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            via header (as string) on it.
 *          via-header = "Via: " 1#(sent-protocol sent-by *(";"via-params)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the party header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderEncode(
                          IN    RvSipViaHeaderHandle   hHeader,
                          IN    HRPOOL                 hPool,
                          IN    HPAGE                  hPage,
                          IN    RvBool                 bInUrlHeaders,
                          IN    RvBool                 bForceCompactForm,
                          INOUT RvUint32*              length)
{
    MsgViaHeader*   pHeader = (MsgViaHeader*)hHeader;
    RvStatus       stat;
    RvChar         strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ViaHeaderEncode - Encoding Via header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "Via" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "v", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Via", length);
    }
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ViaHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    length);
    if(stat != RV_OK)
        return stat;


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
                "ViaHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* set "SIP/2.0/" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "SIP/2.0/", length);

    if(stat != RV_OK)
        return stat;

    /* set transport */
    if(pHeader->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        stat = MsgUtilsEncodeTransportType(pHeader->pMsgMgr,
                                      hPool,
                                      hPage,
                                      pHeader->eTransport,
                                      pHeader->hPool,
                                      pHeader->hPage,
                                      pHeader->strTransport,
                                      length);
        if(stat != RV_OK)
            return stat;
    }
    else
    {
        /* not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ViaHeaderEncode - Failed. eTransport is UNDEFINED. not an optional param!!! hHeader 0x%p,",
            hHeader));
        return RV_ERROR_BADPARAM;
    }

    /* set (host[":"port]) */
    if(pHeader->strHost > UNDEFINED)
    {
        /* set space */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSpaceChar(bInUrlHeaders), length);
        if(stat != RV_OK)
            return stat;

        /* set strHost */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strHost,
                                    length);
        if(stat != RV_OK)
            return stat;

        if(pHeader->portNum > UNDEFINED)
        {
            /* set ":" and portNum */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, ":", length);
            if(stat != RV_OK)
                return stat;

            MsgUtils_itoa(pHeader->portNum, strHelper);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
            if(stat != RV_OK)
                return stat;
        }
    }
    else
    {
        /* this is not an optional part of the header */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ViaHeaderEncode - Failed. strHost is NULL. not an optional param!!! hHeader 0x%p,",
            hHeader));
        return RV_ERROR_BADPARAM;
    }

    /* set via params *(";" viaParams)*/

    if(pHeader->strMaddrParam > UNDEFINED)
    {
        /* set " ;maddr=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "maddr", length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;

        /* set maddrParam */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strMaddrParam,
                                    length);
    }

    if(pHeader->bIsHidden == RV_TRUE)
    {
        /* set ";hidden" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "hidden", length);
        if(stat != RV_OK)
             return stat;
    }

    if(pHeader->bAlias == RV_TRUE)
    {
        /* set ";alias" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "alias", length);
        if(stat != RV_OK)
            return stat;
    }

    if(pHeader->ttlNum != UNDEFINED)
    {
        /* set " ;ttl=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "ttl", length);
        if(stat != RV_OK)
             return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;

        MsgUtils_itoa(pHeader->ttlNum, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
        if(stat != RV_OK)
             return stat;
    }

    if(pHeader->strReceivedParam > UNDEFINED)
    {
        /* set " ;received=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "received", length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;

        /* set pHeader->strReceivedParam */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strReceivedParam,
                                    length);
    }

    if (pHeader->bRportExists == RV_TRUE)
    {
        /* set " ;rport" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "rport", length);

        if(pHeader->rportParam > UNDEFINED)
        {
            /* set "=" and portNum */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders),length);
            if(stat != RV_OK)
                 return stat;

            MsgUtils_itoa(pHeader->rportParam, strHelper);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
        }
    }

    if(pHeader->strBranchParam  > UNDEFINED)
    {
        /* set " ;branch=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "branch", length);
        if(stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;

        /* set pHeader->strBranchParam */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBranchParam,
                                    length);
    }
    /* set compression type */
    if(pHeader->eComp > RVSIP_COMP_UNDEFINED)
    {
        /* set " ;comp=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                      MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "comp", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                      MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* set pHeader->strCompParam */
        stat = MsgUtilsEncodeCompType(pHeader->pMsgMgr,
                                      hPool,
                                      hPage,
                                      pHeader->eComp,
                                      pHeader->hPool,
                                      pHeader->hPage,
                                      pHeader->strCompParam,
                                      length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	if(pHeader->strSigCompIdParam > UNDEFINED)
	{
		/* set ";sigcomp-id="" */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "sigcomp-id", length);
        if(stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;

        /* set pHeader->strSigCompIdParam */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strSigCompIdParam,
                                    length);
        if(stat != RV_OK)
             return stat;
		/* set the closing " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders),length);
		if(stat != RV_OK)
             return stat;
	}

    if(pHeader->strViaParams > UNDEFINED)
    {
        RvChar tempChar;

        tempChar = '(';
        stat = RPOOL_CopyToExternal(pHeader->hPool, pHeader->hPage,
                                    pHeader->strViaParams, &tempChar, 1);
        if ('(' == tempChar)
        {
            /* set " " */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, " ", length);
        }
        else
        {
            /* set ";" */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                                MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        }
        if(stat != RV_OK)
             return stat;

        /* set pHeader->strViaParams */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strViaParams,
                                    length);
        if(stat != RV_OK)
             return stat;
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ViaHeaderEncode - Failed to encode Via header. stat: %d, header 0x%p",
            stat, hHeader));
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ViaHeaderEncode - Via header 0x%p was encoded successfully.",
            hHeader));
    }
    return stat;

}

/***************************************************************************
 * RvSipViaHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Via header-for example,
 *         "Via: SIP/2.0/UDP h.caller.com"-into a Via header object.
 *         All the textual fields are placed inside the object.
 *         You must construct a Via header before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Via header object.
 *    buffer    - Buffer containing a textual Via header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderParse(
                                     IN    RvSipViaHeaderHandle     hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;
    RvStatus             rv;
    if (hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ViaHeaderClean(pHeader, RV_FALSE);

   rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_VIA,
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
 * RvSipViaHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Via header value into an Via header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipViaHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Via header object.
 *    buffer    - The buffer containing a textual Via header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderParseValue(
                                     IN    RvSipViaHeaderHandle     hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;
	RvBool        bIsCompact;
    RvStatus      rv;
	
    if (hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    /* The is-compact indication is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    ViaHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_VIA,
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
 * RvSipViaHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Via header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipViaHeaderFix(
                                     IN RvSipViaHeaderHandle hHeader,
                                     IN RvChar*             pFixedBuffer)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipViaHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipViaHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipViaHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipViaHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Via object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The via header to take the pool from.
 ***************************************************************************/
HRPOOL SipViaHeaderGetPool(RvSipViaHeaderHandle hHeader)
{
    return ((MsgViaHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipViaHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Via object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The via header to take the page from.
 ***************************************************************************/
HPAGE SipViaHeaderGetPage(RvSipViaHeaderHandle hHeader)
{
    return ((MsgViaHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipViaHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Via header fields are kept in a string format-for
 *          example, the host parameter. In order to get such a field from
 *          the Via header object, your application should supply an adequate
 *          buffer to where the string will be copied. This function provides
 *          you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Via header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipViaHeaderGetStringLength(
                                      IN  RvSipViaHeaderHandle     hHeader,
                                      IN  RvSipViaHeaderStringName stringName)
{
    RvInt32  stringOffset;
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_VIA_TRANSPORT:
        {
            stringOffset = SipViaHeaderGetStrTransport(hHeader);
            break;
        }
        case RVSIP_VIA_HOST:
        {
            stringOffset = SipViaHeaderGetHost(hHeader);
            break;
        }
        case RVSIP_VIA_MADDR_PARAM:
        {
            stringOffset = SipViaHeaderGetMaddrParam(hHeader);
            break;
        }
        case RVSIP_VIA_RECEIVED_PARAM:
        {
            stringOffset = SipViaHeaderGetReceivedParam(hHeader);
            break;
        }
        case RVSIP_VIA_BRANCH_PARAM:
        {
            stringOffset = SipViaHeaderGetBranchParam(hHeader);
            break;
        }
        case RVSIP_VIA_COMP_PARAM:
        {
            stringOffset = SipViaHeaderGetStrCompParam(hHeader);
            break;
        }

		case RVSIP_VIA_SIGCOMPID_PARAM:
		{
			stringOffset = SipViaHeaderGetStrSigCompIdParam(hHeader);
			break;
		}

        case RVSIP_VIA_BAD_SYNTAX:
        {
            stringOffset = SipViaHeaderGetStrBadSyntax(hHeader);
            break;
        }
        case RVSIP_VIA_OTHER_PARAMS:
        {
            stringOffset = SipViaHeaderGetViaParams(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipViaHeaderGetStringLength - unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool,pHeader->hPage,stringOffset) + 1);
    else
        return 0;
}
/***************************************************************************
 * RvSipViaHeaderGetTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport protocol enumeration from the Via header object.
 *          If this function returns RVSIP_TRANSPORT_OTHER, call
 *          RvSipViaHeaderGetStrTransport() to get the transport protocol
 *          in a string format.
 *
 * Return Value: Returns the transport protocol enumeration of the Via
 *        header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hViaHeader - Handle to the Via header object.
 ***************************************************************************/
RVAPI RvSipTransport RVCALLCONV RvSipViaHeaderGetTransport(
                                        IN  RvSipViaHeaderHandle hSipViaHeader)
{
     if(hSipViaHeader == NULL)
         return RVSIP_TRANSPORT_UNDEFINED;

    return ((MsgViaHeader*)hSipViaHeader)->eTransport;
}

/***************************************************************************
 * SipViaHeaderGetStrTransport
 * ------------------------------------------------------------------------
 * General: This method returns the transport protocol string value from the
 *          MsgViaHeader object.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrTransport(IN  RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strTransport;
}

/***************************************************************************
 * RvSipViaHeaderGetStrTransport
 * ------------------------------------------------------------------------
 * General: Copies the transport protocol string from the Via header object
 *          into a given buffer. Use this function if
 *          RvSipViaHeaderGetTransport() returns RVSIP_TRANSPORT_OTHER.
 *          If the bufferLen size is adequate, the function copies the
 *          parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen paramater contains
 *          the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hHeader - Handle to the Via header object.
 *         strBuffer - Buffer to fill with the requested parameter.
 *         bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetStrTransport(
                                        IN  RvSipViaHeaderHandle hHeader,
                                        IN  RvChar*            strBuffer,
                                        IN  RvUint             bufferLen,
                                        OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetStrTransport(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetTransport
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the transport in the MsgViaHeader
 *           object. User can set a transport enum value (in eTransport) and no string,
 *           or to set eTransport to be RVSIP_TRANSPORT_OTHER, and then put the
 *           transport value in strTransport. the API will call it with NULL pool
 *           and page, to make a real allocation and copy. internal modules
 *           (such as parser) will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 *    eTransport    - The enum transport protocol value to be set in the object.
 *  strTransport  - String of transportType, in case eTransport is
 *                  RVSIP_TRANSPORT_OTHER. may be NULL.
 *  hPool         - The pool on which the string lays (if relevant).
 *  hPage         - The page on which the string lays (if relevant).
 *  strOffset     - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetTransport(IN    RvSipViaHeaderHandle hSipViaHeader,
                                   IN    RvSipTransport       eTransport,
                                   IN    RvChar*             strTransport,
                                   IN    HRPOOL               hPool,
                                   IN    HPAGE                hPage,
                                   IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader = (MsgViaHeader*)hSipViaHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader->eTransport = eTransport;

    if(eTransport == RVSIP_TRANSPORT_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strTransport,
                                      pHeader->hPool, pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strTransport = newStr;
#ifdef SIP_DEBUG
        pHeader->pTransport = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strTransport);
#endif
    }
    else
    {
        pHeader->strTransport = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pTransport = NULL;
#endif
    }
    return RV_OK;
}


/***************************************************************************
 * RvSipViaHeaderSetTransport
 * ------------------------------------------------------------------------
 * General:  Sets the transport type the Via header object.
 *           You can use this parametere only if the eTransport parameter is
 *           set to RVSIP_TRANSPORT_OTHER. In this case you can supply the
 *           transport protocol as a string.
 *           If eTransport is set to RVSIP_TRANSPORT_OTHER strTransport is
 *           copied to the header. Otherwise, strTransport is ignored.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    eTransport    - The enumeration transport protocol to set in the object.
 *  strTransport  - You can use this parametere only if the eTransport
 *                  parameter is set to RVSIP_TRANSPORT_OTHER. In this case
 *                  you can supply the transport protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetTransport(
                                     IN    RvSipViaHeaderHandle hSipViaHeader,
                                     IN    RvSipTransport       eTransport,
                                     IN    RvChar*             strTransport)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetTransport(hSipViaHeader,
                                    eTransport,
                                    strTransport,
                                    NULL,
                                    NULL_PAGE,
                                    UNDEFINED);
}

/***************************************************************************
 * SipViaHeaderGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the host name in the MsgViaHeader.
 * Return Value: host name string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetHost(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strHost;
}

/***************************************************************************
 * RvSipViaHeaderGetHost
 * ------------------------------------------------------------------------
 * General:  Copies the host name-send-by parameter-from the Via header
 *           object into a given buffer. If the bufferLen size is adequate,
 *           the function copies the parameter into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the
 *           actualLen parameter contains the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetHost(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetHost(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipViaHeaderSetHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strHost in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipViaHeader - Handle of the Via header object.
 *            strHost       - The host value to be set in the object. If Null, the
 *                          exist host string in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetHost(IN    RvSipViaHeaderHandle hSipViaHeader,
                              IN    RvChar*             strHost,
                              IN    HRPOOL               hPool,
                              IN    HPAGE                hPage,
                              IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgViaHeader*)hSipViaHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strHost,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strHost = newStr;
#ifdef SIP_DEBUG
    pHeader->pHost = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strHost);
#endif

    return RV_OK;
}
/***************************************************************************
 * RvSipViaHeaderSetHost
 * ------------------------------------------------------------------------
 * General: Sets the host field in the Via header object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    strHost       - The host to be set in the object. If Null is supplied,
 *                  the existing host string is removed from the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetHost(
                                         IN    RvSipViaHeaderHandle hViaHeader,
                                         IN    RvChar*             strHost)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetHost(hViaHeader, strHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipViaHeaderGetPortNum
 * ------------------------------------------------------------------------
 * General: Gets the host port from the Via header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader - Handle to the Via header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipViaHeaderGetPortNum(
                                         IN RvSipViaHeaderHandle hSipViaHeader)
{
    if(hSipViaHeader == NULL)
        return UNDEFINED;

    return ((MsgViaHeader*)hSipViaHeader)->portNum;
}


/***************************************************************************
 * RvSipViaHeaderSetPortNum
 * ------------------------------------------------------------------------
 * General:  Sets the host port number of the Via header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader - Handle to the Via header object.
 *    portNum       - The port number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetPortNum(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvInt32             portNum)
{
    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgViaHeader*)hSipViaHeader)->portNum = portNum;
    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderGetTtlNum
 * ------------------------------------------------------------------------
 * General: Gets the ttl from the Via header object.
 * Return Value: Returns the ttl number, or UNDEFINED if the ttl
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader - Handle to the Via header object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipViaHeaderGetTtlNum(
                                         IN RvSipViaHeaderHandle hSipViaHeader)
{
    if(hSipViaHeader == NULL)
        return UNDEFINED;

    return ((MsgViaHeader*)hSipViaHeader)->ttlNum;
}


/***************************************************************************
 * RvSipViaHeaderSetTtlNum
 * ------------------------------------------------------------------------
 * General: Sets the ttl field in the Via header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader - Handle to the Via header object.
 *    ttlNum        - The ttl value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetTtlNum(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvInt16             ttlNum)
{
    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgViaHeader*)hSipViaHeader)->ttlNum = ttlNum;
    return RV_OK;
}

/***************************************************************************
 * SipViaHeaderGetMaddrParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Maddr parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetMaddrParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strMaddrParam;
}

/***************************************************************************
 * RvSipViaHeaderGetMaddrParam
 * ------------------------------------------------------------------------
 * General: Copies the maddr parameter of the Via header object into a
 *          given buffer. If the bufferLen size is adequate, the function
 *          copies the parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter contains
 *          the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetMaddrParam(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetMaddrParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}
/***************************************************************************
 * SipViaHeaderSetMaddrParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the MaddrParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipViaHeader - Handle of the Via header object.
 *            strMaddrParam - The maddr parameter to be set in the object.
 *                          If Null, the exist MsddrParam string in the
 *                          object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetMaddrParam(IN    RvSipViaHeaderHandle hSipViaHeader,
                                    IN    RvChar*             strMaddrParam,
                                    IN    HRPOOL               hPool,
                                    IN    HPAGE                hPage,
                                    IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgViaHeader*)hSipViaHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strMaddrParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strMaddrParam = newStr;
#ifdef SIP_DEBUG
    pHeader->pMaddrParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                        pHeader->strMaddrParam);
#endif

    return RV_OK;
}
/***************************************************************************
 * RvSipViaHeaderSetMaddrParam
 * ------------------------------------------------------------------------
 * General: Sets the maddr string parameter in the Via header object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    strMaddrParam - The Maddr parameter to be set in the object. If Null is
 *                  supplied, the existing Maddr parameter string is removed
 *                  from the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetMaddrParam(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvChar              *strMaddrParam)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetMaddrParam(hSipViaHeader, strMaddrParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipViaHeaderGetReceivedParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Received parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetReceivedParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strReceivedParam;
}

/***************************************************************************
 * RvSipViaHeaderGetReceivedParam
 * ------------------------------------------------------------------------
 * General: Copies the received parameter of the Via header object into a
 *          given buffer. If the bufferLen size is adequate, the function
 *          copies the parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter
 *          contains the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetReceivedParam(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetReceivedParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetReceivedParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ReceivedParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipViaHeader    - Handle of the Via header object.
 *            strReceivedParam - The Received parameter to be set in the object.
 *                             If Null, the exist ReceivedParam string in the
 *                             object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetReceivedParam(IN    RvSipViaHeaderHandle hSipViaHeader,
                                       IN    RvChar*             strReceivedParam,
                                       IN    HRPOOL               hPool,
                                       IN    HPAGE                hPage,
                                       IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgViaHeader*)hSipViaHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strReceivedParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strReceivedParam = newStr;
#ifdef SIP_DEBUG
    pHeader->pReceivedParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strReceivedParam);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderSetReceivedParam
 * ------------------------------------------------------------------------
 * General: Sets the value of the received string parameter in the Via
 *          header object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader    - Handle to the Via header object.
 *    strReceivedParam - The Received parameter to be set in the object.
 *                     If Null is supplied, the existing received parameter
 *                     string is removed from the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetReceivedParam(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvChar*             strReceivedParam)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetReceivedParam(hSipViaHeader,
                                        strReceivedParam,
                                        NULL,
                                        NULL_PAGE,
                                        UNDEFINED);
}

/***************************************************************************
 * RvSipViaHeaderGetRportParam
 * ------------------------------------------------------------------------
 * General: Gets the rport parameter from the Via header object.
 * Return Value: Returns RV_OK when rport parameter appears in Via header,
 *               or RV_ERROR_UNKNOWN in case of non existing rport parameter,
 *               RV_ERROR_INVALID_HANDLE, or RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipViaHeader - Handle to the Via header object.
 * output: rportParam - The rport (number) value , or UNDEFINED if the rport
 *               parameter is empty.
 *         bUseRport     - Indication if 'rport' parameter exists in header
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetRportParam(
                                         IN  RvSipViaHeaderHandle hSipViaHeader,
                                         OUT RvInt32*             rportParam,
                                         OUT RvBool*              bUseRport)
{
    MsgViaHeader* pHeader;

    if (hSipViaHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (rportParam == NULL || bUseRport == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    pHeader    = (MsgViaHeader*)hSipViaHeader;

    *bUseRport = pHeader->bRportExists;
    if (pHeader->bRportExists == RV_FALSE)
    {
        return RV_ERROR_NOT_FOUND;
    }


    *rportParam = ((MsgViaHeader*)hSipViaHeader)->rportParam;

    return RV_OK;
}


/***************************************************************************
 * RvSipViaHeaderSetRportParam
 * ------------------------------------------------------------------------
 * General:  Sets the rport parameter value in Via header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    rportParam    - The rport number to be set in the Via header object.
 *  useRport      - Indication if the 'rport' should be added to Via Header
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetRportParam(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvInt32             rportParam,
                                         IN    RvBool              bUseRport)
{
    MsgViaHeader* pHeader;

    if (hSipViaHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    pHeader = (MsgViaHeader*)hSipViaHeader;

    pHeader->bRportExists = bUseRport;
    if (pHeader->bRportExists == RV_TRUE)
    {
        /* Empty rport parameter might be added (with UNDEFINED value) */
        pHeader->rportParam = rportParam;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderGetAliasParam
 * ------------------------------------------------------------------------
 * General: Gets the alias parameter from the Via header object.
 * Return Value: true - if alias param exists. false - if not.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipViaHeader - Handle to the Via header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipViaHeaderGetAliasParam(
                                         IN  RvSipViaHeaderHandle hSipViaHeader)
{
    MsgViaHeader* pHeader;

    if (hSipViaHeader == NULL)
    {
        return RV_FALSE;
    }

    pHeader    = (MsgViaHeader*)hSipViaHeader;

    return pHeader->bAlias;
}


/***************************************************************************
 * RvSipViaHeaderSetAliasParam
 * ------------------------------------------------------------------------
 * General:  Sets the alias parameter in Via header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    bAlias        - Indication if the 'alias' should be added to Via Header
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetAliasParam(
                             IN    RvSipViaHeaderHandle hSipViaHeader,
                             IN    RvBool               bAlias)
{
    MsgViaHeader* pHeader;

    if (hSipViaHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    pHeader = (MsgViaHeader*)hSipViaHeader;

    pHeader->bAlias = bAlias;

    return RV_OK;
}

/***************************************************************************
 * SipViaHeaderGetBranchParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Branch parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetBranchParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strBranchParam;
}

/***************************************************************************
 * RvSipViaHeaderGetBranchParam
 * ------------------------------------------------------------------------
 * General: Copies the Branch parameter from the Via header object into a
 *          given buffer. If the bufferLen size is adequate, the function
 *          copies the parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter contains
 *          the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetBranchParam(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetBranchParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipViaHeaderSetBranchParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BranchParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipViaHeader  - Handle of the Via header object.
 *            strBranchParam - The Branch parameter to be set in the object.
 *                           If Null, the exist BranchParam string in the
 *                           object will be removed.
 *          strOffset      - Offset of a string on the same page to use in an attach case.
 *                           (else - UNDEFINED)
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetBranchParam( IN    RvSipViaHeaderHandle hSipViaHeader,
                                      IN    RvChar*             strBranchParam,
                                      IN    HRPOOL               hPool,
                                      IN    HPAGE                hPage,
                                      IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgViaHeader*)hSipViaHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strBranchParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strBranchParam = newStr;
#ifdef SIP_DEBUG
    pHeader->pBranchParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strBranchParam);
#endif

    return RV_OK;
}
/***************************************************************************
 * RvSipViaHeaderSetBranchParam
 * ------------------------------------------------------------------------
 * General: Sets the branch parameter in the Via header object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hViaHeader       - Handle to the Via header object.
 *    strBranchParam   - The Branch parameter to be set in the object.
 *                     If NULL is supplied, the existing Branch string is
 *                     removed from the Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetBranchParam(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvChar*             strBranchParam)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetBranchParam(hSipViaHeader, strBranchParam, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipViaHeaderGetCompParam
 * ------------------------------------------------------------------------
 * General: Gets the 'comp' enumeration from the Via header object.
 *          If this function returns RVSIP_SIGCOMP_OTHER, call
 *          RvSipViaHeaderGetStrCompParam() to get the 'comp' parameter
 *          in a string format.
 *
 * Return Value: Returns the 'comp' enumeration of the Via
 *        header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hViaHeader - Handle to the Via header object.
 ***************************************************************************/
RVAPI RvSipCompType RVCALLCONV RvSipViaHeaderGetCompParam(
                                        IN  RvSipViaHeaderHandle hSipViaHeader)
{
    if(hSipViaHeader == NULL)
         return RVSIP_COMP_UNDEFINED;

    return ((MsgViaHeader*)hSipViaHeader)->eComp;
}

/***************************************************************************
 * SipViaHeaderGetStrCompParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the 'comp' parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrCompParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strCompParam;
}


/***************************************************************************
 * RvSipViaHeaderGetStrCompParam
 * ------------------------------------------------------------------------
 * General: Copies the Comp parameter from the Via header object into a
 *          given buffer. If the bufferLen size is adequate, the function
 *          copies the parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter contains
 *          the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or 
 *               RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetStrCompParam(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetStrCompParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetCompParam
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the comp in the MsgViaHeader
 *           object. User can set a comp enum value (in eComp) and no string,
 *           or to set eComp to be RVSIP_COMP_OTHER, and then put the
 *           comp value in strComp. the API will call it with NULL pool
 *           and page, to make a real allocation and copy. internal modules
 *           (such as parser) will call directly to this function, with valid
 *           pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 *    eComp         - The enum compression type value to be set in the object.
 *  strComp       - String of compType, in case eCompType is
 *                  RVSIP_COMP_OTHER. may be NULL.
 *  hPool         - The pool on which the string lays (if relevant).
 *  hPage         - The page on which the string lays (if relevant).
 *  strOffset     - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetCompParam(IN    RvSipViaHeaderHandle hSipViaHeader,
                                   IN    RvSipCompType        eComp,
                                   IN    RvChar*             strComp,
                                   IN    HRPOOL               hPool,
                                   IN    HPAGE                hPage,
                                   IN    RvInt32             strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader = (MsgViaHeader*)hSipViaHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader->eComp = eComp;

    if(eComp == RVSIP_COMP_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strComp,
                                      pHeader->hPool, pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strCompParam = newStr;
#ifdef SIP_DEBUG
        pHeader->pCompParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strCompParam);
#endif
    }
    else
    {
        pHeader->strCompParam = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pCompParam = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderSetCompParam
 * ------------------------------------------------------------------------
 * General:  Sets the compression type in the Via header object.
 *           You can use strTransport parameter only if the eComp parameter
 *           is set to RVSIP_COMP_OTHER. In this case you can supply the
 *           compression type as a string.
 *           If eComp is set to RVSIP_COMP_OTHER strComp is
 *           copied to the header. Otherwise, strComp is ignored.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    eComp         - The enumeration transport protocol to set in the object.
 *  strComp       - You can use this parameter only if the eComp
 *                  parameter is set to RVSIP_COMP_OTHER. In this case
 *                  you can supply the compression type as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetCompParam(
                                     IN    RvSipViaHeaderHandle hSipViaHeader,
                                     IN    RvSipCompType        eComp,
                                     IN    RvChar*             strComp)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetCompParam(hSipViaHeader,
                                    eComp,
                                    strComp,
                                    NULL,
                                    NULL_PAGE,
                                    UNDEFINED);
}

/***************************************************************************
 * SipViaHeaderGetStrSigCompIdParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the 'SigCompId' parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrSigCompIdParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strSigCompIdParam;
}


/***************************************************************************
 * RvSipViaHeaderGetSigCompIdParam
 * ------------------------------------------------------------------------
 * General: Copies the sigcomp-id parameter from the Via header object into a
 *          given buffer. If the bufferLen size is adequate, the function
 *          copies the parameter into the strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter contains
 *          the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetSigCompIdParam(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL && bufferLen != 0)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetStrSigCompIdParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetSigCompIdParam
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the sigcomp-id in the MsgViaHeader
 *           object. User can set the sigcomp-id value in strSigCompId. the API will 
 *			 call it with NULL pool and page, to make a real allocation and copy. 
 *			 internal modules (such as parser) will call directly to this function, with valid
 *           pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *  hSipViaHeader - Handle of the Via header object.
 *  strSigCompId  - String of SigCompId
 *  hPool         - The pool on which the string lays (if relevant).
 *  hPage         - The page on which the string lays (if relevant).
 *  strOffset     - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetSigCompIdParam(IN    RvSipViaHeaderHandle hSipViaHeader,
                                   IN    RvChar*	strSigCompId,
                                   IN    HRPOOL		hPool,
                                   IN    HPAGE		hPage,
                                   IN    RvInt32	strOffset)
{
    RvInt32      newStr;
    MsgViaHeader* pHeader = (MsgViaHeader*)hSipViaHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strSigCompId,
		pHeader->hPool, pHeader->hPage, &newStr);
	if (RV_OK != retStatus)
	{
		return retStatus;
	}
	pHeader->strSigCompIdParam = newStr;
#ifdef SIP_DEBUG
	pHeader->pSigCompIdParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
		pHeader->strSigCompIdParam);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderSetSigCompIdParam
 * ------------------------------------------------------------------------
 * General:  Sets the "sigcomp-id" parameter in the Via header object.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle to the Via header object.
 *    strSigCompId  - The SigCompId parameter to set
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetSigCompIdParam(
                                     IN    RvSipViaHeaderHandle hSipViaHeader,
                                     IN    RvChar*             strSigCompId)
{
    /* validity checking will be done in the internal function */
    return SipViaHeaderSetSigCompIdParam(hSipViaHeader,
                                    strSigCompId,
                                    NULL,
                                    NULL_PAGE,
                                    UNDEFINED);
}

/***************************************************************************
 * RvSipViaHeaderGetHiddenParam
 * ------------------------------------------------------------------------
 * General: This method gets the hidden parameter from the Via Header object.
 * Return Value: The hidden parameter value. If the hidden parameter exists
 *               in the Via Header then RV_TRUE will be returned, otherwise - RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipViaHeaderGetHiddenParam(IN RvSipViaHeaderHandle hSipViaHeader)
{
    if(hSipViaHeader == NULL)
    {
        return RV_FALSE;
    }
    return ((MsgViaHeader*)hSipViaHeader)->bIsHidden;
}

/***************************************************************************
 * RvSipViaHeaderSetHiddenParam
 * ------------------------------------------------------------------------
 * General: This method sets the hidden parameter in the Via Header object.
 *          If the hidden parameter exists in the Via Header RV_TRUE will be
 *          set, otherwise - RV_FALSE.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:   hSipViaHeader - Handle of the Via header object.
 *           hiddenValue   - The hidden parameter value to set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetHiddenParam(IN  RvSipViaHeaderHandle hSipViaHeader,
                                                        IN RvBool              hiddenValue)
{
    if(hSipViaHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    ((MsgViaHeader*)hSipViaHeader)->bIsHidden = hiddenValue;
    return RV_OK;
}

/***************************************************************************
 * SipViaHeaderGetViaParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the ViaParams value.
 * Return Value: branch string offset or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetViaParams(IN RvSipViaHeaderHandle hSipViaHeader)
{
    return ((MsgViaHeader*)hSipViaHeader)->strViaParams;
}

/***************************************************************************
 * RvSipViaHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Via header other params field of the Via header
 *          object into a given buffer.
 *          Not all the Via header parameters have separated fields in the
 *          Via header object.
 *          Parameters with no specific fields are refered to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer length.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 *  output:actualLen - The length of the requested parameter + 1, to
 *         include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetOtherParams(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*             strBuffer,
                                               IN RvUint              bufferLen,
                                               OUT RvUint*            actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetViaParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetViaParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ViaParams in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipViaHeader - Handle of the Via header object.
 *            strViaParams  - The via parameter to be set in the object.
 *                          If Null, the exist ViaParam string in the
 *                          object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetViaParams(IN    RvSipViaHeaderHandle hSipViaHeader,
                                   IN    RvChar*             strViaParams,
                                   IN    HRPOOL               hPool,
                                   IN    HPAGE                hPage,
                                   IN    RvInt32             strOffset)
{

    RvInt32      newStr;
    MsgViaHeader* pHeader;
    RvStatus     retStatus;

    if(hSipViaHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgViaHeader*)hSipViaHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strViaParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strViaParams = newStr;
#ifdef SIP_DEBUG
    pHeader->pViaParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strViaParams);
#endif

    return RV_OK;
}
/***************************************************************************
 * RvSipViaHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the Via header object.
 *          Not all the Via header parameters have separated fields in the
 *          Via header object. Parameters with no specific fields are refered
 *          to as other params. They are kept in the object in one concatenated
 *          string in the form- "name=value;name=value..."
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader    - Handle to the Via header object.
 *    strViaParams     - The Via Params string to be set in the Via header.
 *                     If NULL is supplied, the existing Other Params field
 *                     is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetOtherParams(
                                         IN    RvSipViaHeaderHandle hSipViaHeader,
                                         IN    RvChar*             strViaParams)
{

    /* validity checking will be done in the internal function */
    return SipViaHeaderSetViaParams(hSipViaHeader, strViaParams, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipViaHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrBadSyntax(
                                    IN RvSipViaHeaderHandle hHeader)
{
    return ((MsgViaHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipViaHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Via header,
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
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetStrBadSyntax(
                                               IN RvSipViaHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgViaHeader*)hHeader)->hPool,
                                  ((MsgViaHeader*)hHeader)->hPage,
                                  SipViaHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipViaHeaderSetStrBadSyntax
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
RvStatus SipViaHeaderSetStrBadSyntax(
                                  IN RvSipViaHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgViaHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgViaHeader*)hHeader;

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
 * RvSipViaHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Via header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetStrBadSyntax(
                                  IN RvSipViaHeaderHandle hHeader,
                                  IN RvChar*             strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipViaHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * SipViaHeaderIsEqual
 * ------------------------------------------------------------------------
 * General: Used to compare to two via headers
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNewVia  - first via header.
 *            hOldVia  - second via header.
 ***************************************************************************/
RvBool SipViaHeaderIsEqual(IN RvSipViaHeaderHandle hNewVia,
                            IN RvSipViaHeaderHandle hOldVia)
{
    RPOOL_Ptr oldVia;
    RPOOL_Ptr newVia;
    RvSipTransport newTransport,oldTransport;
    RvInt32       newPort,     oldPort;
    MsgViaHeader*  pHeader;

    pHeader = (MsgViaHeader*)hOldVia;
    /*compare the branch of the top via*/
    if(((hNewVia != NULL) && (hOldVia == NULL))||
       ((hNewVia == NULL) && (hOldVia != NULL)))
    {
           return RV_FALSE;
    }
    else if((hNewVia == NULL) && (hOldVia == NULL))
    {
           return RV_TRUE;
    }
	else if (hNewVia == hOldVia)
	{
		/* this is the same object */
		return RV_TRUE;
	}

	if (SipViaHeaderGetStrBadSyntax(hNewVia) != UNDEFINED ||
		SipViaHeaderGetStrBadSyntax(hOldVia) != UNDEFINED)
	{
		/* bad syntax string is uncomparable */
		return RV_FALSE;
	}

    /*Compare via transport*/
    newTransport = RvSipViaHeaderGetTransport(hNewVia);
    oldTransport = RvSipViaHeaderGetTransport(hOldVia);

    newTransport = (newTransport == RVSIP_TRANSPORT_UNDEFINED)? RVSIP_TRANSPORT_UDP:newTransport;
    oldTransport = (oldTransport == RVSIP_TRANSPORT_UNDEFINED)? RVSIP_TRANSPORT_UDP:oldTransport;

    if(newTransport !=  oldTransport)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - transport is different", hNewVia, hOldVia));
        return(RV_FALSE);
    }


    /*Compare via Port*/
    newPort = RvSipViaHeaderGetPortNum(hNewVia);
    oldPort = RvSipViaHeaderGetPortNum(hOldVia);

    newPort = (newPort == UNDEFINED) ? 5060 : newPort;
    oldPort = (oldPort == UNDEFINED) ? 5060 : oldPort;

    if(newPort != oldPort)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - port is different", hNewVia, hOldVia));
        return(RV_FALSE);
    }

    oldVia.hPool  = SipViaHeaderGetPool(hOldVia);
    oldVia.hPage  = SipViaHeaderGetPage(hOldVia);
    newVia.hPool  = SipViaHeaderGetPool(hNewVia);
    newVia.hPage  = SipViaHeaderGetPage(hNewVia);

    /*Compare Host*/
    oldVia.offset = SipViaHeaderGetHost(hOldVia);
    newVia.offset = SipViaHeaderGetHost(hNewVia);
    if(newVia.offset != UNDEFINED && oldVia.offset != UNDEFINED)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
        if(0 != SPIRENT_RPOOL_Stricmp(oldVia.hPool,oldVia.hPage,oldVia.offset,
                                  newVia.hPool,newVia.hPage,newVia.offset))
#else /* !defined(UPDATED_BY_SPIRENT) */
        if(RV_FALSE == RPOOL_Strcmp(oldVia.hPool,oldVia.hPage,oldVia.offset,
                                     newVia.hPool,newVia.hPage,newVia.offset))
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        {
            RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - host is different", hNewVia, hOldVia));
            return RV_FALSE;
        }
    }
    else if(newVia.offset == UNDEFINED && oldVia.offset != UNDEFINED)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - host is different", hNewVia, hOldVia));
        return RV_FALSE;
    }
    else if(newVia.offset != UNDEFINED && oldVia.offset == UNDEFINED)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - host is different", hNewVia, hOldVia));
        return RV_FALSE;
    }


    oldVia.offset = SipViaHeaderGetBranchParam(hOldVia);
    newVia.offset = SipViaHeaderGetBranchParam(hNewVia);
    if(newVia.offset != UNDEFINED && oldVia.offset != UNDEFINED)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        if(0 != SPIRENT_RPOOL_Stricmp(oldVia.hPool,oldVia.hPage,oldVia.offset,
            newVia.hPool,newVia.hPage,newVia.offset))
#else
        if(RV_FALSE == RPOOL_Stricmp(oldVia.hPool,oldVia.hPage,oldVia.offset,
            newVia.hPool,newVia.hPage,newVia.offset))
#endif
/* SPIRENT_END */
        {
            RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - branch is different", hNewVia, hOldVia));
            return RV_FALSE;

        }
    }
    else if(newVia.offset == UNDEFINED && oldVia.offset != UNDEFINED)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - branch is different", hNewVia, hOldVia));
        return RV_FALSE;
    }
    else if(newVia.offset != UNDEFINED && oldVia.offset == UNDEFINED)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are not equal - branch is different", hNewVia, hOldVia));
        return RV_FALSE;
    }

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "SipViaHeaderIsEqual - Header 0x%p and header 0x%p are equal", hNewVia, hOldVia));

    return(RV_TRUE);
}

/***************************************************************************
 * RvSipViaHeaderIsEqual
 * ------------------------------------------------------------------------
 * General: Compares two Via header objects. Via header fields are considered equal if
 *         their hosts are equal (via case-sensitive comparison), their branches are equal
 *         (via case-insensitive comparison), their transports are equal (no transport
 *         is considered to be UDP) and their ports are equal (no port is considered to 
 *         be 5060).
 * Return Value: Returns RV_TRUE if the Via header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the Via header object.
 *    hOtherHeader - Handle to the Via header object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderIsEqual(
												const RvSipViaHeaderHandle hHeader,
												const RvSipViaHeaderHandle hOtherHeader)
{
    if(hHeader == NULL || hOtherHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipViaHeaderIsEqual(hHeader, hOtherHeader);

}

/***************************************************************************
 * SipViaHeaderCompareBranchParams
 * ------------------------------------------------------------------------
 * General: Compares two branches
 * Return Value: RV_TRUE if the branches are the same, RV_FALSE - otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hViaHeader      - Handle to the first Via header object.
 *           hOtherViaHeader - Handle to the second Via header object.
 ***************************************************************************/
RvBool SipViaHeaderCompareBranchParams(IN RvSipViaHeaderHandle hViaHeader,
                                        IN RvSipViaHeaderHandle hOtherViaHeader)
{
    RPOOL_Ptr viaBranchPtr;
    RPOOL_Ptr otherViaBranchPtr;
    RvBool   bAreEqual;
    MsgViaHeader* pHeader;

    pHeader = (MsgViaHeader*)hViaHeader;
    viaBranchPtr.hPool = SipViaHeaderGetPool(hViaHeader);
    otherViaBranchPtr.hPool = SipViaHeaderGetPool(hOtherViaHeader);
    viaBranchPtr.hPage = SipViaHeaderGetPage(hViaHeader);
    otherViaBranchPtr.hPage = SipViaHeaderGetPage(hOtherViaHeader);
    viaBranchPtr.offset = SipViaHeaderGetBranchParam(hViaHeader);
    otherViaBranchPtr.offset = SipViaHeaderGetBranchParam(hOtherViaHeader);

    if(viaBranchPtr.offset != UNDEFINED && otherViaBranchPtr.offset != UNDEFINED)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        bAreEqual = (SPIRENT_RPOOL_Stricmp(viaBranchPtr.hPool, viaBranchPtr.hPage, viaBranchPtr.offset,
            otherViaBranchPtr.hPool, otherViaBranchPtr.hPage, otherViaBranchPtr.offset) == 0);
#else
        bAreEqual = RPOOL_Stricmp(viaBranchPtr.hPool, viaBranchPtr.hPage, viaBranchPtr.offset,
            otherViaBranchPtr.hPool, otherViaBranchPtr.hPage, otherViaBranchPtr.offset);
#endif
/* SPIRENT_END */
    }
    else if(viaBranchPtr.offset == UNDEFINED)
    {
        if(otherViaBranchPtr.offset == UNDEFINED)
        {
            bAreEqual = RV_TRUE;
        }
        else
        {
            bAreEqual = RV_FALSE;
        }
    }
    else /* otherViaBranchPtr.offset == UNDEFINED && viaBranchPtr.offset != UNDEFINED */
    {
        bAreEqual = RV_FALSE;
    }
    if(bAreEqual == RV_FALSE)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipViaHeaderCompareBranchParams - Header 0x%p and header 0x%p branches are not equal", hViaHeader, hOtherViaHeader));
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipViaHeaderCompareBranchParams - Header 0x%p and header 0x%p branches are equal", hViaHeader, hOtherViaHeader));
    }
    return bAreEqual;
}

/***************************************************************************
 * SipViaHeaderCompareSentByParams
 * ------------------------------------------------------------------------
 * General: Compares two sent-by params
 * Return Value: RV_TRUE if the sent-by params are the same, RV_FALSE - otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hViaHeader      - Handle to the first Via header object.
 *           hOtherViaHeader - Handle to the second Via header object.
***************************************************************************/
RvBool SipViaHeaderCompareSentByParams(IN RvSipViaHeaderHandle hViaHeader,
                                        IN RvSipViaHeaderHandle hOtherViaHeader)
{
    RPOOL_Ptr viaHostPtr;
    RPOOL_Ptr otherViaHostPtr;
    RvInt32  port1;
    RvInt32  port2;
    MsgViaHeader* pHeader;

    pHeader = (MsgViaHeader*)hViaHeader;
    viaHostPtr.hPool = SipViaHeaderGetPool(hViaHeader);
    otherViaHostPtr.hPool = SipViaHeaderGetPool(hOtherViaHeader);
    viaHostPtr.hPage = SipViaHeaderGetPage(hViaHeader);
    otherViaHostPtr.hPage = SipViaHeaderGetPage(hOtherViaHeader);
    viaHostPtr.offset = SipViaHeaderGetHost(hViaHeader);
    otherViaHostPtr.offset = SipViaHeaderGetHost(hOtherViaHeader);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    if(SPIRENT_RPOOL_Stricmp(viaHostPtr.hPool, viaHostPtr.hPage, viaHostPtr.offset,
        otherViaHostPtr.hPool, otherViaHostPtr.hPage, otherViaHostPtr.offset) != 0)
#else
    if(RPOOL_Stricmp(viaHostPtr.hPool, viaHostPtr.hPage, viaHostPtr.offset,
        otherViaHostPtr.hPool, otherViaHostPtr.hPage, otherViaHostPtr.offset) == RV_FALSE)
#endif
/* SPIRENT_END */
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipViaHeaderCompareSentByParams - Header 0x%p and header 0x%p host is not equal", hViaHeader, hOtherViaHeader));
        return RV_FALSE;
    }
    port1 = RvSipViaHeaderGetPortNum(hViaHeader);
    port2 = RvSipViaHeaderGetPortNum(hOtherViaHeader);
    if(port1 == port2)
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipViaHeaderCompareSentByParams - Header 0x%p and header 0x%p sent-by param is equal", hViaHeader, hOtherViaHeader));
        return RV_TRUE;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipViaHeaderCompareSentByParams - Header 0x%p and header 0x%p port is not equal", hViaHeader, hOtherViaHeader));
        return RV_FALSE;
    }
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipViaHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the via header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipViaHeader - Handle to the Via header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetRpoolString(
                             IN    RvSipViaHeaderHandle      hSipViaHeader,
                             IN    RvSipViaHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgViaHeader* pHeader;
    RvInt32      requestedParamOffset;

    if(hSipViaHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgViaHeader*)hSipViaHeader;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipViaHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_VIA_HOST:
        requestedParamOffset = pHeader->strHost;
        break;
    case RVSIP_VIA_MADDR_PARAM:
        requestedParamOffset = pHeader->strMaddrParam;
        break;
    case RVSIP_VIA_RECEIVED_PARAM:
        requestedParamOffset = pHeader->strReceivedParam;
        break;
    case RVSIP_VIA_BRANCH_PARAM:
        requestedParamOffset = pHeader->strBranchParam;
        break;
    case RVSIP_VIA_OTHER_PARAMS:
        requestedParamOffset = pHeader->strViaParams;
        break;
    case RVSIP_VIA_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
	case RVSIP_VIA_SIGCOMPID_PARAM:
		requestedParamOffset = pHeader->strSigCompIdParam;
		break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipViaHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipViaHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}
#endif /* #ifndef RV_SIP_PRIMITIVES */
/***************************************************************************
 * RvSipViaHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Via header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipViaHeader - Handle to the Via header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetRpoolString(
                             IN    RvSipViaHeaderHandle      hSipViaHeader,
                             IN    RvSipViaHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hSipViaHeader;
    if(pHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipViaHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_VIA_HOST:
        return SipViaHeaderSetHost(hSipViaHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_VIA_MADDR_PARAM:
        return SipViaHeaderSetMaddrParam(hSipViaHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_VIA_RECEIVED_PARAM:
        return SipViaHeaderSetReceivedParam(hSipViaHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_VIA_BRANCH_PARAM:
        return SipViaHeaderSetBranchParam(hSipViaHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_VIA_OTHER_PARAMS:
        return SipViaHeaderSetViaParams(hSipViaHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipViaHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/***************************************************************************
 * RvSipViaHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Via header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderSetCompactForm(
                                   IN    RvSipViaHeaderHandle hHeader,
                                   IN    RvBool                bIsCompact)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;
    if(pHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipViaHeaderSetCompactForm - Setting compact form of Header 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}

/***************************************************************************
 * RvSipViaHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Via header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipViaHeaderGetCompactForm(
                                   IN    RvSipViaHeaderHandle hHeader,
                                   IN    RvBool              *pbIsCompact)
{
    MsgViaHeader* pHeader = (MsgViaHeader*)hHeader;
    if(pHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    *pbIsCompact = pHeader->bIsCompact;
    return RV_OK;

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ViaHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ViaHeaderClean(IN MsgViaHeader* pHeader,
						   IN RvBool        bCleanBS)
{
    pHeader->ttlNum             = UNDEFINED;
    pHeader->strBranchParam     = UNDEFINED;
    pHeader->strMaddrParam      = UNDEFINED;
    pHeader->strReceivedParam   = UNDEFINED;
    pHeader->rportParam         = UNDEFINED;
    pHeader->bRportExists       = RV_FALSE;
    pHeader->bIsHidden          = RV_FALSE;
    pHeader->strViaParams       = UNDEFINED;
    pHeader->eHeaderType        = RVSIP_HEADERTYPE_VIA;
    pHeader->eComp              = RVSIP_COMP_UNDEFINED;
    pHeader->strCompParam       = UNDEFINED;
	pHeader->strSigCompIdParam  = UNDEFINED;
    pHeader->bAlias             = RV_FALSE;
#ifdef SIP_DEBUG
    pHeader->pBranchParam       = NULL;
    pHeader->pMaddrParam        = NULL;
    pHeader->pReceivedParam     = NULL;
    pHeader->pViaParams         = NULL;
    pHeader->pCompParam         = NULL;
	pHeader->pSigCompIdParam    = NULL;
#endif /* SIP_DEBUG */

	if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->strHost          = UNDEFINED;
        pHeader->eTransport       = RVSIP_TRANSPORT_UNDEFINED;
        pHeader->strTransport     = UNDEFINED;
        pHeader->strBadSyntax     = UNDEFINED;
        pHeader->portNum          = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
        pHeader->pHost            = NULL;
        pHeader->pTransport       = NULL;
#endif
    }
}




#ifdef __cplusplus
}
#endif

