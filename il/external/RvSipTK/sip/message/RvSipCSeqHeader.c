/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          RvSipCSeqHeader.c                                 *
 *                                                                            *
 * The file defines the methods of the CSeq header object:                    *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
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
#ifndef RV_SIP_LIGHT

#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"
#include "rvansi.h"
#include <string.h>
#include <stdlib.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void CSeqHeaderClean(IN MsgCSeqHeader* pHeader,
                            IN RvBool        bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCSeqHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a CSeq header object inside a given message object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message related to the new header object.
 * output: hHeader - Handle to the CSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderConstructInMsg(
                                           IN  RvSipMsgHandle         hSipMsg,
                                           OUT RvSipCSeqHeaderHandle *hHeader)
{
    MsgMessage*           msg;
    RvStatus           stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipCSeqHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    /* attach the header in the msg */
    msg->hCSeqHeader = *hHeader;
    return RV_OK;
}


/***************************************************************************
 * RvSipCSeqHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone CSeq Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed CSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderConstruct(
                                           IN  RvSipMsgMgrHandle      hMsgMgr,
                                           IN  HRPOOL                 hPool,
                                           IN  HPAGE                  hPage,
                                           OUT RvSipCSeqHeaderHandle* hHeader)
{
    MsgCSeqHeader*    pHeader;
    MsgMgr*           pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgCSeqHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgCSeqHeader),
                                                    RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr        = pMsgMgr;
    pHeader->hPage          = hPage;
    pHeader->hPool          = hPool;
    CSeqHeaderClean(pHeader, RV_TRUE);


    *hHeader = (RvSipCSeqHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderConstruct - CSeq header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipCSeqHeaderConstructInRAckHeader
 * ------------------------------------------------------------------------
 * General: Constructs a CSeq header object in a given RAck header.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader - Handle to the RAck Header
 * output: phCSeq - Handle to the newly constructed CSeq object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderConstructInRAckHeader(
                                       IN  RvSipRAckHeaderHandle  hHeader,
                                       OUT RvSipCSeqHeaderHandle  *phCSeq)
{
    MsgRAckHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if (phCSeq == NULL)
        return RV_ERROR_NULLPTR;

    *phCSeq = NULL;

    pHeader = (MsgRAckHeader*)hHeader;

    /* creating the new date object */
    stat = RvSipCSeqHeaderConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr, pHeader->hPool, pHeader->hPage, phCSeq);

    if(stat != RV_OK)
    {
        return stat;
    }
    /* associating the new date parameter to the Expires header */
    pHeader->hCSeq = *phCSeq;
    return RV_OK;
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * RvSipCSeqHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source CSeq header object to a destination CSeq header
 *          object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination CSeq header object.
 *    hSource      - Handle to the source CSeq header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderCopy(
                                       INOUT    RvSipCSeqHeaderHandle hDestination,
                                       IN       RvSipCSeqHeaderHandle hSource)
{
    MsgCSeqHeader* source;
    MsgCSeqHeader* dest;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest   = (MsgCSeqHeader*)hDestination;
    source = (MsgCSeqHeader*)hSource;

    dest->eHeaderType = source->eHeaderType;

	SipCommonCSeqCopy(&source->step,&dest->step); 
    dest->eMethod = source->eMethod;

    if(source->strOtherMethod > UNDEFINED)
    {
        dest->strOtherMethod = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strOtherMethod);
        if(dest->strOtherMethod == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipCSeqHeaderCopy - Failed to copy method string"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOtherMethod = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strOtherMethod);
#endif
    }
    else
    {
        dest->strOtherMethod = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOtherMethod = NULL;
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
                "RvSipCSeqHeaderCopy - Failed to copy BadSyntax string"));
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
 * RvSipCSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a CSeq header object to a textual CSeq header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage(). The
 *          allocated page must be freed only if this function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader  - Handle to the CSeq header object.
 *         hPool    - Handle to the specified memory pool.
 * output: hPage    - The memory page allocated to contain the encoded header.
 *         length   - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderEncode(
                                          IN    RvSipCSeqHeaderHandle  hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgCSeqHeader* pHeader;

    pHeader = (MsgCSeqHeader*)hHeader;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipCSeqHeaderEncode - Failed, RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipCSeqHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = CSeqHeaderEncode(hHeader, hPool, *hPage, RV_TRUE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipCSeqHeaderEncode - Failed. Free page 0x%p on pool 0x%p. CSeqHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * CSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            CSeq header (as string) on it.
 *          CSeq = "CSeq: " 1*DIGIT Method
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the CSeq header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderEncode(
                              IN    RvSipCSeqHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                b_isCSeq,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length)
{
    MsgCSeqHeader*  pHeader;
    RvStatus       stat;
    RvChar         strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgCSeqHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "CSeqHeaderEncode - Encoding CSeq header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* The CSeq header looks like: "CSeq: 1 INVITE"*/

    /* set "CSeq: " */
    if(b_isCSeq == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "CSeq", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "CSeqHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
            return stat;
        }
        /* encode ": " or "=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);
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
    else
    {
		RvStatus rv; 
		RvInt32  cseqStep; 

        /* step */
		rv = SipCommonCSeqGetStep(&pHeader->step,&cseqStep); 
		if (rv != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "CSeqHeaderEncode - Failed to encode header. CSeq step is probably NOT initialized in hPool 0x%p, hPage 0x%p",
                hPool, hPage));
			return rv;
		}
#ifdef RV_SIP_UNSIGNED_CSEQ	
		RvSprintf(strHelper,"%u",cseqStep); 
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
		MsgUtils_itoa(cseqStep, strHelper);
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
        
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);

        /* put space between step to method */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSpaceChar(bInUrlHeaders), length);

        /* method */
        stat = MsgUtilsEncodeMethodType(pHeader->pMsgMgr,
                                        hPool,
                                        hPage,
                                        pHeader->eMethod,
                                        pHeader->hPool,
                                        pHeader->hPage,
                                        pHeader->strOtherMethod,
                                        length);
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "CSeqHeaderEncode - Failed to encode CSeq header. RvStatus is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipCSeqHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual CSeq header-for example, "CSeq: 1 INVITE"-into a
 *         CSeq header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the CSeq header object.
 *    buffer    - Buffer containing a textual CSeq header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderParse(
                                     IN    RvSipCSeqHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgCSeqHeader* pHeader = (MsgCSeqHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    CSeqHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CSEQ,
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
 * RvSipCSeqHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual CSeq header value into an CSeq header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipCSeqHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the CSeq header object.
 *    buffer    - The buffer containing a textual CSeq header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderParseValue(
                                     IN    RvSipCSeqHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgCSeqHeader* pHeader = (MsgCSeqHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    CSeqHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CSEQ,
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
 * RvSipCSeqHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an CSeq header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderFix(
                                     IN RvSipCSeqHeaderHandle hHeader,
                                     IN RvChar*              pFixedBuffer)
{
    MsgCSeqHeader* pHeader = (MsgCSeqHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipCSeqHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipCSeqHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the CSeq object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipCSeqHeaderGetPool(RvSipCSeqHeaderHandle hHeader)
{
    return ((MsgCSeqHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipCSeqHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the CSeq object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipCSeqHeaderGetPage(RvSipCSeqHeaderHandle hHeader)
{
    return ((MsgCSeqHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipCSeqHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the CSeq header fields are kept in a string format-for example, the
 *          CSeq header method. In order to get such a field from the CSeq header object,
 *          your application should supply an adequate buffer to where the string will be
 *          copied.
 *          This function provides you with the length of the string to enable you to
 *          allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the CSeq header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipCSeqHeaderGetStringLength(
                                      IN  RvSipCSeqHeaderHandle     hHeader,
                                      IN  RvSipCSeqHeaderStringName stringName)
{
    RvInt32       string;
    MsgCSeqHeader* pHeader = (MsgCSeqHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_CSEQ_METHOD:
        {
            string = SipCSeqHeaderGetStrMethodType(hHeader);
            break;
        }
        case RVSIP_CSEQ_BAD_SYNTAX:
        {
            string = SipCSeqHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipCSeqHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(string > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, string)+1);
    else
        return 0;
}

#ifdef RV_SIP_UNSIGNED_CSEQ
/***************************************************************************
 * RvSipCSeqHeaderGetStep
 * ------------------------------------------------------------------------
 * General: Gets the step value from the CSeq header object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the CSeq header object.
 *    PStep   - Pointer the step number value, or UNDEFINED if the 
 *				step is not set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderGetStep(
                                          IN  RvSipCSeqHeaderHandle hHeader,
										  OUT RvUint32			   *pStep)
{
	RvStatus	   rv;
	MsgCSeqHeader *pHeader = (MsgCSeqHeader*)hHeader; 
	
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	rv = SipCommonCSeqGetStep(&pHeader->step,(RvInt32*)pStep); 
	if (rv != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderGetStep - header 0x%p - Failure couldn't retrieve cseq step", hHeader));
		
		return rv;
	}

	return RV_OK;
}
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
/***************************************************************************
 * RvSipCSeqHeaderGetStep
 * ------------------------------------------------------------------------
 * General: Gets the step value from the CSeq header object.
 * Return Value: Returns the step number value, or UNDEFINED if the step is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the CSeq header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipCSeqHeaderGetStep(
                                          IN RvSipCSeqHeaderHandle hHeader)
{
	RvStatus	   rv;
	RvInt32		   cseqStep;
	MsgCSeqHeader *pHeader = (MsgCSeqHeader*)hHeader; 
	
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
	rv = SipCommonCSeqGetStep(&pHeader->step,&cseqStep); 
	if (rv != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipCSeqHeaderGetStep - header 0x%p - Couldn't retrieve cseq step", hHeader));
		
		return UNDEFINED;
	}
	
	return cseqStep;
}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

/***************************************************************************
 * SipCSeqHeaderGetInitializedCSeq
 * ------------------------------------------------------------------------
 * General: Gets the step value from the CSeq header object and check if
 *			it was already initialized. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	  - Handle to the CSeq header object.
 *	  pStep       - Pointer to the value of the retrieved CSeq
 ***************************************************************************/
RvStatus RVCALLCONV SipCSeqHeaderGetInitializedCSeq(
										IN  RvSipCSeqHeaderHandle hHeader,
										OUT RvInt32				 *pStep)
{
	RvStatus	   rv; 
	MsgCSeqHeader *pHeader   = (MsgCSeqHeader*)hHeader;
	
	rv = SipCommonCSeqGetStep(&pHeader->step,pStep); 
	if (rv != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SipCSeqHeaderGetInitializedCSeq - header 0x%p - Couldn't retrieve cseq step", hHeader));
		
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipCSeqHeaderSetStep
 * ------------------------------------------------------------------------
 * General:Sets the CSeq step value in the CSeq header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the CSeq header object.
 *    step    - CSeq value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderSetStep(
                                          IN    RvSipCSeqHeaderHandle hHeader,
                                          IN    RvInt32               step)
{
	RvStatus	   rv; 
    MsgCSeqHeader *pHeader = (MsgCSeqHeader*)hHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	rv = SipCommonCSeqSetStep(step,&pHeader->step); 
	if (rv != RV_OK)
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
			"RvSipCSeqHeaderSetStep - Could not set cseq step"));
		
		return rv;		
	}

	return RV_OK;
}


/***************************************************************************
 * RvSipCSeqHeaderGetMethodType
 * ------------------------------------------------------------------------
 * General: Gets the method type enumeration from the CSeq header object.
 *          If this function returns RVSIP_METHOD_OTHER, call
 *          RvSipCseqHeaderGetStrMethodType() to get the actual method in a string
 *          format.
 * Return Value: Returns the method type enumeration from the CSeq header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the CSeq header object.
 ***************************************************************************/
RVAPI RvSipMethodType RVCALLCONV RvSipCSeqHeaderGetMethodType(
                                          IN  RvSipCSeqHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_METHOD_UNDEFINED;

    return ((MsgCSeqHeader*)hHeader)->eMethod;
}

/***************************************************************************
 * RvSipCSeqHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General: Copies the method type string from the CSeq header object into a given buffer.
 *          Use this function if RvSipCSeqHeaderGetMethodType() returns
 *          RVSIP_METHOD_OTHER.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the CSeq header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end.
 ***************************************************************************/
RvInt32 SipCSeqHeaderGetStrMethodType(IN  RvSipCSeqHeaderHandle hHeader)
{
    return ((MsgCSeqHeader*)hHeader)->strOtherMethod;
}

/***************************************************************************
 * RvSipCSeqHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type string value from the
 *          CSeq object.
 *          If the bufferLen is big enough, the function will copy the parameter,
 *          into the strBuffer. Else, it will return RV_ERROR_INSUFFICIENT_BUFFER, and
 *          the actualLen param will contain the needed buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle of the CSeq header object.
 *        strBuffer - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter + 1 for null in the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderGetStrMethodType(
                                          IN  RvSipCSeqHeaderHandle hHeader,
                                          IN  RvChar*              strBuffer,
                                          IN  RvUint               bufferLen,
                                          OUT RvUint*              actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgCSeqHeader*)hHeader)->hPool,
                                  ((MsgCSeqHeader*)hHeader)->hPage,
                                  SipCSeqHeaderGetStrMethodType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipCSeqHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General:This method sets the method type in the CSeq object. If eMethodType
 *         is RVSIP_METHOD_OTHER, the pMethodTypeStr will be copied to the header,
 *         otherwise it will be ignored.
 *         the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle of the CSeq header object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - text string giving the method type to be set in the object.
 *                  This argument is needed when eMethodType is RVSIP_METHOD_OTHER.
 *                  otherwise it may be NULL.
 *  strMethodOffset - Offset of a string on the page  (if relevant).
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipCSeqHeaderSetMethodType(IN    RvSipCSeqHeaderHandle hHeader,
                                     IN    RvSipMethodType       eMethodType,
                                     IN    RvChar*              strMethodType,
                                     IN    HRPOOL                hPool,
                                     IN    HPAGE                 hPage,
                                     IN    RvInt32              strMethodOffset)
{
    RvInt32       newStr;
    MsgCSeqHeader* pHeader;
    RvStatus      retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgCSeqHeader*)hHeader;

    pHeader->eMethod = eMethodType;

    if(eMethodType == RVSIP_METHOD_OTHER)
    {
        retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strMethodOffset,
                                      strMethodType, pHeader->hPool,
                                      pHeader->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pHeader->strOtherMethod = newStr;
#ifdef SIP_DEBUG
        pHeader->pOtherMethod = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                             pHeader->strOtherMethod);
#endif
    }
    else
    {
        pHeader->strOtherMethod = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pOtherMethod = NULL;
#endif
    }

    return RV_OK;
}



/***************************************************************************
 * RvSipCSeqHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General:Sets the method type in the CSeq object.
 *         If eMethodType is RVSIP_METHOD_OTHER, strMethodType is copied to the
 *         header. Otherwise strMethodType is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the CSeq header object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - You can use this parameter only if the eMethodType parameter is set to
 *                  RVSIP_METHOD_OTHER. In this case, you can supply the method as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderSetMethodType(
                                             IN    RvSipCSeqHeaderHandle hHeader,
                                             IN    RvSipMethodType       eMethodType,
                                             IN    RvChar*              pMethodTypeStr)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipCSeqHeaderSetMethodType(hHeader, eMethodType, pMethodTypeStr, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
* SipCSeqHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General:This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipCSeqHeaderGetStrBadSyntax(IN  RvSipCSeqHeaderHandle hHeader)
{
    return ((MsgCSeqHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipCSeqHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad CSeq header,
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
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderGetStrBadSyntax(
                                          IN  RvSipCSeqHeaderHandle hHeader,
                                          IN  RvChar*              strBuffer,
                                          IN  RvUint               bufferLen,
                                          OUT RvUint*              actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgCSeqHeader*)hHeader)->hPool,
                                  ((MsgCSeqHeader*)hHeader)->hPage,
                                  SipCSeqHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipCSeqHeaderSetStrBadSyntax
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
RvStatus SipCSeqHeaderSetStrBadSyntax(
                                  IN RvSipCSeqHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgCSeqHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgCSeqHeader*)hHeader;

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
 * RvSipCSeqHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal CSeq header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCSeqHeaderSetStrBadSyntax(
                                  IN RvSipCSeqHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipCSeqHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CSeqHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void CSeqHeaderClean(IN MsgCSeqHeader* pHeader,
                            IN RvBool        bCleanBS)
{
    pHeader->eMethod        = RVSIP_METHOD_UNDEFINED;
    pHeader->strOtherMethod = UNDEFINED;
    pHeader->eHeaderType    = RVSIP_HEADERTYPE_CSEQ;
	SipCommonCSeqReset(&pHeader->step); 

#ifdef SIP_DEBUG
    pHeader->pOtherMethod   = NULL;
#endif

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif
