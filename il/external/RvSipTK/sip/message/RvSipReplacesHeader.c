
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReplacesHeader.c                                  *
 *                                                                            *
 * The file defines the methods of the Replaces header object:                *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ----------                                            *
 *     Shiri Margel      Jun.2002                                             *
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipReplacesHeader.h"
#include "RvSipMsg.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipReplacesHeader.h"
#include "_SipReferToHeader.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


static RvStatus findNextParamName(IN     HRPOOL                    hPool,
                                   IN     HPAGE                     hPage,
                                   INOUT  RvInt32                  *offset,
                                   OUT    RvChar                   *paramName);

static RvStatus updateNextParamVal(IN    HRPOOL                    hPool,
                                    IN    HPAGE                     hPage,
                                    IN    RvChar                   *strToUpdate,
                                    INOUT RvInt32                  *offset,
                                    INOUT MsgReplacesHeader        **pReplacesHeader);

static RvStatus updateNextParamValInReplacesHeader(IN    HRPOOL              hPool,
                                                    IN    HPAGE               hPage,
                                                    IN    RvInt32            paramOffset,
                                                    IN    RvChar            *pParam,
                                                    IN    RvChar            *strToUpdate,
                                                    INOUT MsgReplacesHeader **pReplacesHeader);

static RvStatus changeFromKnownHexVals(IN    HRPOOL    hPool,
                                        IN    HPAGE     hPage,
                                        IN    RvInt32  paramOffset,
                                        OUT   RvInt32 *newParamOffset,
                                        OUT   RvBool  *pbFoundKnownHexVals);

static RvStatus changeToKnownHexVals(IN    HRPOOL    hPool,
                                        IN    HPAGE     hPage,
                                        IN    RvInt32  paramOffset,
                                        OUT   RvInt32 *newParamOffset,
                                        OUT   RvBool  *pbFoundKnownHexVals);

static void ReplacesHeaderClean( IN MsgReplacesHeader* pHeader,
                                 IN RvBool               bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipReplacesHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Replaces header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Replaces header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipReplacesHeaderHandle* hHeader)
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

    stat = RvSipReplacesHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                        msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_REPLACES,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * SipReplacesHeaderConstructInReferToHeader
 * ------------------------------------------------------------------------
 * General: Constructs a Replaces header inside a given Refer-To
 *          header. The address handle is returned.
 *          This is an internal function. the difference between this function,
 *          and the regular API function, is that it doesn't call to
 *          SipReferToRemoveOldReplacesFromHeadersList().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Refer-To header.
 * output: phReplacesHeader  - Handle to the newly constructed Replaces header.
 ***************************************************************************/
RvStatus RVCALLCONV SipReplacesHeaderConstructInReferToHeader(
                                          IN  RvSipReferToHeaderHandle   hHeader,
                                          OUT RvSipReplacesHeaderHandle *phReplacesHeader)
{
#ifdef RV_SIP_SUBS_ON
    MsgReferToHeader*   pHeader;
    RvStatus           stat = RV_OK;

    if(phReplacesHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phReplacesHeader=NULL;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferToHeader*)hHeader;

    /* creating the new url object */
    stat = RvSipReplacesHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                                        pHeader->hPool, pHeader->hPage, phReplacesHeader);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new url to the Refer-To header */
    pHeader->hReplacesHeader = *phReplacesHeader;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hHeader);
    *phReplacesHeader = NULL;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipReplacesHeaderConstructInReferToHeader
 * ------------------------------------------------------------------------
 * General: Constructs a Replaces header inside a given Refer-To
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Refer-To header.
 * output: phReplacesHeader  - Handle to the newly constructed Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderConstructInReferToHeader(
                                          IN  RvSipReferToHeaderHandle   hHeader,
                                          OUT RvSipReplacesHeaderHandle *phReplacesHeader)
{
    MsgReferToHeader*   pHeader;
    RvStatus           stat = RV_OK;

    if(phReplacesHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phReplacesHeader=NULL;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReferToHeader*)hHeader;

    stat = SipReferToRemoveOldReplacesFromHeadersList(hHeader);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* creating the new url object */
    stat = RvSipReplacesHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                                        pHeader->hPool, pHeader->hPage, phReplacesHeader);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associating the new url to the Refer-To header */
    pHeader->hReplacesHeader = *phReplacesHeader;

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipReplacesHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Replaces Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Replaces header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderConstruct(
                                        IN  RvSipMsgMgrHandle         hMsgMgr,
                                        IN  HRPOOL                    hPool,
                                        IN  HPAGE                     hPage,
                                        OUT RvSipReplacesHeaderHandle* hHeader)
{
    MsgReplacesHeader*  pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    /* allocate the header */
    pHeader = (MsgReplacesHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                        hPool,
                                                        hPage,
                                                        sizeof(MsgReplacesHeader),
                                                        RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReplacesHeaderConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;

    ReplacesHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipReplacesHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipReplacesHeaderConstruct - Replaces header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));


    return RV_OK;
}

/***************************************************************************
 * RvSipReplacesHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Replaces header object to a
 *          destination Refer-To header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Replaces header object.
 *    hSource      - Handle to the source Replaces header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderCopy(
                                    INOUT  RvSipReplacesHeaderHandle hDestination,
                                    IN     RvSipReplacesHeaderHandle hSource)
{
    MsgReplacesHeader* source;
    MsgReplacesHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgReplacesHeader*)hSource;
    dest   = (MsgReplacesHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* early-only flag */
    dest->eEarlyFlagParamType = source->eEarlyFlagParamType;

    /* From Tag */
    if(source->strFromTag > UNDEFINED)
    {
        dest->strFromTag = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                           dest->hPool,
                                                           dest->hPage,
                                                           source->hPool,
                                                           source->hPage,
                                                           source->strFromTag);

        if (dest->strFromTag == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderCopy - Failed to copy From Tag param."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pFromTag = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                  dest->strFromTag);
#endif
    }
    else
    {
        dest->strFromTag = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pFromTag = NULL;
#endif
    }
    /* To Tag */
    if(source->strToTag > UNDEFINED)
    {
        dest->strToTag = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                           dest->hPool,
                                                           dest->hPage,
                                                           source->hPool,
                                                           source->hPage,
                                                           source->strToTag);

        if (dest->strToTag == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderCopy - Failed to copy To Tag param."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pToTag = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                  dest->strToTag);
#endif
    }
    else
    {
        dest->strToTag = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pToTag = NULL;
#endif
    }
    /* Call-ID */
    if(source->strCallID > UNDEFINED)
    {
        dest->strCallID = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                           dest->hPool,
                                                           dest->hPage,
                                                           source->hPool,
                                                           source->hPage,
                                                           source->strCallID);

        if (dest->strCallID == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderCopy - Failed to copy Call-ID param."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCallID = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                  dest->strCallID);
#endif
    }
    else
    {
        dest->strCallID = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCallID = NULL;
#endif
    }
    /* Other Parameters */
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
                "RvSipReplacesHeaderCopy - Failed to copy other parameters string."));
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
                "RvSipReplacesHeaderCopy - failed in coping strBadSyntax."));
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
 * ReplacesHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Replaces header (as string) on it.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Replaces header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReplacesHeaderEncode(
                            IN    RvSipReplacesHeaderHandle hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                  bInUrlHeaders,
                            INOUT RvUint32*               length)
{
    MsgReplacesHeader*  pHeader;
    RvStatus            stat = RV_OK;
    RvBool              bFoundHexVals = RV_FALSE;
    RvInt32             newOffset = UNDEFINED;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgReplacesHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ReplacesHeaderEncode - Encoding Replaces header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Replaces", length);
    if(stat != RV_OK)
    {
        return stat;
    }

    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);
    if(stat != RV_OK)
    {
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
                "ReplacesHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /*call-id*/
    if(pHeader->strCallID > UNDEFINED)
    {
        if(bInUrlHeaders == RV_TRUE)
        {
            /* change the '@' in the call-id string to escaped value */
            stat = changeToKnownHexVals(pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strCallID,
                                    &newOffset,
                                    &bFoundHexVals);
             if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                    stat));
                return stat;
            }
        }
        if(bFoundHexVals == RV_FALSE)
        {
            newOffset = pHeader->strCallID;
        }
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    newOffset,
                                    length);

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
    }
    else
    {
        /* call-id is a mandatory parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReplacesHeaderEncode: Failed. no call-id parameter."));
        return RV_ERROR_UNKNOWN;
    }

    /* to-tag */
    if(pHeader->strToTag > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                    MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "to-tag", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
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
                                    pHeader->strToTag,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
    }
    else
    {
        /* to-tag is a mandatory parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReplacesHeaderEncode: Failed. no to-tag parameter."));
        return RV_ERROR_UNKNOWN;
    }

    /*from-tag*/
    if(pHeader->strFromTag > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                    MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "from-tag", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        /* = */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }

        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strFromTag,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
    }
    else
    {
        /* from-tag is a mandatory parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ReplacesHeaderEncode: Failed. no from-tag parameter."));
        return RV_ERROR_UNKNOWN;
    }

    /*other params*/
    if(pHeader->strOtherParams > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                    MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
    }

    /* encode ";early-only" */
    if(RVSIP_REPLACES_EARLY_FLAG_TYPE_UNDEFINED != pHeader->eEarlyFlagParamType)
    {
        /*;*/
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders),
                                        length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "early-only", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                stat));
            return stat;
        }
        if(RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_1== pHeader->eEarlyFlagParamType)
        {
            /*=*/
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),
                                            length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                    stat));
                return stat;
            }
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "1", length);
        }
        else if (RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_TRUE== pHeader->eEarlyFlagParamType)
        {
            /*=*/
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),
                                            length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ReplacesHeaderEncode: Failed to encode Replaces header. stat is %d",
                    stat));
                return stat;
            }
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "true", length);
        }
    }
    return stat;
}

/***************************************************************************
 * RvSipReplacesHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Replaces header object to a textual Replaces header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Replaces header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderEncode(
                                        IN    RvSipReplacesHeaderHandle hHeader,
                                        IN    HRPOOL                   hPool,
                                        OUT   HPAGE*                   hPage,
                                        OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if ((hPool == NULL) || (pHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ReplacesHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplacesHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ReplacesHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * RvSipReplacesHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Replaces header-for example,
 *         "Replaces: 123@sshhdd;to-tag=asdf;from-tag=12345"-into a Replaces header
 *           object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Replaces header object.
 *    buffer    - Buffer containing a textual Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderParse(
                                     IN  RvSipReplacesHeaderHandle  hHeader,
                                     IN  RvChar*                   buffer)
{
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ReplacesHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REPLACES,
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
 * RvSipReplacesHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Replaces header value into an Replaces header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReplacesHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Replaces header object.
 *    buffer    - The buffer containing a textual Replaces header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderParseValue(
                                     IN  RvSipReplacesHeaderHandle  hHeader,
                                     IN  RvChar*                   buffer)
{
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ReplacesHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_REPLACES,
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
 * RvSipReplacesHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Replaces header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderFix(
                                     IN RvSipReplacesHeaderHandle hHeader,
                                     IN RvChar*                  pFixedBuffer)
{
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipReplacesHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReplacesHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipReplacesHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipReplacesHeaderGetToTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the To Tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetToTag(IN RvSipReplacesHeaderHandle hHeader)
{
    return ((MsgReplacesHeader *)hHeader)->strToTag;
}

/***************************************************************************
 * RvSipReplacesHeaderGetToTag
 * ------------------------------------------------------------------------
 * General: Copies the To Tag parameter of the Replaces header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetToTag(
                                               IN RvSipReplacesHeaderHandle   hHeader,
                                               IN RvChar*                    strBuffer,
                                               IN RvUint                     bufferLen,
                                               OUT RvUint*                   actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReplacesHeader*)hHeader)->hPool,
                                  ((MsgReplacesHeader*)hHeader)->hPage,
                                  SipReplacesHeaderGetToTag(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplacesHeaderGetFromTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the From Tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetFromTag(IN RvSipReplacesHeaderHandle hHeader)
{
    return ((MsgReplacesHeader *)hHeader)->strFromTag;
}

/***************************************************************************
 * RvSipReplacesHeaderGetFromTag
 * ------------------------------------------------------------------------
 * General: Copies the From Tag parameter of the Replaces header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetFromTag(
                                               IN RvSipReplacesHeaderHandle   hHeader,
                                               IN RvChar*                    strBuffer,
                                               IN RvUint                     bufferLen,
                                               OUT RvUint*                   actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReplacesHeader*)hHeader)->hPool,
                                  ((MsgReplacesHeader*)hHeader)->hPage,
                                  SipReplacesHeaderGetFromTag(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplacesHeaderGetCallID
 * ------------------------------------------------------------------------
 * General: This method retrieves the Call-ID field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetCallID(IN RvSipReplacesHeaderHandle hHeader)
{
    return ((MsgReplacesHeader *)hHeader)->strCallID;
}

/***************************************************************************
 * RvSipReplacesHeaderGetCallID
 * ------------------------------------------------------------------------
 * General: Copies the Call ID parameter of the Replaces header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetCallID(
                                               IN RvSipReplacesHeaderHandle   hHeader,
                                               IN RvChar*                    strBuffer,
                                               IN RvUint                     bufferLen,
                                               OUT RvUint*                   actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgReplacesHeader*)hHeader)->hPool,
                                  ((MsgReplacesHeader*)hHeader)->hPage,
                                  SipReplacesHeaderGetCallID(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplacesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the Call-ID field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetOtherParams(IN RvSipReplacesHeaderHandle hHeader)
{
    return ((MsgReplacesHeader *)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipReplacesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other parameters string of the Replaces header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetOtherParams(
                                    IN RvSipReplacesHeaderHandle      hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReplacesHeader*)hHeader)->hPool,
                                  ((MsgReplacesHeader*)hHeader)->hPage,
                                  SipReplacesHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplacesHeaderSetToTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strToTag in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strToTag  - The to tag string to be set - if Null, the exist to tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strToTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetToTag(IN    RvSipReplacesHeaderHandle hHeader,
                                   IN    RvChar *              strToTag,
                                   IN    HRPOOL                 hPool,
                                   IN    HPAGE                  hPage,
                                   IN    RvInt32               strToTagOffset)
{
    RvInt32        newStr;
    MsgReplacesHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplacesHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strToTagOffset, strToTag,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strToTag = newStr;
#ifdef SIP_DEBUG
    pHeader->pToTag = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strToTag);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReplacesHeaderSetToTag
 * ------------------------------------------------------------------------
 * General: Sets the To Tag field in the Replaces header object
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Replaces header object.
 *    strToTag  - The To Tag field to be set in the Replaces header object. If NULL
 *              is supplied, the existing To Tag is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetToTag(
                                         IN    RvSipReplacesHeaderHandle hHeader,
                                         IN    RvChar *                 strToTag)
{
    /* validity checking will be done in the internal function */
    return SipReplacesHeaderSetToTag(hHeader, strToTag, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReplacesHeaderSetFromTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strFromTag in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strFromTag  - The from tag string to be set - if Null, the exist from tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strFromTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetFromTag(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strFromTag,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strFromTagOffset)
{
    RvInt32        newStr;
    MsgReplacesHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplacesHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strFromTagOffset, strFromTag,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strFromTag = newStr;
#ifdef SIP_DEBUG
    pHeader->pFromTag = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strFromTag);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReplacesHeaderSetFromTag
 * ------------------------------------------------------------------------
 * General: Sets the From Tag field in the Replaces header object
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Replaces header object.
 *    strToTag  - The From Tag field to be set in the Replaces header object.  If NULL
 *              is supplied, the existing From Tag is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetFromTag(
                                         IN    RvSipReplacesHeaderHandle hHeader,
                                         IN    RvChar *              strFromTag)
{
    /* validity checking will be done in the internal function */
    return SipReplacesHeaderSetFromTag(hHeader, strFromTag, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReplacesHeaderSetCallID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strCallID in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strCallID  - The Call-ID string to be set - if Null, the exist Call-ID in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strCallIDOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetCallID(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strCallID,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strCallIDOffset)
{
    RvInt32        newStr;
    MsgReplacesHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplacesHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strCallIDOffset, strCallID,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strCallID = newStr;
#ifdef SIP_DEBUG
    pHeader->pCallID = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                  pHeader->strCallID);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipReplacesHeaderSetCallID
 * ------------------------------------------------------------------------
 * General: Sets the Call-ID field in the Replaces header object
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Replaces header object.
 *    strToTag  - The Call-ID field to be set in the Replaces header object. If NULL
 *              is supplied, the existing Call-ID is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetCallID(
                                         IN    RvSipReplacesHeaderHandle hHeader,
                                         IN    RvChar *              strCallID)
{
    /* validity checking will be done in the internal function */
    return SipReplacesHeaderSetCallID(hHeader, strCallID, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipReplacesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strOtherParams  - The OtherParams string to be set - if Null, the exist Call-ID in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOtherParamsOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetOtherParams(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strOtherParams,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOtherParamsOffset)
{
    RvInt32        newStr;
    MsgReplacesHeader* pHeader;
    RvStatus       retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgReplacesHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOtherParamsOffset, strOtherParams,
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
 * RvSipReplacesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other parameters string in the Replaces header object
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Replaces header object.
 *    strToTag  - The other parameters string to be set in the Replaces header object. If
 *                NULL is supplied, the existing other parameters string is removed from
 *              the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetOtherParams(
                                 IN   RvSipReplacesHeaderHandle  hHeader,
                                 IN   RvChar                    *otherParams)
{
    /* validity checking will be done in the internal function */
    return SipReplacesHeaderSetOtherParams(hHeader, otherParams, NULL, NULL_PAGE, UNDEFINED);
}



/***************************************************************************
 * RvSipReplaceHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Replaces header fields are kept in a string format-
 *          for example, the Refer-To header from tag. In order to get
 *          such a field from the Refer-To header object, your application
 *          should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to
 *          enable you to allocate an appropriate buffer size before calling
 *          the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Replaces header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReplaceHeaderGetStringLength(
                                IN  RvSipReplacesHeaderHandle     hHeader,
                                IN  RvSipReplacesHeaderStringName stringName)
{
    RvInt32 stringOffset;

    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_REPLACES_CALL_ID:
        {
            stringOffset = SipReplacesHeaderGetCallID(hHeader);
            break;
        }
        case RVSIP_REPLACES_FROM_TAG:
        {
            stringOffset = SipReplacesHeaderGetFromTag(hHeader);
            break;
        }
        case RVSIP_REPLACES_TO_TAG:
        {
            stringOffset = SipReplacesHeaderGetToTag(hHeader);
            break;
        }
        case RVSIP_REPLACES_OTHER_PARAMS:
        {
            stringOffset = SipReplacesHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_REPLACES_BAD_SYNTAX:
        {
            stringOffset = SipReplacesHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipReplaceHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipReplacesHeaderGetPool(hHeader),
                             SipReplacesHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * SipReplacesHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Replaces header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReplaces - The Replaces header to take the page from.
 ***************************************************************************/
HRPOOL SipReplacesHeaderGetPool(RvSipReplacesHeaderHandle hReplaces)
{
    return ((MsgReplacesHeader*)hReplaces)->hPool;
}

/***************************************************************************
 * SipReplacesHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Replaces header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReplaces - The Replaces header to take the page from.
 ***************************************************************************/
HPAGE SipReplacesHeaderGetPage(RvSipReplacesHeaderHandle hReplaces)
{
    return ((MsgReplacesHeader*)hReplaces)->hPage;
}

/***************************************************************************
 * SipReplacesHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetStrBadSyntax(
                                    IN RvSipReplacesHeaderHandle hHeader)
{
    return ((MsgReplacesHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipReplacesHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Replaces header,
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
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetStrBadSyntax(
                                               IN RvSipReplacesHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgReplacesHeader*)hHeader)->hPool,
                                  ((MsgReplacesHeader*)hHeader)->hPage,
                                  SipReplacesHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipReplacesHeaderSetStrBadSyntax
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
RvStatus SipReplacesHeaderSetStrBadSyntax(
                                  IN RvSipReplacesHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgReplacesHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgReplacesHeader*)hHeader;

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
 * RvSipReplacesHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Replaces header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetStrBadSyntax(
                                  IN RvSipReplacesHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipReplacesHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/***************************************************************************
 * SipReplacesHeaderParseFromAddrSpec
 * ------------------------------------------------------------------------
 * General: Parse a replaces header from address spec.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool - Handle to the pool where the Replaces header in the address
 *                    spec is saved.
 *            hPage - Handle to the page where the Replaces header in the address
 *                    spec is saved.
 *            offset - The offset of the Replaces header in the address spec.
 *                     We will parse the Replaces header till we reach the '&' or '\0'.
 ***************************************************************************/
RvStatus SipReplacesHeaderParseFromAddrSpec(IN  HRPOOL                     hPool,
                                             IN  HPAGE                      hPage,
                                             IN  RvInt32                   offset,
                                             OUT RvSipReplacesHeaderHandle *hReplacesHeader)
{
    RvStatus           rv = RV_OK;
    RvInt32            newOffset;
    RvInt32            length;
    RvBool             bFindChar = RV_FALSE;
    MsgReplacesHeader  *pReplacesHeader;
    RvInt32            safeCounter = 0;
    RvBool             bToTagExists = RV_FALSE;
    RvBool             bFromTagExists = RV_FALSE;

    if(hReplacesHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(*hReplacesHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pReplacesHeader = (MsgReplacesHeader *)*hReplacesHeader;

    length = RPOOL_Strlen(hPool, hPage, offset);

    /* searches for the = after the 'Replaces', and before the Call-ID */
    bFindChar = MsgUtilsFindCharInPage(hPool, hPage, offset,
                                        length, '=', &newOffset);

    if(bFindChar == RV_FALSE)
    {
        return RV_ERROR_UNKNOWN;
    }
    /* parse Call-ID */
    rv = updateNextParamVal(hPool, hPage, "Call-ID", &newOffset, &pReplacesHeader);

    while(newOffset != UNDEFINED && safeCounter < 1000)
    {
        RvChar paramName[10];

        /* Checks the name of the next parameter in the Replaces header */
        rv = findNextParamName(hPool, hPage, &newOffset, paramName);
        if(rv != RV_OK)
        {
            return rv;
        }
        else if(strcmp(paramName, "to-tag") == 0)
        {
            if(bToTagExists == RV_TRUE)
            {
                return RV_ERROR_UNKNOWN;
            }
            bToTagExists = RV_TRUE;
            /* Parse to tag */
            rv = updateNextParamVal(hPool, hPage, "to-tag", &newOffset, &pReplacesHeader);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
        else if(strcmp(paramName, "from-tag") == 0)
        {
            if(bFromTagExists == RV_TRUE)
            {
                return RV_ERROR_UNKNOWN;
            }
            bFromTagExists = RV_TRUE;
            /* Parse from tag */
            rv = updateNextParamVal(hPool, hPage, "from-tag", &newOffset, &pReplacesHeader);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
        else
        {
            /* Parse other param (generic-param) */
            rv = updateNextParamVal(hPool, hPage, "other", &newOffset, &pReplacesHeader);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
    }
    if(safeCounter == 1000)
    {
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipReplacesHeaderSetEarlyFlagParam
 * ------------------------------------------------------------------------
 * General: Sets the early flag parameter..
 *          The BNF definition of this parameter is "early-only;".
 *          However, you can also set this parameter to be "early-only=1;"
 *          or "early-only=true;" (these options are not standard compliant!!!)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - The handle to the header object.
 *  eEarlyFlag - The early flag value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderSetEarlyFlagParam(
                                  IN RvSipReplacesHeaderHandle hHeader,
                                  IN RvSipReplacesEarlyFlagType eEarlyFlag)
{
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;
    if(pHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader->eEarlyFlagParamType = eEarlyFlag;
    return RV_OK;

}

/***************************************************************************
 * RvSipReplacesHeaderGetEarlyFlagParam
 * ------------------------------------------------------------------------
 * General: Gets the early flag parameter..
 *          The BNF definition of this parameter is "early-only;".
 *          However, you can also set this parameter to be "early-only=1;"
 *          or "early-only=true;" (these options are not standard compliant!!!)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - The handle to the header object.
 *  peEarlyFlag - The early flag value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplacesHeaderGetEarlyFlagParam(
                                  IN  RvSipReplacesHeaderHandle   hHeader,
                                  OUT RvSipReplacesEarlyFlagType *peEarlyFlag)
{
    MsgReplacesHeader* pHeader = (MsgReplacesHeader*)hHeader;
    if(pHeader == NULL || NULL == peEarlyFlag)
        return RV_ERROR_INVALID_HANDLE;

    *peEarlyFlag = pHeader->eEarlyFlagParamType;
    return RV_OK;

}
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * findNextParamName
 * ------------------------------------------------------------------------
 * General: Finds the next parameter name in the Replaces header in address spec.
 *          the offset to start looking is after %3B (';'). If the name is to-tag or from-tag
 *          this function wiil set the offset to point to after the to-tag= / from-tag=,
 *          otherwise the offset remains as it was.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool - Handle to the pool where the Replaces header in the address
 *                    spec is saved.
 *            hPage - Handle to the page where the Replaces header in the address
 *                    spec is saved.
 *            offset - The offset from which to find the next parameter name.
 *  Output:   offset - If the name is to-tag or from-tag this function wiil set the offset
 *                     to point to after the to-tag= / from-tag=, otherwise the offset
 *                     remains as it was.
 *            paramName - The name of the next parameter: to-tag / from-tag / other.
 ***************************************************************************/
static RvStatus findNextParamName(IN     HRPOOL                    hPool,
                                   IN     HPAGE                     hPage,
                                   INOUT  RvInt32                  *offset,
                                   OUT    RvChar                   *paramName)
{
    RvBool   bFindEqual = RV_FALSE;
    RvBool   bFindChar  = RV_FALSE;
    RvInt32  newOffset;
    RvInt32  length;
    RvInt32  safeCounter = 0;

    while(bFindEqual == RV_FALSE && safeCounter < 1000)
    {
        length = RPOOL_Strlen(hPool, hPage, *offset);

        bFindChar = MsgUtilsFindCharInPage(hPool, hPage, *offset,
                                        length, '%', &newOffset);

        if(bFindChar == RV_FALSE)
        {
            strcpy(paramName, "other");
            return RV_OK;
        }
        if(RPOOL_CmpiPrefixToExternal(hPool, hPage, newOffset, "3D") == RV_TRUE)
        {
            length = (RPOOL_Strlen(hPool, hPage, *offset) - RPOOL_Strlen(hPool, hPage, newOffset)) - 1;
            if(length > 9)
            {
                length = 9;
            }
            RPOOL_CopyToExternal(hPool, hPage, *offset, paramName, length);
            paramName[length] = '\0';
            bFindEqual = RV_TRUE;
            /* Update the new offset only for to-tag and from-tag because other header is copied
               from the ';' (%3B) before it and from the '=' (%3D) */
            if((strcmp(paramName, "to-tag") == 0) || (strcmp(paramName, "from-tag") == 0))
            {
                *offset = newOffset + 2;
            }
        }
        else if(RPOOL_CmpiPrefixToExternal(hPool, hPage, newOffset, "3B") == RV_TRUE)
        /* other param - there is no '=' before the ';' */
        {
            strcpy(paramName, "other");
            return RV_OK;
        }
        safeCounter++;
    }
    return RV_OK;
}

/***************************************************************************
 * updateNextParamVal
 * ------------------------------------------------------------------------
 * General: Updates the next parameter of the Replaces header in the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool - Handle to the pool where the Replaces header in the address
 *                    spec is saved.
 *            hPage - Handle to the page where the Replaces header in the address
 *                    spec is saved.
 *            strToUpdate - The parameter from the Replaces header to update:
 *                          to-tag/from-tag/call-id/other
 *            offset - The offset from which to find the next parameter value.
 *            pReplacesHeader - The replaces header to update.
 *  Output:   offset - The offset after the paramater value. If the parameter is the last in the
 *                     Replaces header UNDEFINED is set.
 *            pReplacesHeader - The updated replaces header.
 ***************************************************************************/
static RvStatus updateNextParamVal(IN    HRPOOL                    hPool,
                                    IN    HPAGE                     hPage,
                                    IN    RvChar                   *strToUpdate,
                                    INOUT RvInt32                  *offset,
                                    INOUT MsgReplacesHeader        **pReplacesHeader)
{
    RvBool  bUpdateParamVal = RV_FALSE;
    RvBool  bFindChar       = RV_FALSE;
    RvInt32 newOffset;
    RvInt32 tmpOffset       = *offset;
    RvInt32 length;
    RvInt32 paramOffset     = UNDEFINED;
    RvInt32 size;
    RvStatus rv             = RV_OK;
    RvInt32  safeCounter    = 0;
    RvChar *pParam          = NULL;

    /* parse param value */
    while(bUpdateParamVal == RV_FALSE && safeCounter < 1000)
    {
        length = RPOOL_Strlen(hPool, hPage, tmpOffset);

        bFindChar = MsgUtilsFindCharInPage(hPool, hPage, tmpOffset,
                                            length, '%', &newOffset);

        if(bFindChar == RV_FALSE)
        {
            bFindChar = MsgUtilsFindCharInPage(hPool, hPage, tmpOffset,
                                    length, '&', &newOffset);
            if(bFindChar == RV_FALSE)
            {
                length = RPOOL_Strlen(hPool, hPage, *offset);
            }
            else
            {
                length = RPOOL_Strlen(hPool, hPage, *offset) - RPOOL_Strlen(hPool, hPage, newOffset);
            }
            rv = RPOOL_Append((*pReplacesHeader)->hPool, (*pReplacesHeader)->hPage, length + 1,
                                                                RV_FALSE, &paramOffset);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RPOOL_CopyFrom((*pReplacesHeader)->hPool, (*pReplacesHeader)->hPage, paramOffset,
                                    hPool, hPage, *offset, length);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RPOOL_CopyFromExternal((*pReplacesHeader)->hPool,
                                        (*pReplacesHeader)->hPage,paramOffset + length,"\0",1);

#ifdef SIP_DEBUG
            pParam = (RvChar *)RPOOL_GetPtr((*pReplacesHeader)->hPool,
                                                    (*pReplacesHeader)->hPage,
                                                    paramOffset);
#endif
            bUpdateParamVal = RV_TRUE;
            *offset = UNDEFINED;

        }
        /* check weather the % is a start of %3B (;) */
        else if(RPOOL_CmpiPrefixToExternal(hPool, hPage, newOffset, "3B") == RV_TRUE)
        {
            length = RPOOL_Strlen(hPool, hPage, *offset) - RPOOL_Strlen(hPool, hPage, newOffset);
            rv = RPOOL_Append((*pReplacesHeader)->hPool, (*pReplacesHeader)->hPage, length, RV_FALSE, &paramOffset);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RPOOL_CopyFrom((*pReplacesHeader)->hPool, (*pReplacesHeader)->hPage, paramOffset,
                hPool, hPage, *offset, length - 1);
            if(rv != RV_OK)
            {
                return rv;
            }
            size = RPOOL_GetSize((*pReplacesHeader)->hPool, (*pReplacesHeader)->hPage);
            rv = RPOOL_CopyFromExternal((*pReplacesHeader)->hPool,(*pReplacesHeader)->hPage,size-1,"\0",1);
            if(rv != RV_OK)
            {
                return rv;
            }
    #ifdef SIP_DEBUG
            pParam = (RvChar *)RPOOL_GetPtr((*pReplacesHeader)->hPool,
                                  (*pReplacesHeader)->hPage,
                                  paramOffset);
    #endif
            bUpdateParamVal = RV_TRUE;
            *offset = newOffset+2;
        }
        else
        {
            tmpOffset = newOffset;
        }
        safeCounter++;
    }
    if(safeCounter == 1000)
    {
        return RV_ERROR_UNKNOWN;
    }
    /* find param offset in the Replaces header */
    return updateNextParamValInReplacesHeader(hPool, hPage, paramOffset, pParam, strToUpdate, pReplacesHeader);
}

/***************************************************************************
 * updateNextParamValInReplacesHeader
 * ------------------------------------------------------------------------
 * General: Updates the next parameter of the Replaces header in the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool - Handle to the pool where the Replaces header in the address
 *                    spec is saved.
 *            hPage - Handle to the page where the Replaces header in the address
 *                    spec is saved.
 *            paramOffset - The parameter from the Replaces header to update:
 *                          to-tag/from-tag/call-id/other.
 *            pParam - Pointer to the parameter string.
 *            strToUpdate - to-tag/from-tag/call-id/other
 *            pReplacesHeader - The replaces header to update.
 *  Output:   pReplacesHeader - The updated replaces header.
 ***************************************************************************/
static RvStatus updateNextParamValInReplacesHeader(IN    HRPOOL              hPool,
                                                    IN    HPAGE               hPage,
                                                    IN    RvInt32            paramOffset,
                                                    IN    RvChar            *pParam,
                                                    IN    RvChar            *strToUpdate,
                                                    INOUT MsgReplacesHeader **pReplacesHeader)
{
    RvStatus rv = RV_OK;
    RvInt32  length;
    RvInt32  newOffset = 0;
    RvBool   bFoundKnownHexVals = RV_FALSE;
	
	RV_UNUSED_ARG(pParam)

    if(strcmp(strToUpdate, "Call-ID") == 0)
    {
        /* for now the only known Hex param in a Call-ID is @ (this is not allowed in tags */
        rv = changeFromKnownHexVals(hPool, hPage, paramOffset, &newOffset, &bFoundKnownHexVals);
        if(rv != RV_OK)
        {
            return rv;
        }
        if(bFoundKnownHexVals == RV_TRUE)
        {
            (*pReplacesHeader)->strCallID = newOffset;
#ifdef SIP_DEBUG
            (*pReplacesHeader)->pCallID = (RvChar *)RPOOL_GetPtr(hPool, hPage, newOffset);
#endif
        }
        else
        {
            (*pReplacesHeader)->strCallID = paramOffset;
#ifdef SIP_DEBUG
            (*pReplacesHeader)->pCallID = pParam;
#endif
        }
    }
    else if(strcmp(strToUpdate, "to-tag") == 0)
    {
        (*pReplacesHeader)->strToTag = paramOffset;
#ifdef SIP_DEBUG
        (*pReplacesHeader)->pToTag = pParam;
#endif
    }
    else if(strcmp(strToUpdate, "from-tag") == 0)
    {
        (*pReplacesHeader)->strFromTag = paramOffset;
#ifdef SIP_DEBUG
        (*pReplacesHeader)->pFromTag = pParam;
#endif
    }
    else
    {
        if((*pReplacesHeader)->strOtherParams == UNDEFINED)
        {
            (*pReplacesHeader)->strOtherParams = paramOffset;
#ifdef SIP_DEBUG
            (*pReplacesHeader)->pOtherParams = pParam;
#endif
        }
        else /* Append the found string to the end of the other params */
        {
            RvInt32 otherCurLength;
            RvInt32 otherOffset;

            length = RPOOL_Strlen(hPool, hPage, paramOffset);

            otherCurLength = RPOOL_Strlen((*pReplacesHeader)->hPool,
                                          (*pReplacesHeader)->hPage,
                                          (*pReplacesHeader)->strOtherParams);

            rv = RPOOL_Append((*pReplacesHeader)->hPool,
                              (*pReplacesHeader)->hPage,
                              length + otherCurLength + 2,
                              RV_FALSE,
                              &otherOffset);

            if(rv != RV_OK)
            {
                return rv;
            }
            rv =  RPOOL_CopyFrom((*pReplacesHeader)->hPool,
                                 (*pReplacesHeader)->hPage,
                                 otherOffset,
                                 (*pReplacesHeader)->hPool,
                                 (*pReplacesHeader)->hPage,
                                 (*pReplacesHeader)->strOtherParams,
                                 otherCurLength);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RPOOL_CopyFromExternal((*pReplacesHeader)->hPool,
                                        (*pReplacesHeader)->hPage,
                                        otherOffset + otherCurLength,
                                        ";",
                                        1);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv =  RPOOL_CopyFrom((*pReplacesHeader)->hPool,
                                 (*pReplacesHeader)->hPage,
                                 otherOffset + otherCurLength + 1,
                                 (*pReplacesHeader)->hPool,
                                 (*pReplacesHeader)->hPage,
                                 paramOffset,
                                 length);
            if(rv != RV_OK)
            {
                return rv;
            }

            rv = RPOOL_CopyFromExternal((*pReplacesHeader)->hPool,
                                        (*pReplacesHeader)->hPage,
                                        otherOffset + otherCurLength + length + 1,
                                        "\0",
                                        1);
            if(rv != RV_OK)
            {
                return rv;
            }
            (*pReplacesHeader)->strOtherParams = otherOffset;

#ifdef SIP_DEBUG
            (*pReplacesHeader)->pOtherParams = (RvChar *)RPOOL_GetPtr((*pReplacesHeader)->hPool,
                                                         (*pReplacesHeader)->hPage,
                                                         (*pReplacesHeader)->strOtherParams);
#endif
        }
    }
    return RV_OK;
}

/***************************************************************************
 * changeFromKnownHexVals
 * ------------------------------------------------------------------------
 * General: Change the known Hex vals in the received string (in rpool) to ascii.
 *          The known Hex vals are: '@' = '%40', which can appear once in Call-ID.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool               - Handle to the pool
 *            hPage               - Handle to the page
 *            paramOffset         - The offset that the string starts in the page
 *  Output:   newParamOffset      - The new offset of the string without the known Hex
 *                                  values (will be updated only if known Hex values will
 *                                  be found.
 *            pbFoundKnownHexVals - RV_TRUE if Hex values were found, RV_FALSE otherwise.
 ***************************************************************************/
static RvStatus changeFromKnownHexVals(IN    HRPOOL    hPool,
                                        IN    HPAGE     hPage,
                                        IN    RvInt32  paramOffset,
                                        OUT   RvInt32 *newParamOffset,
                                        OUT   RvBool  *pbFoundKnownHexVals)
{
    RvBool   bFindChar         = RV_FALSE;

    RvInt32  newOffset;
    RvInt32  length;
    RvInt32  oldLength;
    RvStatus rv                = RV_OK;

    *pbFoundKnownHexVals    = RV_FALSE;
    *newParamOffset         = 0;
    newOffset               = paramOffset;

   length = RPOOL_Strlen(hPool, hPage, paramOffset);

   bFindChar = MsgUtilsFindCharInPage(hPool, hPage, paramOffset,
                                           length, '%', &newOffset);

   if(bFindChar == RV_FALSE)
   {
       *pbFoundKnownHexVals = RV_FALSE;
       return RV_OK;
   }

   if(RPOOL_CmpPrefixToExternal(hPool, hPage, newOffset, "40") == RV_TRUE) /* @ */
   {
       *pbFoundKnownHexVals = RV_TRUE;
       rv = RPOOL_Append(hPool, hPage, length-1, RV_FALSE, newParamOffset);
       if(rv != RV_OK)
       {
             return rv;
       }

       length = (RPOOL_Strlen(hPool, hPage, paramOffset) - RPOOL_Strlen(hPool, hPage, newOffset)) - 1;

       rv = RPOOL_CopyFrom(hPool, hPage, *newParamOffset, hPool, hPage, paramOffset, length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = RPOOL_CopyFromExternal(hPool, hPage, *newParamOffset + length, "@", 1);
        if(rv != RV_OK)
        {
            return rv;
        }
        newOffset = newOffset + 2;

        oldLength = length;
        length = RPOOL_Strlen(hPool, hPage, newOffset);

        rv = RPOOL_CopyFrom(hPool, hPage, *newParamOffset + oldLength + 1, hPool, hPage, newOffset, length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = RPOOL_CopyFromExternal(hPool, hPage, *newParamOffset + length + oldLength + 1, "\0", 1);
    }
    return rv;
}

/***************************************************************************
 * changeToKnownHexVals
 * ------------------------------------------------------------------------
 * General: Change the ascii vals in the received string (in rpool) to Hex values
 *          (for specific values).
 *          The known vals are: '%40' = '@' which can appear once in Call-ID.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool               - Handle to the pool
 *            hPage               - Handle to the page
 *            paramOffset         - The offset that the string starts in the page
 *  Output:   newParamOffset      - The new offset of the string with the known Hex
 *                                  values (will be updated only if the specific ascii
 *                                  values will be found.
 *            pbFoundKnownHexVals - RV_TRUE if the specific ascii values were found, RV_FALSE otherwise.
 ***************************************************************************/
static RvStatus changeToKnownHexVals(IN    HRPOOL    hPool,
                                      IN    HPAGE     hPage,
                                      IN    RvInt32  paramOffset,
                                      OUT   RvInt32 *newParamOffset,
                                      OUT   RvBool  *pbFoundKnownHexVals)
{
    RvBool   bFindChar         = RV_FALSE;

    RvInt32  newOffset;
    RvInt32  length;
    RvInt32  oldLength;
    RvStatus rv                = RV_OK;

    *pbFoundKnownHexVals    = RV_FALSE;
    *newParamOffset         = 0;

    length = RPOOL_Strlen(hPool, hPage, paramOffset);

    bFindChar = MsgUtilsFindCharInPage(hPool, hPage, paramOffset,
                                       length, '@', &newOffset);

    if(bFindChar == RV_FALSE)
    {
        *pbFoundKnownHexVals = RV_FALSE;
        return RV_OK;
    }

    *pbFoundKnownHexVals = RV_TRUE;

     rv = RPOOL_Append(hPool, hPage, length+3, RV_FALSE, newParamOffset);
     if(rv != RV_OK)
     {
         return rv;
     }


    length = (RPOOL_Strlen(hPool, hPage, paramOffset) - RPOOL_Strlen(hPool, hPage, newOffset)) - 1;

    rv = RPOOL_CopyFrom(hPool, hPage, *newParamOffset, hPool, hPage, paramOffset, length);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = RPOOL_CopyFromExternal(hPool, hPage, *newParamOffset+length, "%40", 3);
    if(rv != RV_OK)
    {
        return rv;
    }

    oldLength = length;
    length = RPOOL_Strlen(hPool, hPage, newOffset);

    rv = RPOOL_CopyFrom(hPool, hPage, *newParamOffset + oldLength + 3, hPool, hPage, newOffset, length);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = RPOOL_CopyFromExternal(hPool, hPage, *newParamOffset+length+3+oldLength, "\0", 1);
    return rv;
}

/***************************************************************************
 * ReplacesHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ReplacesHeaderClean(IN MsgReplacesHeader* pHeader,
                                IN RvBool            bCleanBS)
{
    pHeader->eHeaderType    = RVSIP_HEADERTYPE_REPLACES;
    pHeader->strCallID        = UNDEFINED;
    pHeader->strFromTag        = UNDEFINED;
    pHeader->strToTag        = UNDEFINED;
    pHeader->strOtherParams    = UNDEFINED;
    pHeader->eEarlyFlagParamType     = RVSIP_REPLACES_EARLY_FLAG_TYPE_UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pFromTag        = NULL;
    pHeader->pToTag            = NULL;
    pHeader->pCallID        = NULL;
    pHeader->pOtherParams    = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }

}
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif
