/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAuthorizationHeader.c                             *
 *                                                                            *
 * The file defines the methods of the Authorization header object:           *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Oren Libis         Jan. 2001                                            *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "RvSipAuthorizationHeader.h"
#include "_SipAuthorizationHeader.h"
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
static void AuthorizationHeaderClean(IN MsgAuthorizationHeader* pHeader,
                                     IN RvBool                 bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAuthorizationHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Authorization header object inside a given message object. The
 *          header is kept in the header list of the message. You can choose to insert the
 *          header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Authorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderConstructInMsg(
                                   IN  RvSipMsgHandle                  hSipMsg,
                                   IN  RvBool                         pushHeaderAtHead,
                                   OUT RvSipAuthorizationHeaderHandle *hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipAuthorizationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_AUTHORIZATION,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipAuthorizationHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Authorization Header object. The header
 *          is constructed on a given page taken from a specified pool. The handle to the
 *          new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Authorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderConstruct(
                                           IN  RvSipMsgMgrHandle                hMsgMgr,
                                           IN  HRPOOL                           hPool,
                                           IN  HPAGE                            hPage,
                                           OUT RvSipAuthorizationHeaderHandle   *hHeader)
{
    MsgAuthorizationHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgAuthorizationHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                             hPool,
                                                             hPage,
                                                             sizeof(MsgAuthorizationHeader),
                                                             RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
     pHeader->eAuthHeaderType = RVSIP_AUTH_UNDEFINED_AUTHORIZATION_HEADER;
    AuthorizationHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAuthorizationHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderConstruct - Authorization header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            pMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Authorization header object to a destination
 *          Authorization header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Authorization header object.
 *    hSource      - Handle to the source Authorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderCopy(
									 IN    RvSipAuthorizationHeaderHandle hDestination,
                                     IN    RvSipAuthorizationHeaderHandle hSource)
{
    MsgAuthorizationHeader* source, *dest;
    RvStatus                status;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgAuthorizationHeader*)hSource;
    dest   = (MsgAuthorizationHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /*header Type*/
    status  = RvSipAuthorizationHeaderSetHeaderType(hDestination,
                                                     source->eAuthHeaderType);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderCopy: Failed to copy Authorization header type. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }
    /* authentication scheme */
    /*status =  RvSipAuthorizationHeaderSetAuthScheme(hDestination,
                                                    source->eAuthScheme,
                                                    source->strAuthScheme);*/
    dest->eAuthScheme = source->eAuthScheme;
    if((source->strAuthScheme > UNDEFINED) && (dest->eAuthScheme == RVSIP_AUTH_SCHEME_OTHER))
    {
        dest->strAuthScheme = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAuthScheme);
        if (dest->strAuthScheme == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy authentication scheme. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAuthScheme = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strAuthScheme);
#endif
    }
    else
    {
         dest->strAuthScheme = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAuthScheme = NULL;
#endif
    }

    /* realm */
    /*status =  RvSipAuthorizationHeaderSetRealm(hDestination,
                                               source->strRealm);*/
    if(source->strRealm > UNDEFINED)
    {
        dest->strRealm = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strRealm);

        if (dest->strRealm == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy realm. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pRealm = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                    dest->strRealm);
#endif
    }
    else
    {
        dest->strRealm = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pRealm = NULL;
#endif
    }

    /* user name */
    /*status =  RvSipAuthorizationHeaderSetUserName(hDestination,
                                                  source->strUserName);*/
    if(source->strUserName > UNDEFINED)
    {
        dest->strUserName = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strUserName);
        if (dest->strUserName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy user name. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pUserName = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                       dest->strUserName);
#endif
    }
    else
    {
        dest->strUserName = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pUserName = NULL;
#endif
    }

    /* nonce */
    /*status =  RvSipAuthorizationHeaderSetNonce(hDestination,
                                                source->strNonce);*/
    if(source->strNonce > UNDEFINED)
    {
        dest->strNonce = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strNonce);
        if (dest->strNonce == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy nonce. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pNonce = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                       dest->strNonce);
#endif
    }
    else
    {
        dest->strNonce = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pNonce = NULL;
#endif
    }

    /* opaque */
    /*status =  RvSipAuthorizationHeaderSetOpaque(hDestination,
                                                source->strOpaque);*/
    if(source->strOpaque > UNDEFINED)
    {
        dest->strOpaque = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strOpaque);

        if (dest->strOpaque == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy opaque. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOpaque = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                       dest->strOpaque);
#endif
    }
    else
    {
        dest->strOpaque = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOpaque = NULL;
#endif
    }

    /* qop */
    status =  RvSipAuthorizationHeaderSetQopOption(hDestination,
                                                   source->eAuthQop);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderCopy: Failed to copy qop. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }

    /* auth param */
    /*status =  RvSipAuthorizationHeaderSetAuthParam(hDestination,
                                                   source->strAuthParam);*/
    if(source->strAuthParam > UNDEFINED)
    {
        dest->strAuthParam = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strAuthParam);
        if (dest->strAuthParam == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy AuthParam. hDest 0x%p, status %d",
                hDestination, status));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAuthParam = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strAuthParam);
#endif
    }
    else
    {
        dest->strAuthParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAuthParam = NULL;
#endif
    }

    /* cnonce */
    /*status =  RvSipAuthorizationHeaderSetCNonce(hDestination,
                                                source->strCnonce);*/
    if(source->strCnonce > UNDEFINED)
    {
        dest->strCnonce = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strCnonce);
        if (dest->strCnonce == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy CNonce. hDest 0x%p, status %d",
                hDestination, status));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCnonce = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strCnonce);
#endif
    }
    else
    {
        dest->strCnonce = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCnonce = NULL;
#endif
    }

    /* nonce count */
    status =  RvSipAuthorizationHeaderSetNonceCount(hDestination,
                                                    source->nonceCount);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderCopy: Failed to copy nonce count. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }

    /* algorithm */
    /*tatus =  RvSipAuthorizationHeaderSetAuthAlgorithm(hDestination,
                                                       source->eAlgorithm,
                                                       source->strAlgorithm);*/
    dest->eAlgorithm = source->eAlgorithm;
    if(source->strAlgorithm > UNDEFINED)
    {
        dest->strAlgorithm = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                             dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAlgorithm);
        if (dest->strAlgorithm == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy authentication algorithm. hDest 0x%p, status %d",
                hDestination, status));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAlgorithm = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strAlgorithm);
#endif
    }
    else
    {
        dest->strAlgorithm = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAlgorithm = NULL;
#endif
    }

    /* digest-uri */
    status =  RvSipAuthorizationHeaderSetDigestUri(hDestination,
                                                   source->hDigestUri);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderCopy: Failed to copy digest uri. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }

/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    /* orig-uri */
    if(source->strOriginalUri > UNDEFINED)
    {
        dest->strOriginalUri = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strOriginalUri);
        
        if (dest->strOriginalUri == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy strOriginalUri. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        dest->strOriginalUri = UNDEFINED;
    }
/*#endif*/ /*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    /* response */
    /*status =  RvSipAuthorizationHeaderSetResponse(hDestination,
                                                  source->strResponse);*/
    if(source->strResponse > UNDEFINED)
    {
        dest->strResponse = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strResponse);
        if (dest->strResponse == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy response. hDest 0x%p, status %d",
                hDestination, status));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pResponse = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strResponse);
#endif
    }
    else
    {
        dest->strResponse = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pResponse = NULL;
#endif
    }

	/* auts */
    if(source->strAuts > UNDEFINED)
    {
        dest->strAuts = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strAuts);
        if (dest->strAuts == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderCopy: Failed to copy Auts. hDest 0x%p, status %d",
                hDestination, status));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAuts = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                        dest->strAuts);
#endif
    }
    else
    {
        dest->strAuts = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAuts = NULL;
#endif
    }

	/* Integrity Protected */
	dest->eProtected = source->eProtected;

	/* AKA Version */
    dest->nAKAVersion = source->nAKAVersion;

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
                "RvSipAuthorizationHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipAuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Authorization header object to a textual Authorization header. The
 *          textual header is placed on a page taken from a specified pool. In order to copy
 *          the textual header from the page to a consecutive buffer, use
 *          RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Authorization header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderEncode(
                                          IN    RvSipAuthorizationHeaderHandle  hHeader,
                                          IN    HRPOOL                          hPool,
                                          OUT   HPAGE*                          hPage,
                                          OUT   RvUint32*                      length)
{
    RvStatus stat;
    MsgAuthorizationHeader* pHeader;

    pHeader = (MsgAuthorizationHeader*)hHeader;
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
                "RvSipAuthorizationHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AuthorizationHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderEncode - Failed. Free page 0x%p on pool 0x%p. AuthorizationHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Authorization header (as string) on it.
 *          format: "WWW-Authenticate: " AuthScheme digest-challenge
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
RvStatus RVCALLCONV AuthorizationHeaderEncode(
                                      IN    RvSipAuthorizationHeaderHandle  hHeader,
                                      IN    HRPOOL                          hPool,
                                      IN    HPAGE                           hPage,
                                      IN    RvBool                bInUrlHeaders,
                                      INOUT RvUint32*                      length)
{
    MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)hHeader;
    RvStatus                stat;
    RvChar                  tempNc[9];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AuthorizationHeaderEncode - Encoding Authorization header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    if (pHeader->eAuthHeaderType == RVSIP_AUTH_AUTHORIZATION_HEADER)
    {
        /* encode "Authorization " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Authorization", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        /* encode "Proxy-Authorization: " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Proxy-Authorization", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
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
                "AuthorizationHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* Authorization scheme */
    stat = MsgUtilsEncodeAuthScheme(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->eAuthScheme,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strAuthScheme,
                                    length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* space after the authentication scheme */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetSpaceChar(bInUrlHeaders), length);
    /* if the auth scheme is not digest, encode only the auth params */
    if (pHeader->eAuthScheme != RVSIP_AUTH_SCHEME_DIGEST)
    {
        /* set the auth-params */
        if(pHeader->strAuthParam > UNDEFINED)
        {
            /* set auth parmas */
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                        hPool,
                                        hPage,
                                        pHeader->hPool,
                                        pHeader->hPage,
                                        pHeader->strAuthParam,
                                        length);
        }

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        return RV_OK;
    }


    /* set "username =" username, */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "username", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
    if(pHeader->strUserName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strUserName,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }

    }
    else
    {
         /* "" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",realm =" realm, */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "realm", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
    if(pHeader->strRealm > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strRealm,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        /* "" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",nonce =" nonce */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "nonce", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
    if(pHeader->strNonce > UNDEFINED)
    {

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strNonce,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",uri =" digest-uri */
    if(pHeader->hDigestUri != NULL)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "uri", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        /* first " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
        /* orig-uri */
        if(pHeader->strOriginalUri > UNDEFINED)
        {
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,hPool, hPage, 
                                         pHeader->hPool, pHeader->hPage, pHeader->strOriginalUri, length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "AuthorizationHeaderEncode - Failed to encode orig uri in Authorization header. RvStatus is %d, hPool 0x%p, hPage %d",
                    stat, hPool, hPage));
                return stat;
            }
        }
        else
        {
/*#endif #ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/

            stat = AddrEncode(pHeader->hDigestUri, hPool, hPage, bInUrlHeaders, RV_FALSE, length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                    stat, hPool, hPage));
                return stat;
            }
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
        }
/*#endif #ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/

        /* ending " */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",response =" response */
    /* "," */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "response", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
    if(pHeader->strResponse != UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strResponse,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        /* "" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",algorithm =" algorithm */
    if (pHeader->eAlgorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "algorithm", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);

		/* Check the AKA Version and add it to the header if defined */
		if(pHeader->nAKAVersion > UNDEFINED)
		{
			RvChar strHelper[32];
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "AKAv", length);
			
			MsgUtils_itoa(pHeader->nAKAVersion, strHelper);
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);

			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "-", length);

            if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
                return stat;
			}
		}
		
        stat = MsgUtilsEncodeAuthAlgorithm(pHeader->pMsgMgr,
                                           hPool,
                                           hPage,
                                           pHeader->eAlgorithm,
                                           pHeader->hPool,
                                           pHeader->hPage,
                                           pHeader->strAlgorithm,
                                           length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",cnonce =" cnonce */
    if (pHeader->strCnonce > UNDEFINED)
    {
        /* "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "cnonce", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCnonce,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
     }

    /* set ",opaque =" opaque */
    if (pHeader->strOpaque > UNDEFINED)
    {
        /* "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "opaque", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOpaque,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ,"qop=" qop */
    if (pHeader->eAuthQop != RVSIP_AUTH_QOP_OTHER &&
        pHeader->eAuthQop != RVSIP_AUTH_QOP_UNDEFINED)
    {
        /* "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "qop", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeQopOptions(pHeader->pMsgMgr, hPool, hPage, pHeader->eAuthQop, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ,"nc =" nonce-count */
    if (pHeader->nonceCount != UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "nc", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
        RvSprintf(tempNc, "%08x", pHeader->nonceCount);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, tempNc, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

	/* set ",integrity-protected =" integrity-protected */
    if (pHeader->eProtected != RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetCommaChar(bInUrlHeaders),length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "integrity-protected", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders),length);
        
		if(pHeader->eProtected == RVSIP_AUTH_INTEGRITY_PROTECTED_YES)
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "yes",length);
		else
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "no",length);

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

	/* set: ,auts="auts-value" */
    if (pHeader->strAuts > UNDEFINED)
    {
        /* ",auts=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetCommaChar(bInUrlHeaders),length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "auts", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),length);

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strAuts,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set "," auth-param */
    if (pHeader->strAuthParam > UNDEFINED)
    {
        /* "," */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetCommaChar(bInUrlHeaders),length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strAuthParam,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthorizationHeaderEncode - Failed to encode Authorization header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Authorization header into an Authorization header object.
 *          All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Authorization header object.
 *    buffer    - Buffer containing a textual Authorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderParse(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN RvChar*                        buffer)
{
    MsgAuthorizationHeader* pHeader = (MsgAuthorizationHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    AuthorizationHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_AUTHORIZATION,
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
 * RvSipAuthorizationHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Authorization header value into an Authorization header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAuthorizationHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Authorization header object.
 *    buffer    - The buffer containing a textual Authorization header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderParseValue(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN RvChar*                        buffer)
{
    MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)hHeader;
    SipParseType             eParseType;
	RvStatus                 rv;

    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    if (pHeader->eAuthHeaderType == RVSIP_AUTH_PROXY_AUTHORIZATION_HEADER)
	{
		eParseType = SIP_PARSETYPE_PROXY_AUTHORIZATION;
	}
	else
	{
		eParseType = SIP_PARSETYPE_AUTHORIZATION;
	}
	
    AuthorizationHeaderClean(pHeader, RV_FALSE);
	
    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        eParseType,
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
 * RvSipAuthorizationHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Authorization header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderFix(
                                     IN RvSipAuthorizationHeaderHandle hHeader,
                                     IN RvChar*                       pFixedBuffer)
{
    MsgAuthorizationHeader* pHeader = (MsgAuthorizationHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipAuthorizationHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthorizationHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAuthorizationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Authorization object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthorizationHeaderGetPool(RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAuthorizationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Authorization object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthorizationHeaderGetPage(RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Authorization header fields are kept in a string format-for
 *          example, the Authorization header user name string. In order to get such a field
 *          from the Authorization header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Authorization header object.
 *  eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAuthorizationHeaderGetStringLength(
                                      IN  RvSipAuthorizationHeaderHandle     hHeader,
                                      IN  RvSipAuthorizationHeaderStringName eStringName)
{
    RvInt32    stringOffset;
    MsgAuthorizationHeader* pHeader = (MsgAuthorizationHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (eStringName)
    {
        case RVSIP_AUTHORIZATION_AUTHSCHEME:
        {
            stringOffset = SipAuthorizationHeaderGetStrAuthScheme(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_USERNAME:
        {
            stringOffset = SipAuthorizationHeaderGetUserName(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_REALM:
        {
            stringOffset = SipAuthorizationHeaderGetRealm(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_NONCE:
        {
            stringOffset = SipAuthorizationHeaderGetNonce(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_OPAQUE:
        {
            stringOffset = SipAuthorizationHeaderGetOpaque(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_OTHER_PARAMS:
        {
            stringOffset = SipAuthorizationHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_RESPONSE:
        {
            stringOffset = SipAuthorizationHeaderGetResponse(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_CNONCE:
        {
            stringOffset = SipAuthorizationHeaderGetCNonce(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_ALGORITHM:
        {
            stringOffset = SipAuthorizationHeaderGetStrAuthAlgorithm(hHeader);
            break;
        }
		case RVSIP_AUTHORIZATION_AUTS:
        {
            stringOffset = SipAuthorizationHeaderGetStrAuts(hHeader);
            break;
        }
        case RVSIP_AUTHORIZATION_BAD_SYNTAX:
        {
            stringOffset = SipAuthorizationHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetCredentialIdentifier
 * ------------------------------------------------------------------------
 * General: Gets the Identifiers of the credentials Authentication header.
 *          This function will return the realm and username strings,
 *          without quotation marks!!
 *          If you want this parameters in quotation marks, use the specific
 *          get functions, and not this function.
 *          If the realmLen or usernameLen is adequate, the function copies the
 *          strings into strRealm and strUsername buffers,
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and realmActualLen
 *          and usernameActualLen contain the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader          - Handle to the header object.
 *        strRealmBuff     - Buffer to fill with the realm parameter.
 *        realmBuffLen     - The length of the given realm buffer.
 *        strUsernameBuff  - Buffer to fill with the username parameter.
 *        usernameBuffLen  - The length of the username given buffer.
 *        strNonceBuff     - Buffer to fill with the nonce parameter. might be NULL,
 *                           if user don't need this information.
 *        nonceBuffLen     - The length of the nonce given buffer.
 * output:realmActualLen   - The length of the realm parameter, + 1 to include
 *                           a NULL value at the end.
 *        usernameActualLen- The length of the username parameter, + 1 to include
 *                           a NULL value at the end.
 *        nonceActualLen   - The length of the nonce parameter, + 1 to include
 *                           a NULL value at the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetCredentialIdentifier(
                               IN  RvSipAuthorizationHeaderHandle hHeader,
                               IN  RvChar*                       strRealmBuff,
                               IN  RvUint                        realmBuffLen,
                               IN  RvChar*                       strUsernameBuff,
                               IN  RvUint                        usernameBuffLen,
                               IN  RvChar*                       strNonceBuff,
                               IN  RvUint                        nonceBuffLen,
                               OUT RvUint*                       realmActualLen,
                               OUT RvUint*                       usernameActualLen,
                               OUT RvUint*                       nonceActualLen)
{
    MsgAuthorizationHeader* pHeader = (MsgAuthorizationHeader*)hHeader;
    RvStatus               tempStatus1, tempStatus2, tempStatus3;

    if(realmActualLen == NULL || usernameActualLen == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *realmActualLen = *usernameActualLen = 0;
    tempStatus1 = tempStatus2 = tempStatus3 = RV_OK;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* realm - we give offset + 1 to skip the first quotation mark */
    tempStatus1 = MsgUtilsFillUserBuffer(pHeader->hPool,
                                        pHeader->hPage,
                                        (pHeader->strRealm) + 1,
                                        strRealmBuff,
                                        realmBuffLen,
                                        realmActualLen);
    if(tempStatus1 == RV_OK)
    {
        /* set NULL instead of last quotation mark. */
        strRealmBuff[*realmActualLen - 2] = '\0';
    }

    /* username */
    tempStatus2 = MsgUtilsFillUserBuffer(pHeader->hPool,
                                        pHeader->hPage,
                                        (pHeader->strUserName) + 1,
                                        strUsernameBuff,
                                        usernameBuffLen,
                                        usernameActualLen);
    if(tempStatus2 == RV_OK)
    {
        /* set NULL instead of last quotation mark. */
        strUsernameBuff[*usernameActualLen - 2] = '\0';
    }

    /* nonce */
    if(strNonceBuff != NULL)
    {
        if(nonceActualLen == NULL)
        {
            return RV_ERROR_NULLPTR;
        }
        *nonceActualLen = 0;
        tempStatus3 = MsgUtilsFillUserBuffer(pHeader->hPool,
                                        pHeader->hPage,
                                        (pHeader->strNonce) + 1,
                                        strNonceBuff,
                                        nonceBuffLen,
                                        nonceActualLen);
        if(tempStatus3 == RV_OK)
        {
            /* set NULL instead of last quotation mark. */
            strNonceBuff[*nonceActualLen - 2] = '\0';
        }
    }

    if(tempStatus1 != RV_OK || tempStatus2 != RV_OK || tempStatus3 != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderGetCredentialIdentifier - returns RV_ERROR_INSUFFICIENT_BUFFER"));
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetAuthScheme
 * ------------------------------------------------------------------------
 * General: Gets the Authentication scheme enumeration value from the Authorization
 *          header object.
 * Return Value: The Unauthentication scheme enumeration from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvSipAuthScheme RVCALLCONV RvSipAuthorizationHeaderGetAuthScheme(
                                      IN  RvSipAuthorizationHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_AUTH_SCHEME_UNDEFINED;

    return ((MsgAuthorizationHeader*)hHeader)->eAuthScheme;
}

/***************************************************************************
 * SipAuthorizationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: This method retrieves the Authorization scheme string value from the
 *          Authorization object.
 * Return Value: text string offset giving the Authorization scheme, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuthScheme(
                                            IN  RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strAuthScheme;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: Gets the Authentication scheme string value from the Authorization header
 *          object.
 *          Use this function if RvSipAuthorizationHeaderGetAuthScheme() returns
 *          RVSIP_AUTH_SCHEME_OTHER.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetStrAuthScheme(
                                       IN  RvSipAuthorizationHeaderHandle hHeader,
                                       IN  RvChar*                       strBuffer,
                                       IN  RvUint                        bufferLen,
                                       OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetStrAuthScheme(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipAuthorizationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthScheme in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader       - Handle of the Authorization header object.
 *        eAuthScheme   - The Authorization scheme to be set in the object.
 *          strAuthScheme - text string giving the method type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetAuthScheme(
                                  IN    RvSipAuthorizationHeaderHandle  hHeader,
                                  IN    RvSipAuthScheme                 eAuthScheme,
                                  IN    RvChar*                        strAuthScheme,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset)
{
    MsgAuthorizationHeader*   pHeader;
    RvInt32                  newStr;
    RvStatus                 retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    pHeader->eAuthScheme = eAuthScheme;

    if(eAuthScheme == RVSIP_AUTH_SCHEME_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strAuthScheme,
                                      pHeader->hPool, pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strAuthScheme = newStr;
#ifdef SIP_DEBUG
        pHeader->pAuthScheme = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                            pHeader->strAuthScheme);
#endif
    }
    else
    {
        pHeader->strAuthScheme = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pAuthScheme = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: Sets the Authentication scheme in the Authorization header object.
 *          If eAuthScheme is RVSIP_AUTH_SCHEME_OTHER, strAuthScheme is
 *          copied to the header. Otherwise, strAuthScheme is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Authorization header object.
 *  eAuthScheme   - The Authentication scheme to be set in the object.
 *    strAuthScheme - Text string giving the Authentication scheme to be set in the object. Use this
 *                  parameter only if eAuthScheme is RVSIP_AUTH_SCHEME_OTHER.
 *                  Otherwise, the parameter is set to NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetAuthScheme(
                                      IN RvSipAuthorizationHeaderHandle  hHeader,
                                      IN RvSipAuthScheme                 eAuthScheme,
                                      IN RvChar*                        strAuthScheme)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetAuthScheme(hHeader,
                                               eAuthScheme,
                                               strAuthScheme,
                                               NULL,
                                               NULL_PAGE,
                                               UNDEFINED);
}


/***************************************************************************
 * SipAuthorizationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General:This method gets the realm string from the Authorization header.
 * Return Value: realm offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetRealm(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strRealm;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General: Copies the realm string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetRealm(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetRealm(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the realm string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pRealm - The realm to be set in the Authorization header - If
 *                      NULL, the exist realm string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetRealm(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pRealm,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32                newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pRealm,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strRealm = newStr;
#ifdef SIP_DEBUG
    pHeader->pRealm = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strRealm);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General:Sets the realm string in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authorization header object.
 *    pRealm   - The realm string to be set in the Authorization header. If NULL is supplied, the
 *             existing realm is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetRealm(
                                     IN RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                       pRealm)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetRealm(hHeader, pRealm, NULL, NULL_PAGE, UNDEFINED);
}



/***************************************************************************
 * SipAuthorizationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the Authorization header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetNonce(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strNonce;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General: Copies the Nonce string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetNonce(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetNonce(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pNonce - The nonce to be set in the Authorization header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetNonce(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32               newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus              retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pNonce,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strNonce = newStr;
#ifdef SIP_DEBUG
    pHeader->pNonce = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strNonce);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General:Sets the nonce string in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authorization header object.
 *    pNonce   - The nonce string to be set in the Authorization header. If a NULL value is
 *             supplied, the existing nonce string in the header object is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetNonce(
                                     IN RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                       pNonce)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetNonce(hHeader, pNonce, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthorizationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General:This method gets the opaque string from the Authorization header.
 * Return Value: opaque offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOpaque(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strOpaque;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General: Copies the opaque string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetOpaque(
                                           IN RvSipAuthorizationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetOpaque(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the opaque string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pOpaque - The opaque to be set in the Authorization header - If
 *                      NULL, the exist opaque string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOpaque(
                                     IN    RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvChar                       *pOpaque,
                                     IN    HRPOOL                         hPool,
                                     IN    HPAGE                          hPage,
                                     IN    RvInt32                       strOffset)
{
    RvInt32                newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOpaque,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strOpaque = newStr;
#ifdef SIP_DEBUG
    pHeader->pOpaque = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strOpaque);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General:Sets the opaque string in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authorization header object.
 *    pOpaque  - The opaque string to be set in the Authorization header. If a NULL value is
 *             supplied, the existing opaque string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetOpaque(
                                     IN RvSipAuthorizationHeaderHandle hHeader,
                                     IN RvChar *                       pOpaque)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetOpaque(hHeader, pOpaque, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthorizationHeaderGetUserName
 * ------------------------------------------------------------------------
 * General:This method gets the user name string from the Authorization header.
 * Return Value: user name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetUserName(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strUserName;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetUserName
 * ------------------------------------------------------------------------
 * General: Copies the user name string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetUserName(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetUserName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetUserName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the user name string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pUserName    - The user name to be set in the Authorization header - If
 *                      NULL, the exist user name string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetUserName(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pUserName,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32                newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pUserName,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strUserName = newStr;
#ifdef SIP_DEBUG
    pHeader->pUserName = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strUserName);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetUserName
 * ------------------------------------------------------------------------
 * General:Sets the user name string in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Authorization header object.
 *    pUserName - The user name string to be set in the Authorization header. If a NULL value is
 *              supplied, the existing user name string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetUserName(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN RvChar *                       pUserName)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetUserName(hHeader, pUserName, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthorizationHeaderGetStrAuts
 * ------------------------------------------------------------------------
 * General:This method gets the Auts string from the Authorization header.
 * Return Value: Auts offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuts(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strAuts;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetStrAuts
 * ------------------------------------------------------------------------
 * General: Copies the Auts string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetStrAuts(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetStrAuts(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetStrAuts
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Auts string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader    - Handle of the Authorization header object.
 *         pAuts	- The Auts to be set in the Authorization header - If
 *                    NULL, the exist Auts string in the header will be removed.
 *        strOffset - Offset of a string on the page  (if relevant).
 *        hPool		- The pool on which the string lays (if relevant).
 *        hPage		- The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetStrAuts(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pAuts,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32					newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus				retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pAuts,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strAuts = newStr;
#ifdef SIP_DEBUG
    pHeader->pAuts = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
											pHeader->strAuts);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetStrAuts
 * ------------------------------------------------------------------------
 * General:Sets the nonce string in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authorization header object.
 *    pAuts	   - The Auts string to be set in the Authorization header. If a NULL value is
 *				 supplied, the existing Auts string in the header object is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetStrAuts(
                                     IN RvSipAuthorizationHeaderHandle hHeader,
                                     IN	RvChar *					   pAuts)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetStrAuts(hHeader, pAuts, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetAKAVersion
 * ------------------------------------------------------------------------
 * General: Gets the AKAVersion from the Authorization header object.
 * Return Value: Returns the AKAVersion, or UNDEFINED if the AKAVersion
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAuthorizationHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAuthorizationHeaderGetAKAVersion(
                                         IN RvSipAuthorizationHeaderHandle hAuthorizationHeader)
{
    if(hAuthorizationHeader == NULL)
        return UNDEFINED;

    return ((MsgAuthorizationHeader*)hAuthorizationHeader)->nAKAVersion;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetAKAVersion
 * ------------------------------------------------------------------------
 * General:  Sets the AKAVersion number of the Authorization header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAuthorizationHeader - Handle to the Authorization header object.
 *    AKAVersion	       - The AKAVersion number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetAKAVersion(
                                         IN    RvSipAuthorizationHeaderHandle	hAuthorizationHeader,
                                         IN    RvInt32							AKAVersion)
{
    if(hAuthorizationHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgAuthorizationHeader*)hAuthorizationHeader)->nAKAVersion = AKAVersion;
    return RV_OK;
}

/***************************************************************************
 * SipAuthorizationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the AuthParam string from the Authorization header.
 * Return Value: AuthParam offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOtherParams(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strAuthParam;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other params field of the Authorization
 *          header object into a given buffer.
 *          Not all the Authorization header parameters have separated fields in the
 *          Authorization header object. Parameters with no specific fields are refered to as
 *          other params. They are kept in the object in one concatenated string in the
 *          form-"name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authorization header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetOtherParams(
                                           IN RvSipAuthorizationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the AuthParam string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pAuthParam - The AuthParam to be set in the Authorization header - If
 *                      NULL, the exist AuthParam string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOtherParams(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pAuthParam,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32                newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pAuthParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strAuthParam = newStr;
#ifdef SIP_DEBUG
    pHeader->pAuthParam = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strAuthParam);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Authorization header object.
 *         Not all the Party header parameters have separated fields in the Party header
 *         object. Parameters with no specific fields are refered to as other params. They
 *         are kept in the object in one concatenated string in the form-
 *         "name=value;name=value...".
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Authorization header object.
 *    pOtherParams  - The other params field to be set in the Authorization header. If NULL is
 *                  supplied, the existing other params field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetOtherParams(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar *                    strOtherParams)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetOtherParams(hHeader, strOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipAuthorizationHeaderGetNonceCount
 * ------------------------------------------------------------------------
 * General: Gets the nonce count value from the Authorization Header object.
 * Return Value: Returns the nonce count value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAuthorizationHeaderGetNonceCount(
                                    IN RvSipAuthorizationHeaderHandle hSipAuthHeader)
{
    if(hSipAuthHeader == NULL)
        return UNDEFINED;

    return ((MsgAuthorizationHeader*)hSipAuthHeader)->nonceCount;
}


/***************************************************************************
 * RvSipAuthorizationHeaderSetNonceCount
 * ------------------------------------------------------------------------
 * General: Sets the nonce count value in the Authorization Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader          - Handle to the Authorization header object.
 *    nonceCount              - The nonce count value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetNonceCount(
                                 IN    RvSipAuthorizationHeaderHandle hSipAuthHeader,
                                 IN    RvInt32                      nonceCount)
{
    if(hSipAuthHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgAuthorizationHeader*)hSipAuthHeader)->nonceCount = nonceCount;
    return RV_OK;
}


/***************************************************************************
 * RvSipAuthorizationHeaderGetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Gets the authentication algorithm enumeration from the Authorization header
 *          object.
 *          if this function returns RVSIP_AUTH_ALGORITHM_OTHER, call
 *          RvSipAuthorizationHeaderGetStrAuthAlgorithm() to get the algorithm in a
 *          string format.
 * Return Value: Returns the authentication algorithm enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvSipAuthAlgorithm RVCALLCONV RvSipAuthorizationHeaderGetAuthAlgorithm(
                                      IN  RvSipAuthorizationHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_AUTH_ALGORITHM_UNDEFINED;

    return ((MsgAuthorizationHeader*)hHeader)->eAlgorithm;
}

/***************************************************************************
 * SipAuthorizationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This method retrieves the Authorization algorithm string value from the
 *          Authorization object.
 * Return Value: text string giving the Authorization scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuthAlgorithm(
                                            IN  RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strAlgorithm;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Copies the authentication algorithm string from the Authorization header object
 *          into a given buffer. Use this function if
 *          RvSipAuthorizationHeaderGetAuthAlgorithm() returns
 *          RVSIP_AUTH_ALGORITHM_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetStrAuthAlgorithm(
                                           IN  RvSipAuthorizationHeaderHandle  hHeader,
                                           IN  RvChar*                        strBuffer,
                                           IN  RvUint                         bufferLen,
                                           OUT RvUint*                        actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetStrAuthAlgorithm(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipAuthorizationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthAlgorithm in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Authorization header object.
 *        eAuthAlgorithm   - The Authorization algorithm to be set in the object.
 *          strAuthAlgorithm - text string giving the algorithm type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetAuthAlgorithm(
                                  IN    RvSipAuthorizationHeaderHandle  hHeader,
                                  IN    RvSipAuthAlgorithm              eAuthAlgorithm,
                                  IN    RvChar*                        strAuthAlgorithm,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset)
{
    MsgAuthorizationHeader     *header;
    RvInt32                    newStr;
    RvStatus                   retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAuthorizationHeader*)hHeader;

    header->eAlgorithm = eAuthAlgorithm;

    if(eAuthAlgorithm == RVSIP_AUTH_ALGORITHM_OTHER)
    {
        retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strOffset, strAuthAlgorithm,
                                      header->hPool, header->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        header->strAlgorithm = newStr;
#ifdef SIP_DEBUG
        header->pAlgorithm = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                         header->strAlgorithm);
#endif
    }
    else
    {
        header->strAlgorithm = UNDEFINED;
#ifdef SIP_DEBUG
        header->pAlgorithm = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Sets the authentication algorithm in Authorization header object.
 *          If eAuthAlgorithm is RVSIP_AUTH_SCHEME_OTHER, strAuthAlgorithm is
 *          copied to the header. Otherwise, strAuthAlgorithm is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Authorization header object.
 *  eAuthAlgorithm    - The Authentication algorithm to be set in the object.
 *    strAuthAlgorithm  - Text string giving the Authorization algorithm to be set in the object. You can
 *                      use this parameter if eAuthAlgorithm is set to
 *                      RVSIP_AUTH_ALGORITHM_OTHER. Otherwise, the parameter is set to
 *                      NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetAuthAlgorithm(
                                  IN     RvSipAuthorizationHeaderHandle     hHeader,
                                  IN    RvSipAuthAlgorithm                 eAuthAlgorithm,
                                  IN    RvChar                            *strAuthAlgorithm)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetAuthAlgorithm(hHeader,
                                                  eAuthAlgorithm,
                                                  strAuthAlgorithm,
                                                  NULL,
                                                  NULL_PAGE,
                                                  UNDEFINED);

}


/***************************************************************************
 * RvSipAuthorizationHeaderGetQopOption
 * ------------------------------------------------------------------------
 * General: Gets the Qop option enumeration from the Authorization object.
 * Return Value: The qop type from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Returns the Qop option enumeration from the object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipAuthorizationHeaderGetQopOption(
                                      IN  RvSipAuthorizationHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_AUTH_QOP_UNDEFINED;

    return ((MsgAuthorizationHeader*)hHeader)->eAuthQop;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetQopOption
 * ------------------------------------------------------------------------
 * General: Sets the Qop option enumeration in the Authorization Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authorization header object.
 *    eQop           - The Qop option to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetQopOption(
                                 IN    RvSipAuthorizationHeaderHandle hSipAuthHeader,
                                 IN    RvSipAuthQopOption             eQop)
{
    if(hSipAuthHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgAuthorizationHeader*)hSipAuthHeader)->eAuthQop = eQop;
    return RV_OK;
}


/***************************************************************************
 * SipAuthorizationHeaderGetResponse
 * ------------------------------------------------------------------------
 * General:This method gets the Response string from the Authorization header.
 * Return Value: Response offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetResponse(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strResponse;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetResponse
 * ------------------------------------------------------------------------
 * General: Copies the Response string from the Authorization header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetResponse(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetResponse(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetResponse
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Response string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pResponse - The Response to be set in the Authorization header - If
 *                      NULL, the exist Response string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetResponse(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN RvChar                         *pResponse,
                                     IN HRPOOL                          hPool,
                                     IN HPAGE                           hPage,
                                     IN RvInt32                        strOffset)
{
    RvInt32                newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus               retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pResponse,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strResponse = newStr;
#ifdef SIP_DEBUG
    pHeader->pResponse = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strResponse);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetResponse
 * ------------------------------------------------------------------------
 * General: Sets the Response string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Authorization header object.
 *    pResponse  - The Response string to be set in the Authorization header. If a NULL value is
 *               supplied, the existing Response string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetResponse(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar *                       pResponse)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetResponse(hHeader, pResponse, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthorizationHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General:This method gets the CNonce string from the Authorization header.
 * Return Value: CNonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetCNonce(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strCnonce;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General: Copies the CNonce string from the Authorization header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetCNonce(
                                           IN RvSipAuthorizationHeaderHandle  hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetCNonce(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the CNonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pCNonce - The CNonce to be set in the Authorization header - If
 *                      NULL, the exist CNonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetCNonce(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pCNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32               newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus              retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pCNonce,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCnonce = newStr;
#ifdef SIP_DEBUG
    pHeader->pCnonce = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strCnonce);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General: Sets the CNonce string in the Authorization object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authorization header object.
 *    pCNonce  - The CNonce string to be set in the Authorization header. If a NULL value is
 *             supplied, the existing CNonce string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetCNonce(
                                     IN    RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                       pCNonce)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetCNonce(hHeader, pCNonce, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipAuthorizationHeaderGetDigestUri
 * ------------------------------------------------------------------------
 * General: Gets the handle to the address object digest Uri field from the
 *          Authorization header object.
 * Return Value: Returns a handle to the Address object, or NULL if the Address object does not
 *               exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipAuthorizationHeaderGetDigestUri
                                            (IN RvSipAuthorizationHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgAuthorizationHeader*)hHeader)->hDigestUri;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetDigestUri
 * ------------------------------------------------------------------------
 * General: Sets the digest uri parameter in the Authorization header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Authorization header object
 *  hDigestUri - Handle to the Address object. If a NULL is supplied, the existing addressSpec in
 *               the Authorization header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetDigestUri
                                    (IN    RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvSipAddressHandle             hDigestUri)
{
    RvStatus               stat;
    MsgAddress             *pAddr   = (MsgAddress*)hDigestUri;
    MsgAuthorizationHeader *pHeader = (MsgAuthorizationHeader*)hHeader;
    RvSipAddressType        currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hDigestUri == NULL)
    {
        pHeader->hDigestUri = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
               return RV_ERROR_BADPARAM;
        }
        if (NULL != pHeader->hDigestUri)
        {
            currentAddrType = RvSipAddrGetAddrType(pHeader->hDigestUri);
        }

        /* if no address object was allocated, we will construct it */
        if((pHeader->hDigestUri == NULL) ||
           (pAddr->eAddrType != currentAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInAuthorizationHeader(hHeader,
                                                           pAddr->eAddrType,
                                                           &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }
        return RvSipAddrCopy(pHeader->hDigestUri,
                             hDigestUri);
    }
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Authorization Header object.
 * Return Value: Returns the Authorization header type enumeration from the authorization
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authorization header object.
 ***************************************************************************/
RVAPI RvSipAuthorizationHeaderType  RVCALLCONV RvSipAuthorizationHeaderGetHeaderType(
                                              IN RvSipAuthorizationHeaderHandle hSipAuthHeader)
{
    if(hSipAuthHeader == NULL)
        return RVSIP_AUTH_UNDEFINED_AUTHORIZATION_HEADER;

    return ((MsgAuthorizationHeader*)hSipAuthHeader)->eAuthHeaderType;
}


/***************************************************************************
 * RvSipAuthorizationHeaderSetHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Authorization Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader    - Handle to the Authorization header object.
 *    eHeaderType       - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetHeaderType(
                                 IN    RvSipAuthorizationHeaderHandle hSipAuthHeader,
                                 IN    RvSipAuthorizationHeaderType   eHeaderType)
{
    if(hSipAuthHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgAuthorizationHeader*)hSipAuthHeader)->eAuthHeaderType = eHeaderType;
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetIntegrityProtected
 * ------------------------------------------------------------------------
 * General: Gets the IntegrityProtected option enumeration from the Authorization object.
 * Return Value: The IntegrityProtected type from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - The Header from which to get the IntegrityProtected enumeration.
 ***************************************************************************/
RVAPI RvSipAuthIntegrityProtected RVCALLCONV RvSipAuthorizationHeaderGetIntegrityProtected(
                                      IN  RvSipAuthorizationHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;

    return ((MsgAuthorizationHeader*)hHeader)->eProtected;
}

/***************************************************************************
 * RvSipAuthorizationHeaderSetIntegrityProtected
 * ------------------------------------------------------------------------
 * General: Sets the IntegrityProtected enumeration in the Authorization Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader		- Handle to the Authorization header object.
 *    eIntegrityProtected   - The IntegrityProtected value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetIntegrityProtected(
                                 IN    RvSipAuthorizationHeaderHandle	hSipAuthHeader,
                                 IN    RvSipAuthIntegrityProtected		eIntegrityProtected)
{
    if(hSipAuthHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgAuthorizationHeader*)hSipAuthHeader)->eProtected = eIntegrityProtected;
    return RV_OK;
}

/***************************************************************************
 * SipAuthorizationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrBadSyntax(
                                            IN  RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAuthorizationHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Authorization header,
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
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetStrBadSyntax(
                                           IN  RvSipAuthorizationHeaderHandle  hHeader,
                                           IN  RvChar*                        strBuffer,
                                           IN  RvUint                         bufferLen,
                                           OUT RvUint*                        actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgAuthorizationHeader*)hHeader)->hPool,
                                  ((MsgAuthorizationHeader*)hHeader)->hPage,
                                  SipAuthorizationHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthorizationHeaderSetStrBadSyntax
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
RvStatus SipAuthorizationHeaderSetStrBadSyntax(
                                  IN RvSipAuthorizationHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgAuthorizationHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgAuthorizationHeader*)hHeader;

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
 * RvSipAuthorizationHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Authorization header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetStrBadSyntax(
                                  IN RvSipAuthorizationHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipAuthorizationHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipAuthorizationHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Authorization header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthHeader - Handle to the Authorization header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Authorization header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderGetRpoolString(
                             IN    RvSipAuthorizationHeaderHandle      hSipAuthHeader,
                             IN    RvSipAuthorizationHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgAuthorizationHeader* pHeader = (MsgAuthorizationHeader*)hSipAuthHeader;
    RvInt32      requestedParamOffset;

    if(hSipAuthHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthorizationHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_AUTHORIZATION_AUTHSCHEME:
        requestedParamOffset = pHeader->strAuthScheme;
        break;
    case RVSIP_AUTHORIZATION_USERNAME:
        requestedParamOffset = pHeader->strUserName;
        break;
    case RVSIP_AUTHORIZATION_REALM:
        requestedParamOffset = pHeader->strRealm;
        break;
    case RVSIP_AUTHORIZATION_NONCE:
        requestedParamOffset = pHeader->strNonce;
        break;
    case RVSIP_AUTHORIZATION_CNONCE:
        requestedParamOffset = pHeader->strCnonce;
        break;
   case RVSIP_AUTHORIZATION_OPAQUE:
        requestedParamOffset = pHeader->strOpaque;
        break;
    case RVSIP_AUTHORIZATION_RESPONSE:
        requestedParamOffset = pHeader->strResponse;
        break;
    case RVSIP_AUTHORIZATION_ALGORITHM:
        requestedParamOffset = pHeader->strAlgorithm;
        break;
	case RVSIP_AUTHORIZATION_AUTS:
        requestedParamOffset = pHeader->strAuts;
        break;
    case RVSIP_AUTHORIZATION_OTHER_PARAMS:
        requestedParamOffset = pHeader->strAuthParam;
        break;
    case RVSIP_AUTHORIZATION_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthorizationHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipAuthorizationHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipAuthorizationHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Authorization header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthHeader - Handle to the Authorization header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthorizationHeaderSetRpoolString(
                             IN RvSipAuthorizationHeaderHandle      hSipAuthHeader,
                             IN RvSipAuthorizationHeaderStringName  eStringName,
                             IN RPOOL_Ptr                           *pRpoolPtr)
{
    MsgAuthorizationHeader* pHeader;

    pHeader = (MsgAuthorizationHeader*)hSipAuthHeader;
    if(hSipAuthHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipAuthorizationHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }
    switch(eStringName)
    {
    case RVSIP_AUTHORIZATION_AUTHSCHEME:
        return SipAuthorizationHeaderSetAuthScheme(hSipAuthHeader,RVSIP_AUTH_SCHEME_OTHER,
                                               NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_USERNAME:
        return SipAuthorizationHeaderSetUserName(hSipAuthHeader,NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_REALM:
        return SipAuthorizationHeaderSetRealm(hSipAuthHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_NONCE:
        return SipAuthorizationHeaderSetNonce(hSipAuthHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_CNONCE:
        return SipAuthorizationHeaderSetCNonce(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_OPAQUE:
        return SipAuthorizationHeaderSetOpaque(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_RESPONSE:
        return SipAuthorizationHeaderSetResponse(hSipAuthHeader,NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_OTHER_PARAMS:
        return SipAuthorizationHeaderSetOtherParams(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHORIZATION_ALGORITHM:
        return SipAuthorizationHeaderSetAuthAlgorithm(hSipAuthHeader,RVSIP_AUTH_ALGORITHM_OTHER,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
	case RVSIP_AUTHORIZATION_AUTS:
        return SipAuthorizationHeaderSetUserName(hSipAuthHeader,NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);

    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthorizationHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
/***************************************************************************
 * SipAuthorizationHeaderSetOrigUri
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pDisplayName - The nonce to be set in the Authorization header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOrigUri(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pOrigUri,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset)
{
    RvInt32               newStr;
    MsgAuthorizationHeader *pHeader;
    RvStatus              retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOrigUri,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strOriginalUri = newStr;

    return RV_OK;
}

/***************************************************************************
 * SipAuthorizationHeaderGetOrigUri
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the Authorization header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOrigUri(
                                    IN RvSipAuthorizationHeaderHandle hHeader)
{
    return ((MsgAuthorizationHeader*)hHeader)->strOriginalUri;
}

/*#endif #ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AuthorizationHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AuthorizationHeaderClean( IN MsgAuthorizationHeader* pHeader,
                                      IN RvBool                 bCleanBS)
{
    pHeader->eAlgorithm      = RVSIP_AUTH_ALGORITHM_UNDEFINED;
    pHeader->eAuthScheme     = RVSIP_AUTH_SCHEME_UNDEFINED;
    pHeader->strAuthScheme   = UNDEFINED;
    pHeader->strRealm        = UNDEFINED;
    pHeader->strNonce        = UNDEFINED;
    pHeader->strCnonce       = UNDEFINED;
    pHeader->nonceCount      = UNDEFINED;
    pHeader->strOpaque       = UNDEFINED;
    pHeader->eAlgorithm      = RVSIP_AUTH_ALGORITHM_UNDEFINED;
    pHeader->strAlgorithm    = UNDEFINED;
    pHeader->eAuthQop        = RVSIP_AUTH_QOP_UNDEFINED;
    pHeader->strUserName     = UNDEFINED;
    pHeader->strResponse     = UNDEFINED;
    pHeader->hDigestUri      = NULL;
    pHeader->strAuthParam    = UNDEFINED;
    pHeader->eHeaderType     = RVSIP_HEADERTYPE_AUTHORIZATION;
	pHeader->strAuts	     = UNDEFINED;
	pHeader->eProtected	     = RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;
	pHeader->nAKAVersion	 = UNDEFINED;
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    pHeader->strOriginalUri = UNDEFINED;
/*#endif*/ /*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/

#ifdef SIP_DEBUG
    pHeader->pAuthScheme	= NULL;
    pHeader->pRealm			= NULL;
    pHeader->pNonce			= NULL;
    pHeader->pCnonce		= NULL;
    pHeader->pOpaque		= NULL;
    pHeader->pAlgorithm		= NULL;
    pHeader->pUserName		= NULL;
    pHeader->pResponse		= NULL;
    pHeader->pAuthParam		= NULL;
	pHeader->pAuts			= NULL;
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
