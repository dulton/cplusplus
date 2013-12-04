/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipOtherHeader.h                                     *
 *                                                                            *
 * The file defines the methods of the Other header object:                   *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 * The OtherHeader should be use for every header that has no specific object *
 * (not to use for To, From, Cseq ... that have specific objects)             *
 *                                                                            *
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

#include "RvSipOtherHeader.h"
#include "_SipOtherHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipBodyPart.h"
#include <string.h>


/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/

/***************************************************************************
 * RvSipOtherHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Other header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           IN  RvBool                 pushHeaderAtHead,
                                           OUT RvSipOtherHeaderHandle* hHeader)
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

    stat = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_OTHER,
                              &hListElem,
                              (void**)hHeader);
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipOtherHeaderConstructInBodyPart
 * ------------------------------------------------------------------------
 * General: Constructs a Other header object inside a given message body
 *          part object. The header is kept in the other headers list of
 *          the message body part object. You can choose to insert the
 *          header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBodyPart        - Handle to the message body-part object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the list-
 *                            RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderConstructInBodyPart(
                                    IN  RvSipBodyPartHandle     hBodyPart,
                                    IN  RvBool                 pushHeaderAtHead,
                                    OUT RvSipOtherHeaderHandle* hHeader)
{
    BodyPart              *pBodyPart;
    MsgOtherHeader        *pOtherHeader;
    RvSipHeadersLocation  location;
    RvStatus             stat;

    if(hBodyPart == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the body part in the body */
    stat = BodyPartPushHeader(hBodyPart,
                                 location,
                                 NULL,
                                 hHeader);
    if (RV_OK != stat)
        return stat;

    pBodyPart = (BodyPart *)hBodyPart;
    pOtherHeader = (MsgOtherHeader *)*hHeader;

    pOtherHeader->pMsgMgr          = pBodyPart->pMsgMgr;
    pOtherHeader->hPage            = pBodyPart->hPage;
    pOtherHeader->hPool            = pBodyPart->hPool;
    pOtherHeader->strHeaderName    = UNDEFINED;
    pOtherHeader->strHeaderVal     = UNDEFINED;
    pOtherHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
    pOtherHeader->pHeaderName      = NULL;
    pOtherHeader->pHeaderVal       = NULL;
#endif

    return RV_OK;
}
#endif /* #ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * RvSipOtherHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Other Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderConstruct(
                                           IN  RvSipMsgMgrHandle       hMsgMgr,
                                           IN  HRPOOL                  hPool,
                                           IN  HPAGE                   hPage,
                                           OUT RvSipOtherHeaderHandle* hHeader)
{
    MsgOtherHeader*   pHeader;
    MsgMgr*           pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgOtherHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                     hPool,
                                                     hPage,
                                                     sizeof(MsgOtherHeader),
                                                     RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipOtherHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p, hHeader 0x%p",
            hPool, hPage, hHeader));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr       = pMsgMgr;
    pHeader->strHeaderName = UNDEFINED;
    pHeader->strHeaderVal  = UNDEFINED;
    pHeader->hPage         = hPage;
    pHeader->hPool         = hPool;
    pHeader->eHeaderType   = RVSIP_HEADERTYPE_OTHER;
    pHeader->strBadSyntax  = UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pHeaderName = NULL;
    pHeader->pHeaderVal  = NULL;
    pHeader->pBadSyntax  = NULL;
#endif

    *hHeader = (RvSipOtherHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipOtherHeaderConstruct - Other header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipOtherHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Other header object to a destination Other header
 *          object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Other header object.
 *    hSource      - Handle to the source Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderCopy(
                                         INOUT RvSipOtherHeaderHandle hDestination,
                                         IN    RvSipOtherHeaderHandle hSource)
{
    MsgOtherHeader* source;
    MsgOtherHeader* dest;

    if ((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgOtherHeader*)hSource;
    dest   = (MsgOtherHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* headerName */
    if(source->strHeaderName > UNDEFINED)
    {
        dest->strHeaderName = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strHeaderName);
        if (dest->strHeaderName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderCopy: Failed to copy headerName. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pHeaderName = (RvChar*)RPOOL_GetPtr(dest->hPool,
                                                  dest->hPage,
                                                  dest->strHeaderName);
#endif
    }
    else
    {
        dest->strHeaderName = UNDEFINED;
#ifdef SIP_DEBUG
    dest->pHeaderName = NULL;
#endif
    }

    /* headerVal */
    /*stat = RvSipOtherHeaderSetValue(hDestination, source->strHeaderVal);*/
    if(source->strHeaderVal > UNDEFINED)
    {
        dest->strHeaderVal = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                          dest->hPool,
                                                          dest->hPage,
                                                          source->hPool,
                                                          source->hPage,
                                                          source->strHeaderVal);
        if (dest->strHeaderVal == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderCopy - Failed to copy headerVal. hDest 0x%p",
                hDestination));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pHeaderVal = (RvChar*)RPOOL_GetPtr(dest->hPool,
                                                  dest->hPage,
                                                  dest->strHeaderVal);
#endif
    }
    else
    {
        dest->strHeaderVal = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pHeaderVal = NULL;
#endif
    }

    /* bad-syntax string */
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
                "RvSipOtherHeaderCopy - Failed to copy BadSyntax string"));
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
 * RvSipOtherHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a contact header object to a textual Other header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Other header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderEncode(
                                          IN    RvSipOtherHeaderHandle hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgOtherHeader* pHeader = (MsgOtherHeader*)hHeader;

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
                "RvSipOtherHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = OtherHeaderEncode(hHeader, hPool, *hPage, RV_TRUE, RV_FALSE, RV_FALSE,length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderEncode - Faield. Free page 0x%p on pool 0x%p. OtherHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * OtherHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            other header (as string) on it.
 *          message-header = headerName ":" [header-value]
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no header were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the other header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstInMessage - RV_TRUE if the header encoded is the first header encoded from
 *                            all the other headers with the same name.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - if RV_TRUE, will force compact form to Supported headers only.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV OtherHeaderEncode(
                              IN    RvSipOtherHeaderHandle hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                 bFirstInMessage,
                              IN    RvBool                 bInUrlHeaders,
                              IN    RvBool                 bForceCompactForm,
                              INOUT RvUint32*              length)
{
    MsgOtherHeader* pHeader;
    RvStatus         stat = RV_OK;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgOtherHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "OtherHeaderEncode - Encoding Other header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));


    /* set headerName */
    if(pHeader->strHeaderName > UNDEFINED)
    {
        if(bFirstInMessage == RV_TRUE)
        {
            if(bForceCompactForm == RV_TRUE &&
               RPOOL_CmpToExternal(pHeader->hPool,
                                   pHeader->hPage,
                                   pHeader->strHeaderName,
                                   "Supported",RV_TRUE) == RV_TRUE)
            {
                stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                                    "k",
                                                    length);
            }
            else
            {
                stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                            hPool,
                                            hPage,
                                            pHeader->hPool,
                                            pHeader->hPage,
                                            pHeader->strHeaderName,
                                            length);
            }
            if (stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "OtherHeaderEncode - Failed to encode header name. stat: %d",stat));
                return stat;
            }
            /* put ': ' or '=' in the buffer */
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                                MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                                length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr,
                                                hPool,
                                                hPage,
                                                ",",
                                                length);
        }
    }

    /* bad syntax */
    if(pHeader->strBadSyntax > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBadSyntax,
                                    length);
    }
    else if(pHeader->strHeaderVal > UNDEFINED) /* set header value */
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strHeaderVal,
                                    length);
    }

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "OtherHeaderEncode - Failed to encode Other header. stat: %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipOtherHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Other header-for example, "Content-Disposition:
 *         session"-into a Other header object. All the textual fields are placed inside the
 *         object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Other header object.
 *    buffer    - Buffer containing a textual Other header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderParse(
                                     IN    RvSipOtherHeaderHandle   hHeader,
                                     IN    RvChar*                 buffer)
{
          RvStatus   stat;
          RvInt32    index;
          RvInt32    tempIndex;
    const RvChar     SP = 0x20;
    const RvChar     HT = 0x09;
          RPOOL_ITEM_HANDLE encoded;
          RvInt32    nameOffset;
          RvInt32    offset;
          MsgOtherHeader* pHeader = (MsgOtherHeader*)hHeader;

    if(pHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (NULL == buffer)
        return RV_ERROR_NULLPTR;

    /* skip white spaces and tubs */
    while ((*buffer != '\0') &&
           ((*buffer == SP) ||
            (*buffer == HT)))
    {
        buffer++;
    }
    /* find ":" */
    index = 0;
    while ((buffer[index] != '\0') &&
           (buffer[index] != ':'))
    {
        index++;
    }
    if ('\0' == buffer[index])
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderParse - Failed to parse other header. missing :"));
        return RV_ERROR_UNKNOWN;
    }
    /* delete following white spaces and tubs */
    tempIndex = index - 1;
    while ((tempIndex >= 0) &&
           ((buffer[tempIndex] == SP) ||
            (buffer[tempIndex] == HT)))
    {
        tempIndex--;
    }
    tempIndex++;
    if (0 == tempIndex)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderParse - Failed to parse other header. missing header name"));
        return RV_ERROR_UNKNOWN;
    }
    /* appending the header name (+'\0') to the page of the header and then
       set it to the header */
    stat = RPOOL_AppendFromExternal(SipOtherHeaderGetPool(hHeader),
                                    SipOtherHeaderGetPage(hHeader),
                                    (void*)buffer,
                                    tempIndex,
                                    RV_FALSE,
                                    &nameOffset,
                                    &encoded);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipOtherHeaderParse - Failed, AppendFromExternal failed."));
    }
    stat = RPOOL_AppendFromExternal(SipOtherHeaderGetPool(hHeader),
                                    SipOtherHeaderGetPage(hHeader),
                                    (void*)"\0",
                                    1,
                                    RV_FALSE,
                                    &offset,
                                    &encoded);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipOtherHeaderParse - Failed, AppendFromExternal failed."));
    }
    stat = SipOtherHeaderSetName(hHeader,
                                 NULL,
                                 SipOtherHeaderGetPool(hHeader),
                                 SipOtherHeaderGetPage(hHeader),
                                 nameOffset);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderParse - Failed to set other header name"));
        return stat;
    }
    /* skip the ":" */
    index++;
    /* Skip preceeding white spaces and tubs */
    while ((buffer[index] != '\0') &&
           ((buffer[index] == SP) ||
            (buffer[index] == HT)))
    {
        index++;
    }
    /* Set header value. The header value is allowed to be "" */
    stat = RvSipOtherHeaderSetValue(hHeader, buffer+index);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderParse - Failed to set other header value"));
        return stat;
    }
    return RV_OK;
}

/***************************************************************************
 * SipOtherHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Other object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipOtherHeaderGetPool(RvSipOtherHeaderHandle hHeader)
{
    return ((MsgOtherHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipOtherHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Other object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipOtherHeaderGetPage(RvSipOtherHeaderHandle hHeader)
{
    return ((MsgOtherHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipOtherHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Other header fields are kept in a string format-for example, the
 *          Other header name. In order to get such a field from the Other header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to
 *          allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Other header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipOtherHeaderGetStringLength(
                                      IN  RvSipOtherHeaderHandle     hHeader,
                                      IN  RvSipOtherHeaderStringName stringName)
{
    RvInt32    string;
    MsgOtherHeader* pHeader = (MsgOtherHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_OTHER_HEADER_NAME:
        {
            string = SipOtherHeaderGetName(hHeader);
            break;
        }
        case RVSIP_OTHER_HEADER_VALUE:
        {
            string = SipOtherHeaderGetValue(hHeader);
            break;
        }
        case RVSIP_OTHER_BAD_SYNTAX:
        {
            string = SipOtherHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(string > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, string)+1);
    else
        return 0;
}

/***************************************************************************
 * SipOtherHeaderGetName
 * ------------------------------------------------------------------------
 * General: This method retreives the name of the header.
 * Return Value: offset of the name string, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Other header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetName(IN RvSipOtherHeaderHandle hHeader)
{
    return ((MsgOtherHeader*)hHeader)->strHeaderName;
}

/***************************************************************************
 * RvSipOtherHeaderGetName
 * ------------------------------------------------------------------------
 * General: Copies the name of the Other header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderGetName(
                                               IN RvSipOtherHeaderHandle   hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgOtherHeader*)hHeader)->hPool,
                                  ((MsgOtherHeader*)hHeader)->hPage,
                                  SipOtherHeaderGetName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipOtherHeaderSetName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pName in the
 *          OtherHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Other header object.
 *            pName   - The name of the header to be set in the Other header.
 *                    if NULL - the exist name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          NameOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipOtherHeaderSetName(IN    RvSipOtherHeaderHandle hHeader,
                                IN    RvChar*               pName,
                                IN    HRPOOL                 hPool,
                                IN    HPAGE                  hPage,
                                IN    RvInt32               NameOffset)
{
    RvInt32              newStr;
    MsgOtherHeader*       pHeader;
    RvStatus             retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgOtherHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, NameOffset, pName,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strHeaderName = newStr;

#ifdef SIP_DEBUG
        pHeader->pHeaderName = (RvChar*)RPOOL_GetPtr(pHeader->hPool,
                                                      pHeader->hPage,
                                                      pHeader->strHeaderName);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipOtherHeaderSetName
 * ------------------------------------------------------------------------
 * General: Sets the name of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle of the Other header object.
 *    strName   - The name of the header to be set in the Other header. If NULL is supplied, the
 *              existing header name is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderSetName
                                        (IN    RvSipOtherHeaderHandle hHeader,
                                         IN    RvChar*               pName)
{
    /* the validity checking will be done inside the internal function */
    return SipOtherHeaderSetName(hHeader, pName, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipOtherHeaderGetValue
 * ------------------------------------------------------------------------
 * General: This method gets the value of the header.
 * Return Value: offset of the value string, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Other header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetValue(IN RvSipOtherHeaderHandle hHeader)
{
    return ((MsgOtherHeader*)hHeader)->strHeaderVal;
}

/***************************************************************************
 * RvSipOtherHeaderGetValue
 * ------------------------------------------------------------------------
 * General: Copies the value of the Other header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle of the Other header object.
 *        strBuffer  - buffer to fill with the requested parameter.
 *        bufferLen  - the length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderGetValue(
                                               IN RvSipOtherHeaderHandle   hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgOtherHeader*)hHeader)->hPool,
                                  ((MsgOtherHeader*)hHeader)->hPage,
                                  SipOtherHeaderGetValue(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipOtherHeaderSetValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pValue in the
 *          OtherHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Other header object.
 *            pValue  - pointer to the value of the header to be set in the Other header.
 *                    if NULL - the exist name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          valueOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipOtherHeaderSetValue(IN    RvSipOtherHeaderHandle hHeader,
                                 IN    RvChar*               pValue,
                                 IN    HRPOOL                 hPool,
                                 IN    HPAGE                  hPage,
                                 IN    RvInt32               valueOffset)
{
    RvInt32              newStr;
    MsgOtherHeader*       pHeader;
    RvStatus             retStatus;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pHeader = (MsgOtherHeader*)hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, valueOffset, pValue,
                                  pHeader->hPool, pHeader->hPage,
                                  &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strHeaderVal = newStr;

#ifdef SIP_DEBUG
        pHeader->pHeaderVal = (RvChar*)RPOOL_GetPtr(pHeader->hPool,
                                                     pHeader->hPage,
                                                     pHeader->strHeaderVal);
#endif

    return RV_OK;
}
/***************************************************************************
 * RvSipOtherHeaderSetValue
 * ------------------------------------------------------------------------
 * General: Sets the value of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle of the Other header object.
 *    strValue - The header value to be set in the Other header. If NULL is supplied, the existing
 *             header value is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderSetValue
                                            (IN    RvSipOtherHeaderHandle hHeader,
                                             IN    RvChar*               strValue)
{
    /* The validity checking will be done in the internal function. */
    return SipOtherHeaderSetValue(hHeader, strValue, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * SipOtherHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetStrBadSyntax(
                                    IN RvSipOtherHeaderHandle hHeader)
{
    return ((MsgOtherHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipOtherHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Other header,
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
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderGetStrBadSyntax(
                                               IN RvSipOtherHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgOtherHeader*)hHeader)->hPool,
                                  ((MsgOtherHeader*)hHeader)->hPage,
                                  SipOtherHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipOtherHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Other-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipOtherHeaderSetStrBadSyntax(
                                  IN RvSipOtherHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgOtherHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgOtherHeader*)hHeader;

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
 * RvSipOtherHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Other header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderSetStrBadSyntax(
                                  IN RvSipOtherHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipOtherHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipOtherHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Other header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipOtherHeader - Handle to the Other header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderGetRpoolString(
                             IN    RvSipOtherHeaderHandle      hSipOtherHeader,
                             IN    RvSipOtherHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgOtherHeader* pHeader = (MsgOtherHeader*)hSipOtherHeader;
    RvInt32        requestedParamOffset;

    if(hSipOtherHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_OTHER_HEADER_NAME:
        requestedParamOffset = pHeader->strHeaderName;
        break;
    case RVSIP_OTHER_HEADER_VALUE:
        requestedParamOffset = pHeader->strHeaderVal;
        break;
    case RVSIP_OTHER_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipOtherHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
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
                  "RvSipOtherHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipOtherHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Other header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipOtherHeader - Handle to the Other header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipOtherHeaderSetRpoolString(
                             IN    RvSipOtherHeaderHandle      hSipOtherHeader,
                             IN    RvSipOtherHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr)
{
    MsgOtherHeader* pHeader = (MsgOtherHeader*)hSipOtherHeader;

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
                 "RvSipOtherHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_OTHER_HEADER_NAME:
        return SipOtherHeaderSetName(hSipOtherHeader,NULL,pRpoolPtr->hPool,
                               pRpoolPtr->hPage,
                               pRpoolPtr->offset);
    case RVSIP_OTHER_HEADER_VALUE:
        return SipOtherHeaderSetValue(hSipOtherHeader,NULL,pRpoolPtr->hPool,
                                      pRpoolPtr->hPage,
                                      pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipOtherHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

#endif /*#ifndef RV_SIP_PRIMITIVES*/







#ifdef __cplusplus
}
#endif
