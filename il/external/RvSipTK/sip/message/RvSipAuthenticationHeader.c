/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                      RvSipAuthenticationHeader.c                           *
 *                                                                            *
 * The file defines the methods of the Authentication header object:          *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Oren Libis         Jan. 2001                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON
#include "RvSipAuthenticationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AuthenticationHeaderClean( IN MsgAuthenticationHeader* pHeader,
                                    IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAuthenticationHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Authentication header object inside a given message object. The
 *          header is kept in the header list of the message. You can choose to insert the
 *          header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Authentication header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderConstructInMsg(
                                   IN  RvSipMsgHandle                  hSipMsg,
                                   IN  RvBool                         pushHeaderAtHead,
                                   OUT RvSipAuthenticationHeaderHandle *hHeader)
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

    stat = RvSipAuthenticationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_AUTHENTICATION,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipAuthenticationHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Authentication Header object. The
 *          header is constructed on a given page taken from a specified pool. The handle to
 *          the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle of the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Authentication header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderConstruct(
                                           IN  RvSipMsgMgrHandle                hMsgMgr,
                                           IN  HRPOOL                           hPool,
                                           IN  HPAGE                            hPage,
                                           OUT RvSipAuthenticationHeaderHandle* hHeader)
{
    MsgAuthenticationHeader*   pHeader;
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

    pHeader = (MsgAuthenticationHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                              hPool,
                                                              hPage,
                                                              sizeof(MsgAuthenticationHeader),
                                                              RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr         = pMsgMgr;
    pHeader->hPage           = hPage;
    pHeader->hPool           = hPool;
    pHeader->eAuthHeaderType = RVSIP_AUTH_UNDEFINED_AUTHENTICATION_HEADER;

    AuthenticationHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipAuthenticationHeaderHandle)pHeader;
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderConstruct - Authentication header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hMsgMgr, hPool, hPage, *hHeader));
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Authentication header object to a destination
 *          Authentication header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Authentication header object.
 *    hSource      - Handle to the source Authentication header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderCopy(
                                     IN RvSipAuthenticationHeaderHandle hDestination,
                                     IN RvSipAuthenticationHeaderHandle hSource)
{
    MsgAuthenticationHeader* source, *dest;
    RvStatus                status;

    if ((hDestination == NULL)||(hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgAuthenticationHeader*)hSource;
    dest   = (MsgAuthenticationHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /*header Type*/
    status  = RvSipAuthenticationHeaderSetHeaderType(hDestination,
                                                     source->eAuthHeaderType);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderCopy: Failed to copy authentication header type. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }
    /* authentication scheme */
    /*status =  RvSipAuthenticationHeaderSetAuthScheme(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication scheme. hDest 0x%p",
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
    /*status =  RvSipAuthenticationHeaderSetRealm(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy realm. hDest 0x%p",
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

    /* domain */
    /*status =  RvSipAuthenticationHeaderSetDomain(hDestination,
                                                 source->strDomain);*/
    if(source->strDomain > UNDEFINED)
    {
        dest->strDomain = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strDomain);

        if (dest->strDomain == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderCopy: Failed to copy domain. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pDomain = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                     dest->strDomain);
#endif
    }
    else
    {
        dest->strDomain = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDomain = NULL;
#endif
    }

    /* nonce */
    /*status =  RvSipAuthenticationHeaderSetNonce(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy nonce. hDest 0x%p",
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
    /*status =  RvSipAuthenticationHeaderSetOpaque(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy opaque. hDest 0x%p",
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
    /*status =  RvSipAuthenticationHeaderSetStrQop(hDestination,
                                                 source->strQop);*/
    if(source->strQop > UNDEFINED)
    {
        dest->strQop = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                       dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strQop);
        if (dest->strQop == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderCopy: Failed to copy qop. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pQop = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                     dest->strQop);
#endif
    }
    else
    {
        dest->strQop = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pQop = NULL;
#endif
    }

    /* qop type */
    status =  RvSipAuthenticationHeaderSetQopOption(hDestination,
                                                    source->eAuthQop);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderCopy: Failed to copy qop options. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }

    /* auth param */
    /*status =  RvSipAuthenticationHeaderSetAuthParam(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy AuthParam. hDest 0x%p, status %d",
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

    /* stale */
    status =  RvSipAuthenticationHeaderSetStale(hDestination,
                                                source->eStale);
    if (status != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderCopy: Failed to copy stale. hDest 0x%p, status %d",
            hDestination, status));
        return status;
    }

    /* algorithm */
    /*status =  RvSipAuthenticationHeaderSetAuthAlgorithm(hDestination,
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
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication algorithm. hDest 0x%p, status %d",
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

    /* username */
    /*status =  SipAuthenticationHeaderSetUserName(hDestination,
                                                 source->strUserName,
                                                 RV_FALSE);*/
    dest->strUserName = source->strUserName;
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
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication user name. hDest 0x%p",
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

    /* password */
    /*status =  SipAuthenticationHeaderSetPassword(hDestination,
                                                 source->strPassword,
                                                 RV_FALSE);*/
    dest->strPassword = source->strPassword;
    if(source->strPassword > UNDEFINED)
    {
        dest->strPassword = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strPassword);
        if (dest->strPassword == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication user name. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPassword = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strPassword);
#endif
    }
    else
    {
        dest->strPassword = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPassword = NULL;
#endif
    }

	/* AKA Version */
    dest->nAKAVersion = source->nAKAVersion;

	/* strIntegrityKey */
    if(source->strIntegrityKey > UNDEFINED)
    {
        dest->strIntegrityKey = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strIntegrityKey);
        if (dest->strIntegrityKey == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication strIntegrityKey. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pIntegrityKey = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strIntegrityKey);
#endif
    }
    else
    {
        dest->strIntegrityKey = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIntegrityKey = NULL;
#endif
    }

	/* strCipherKey */
    if(source->strCipherKey > UNDEFINED)
    {
        dest->strCipherKey = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strCipherKey);
        if (dest->strCipherKey == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderCopy: Failed to copy authentication strCipherKey. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCipherKey = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strCipherKey);
#endif
    }
    else
    {
        dest->strCipherKey = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCipherKey = NULL;
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
                "RvSipAuthenticationHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipAuthenticationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Authentication header object to a textual Authentication header. The
 *          textual header is placed on a page taken from a specified pool. In order to copy
 *          the textual header from the page to a consecutive buffer, use
 *          RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage(). The
 *          allocated page must be freed only if this function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Authentication header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderEncode(
                                          IN    RvSipAuthenticationHeaderHandle hHeader,
                                          IN    HRPOOL                          hPool,
                                          OUT   HPAGE*                          hPage,
                                          OUT   RvUint32*                      length)
{
    RvStatus stat;
    MsgAuthenticationHeader* pHeader;

    pHeader = (MsgAuthenticationHeader*)hHeader;

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
                "RvSipAuthenticationHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AuthenticationHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderEncode - Failed. Free page 0x%p on pool 0x%p. AuthenticationHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * AuthenticationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Authentication header (as string) on it.
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
RvStatus RVCALLCONV AuthenticationHeaderEncode(
                                       IN    RvSipAuthenticationHeaderHandle hHeader,
                                       IN    HRPOOL                          hPool,
                                       IN    HPAGE                           hPage,
                                       IN    RvBool                bInUrlHeaders,
                                       INOUT RvUint32*                      length)
{
    MsgAuthenticationHeader* pHeader = (MsgAuthenticationHeader*)hHeader;
    RvStatus                stat;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "AuthenticationHeaderEncode - Encoding Authentication header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    if (pHeader->eAuthHeaderType == RVSIP_AUTH_WWW_AUTHENTICATION_HEADER)
    {
        /* encode "WWW-Authenticate" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "WWW-Authenticate", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header (header name). RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }
    else
    {
        /* encode "Proxy-Authenticate" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Proxy-Authenticate", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header (header name). RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* : or = */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                            MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders), length);

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
                "AuthenticationHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }


    /* authentication scheme */
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
            "AuthenticationHeaderEncode - Failed to encode Authentication header (scheme). RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        return RV_OK;
    }

    /* set "realm =" realm, */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "realm", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthenticationHeaderEncode - Failed to encode Authentication header (realm). RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* = */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetEqualChar(bInUrlHeaders), length);

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
                "AuthenticationHeaderEncode - Failed to encode Authentication header (realm). RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set "domain =" domain, */
    if (pHeader->strDomain > UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "domain", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* = */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetEqualChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDomain,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",nonce =" nonce */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "nonce", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetEqualChar(bInUrlHeaders), length);

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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",opaque =" opaque */
    if (pHeader->strOpaque > UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "opaque", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}
		
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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",stale =" stale */
    if (pHeader->eStale != RVSIP_AUTH_STALE_UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "stale", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}

        if (pHeader->eStale == RVSIP_AUTH_STALE_TRUE)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "true", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "false", length);
        }
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

    /* set ",algorithm =" algorithm */
    if (pHeader->eAlgorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "algorithm", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}

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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }

    }

    /* set ",qop=" qop */
    if (pHeader->eAuthQop != RVSIP_AUTH_QOP_UNDEFINED)
    {
        /* ,qop= */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                         MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "qop", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}

        if (pHeader->eAuthQop == RVSIP_AUTH_QOP_OTHER)
        {
            if (pHeader->strQop != UNDEFINED)
            {
               stat = MsgUtilsEncodeStringExt(pHeader->pMsgMgr,
                    hPool, hPage, pHeader->hPool, pHeader->hPage,
                    pHeader->strQop, RV_TRUE /*bSetQuotes*/, bInUrlHeaders,
                    length);
                if(stat != RV_OK)
                {
                    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                        "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                        stat, hPool, hPage));
                    return stat;
                }
            }
        }
        else
        {
            /* Add opening '"' */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                          MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                    stat, hPool, hPage));
                return stat;
            }

            stat = MsgUtilsEncodeQopOptions(pHeader->pMsgMgr, hPool, hPage, pHeader->eAuthQop, length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                    stat, hPool, hPage));
                return stat;
            }
            if (pHeader->strQop != UNDEFINED)
            {
                stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                         MsgUtilsGetCommaChar(bInUrlHeaders), length);
                if(stat != RV_OK)
                {
                    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                        "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                        stat, hPool, hPage));
                    return stat;
                }
                stat = MsgUtilsEncodeStringExt(pHeader->pMsgMgr, hPool, hPage,
                    pHeader->hPool, pHeader->hPage, pHeader->strQop,
                    RV_FALSE /*bSetQuotes*/, bInUrlHeaders, length);
                if(stat != RV_OK)
                {
                    RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                        "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                        stat, hPool, hPage));
                    return stat;
                }
            }
            /* Add closing '"' */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                          MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                    stat, hPool, hPage));
                return stat;
            }
        }
    }

	/* set ",ik =" integrity-key */
    if (pHeader->strIntegrityKey > UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "ik", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strIntegrityKey,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
    }

	/* set ",ck =" cipher-key */
    if (pHeader->strCipherKey > UNDEFINED)
    {
        /* , */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                MsgUtilsGetCommaChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "ck", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
		{
            return stat;
		}

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCipherKey,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
		}
    }

    /* set ,"auth-param=" auth-param */
    if (pHeader->strAuthParam > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                         MsgUtilsGetCommaChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "AuthenticationHeaderEncode - Failed to encode Authentication header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Authentication header into an Authentication header object.
 *         All the textual fields are placed inside the object.
 *         You must construct an Authentication header before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Authentication header object.
 *    buffer    - Buffer containing a textual Authentication header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderParse(
                                     IN RvSipAuthenticationHeaderHandle hHeader,
                                     IN RvChar*                        buffer)
{
    MsgAuthenticationHeader* pHeader = (MsgAuthenticationHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    AuthenticationHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_WWW_AUTHENTICATE,
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
 * RvSipAuthenticationHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Authentication header value into an
 *          Authentication header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAuthenticationHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Authentication header object.
 *    buffer    - The buffer containing a textual Authentication header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderParseValue(
                                     IN RvSipAuthenticationHeaderHandle hHeader,
                                     IN RvChar*                        buffer)
{
    MsgAuthenticationHeader* pHeader = (MsgAuthenticationHeader*)hHeader;
    RvStatus                 rv;
	SipParseType             eParseType;

    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    if (pHeader->eAuthHeaderType == RVSIP_AUTH_WWW_AUTHENTICATION_HEADER)
	{
		eParseType = SIP_PARSETYPE_WWW_AUTHENTICATE;
	}
	else
	{
		eParseType = SIP_PARSETYPE_PROXY_AUTHENTICATE;
	}

    AuthenticationHeaderClean(pHeader, RV_FALSE);
	
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
 * RvSipAuthenticationHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Authentication header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderFix(
                                     IN RvSipAuthenticationHeaderHandle hHeader,
                                     IN RvChar*                     pFixedBuffer)
{
    MsgAuthenticationHeader* pHeader = (MsgAuthenticationHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipAuthenticationHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipAuthenticationHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipAuthenticationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Authentication object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthenticationHeaderGetPool(RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipAuthenticationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Authentication object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthenticationHeaderGetPage(RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Authentication header fields are kept in a string format-for
 *          example, the nonce parameter. In order to get such a field from the
 *          Authentication header object, your application should supply an adequate buffer
 *          to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Authentication header object.
 *  eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAuthenticationHeaderGetStringLength(
                                      IN  RvSipAuthenticationHeaderHandle     hHeader,
                                      IN  RvSipAuthenticationHeaderStringName eStringName)
{
    RvInt32                stringOffset;
    MsgAuthenticationHeader* pHeader = (MsgAuthenticationHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (eStringName)
    {
         case RVSIP_AUTHENTICATION_AUTHSCHEME:
        {
            stringOffset = SipAuthenticationHeaderGetStrAuthScheme(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_REALM:
        {
            stringOffset = SipAuthenticationHeaderGetRealm(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_DOMAIN:
        {
            stringOffset = SipAuthenticationHeaderGetDomain(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_NONCE:
        {
            stringOffset = SipAuthenticationHeaderGetNonce(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_OPAQUE:
        {
            stringOffset = SipAuthenticationHeaderGetOpaque(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_ALGORITHM:
        {
            stringOffset = SipAuthenticationHeaderGetStrAuthAlgorithm(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_QOP:
        {
            stringOffset = SipAuthenticationHeaderGetStrQop(hHeader);
            break;
        }
		case RVSIP_AUTHENTICATION_INTEGRITY_KEY:
        {
            stringOffset = SipAuthenticationHeaderGetStrIntegrityKey(hHeader);
            break;
        }
		case RVSIP_AUTHENTICATION_CIPHER_KEY:
        {
            stringOffset = SipAuthenticationHeaderGetStrCipherKey(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_OTHER_PARAMS:
        {
            stringOffset = SipAuthenticationHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_AUTHENTICATION_BAD_SYNTAX:
        {
            stringOffset = SipAuthenticationHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderGetStringLength - Unknown stringName %d", eStringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pHeader->hPool,
			pHeader->hPage,
			stringOffset)+1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetAuthScheme
 * ------------------------------------------------------------------------
 * General: Gets the Authentication scheme enumeration from the Authentication header
 *          object.
 *          If this function returns RVSIP_AUTH_SCHEME_OTHER, call
 *          RvSipAuthenticationHeaderGetStrAuthScheme() to get the actual
 *          Authentication scheme in a string format.
 * Return Value: Returns the Authentication scheme enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvSipAuthScheme RVCALLCONV RvSipAuthenticationHeaderGetAuthScheme(
                                      IN  RvSipAuthenticationHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_AUTH_SCHEME_UNDEFINED;
	}

    return ((MsgAuthenticationHeader*)hHeader)->eAuthScheme;
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: This method retrieves the authentication scheme string value from the
 *          Authentication object.
 * Return Value: text string giving the authentication scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrAuthScheme(
                                            IN  RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strAuthScheme;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: Copies the authentication scheme string value from the Authentication object
 *          into a given buffer.
 *          Use this function if RvSipAuthenticationHeaderGetAuthScheme() returns
 *          RVSIP_AUTH_SCHEME_OTHER.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrAuthScheme(
                                       IN  RvSipAuthenticationHeaderHandle hHeader,
                                       IN  RvChar*                        strBuffer,
                                       IN  RvUint                         bufferLen,
                                       OUT RvUint*                        actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrAuthScheme(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthScheme in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader       - Handle of the Authentication header object.
 *        eAuthScheme   - The authentication scheme to be set in the object.
 *          strAuthScheme - text string giving the method type to be set in the object.
 *        strOffset - Offset of a string
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetAuthScheme(
                                  IN    RvSipAuthenticationHeaderHandle hHeader,
                                  IN    RvSipAuthScheme                 eAuthScheme,
                                  IN    RvChar*                        strAuthScheme,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset)
{
    MsgAuthenticationHeader*   header;
    RvInt32                   newStrOffset;
    RvStatus                  retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgAuthenticationHeader*)hHeader;

    header->eAuthScheme = eAuthScheme;

    if(eAuthScheme == RVSIP_AUTH_SCHEME_OTHER)
    {
        retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strOffset, strAuthScheme,
                                      header->hPool, header->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        header->strAuthScheme = newStrOffset;
#ifdef SIP_DEBUG
        header->pAuthScheme = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                           header->strAuthScheme);
#endif
    }
    else
    {
        header->strAuthScheme = UNDEFINED;
#ifdef SIP_DEBUG
        header->pAuthScheme = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: Sets the authentication scheme in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Authentication header object.
 *  eAuthScheme   - The authentication scheme to be set in the object.
 *    strAuthScheme - Text string giving the authentication scheme to be set in the object. This
 *                  parameter is required when eAuthScheme is
 *                  RVSIP_AUTH_SCHEME_OTHER. Otherwise, the parameter is set to NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetAuthScheme(
                                      IN    RvSipAuthenticationHeaderHandle hHeader,
                                      IN    RvSipAuthScheme                 eAuthScheme,
                                      IN    RvChar*                        strAuthScheme)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetAuthScheme(hHeader,
                                                eAuthScheme,
                                                strAuthScheme,
                                                NULL,
                                                NULL_PAGE,
                                                UNDEFINED);

}

/***************************************************************************
 * SipAuthenticationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General:This method gets the realm string from the authentication header.
 * Return Value: realm offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetRealm(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strRealm;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General: Copies the realm value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetRealm(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetRealm(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the realm string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pRealm - The realm to be set in the Authentication header - If
 *                      NULL, the exist realm string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetRealm(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pRealm,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pRealm,
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
 * RvSipAuthenticationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General:Sets the realm string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication header object.
 *    pRealm   - The realm to be set in the Authentication header. If a NULL value is supplied,
 *             the existing realm string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetRealm(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pRealm)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetRealm(hHeader, pRealm, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthenticationHeaderGetDomain
 * ------------------------------------------------------------------------
 * General:This method gets the domain string from the authentication header.
 * Return Value: domain offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetDomain(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strDomain;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetDomain
 * ------------------------------------------------------------------------
 * General: Copies the domain value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetDomain(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetDomain(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetDomain
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Domain string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pDomain - The domain to be set in the Authentication header - If
 *                      NULL, the exist Domain string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetDomain(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pDomain,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pDomain,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strDomain = newStr;
#ifdef SIP_DEBUG
    pHeader->pDomain = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                    pHeader->strDomain);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetDomain
 * ------------------------------------------------------------------------
 * General: Sets the domain string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication header object.
 *    pDomain  - The domain to be set in the Authentication header. If a NULL value is supplied,
 *             the existing domain string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetDomain(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pDomain)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetDomain(hHeader, pDomain, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthenticationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the authentication header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetNonce(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strNonce;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General: Copies the nonce value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetNonce(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetNonce(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pNonce - The nonce to be set in the Authentication header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetNonce(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pNonce,
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
 * RvSipAuthenticationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General: Sets the nonce string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication header object.
 *    pNonce   - The nonce to be set in the Authentication header. If a NULL value is supplied,
 *             the existing nonce string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetNonce(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pNonce)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetNonce(hHeader, pNonce, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthenticationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General:This method gets the opaque string from the authentication header.
 * Return Value: opaque offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetOpaque(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strOpaque;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General: Copies the opaque value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetOpaque(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetOpaque(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the opaque string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pOpaque - The opaque to be set in the Authentication header - If
 *                      NULL, the exist opaque string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetOpaque(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pOpaque,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pOpaque,
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
 * RvSipAuthenticationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General:Sets the opaque string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Authentication header object.
 *    pOpaque   - The opaque string to be set in the Authentication header. If a NULL value is
 *              supplied, the existing opaque string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetOpaque(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pOpaque)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetOpaque(hHeader, pOpaque, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General:This method gets the qop string from the authentication header.
 * Return Value: qop or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrQop(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strQop;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General: Copies the Qop string value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrQop(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrQop(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetStrQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the qop string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pQop         - The qop to be set in the Authentication header - If
 *                      NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrQop(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pQop,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pQop, pHeader->hPool,
                                  pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strQop = newStr;
#ifdef SIP_DEBUG
    pHeader->pQop = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                   pHeader->strQop);
#endif

    pHeader->eAuthQop = RVSIP_AUTH_QOP_OTHER;

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetStrQop
 * ------------------------------------------------------------------------
 * General:Sets the Qop string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication header object.
 *    pQop     - The Qop string to be set in the Authentication header. If a NULL value is supplied, the
 *             existing Qop string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetStrQop(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pQop)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetStrQop(hHeader, pQop, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipAuthenticationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the AuthParam string from the authentication header.
 * Return Value: AuthParam offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetOtherParams(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strAuthParam;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Authentication header other params field of the Authentication
 *          header object into a given buffer.
 *          Not all the Authentication header parameters have separated fields in the
 *          Authentication header object. Parameters with no specific fields are refered to as
 *          other params. They are kept in the object in one concatenated string in the
 *          form-"name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetOtherParams(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the AuthParam string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pAuthParam - The AuthParam to be set in the Authentication header - If
 *                      NULL, the exist AuthParam string in the header will be removed.
 *       offset       - Offset of a string (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetOtherParams(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pAuthParam,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pAuthParam,
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
 * RvSipAuthenticationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the Authentication header object.
 *          Not all the Party header parameters have separated fields in the Party header
 *          object. Parameters with no specific fields are refered to as other params. They
 *          are kept in the object in one concatenated string in the form-
 *          "name=value;name=value...".
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the Authentication header object.
 *    pOtherParams - The other params field to be set in the Authentication header. If NULL is
 *                 supplied, the existing other params field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetOtherParams(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipAuthenticationHeaderGetStale
 * ------------------------------------------------------------------------
 * General: Gets the stale enumeration from the Authentication Header object.
 * Return Value: Returns the stale enumeration from the header object
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvSipAuthStale RVCALLCONV RvSipAuthenticationHeaderGetStale(
                                    IN RvSipAuthenticationHeaderHandle hSipAuthHeader)
{
    if(hSipAuthHeader == NULL)
	{
        return RVSIP_AUTH_STALE_UNDEFINED;
	}

    return ((MsgAuthenticationHeader*)hSipAuthHeader)->eStale;
}


/***************************************************************************
 * RvSipAuthenticationHeaderSetStale
 * ------------------------------------------------------------------------
 * General: Sets the stale field in the Authentication Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authentication header object.
 *    eStale         - The stale value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetStale(
                                 IN RvSipAuthenticationHeaderHandle hSipAuthHeader,
                                 IN RvSipAuthStale                  eStale)
{
    if(hSipAuthHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgAuthenticationHeader*)hSipAuthHeader)->eStale = eStale;
    return RV_OK;
}


/***************************************************************************
 * RvSipAuthenticationHeaderGetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Gets the algorithm type enumeration from the Authentication header object.
 *          If this function returns RVSIP_AUTH_ALGORITHM_OTHER, call
 *          RvSipAuthenticationHeaderGetStrAuthAlgorithm(), to get the actual algorithm
 *          type in a string format.
 * Return Value: Returns the algorithm type enumeration of the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvSipAuthAlgorithm RVCALLCONV RvSipAuthenticationHeaderGetAuthAlgorithm(
                                      IN  RvSipAuthenticationHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_AUTH_ALGORITHM_UNDEFINED;
	}

    return ((MsgAuthenticationHeader*)hHeader)->eAlgorithm;
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This method retrieves the authentication algorithm string value from the
 *          Authentication object.
 * Return Value: text string giving the authentication scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrAuthAlgorithm(
                                            IN  RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strAlgorithm;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Copies the authentication algorithm string from the Authentication header
 *          object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *        the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrAuthAlgorithm(
                                           IN  RvSipAuthenticationHeaderHandle hHeader,
                                           IN  RvChar*                        strBuffer,
                                           IN  RvUint                         bufferLen,
                                           OUT RvUint*                        actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrAuthAlgorithm(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthAlgorithm in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Authentication header object.
 *        eAuthAlgorithm   - The authentication algorithm to be set in the object.
 *          strAuthAlgorithm - text string giving the algorithm type to be set in the object.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetAuthAlgorithm(
                                  IN    RvSipAuthenticationHeaderHandle hHeader,
                                  IN    RvSipAuthAlgorithm              eAuthAlgorithm,
                                  IN    RvChar*                        strAuthAlgorithm,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        offset)
{
    MsgAuthenticationHeader    *header;
    RvInt32                    newStr;
    RvStatus                   retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgAuthenticationHeader*)hHeader;

    header->eAlgorithm = eAuthAlgorithm;

    if(eAuthAlgorithm == RVSIP_AUTH_ALGORITHM_OTHER)
    {
        retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, offset, strAuthAlgorithm,
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
 * RvSipAuthenticationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: Sets the authentication algorithm in Authentication object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Authentication header object.
 *  eAuthAlgorithm    - The authentication scheme to be set in the object.
 *    strAuthAlgorithm  - Text string giving the authentication algorithm to be set in the object. Use this
 *                      parameter only if eAuthAlgorithm is set to
 *                      RVSIP_AUTH_ALGORITHM_OTHER. Otherwise, the parameter is NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetAuthAlgorithm(
                                      IN    RvSipAuthenticationHeaderHandle    hHeader,
                                      IN    RvSipAuthAlgorithm                 eAuthAlgorithm,
                                      IN    RvChar                            *strAuthAlgorithm)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetAuthAlgorithm(hHeader,
                                                   eAuthAlgorithm,
                                                   strAuthAlgorithm,
                                                   NULL,
                                                   NULL_PAGE,
                                                   UNDEFINED);

}

/***************************************************************************
 * RvSipAuthenticationHeaderGetQopOption
 * ------------------------------------------------------------------------
 * General: Gets the Qop option from the Authentication header object.
 * Return Value: The Qop option from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipAuthenticationHeaderGetQopOption(
                                      IN  RvSipAuthenticationHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_AUTH_QOP_UNDEFINED;
	}

    return ((MsgAuthenticationHeader*)hHeader)->eAuthQop;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetQopOption
 * ------------------------------------------------------------------------
 * General: Sets the Qop option enumeration in the Authentication Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader           - Handle to the Authentication header object.
 *    eQop                     - The Qop option enumeration to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetQopOption(
                                 IN    RvSipAuthenticationHeaderHandle hSipAuthHeader,
                                 IN    RvSipAuthQopOption              eQop)
{
    if(hSipAuthHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgAuthenticationHeader*)hSipAuthHeader)->eAuthQop = eQop;
    return RV_OK;
}


/***************************************************************************
 * RvSipAuthenticationHeaderGetHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Authentication Header object.
 * Return Value: Returns the Authentication header type enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvSipAuthenticationHeaderType  RVCALLCONV RvSipAuthenticationHeaderGetHeaderType(
                                                    IN RvSipAuthenticationHeaderHandle hSipAuthHeader)
{
    if(hSipAuthHeader == NULL)
	{
        return RVSIP_AUTH_UNDEFINED_AUTHENTICATION_HEADER;
	}

    return ((MsgAuthenticationHeader*)hSipAuthHeader)->eAuthHeaderType;
}


/***************************************************************************
 * RvSipAuthenticationHeaderSetHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Authentication Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthnHeader - Handle to the Authentication header object.
 *    eHeaderType     - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetHeaderType(
                                 IN    RvSipAuthenticationHeaderHandle hSipAuthHeader,
                                 IN    RvSipAuthenticationHeaderType   eHeaderType)
{
    if(hSipAuthHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgAuthenticationHeader*)hSipAuthHeader)->eAuthHeaderType = eHeaderType;
    return RV_OK;
}



/***************************************************************************
 * SipAuthenticationHeaderGetUserName
 * ------------------------------------------------------------------------
 * General:This method gets the username offset from the authentication header.
 * Return Value: username offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 RVCALLCONV SipAuthenticationHeaderGetUserName(
                                           IN RvSipAuthenticationHeaderHandle hHeader)

{
    return ((MsgAuthenticationHeader*)hHeader)->strUserName;
}

/***************************************************************************
 * SipAuthenticationHeaderSetUserName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the UserName string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pUserName    - The UserName to be set in the Authentication header - If
 *                      NULL, the exist UserName string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetUserName(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pUserName,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pUserName,
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
 * SipAuthenticationHeaderGetPassword
 * ------------------------------------------------------------------------
 * General:This method gets the password offset from the authentication header.
 * Return Value: password offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 RVCALLCONV SipAuthenticationHeaderGetPassword(
                                           IN RvSipAuthenticationHeaderHandle hHeader)

{
    return ((MsgAuthenticationHeader*)hHeader)->strPassword;
}

/***************************************************************************
 * SipAuthenticationHeaderSetPassword
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Password string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pPassword - The Password to be set in the Authentication header - If
 *                      NULL, the exist Password string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetPassword(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pPassword,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pPassword,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strPassword = newStr;
#ifdef SIP_DEBUG
    pHeader->pPassword = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                      pHeader->strPassword);
#endif

    return RV_OK;
}

/***************************************************************************
* SipAuthenticationHeaderClearPassword
* ------------------------------------------------------------------------
* General: This is an internal function that overrides the Password string
*          with zeroes in order to ensure hijacking by memory dump.
* ------------------------------------------------------------------------
* Arguments:
* Input:hHeader - Handle of the Authentication header object.
***************************************************************************/
void SipAuthenticationHeaderClearPassword(
                                 IN    RvSipAuthenticationHeaderHandle hHeader)
{
    MsgAuthenticationHeader* pHeader;
    RvInt32                  passwordSize;

    if(hHeader == NULL)
        return;

    pHeader = (MsgAuthenticationHeader*)hHeader;

    if (0 == pHeader->strPassword)
    {
        return;
    }

    passwordSize = RPOOL_Strlen(pHeader->hPool, pHeader->hPage,
        pHeader->strPassword);
    if (0 >= passwordSize)
    {
        return;
    }

    RPOOL_ResetPage(pHeader->hPool, pHeader->hPage, pHeader->strPassword,
        passwordSize);
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetAKAVersion
 * ------------------------------------------------------------------------
 * General: Gets the AKAVersion from the Authentication header object.
 * Return Value: Returns the AKAVersion, or UNDEFINED if the AKAVersion
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAuthenticationHeader - Handle to the Authentication header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAuthenticationHeaderGetAKAVersion(
                                         IN RvSipAuthenticationHeaderHandle hSipAuthenticationHeader)
{
    if(hSipAuthenticationHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgAuthenticationHeader*)hSipAuthenticationHeader)->nAKAVersion;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetAKAVersion
 * ------------------------------------------------------------------------
 * General:  Sets the AKAVersion number of the Authentication header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAuthenticationHeader - Handle to the Authentication header object.
 *    AKAVersion	        - The AKAVersion number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetAKAVersion(
                                         IN    RvSipAuthenticationHeaderHandle hSipAuthenticationHeader,
                                         IN    RvInt32							AKAVersion)
{
    if(hSipAuthenticationHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgAuthenticationHeader*)hSipAuthenticationHeader)->nAKAVersion = AKAVersion;
    return RV_OK;
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General:This method gets the IntegrityKey string from the authentication header.
 * Return Value: IntegrityKey or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrIntegrityKey(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strIntegrityKey;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General: Copies the IntegrityKey string value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrIntegrityKey(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrIntegrityKey(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IntegrityKey string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader		- Handle of the Authentication header object.
 *        pIntegrityKey - The qop to be set in the Authentication header - If
 *						  NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrIntegrityKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pIntegrityKey,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pIntegrityKey, pHeader->hPool,
                                  pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strIntegrityKey = newStr;
#ifdef SIP_DEBUG
    pHeader->pIntegrityKey = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                   pHeader->strIntegrityKey);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General:Sets the IntegrityKey string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- Handle to the Authentication header object.
 *    pIntegrityKey	- The IntegrityKey string to be set in the Authentication header. If a NULL 
 *					  value is supplied, the existing IntegrityKey string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetStrIntegrityKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pIntegrityKey)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetStrIntegrityKey(hHeader, pIntegrityKey, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrCipherKey
 * ------------------------------------------------------------------------
 * General:This method gets the CipherKey string from the authentication header.
 * Return Value: CipherKey or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrCipherKey(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strCipherKey;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrCipherKey
 * ------------------------------------------------------------------------
 * General: Copies the CipherKey string value of the Authentication object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrCipherKey(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrCipherKey(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetStrCipherKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the CipherKey string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader	 - Handle of the Authentication header object.
 *        pCipherKey - The qop to be set in the Authentication header - If
 *						NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrCipherKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pCipherKey,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset)
{
    RvInt32                 newStr;
    MsgAuthenticationHeader *pHeader;
    RvStatus                retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgAuthenticationHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, offset, pCipherKey, pHeader->hPool,
                                  pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCipherKey = newStr;
#ifdef SIP_DEBUG
    pHeader->pCipherKey = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                   pHeader->strCipherKey);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipAuthenticationHeaderSetStrCipherKey
 * ------------------------------------------------------------------------
 * General:Sets the CipherKey string in the Authentication header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- Handle to the Authentication header object.
 *    pCipherKey	- The CipherKey string to be set in the Authentication header. If a NULL 
 *					  value is supplied, the existing CipherKey string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetStrCipherKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar *                       pCipherKey)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetStrCipherKey(hHeader, pCipherKey, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipAuthenticationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrBadSyntax(
                                    IN RvSipAuthenticationHeaderHandle hHeader)
{
    return ((MsgAuthenticationHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipAuthenticationHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Authentication header,
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetStrBadSyntax(
                                           IN RvSipAuthenticationHeaderHandle hHeader,
                                           IN RvChar*                        strBuffer,
                                           IN RvUint                         bufferLen,
                                           OUT RvUint*                       actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgAuthenticationHeader*)hHeader)->hPool,
                                  ((MsgAuthenticationHeader*)hHeader)->hPage,
                                  SipAuthenticationHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAuthenticationHeaderSetStrBadSyntax
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
RvStatus SipAuthenticationHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgAuthenticationHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgAuthenticationHeader*)hHeader;

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
 * RvSipAuthenticationHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Authentication header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipAuthenticationHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipAuthenticationHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Authentication header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthHeader - Handle to the Authentication header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Authentication header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderGetRpoolString(
                             IN    RvSipAuthenticationHeaderHandle      hSipAuthHeader,
                             IN    RvSipAuthenticationHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgAuthenticationHeader* pHeader;
    RvInt32      requestedParamOffset;

    pHeader = (MsgAuthenticationHeader*)hSipAuthHeader;

    if(hSipAuthHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthenticationHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_AUTHENTICATION_AUTHSCHEME:
        requestedParamOffset = pHeader->strAuthScheme;
        break;
    case RVSIP_AUTHENTICATION_REALM:
        requestedParamOffset = pHeader->strRealm;
        break;
    case RVSIP_AUTHENTICATION_NONCE:
        requestedParamOffset = pHeader->strNonce;
        break;
    case RVSIP_AUTHENTICATION_DOMAIN:
        requestedParamOffset = pHeader->strDomain;
        break;
	case RVSIP_AUTHENTICATION_OPAQUE:
        requestedParamOffset = pHeader->strOpaque;
        break;
    case RVSIP_AUTHENTICATION_QOP:
        requestedParamOffset = pHeader->strQop;
        break;
    case RVSIP_AUTHENTICATION_ALGORITHM:
        requestedParamOffset = pHeader->strAlgorithm;
        break;
    case RVSIP_AUTHENTICATION_OTHER_PARAMS:
        requestedParamOffset = pHeader->strAuthParam;
        break;
    case RVSIP_AUTHENTICATION_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAuthenticationHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipAuthenticationHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipAuthenticationHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Authentication header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthHeader - Handle to the Authentication header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationHeaderSetRpoolString(
                             IN    RvSipAuthenticationHeaderHandle      hSipAuthHeader,
                             IN    RvSipAuthenticationHeaderStringName  eStringName,
                             IN    RPOOL_Ptr							*pRpoolPtr)
{
    MsgAuthenticationHeader* pHeader;

    pHeader = (MsgAuthenticationHeader*)hSipAuthHeader;
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
                 "RvSipAuthenticationHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_AUTHENTICATION_AUTHSCHEME:
        return SipAuthenticationHeaderSetAuthScheme(hSipAuthHeader,RVSIP_AUTH_SCHEME_OTHER,
                                               NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_REALM:
        return SipAuthenticationHeaderSetRealm(hSipAuthHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_NONCE:
        return SipAuthenticationHeaderSetNonce(hSipAuthHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_DOMAIN:
        return SipAuthenticationHeaderSetDomain(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_OPAQUE:
        return SipAuthenticationHeaderSetOpaque(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_QOP:
        return SipAuthenticationHeaderSetStrQop(hSipAuthHeader,NULL,
                                                 pRpoolPtr->hPool,
                                                 pRpoolPtr->hPage,
                                                 pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_OTHER_PARAMS:
        return SipAuthenticationHeaderSetOtherParams(hSipAuthHeader,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    case RVSIP_AUTHENTICATION_ALGORITHM:
        return SipAuthenticationHeaderSetAuthAlgorithm(hSipAuthHeader,RVSIP_AUTH_ALGORITHM_OTHER,NULL,
                                               pRpoolPtr->hPool,
                                               pRpoolPtr->hPage,
                                               pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipAuthenticationHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AuthenticationHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void AuthenticationHeaderClean( IN MsgAuthenticationHeader* pHeader,
                                       IN RvBool                  bCleanBS)
{
    pHeader->strDomain       = UNDEFINED;
    pHeader->eAuthScheme     = RVSIP_AUTH_SCHEME_UNDEFINED;
    pHeader->strAuthScheme   = UNDEFINED;
    pHeader->strRealm        = UNDEFINED;
    pHeader->strNonce        = UNDEFINED;
    pHeader->strOpaque       = UNDEFINED;
    pHeader->eStale          = RVSIP_AUTH_STALE_UNDEFINED;
    pHeader->eAlgorithm      = RVSIP_AUTH_ALGORITHM_UNDEFINED;
    pHeader->strAlgorithm    = UNDEFINED;
    pHeader->eAuthQop        = RVSIP_AUTH_QOP_UNDEFINED;
    pHeader->strQop          = UNDEFINED;
    pHeader->strAuthParam    = UNDEFINED;
    pHeader->strUserName     = UNDEFINED;
    pHeader->strPassword     = UNDEFINED;
	pHeader->nAKAVersion	 = UNDEFINED;
    pHeader->eHeaderType     = RVSIP_HEADERTYPE_AUTHENTICATION;
	pHeader->strIntegrityKey = UNDEFINED;
	pHeader->strCipherKey	 = UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pDomain		= NULL;
    pHeader->pAuthScheme	= NULL;
    pHeader->pRealm			= NULL;
    pHeader->pNonce			= NULL;
    pHeader->pOpaque		= NULL;
    pHeader->pAlgorithm		= NULL;
    pHeader->pQop			= NULL;
    pHeader->pAuthParam		= NULL;
    pHeader->pUserName		= NULL;
    pHeader->pPassword		= NULL;
    pHeader->pIntegrityKey	= NULL;
    pHeader->pCipherKey		= NULL;
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
