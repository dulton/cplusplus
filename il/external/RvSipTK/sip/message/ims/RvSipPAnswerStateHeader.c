/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipPAnswerStateHeader.c						          *
 *                                                                            *
 * The file defines the methods of the PAnswerState header object:            *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ----------                                            *
 *      Mickey           Jul.2007                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipPAnswerStateHeader.h"
#include "_SipPAnswerStateHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void PAnswerStateHeaderClean(IN MsgPAnswerStateHeader* pHeader,
										  IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPAnswerStateHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PAnswerState header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PAnswerState header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPAnswerStateHeaderHandle* hHeader)
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

    stat = RvSipPAnswerStateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
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
                              RVSIP_HEADERTYPE_P_ANSWER_STATE,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipPAnswerStateHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PAnswerState Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PAnswerState header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPAnswerStateHeaderHandle* hHeader)
{
    MsgPAnswerStateHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    pHeader = (MsgPAnswerStateHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgPAnswerStateHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPAnswerStateHeaderConstruct - Failed to construct PAnswerState header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    PAnswerStateHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipPAnswerStateHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipPAnswerStateHeaderConstruct - PAnswerState header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipPAnswerStateHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PAnswerState header object to a destination PAnswerState
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PAnswerState header object.
 *    hSource      - Handle to the destination PAnswerState header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderCopy(
                                         INOUT    RvSipPAnswerStateHeaderHandle hDestination,
                                         IN    RvSipPAnswerStateHeaderHandle hSource)
{
    MsgPAnswerStateHeader*   source;
    MsgPAnswerStateHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
    source = (MsgPAnswerStateHeader*)hSource;
    dest   = (MsgPAnswerStateHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* Answer Type */
	dest->eAnswerType = source->eAnswerType;
    if(source->strAnswerType > UNDEFINED)
    {
        dest->strAnswerType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAnswerType);
        if(dest->strAnswerType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipPAnswerStateHeaderCopy - Failed to copy AnswerType"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pAnswerType */
#ifdef SIP_DEBUG
        dest->pAnswerType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strAnswerType);
#endif
    }
    else
    {
        dest->strAnswerType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAnswerType = NULL;
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
                      "RvSipPAnswerStateHeaderCopy - didn't manage to copy OtherParams"));
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
                "RvSipPAnswerStateHeaderCopy - failed in copying strBadSyntax."));
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
 * RvSipPAnswerStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PAnswerState header object to a textual PAnswerState header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PAnswerState header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderEncode(
                                          IN    RvSipPAnswerStateHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgPAnswerStateHeader* pHeader;

    pHeader = (MsgPAnswerStateHeader*)hHeader;

    if(hPage == NULL || length == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL) || (hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAnswerStateHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAnswerStateHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = PAnswerStateHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAnswerStateHeaderEncode - Failed. Free page 0x%p on pool 0x%p. PAnswerStateHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * PAnswerStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            PAnswerState header (as string) on it.
 *          format: "P-Answer-State: Confirmed"
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the PAnswerState header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV PAnswerStateHeaderEncode(
                                  IN    RvSipPAnswerStateHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgPAnswerStateHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}
	
    pHeader = (MsgPAnswerStateHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "PAnswerStateHeaderEncode - Encoding PAnswerState header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "P-Answer-State" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "P-Answer-State", length);
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PAnswerStateHeaderEncode - Failed to encoding PAnswerState header. stat is %d",
            stat));
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
                "PAnswerStateHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

    /* Answer Type */
	stat = MsgUtilsEncodeAnswerType(pHeader->pMsgMgr,
									hPool,
									hPage,
									pHeader->eAnswerType,
									pHeader->hPool,
									pHeader->hPage,
									pHeader->strAnswerType,
									length);
	if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "PAnswerStateHeaderEncode - Failed to encode Answer Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
		return stat;
    }

    /* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set OtherParmas */
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
            "PAnswerStateHeaderEncode - Failed to encoding PAnswerState header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAnswerState header-for example,
 *          "PAnswerState:IEEE802.11a"-into a PAnswerState header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PAnswerState header object.
 *    buffer    - Buffer containing a textual PAnswerState header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderParse(
                                     IN    RvSipPAnswerStateHeaderHandle	hHeader,
                                     IN    RvChar*								buffer)
{
    MsgPAnswerStateHeader*	pHeader = (MsgPAnswerStateHeader*)hHeader;
	RvStatus						rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    PAnswerStateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_ANSWER_STATE,
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
 * RvSipPAnswerStateHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAnswerState header value into an PAnswerState header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPAnswerStateHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PAnswerState header object.
 *    buffer    - The buffer containing a textual PAnswerState header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderParseValue(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgPAnswerStateHeader*    pHeader = (MsgPAnswerStateHeader*)hHeader;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
        return RV_ERROR_BADPARAM;
	}

    PAnswerStateHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_P_ANSWER_STATE,
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
 * RvSipPAnswerStateHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PAnswerState header with bad-syntax information.
 *          A SIP header has the following grammar:
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderFix(
                                     IN RvSipPAnswerStateHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgPAnswerStateHeader* pHeader = (MsgPAnswerStateHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipPAnswerStateHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPAnswerStateHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipPAnswerStateHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipPAnswerStateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PAnswerState object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipPAnswerStateHeaderGetPool(RvSipPAnswerStateHeaderHandle hHeader)
{
    return ((MsgPAnswerStateHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipPAnswerStateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PAnswerState object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipPAnswerStateHeaderGetPage(RvSipPAnswerStateHeaderHandle hHeader)
{
    return ((MsgPAnswerStateHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PAnswerState header fields are kept in a string format-for example, the
 *          PAnswerState header AnswerType name. In order to get such a field from the PAnswerState header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PAnswerState header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPAnswerStateHeaderGetStringLength(
                                      IN  RvSipPAnswerStateHeaderHandle     hHeader,
                                      IN  RvSipPAnswerStateHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgPAnswerStateHeader* pHeader = (MsgPAnswerStateHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;
	}

    switch (stringName)
    {
        case RVSIP_P_ANSWER_STATE_ANSWER_TYPE:
        {
            stringOffset = SipPAnswerStateHeaderGetStrAnswerType(hHeader);
            break;
        }
		case RVSIP_P_ANSWER_STATE_OTHER_PARAMS:
        {
            stringOffset = SipPAnswerStateHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_P_ANSWER_STATE_BAD_SYNTAX:
        {
            stringOffset = SipPAnswerStateHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipPAnswerStateHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * SipPAnswerStateHeaderGetStrAnswerType
 * ------------------------------------------------------------------------
 * General:This method gets the StrAnswerType embedded in the PAnswerState header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAnswerState header object..
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetStrAnswerType(
                                    IN RvSipPAnswerStateHeaderHandle hHeader)
{
    return ((MsgPAnswerStateHeader*)hHeader)->strAnswerType;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderGetStrAnswerType
 * ------------------------------------------------------------------------
 * General: Copies the StrAnswerType from the PAnswerState header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetStrAnswerType(
                                               IN RvSipPAnswerStateHeaderHandle hHeader,
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

    return MsgUtilsFillUserBuffer(((MsgPAnswerStateHeader*)hHeader)->hPool,
                                  ((MsgPAnswerStateHeader*)hHeader)->hPage,
                                  SipPAnswerStateHeaderGetStrAnswerType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAnswerStateHeaderSetAnswerType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pAnswerType in the
 *          PAnswerStateHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PAnswerState header object.
 *         pAnswerType - The Answer Type to be set in the PAnswerState header - If
 *                NULL, the existing Answer Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAnswerStateHeaderSetAnswerType(
                             IN    RvSipPAnswerStateHeaderHandle	hHeader,
							 IN	   RvSipPAnswerStateAnswerType	eAnswerType,
                             IN    RvChar *								pAnswerType,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset)
{
    RvInt32							newStr;
    MsgPAnswerStateHeader*	pHeader;
    RvStatus						retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAnswerStateHeader*) hHeader;

	pHeader->eAnswerType = eAnswerType;

	if(eAnswerType == RVSIP_ANSWER_TYPE_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pAnswerType,
									  pHeader->hPool, pHeader->hPage, &newStr);
		if (RV_OK != retStatus)
		{
			return retStatus;
		}
		pHeader->strAnswerType = newStr;
#ifdef SIP_DEBUG
	    pHeader->pAnswerType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strAnswerType);
#endif
    }
	else
	{
		pHeader->strAnswerType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pAnswerType = NULL;
#endif
	}
    return RV_OK;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderSetAnswerType
 * ------------------------------------------------------------------------
 * General:Sets the Answer Type in the PAnswerState header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strAnswerType - The Answer Type to be set in the PAnswerState header. If NULL is supplied, the
 *                 existing Answer Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetAnswerType(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
									 IN	   RvSipPAnswerStateAnswerType eAnswerType,
                                     IN    RvChar*                 strAnswerType)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    
	return SipPAnswerStateHeaderSetAnswerType(hHeader, eAnswerType, strAnswerType, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipPAnswerStateHeaderGetAnswerType
 * ------------------------------------------------------------------------
 * General: Returns the enumeration value for the access type.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 * output:RvSipPAnswerStateAnswerType - enumeration for the access type.
 ***************************************************************************/
RVAPI RvSipPAnswerStateAnswerType RVCALLCONV RvSipPAnswerStateHeaderGetAnswerType(
                                               IN RvSipPAnswerStateHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_ANSWER_TYPE_UNDEFINED;
	}

    return ((MsgPAnswerStateHeader*)hHeader)->eAnswerType;
}

/***************************************************************************
 * SipPAnswerStateHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the PAnswerState header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAnswerState header object..
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetOtherParams(
                                            IN RvSipPAnswerStateHeaderHandle hHeader)
{
    return ((MsgPAnswerStateHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PAnswerState header other params field of the PAnswerState header object into a
 *          given buffer.
 *          Not all the PAnswerState header parameters have separated fields in the PAnswerState
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAnswerState header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetOtherParams(
                                               IN RvSipPAnswerStateHeaderHandle hHeader,
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
	
    return MsgUtilsFillUserBuffer(((MsgPAnswerStateHeader*)hHeader)->hPool,
                                  ((MsgPAnswerStateHeader*)hHeader)->hPage,
                                  SipPAnswerStateHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAnswerStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PAnswerStateHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAnswerState header object.
 *            pOtherParams - The Other Params to be set in the PAnswerState header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAnswerStateHeaderSetOtherParams(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgPAnswerStateHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgPAnswerStateHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOtherParams,
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
 * RvSipPAnswerStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PAnswerState header object.
 *         Not all the PAnswerState header parameters have separated fields in the PAnswerState
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAnswerState header object.
 *    strOtherParams - The extended parameters field to be set in the PAnswerState header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetOtherParams(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAnswerStateHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipPAnswerStateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetStrBadSyntax(
                                    IN RvSipPAnswerStateHeaderHandle hHeader)
{
    return ((MsgPAnswerStateHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipPAnswerStateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad PAnswerState header,
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetStrBadSyntax(
                                               IN RvSipPAnswerStateHeaderHandle hHeader,
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
	
    return MsgUtilsFillUserBuffer(((MsgPAnswerStateHeader*)hHeader)->hPool,
                                  ((MsgPAnswerStateHeader*)hHeader)->hPage,
                                  SipPAnswerStateHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipPAnswerStateHeaderSetStrBadSyntax
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
RvStatus SipPAnswerStateHeaderSetStrBadSyntax(
                                  IN RvSipPAnswerStateHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgPAnswerStateHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgPAnswerStateHeader*)hHeader;

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
 * RvSipPAnswerStateHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PAnswerState header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetStrBadSyntax(
                                  IN RvSipPAnswerStateHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipPAnswerStateHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PAnswerStateHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void PAnswerStateHeaderClean( IN MsgPAnswerStateHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType		= RVSIP_HEADERTYPE_P_ANSWER_STATE;
    pHeader->strOtherParams		= UNDEFINED;
	pHeader->strAnswerType		= UNDEFINED;

#ifdef SIP_DEBUG
    pHeader->pOtherParams		= NULL;
	pHeader->pAnswerType		= NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

