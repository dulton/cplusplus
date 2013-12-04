/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipContentIDHeader.c                                 *
 *                                                                            *
 * The file defines the methods of the Content-ID header object               *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Mickey Farkash    Jan 2007                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipContentIDHeader.h"
#include "_SipContentIDHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipBody.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ContentIDHeaderClean(IN MsgContentIDHeader* pHeader,
							     IN RvBool              bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/
/*@*****************************************************************************
 * RvSipContentIDHeaderConstructInBody
 * ------------------------------------------------------------------------
 * General: Constructs a Content-ID header object inside a given message
 *          body object.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hBody    - The handle to the message body object.
 * Output: phHeader - The handle to the newly constructed Content-ID header
 *                    object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderConstructInBody(
                               IN  RvSipBodyHandle               hBody,
                               OUT RvSipContentIDHeaderHandle* phHeader)

{

    Body*       pBody;
    RvStatus    stat;

    if(hBody == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(phHeader == NULL)
        return RV_ERROR_NULLPTR;

    *phHeader = NULL;

    pBody = (Body*)hBody;

    stat = RvSipContentIDHeaderConstruct((RvSipMsgMgrHandle)pBody->pMsgMgr,
                                            pBody->hPool, pBody->hPage, phHeader);
    if(stat != RV_OK)
        return stat;

    /* attach the header in the msg */
    pBody->pContentID = (MsgContentIDHeader *)*phHeader;

    return RV_OK;
}

/*@*****************************************************************************
 * RvSipContentIDHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Content-ID Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - The handle to the message manager.
 *         hPool    - The handle to the memory pool that the object will use.
 *         hPage    - The handle to the memory page that the object will use.
 * output: phHeader - The handle to the newly constructed Content-ID header
 *                   object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderConstruct(
                                    IN  RvSipMsgMgrHandle             hMsgMgr,
                                    IN  HRPOOL                        hPool,
                                    IN  HPAGE                         hPage,
                                    OUT RvSipContentIDHeaderHandle* phHeader)
{
    MsgContentIDHeader*  pHeader;
    MsgMgr*                pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (NULL == phHeader) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    *phHeader = NULL;

    /* allocate the header */
    pHeader = (MsgContentIDHeader*)SipMsgUtilsAllocAlign(
                                                  pMsgMgr,
                                                  hPool,
                                                  hPage,
                                                  sizeof(MsgContentIDHeader),
                                                  RV_TRUE);
    if(pHeader == NULL)
    {
        *phHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipContentIDHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr         = pMsgMgr;
    pHeader->hPage           = hPage;
    pHeader->hPool           = hPool;

    ContentIDHeaderClean(pHeader, RV_TRUE);

    *phHeader = (RvSipContentIDHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipContentIDHeaderConstruct - Content-ID header was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, phHeader=0x%p)",
              hMsgMgr, hPool, hPage, *phHeader));


    return RV_OK;
}

/*@*****************************************************************************
 * RvSipContentIDHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Content-ID header object to
 *          a destination Content-ID header object.
 *          You must construct the destination object before using this
 *          function.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - The handle to the destination Content-ID header object.
 *    hSource      - The handle to the source Content-ID header object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderCopy(
                             INOUT RvSipContentIDHeaderHandle hDestination,
                             IN    RvSipContentIDHeaderHandle hSource)
{
    MsgContentIDHeader* source;
    MsgContentIDHeader* dest;
	RvStatus            stat;
	

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgContentIDHeader*)hSource;
    dest   = (MsgContentIDHeader*)hDestination;

    /* AddrSpec */
    stat = RvSipContentIDHeaderSetAddrSpec(hDestination, source->hAddrSpec);
	if (RV_OK != stat)
	{
		RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
			   "RvSipContentIDHeaderCopy - Failed in RvSipContentIDHeaderSetAddrSpec. stat=%d", stat));
        return stat;
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
                "RvSipContentIDHeaderCopy - failed in coping strBadSyntax."));
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

/*@*****************************************************************************
 * RvSipContentIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Content-ID header object to a textual Content-ID
 *          header. The textual header is placed on a page taken from a
 *          specified pool. In order to copy the textual header from the
 *          page to a consecutive buffer, use RPOOL_CopyToExternal().
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - The handle to the Content-ID header object.
 *        hPool    - The handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderEncode(
                                  IN   RvSipContentIDHeaderHandle hHeader,
                                  IN   HRPOOL                       hPool,
                                  OUT  HPAGE*                       hPage,
                                  OUT  RvUint32*                   length)
{
    RvStatus stat;
    MsgContentIDHeader* pHeader;

    pHeader = (MsgContentIDHeader*)hHeader;
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
                  "RvSipContentIDHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                  hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentIDHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                  *hPage, hPool, hHeader));
    }
    stat = ContentIDHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContentIDHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ContentIDHeaderEncode fail",
                  *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * ContentIDHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-ID header (as string) on it.
 *          Content-ID = "Content-ID:" <addr-spec>
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-ID header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentIDHeaderEncode(
                                  IN    RvSipContentIDHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bInUrlHeaders,
                                  INOUT RvUint32*                    length)
{
    MsgContentIDHeader*   pHeader;
    RvStatus               stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgContentIDHeader*)hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
              "ContentIDHeaderEncode - Encoding Content-ID header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
              hHeader, hPool, hPage));

    /* set "Content-ID" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Content-ID", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContentIDHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
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
                "ContentIDHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    if (pHeader->hAddrSpec != NULL)
    {
        /* Set the adder-spec */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
            return stat;
    }
    else
    {
        /* This is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContentIDHeaderEncode: Failed. No hAddrSpec was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "ContentIDHeaderEncode: Failed to encode Content-ID header. stat is %d",
                  stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipContentIDHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual ContentID header - for example:
 *         "Content-ID: <sip:alice@wonderland.com>"
 *         - into a Content-ID header object. All the textual fields are
 *         placed inside the object.
 *          Note: Before performing the parse operation the stack
 *          resets all the header fields. Therefore, if the parse
 *          function fails, you will not be able to get the previous
 *          header field values.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Content-ID header object.
 *    buffer    - Buffer containing a textual Content-ID header.
 * Return Value: Returns RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderParse(
                                 IN    RvSipContentIDHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer)
{
    MsgContentIDHeader* pHeader = (MsgContentIDHeader*)hHeader;
    RvStatus             rv;
     if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContentIDHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_ID,
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

/*@*****************************************************************************
 * RvSipContentIDHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Content-ID header value into an ContentID
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContentIDHeaderParse() function to parse
 *          strings that also include the header-name.
 *          Note: Before performing the parse operation the stack
 *          resets all the header fields. Therefore, if the parse
 *          function fails, you will not be able to get the previous
 *          header field values.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Content-ID header object.
 *    buffer    - The buffer containing a textual Content-ID header value.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderParseValue(
                                 IN    RvSipContentIDHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer)
{
    MsgContentIDHeader* pHeader = (MsgContentIDHeader*)hHeader;
    RvStatus            rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContentIDHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTENT_ID,
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

/*@*****************************************************************************
 * RvSipContentIDHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Content-ID header with bad-syntax information.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as it was.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderFix(
                                     IN RvSipContentIDHeaderHandle hHeader,
                                     IN RvChar*                     pFixedBuffer)
{
    MsgContentIDHeader* pHeader = (MsgContentIDHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipContentIDHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentIDHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContentIDHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipContentIDHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-ID header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentID - The Content-ID header to take the page from.
 ***************************************************************************/
HRPOOL SipContentIDHeaderGetPool(RvSipContentIDHeaderHandle hContentID)
{
    return ((MsgContentIDHeader*)hContentID)->hPool;
}

/***************************************************************************
 * SipContentIDHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-ID header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentID - The Content-ID header to take the page from.
 ***************************************************************************/
HPAGE SipContentIDHeaderGetPage(RvSipContentIDHeaderHandle hContentID)
{
    return ((MsgContentIDHeader*)hContentID)->hPage;
}

/***************************************************************************
 * RvSipContentIDHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Content-ID header fields are kept in a string format
 *          - for example, the Content-ID header parameter bad syntax. In order
 *          to get such a field from the Content-ID header object, your
 *          application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader    - The handle to the Content-ID header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 * Return Value: Returns the length of the specified string.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipContentIDHeaderGetStringLength(
                             IN  RvSipContentIDHeaderHandle     hHeader,
                             IN  RvSipContentIDHeaderStringName stringName)
{
    RvInt32 stringOffset;
    MsgContentIDHeader* pHeader = (MsgContentIDHeader*)hHeader;

    if(pHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_CONTENT_ID_BAD_SYNTAX:
        {
            stringOffset = SipContentIDHeaderGetStrBadSyntax(hHeader);
            break;
        }
		
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "RvSipContentIDHeaderGetStringLength -Unknown stringName %d", stringName));
            return 0;
        }
    }
    if (stringOffset > UNDEFINED)
        return (RPOOL_Strlen(SipContentIDHeaderGetPool(hHeader),
                             SipContentIDHeaderGetPage(hHeader),
                             stringOffset) + 1);
    else
        return 0;
}

/*@*****************************************************************************
 * RvSipContentIDHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the Content-ID header object as an Address object.
 *          This function returns the handle to the address object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - The handle to the Content-ID header object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 ****************************************************************************@*/
RVAPI RvSipAddressHandle RVCALLCONV RvSipContentIDHeaderGetAddrSpec(
                                    IN RvSipContentIDHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgContentIDHeader*)hHeader)->hAddrSpec;
}

/*@*****************************************************************************
 * RvSipContentIDHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the Content-ID header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Content-ID header object.
 *    hAddrSpec - The handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the Content-ID header.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderSetAddrSpec(
                                        IN    RvSipContentIDHeaderHandle hHeader,
                                        IN    RvSipAddressHandle       hAddrSpec)
{
    RvStatus             stat;
    RvSipAddressType     currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress          *pAddr           = (MsgAddress*)hAddrSpec;
    MsgContentIDHeader  *pHeader         = (MsgContentIDHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hAddrSpec == NULL)
    {
        pHeader->hAddrSpec = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
               return RV_ERROR_BADPARAM;
        }
        if (NULL != pHeader->hAddrSpec)
        {
            currentAddrType = RvSipAddrGetAddrType(pHeader->hAddrSpec);
        }

        /* If no address object was allocated, we will construct it */
        if((pHeader->hAddrSpec == NULL) ||
           (currentAddrType != pAddr->eAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInContentIDHeader(hHeader,
                                                       pAddr->eAddrType,
                                                       &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }
        
        stat = RvSipAddrCopy(pHeader->hAddrSpec, hAddrSpec);
        
        return stat;
    }
}

/***************************************************************************
* SipContentIDHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
* hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipContentIDHeaderGetStrBadSyntax(
                                     IN RvSipContentIDHeaderHandle hHeader)
{
    return ((MsgContentIDHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipContentIDHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ContentID header,
 *          and you wish to see the header-value.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer with which to fill the bad syntax string.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the bad syntax + 1, to include
 *                     a NULL value at the end of the parameter.
 * Return Value: Returns RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderGetStrBadSyntax(
                                    IN RvSipContentIDHeaderHandle   hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgContentIDHeader*)hHeader)->hPool,
                                  ((MsgContentIDHeader*)hHeader)->hPage,
                                  SipContentIDHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipContentIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipContentIDHeaderSetStrBadSyntax(
                                  IN RvSipContentIDHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgContentIDHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    header = (MsgContentIDHeader*)hHeader;

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
 * RvSipContentIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Content-ID header.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 * Return Value: Returns RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderSetStrBadSyntax(
                                  IN RvSipContentIDHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipContentIDHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ContentIDHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ContentIDHeaderClean(IN MsgContentIDHeader* pHeader,
								 IN RvBool              bCleanBS)
{

    pHeader->hAddrSpec             = NULL;

    if(bCleanBS == RV_TRUE)
    {
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


