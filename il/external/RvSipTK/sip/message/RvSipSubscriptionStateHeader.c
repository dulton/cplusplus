
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipSubscriptionStateHeader.c                         *
 *                                                                            *
 * The file defines the methods of the Subscription-State header object:      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Ofra Wachsman    May 2002                                              *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES

#include "RvSipSubscriptionStateHeader.h"
#include "_SipSubscriptionStateHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void SubscriptionStateHeaderClean(IN MsgSubscriptionStateHeader* pHeader,
                                         IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipSubscriptionStateHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Subscription-State header object inside a given message
 *          object. The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE),
 *                            or to the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Subscription-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderConstructInMsg(
                               IN  RvSipMsgHandle                      hSipMsg,
                               IN  RvBool                              pushHeaderAtHead,
                               OUT RvSipSubscriptionStateHeaderHandle *hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem= NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipSubscriptionStateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                  msg->hPool, msg->hPage, hHeader);

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
                              RVSIP_HEADERTYPE_SUBSCRIPTION_STATE,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Subscription-State Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Subscription-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderConstruct(
                                IN  RvSipMsgMgrHandle                   hMsgMgr,
                                IN  HRPOOL                              hPool,
                                IN  HPAGE                               hPage,
                                OUT RvSipSubscriptionStateHeaderHandle *hHeader)
{
    MsgSubscriptionStateHeader*    pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgSubscriptionStateHeader*)SipMsgUtilsAllocAlign(
                                                        pMsgMgr,
                                                        hPool,
                                                        hPage,
                                                        sizeof(MsgSubscriptionStateHeader),
                                                        RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;
    SubscriptionStateHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipSubscriptionStateHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderConstruct - header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Subscription-State header object to a
 *          destination Subscription-State header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Subscription-State header object.
 *    hSource      - Handle to the source Subscription-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderCopy(
                                INOUT  RvSipSubscriptionStateHeaderHandle hDestination,
                                IN     RvSipSubscriptionStateHeaderHandle hSource)
{
    MsgSubscriptionStateHeader* source = (MsgSubscriptionStateHeader*)hSource;
    MsgSubscriptionStateHeader* dest = (MsgSubscriptionStateHeader*)hDestination;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest->eHeaderType = source->eHeaderType;

    dest->expiresVal = source->expiresVal;
    dest->retryAfterVal = source->retryAfterVal;

    /* eReason + strReason */
    dest->eReason = source->eReason;

    if(source->strReason > UNDEFINED)
    {
        dest->strReason = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strReason);
        if(dest->strReason == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipSubscriptionStateHeaderCopy - Failed to copy reason string"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pReason = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strReason);
#endif
    }
    else
    {
        dest->strReason = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pReason = NULL;
#endif
    }

    /* eSubstate + strSubstate */
    dest->eSubstate = source->eSubstate;

    if(source->strSubstate > UNDEFINED)
    {
        dest->strSubstate = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strSubstate);
        if(dest->strSubstate == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipSubscriptionStateHeaderCopy - Failed to copy substate string"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pSubstate = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strSubstate);
#endif
    }
    else
    {
        dest->strSubstate = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pSubstate = NULL;
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
                    "RvSipSubscriptionStateHeaderCopy - Failed to copy other params string"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOtherParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strOtherParams);
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
                "RvSipSubscriptionStateHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipSubscriptionStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Subscription-State header object to a textual Subscription-State
 *          header. The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Subscription-State header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderEncode(
                                IN    RvSipSubscriptionStateHeaderHandle hHeader,
                                IN    HRPOOL                             hPool,
                                OUT   HPAGE                             *hPage,
                                OUT   RvUint32                          *length)
{
    RvStatus stat;
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((pHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSubscriptionStateHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSubscriptionStateHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = SubscriptionStateHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSubscriptionStateHeaderEncode - Failed. Free page 0x%p on pool 0x%p. SubscriptionStateHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * SubscriptionStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Subscription-State header (as string) on it.
 *          format: "Subscription-State: "
 *                  substate-value *(";" subexp-params)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Subscription-State header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV SubscriptionStateHeaderEncode(
                      IN    RvSipSubscriptionStateHeaderHandle  hHeader,
                      IN    HRPOOL                              hPool,
                      IN    HPAGE                               hPage,
                      IN    RvBool                bInUrlHeaders,
                      INOUT RvUint32*                          length)
{
    MsgSubscriptionStateHeader*  pHeader;
    RvStatus          stat;
    RvChar            strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgSubscriptionStateHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "SubscriptionStateHeaderEncode - Encoding Subscription-state header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "Subscription-State" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Subscription-State", length);
    if (RV_OK != stat)
    {
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    length);
    if (stat != RV_OK)
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
                "SubscriptionStateHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* substate */
    if(pHeader->eSubstate < 0)
    {
        /*mandatoty parameter*/
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "SubscriptionStateHeaderEncode(hHeader=0x%p) - Failed. no substate was set, mandatory param!",
             hHeader));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    stat = MsgUtilsEncodeSubstateVal(pHeader->pMsgMgr, hPool, hPage,
                                     pHeader->eSubstate, pHeader->hPool, pHeader->hPage,
                                     pHeader->strSubstate, length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "SubscriptionStateHeaderEncode - failed to set substate. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));
        return stat;
    }

    /* ;reason= */
    if(pHeader->eReason != RVSIP_SUBSCRIPTION_REASON_UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "reason", length);
        if (RV_OK != stat)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        stat = MsgUtilsEncodeSubsReason(pHeader->pMsgMgr, hPool, hPage,
                                        pHeader->eReason, pHeader->hPool, pHeader->hPage,
                                        pHeader->strReason, length);
        if (RV_OK != stat)
        {
            return stat;
        }

    }

    /* ;expires= */
    if(pHeader->expiresVal > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "expires", length);
        if (RV_OK != stat)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        MsgUtils_itoa(pHeader->expiresVal, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
        if (RV_OK != stat)
        {
            return stat;
        }
    }

    /* ;retry-After= */
    if(pHeader->retryAfterVal > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "retry-after", length);
        if (RV_OK != stat)
            return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        MsgUtils_itoa(pHeader->retryAfterVal, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
        if (RV_OK != stat)
        {
            return stat;
        }
    }

    /* other params */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (stat != RV_OK)
            return stat;

        /* set other Parmas */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
    }

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SubscriptionStateHeaderEncode - Failed to encode subscription-State header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Subscription-State header-for example,
 *         "Subscription-State: "active";expires=100" -
 *          into a Subscription-State header object.
 *         All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Subscription-State header object.
 *    buffer    - Buffer containing a textual Subscription-State header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderParse(
                                 IN  RvSipSubscriptionStateHeaderHandle  hHeader,
                                 IN  RvChar                             *buffer)
{
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    SubscriptionStateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_SUBSCRIPTION_STATE,
                                        buffer,
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
 * RvSipSubscriptionStateHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual SubscriptionState header value into an
 *          SubscriptionState header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipSubscriptionStateHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the SubscriptionState header object.
 *    buffer    - The buffer containing a textual SubscriptionState header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderParseValue(
                                 IN  RvSipSubscriptionStateHeaderHandle  hHeader,
                                 IN  RvChar                             *buffer)
{
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    SubscriptionStateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_SUBSCRIPTION_STATE,
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
 * RvSipSubscriptionStateHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an SubscriptionState header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderFix(
                                     IN RvSipSubscriptionStateHeaderHandle hHeader,
                                     IN RvChar                            *pFixedBuffer)
{
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipSubscriptionStateHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * SipSubscriptionStateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Contact object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipSubscriptionStateHeaderGetPool(RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipSubscriptionStateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Contact object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipSubscriptionStateHeaderGetPage(RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Subscription-State header fields are kept in a string format-
 *          for example, the Subscription-State header other params. In order to get
 *          such a field from the Subscription-State header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to
 *          enable you to allocate an appropriate buffer size before calling
 *          the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Subscription-State header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipSubscriptionStateHeaderGetStringLength(
                                IN  RvSipSubscriptionStateHeaderHandle     hHeader,
                                IN  RvSipSubscriptionStateHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_SUBSCRIPTION_STATE_SUBSTATE_VAL:
        {
            stringOffset = SipSubscriptionStateHeaderGetStrSubstate(hHeader);
            break;
        }
        case RVSIP_SUBSCRIPTION_STATE_REASON:
        {
            stringOffset = SipSubscriptionStateHeaderGetStrReason(hHeader);
            break;
        }
        case RVSIP_SUBSCRIPTION_STATE_OTHER_PARAMS:
        {
            stringOffset = SipSubscriptionStateHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_SUBSCRIPTION_STATE_BAD_SYNTAX:
        {
            stringOffset = SipSubscriptionStateHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSubscriptionStateHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}


/***************************************************************************
 * RvSipSubscriptionStateHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Subscription-State header into
 *          a given page from a specified pool. The copied string is not
 *          consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipSubscriptionStateHeader - Handle to the Subscription-State
 *                                       header object.
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
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderGetRpoolString(
                             IN    RvSipSubscriptionStateHeaderHandle      hSipSubscriptionStateHeader,
                             IN    RvSipSubscriptionStateHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                               *pRpoolPtr)
{
    MsgSubscriptionStateHeader* pHeader;
    RvInt32        requestedParamOffset;

    if(hSipSubscriptionStateHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pHeader = (MsgSubscriptionStateHeader*)hSipSubscriptionStateHeader;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipSubscriptionStateHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_SUBSCRIPTION_STATE_SUBSTATE_VAL:
        requestedParamOffset = pHeader->strSubstate;
        break;
    case RVSIP_SUBSCRIPTION_STATE_REASON:
        requestedParamOffset = pHeader->strReason;
        break;
    case RVSIP_SUBSCRIPTION_STATE_OTHER_PARAMS:
        requestedParamOffset = pHeader->strOtherParams;
        break;
    case RVSIP_SUBSCRIPTION_STATE_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipSubscriptionStateHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipSubscriptionStateHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Subscription-State
 *          header object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipSubscriptionStateHeader - Handle to the Subscription-State header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetRpoolString(
                             IN    RvSipSubscriptionStateHeaderHandle      hSipSubscriptionStateHeader,
                             IN    RvSipSubscriptionStateHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                               *pRpoolPtr)
{
    MsgSubscriptionStateHeader* pHeader;

    pHeader = (MsgSubscriptionStateHeader*)hSipSubscriptionStateHeader;
    if(hSipSubscriptionStateHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipSubscriptionStateHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_SUBSCRIPTION_STATE_SUBSTATE_VAL:
        return SipSubscriptionStateHeaderSetSubstate(hSipSubscriptionStateHeader,
                                   RVSIP_SUBSCRIPTION_SUBSTATE_OTHER,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_SUBSCRIPTION_STATE_REASON:
        return SipSubscriptionStateHeaderSetReason(hSipSubscriptionStateHeader,
                                   RVSIP_SUBSCRIPTION_REASON_OTHER,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    case RVSIP_SUBSCRIPTION_STATE_OTHER_PARAMS:
        return SipSubscriptionStateHeaderSetOtherParams(hSipSubscriptionStateHeader,NULL,
                                   pRpoolPtr->hPool,
                                   pRpoolPtr->hPage,
                                   pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSubscriptionStateHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
}


/***************************************************************************
 * RvSipSubscriptionStateHeaderGetSubstate
 * ------------------------------------------------------------------------
 * General: This function gets the substate eunmuration value of the
 *          subscription-state object. If RVSIP_SUBSCRIPTION_SUBSTATE_OTHER
 *          is returned, you can receive the substate string using
 *          RvSipSubscriptionStateHeaderGetStrSubstate().
 * Return Value: Returns an enumuration value of substate.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Subscription-State header object.
 ***************************************************************************/
RVAPI RvSipSubscriptionSubstate RVCALLCONV RvSipSubscriptionStateHeaderGetSubstate(
                                            IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED;

    return ((MsgSubscriptionStateHeader*)hHeader)->eSubstate;
}

/***************************************************************************
 * SipSubscriptionStateHeaderGetSubstate
 * ------------------------------------------------------------------------
 * General:This method gets the substate string from Subscroption-State header.
 * Return Value: substate offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrSubstate(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->strSubstate;
}
/***************************************************************************
 * RvSipSubscriptionStateHeaderGetStrSubstate
 * ------------------------------------------------------------------------
 * General: Copies the substate string from the Subscription-State header into
 *          a given buffer. Use this function when the substate enumeration
 *          returned by RvSipSubscriptionStateHeaderGetSubstate() is
 *          RVSIP_SUBSCRIPTION_SUBSTATE_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderGetStrSubstate(
                            IN  RvSipSubscriptionStateHeaderHandle   hHeader,
                            IN  RvChar                              *strBuffer,
                            IN  RvUint                               bufferLen,
                            OUT RvUint                              *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgSubscriptionStateHeader*)hHeader)->hPool,
                                  ((MsgSubscriptionStateHeader*)hHeader)->hPage,
                                  SipSubscriptionStateHeaderGetStrSubstate(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSubscriptionStateHeaderSetSubstate
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the substate in the
 *           SubscriptionState header object.
 *           User can set a substate enum value (in eSubstate) and no string,
 *           or to set eSubstate to be RVSIP_SUBSCRIPTION_SUBSTATE_OTHER, and
 *           then put the substate value in strSubstate.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle of the Suibscription-State header object.
 *    eSubstate   - The enum substate value to be set in the object.
 *  strSubstate - String of substate, in case eSubstate is
 *                  RVSIP_SUBSCRIPTION_SUBSTATE_OTHER. may be NULL.
 *  hPool       - The pool on which the string lays (if relevant).
 *  hPage       - The page on which the string lays (if relevant).
 *  strOffset   - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetSubstate(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionSubstate          eSubstate,
                             IN  RvChar                            *strSubstate,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                            strOffset)
{
    RvInt32      newStr;
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader->eSubstate = eSubstate;

    if(eSubstate == RVSIP_SUBSCRIPTION_SUBSTATE_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strSubstate,
                                      pHeader->hPool, pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strSubstate = newStr;
#ifdef SIP_DEBUG
        pHeader->pSubstate = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strSubstate);
#endif
    }
    else
    {
        pHeader->strSubstate = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pSubstate = NULL;
#endif
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderSetSubstate
 * ------------------------------------------------------------------------
 * General: Sets the substate field in the Subscription-State header object.
 *          If the enumeration given by eSubstate is RVSIP_SUBSCRIPTION_SUBSTATE_OTHER,
 *          sets the substate string given by strSubstate in the
 *          Subscription-State header object.
 *          Use RVSIP_SUBSCRIPTION_SUBSTATE_OTHER when the substate you wish
 *          to set to the Subscription-State does not have a
 *          matching enumeration value in the RvSipSubscriptionSubstate enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Subscription-State header object.
 *    eSubstate  - The substate enumeration to be set in the Subscription-State
 *               header object. If RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED is supplied,
 *               the existing substate is removed from the header.
 *    strSubstate - The substate string to be set in the Subscription-State header.
 *                (relevant when eSubstate is RVSIP_SUBSCRIPTION_SUBSTATE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetSubstate(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionSubstate          eSubstate,
                             IN  RvChar                            *strSubstate)
{
    /* validity checking will be done in the internal function */
    return SipSubscriptionStateHeaderSetSubstate(hHeader,
                                                eSubstate,
                                                strSubstate,
                                                NULL,
                                                NULL_PAGE,
                                                UNDEFINED);
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetReason
 * ------------------------------------------------------------------------
 * General: This function gets the reason eunmuration value of the
 *          subscription-state object. If RVSIP_SUBSCRIPTION_REASON_OTHER
 *          is returned, you can receive the reason string using
 *          RvSipSubscriptionStateHeaderGetStrReason().
 * Return Value: Returns an enumuration value of substate.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Subscription-State header object.
 ***************************************************************************/
RVAPI RvSipSubscriptionReason RVCALLCONV RvSipSubscriptionStateHeaderGetReason(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    if(hHeader == NULL)
    {
        return RVSIP_SUBSCRIPTION_REASON_UNDEFINED;
    }

    return ((MsgSubscriptionStateHeader*)hHeader)->eReason;
}

/***************************************************************************
 * SipSubscriptionStateHeaderGetStrReason
 * ------------------------------------------------------------------------
 * General:This method gets the reason string from Subscroption-State header.
 * Return Value: reason offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrReason(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->strReason;
}


/***************************************************************************
 * RvSipSubscriptionStateHeaderGetStrReason
 * ------------------------------------------------------------------------
 * General: Copies the reason string from the Subscription-State header into
 *          a given buffer. Use this function when the reason enumeration
 *          returned by RvSipSubscriptionStateHeaderGetReason() is
 *          RVSIP_SUBSCRIPTION_REASON_OTHER.
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
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderGetStrReason(
                            IN RvSipSubscriptionStateHeaderHandle   hHeader,
                            IN RvChar                              *strBuffer,
                            IN RvUint                               bufferLen,
                            OUT RvUint                             *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgSubscriptionStateHeader*)hHeader)->hPool,
                                  ((MsgSubscriptionStateHeader*)hHeader)->hPage,
                                  SipSubscriptionStateHeaderGetStrReason(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSubscriptionStateHeaderSetReason
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the reaon in the
 *           SubscriptionState header object.
 *           User can set a reason enum value (in eReason) and no string,
 *           or to set eReason to be RVSIP_SUBSCRIPTION_REASON_OTHER, and
 *           then put the reason value in strReason.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle of the Suibscription-State header object.
 *    eReason     - The enum reason value to be set in the object.
 *  strReason   - String of reson, in case eSubstate is
 *                RVSIP_SUBSCRIPTION_REASON_OTHER. may be NULL.
 *  hPool       - The pool on which the string lays (if relevant).
 *  hPage       - The page on which the string lays (if relevant).
 *  strOffset   - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetReason(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionReason            eReason,
                             IN  RvChar                            *strReason,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                            strOffset)
{
    RvInt32      newStr;
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader->eReason = eReason;

    if(eReason == RVSIP_SUBSCRIPTION_REASON_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strReason,
                                      pHeader->hPool, pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strReason = newStr;
#ifdef SIP_DEBUG
        pHeader->pReason = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                        pHeader->strReason);
#endif
    }
    else
    {
        pHeader->strReason = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pReason = NULL;
#endif
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderSetReason
 * ------------------------------------------------------------------------
 * General: Sets the reason field in the Subscription-State header object.
 *          If the enumeration given by eReason is RVSIP_SUBSCRIPTION_REASON_OTHER,
 *          sets the reason string given by strReason in the
 *          Subscription-State header object.
 *          Use RVSIP_SUBSCRIPTION_REASON_OTHER when the substate you wish
 *          to set to the Subscription-State does not have a
 *          matching enumeration value in the RvSipSubscriptionReason enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Subscription-State header object.
 *    eReason   - The reason enumeration to be set in the Subscription-State
 *               header object. If RVSIP_SUBSCRIPTION_REASON_UNDEFINED is supplied,
 *               the existing reason is removed from the header.
 *    strReason - The reason string to be set in the Subscription-State header.
 *                (relevant when eReason is RVSIP_SUBSCRIPTION_REASON_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetReason(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionReason            eReason,
                             IN  RvChar                            *strReason)
{
    /* validity checking will be done in the internal function */
    return SipSubscriptionStateHeaderSetReason(hHeader,
                                                eReason,
                                                strReason,
                                                NULL,
                                                NULL_PAGE,
                                                UNDEFINED);
}

/***************************************************************************
 * SipSubscriptionStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the other Params value.
 * Return Value: other params string offset or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetOtherParams(
                               IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Subscription-State header other params field of the
 *          Subscription-State header object into a given buffer.
 *          Not all the Subscription-State header parameters have separated
 *          fields in the Subscribe-State header object. Parameters with no
 *          specific fields are refered to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Subscription-State header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a
 *                     NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderGetOtherParams(
                               IN  RvSipSubscriptionStateHeaderHandle hHeader,
                               IN  RvChar                           *strBuffer,
                               IN  RvUint                            bufferLen,
                               OUT RvUint                           *actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgSubscriptionStateHeader*)hHeader)->hPool,
                                  ((MsgSubscriptionStateHeader*)hHeader)->hPage,
                                  SipSubscriptionStateHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSubscriptionStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the other params in the
 *           SubscriptionState header object.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle of the Suibscription-State header object.
 *    strOtherParams - The other parameter to be set in the object.
 *                   If Null, the exist ViaParam string in the
 *                   object will be removed.
 *  hPool          - The pool on which the string lays (if relevant).
 *  hPage          - The page on which the string lays (if relevant).
 *  strOffset      - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetOtherParams(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvChar                            *strOtherParams,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                            strOffset)
{
    RvInt32      newStr;
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;
    RvStatus     retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, strOtherParams,
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
 * RvSipSubscriptionStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Subscription-State header object.
 *         Not all the Subscription-State header parameters have separated
 *         fields in the Subscription-State header object.
 *         Parameters with no specific fields are refered to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Subscription-State header object.
 *    strOtherParam - The extended parameters field to be set in the Subscription-State
 *                  header. If NULL is supplied, the existing extended parameters field
 *                  is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetOtherParams(
                                 IN    RvSipSubscriptionStateHeaderHandle hHeader,
                                 IN    RvChar                            *strOtherParam)
{
    /* validity checking will be done in the internal function */
    return SipSubscriptionStateHeaderSetOtherParams(hHeader,
                                                    strOtherParam,
                                                    NULL,
                                                    NULL_PAGE,
                                                    UNDEFINED);
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetExpiresParam
 * ------------------------------------------------------------------------
 * General: The Subscription-State header may contain an expires parameter.
 *          This function returns the expires parameter value.
 * Return Value: Returns the expires parameter value, UNDEFINED if not exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Subscription-State  header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSubscriptionStateHeaderGetExpiresParam(
                                         IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgSubscriptionStateHeader*)hHeader)->expiresVal;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderSetExpiresParam
 * ------------------------------------------------------------------------
 * General: Sets an expires parameter in the Subscription-State header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Subscription-State  header object.
 *    hExpires - Expires value to be set in the Subscription-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetExpiresParam(
                                 IN    RvSipSubscriptionStateHeaderHandle hHeader,
                                 IN    RvInt32                            expiresVal)
{
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;

    if(pHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(expiresVal < UNDEFINED)
    {
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderSetExpiresParam - negative expires value, unset expires value"));
        ((MsgSubscriptionStateHeader*)hHeader)->expiresVal = UNDEFINED;
    }
    else
    {
        ((MsgSubscriptionStateHeader*)hHeader)->expiresVal = expiresVal;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetRetryAfter
 * ------------------------------------------------------------------------
 * General: The Subscription-State header may contain a retry-after parameter.
 *          This function returns this parameter value.
 * Return Value: Returns the retry-after parameter value, UNDEFINED if not exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Subscription-State  header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSubscriptionStateHeaderGetRetryAfter(
                                IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgSubscriptionStateHeader*)hHeader)->retryAfterVal;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderSetRetryAfter
 * ------------------------------------------------------------------------
 * General: Sets a retry-after parameter in the Subscription-State header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Subscription-State  header object.
 *    hExpires - Retry-after value to be set in the Subscription-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetRetryAfter(
                                 IN    RvSipSubscriptionStateHeaderHandle hHeader,
                                 IN    RvInt32                            retryAfterVal)
{
    MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(retryAfterVal < UNDEFINED)
    {
        RvLogWarning(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSubscriptionStateHeaderSetRetryAfter - negative retryAfterVal, unset retry-after value"));
        pHeader->retryAfterVal = UNDEFINED;
    }
    else
    {
        pHeader->retryAfterVal = retryAfterVal;
    }

    return RV_OK;
}

/***************************************************************************
 * SipSubscriptionStateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrBadSyntax(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader)
{
    return ((MsgSubscriptionStateHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipSubscriptionStateHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad SubscriptionState header,
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
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderGetStrBadSyntax(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader,
                                    IN RvChar                            *strBuffer,
                                    IN RvUint                             bufferLen,
                                    OUT RvUint                           *actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgSubscriptionStateHeader*)hHeader)->hPool,
                                  ((MsgSubscriptionStateHeader*)hHeader)->hPage,
                                  SipSubscriptionStateHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSubscriptionStateHeaderSetStrBadSyntax
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
RvStatus SipSubscriptionStateHeaderSetStrBadSyntax(
                                  IN RvSipSubscriptionStateHeaderHandle hHeader,
                                  IN RvChar                            *strBadSyntax,
                                  IN HRPOOL                             hPool,
                                  IN HPAGE                              hPage,
                                  IN RvInt32                            strBadSyntaxOffset)
{
    MsgSubscriptionStateHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgSubscriptionStateHeader*)hHeader;

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
 * RvSipSubscriptionStateHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal SubscriptionState header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubscriptionStateHeaderSetStrBadSyntax(
                                IN RvSipSubscriptionStateHeaderHandle hHeader,
                                IN RvChar*                            strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipSubscriptionStateHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubscriptionStateHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void SubscriptionStateHeaderClean(IN MsgSubscriptionStateHeader* pHeader,
                                         IN RvBool               bCleanBS)
{
    pHeader->eReason        = RVSIP_SUBSCRIPTION_REASON_UNDEFINED;
    pHeader->strReason      = UNDEFINED;
    pHeader->eSubstate      = RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED;
    pHeader->strSubstate    = UNDEFINED;
    pHeader->expiresVal     = UNDEFINED;
    pHeader->retryAfterVal  = UNDEFINED;
    pHeader->strOtherParams = UNDEFINED;
    pHeader->eHeaderType    = RVSIP_HEADERTYPE_SUBSCRIPTION_STATE;

#ifdef SIP_DEBUG
    pHeader->pOtherParams   = NULL;
    pHeader->pReason        = NULL;
    pHeader->pSubstate      = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }

}

#endif /*#ifndef RV_SIP_PRIMITIVES */
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef __cplusplus
}
#endif

