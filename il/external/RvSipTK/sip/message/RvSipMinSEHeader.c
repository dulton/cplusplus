/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipMinSEHeader.c                             *
 *                                                                            *
 * The file defines the methods of the Min SE header object.                  *
 * The Min SE header functions enable you to construct, copy, encode, parse,  *
 * access and change Min-SE Header parameters.                                *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Michal Mashiach    June 2001                                           *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipMinSEHeader.h"
#include "_SipMinSEHeader.h"
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
static void MinSEHeaderClean( IN MsgMinSEHeader* pHeader,
                              IN RvBool         bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipMinSEHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Min SE header object inside a given message object.
 *          The header is kept in the header list of the message. You can
 *          choose to insert the header either at the head or tail of the list.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header
 *                            should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: phHeader - Handle to the newly constructed Min SE header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderConstructInMsg(
                        IN  RvSipMsgHandle          hSipMsg,
                        IN  RvBool                 pushHeaderAtHead,
                        OUT RvSipMinSEHeaderHandle* phHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;
    
    msg = (MsgMessage*)hSipMsg;

    stat = RvSipMinSEHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
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
                              RVSIP_HEADERTYPE_MINSE,
                              &hListElem,
                              (void**)phHeader);
}



/***************************************************************************
 * RvSipMinSEHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Min SE Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: phHeader - Handle to the newly constructed Min SE  header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderConstruct(
                               IN  RvSipMsgMgrHandle                hMsgMgr,
                               IN  HRPOOL                           hPool,
                               IN  HPAGE                            hPage,
                               OUT RvSipMinSEHeaderHandle* phHeader)
{
    MsgMinSEHeader*   pHeader;
    MsgMgr*           pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if (phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pHeader = (MsgMinSEHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgMinSEHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipMinSEHeaderConstruct - Failed to construct Min-SE header. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    MinSEHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipMinSEHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipMinSEHeaderConstruct - Min SE header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *phHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipMinSEHeaderCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Min SE header object to a
 *         destination Min SE header object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Handle to the destination Min SE header object.
 *    pSource      - Handle to the source Min SE header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderCopy(
                                     INOUT RvSipMinSEHeaderHandle hDestination,
                                     IN    RvSipMinSEHeaderHandle hSource)
{
    MsgMinSEHeader*   source;
    MsgMinSEHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgMinSEHeader*)hSource;
    dest   = (MsgMinSEHeader*)hDestination;

    dest->eHeaderType  = source->eHeaderType;
    dest->deltaSeconds = source->deltaSeconds;
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
                "RvSipMinSEHeaderCopy - didn't manage to copy MinSEParams"));
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
                "RvSipMinSEHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipMinSEHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Min SE header object to a textual Min SE header.
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
 * input: hHeader  - Handle to the Min SE header object.
 *        hPool    - Handle to the specified memory pool.
 * output: phPage   - The memory page allocated to contain the encoded header.
 *         pLength  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderEncode(
                              IN    RvSipMinSEHeaderHandle hHeader,
                              IN    HRPOOL                 hPool,
                              OUT   HPAGE*                 phPage,
                              OUT   RvUint32*             pLength)
{
    RvStatus stat;
    MsgMinSEHeader* pHeader = (MsgMinSEHeader*)hHeader;

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
                  "RvSipMinSEHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipMinSEHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *phPage, hPool, hHeader));
    }

    *pLength = 0; /* This way length will give the real length, and will not
                     just add the new length to the old one */
    stat = MinSEHeaderEncode(hHeader, hPool, *phPage, RV_FALSE, pLength);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *phPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipMinSEHeaderEncode - Failed. Free page 0x%p on pool 0x%p. MinSEHeaderEncode fail",
                  *phPage, hPool));
    }
    return stat;
}


/***************************************************************************
 * MinSEHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Min SE header (as string) on it.
 *          format: "Min SE: "
 *                  delta-seconds
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Min SE header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV MinSEHeaderEncode(
                        IN    RvSipMinSEHeaderHandle hHeader,
                        IN    HRPOOL                 hPool,
                        IN    HPAGE                  hPage,
                        IN    RvBool                bInUrlHeaders,
                        INOUT RvUint32*             pLength)
{
    MsgMinSEHeader*  pHeader;
    RvStatus          stat = RV_OK;
    RvChar            strHelper[16];

   if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    pHeader = (MsgMinSEHeader*) hHeader;

     RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "MinSEHeaderEncode - Encoding Min SE header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
               hHeader, hPool, hPage));

    /* put "Min-SE" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Min-SE", pLength);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "MinSEHeaderEncode - Failed to encoding Min-SE header. stat is %d",
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
                "MinSEHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    MsgUtils_itoa(pHeader->deltaSeconds , strHelper);

    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, pLength);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "MinSEHeaderEncode - Failed to encode Min SE object. stat is %d",
            stat));
        return stat;
    }

    /* set the  params. (insert ";" in the begining) */
    if(pHeader->strParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                        MsgUtilsGetSemiColonChar(bInUrlHeaders), pLength);

        /* set Parmas */
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
            "MinSEHeaderEncode - Failed to encoding MinSE header. stat is %d",
            stat));
    }
    return stat;

}

/***************************************************************************
 * RvSipMinSEHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Min SE header-for example,
 *         "Min-SE: 3600 " - into a Min SE header
 *          object. All the textual fields are placed inside the object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_INVALID_HANDLE,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   -    A handle to the Min SE header object.
 *    strBuffer    - Buffer containing a textual Min SE header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderParse(
                                     IN    RvSipMinSEHeaderHandle hHeader,
                                     IN    RvChar*               strBuffer)
{
    MsgMinSEHeader* pHeader = (MsgMinSEHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (strBuffer == NULL)
        return RV_ERROR_BADPARAM;

    MinSEHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_MINSE,
                                        strBuffer,
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
 * RvSipMinSEHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual MinSE header value into an MinSE header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipMinSEHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the MinSE header object.
 *    buffer    - The buffer containing a textual MinSE header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderParseValue(
                                     IN    RvSipMinSEHeaderHandle hHeader,
                                     IN    RvChar*               strBuffer)
{
    MsgMinSEHeader* pHeader = (MsgMinSEHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (strBuffer == NULL)
        return RV_ERROR_BADPARAM;

    MinSEHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_MINSE,
                                        strBuffer,
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
 * RvSipMinSEHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an MinSE header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderFix(
                                     IN RvSipMinSEHeaderHandle hHeader,
                                     IN RvChar*               pFixedBuffer)
{
    MsgMinSEHeader* pHeader = (MsgMinSEHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipMinSEHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipMinSEHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipMinSEHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipMinSEHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Min SE object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipMinSEHeaderGetPool(RvSipMinSEHeaderHandle hHeader)
{
    return ((MsgMinSEHeader *)hHeader)->hPool;
}

/***************************************************************************
 * SipMinSEHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Min SE object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipMinSEHeaderGetPage(RvSipMinSEHeaderHandle hHeader)
{
    return ((MsgMinSEHeader *)hHeader)->hPage;
}


/***************************************************************************
 * RvSipMinSEHeaderGetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Gets the delta-seconds integer of the Min SE header.
 *          If the delta-seconds integer is not set, UNDEFINED is returned.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - hHeader is NULL.
 *               RV_ERROR_NULLPTR - pDeltaSeconds is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader - Handle to the Min SE header object.
 *  Output: pDeltaSeconds - The delta-seconds integer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderGetDeltaSeconds(
                    IN  RvSipMinSEHeaderHandle hHeader,
                    OUT RvInt32              *pDeltaSeconds)
{
    MsgMinSEHeader *pHeader;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDeltaSeconds)
    {
        return RV_ERROR_NULLPTR;
    }
    pHeader = (MsgMinSEHeader *)hHeader;
    *pDeltaSeconds = pHeader->deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * RvSipMinSEHeaderSetDeltaSeconds
 * ------------------------------------------------------------------------
 * General: Sets the delta seconds integer of the Min SE header.
 *          If the given delta-seconds is UNDEFINED, the delta-seconds of
 *          the Min SE header is removed
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Min SE header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Min SE header object.
 *         deltaSeconds - The delta-seconds to be set to the Min SE header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderSetDeltaSeconds(
                        IN  RvSipMinSEHeaderHandle hHeader,
                        IN  RvInt32              deltaSeconds)
{
    MsgMinSEHeader *pMinSE;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pMinSE = (MsgMinSEHeader *)hHeader;

    pMinSE->deltaSeconds = deltaSeconds;
    return RV_OK;
}


/***************************************************************************
 * SipMinSEHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipMinSEHeaderGetOtherParams(
                                     IN RvSipMinSEHeaderHandle hHeader)
{
    return ((MsgMinSEHeader*)hHeader)->strParams;
}

/***************************************************************************
 * RvSipMinSEHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other parameters string from the MinSE header
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
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderGetOtherParams(
                                    IN RvSipMinSEHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgMinSEHeader*)hHeader)->hPool,
                                  ((MsgMinSEHeader*)hHeader)->hPage,
                                  SipMinSEHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipMinSEHeaderSetOtherParams
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
RvStatus SipMinSEHeaderSetOtherParams(
                             IN    RvSipMinSEHeaderHandle hHeader,
                             IN    RvChar *                    strOtherParams,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             IN    RvInt32                     strOffset)
{
    RvInt32              newStr;
    MsgMinSEHeader* pHeader;
    RvStatus             retStatus;

    if (hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgMinSEHeader*)hHeader;

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
 * RvSipMinSEHeaderSetOtherParams
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
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderSetOtherParams(
                                 IN   RvSipMinSEHeaderHandle  hHeader,
                                 IN   RvChar                      *otherParams)
{
    /* validity checking will be done in the internal function */
    return SipMinSEHeaderSetOtherParams(hHeader, otherParams, NULL,
                                              NULL_PAGE, UNDEFINED);
}
/***************************************************************************
 * RvSipMinSEHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: The other params of MinSE header fields are kept in a string
 *          format.
 *          In order to get such a field from the MinSE header
 *          object, your application should supply an adequate buffer to where
 *          the string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the MinSE header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipMinSEHeaderGetStringLength(
                      IN  RvSipMinSEHeaderHandle     hHeader,
                      IN  RvSipMinSEHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgMinSEHeader* pHeader = (MsgMinSEHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_MINSE_OTHER_PARAMS:
        {
            stringOffset = SipMinSEHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_MINSE_BAD_SYNTAX:
        {
            stringOffset = SipMinSEHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RvSipMinSEHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipMinSEHeaderGetPool(hHeader),
                             SipMinSEHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/***************************************************************************
 * SipMinSEHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipMinSEHeaderGetStrBadSyntax(
                                    IN RvSipMinSEHeaderHandle hHeader)
{
    return ((MsgMinSEHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipMinSEHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad MinSE header,
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
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderGetStrBadSyntax(
                                               IN RvSipMinSEHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgMinSEHeader*)hHeader)->hPool,
                                  ((MsgMinSEHeader*)hHeader)->hPage,
                                  SipMinSEHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipMinSEHeaderSetStrBadSyntax
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
RvStatus SipMinSEHeaderSetStrBadSyntax(
                                  IN RvSipMinSEHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgMinSEHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgMinSEHeader*)hHeader;

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
 * RvSipMinSEHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal MinSE header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMinSEHeaderSetStrBadSyntax(
                                  IN RvSipMinSEHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipMinSEHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * MinSEHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void MinSEHeaderClean( IN MsgMinSEHeader* pHeader,
                              IN RvBool               bCleanBS)
{
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_MINSE;
    pHeader->deltaSeconds     = 0;
    pHeader->strParams        = UNDEFINED;

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}
#endif /* RV_SIP_PRIMITIVES */



