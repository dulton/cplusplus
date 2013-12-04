/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipSessionExpiresHeader.c                    *
 *                                                                            *
 * The file defines the methods of the Session Expires header object.         *
 * The Session Expires header functions enable you to construct, copy, encode *
 * parse, access and change Session Expires Header parameters.                *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Michal Mashiach    June 2001                                           *
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipSessionExpiresHeader.h"
#include "_SipSessionExpiresHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipMsgTypes.h"
#include <string.h>
#include <stdlib.h>


/*-----------------------------------------------------------------------*/
/*                        MODULE FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void SessionExpiresHeaderClean(IN MsgSessionExpiresHeader* pHeader,
                                      IN RvBool                   bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipSessionExpiresHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Session Expires header object inside a given message object.
 *          The header is kept in the header list of the message. You can
 *          choose to insert the header either at the head or tail of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: phHeader - Handle to the newly constructed Session Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderConstructInMsg(
                        IN  RvSipMsgHandle                   hSipMsg,
                        IN  RvBool                          pushHeaderAtHead,
                        OUT RvSipSessionExpiresHeaderHandle* phHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem= NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipSessionExpiresHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                               msg->hPool, msg->hPage, phHeader);
    if(stat != RV_OK)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*phHeader,
                              RVSIP_HEADERTYPE_SESSION_EXPIRES,
                              &hListElem,
                              (void**)phHeader);
}



/***************************************************************************
 * RvSipSessionExpiresHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Session Expires Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: phHeader - Handle to the newly constructed Session Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderConstruct(
                               IN  RvSipMsgMgrHandle                hMsgMgr,
                               IN  HRPOOL                           hPool,
                               IN  HPAGE                            hPage,
                               OUT RvSipSessionExpiresHeaderHandle* phHeader)
{
    MsgSessionExpiresHeader*   pHeader;
    MsgMgr*                    pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pHeader = (MsgSessionExpiresHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgSessionExpiresHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipSessionExpiresHeaderConstruct - Failed to construct session expire header. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    SessionExpiresHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipSessionExpiresHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipSessionExpiresHeaderConstruct - Session Expires header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *phHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Session Expires header object to a
 *         destination Session Expires header object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Session Expires header object.
 *    pSource      - Handle to the source Session Expires header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderCopy(
                                     INOUT RvSipSessionExpiresHeaderHandle hDestination,
                                     IN    RvSipSessionExpiresHeaderHandle hSource)
{
    MsgSessionExpiresHeader*   source;
    MsgSessionExpiresHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgSessionExpiresHeader*)hSource;
    dest   = (MsgSessionExpiresHeader*)hDestination;

    dest->eHeaderType  = source->eHeaderType;
    dest->deltaSeconds = source->deltaSeconds;
    dest->eRefresherType  = source->eRefresherType;
    dest->bIsCompact    = source->bIsCompact;
    if(source->strParams > UNDEFINED)
    {
        dest->strParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                        dest->hPool,
                                                        dest->hPage,
                                                        source->hPool,
                                                        source->hPage,
                                                        source->strParams);
        if(dest->strParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSessionExpiresHeaderCopy - didn't manage to copy SessionExpiresParams"));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        dest->strParams = UNDEFINED;
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
                "RvSipSessionExpiresHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipSessionExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Session Expires header object to a textual Session Expires header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage().
 *          The allocated page must be freed only if this function returns
 *          RV_OK.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength or phPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Session Expires header object.
 *        hPool    - Handle to the specified memory pool.
 * output: phPage   - The memory page allocated to contain the encoded header.
 *         pLength  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderEncode(
                              IN    RvSipSessionExpiresHeaderHandle hHeader,
                              IN    HRPOOL                          hPool,
                              OUT   HPAGE*                          phPage,
                              OUT   RvUint32*                      pLength)
{
    RvStatus stat;
    MsgSessionExpiresHeader* pHeader = (MsgSessionExpiresHeader*)hHeader;

    if(phPage == NULL || pLength == NULL)
        return RV_ERROR_NULLPTR;

    *phPage = NULL_PAGE;
       *pLength = 0;

    if((pHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, phPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipSessionExpiresHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipSessionExpiresHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *phPage, hPool, hHeader));
    }

    *pLength = 0; /* This way length will give the real length, and will not
                     just add the new length to the old one */
    stat = SessionExpiresHeaderEncode(hHeader, hPool, *phPage, RV_FALSE, RV_FALSE, pLength);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *phPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipSessionExpiresHeaderEncode - Failed. Free page 0x%p on pool 0x%p. SessionExpiresHeaderEncode fail",
                  *phPage, hPool));
    }
    return stat;
}


/***************************************************************************
 * SessionExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Session Expires header (as string) on it.
 *          format: "Session Expires: "
 *                  delta-seconds [refresher]
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Session Expires header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV SessionExpiresHeaderEncode(
                        IN    RvSipSessionExpiresHeaderHandle hHeader,
                        IN    HRPOOL                          hPool,
                        IN    HPAGE                           hPage,
                        IN    RvBool                          bInUrlHeaders,
                        IN    RvBool                          bForceCompactForm,
                        INOUT RvUint32*                       pLength)
{
    MsgSessionExpiresHeader*  pHeader;
    RvStatus          stat = RV_OK;
    RvChar            strHelper[16];

   if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    pHeader = (MsgSessionExpiresHeader*) hHeader;

    if(pHeader->deltaSeconds == UNDEFINED &&
       pHeader->strBadSyntax == UNDEFINED)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SessionExpiresHeaderEncode - Session Expires header has no session expires value. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
            hHeader, hPool, hPage));
        return RV_ERROR_BADPARAM;
    }
     RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "SessionExpiresHeaderEncode - Encoding Session Expires header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
               hHeader, hPool, hPage));

    /* put "Session-Expires" */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "x", pLength);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Session-Expires", pLength);
    }
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "SessionExpiresHeaderEncode - Failed to encoding Session expires header. stat is %d",
                  stat));
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    pLength);


    /* bad - syntax */
    if(pHeader->strBadSyntax > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBadSyntax,
                                    pLength);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SessionExpiresHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    MsgUtils_itoa(pHeader->deltaSeconds , strHelper);


    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SessionExpiresHeaderEncode - Failed to encode Session Expires object. stat is %d",
            stat));
        return stat;
    }
    if (pHeader->eRefresherType != RVSIP_SESSION_EXPIRES_REFRESHER_NONE)
    {
        /* put "; refresher =" after delta second if there is refresher type */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "refresher", pLength);
        if (stat != RV_OK)
            return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;


        if (pHeader->eRefresherType ==RVSIP_SESSION_EXPIRES_REFRESHER_UAC)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "uac", pLength);
            if (RV_OK != stat)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "SessionExpiresHeaderEncode - Failed to encode Session Expires object. stat is %d",
                    stat));
                return stat;
            }

        }
        else if (pHeader ->eRefresherType == RVSIP_SESSION_EXPIRES_REFRESHER_UAS)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "uas", pLength);
            if (RV_OK != stat)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "SessionExpiresHeaderEncode - Failed to encode Session Expires object. stat is %d",
                    stat));
                return stat;
            }
        }

    }
    /* set the contact-params. (insert ";" in the begining) */
    if(pHeader->strParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), pLength);
        if (stat != RV_OK)
            return stat;

        /* set contactParmas */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strParams,
                                    pLength);
    }

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SessionExpiresHeaderEncode - Failed to encoding Session Expires header. stat is %d",
            stat));
    }

    return stat;

}

/***************************************************************************
 * RvSipSessionExpiresHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Session Expires header-for example,
 *         "Session Expires: 3600;refresher=uac" - into a Session Expires header
 *          object. All the textual fields are placed inside the object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_INVALID_HANDLE,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   -    A handle to the Session Expires header object.
 *    strBuffer    - Buffer containing a textual Session Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderParse(
                                     IN    RvSipSessionExpiresHeaderHandle hHeader,
                                     IN    RvChar*                        strBuffer)
{
    MsgSessionExpiresHeader* pHeader = (MsgSessionExpiresHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (strBuffer == NULL)
        return RV_ERROR_BADPARAM;

    SessionExpiresHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_SESSION_EXPIRES,
                                        strBuffer,
                                        RV_FALSE,
                                         (void*)hHeader);;
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
 * RvSipSessionExpiresHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual SessionExpires header value into an
 *          SessionExpires header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value
 *          part as a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipSessionExpiresHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the SessionExpires header object.
 *    buffer    - The buffer containing a textual SessionExpires header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderParseValue(
                                     IN    RvSipSessionExpiresHeaderHandle hHeader,
                                     IN    RvChar*                        strBuffer)
{
    MsgSessionExpiresHeader* pHeader = (MsgSessionExpiresHeader*)hHeader;
	RvBool                   bIsCompact;
    RvStatus                 rv;
	
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (strBuffer == NULL)
        return RV_ERROR_BADPARAM;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    SessionExpiresHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_SESSION_EXPIRES,
                                        strBuffer,
                                        RV_TRUE,
                                         (void*)hHeader);;
    
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
 * RvSipSessionExpiresHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an SessionExpires header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderFix(
                                     IN RvSipSessionExpiresHeaderHandle hHeader,
                                     IN RvChar*                        pFixedBuffer)
{
    MsgSessionExpiresHeader* pHeader = (MsgSessionExpiresHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipSessionExpiresHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSessionExpiresHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSessionExpiresHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipSessionExpiresHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Session Expires object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipSessionExpiresHeaderGetPool(RvSipSessionExpiresHeaderHandle hHeader)
{
    return ((MsgSessionExpiresHeader *)hHeader)->hPool;
}

/***************************************************************************
 * SipSessionExpiresHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Session Expires object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipSessionExpiresHeaderGetPage(RvSipSessionExpiresHeaderHandle hHeader)
{
    return ((MsgSessionExpiresHeader *)hHeader)->hPage;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderGetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Gets the delta-seconds integer of the Session Expires header.
 *          If the delta-seconds integer is not set, UNDEFINED is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - pDeltaSeconds is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Session Expires header object.
 *  Output: pDeltaSeconds - The delta-seconds integer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderGetDeltaSeconds(
                    IN  RvSipSessionExpiresHeaderHandle hHeader,
                    OUT RvInt32                       *pDeltaSeconds)
{
    MsgSessionExpiresHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDeltaSeconds)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgSessionExpiresHeader *)hHeader;
    *pDeltaSeconds = pHeader->deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderSetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Sets the delta seconds integer of the Session Expires header.
 *          If the given delta-seconds is UNDEFINED, the delta-seconds of
 *          the Session Expires header is removed
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Session Expires header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Session Expires header object.
 *         deltaSeconds - The delta-seconds to be set to the Session Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderSetDeltaSeconds(
                        IN  RvSipSessionExpiresHeaderHandle hHeader,
                        IN  RvInt32                       deltaSeconds)
{
    MsgSessionExpiresHeader *pSessionExpires;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pSessionExpires = (MsgSessionExpiresHeader *)hHeader;
    pSessionExpires->deltaSeconds = deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderGetRefresherType
 * ------------------------------------------------------------------------
 * General: Gets the refresher type of the Session Expires header.
 *          If the refresher type is not set,
 *          RVSIP_SESSION_EXPIRES_REFRESHER_NONE is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - peRefresherType is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Session Expires header object.
 *  Output: peRefresherType - The refresher type.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderGetRefresherType(
                    IN  RvSipSessionExpiresHeaderHandle  hHeader,
                    OUT RvSipSessionExpiresRefresherType *peRefresherType)
{
    MsgSessionExpiresHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == peRefresherType)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgSessionExpiresHeader *)hHeader;
    *peRefresherType = pHeader->eRefresherType;
    return RV_OK;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderSetRefresherType
 * ------------------------------------------------------------------------
 * General: Sets the refresher type of the Session Expires header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Session Expires header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Session Expires header object.
 *         deltaSeconds - The refreher type to be set to the Session Expires header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderSetRefresherType(
                        IN  RvSipSessionExpiresHeaderHandle  hHeader,
                        IN  RvSipSessionExpiresRefresherType eRefresherType)
{
    MsgSessionExpiresHeader *pSessionExpires;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pSessionExpires = (MsgSessionExpiresHeader *)hHeader;

    pSessionExpires->eRefresherType = eRefresherType;
    return RV_OK;
}

/***************************************************************************
 * SipSessionExpiresHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipSessionExpiresHeaderGetOtherParams(
                                     IN RvSipSessionExpiresHeaderHandle hHeader)
{
    return ((MsgSessionExpiresHeader*)hHeader)->strParams;
}

/***************************************************************************
 * RvSipSessionExpiresHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other parameters string from the Session-Expires header
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
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderGetOtherParams(
                                    IN RvSipSessionExpiresHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgSessionExpiresHeader*)hHeader)->hPool,
                                  ((MsgSessionExpiresHeader*)hHeader)->hPage,
                                  SipSessionExpiresHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSessionExpiresHeaderSetOtherParams
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
RvStatus SipSessionExpiresHeaderSetOtherParams(
                             IN    RvSipSessionExpiresHeaderHandle hHeader,
                             IN    RvChar *                    strOtherParams,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             IN    RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgSessionExpiresHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgSessionExpiresHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strOtherParams,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strParams = newStr;
    return RV_OK;
}

/***************************************************************************
 * RvSipSessionExpiresHeaderSetOtherParams
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
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderSetOtherParams(
                                 IN   RvSipSessionExpiresHeaderHandle  hHeader,
                                 IN   RvChar                      *otherParams)
{
    /* validity checking will be done in the internal function */
    return SipSessionExpiresHeaderSetOtherParams(hHeader, otherParams, NULL,
                                              NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipSessionExpiresHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSessionExpiresHeaderGetStrBadSyntax(
                                    IN RvSipSessionExpiresHeaderHandle hHeader)
{
    return ((MsgSessionExpiresHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipSessionExpiresHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad SessionExpires header,
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
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderGetStrBadSyntax(
                                               IN RvSipSessionExpiresHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgSessionExpiresHeader*)hHeader)->hPool,
                                  ((MsgSessionExpiresHeader*)hHeader)->hPage,
                                  SipSessionExpiresHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSessionExpiresHeaderSetStrBadSyntax
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
RvStatus SipSessionExpiresHeaderSetStrBadSyntax(
                                  IN RvSipSessionExpiresHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgSessionExpiresHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgSessionExpiresHeader*)hHeader;

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
 * RvSipSessionExpiresHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal SessionExpires header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderSetStrBadSyntax(
                                  IN RvSipSessionExpiresHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipSessionExpiresHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * RvSipSessionExpiresHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: The other params of Session-Expires header fields are kept in a string
 *          format.
 *          In order to get such a field from the Session-Expires header
 *          object, your application should supply an adequate buffer to where
 *          the string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Session-Expires header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipSessionExpiresHeaderGetStringLength(
                      IN  RvSipSessionExpiresHeaderHandle     hHeader,
                      IN  RvSipSessionExpiresHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgSessionExpiresHeader* pHeader = (MsgSessionExpiresHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_SESSION_EXPIRES_OTHER_PARAMS:
        {
            stringOffset = SipSessionExpiresHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_SESSION_EXPIRES_BAD_SYNTAX:
        {
            stringOffset = SipSessionExpiresHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RvSipSessionExpiresHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipSessionExpiresHeaderGetPool(hHeader),
                             SipSessionExpiresHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * RvSipSessionExpiresHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Session-Expires header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderSetCompactForm(
                                   IN RvSipSessionExpiresHeaderHandle hHeader,
                                   IN RvBool                      bIsCompact)
{
    MsgSessionExpiresHeader* pHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader = (MsgSessionExpiresHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipSessionExpiresHeaderSetCompactForm - Setting compact form of hHeader 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}


/***************************************************************************
 * RvSipSessionExpiresHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Session-Expires header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSessionExpiresHeaderGetCompactForm(
                                   IN    RvSipSessionExpiresHeaderHandle hHeader,
                                   IN    RvBool                      *pbIsCompact)
{
    MsgSessionExpiresHeader* pHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    pHeader = (MsgSessionExpiresHeader*)hHeader;
	

    *pbIsCompact = pHeader->bIsCompact;
    return RV_OK;

}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SessionExpiresHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Pointer to the header object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void SessionExpiresHeaderClean(IN MsgSessionExpiresHeader* pHeader,
                                      IN RvBool                   bCleanBS)
{
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_SESSION_EXPIRES;
    pHeader->deltaSeconds     = UNDEFINED;
    pHeader->eRefresherType   = RVSIP_SESSION_EXPIRES_REFRESHER_NONE;
    pHeader->strParams        = UNDEFINED;

	if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}
#endif /* RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif





