
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          RvSipRAckHeader.c                                 *
 *                                                                            *
 * The file defines the methods of the RAck header object:                    *
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

#include "RvSipRAckHeader.h"
#include "_SipRAckHeader.h"
#include "RvSipCSeqHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void RAckHeaderClean( IN MsgRAckHeader* pHeader,
                             IN RvBool        bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipRAckHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a RAck header object inside a given message object.
 *          The header is kept in the header list of the message.
 *          You can choose to insert the header either at the head or tail of
 *          the header list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message related to the new header object.
 * output: hHeader - Handle to the RAck header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderConstructInMsg(
                                           IN  RvSipMsgHandle          hSipMsg,
                                           IN  RvBool                 pushHeaderAtHead,
                                           OUT RvSipRAckHeaderHandle *hHeader)
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

    stat = RvSipRAckHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);

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
                              RVSIP_HEADERTYPE_RACK,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipRAckHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone RAck Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed RAck header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderConstruct(
                                           IN  RvSipMsgMgrHandle      hMsgMgr,
                                           IN  HRPOOL                 hPool,
                                           IN  HPAGE                  hPage,
                                           OUT RvSipRAckHeaderHandle* hHeader)
{
    MsgRAckHeader*  pHeader;
    MsgMgr*         pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgRAckHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgRAckHeader),
                                                    RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRAckHeaderConstruct - Allocation failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr     = pMsgMgr;
    pHeader->hPage       = hPage;
    pHeader->hPool       = hPool;
    RAckHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipRAckHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipRAckHeaderConstruct - RAck header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipRAckHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source RAck header object to a destination RAck header
 *          object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination RAck header object.
 *    hSource      - Handle to the source RAck header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderCopy(
                                       INOUT    RvSipRAckHeaderHandle hDestination,
                                       IN       RvSipRAckHeaderHandle hSource)
{
    MsgRAckHeader* source;
    MsgRAckHeader* dest;
    RvStatus retStatus = RV_OK;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest   = (MsgRAckHeader*)hDestination;
    source = (MsgRAckHeader*)hSource;

    dest->eHeaderType = source->eHeaderType;
	dest->rseq.bInitialized = source->rseq.bInitialized;
	dest->rseq.val			= source->rseq.val;

    if(source->hCSeq == NULL)
    {
        dest->hCSeq = NULL;
    }
    else
    {
        if (NULL == dest->hCSeq)
        {
            retStatus = RvSipCSeqHeaderConstructInRAckHeader(hDestination, &(dest->hCSeq));
            if (RV_OK != retStatus)
            {
                return retStatus;
            }
        }
        retStatus = RvSipCSeqHeaderCopy(dest->hCSeq, source->hCSeq);
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
                "RvSipRAckHeaderCopy - failed in coping strBadSyntax."));
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

    return retStatus;
}

/***************************************************************************
 * RvSipRAckHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a RAck header object to a textual RAck header. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage(). The
 *          allocated page must be freed only if this function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader  - Handle to the RAck header object.
 *         hPool    - Handle to the specified memory pool.
 * output: hPage    - The memory page allocated to contain the encoded header.
 *         length   - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderEncode(
                                          IN    RvSipRAckHeaderHandle  hHeader,
                                          IN    HRPOOL                 hPool,
                                          OUT   HPAGE*                 hPage,
                                          OUT   RvUint32*             length)
{
    RvStatus stat;
    MsgRAckHeader* pHeader = (MsgRAckHeader*)hHeader;

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
                "RvSipRAckHeaderEncode - Failed, RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRAckHeaderEncode - Got new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = RAckHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRAckHeaderEncode - Failed. Free page 0x%p on pool 0x%p. RAckHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * RAckHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            RAck header (as string) on it.
 *          RAck = "RAck: " 1*DIGIT
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the RAck header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderEncode(
                              IN    RvSipRAckHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length)
{
    MsgRAckHeader*  pHeader;
    RvStatus       stat;
    RvChar         strHelper[16];

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgRAckHeader*)hHeader;

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RAckHeaderEncode - Encoding RAck header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* validation checking is done only for a legal header */
    if(pHeader->strBadSyntax == UNDEFINED)
    {
        /* the header must contain response-num */
        if (pHeader->hCSeq == NULL || pHeader->rseq.bInitialized == RV_FALSE)
		{
            return RV_ERROR_BADPARAM;
		}
    }
    /* The RAck header looks like: "RAck:300 1 INVITE"*/

    /* set "RAck" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "RAck", length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RAckHeaderEncode - Failed to encode header name. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                    MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                    length);
    if(stat != RV_OK)
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
                "RAckHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* responseNum */
	RvSprintf(strHelper,"%u",pHeader->rseq.val); 
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
    if(stat != RV_OK)
        return stat;

    /* space */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetSpaceChar(bInUrlHeaders), length);
    if(stat != RV_OK)
        return stat;

    /* cseq header */
    stat = CSeqHeaderEncode(pHeader->hCSeq,hPool,hPage, RV_FALSE, bInUrlHeaders, length);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RAckHeaderEncode - Failed to encode RAck header. RvStatus is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipRAckHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual RAck header-for example, "RAck: 1 INVITE"-into a
 *         RAck header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the RAck header object.
 *    buffer    - Buffer containing a textual RAck header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderParse(
                                     IN    RvSipRAckHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgRAckHeader* pHeader = (MsgRAckHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RAckHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RACK,
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
 * RvSipRAckHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual RAck header value into an RAck header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipRAckHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the RAck header object.
 *    buffer    - The buffer containing a textual RAck header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderParseValue(
                                     IN    RvSipRAckHeaderHandle    hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgRAckHeader* pHeader = (MsgRAckHeader*)hHeader;
    RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RAckHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_RACK,
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
 * RvSipRAckHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an RAck header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderFix(
                                     IN RvSipRAckHeaderHandle hHeader,
                                     IN RvChar*              pFixedBuffer)
{
    MsgRAckHeader* pHeader = (MsgRAckHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipRAckHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRAckHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipRAckHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipRAckHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RAck object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipRAckHeaderGetPool(RvSipRAckHeaderHandle hHeader)
{
    return ((MsgRAckHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipRAckHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RAck object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipRAckHeaderGetPage(RvSipRAckHeaderHandle hHeader)
{
    return ((MsgRAckHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipRAckHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the RAck header fields are kept in a string format.
 *          In order to get such a field from the RAck header object,
 *          your application should supply an adequate buffer to where the
 *          string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the RAck header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipRAckHeaderGetStringLength(
                                      IN  RvSipRAckHeaderHandle     hHeader,
                                      IN  RvSipRAckHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgRAckHeader* pHeader = (MsgRAckHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_RACK_BAD_SYNTAX:
        {
            stringOffset = SipRAckHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipRAckHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}

/***************************************************************************
 * RvSipRAckHeaderIsResponseNumInitialized
 * ------------------------------------------------------------------------
 * General: Checks if the response num value from the RAck header object is
 *		    defined. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the RAck header object.
 *	  pbInit    - Indication of the resp number definition.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderIsResponseNumInitialized(
                                          IN  RvSipRAckHeaderHandle hHeader,
										  OUT RvBool			   *pbInit)
{
	MsgRAckHeader *pHeader = (MsgRAckHeader*)hHeader; 
		
    if(hHeader == NULL)
	{ 
		return RV_ERROR_INVALID_HANDLE; 	 
	}
	
	*pbInit = pHeader->rseq.bInitialized;

	return RV_OK;
}

/***************************************************************************
 * RvSipRAckHeaderGetResponseNum
 * ------------------------------------------------------------------------
 * General: Gets the response num value from the RAck header object.
 * Return Value: Returns the response number value, or UNDEFINED if the number
 *               is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RAck header object.
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipRAckHeaderGetResponseNum(
                                          IN  RvSipRAckHeaderHandle hHeader)
{
	if(hHeader == NULL)
	{
        return 0;
	}

	return ((MsgRAckHeader*)hHeader)->rseq.val; 
}



/***************************************************************************
 * RvSipRAckHeaderSetResponseNum
 * ------------------------------------------------------------------------
 * General:Sets the response num value in the RAck header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the RAck header object.
 *    responseNum    - response num value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderSetResponseNum(
                                          IN    RvSipRAckHeaderHandle hHeader,
                                          IN    RvUint32              responseNum)
{
	MsgRAckHeader *pHeader = (MsgRAckHeader*)hHeader; 
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader->rseq.bInitialized = RV_TRUE; 
	pHeader->rseq.val		   = responseNum; 
	 
	return RV_OK;
}

/***************************************************************************
 * RvSipRAckHeaderGetCSeqHandle
 * ------------------------------------------------------------------------
 * General: Gets the Cseq Handle frp, tje RAck header.
 * Return Value: Returns the handle to the CSeq header object, or NULL
 *               if the CSeq header object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the RAck header object.
 ***************************************************************************/
RVAPI RvSipCSeqHeaderHandle RVCALLCONV RvSipRAckHeaderGetCSeqHandle(
                                        IN  RvSipRAckHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return NULL;
    }
    return ((MsgRAckHeader *)hHeader)->hCSeq;

}


/***************************************************************************
 * RvSipRAckHeaderSetCSeqHandle
 * ------------------------------------------------------------------------
 * General: Sets a new CSeq header in the RAck header object.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The RAck header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the RAck header object.
 *         hCSeq - The CSeq handle to be set in the RAck header.
 *                 If the CSeq handle is NULL, the existing CSeq header
 *                 is removed from the RAck header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderSetCSeqHandle(
                                       IN  RvSipRAckHeaderHandle hHeader,
                                       IN  RvSipCSeqHeaderHandle hCSeq)
{

    MsgRAckHeader         *pRAck;
    RvStatus             retStatus;
    RvSipCSeqHeaderHandle hTempCSeq;

    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    pRAck = (MsgRAckHeader *)hHeader;

    if (NULL != pRAck->hCSeq)
    {
        retStatus = RvSipCSeqHeaderCopy(pRAck->hCSeq, hCSeq);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        return RV_OK;
    }

    retStatus = RvSipCSeqHeaderConstructInRAckHeader(hHeader, &hTempCSeq);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipCSeqHeaderCopy(hTempCSeq, hCSeq);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pRAck->hCSeq = hTempCSeq;
    return RV_OK;
}

/***************************************************************************
 * SipRAckHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRAckHeaderGetStrBadSyntax(
                                    IN RvSipRAckHeaderHandle hHeader)
{
    return ((MsgRAckHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipRAckHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad RAck header,
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
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderGetStrBadSyntax(
                                               IN RvSipRAckHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgRAckHeader*)hHeader)->hPool,
                                  ((MsgRAckHeader*)hHeader)->hPage,
                                  SipRAckHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipRAckHeaderSetStrBadSyntax
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
RvStatus SipRAckHeaderSetStrBadSyntax(
                                  IN RvSipRAckHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgRAckHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgRAckHeader*)hHeader;

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
 * RvSipRAckHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal RAck header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipRAckHeaderSetStrBadSyntax(
                                  IN RvSipRAckHeaderHandle hHeader,
                                  IN RvChar*              strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipRAckHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RAckHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader        - Pointer to the header object to clean.
 *  bCleanBS       - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void RAckHeaderClean( IN MsgRAckHeader* pHeader,
                             IN RvBool        bCleanBS)
{
	pHeader->rseq.bInitialized = RV_FALSE;
	pHeader->rseq.val		   = 0;
    pHeader->hCSeq			   = NULL;
    pHeader->eHeaderType  = RVSIP_HEADERTYPE_RACK;

    if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif

