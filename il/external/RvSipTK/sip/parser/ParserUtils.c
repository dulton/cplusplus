/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <ParserUtils.c>
 *
 * This file defines functions which are used from the parser to initialize sip
 * message or sip header.
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/
#include "RV_SIP_DEF.h"

#include "ParserUtils.h"
#include "ParserProcess.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"



/***************************************************************************
 * ParserErrLog
 * ------------------------------------------------------------------------
 * General: This function stop the parser and print an error message from
 *          the parser.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb - Global structure uses to hold global variables of the parser.
 ***************************************************************************/
void RVCALLCONV ParserErrLog(
                        IN SipParser_pcb_type * pcb)
{
    char err_buf[256] = {0};
    char * buf = err_buf;
    ParserMgr* pParserMgr;

    pParserMgr = (ParserMgr*)pcb->pParserMgr;

    pcb->exit_flag = AG_SEMANTIC_ERROR_CODE;

    buf+=RvSprintf(buf,"SIP Parser Error : ");

    /* we print the column in the header value. column+3 --> to skip
       the 3 characters (header type prefix) we set before the value */
    if(pcb->eWhichHeader == SIP_PARSETYPE_UNDEFINED ||
        pcb->eWhichHeader == SIP_PARSETYPE_OTHER)
    {
        buf+=RvSprintf(buf,"%s, line %d, column %d %c",
            pcb->error_message,pcb->lineNumber,pcb->column, '\0');

    }
    else
    {
        buf+=RvSprintf(buf,"%s, line %d, column %d of the header value %c",
            pcb->error_message,pcb->lineNumber,pcb->column, '\0');
    }

        /* Log the error using user-provided function */
    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,err_buf));

#ifndef RV_SIP_PRIMITIVES
    if (SIP_PARSETYPE_MSG == pcb->eHeaderType)
    {
        RvSipMsgHandle hMsg = (RvSipMsgHandle)pcb->pSipObject;
        /* save the parser error string in message, so will be sent in 400 response
           reason phrase */
        if(SipMsgGetBadSyntaxReasonPhrase(hMsg) == UNDEFINED)
        {
            SipMsgSetBadSyntaxReasonPhrase(hMsg,(RvChar*)&err_buf);
        }
    }
#endif /* RV_SIP_PRIMITIVES */

}



/***************************************************************************
 * ParserSetSyntaxErr
 * ------------------------------------------------------------------------
 * General: This function is called when execute a syntax error from the parser.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - Global structure uses to hold global variables of the parser.
 ***************************************************************************/
void RVCALLCONV ParserSetSyntaxErr(
                        IN void * pcb_)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    ParserErrLog(pcb);
}
/***************************************************************************
 * ParserCleanExtParams
 * ------------------------------------------------------------------------
 * General: Clean PCB.ExtParams which used to save the generic-param-list
 *         (if there was generic-param-list).
 * Return Value:void
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input/Output: extParams - Pointer to structure which holds temporary
 *                            page, pool in order to ba able to save the
 *                            extension strings from the parser
 ***************************************************************************/
void ParserCleanExtParams(INOUT void * pExtParams)
{
    ParserExtensionString * pExt = (ParserExtensionString *)pExtParams;
    if ((NULL_PAGE != pExt->hPage) && (0 != pExt->size))
    {
        RPOOL_FreePage(pExt->hPool,pExt->hPage);
        pExt->hPage = NULL_PAGE;
        pExt->size = 0;
    }

}
/***************************************************************************
 * ParserGetUINT32FromString
 * ------------------------------------------------------------------------
 * General: This method transfer string into a numeric number (32 bytes) .
 * Return Value: RV_OK - on success.
 *               RV_ERROR_UNKNOWN - error with the number calculation or overflow.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pBuffer - The string buffer.
 *            length  - The length of the string number in the buffer.
 *  Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
RvStatus RVCALLCONV ParserGetUINT32FromString(
                             IN  ParserMgr *pParserMgr,
                             IN  RvChar   *pBuffer,
                             IN  RvUint32 length,
                             OUT RvUint32  *pNum)
{
    RvInt32 i;
    RvUint32 digit10,digit,old_acc;
    RvUint32 power=1,old_power;
    *pNum= *(pBuffer+length-1)-'0';
    if (*pNum > 9)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetUINT32FromString - Check for int rollover - field too big "));
        return RV_ERROR_UNKNOWN;
    }
    digit10   = *pNum;
    digit     = *pNum;
    old_acc   = *pNum;
	old_power = power;

    for(i=length-2;i>=0;i--)
    {
        digit10 = (*(pBuffer+i)-'0')*power;
        digit = digit10*10;

       /* Check for int rollover - field too big */
        if(digit/10 != digit10)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserGetUINT32FromString - The number is bigger than 2^32 - 1"));

            return RV_ERROR_ILLEGAL_SYNTAX;
        }

        if ((*(pBuffer+i)-'0') < 0 || (*(pBuffer+i)-'0') > 9)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserGetUINT32FromString - Check for int rollover - field too big"));

            return RV_ERROR_ILLEGAL_SYNTAX;
        }
        *pNum+=digit;
        power*=10;

        /* Check if the number is bigger than 2^32 - 1 */
        if(old_acc>*pNum || old_power>power)
        {
           RvLogWarning(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					 "ParserGetUINT32FromString - The number is bigger than 2^32 - 1"));

            return RV_ERROR_ILLEGAL_SYNTAX;
        }

        old_acc   = *pNum;
		old_power = power;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pParserMgr);
#endif

    return(RV_OK);
}


/***************************************************************************
 * ParserGetUINT32FromStringHex
 * ------------------------------------------------------------------------
 * General: This method transfer string into a numeric number (32 bytes) .
 * Return Value: RV_OK - on success.
 *               RV_ERROR_UNKNOWN - error with the number calculation or overflow.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pBuffer - The string buffer.
 *            length  - The length of the string number in the buffer.
 *  Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
RvStatus RVCALLCONV ParserGetUINT32FromStringHex(
                             IN  ParserMgr *pParserMgr,
                             IN  RvChar   *pBuffer,
                             IN  RvUint32 length,
                             OUT RvUint32  *pNum)
{
    RvInt32 i;
    RvUint32 digit16,digit,old_acc;
    RvInt32 power=1,old_power;
    if (*(pBuffer+length -1) <= 'f' && *(pBuffer+length -1) >= 'a')
    {
        *pNum  = ( (*((pBuffer+length)-1)-'a') + 10);
    }
    else
    {
        *pNum = (*(pBuffer+length-1)-'0');
    }
    if (*pNum > 15 )
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetUINT32FromStringHex - Check for int rollover - field too big "));
        return RV_ERROR_UNKNOWN;
    }

    digit16   = *pNum;
    digit     = *pNum;
    old_acc   = *pNum;
	old_power = power;

    for(i=length-2;i>=0;i--)
    {
        if (*(pBuffer+i) <= 'f' && *(pBuffer+i) >= 'a')
        {
            digit16  = ((*(pBuffer+i)-'a')+10)*power;
        }
        else
        {
            digit16 = (*(pBuffer+i)-'0')*power;
        }

        digit = digit16*16;

       /* Check for int rollover - field too big */
        if(digit/16 != digit16)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					  "ParserGetUINT32FromStringHex - Check for int rollover - field too big "));

            return RV_ERROR_ILLEGAL_SYNTAX;
        }
        *pNum+=digit;
        power*=16;

        /* Check for int rollover - field too big */
        if(old_acc>*pNum || old_power>power)
        {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetUINT32FromStringHex - Check for int rollover - field too big "));

            return RV_ERROR_ILLEGAL_SYNTAX;
        }

        old_acc   = *pNum;
		old_power = power;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pParserMgr);
#endif

    return RV_OK;
}

/***************************************************************************
 * ParserGetINT32FromString
 * ------------------------------------------------------------------------
 * General: This method transfer string into a numeric number (32 bytes) .
 * Return Value: RV_OK - on success.
 *               RV_ERROR_UNKNOWN - error with the number calculation or overflow.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pBuffer - The string buffer.
 *            length  - The length of the string number in the buffer.
 *  Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
RvStatus RVCALLCONV ParserGetINT32FromString(
                             IN  ParserMgr *pParserMgr,
                             IN  RvChar   *pBuffer,
                             IN  RvUint32 length,
                             OUT RvInt32  *pNum)
{
    RvUint32 num;
    RvStatus eStat;
    eStat = ParserGetUINT32FromString(pParserMgr,pBuffer,length,&num);
    if (RV_OK != eStat)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetINT32FromString - error in  ParserGetINT32FromString"));
        return eStat;
    }
    /* Check whether the number is bigger than (2^31-1) which is the size of RvInt32 */
    if (num > RV_INT32_MAX)
    {
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				  "ParserGetINT32FromString - Got number bigger than 2^31-1"));
		return RV_ERROR_ILLEGAL_SYNTAX;
    }
    *pNum = num;
    return RV_OK;
}

/***************************************************************************
 * ParserGetINT32FromStringHex
 * ------------------------------------------------------------------------
 * General: This method transfer string into a numeric number (32 bytes) .
 * Return Value: RV_OK - on success.
 *               RV_ERROR_UNKNOWN - error with the number calculation or overflow.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pBuffer - The string buffer.
 *            length  - The length of the string number in the buffer.
 *  Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
RvStatus RVCALLCONV ParserGetINT32FromStringHex(
                             IN  ParserMgr *pParserMgr,
                             IN  RvChar   *pBuffer,
                             IN  RvUint32 length,
                             OUT RvInt32  *pNum)
{
    RvUint32 num;
    RvStatus eStat;
    eStat = ParserGetUINT32FromStringHex(pParserMgr,pBuffer,length,&num);
    if (RV_OK != eStat)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetINT32FromStringHex - error in  ParserGetUINT32FromStringHex"));
        return eStat;
    }
    /* Check whether the number is bigger than (2^31-1) which is the size of RvInt32 */
	if (num > RV_INT32_MAX)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserGetINT32FromStringHex - Got number bigger than 2^31-1"));
		return RV_ERROR_ILLEGAL_SYNTAX;
    }
    *pNum = num;
    return RV_OK;
}

/***************************************************************************
 * ParserGetINT16FromString
 * ------------------------------------------------------------------------
 * General: This method transfer string into a numeric number (16 bytes).
 * Return Value: RV_OK - on success.
 *               RV_ERROR_UNKNOWN - error with the number calculation or overflow.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input : pBuffer - The string buffer.
 *            length  - The length of the string number in the buffer.
 *    Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
RvStatus RVCALLCONV ParserGetINT16FromString(
                             IN  ParserMgr *pParserMgr,
                             IN  RvChar   *pBuffer,
                             IN  RvUint32 length,
                             OUT RvInt16  *pNum)
{
    RvStatus eStat;
    RvInt32 numVal;

    eStat = ParserGetINT32FromString(pParserMgr,pBuffer,length,&numVal);
    if (RV_OK != eStat)
    {
        return eStat;
    }
    if(numVal > RV_INT16_MAX)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserGetINT16FromString - Check for int rollover - field too big "));
        return RV_ERROR_ILLEGAL_SYNTAX;
    }
    *pNum = (RvInt16)numVal;
    return(RV_OK);
}


/***************************************************************************
 * ParserCopyRpoolString
 * ------------------------------------------------------------------------
 * General: The function is for setting a given string
 *          on a non consecutive part of the page.
 *          The given string is on a RPOOL page, and is given as pool+page+offset.
 * Return Value: offset of the new string on destPage, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: destPool     - Handle of the destination rpool.
 *        destPage     - Handle of the destination page
 *        sourcePool   - Handle of the source rpool.
 *        destPage     - Handle of the source page
 *          stringOffset - The offset of the string to set.
 *        stringSize   - The size of the string.
 ***************************************************************************/
RvInt32 ParserCopyRpoolString(IN  ParserMgr *pParserMgr,
                               IN  HRPOOL    destPool,
                               IN  HPAGE     destPage,
                               IN  HRPOOL    sourcePool,
                               IN  HPAGE     sourcePage,
                               IN  RvInt32  stringOffset,
                               IN  RvUint32 stringSize)
{
    RvStatus   eStat;
    RvInt32    offset = UNDEFINED;

    if(stringOffset < 0)
        return UNDEFINED;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pParserMgr);
#endif

    /* append strLen + 1, for terminate with NULL*/
    eStat = RPOOL_Append(destPool, destPage, stringSize, RV_FALSE, &offset);
    if(eStat != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserCopyRpoolString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
                destPool, destPage, eStat));
        return UNDEFINED;
    }
    eStat = RPOOL_CopyFrom(destPool,
                           destPage,
                           offset,
                           sourcePool,
                           sourcePage,
                           stringOffset,
                           stringSize);
    if(eStat != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserCopyRpoolString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
                destPool, destPage, eStat));
        return UNDEFINED;
    }
    else
        return offset;

}

/***************************************************************************
 * ParserAppendCopyRpoolString
 * ------------------------------------------------------------------------
 * General: The function is for setting a given string on a non consecutive part 
 *          of the page, and setting a NULL character in the end.
 *          The given string is on a RPOOL page, and is given as pool+page+offset.
 * Return Value: offset of the new string on destPage, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: destPool     - Handle of the destination rpool.
 *        destPage     - Handle of the destination page
 *        sourcePool   - Handle of the source rpool.
 *        destPage     - Handle of the source page
 *          stringOffset - The offset of the string to set.
 *        stringSize   - The size of the string.
 ***************************************************************************/
RvInt32 ParserAppendCopyRpoolString(IN  ParserMgr *pParserMgr,
                                   IN  HRPOOL    destPool,
                                   IN  HPAGE     destPage,
                                   IN  HRPOOL    sourcePool,
                                   IN  HPAGE     sourcePage,
                                   IN  RvInt32  stringOffset,
                                   IN  RvUint32 stringSize)
{
    RvInt32 offsetParams = UNDEFINED;
    RvInt32 offset       = UNDEFINED;
    RvStatus   rv = RV_OK;
    
    /* copy the rpool string */
    offsetParams = ParserCopyRpoolString(pParserMgr, destPool, destPage, sourcePool,
                                         sourcePage, 0, stringSize);
    if (UNDEFINED == offsetParams)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserAppendCopyRpoolString - error in ParserCopyRpoolString()"));
        return UNDEFINED;
    }
    
    /* set NULL in the end of the string */
    rv = ParserAppendFromExternal(pParserMgr, &offset, destPool, destPage, "\0" , 1);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserAppendCopyRpoolString - error in ParserAppendFromExternal()"));
        return UNDEFINED;
    }
    RV_UNUSED_ARG(stringOffset)
    return offsetParams;
}

/***************************************************************************
 * ParserAppendFromExternal
 * ------------------------------------------------------------------------
 * General: The function is for setting the given string on a non consecutive
 *          part of the page.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool      - Handle to the pool which is used to allocate from.
 *        hPage      - Handle to the page which is used to allocate from.
 *        pData      - The given string.
 *        sizeToCopy - The size of the string.
 * Ouput: pAllocationOffset - Pointer to the offset on the page which the
 *                            string was allocated.
 ***************************************************************************/
RvStatus RVCALLCONV ParserAppendFromExternal(
                                 IN  ParserMgr *pParserMgr,
                                 OUT RvInt32   *pAllocationOffset,
                                 IN  HRPOOL      hPool,
                                 IN  HPAGE       hPage,
                                 IN  RvChar     *pData,
                                 IN  RvUint32   sizeToCopy)
{
    RPOOL_ITEM_HANDLE    ptr;
    if (sizeToCopy == 0)
        return RV_OK;

    if(RPOOL_AppendFromExternal(hPool,
                                hPage,
                                (void*)pData,
                                sizeToCopy,
                                RV_FALSE,
                                pAllocationOffset,
                                &ptr) != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserAppendFromExternal - error in RPOOL_AppendFromExternal, size %d. hPool 0x%p, hPage 0x%p ",
            sizeToCopy,hPool,hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pParserMgr);
#endif

	return RV_OK;
}
/***************************************************************************
 * ParserAllocFromObjPage
 * ------------------------------------------------------------------------
 * General: The function is for setting the given string on a non consecutive
 *          part of the page, it also adds null terminator at the end of the
 *          string in the page.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool      - Handle to the pool which is used to allocate from.
 *        hPage      - Handle to the page which is used to allocate from.
 *        pData      - The given string.
 *        sizeToCopy - The size of the string.
 * Ouput: pAllocationOffset - Pointer to the offset on the page which the
 *                            string was allocated.
 ***************************************************************************/
RvStatus RVCALLCONV ParserAllocFromObjPage(
                                 IN  ParserMgr  *pParserMgr,
                                 OUT RvInt32   *pAllocationOffset,
                                 IN  HRPOOL      hPool,
                                 IN  HPAGE       hPage,
                                 IN  RvChar     *pData,
                                 IN  RvUint32   sizeToCopy)
{
    /* This offset is defined to send it as parameter to cancatanted the null
       terminator at the end of the string, but the offset on the page where the
       string started is *pAllocationOffset*/
    RvInt32 offset = UNDEFINED;

    if(ParserAppendFromExternal(pParserMgr,
                                       pAllocationOffset,
                                       hPool,
                                       hPage,
                                       pData,
                                       sizeToCopy) != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserAllocFromObjPage - error in ParserAppendFromExternal, size %d. hPool 0x%p, hPage 0x%p ",
            sizeToCopy,hPool,hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    if(ParserAppendFromExternal(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
                                       "\0",
                                       1) != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserAllocFromObjPage -  error in ParserAppendFromExternal, size %d. hPool 0x%p, hPage 0x%p ",
            1,hPool,hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}
/***************************************************************************
 * ParserAppendData
 * ------------------------------------------------------------------------
 * General: This method appends new string to page.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_NULLPTR       - In case the new string is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pData      - The new string which will be append.
 *            sizeToCopy - The size of the new string.
 *    Output: pExtParams - The structure which holds the the pool, the page
 *                       and the size of the written page.
 ***************************************************************************/
RvStatus RVCALLCONV ParserAppendData(
                     IN    const RvChar *pData,
                     IN    RvUint32     sizeToCopy,
                     IN    void          *pExtParams)
{
    RvStatus           eStatus;
    RPOOL_ITEM_HANDLE   ptr;
    RvInt32           offset = UNDEFINED;
    ParserExtensionString *pExt = (ParserExtensionString *)pExtParams;

    if (pExt->size == 0)
    {
        /* Get page from the pool */
        eStatus = RPOOL_GetPage(pExt->hPool, 0, &pExt->hPage);
        if(eStatus != RV_OK)
        {
            return eStatus;
        }
    }

    if(RPOOL_AppendFromExternal(pExt->hPool,
                                pExt->hPage,
                                (void*)pData,
                                sizeToCopy,
                                RV_FALSE,
                                &offset,
                                &ptr) != RV_OK)
    {
        RPOOL_FreePage(pExt->hPool, pExt->hPage);
        pExt->hPage = NULL_PAGE;
        pExt->size = 0;
        return RV_ERROR_OUTOFRESOURCES;
    }
    pExt->size  += sizeToCopy;
    return RV_OK;
}

/***************************************************************************
 * ParserInitializePCBStructs
 * ------------------------------------------------------------------------
 * General:This function initialize parser structs that are held in the PCB
 *         for further use.
 * Return Value: none
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input : eType     - The type of the struct to be initialized.
 *    Output: none
 ***************************************************************************/
void RVCALLCONV ParserInitializePCBStructs(IN  SipParsePCBStructType   eType,
                                           IN  void * pcb_)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    switch (eType)
    {
#ifdef RV_SIP_AUTH_ON
        case SIP_PARSE_PCBSTRUCT_AUTHORIZATION:
        {
            memset(&(pcb->authorization), 0, sizeof(pcb->authorization));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_DIGEST_CHALLENGE:
        {
            memset(&(pcb->digestChallenge), 0, sizeof(pcb->digestChallenge));
            pcb->digestChallenge.algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
            return;
        }
        case SIP_PARSE_PCBSTRUCT_AUTHENTICATION:
        {
            memset(&(pcb->authentication), 0, sizeof(pcb->authentication));
            return;
        }
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
        /*-------------------------------------------------------------------*/
        /*                      SUBS-REFER HEADERS                           */
        /*-------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
        case SIP_PARSE_PCBSTRUCT_REFER_TO:
        {
            memset(&(pcb->referTo), 0, sizeof(pcb->referTo));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_REFERRED_BY:
        {
            memset(&(pcb->referredBy), 0, sizeof(pcb->referredBy));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_EVENT_HEADER:
            {
                memset(&(pcb->event), 0, sizeof(pcb->event));
                return;
            }
        case SIP_PARSE_PCBSTRUCT_SUBS_STATE_HEADER:
            {
                memset(&(pcb->subsState), 0, sizeof(pcb->subsState));
                return;
            }
#endif /* #ifdef RV_SIP_SUBS_ON */
        case SIP_PARSE_PCBSTRUCT_RETRY_AFTER:
        {
            memset(&(pcb->retryAfter), 0, sizeof(pcb->retryAfter));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_CONTENT_DISPOSITION:
        {
            memset(&(pcb->contentDisposition), 0, sizeof(pcb->contentDisposition));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_REPLACES_PARAMS:
        {
            memset(&(pcb->replacesParams), 0, sizeof(pcb->replacesParams));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_RACK:
        {
            memset(&(pcb->rack), 0, sizeof(pcb->rack));
            return;
        }
#endif /* RV_SIP_PRIMITIVES */
#ifndef RV_SIP_LIGHT
        case SIP_PARSE_PCBSTRUCT_CONTACT:
        {
            memset(&(pcb->contact), 0, sizeof(pcb->contact));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_CONTENT_TYPE:
        {
            memset(&(pcb->contentType), 0, sizeof(pcb->contentType));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_EXPIRES:
        {
            memset(&(pcb->expires), 0, sizeof(pcb->expires));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_PARTY:
        {
            memset(&(pcb->party), 0, sizeof(pcb->party));
            return;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
        case SIP_PARSE_PCBSTRUCT_REQUEST_LINE:
        {
            memset(&(pcb->requestLine), 0, sizeof(pcb->requestLine));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_ROUTE:
        {
            memset(&(pcb->route), 0, sizeof(pcb->route));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_NAME_ADDR:
        {
            memset(&(pcb->nameAddr), 0, sizeof(pcb->nameAddr));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_EXURI:
        {
            memset(&(pcb->exUri), 0, sizeof(pcb->exUri));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_SIP_URL:
        {
            memset(&(pcb->sipUrl), 0, sizeof(pcb->sipUrl));
            return;
        }
#ifdef RV_SIP_IMS_HEADER_SUPPORT
        case SIP_PARSE_PCBSTRUCT_DIAMETER_URL:
        {
#if defined(UPDATED_BY_SPIRENT)
            memset(&(pcb->telUri), 0, sizeof(pcb->telUri));
#endif      
            memset(&(pcb->diameterUri), 0, sizeof(pcb->diameterUri));
            return;
        }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
        case SIP_PARSE_PCBSTRUCT_SINGLE_VIA:
        {
            memset(&(pcb->singleVia), 0, sizeof(pcb->singleVia));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_URL_PARAMETER:
        {
            memset(&(pcb->urlParameter), 0, sizeof(pcb->urlParameter));
            return;
        }
        case SIP_PARSE_PCBSTRUCT_SENT_PROTOCOL:
        {
            memset(&(pcb->sentProtocol), 0, sizeof(pcb->sentProtocol));
            pcb->sentProtocol.transport.transport = RVSIP_TRANSPORT_UNDEFINED;
            return;
        }
        case SIP_PARSE_PCBSTRUCT_VIA_SENT_BY:
        {
            memset(&(pcb->viaSentBy), 0, sizeof(pcb->viaSentBy));
            return;
        }
#ifdef RV_SIP_AUTH_ON

		case SIP_PARSE_PCBSTRUCT_AUTHENTICATION_INFO:
			{
				memset(&(pcb->auth_info), 0, sizeof(pcb->auth_info));
				return;
			}
#endif /* RV_SIP_AUTH_ON */
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case SIP_PARSE_PCBSTRUCT_REASON:
			{
				memset(&(pcb->reasonHeader), 0, sizeof(pcb->reasonHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_WARNING:
			{
				memset(&(pcb->warningHeader), 0, sizeof(pcb->warningHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_TIMESTAMP:
			{
				memset(&(pcb->timestampHeader), 0, sizeof(pcb->timestampHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_INFO:
			{
				memset(&(pcb->infoHeader), 0, sizeof(pcb->infoHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_ACCEPT:
			{
				memset(&(pcb->acceptHeader), 0, sizeof(pcb->acceptHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_ACCEPT_ENCODING:
			{
				memset(&(pcb->acceptEncodingHeader), 0, sizeof(pcb->acceptEncodingHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_ACCEPT_LANGUAGE:
			{
				memset(&(pcb->acceptLanguageHeader), 0, sizeof(pcb->acceptLanguageHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_REPLY_TO:
			{
				memset(&(pcb->replyToHeader), 0, sizeof(pcb->replyToHeader));
				return;
			}
			
			/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case SIP_PARSE_PCBSTRUCT_P_URI:
			{
				memset(&(pcb->puriHeader), 0, sizeof(pcb->puriHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_VISITED_NETWORK_ID:
			{
				memset(&(pcb->pvisitedNetworkIDHeader), 0, sizeof(pcb->pvisitedNetworkIDHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_ACCESS_NETWORK_INFO:
			{
				memset(&(pcb->paccessNetworkInfoHeader), 0, sizeof(pcb->paccessNetworkInfoHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_CHARGING_FUNCTION_ADDRESSES:
			{
				memset(&(pcb->pchargingFunctionAddressesHeader), 0, sizeof(pcb->pchargingFunctionAddressesHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_CHARGING_VECTOR:
			{
				memset(&(pcb->pchargingVectorHeader), 0, sizeof(pcb->pchargingVectorHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_SECURITY:
			{
				memset(&(pcb->securityHeader), 0, sizeof(pcb->securityHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_MEDIA_AUTHORIZATION:
			{
				memset(&(pcb->pmediaAuthorizationHeader), 0, sizeof(pcb->pmediaAuthorizationHeader));
				return;
			}
        case SIP_PARSE_PCBSTRUCT_P_PROFILE_KEY:
			{
				memset(&(pcb->pprofileKeyHeader), 0, sizeof(pcb->pprofileKeyHeader));
				return;
			}
        case SIP_PARSE_PCBSTRUCT_P_USER_DATABASE:
			{
				memset(&(pcb->puserDatabaseHeader), 0, sizeof(pcb->puserDatabaseHeader));
				return;
			}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case SIP_PARSE_PCBSTRUCT_P_DCS_TRACE_PARTY_ID:
			{
				memset(&(pcb->pdcsTracePartyIDHeader), 0, sizeof(pcb->pdcsTracePartyIDHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_DCS_OSPS:
			{
				memset(&(pcb->pdcsOSPSHeader), 0, sizeof(pcb->pdcsOSPSHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_DCS_BILLING_INFO:
			{
				memset(&(pcb->pdcsBillingInfoHeader), 0, sizeof(pcb->pdcsBillingInfoHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_DCS_LAES:
			{
				memset(&(pcb->pdcsLAESHeader), 0, sizeof(pcb->pdcsLAESHeader));
				return;
			}
		case SIP_PARSE_PCBSTRUCT_P_DCS_REDIRECT:
			{
				memset(&(pcb->pdcsRedirectHeader), 0, sizeof(pcb->pdcsRedirectHeader));
				return;
			}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        default:
            return;
    }
}


