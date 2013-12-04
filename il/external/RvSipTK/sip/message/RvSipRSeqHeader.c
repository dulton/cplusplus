
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          RvSipRSeqHeader.h                                 *
 *                                                                            *
 * The file defines the methods of the RSeq header object:                    *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Sarit             Aug.2001                                            *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipRSeqHeader.h"
#include "_SipRSeqHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "rvansi.h"
#include <string.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void RSeqHeaderClean( IN MsgRSeqHeader* pHeader,
                             IN RvBool        bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipRSeqHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a RSeq header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the header list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message related to the new header object.
 * output: hHeader - Handle to the RSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           IN  RvBool                 pushHeaderAtHead,
                                           OUT RvSipRSeqHeaderHandle *hHeader)
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

    stat = RvSipRSeqHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);

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
                              RVSIP_HEADERTYPE_RSEQ,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipRSeqHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone RSeq Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed RSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderConstruct(
                                           IN  RvSipMsgMgrHandle      hMsgMgr,
                                           IN  HRPOOL                 hPool,
                                           IN  HPAGE                  hPage,
                                           OUT RvSipRSeqHeaderHandle* hHeader)
{
    MsgRSeqHeader*    pHeader;
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
    pHeader = (MsgRSeqHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgRSeqHeader),
                                                    RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRSeqHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;
    RSeqHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipRSeqHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRSeqHeaderConstruct - RSeq header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipRSeqHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source RSeq header object to a destination RSeq header
 *          object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination RSeq header object.
 *    hSource      - Handle to the source RSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderCopy(
                                       INOUT    RvSipRSeqHeaderHandle hDestination,
                                       IN       RvSipRSeqHeaderHandle hSource)
{
    MsgRSeqHeader* source;
    MsgRSeqHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest   = (MsgRSeqHeader*)hDestination;
    source = (MsgRSeqHeader*)hSource;

    dest->eHeaderType		   = source->eHeaderType;
	dest->respNum.bInitialized = source->respNum.bInitialized; 
	dest->respNum.val		   = source->respNum.val; 
    
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
                "RvSipRSeqHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipRSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a RSeq header object to a textual RSeq header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage(). The
 *          allocated page must be freed only if this function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader  - Handle to the RSeq header object.
 *         hPool    - Handle to the specified memory pool.
 * output: hPage    - The memory page allocated to contain the encoded header.
 *         length   - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderEncode(
                                          IN    RvSipRSeqHeaderHandle  hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgRSeqHeader* pHeader = (MsgRSeqHeader*)hHeader;

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
                "RvSipRSeqHeaderEncode - Failed, RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRSeqHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = RSeqHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRSeqHeaderEncode - Failed. Free page 0x%p on pool 0x%p. RSeqHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * RSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            RSeq header (as string) on it.
 *          RSeq = "RSeq: " 1*DIGIT
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the RSeq header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderEncode(
                              IN    RvSipRSeqHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                 bInUrlHeaders,
                              INOUT RvUint32              *length)
{
    MsgRSeqHeader*  pHeader;
    RvStatus         stat;
    RvChar           strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgRSeqHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RSeqHeaderEncode - Encoding RSeq header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* the header must contain response-num */
    if (pHeader->respNum.bInitialized == RV_FALSE &&
		pHeader->strBadSyntax		  == UNDEFINED)
	{
        return RV_ERROR_BADPARAM;
	}

    /* The RSeq header looks like: "RSeq: 1 INVITE"*/

    /* set "RSeq" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "RSeq", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RSeqHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }

    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
										MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
									length);
    if (stat != RV_OK)
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
                "RSeqHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* step */
	if (RV_FALSE == pHeader->respNum.bInitialized)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RSeqHeaderEncode - Response number is not initialized yet"));
		return RV_ERROR_UNINITIALIZED;
	}
	RvSprintf(strHelper,"%u",pHeader->respNum.val); 
    
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RSeqHeaderEncode - Failed to encode RSeq header. RvStatus is %d",
            stat));
		return stat;
    }
    return stat;
}

/***************************************************************************
 * RvSipRSeqHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual RSeq header-for example, "RSeq: 1 INVITE"-into a
 *         RSeq header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the RSeq header object.
 *    buffer    - Buffer containing a textual RSeq header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderParse(
                                     IN    RvSipRSeqHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgRSeqHeader* pHeader = (MsgRSeqHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RSeqHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RSEQ,
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
 * RvSipRSeqHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual RSeq header value into an RSeq header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipRSeqHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the RSeq header object.
 *    buffer    - The buffer containing a textual RSeq header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderParseValue(
                                     IN    RvSipRSeqHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgRSeqHeader* pHeader = (MsgRSeqHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RSeqHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RSEQ,
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
 * RvSipRSeqHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an RSeq header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderFix(
                                     IN RvSipRSeqHeaderHandle hHeader,
                                     IN RvChar*              pFixedBuffer)
{
    MsgRSeqHeader* pHeader = (MsgRSeqHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipRSeqHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRSeqHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRSeqHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipRSeqHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RSeq object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipRSeqHeaderGetPool(RvSipRSeqHeaderHandle hHeader)
{
    return ((MsgRSeqHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipRSeqHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RSeq object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipRSeqHeaderGetPage(RvSipRSeqHeaderHandle hHeader)
{
    return ((MsgRSeqHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipRSeqHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the RSeq header fields are kept in a string format.
 *          In order to get such a field from the RSeq header object,
 *          your application should supply an adequate buffer to where the
 *          string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the RSeq header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipRSeqHeaderGetStringLength(
                                      IN  RvSipRSeqHeaderHandle     hHeader,
                                      IN  RvSipRSeqHeaderStringName stringName)
{
    RvInt32        stringOffset;
    MsgRSeqHeader *pHeader = (MsgRSeqHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_RSEQ_BAD_SYNTAX:
        {
            stringOffset = SipRSeqHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRSeqHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}

/***************************************************************************
 * RvSipRSeqHeaderIsResponseNumInitialized
 * ------------------------------------------------------------------------
 * General: Gets the response num value from the RSeq header object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the RSeq header object.
 *	  pbInit   - Indication of the resp number definition.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderIsResponseNumInitialized(
                                          IN  RvSipRSeqHeaderHandle hHeader,
										  OUT RvBool			   *pbInit)
{
	MsgRSeqHeader *pHeader = (MsgRSeqHeader*)hHeader; 
    
	if(hHeader == NULL)
	{
		*pbInit = RV_FALSE; 

        return RV_ERROR_INVALID_HANDLE;
	}

    *pbInit = pHeader->respNum.bInitialized;

	return RV_OK;
}

/***************************************************************************
 * RvSipRSeqHeaderGetResponseNum
 * ------------------------------------------------------------------------
 * General: Gets the response num value from the RSeq header object.
 * Return Value: Returns the response number value, or UNDEFINED if the number
 *               is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RSeq header object.
 *    pRespNum - The response number value
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipRSeqHeaderGetResponseNum(
                                          IN  RvSipRSeqHeaderHandle hHeader)
{
	if(hHeader == NULL)
	{
        return 0;
	}

	return ((MsgRSeqHeader*)hHeader)->respNum.val; 
}



/***************************************************************************
 * RvSipRSeqHeaderSetResponseNum
 * ------------------------------------------------------------------------
 * General:Sets the response num value in the RSeq header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RSeq header object.
 *    responseNum    - response num value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderSetResponseNum(
                                          IN    RvSipRSeqHeaderHandle hHeader,
                                          IN    RvUint32              responseNum)
{
    MsgRSeqHeader* pHeader = (MsgRSeqHeader*)hHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader->respNum.val		  = responseNum;
	pHeader->respNum.bInitialized = RV_TRUE;
    
	return RV_OK;
}

/***************************************************************************
 * SipRSeqHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRSeqHeaderGetStrBadSyntax(
                                    IN RvSipRSeqHeaderHandle hHeader)
{
    return ((MsgRSeqHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipRSeqHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad RSeq header,
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
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderGetStrBadSyntax(
                                               IN RvSipRSeqHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
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

    return MsgUtilsFillUserBuffer(((MsgRSeqHeader*)hHeader)->hPool,
                                  ((MsgRSeqHeader*)hHeader)->hPage,
                                  SipRSeqHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRSeqHeaderSetStrBadSyntax
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
RvStatus SipRSeqHeaderSetStrBadSyntax(
                                  IN RvSipRSeqHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgRSeqHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgRSeqHeader*)hHeader;

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
 * RvSipRSeqHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal RSeq header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRSeqHeaderSetStrBadSyntax(
                                  IN RvSipRSeqHeaderHandle hHeader,
                                  IN RvChar*              strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipRSeqHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RSeqHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void RSeqHeaderClean( IN MsgRSeqHeader* pHeader,
                             IN RvBool        bCleanBS)
{
    pHeader->respNum.val          = 0;
	pHeader->respNum.bInitialized = RV_FALSE; 
    pHeader->eHeaderType		  = RVSIP_HEADERTYPE_RSEQ;

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

