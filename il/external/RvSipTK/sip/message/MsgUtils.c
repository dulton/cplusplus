/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           MsgUtils.c                                       *
 *                                                                            *
 * The file provides tools to help the encode functions                       *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "MsgUtils.h"
#include "RvSipHeader.h"
#include "RvSipBody.h"
#include "_SipMsg.h"
#include "MsgTypes.h"
#include "RvSipAddress.h"
#include "AdsRpool.h"
#include "_SipCommonUtils.h"
#include "rvansi.h"

#include <string.h>
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define MSG_MAX_QOP_TOKEN_LEN        256

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * MsgUtilsSetString
 * ------------------------------------------------------------------------
 * General: Uses to set a string to an object of the message family.
 *          hOutPool and hOutPage represents the pool and page of the object
 *          itself and hStringPool and hStringPage represents the pool and
 *          page on which the string lays. If the pool and page are equal no
 *          copying is done, and the out offset updated. else the string
 *          is copied to the object's pool. If the string pool and page are
 *          not specified this means that the string is given within the
 *          buffer and it is then copied to the object's pool as well.
 * Return Value: RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES, RV_OK,
 *               RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hStringPool - The pool in which the string lays. (see explanations up)
 *        hStringPage - The pool on which the string lays.
 *        stringOffset - The offset of the string.
 *        buffer - A buffer containing the string. (see explanations up)
 *        hOutPool - The object's pool.
 *        hOutPage - The object's page.
 * Output: pOffset - The offset of the string within the object's pool and page
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsSetString(IN    MsgMgr*     pMsgMgr,
                            IN    HRPOOL      hStringPool,
                            IN    HPAGE       hStringPage,
                            IN const RvInt32 stringOffset,
                            IN    RvChar*    buffer,
                            IN    HRPOOL      hOutPool,
                            IN    HPAGE       hOutPage,
                            OUT   RvInt32   *pOffset)
{
    if (NULL == pOffset)
    {
        return RV_ERROR_NULLPTR;
    }

    if ((NULL != hStringPool) &&
        (NULL_PAGE != hStringPage))
    {
        /* The string lays on the pool and page */
        if (UNDEFINED == stringOffset)
        {
            *pOffset = UNDEFINED;
        }
        else if ((hStringPool == hOutPool) &&
                 (hStringPage == hOutPage))
        {
            /* The pool and page of the string are the pool and page
               of the object */
            *pOffset = stringOffset;
        }
        else
        {
            /* Copy the string from the string's pool and page to the object's
               pool and page */
            *pOffset = MsgUtilsAllocCopyRpoolString(
                                             pMsgMgr, hOutPool, hOutPage, hStringPool,
                                             hStringPage, stringOffset);
            if (UNDEFINED == *pOffset)
            {
                return RV_ERROR_OUTOFRESOURCES;
            }
        }
        return RV_OK;
    }

    if (NULL == buffer)
    {
        *pOffset = UNDEFINED;
        return RV_OK;
    }

    *pOffset = MsgUtilsAllocSetString(pMsgMgr, hOutPool, hOutPage, buffer);
    if (UNDEFINED == *pOffset)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

/***************************************************************************
 * MsgUtilsFillingUserBuffer
 * ------------------------------------------------------------------------
 * General: The function fill a given buffer with a string.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER or RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool     - Handle of the pool with the string.
 *        hPage     - Handle of the page with the string.
 *        paramOffset - The offset of the string to be fill.
 *          buffer    - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter + 1 for null in the end.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsFillUserBuffer(
                                       HRPOOL   hPool,
                                       HPAGE    hPage,
                                 const RvInt32 paramOffset,
                                       RvChar* buffer,
                                       RvUint  bufferLen,
                                       RvUint* actualLen)
{
    if (paramOffset < 0)
        return RV_ERROR_NOT_FOUND;

    *actualLen = (RPOOL_Strlen(hPool, hPage, paramOffset) + 1);

    if(bufferLen < *actualLen)
        return RV_ERROR_INSUFFICIENT_BUFFER;
    else
        RPOOL_CopyToExternal(hPool, hPage, paramOffset, buffer, *actualLen);

    return RV_OK;
}
/***************************************************************************
 * SipMsgUtilsAllocAlign
 * ------------------------------------------------------------------------
 * General: The function allocate consecutive memory on the given page.
 *          This function is used when you allocate memory for a struct,
 *          The address of the struct need to be aligned.
 * Return Value: The pointer to the allocated buffer. NULL if didn't succeed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage -  Handle of the page.
 *          length - length of buffer to be allocate.
 *        consecutiveMemory - is the memory for the string is consecutive or not.
 ***************************************************************************/
RvChar* RVCALLCONV SipMsgUtilsAllocAlign(IN MsgMgr*   pMsgMgr,
                               IN HRPOOL    hPool,
                               IN HPAGE     hPage,
                               IN RvUint32 length,
                               IN RvBool   consecutiveMemory)
{
    RvInt32 allocated;
    RV_UNUSED_ARG(consecutiveMemory);
    RV_UNUSED_ARG(pMsgMgr);

    return RPOOL_AlignAppend(hPool, hPage, length, &allocated);

}

/***************************************************************************
 * MsgUtilsAllocCopyRpoolString
 * ------------------------------------------------------------------------
 * General: The function is for setting a given string (with the NULL in the end)
 *          on a non consecutive part of the page.
 *          The given string is on a RPOOL page, and is given as pool+page+offset.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for strlen+1 because of that.
 * Return Value: offset of the new string on destPage, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pMsgMgr - Pointer to the message manager.
 *        destPool     - Handle of the destination rpool.
 *        destPage     - Handle of the destination page
 *        sourcePool   - Handle of the source rpool.
 *        destPage     - Handle of the source page
 *          stringOffset - The offset of the string to set.
 ***************************************************************************/
RvInt32 RVCALLCONV MsgUtilsAllocCopyRpoolString(
                                         IN MsgMgr*  pMsgMgr,
                                         IN HRPOOL   destPool,
                                         IN HPAGE    destPage,
                                         IN HRPOOL   sourcePool,
                                         IN HPAGE    sourcePage,
                                         IN RvInt32 stringOffset)
{
    RvInt      strLen;
    RvStatus   stat;
    RvInt32    offset;

    if(stringOffset < 0)
        return RV_ERROR_BADPARAM;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    strLen = RPOOL_Strlen(sourcePool, sourcePage, stringOffset);

    /* append strLen + 1, for terminate with NULL*/
    stat = RPOOL_Append(destPool, destPage, strLen+1, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsAllocCopyRpoolString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return UNDEFINED;
    }
    stat = RPOOL_CopyFrom(destPool,
                          destPage,
                          offset,
                          sourcePool,
                          sourcePage,
                          stringOffset,
                          strLen + 1);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsAllocCopyRpoolString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return UNDEFINED;
    }
    else
        return offset;

}
/***************************************************************************
 * MsgUtilsAllocSetString
 * ------------------------------------------------------------------------
 * General: The function is for setting the given string (with the NULL in the end)
 *          on a consecutive part of the page.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for strlen+1 because of that.
 * Return Value: NULL or pointer to the new allocated string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage  - Handle of the page.
 *          string - The string to set.
 ***************************************************************************/
RvInt32 RVCALLCONV MsgUtilsAllocSetString(
                                IN MsgMgr*   pMsgMgr,
                                IN HRPOOL   hPool,
                                IN HPAGE    hPage,
                                IN RvChar* string)
{
    RvInt32           offset;
    RPOOL_ITEM_HANDLE    ptr;
    if(string == NULL)
    {
        return UNDEFINED;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    /* we ask for strlen+1, for setting also the NULL in the end */
    if(RPOOL_AppendFromExternal(hPool,
                                hPage,
                                (void*)string,
                                (RvInt)strlen(string)+1,
                                RV_FALSE,
                                &offset,
                                &ptr) != RV_OK)
    {

        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsAllocSetString - Failed ,AppendFromExternal failed. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return UNDEFINED;
    }

    return (RvInt32)offset;
}

/***************************************************************************
 * MsgUtils_itoa
 * ------------------------------------------------------------------------
 * General: The function fill the buffer with a string that is the number value.
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
void RVCALLCONV MsgUtils_itoa(RvInt32 number, RvChar* buffer)
{
    RvSprintf(buffer, "%d", number);
}

/***************************************************************************
 * MsgUtils_uitoa
 * ------------------------------------------------------------------------
 * General: The function fill the buffer with a string that is the number value.
 *          The number here is an unsigned int.
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
void RVCALLCONV MsgUtils_uitoa(RvUint32 number, RvChar* buffer)
{
    RvSprintf(buffer, "%u", number);
}

#ifndef RV_USE_MACROS
/***************************************************************************
 * MsgUtilsEncodeExternalString
 * ------------------------------------------------------------------------
 * General: The function set a string in the rpool page, by asking for memory
 *          and setting the string (with RPOOL_AppendFromExternal).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool - Handle of the pool
 *        hPage - Handle of the rpool page.
 *          string - the string to set.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeExternalString(
                                       IN  MsgMgr*         pMsgMgr,
                                       IN  HRPOOL         hPool,
                                       IN  HPAGE          hPage,
                                       IN  RvChar*       string,
                                       OUT RvUint32*     length)
{
    RvInt32         offset;
    RvStatus         stat;
    RPOOL_ITEM_HANDLE encoded;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    *length += (RvUint32)strlen(string);

    stat = RPOOL_AppendFromExternal(hPool,
                                    hPage,
                                    (void*)string,
                                    (RvInt)strlen(string),
                                    RV_FALSE,
                                    &offset,
                                    &encoded);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeExternalString - Failed, AppendFromExternal failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            hPool, hPage, stat));
    }
    return stat;
}
#endif /* #ifndef RV_USE_MACROS */

/***************************************************************************
 * MsgUtilsEncodeString
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end of it.
 *          The given string is from another rpool page (pool+page+offset).
 *          The setting os done by asking for memory on the dest page (RPOOL_Append)
 *          and setting the string (with RPOOL_CopyFrom).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hPool       - Handle of the pool of the given string.
 *         hPage       - Handle of the pool page of the given string.
 *           stringOffet - The offset of the string to encode.
 * Output: length      - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeString(
                               IN  MsgMgr*        pMsgMgr,
                               IN  HRPOOL         destPool,
                               IN  HPAGE          destPage,
                               IN  HRPOOL         hPool,
                               IN  HPAGE          hPage,
                               IN  RvInt32       stringOffset,
                               OUT RvUint32*     length)
{
    RvInt32         offset;
    RvStatus         stat = RV_OK;
    RvInt32          strLen;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    strLen = RPOOL_Strlen(hPool, hPage, stringOffset);
    if (strLen == -1)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeString - Failed, RPOOL_Strlen failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return RV_ERROR_UNKNOWN;
    }

    *length += strLen;

    stat = RPOOL_Append(destPool, destPage, strLen, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return stat;
    }

    stat = RPOOL_CopyFrom(destPool,
                          destPage,
                          offset,
                          hPool,
                          hPage,
                          stringOffset,
                          strLen);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, destPool 0x%p, destPage 0x%p, RvStatus: %d",
            hPool, hPage, destPool, destPage, stat));
    }
    return stat;
}

/******************************************************************************
 * MsgUtilsEncodeStringExt
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end
 *          of it. The given string is from another rpool page
 *          (pool+page+offset).
 *          The memory for the string is allocated from the Destination Pool
 *          and is added to the Destination Page using RPOOL_Append().
 *          The string firstly, is extracted from the Source Page to the buffer
 *          and the copied into the allocated memory on the Destination Page.
 *          The function ensures, that the encoded string is / is not
 *          surrounded by the quotes,if it was required by the calling function
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources)
 *               error code, defined in the RV_SIP_DEF.h, otherwise
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pMsgMgr       - Handle of the Message Manager.
 *         destPool      - Handle of the pool for the encoded string.
 *         destPage      - Handle of the page for the encoded string.
 *         hPool         - Handle of the pool of the given string.
 *         hPage         - Handle of the page of the given string.
 *         stringOffet   - The offset of the given string in the hPage page.
 *         bSetQuotes    - If RV_TRUE, the encoded string will be surrounded
 *                         by the quotes (only once).
 *                         If RV_FALSE, the encoded string will be not
 *                         surrounded by the quotes.
 *         bInUrlHeaders - If RV_TRUE, the '%22' symbol will be used for quotes
 *                         If RV_FALSE, the '"' symbol will be used for quotes
 * Output: length        - The given length + the encoded string length.
 *****************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeStringExt(
                                            IN  MsgMgr*     pMsgMgr,
                                            IN HRPOOL       destPool,
                                            IN HPAGE        destPage,
                                            IN  HRPOOL      hPool,
                                            IN  HPAGE       hPage,
                                            IN  RvInt32     stringOffset,
                                            IN  RvBool      bSetQuotes,
                                            IN  RvBool      bInUrlHeaders,
                                            OUT RvUint32*   length)
{
    RvStatus rv;
    RvInt32  offset;
    RvInt32  strLen, quotesStrLen;
    RvChar   buff[MSG_MAX_QOP_TOKEN_LEN];
    RvChar   *pUnquotedStrStart, *pUnquotedStrEnd;
    RvChar   *strStart;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    strLen = RPOOL_Strlen(hPool, hPage, stringOffset);
    if (strLen == -1)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeStringExt - Failed, RPOOL_Strlen failed. hPool 0x%p, hPage 0x%p",
            destPool, destPage));
        return RV_ERROR_UNKNOWN;
    }
    if (strLen+3 > MSG_MAX_QOP_TOKEN_LEN)  /* 3 stands for possible quotes and '\0' */
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeStringExt - string to be encoded is too large %d",
            strLen));
        return RV_ERROR_UNKNOWN;
    }

    quotesStrLen = (RvInt32)strlen(MsgUtilsGetQuotationMarkChar(bInUrlHeaders));
    memcpy(buff, MsgUtilsGetQuotationMarkChar(bInUrlHeaders), quotesStrLen);
    /* Now buff looks like <"> */
    rv = RPOOL_CopyToExternal(hPool, hPage, stringOffset, buff+quotesStrLen, strLen);
    if (RV_OK != rv)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeStringExt - RPOOL_CopyToExternal() failed. hPool 0x%p, hPage 0x%p (rv=%d:%s)",
            destPool, destPage, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Now buff looks like "string or ""string" */

    /* Find start of the unquoted string */
    if (0 == memcmp(buff+quotesStrLen,
                    MsgUtilsGetQuotationMarkChar(bInUrlHeaders), quotesStrLen))
    {
        pUnquotedStrStart = buff + quotesStrLen + quotesStrLen;
    }
    else
    {
        pUnquotedStrStart = buff + quotesStrLen;
    }
    /* Find end of the unquoted string */
    if (0 == memcmp(buff+quotesStrLen+strLen-quotesStrLen,
        MsgUtilsGetQuotationMarkChar(bInUrlHeaders), quotesStrLen))
    {
        pUnquotedStrEnd = buff + quotesStrLen + strLen - quotesStrLen;
    }
    else
    {
        pUnquotedStrEnd = buff + quotesStrLen + strLen;
    }
    
    /* Find final string start and end */
    if (RV_TRUE == bSetQuotes)
    {
        strStart = pUnquotedStrStart - quotesStrLen;
        strLen   = (RvInt32)((pUnquotedStrEnd - pUnquotedStrStart) + 2 * quotesStrLen);
        /* Set closing quotes */
        memcpy(pUnquotedStrEnd, MsgUtilsGetQuotationMarkChar(bInUrlHeaders), quotesStrLen);
    }
    else
    {
        strStart = pUnquotedStrStart;
        strLen   = (RvInt32)(pUnquotedStrEnd - pUnquotedStrStart);
    }

    *length += strLen;
    
    rv = RPOOL_Append(destPool, destPage, strLen, RV_FALSE, &offset);
    if(rv != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeStringExt - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p (rv=%d:%s)",
            destPool, destPage, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = RPOOL_CopyFromExternal(destPool, destPage, offset,
                                strStart, strLen);
    if(rv != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeStringExt - Failed, RPOOL_CopyFromExternal failed. hPool 0x%p, hPage 0x%p, destPool 0x%p, destPage 0x%p(rv=%d:%s)",
            hPool, hPage, destPool, destPage, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * MsgUtilsEncodeMethodType
 * ------------------------------------------------------------------------
 * General: The function add the string of the method to the page, without '/0'.
 *          Note - if the eMethod argument is RVSIP_METHOD_OTHER, the function
 *          will set the strMethod value. else, strMethod will be ignored.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMethod      - Value of the method to encode.
 *        stringPool   - Handle of the memory Pool where the strMethod is.
 *        stringPage   - Handle of the memory Page where the strMethod is.
 *        strMethod    - In case that eMethod is RVSIP_METHOD_OTHER,
 *                       this string offset will contain the method value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMethodType(
                                   IN MsgMgr*         pMsgMgr,
                                   IN HRPOOL          hPool,
                                   IN HPAGE           hPage,
                                   IN RvSipMethodType eMethod,
                                   IN HRPOOL          stringPool,
                                   IN HPAGE           stringPage,
                                   IN RvInt32        strMethod,
                                   OUT RvUint32*     length)

{
    switch(eMethod)
    {
        case RVSIP_METHOD_INVITE:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "INVITE", length);
        }
        case RVSIP_METHOD_ACK:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ACK", length);
        }
        case RVSIP_METHOD_BYE:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "BYE", length);
        }
        case RVSIP_METHOD_REGISTER:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "REGISTER", length);
        }
        case RVSIP_METHOD_REFER:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "REFER", length);
        }
        case RVSIP_METHOD_NOTIFY:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "NOTIFY", length);
        }
        case RVSIP_METHOD_CANCEL:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "CANCEL", length);
        }
        case RVSIP_METHOD_SUBSCRIBE:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "SUBSCRIBE", length);
        }
        case RVSIP_METHOD_PRACK:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "PRACK", length);
        }
        case RVSIP_METHOD_OTHER:
        {
            if(strMethod == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strMethod,
                                            length);
            }
        }
        case RVSIP_METHOD_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeMethodType - Undefined method"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeMethodType - Unknown eMethod: %d", eMethod));
            return RV_ERROR_BADPARAM;
        }
    }
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * MsgUtilsEncodeMediaType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media type to the page
 *          (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMediaType   - Value of the media type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaTypeOffset is.
 *        strMediaTypeOffset - In case that eMediaType is RVSIP_MEDIATYPE_OTHER,
 *                       this string offset will contain the media-type value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMediaType(
                                  IN MsgMgr*         pMsgMgr,
                                  IN HRPOOL         hPool,
                                  IN HPAGE          hPage,
                                  IN RvSipMediaType eMediaType,
                                  IN HRPOOL         stringPool,
                                  IN HPAGE          stringPage,
                                  IN RvInt32       strMediaTypeOffset,
                                  OUT RvUint32*    length)
{
    switch (eMediaType)
    {
    case RVSIP_MEDIATYPE_TEXT:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "text", length);
    case RVSIP_MEDIATYPE_IMAGE:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "image", length);
    case RVSIP_MEDIATYPE_AUDIO:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "audio", length);
    case RVSIP_MEDIATYPE_VIDEO:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "video", length);
    case RVSIP_MEDIATYPE_APPLICATION:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "application", length);
    case RVSIP_MEDIATYPE_MULTIPART:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "multipart", length);
    case RVSIP_MEDIATYPE_MESSAGE:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "message", length);
    case RVSIP_MEDIATYPE_OTHER:
        {
            if(strMediaTypeOffset == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strMediaTypeOffset,
                                            length);
            }
        }
    case RVSIP_MEDIATYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeMediaType - Undefined media type"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeMediaType - Unknown eMediaType: %d", eMediaType));
            return RV_ERROR_BADPARAM;
        }
    }

}

/***************************************************************************
 * MsgUtilsEncodeMediaSubType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media sub type to the page
 *          (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMediaSubType   - Value of the media sub type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaSubTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaSubTypeOffset is.
 *        strMediaSubTypeOffset - In case that eMediaSubType is
 *                       RVSIP_MEDIASUBTYPE_OTHER, this string offset will
 *                       contain the media sub type value. else, it will
 *                       be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMediaSubType(
                                     IN MsgMgr*         pMsgMgr,
                                     IN  HRPOOL             hPool,
                                     IN  HPAGE              hPage,
                                     IN  RvSipMediaSubType  eMediaSubType,
                                     IN  HRPOOL             stringPool,
                                     IN  HPAGE              stringPage,
                                     IN  RvInt32           strMediaSubTypeOffset,
                                     OUT RvUint32*         length)
{
    switch (eMediaSubType)
    {
    case RVSIP_MEDIASUBTYPE_PLAIN:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "plain", length);
    case RVSIP_MEDIASUBTYPE_SDP:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "sdp", length);
    case RVSIP_MEDIASUBTYPE_ISUP:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ISUP", length);
    case RVSIP_MEDIASUBTYPE_QSIG:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "QSIG", length);
    case RVSIP_MEDIASUBTYPE_MIXED:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "mixed", length);
    case RVSIP_MEDIASUBTYPE_ALTERNATIVE:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "alternative", length);
    case RVSIP_MEDIASUBTYPE_DIGEST:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "digest", length);
    case RVSIP_MEDIASUBTYPE_RFC822:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "rfc822", length);
    case RVSIP_MEDIASUBTYPE_3GPP_IMS_XML:
		return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3gpp-ims+xml", length); 
    case RVSIP_MEDIASUBTYPE_PIDF_XML:
		return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "pidf+xml", length); 
	case RVSIP_MEDIASUBTYPE_PARTIAL_PIDF_XML:
		return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "pidf-diff+xml", length); 
	case RVSIP_MEDIASUBTYPE_WATCHERINFO_XML:
		return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "watcherinfo+xml", length); 
	case RVSIP_MEDIASUBTYPE_RELATED:
		return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "related", length); 
    case RVSIP_MEDIASUBTYPE_CSTA_XML:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "csta+xml", length); 
    case RVSIP_MEDIASUBTYPE_OTHER:
        {
            if(strMediaSubTypeOffset == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strMediaSubTypeOffset,
                                            length);
            }
        }
    case RVSIP_MEDIASUBTYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeMediaSubType - Undefined media sub type"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeMediaSubType - Unknown eMediaSubType: %d", eMediaSubType));
            return RV_ERROR_BADPARAM;
        }
    }

}


/***************************************************************************
 * MsgUtilsEncodeConsecutiveMediaType
 * ------------------------------------------------------------------------
 * General: The function copies the string of the media type on the given
 *          buffer (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strBuffer    - The buffer to copy the media type on.
 *          eMediaType   - Value of the media type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaTypeOffset is.
 *        strMediaTypeOffset - In case that eMediaType is RVSIP_MEDIATYPE_OTHER,
 *                       this string offset will contain the media-type value. else,
 *                       it will be UNDEFINED.
 *        bufferLen    - The length of the given buffer.
 *        bIsSufficient - Is the buffer sufficient.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeConsecutiveMediaType(
                                  IN     MsgMgr*          pMsgMgr,
                                  IN     RvSipMediaType   eMediaType,
                                  IN     HRPOOL           stringPool,
                                  IN     HPAGE            stringPage,
                                  IN     RvInt32         strMediaTypeOffset,
                                  IN     RvUint          bufferLen,
                                  INOUT  RvBool         *bIsSufficient,
                                  INOUT  RvChar        **strBuffer,
                                  OUT    RvUint*         length)
{
    RvInt32  strLen =0;
    RvChar   tempBuffer[15] = {'\0'};
    RvStatus stat;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    switch (eMediaType)
    {
    case RVSIP_MEDIATYPE_TEXT:
        {
            strLen = (RvInt32)strlen("text");
            strcpy(tempBuffer, "text");
            break;
        }
    case RVSIP_MEDIATYPE_IMAGE:
        {
            strLen = (RvInt32)strlen("image");
            strcpy(tempBuffer, "image");
            break;
        }
    case RVSIP_MEDIATYPE_AUDIO:
        {
            strLen = (RvInt32)strlen("audio");
            strcpy(tempBuffer, "audio");
            break;
        }
    case RVSIP_MEDIATYPE_VIDEO:
        {
            strLen = (RvInt32)strlen("video");
            strcpy(tempBuffer, "video");
            break;
        }
    case RVSIP_MEDIATYPE_APPLICATION:
        {
            strLen = (RvInt32)strlen("application");
            strcpy(tempBuffer, "application");
            break;
        }
    case RVSIP_MEDIATYPE_MULTIPART:
        {
            strLen = (RvInt32)strlen("multipart");
            strcpy(tempBuffer, "multipart");
            break;
        }
    case RVSIP_MEDIATYPE_MESSAGE:
        {
            strLen = (RvInt32)strlen("message");
            strcpy(tempBuffer, "message");
            break;
        }
    case RVSIP_MEDIATYPE_OTHER:
        break;
    case RVSIP_MEDIATYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeConsecutiveMediaType - Undefined media type"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeConsecutiveMediaType - Unknown eMediaType: %d", eMediaType));
            return RV_ERROR_BADPARAM;
        }
    }

    if (RVSIP_MEDIATYPE_OTHER == eMediaType)
    {
        if(strMediaTypeOffset == UNDEFINED)
            return RV_ERROR_BADPARAM;
        strLen = RPOOL_Strlen(stringPool, stringPage, strMediaTypeOffset);
        if (0 > strLen)
             return RV_ERROR_UNKNOWN;
        *length += strLen;
        if ((*length <= bufferLen) &&
            (RV_TRUE == *bIsSufficient))
        {
            stat = RPOOL_CopyToExternal(stringPool, stringPage,
                                        strMediaTypeOffset,
                                        *strBuffer, strLen);
            if (stat != RV_OK)
                return stat;
            *strBuffer += strLen;
            return RV_OK;
        }
        else
        {
            *bIsSufficient = RV_FALSE;
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }
    }
    /* At this point, the media type can be found on tempBuffer */
    *length += strLen;
    if ((*length > bufferLen) ||
        (RV_FALSE == *bIsSufficient))
    {
        *bIsSufficient = RV_FALSE;
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    strncpy(*strBuffer, tempBuffer, strLen);
    *strBuffer += strLen;
    return RV_OK;
}

/***************************************************************************
 * MsgUtilsEncodeConsecutiveMediaSubType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media sub type to the given
 *          consecutive buffer (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strBuffer    - The buffer to copy the media sub type on.
 *          eMediaSubType   - Value of the media sub type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaSubTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaSubTypeOffset is.
 *        strMediaSubTypeOffset - In case that eMediaSubType is
 *                       RVSIP_MEDIASUBTYPE_OTHER, this string offset will
 *                       contain the media sub type value. else, it will
 *                       be UNDEFINED.
 *        bufferLen    - The maximum length of the buffer.
 *        bIsSufficient - Is the buffer sufficient.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeConsecutiveMediaSubType(
                                     IN     MsgMgr*             pMsgMgr,
                                     IN     RvSipMediaSubType   eMediaSubType,
                                     IN     HRPOOL              stringPool,
                                     IN     HPAGE               stringPage,
                                     IN     RvInt32            strMediaSubTypeOffset,
                                     IN     RvUint             bufferLen,
                                     IN     RvBool            *bIsSufficient,
                                     INOUT  RvChar           **strBuffer,
                                     OUT    RvUint*            length)
{
    RvInt32  strLen=0;
    RvChar   tempBuffer[25];
    RvStatus stat;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    switch (eMediaSubType)
    {
    case RVSIP_MEDIASUBTYPE_PLAIN:
        {
            strLen = (RvInt32)strlen("plain");
            strcpy(tempBuffer, "plain");
            break;
        }
    case RVSIP_MEDIASUBTYPE_SDP:
        {
            strLen = (RvInt32)strlen("sdp");
            strcpy(tempBuffer, "sdp");
            break;
        }
    case RVSIP_MEDIASUBTYPE_ISUP:
        {
            strLen = (RvInt32)strlen("ISUP");
            strcpy(tempBuffer, "ISUP");
            break;
        }
    case RVSIP_MEDIASUBTYPE_QSIG:
        {
            strLen = (RvInt32)strlen("QSIG");
            strcpy(tempBuffer, "QSIG");
            break;
        }
    case RVSIP_MEDIASUBTYPE_MIXED:
        {
            strLen = (RvInt32)strlen("mixed");
            strcpy(tempBuffer, "mixed");
            break;
        }
    case RVSIP_MEDIASUBTYPE_ALTERNATIVE:
        {
            strLen = (RvInt32)strlen("alternative");
            strcpy(tempBuffer, "alternative");
            break;
        }
    case RVSIP_MEDIASUBTYPE_DIGEST:
        {
            strLen = (RvInt32)strlen("digest");
            strcpy(tempBuffer, "digest");
            break;
        }
    case RVSIP_MEDIASUBTYPE_RFC822:
        {
            strLen = (RvInt32)strlen("rfc822");
            strcpy(tempBuffer, "rfc822");
            break;
        }
    case RVSIP_MEDIASUBTYPE_PIDF_XML:
        {
            strLen = (RvInt32)strlen("pidf+xml");
            strcpy(tempBuffer, "pidf+xml");
            break;
        }
    case RVSIP_MEDIASUBTYPE_PARTIAL_PIDF_XML:
        {
            strLen = (RvInt32)strlen("pidf-diff+xml");
            strcpy(tempBuffer, "pidf-diff+xml");
            break;
        }
    case RVSIP_MEDIASUBTYPE_WATCHERINFO_XML:
        {
            strLen = (RvInt32)strlen("watcherinfo+xml");
            strcpy(tempBuffer, "watcherinfo+xml");
            break;
        }
    case RVSIP_MEDIASUBTYPE_RELATED:
        {
            strLen = (RvInt32)strlen("related");
            strcpy(tempBuffer, "related");
            break;
        }
    case RVSIP_MEDIASUBTYPE_CSTA_XML:
        {
            strLen = (RvInt32)strlen("csta+xml");
            strcpy(tempBuffer, "csta+xml");
            break;
        }
    case RVSIP_MEDIASUBTYPE_OTHER:
        {
            if(strMediaSubTypeOffset == UNDEFINED)
                return RV_ERROR_BADPARAM;
            strLen = RPOOL_Strlen(stringPool, stringPage, strMediaSubTypeOffset);
            if (0 > strLen)
                return RV_ERROR_UNKNOWN;
            *length += strLen;
            if ((*length <= bufferLen) && (RV_TRUE == *bIsSufficient))
            {
                stat = RPOOL_CopyToExternal(stringPool, stringPage,
                    strMediaSubTypeOffset,
                    *strBuffer, strLen);
                if (stat != RV_OK)
                    return stat;
                *strBuffer += strLen;
                return RV_OK;
            }
            else
            {
                *bIsSufficient = RV_FALSE;
                return RV_ERROR_INSUFFICIENT_BUFFER;
            }
        }
    case RVSIP_MEDIASUBTYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeConsecutiveMediaSubType - Undefined media sub type"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeConsecutiveMediaSubType - Unknown eMediaSubType: %d", eMediaSubType));
            return RV_ERROR_BADPARAM;
        }
    }

    /* At this point, the media type can be found on tempBuffer */
    *length += strLen;
    if ((*length > bufferLen) ||
        (RV_FALSE == *bIsSufficient))
    {
        *bIsSufficient = RV_FALSE;
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    strncpy(*strBuffer, tempBuffer, strLen);
    *strBuffer += strLen;
    return RV_OK;
}


/***************************************************************************
 * MsgUtilsEncodeDispositionType
 * ------------------------------------------------------------------------
 * General: The function add the string of the disposition type to the page
 *          (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eDispType    - Value of the disposition type to encode.
 *        stringPool   - Handle of the memory Pool where the strDispOffset is.
 *        stringPage   - Handle of the memory Page where the strDispOffset is.
 *        strDispOffset - In case that eDispType is RVSIP_DISPOSITIONTYPE_OTHER,
 *                       this string offset will contain the disposition type
 *                       value. else, it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDispositionType(
                                        IN MsgMgr*              pMsgMgr,
                                        IN HRPOOL               hPool,
                                        IN HPAGE                hPage,
                                        IN RvSipDispositionType eDispType,
                                        IN HRPOOL               stringPool,
                                        IN HPAGE                stringPage,
                                        IN RvInt32             strDispOffset,
                                        OUT RvUint32*          length)
{
    switch (eDispType)
    {
    case RVSIP_DISPOSITIONTYPE_RENDER:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "render", length);
    case RVSIP_DISPOSITIONTYPE_SESSION:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "session", length);
    case RVSIP_DISPOSITIONTYPE_ICON:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "icon", length);
    case RVSIP_DISPOSITIONTYPE_ALERT:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "alert", length);
    case RVSIP_DISPOSITIONTYPE_SIGNAL:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "signal", length);
    case RVSIP_DISPOSITIONTYPE_OTHER:
        {
            if(strDispOffset == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strDispOffset,
                                            length);
            }
        }
    case RVSIP_DISPOSITIONTYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeDispositionType - Undefined disposition type"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeDispositionType - Unknown eDispType: %d", eDispType));
            return RV_ERROR_BADPARAM;
        }
    }

}

/***************************************************************************
 * MsgUtilsEncodeDispositionHandling
 * ------------------------------------------------------------------------
 * General: The function add the string of the disposition handling parameter
 *          to the page (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eDispHandling   - Value of the disposition handling to encode.
 *        stringPool   - Handle of the memory Pool where the strDispHandlingOffset is.
 *        stringPage   - Handle of the memory Page where the strDispHandlingOffset is.
 *        strDispHandlingOffset - In case that eDispHandling is
 *                       RVSIP_DISPOSITIONHANDLING_OTHER, this string offset will
 *                       contain the transport value. else, it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDispositionHandling(
                                    IN MsgMgr*                  pMsgMgr,
                                    IN HRPOOL                   hPool,
                                    IN HPAGE                    hPage,
                                    IN RvSipDispositionHandling eDispHandling,
                                    IN HRPOOL                   stringPool,
                                    IN HPAGE                    stringPage,
                                    IN RvInt32                 strDispHandlingOffset,
                                    OUT RvUint32*              length)
{
    switch (eDispHandling)
    {
    case RVSIP_DISPOSITIONHANDLING_OPTIONAL:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "optional", length);
    case RVSIP_DISPOSITIONHANDLING_REQUIRED:
        return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "required", length);
    case RVSIP_DISPOSITIONHANDLING_OTHER:
        {
            if(strDispHandlingOffset == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strDispHandlingOffset,
                                            length);
            }
        }
    case RVSIP_DISPOSITIONHANDLING_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeDispositionHandling - Undefined disposition handling"));
            return RV_ERROR_BADPARAM;
        }
    default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeDispositionHandling - Unknown eDispHandling: %d", eDispHandling));
            return RV_ERROR_BADPARAM;
        }
    }

}
#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * MsgUtilsEncodeTransportType
 * ------------------------------------------------------------------------
 * General: The function add the string of the transport to the page, without '/0'.
 *          Note - not good for RVSIP_TRANSPORT_OTHER
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eTransport   - Value of the transport to encode.
 *        stringPool   - Handle of the memory Pool where the strTransport is.
 *        stringPage   - Handle of the memory Page where the strTransport is.
 *        strTransportOffset - In case that eTransport is RVSIP_TRANSPORT_OTHER,
 *                       this string offset will contain the transport value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeTransportType(
                                      IN MsgMgr*        pMsgMgr,
                                      IN HRPOOL         hPool,
                                      IN HPAGE          hPage,
                                      IN RvSipTransport eTransport,
                                      IN HRPOOL         stringPool,
                                      IN HPAGE          stringPage,
                                      IN RvInt32       strTransportOffset,
                                      OUT RvUint32*    length)
{

    switch(eTransport)
    {
        case RVSIP_TRANSPORT_UDP:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "UDP", length);
        }
        case RVSIP_TRANSPORT_TCP:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "TCP", length);
        }
        case RVSIP_TRANSPORT_SCTP:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "SCTP", length);
        }
        case RVSIP_TRANSPORT_TLS:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "TLS", length);
        }
        case RVSIP_TRANSPORT_OTHER:
        {
            if(strTransportOffset > UNDEFINED)
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strTransportOffset,
                                            length);
            }
            else
            {
                return RV_ERROR_BADPARAM;
            }

        }
        case RVSIP_TRANSPORT_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeTransportType - Undefined transport type"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeTransportType - Unknown"));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeCompType
 * ------------------------------------------------------------------------
 * General: The function add the string of the compression to the page, without '/0'.
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eComp        - Value of the compression type to encode.
 *        stringPool   - Handle of the memory Pool where the strTransport is.
 *        stringPage   - Handle of the memory Page where the strTransport is.
 *        strCompOffset - In case that eComp is RVSIP_COMP_OTHER,
 *                       this string offset will contain the comp value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeCompType(
                                 IN MsgMgr*        pMsgMgr,
                                 IN HRPOOL         hPool,
                                 IN HPAGE          hPage,
                                 IN RvSipCompType eComp,
                                 IN HRPOOL         stringPool,
                                 IN HPAGE          stringPage,
                                 IN RvInt32       strCompOffset,
                                 OUT RvUint32*    length)
{
    RvChar *strComp = SipMsgGetCompTypeName(eComp);

    switch(eComp)
    {
        case RVSIP_COMP_SIGCOMP:
        {

            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, strComp, length);
        }
        case RVSIP_COMP_OTHER:
        {
            if(strCompOffset > UNDEFINED)
            {
                return MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            stringPool,
                                            stringPage,
                                            strCompOffset,
                                            length);
            }
            else
            {
                return RV_ERROR_BADPARAM;
            }

        }
        case RVSIP_COMP_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeCompType - Undefined compression type"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeCompType - Unknown"));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeCRLF
 * ------------------------------------------------------------------------
 * General: The function set CRLF on the given page.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage - Handle of the page.
 * Output:length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeCRLF(
                             IN  MsgMgr*    pMsgMgr,
                             IN  HRPOOL     hPool,
                             IN  HPAGE      hPage,
                             OUT RvUint32* length)
{
    RvInt32           offset;
    RvChar              cr = CR;
    RvChar              lf = LF;
    RPOOL_ITEM_HANDLE   ptr;
    RvStatus           stat;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMsgMgr);
#endif

    /* CR */
    stat = RPOOL_AppendFromExternal(hPool,
                                     hPage,
                                    (void*)&cr,
                                    sizeof(cr),
                                    RV_FALSE,
                                    &offset,
                                    &ptr);

    /* LF */
    stat = RPOOL_AppendFromExternal(hPool,
                                    hPage,
                                    (void*)&lf,
                                    sizeof(lf),
                                    RV_FALSE,
                                    &offset,
                                    &ptr);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeCRLF - Failed, AppendFromExternal failed. RvStatus: %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
        return stat;
    }
    else
    {
        *length += sizeof(cr) + sizeof(lf);
        return RV_OK;
    }
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * MsgUtilsEncodeAuthScheme
 * ------------------------------------------------------------------------
 * General: The function add the string of the authentication scheme to the page,
 *          without '/0'.
 *          Note - if the eAuthScheme argument is RVSIP_AUTHENTICATION_SCHEME_OTHER, the function
 *          will set the strAuthScheme value. else, strAuthScheme will be ignored.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPool            - Handle of the memory pool for encoded buffer.
 *          hPage            - Handle of the memory page for encoded buffer.
 *            eAuthAlgorithm   - Value of the authentication algorithm to encode.
 *          sourcePool       - Handle of memory pool of strAuthScheme.
 *          sourcePage       - Handle of memory page of strAuthScheme.
 *          strAuthScheme    - String with authentication scheme in it - when the
 *                             eAuthScheme is RVSIP_AUTHENTICATION_SCHEME_OTHER
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAuthScheme(
                                   IN MsgMgr*                   pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipAuthScheme           eAuthScheme,
                                   IN HRPOOL                    sourcePool,
                                   IN HPAGE                     sourcePage,
                                   IN RvInt32                  strAuthScheme,
                                   OUT RvUint32*               length)

{
    RvStatus status;

    switch(eAuthScheme)
    {
        case RVSIP_AUTH_SCHEME_DIGEST:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "Digest", length);
        }
        case RVSIP_AUTH_SCHEME_OTHER:
        {
            if(strAuthScheme == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                status =  MsgUtilsEncodeString(pMsgMgr,
                                            hPool,
                                            hPage,
                                            sourcePool,
                                            sourcePage,
                                            strAuthScheme,
                                            length);
                return status;
            }
        }
        case RVSIP_AUTH_SCHEME_UNDEFINED:
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeAuthScheme - Undefined auth scheme"));
            return RV_ERROR_BADPARAM;
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAuthScheme - Unknown eAuthScheme: %d", eAuthScheme));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: The function add the string of the authentication algorithm to the page,
 *          without '/0'.
 *          Note - if the eAuthAlgorithm argument is RVSIP_AUTHENTICATION_ALGORITHM_OTHER, the function
 *          will set the strAuthAlgorithm value. else, strAuthalgorithm will be ignored.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPool            - Handle of the memory pool for encoded buffer.
 *          hPage            - Handle of the memory page for encoded buffer.
 *            eAuthAlgorithm   - Value of the authentication algorithm to encode.
 *          sourcePool       - Handle of memory pool of strAuthAlgorithm.
 *          sourcePage       - Handle of memory page of strAuthAlgorithm.
 *          strAuthAlgorithm - String with authentication algorithm in it - when the eAuthAlgorithm is
 *                             RVSIP_AUTHENTICATION_ALGORITHM_OTHER
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAuthAlgorithm(
                                      IN MsgMgr*                   pMsgMgr,
                                      IN HRPOOL                    hPool,
                                      IN HPAGE                     hPage,
                                      IN RvSipAuthAlgorithm        eAuthAlgorithm,
                                      IN HRPOOL                    sourcePool,
                                      IN HPAGE                     sourcePage,
                                      IN RvInt32                  strAuthAlgorithm,
                                      OUT RvUint32*               length)

{
    RvStatus status;

    switch(eAuthAlgorithm)
    {
        case RVSIP_AUTH_ALGORITHM_MD5:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "MD5", length);
        }
        case RVSIP_AUTH_ALGORITHM_OTHER:
        {
            if(strAuthAlgorithm == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              sourcePool,
                                              sourcePage,
                                              strAuthAlgorithm,
                                              length);
                return status;
            }
        }
        case RVSIP_AUTH_ALGORITHM_UNDEFINED:
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "MsgUtilsEncodeAuthAlgorithm - Undefined algorithm"));
            return RV_ERROR_BADPARAM;
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAuthAlgorithm - Unknown eAuthAlgorithm: %d", eAuthAlgorithm));
            return RV_ERROR_BADPARAM;
        }
    }
}


/***************************************************************************
 * MsgUtilsEncodeQopOptions
 * ------------------------------------------------------------------------
 * General: The function add the string of the qop options to the page,
 *          without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 *            eQop   - Value of the qop options to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeQopOptions(
                                   IN MsgMgr*               pMsgMgr,
                                   IN HRPOOL                hPool,
                                   IN HPAGE                 hPage,
                                   IN RvSipAuthQopOption    eQop,
                                   OUT RvUint32*           length)

{
    RvStatus status;

    switch(eQop)
    {
        case RVSIP_AUTH_QOP_AUTH_ONLY:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "auth", length);
            return status;
        }
        case RVSIP_AUTH_QOP_AUTHINT_ONLY:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "auth-int", length);
            return status;
        }
        case RVSIP_AUTH_QOP_AUTH_AND_AUTHINT:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "auth,auth-int", length);
            return status;
        }
        case RVSIP_AUTH_QOP_OTHER:
        {
           return RV_OK;
        }
        case RVSIP_AUTH_QOP_UNDEFINED:
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
               "MsgUtilsEncodeQopOptions - Undefined qop option"));
            return RV_ERROR_BADPARAM;
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeQopOptions - Unknown eQop: %d", eQop));
            return RV_ERROR_BADPARAM;
        }
    }
}
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * MsgUtilsEncodeSubstateVal
 * ------------------------------------------------------------------------
 * General: The function add the string of the substate-val to the Subscription-State
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 *            eQop   - Value of the qop options to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSubstateVal(
                                   IN MsgMgr*                  pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipSubscriptionSubstate eSubstate,
                                   IN HRPOOL                    stringPool,
                                   IN HPAGE                     stringPage,
                                   IN RvInt32                  strSubstate,
                                   OUT RvUint32*               length)

{
    RvStatus status;

    switch(eSubstate)
    {
        case RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "active", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_SUBSTATE_PENDING:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "pending", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "terminated", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_SUBSTATE_OTHER:
        {
            if(strSubstate == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strSubstate,
                                              length);
                return status;
            }
        }
        case RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSubstateVal - Undefined subsState"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSubstateVal - Unknown substate value: %d", eSubstate));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeSubsReason
 * ------------------------------------------------------------------------
 * General: The function add the string of the reason to the Subscription-State
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage   - Handle of the memory page for encoded buffer.
 *            eReason - Value of the qop options to encode.
 *         length   - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSubsReason(
                                   IN MsgMgr*                   pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipSubscriptionReason   eReason,
                                   IN HRPOOL                    stringPool,
                                   IN HPAGE                     stringPage,
                                   IN RvInt32                  strReason,
                                   OUT RvUint32*               length)

{
    RvStatus status;

    switch(eReason)
    {
        case RVSIP_SUBSCRIPTION_REASON_DEACTIVATED:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "deactivated", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_PROBATION:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "probation", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_REJECTED:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "rejected", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_TIMEOUT:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "timeout", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_GIVEUP:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "giveup", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_NORESOURCE:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "noresource", length);
            return status;
        }
        case RVSIP_SUBSCRIPTION_REASON_OTHER:
        {
            if(strReason == UNDEFINED)
                return RV_ERROR_BADPARAM;
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strReason,
                                              length);
                return status;
            }
        }
        case RVSIP_SUBSCRIPTION_REASON_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSubsReason - Undefined subs reason"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSubsReason - Unknown reason value: %d", eReason));
            return RV_ERROR_BADPARAM;
        }
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * MsgUtilsEncodeDiameterProtocolType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Protocol to the page, without '/0'.
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *        eProtocol    - Value of the Protocol to encode.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDiameterProtocolType(
                                      IN MsgMgr*                pMsgMgr,
                                      IN HRPOOL                 hPool,
                                      IN HPAGE                  hPage,
                                      IN RvSipDiameterProtocol  eProtocol,
                                     OUT RvUint32*              length)
{

    switch(eProtocol)
    {
        case RVSIP_DIAMETER_PROTOCOL_DIAMETER:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "diameter", length);
        }
        case RVSIP_DIAMETER_PROTOCOL_RADIUS:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "radius", length);
        }
        case RVSIP_DIAMETER_PROTOCOL_TACACS_PLUS:
        {
            return MsgUtilsEncodeExternalString(pMsgMgr,hPool,hPage, "tacacs+", length);
        }
        case RVSIP_DIAMETER_PROTOCOL_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeDiameterProtocolType - Undefined Protocol type"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeDiameterProtocolType - Unknown"));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeAccessType
 * ------------------------------------------------------------------------
 * General: The function add the string of the AccessType to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eAccessType - enumeration for the access type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAccessType(
                                   IN MsgMgr*                  pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipPAccessNetworkInfoAccessType eAccessType,
                                   IN HRPOOL                    stringPool,
                                   IN HPAGE                     stringPage,
                                   IN RvInt32                  strAccessType,
                                   OUT RvUint32*               length)

{
    RvStatus status;

    switch(eAccessType)
    {
        case RVSIP_ACCESS_TYPE_IEEE_802_11A:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "IEEE-802.11a", length);
            return status;
        }
        case RVSIP_ACCESS_TYPE_IEEE_802_11B:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "IEEE-802.11b", length);
            return status;
        }
        case RVSIP_ACCESS_TYPE_3GPP_GERAN:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP-GERAN", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_3GPP_UTRAN_FDD:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP-UTRAN-FDD", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_3GPP_UTRAN_TDD:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP-UTRAN-TDD", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_3GPP_CDMA2000:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP-CDMA2000", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_ADSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ADSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_ADSL2:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ADSL2", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_ADSL2_PLUS:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ADSL2+", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_RADSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP-RADSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_SDSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "SDSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_HDSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "HDSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_HDSL2:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "HDSL2", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_G_SHDSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "G.SHDSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_VDSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "VDSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_IDSL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "IDSL", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_3GPP2_1X:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP2-1X", length);
            return status;
        }
		case RVSIP_ACCESS_TYPE_3GPP2_1X_HRPD:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "3GPP2-1X-HRPD", length);
            return status;
        }
	    case RVSIP_ACCESS_TYPE_DOCSIS:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "DOCSIS", length);
            return status;
        }
        case RVSIP_ACCESS_TYPE_IEEE_802_11:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "IEEE-802.11", length);
            return status;
        }
        case RVSIP_ACCESS_TYPE_IEEE_802_11G:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "IEEE-802.11g", length);
            return status;
        }
        case RVSIP_ACCESS_TYPE_OTHER:
        {
            if(strAccessType == UNDEFINED)
			{
                return RV_ERROR_BADPARAM;
			}
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strAccessType,
                                              length);
                return status;
            }
        }
        case RVSIP_ACCESS_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAccessType - Undefined strAccessType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAccessType - Unknown AccessType value: %d", eAccessType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodePChargingFunctionAddressesList
 * ------------------------------------------------------------------------
 * General: The function adds the string of the list of ccf or ecf to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage	- Handle of the memory page for encoded buffer.
 *			hList	- handle for the list.
 *			eListType - enumeration for the type of the list.
 *			hListPage - Handle of the memory page for the list.
 * Output: length	- The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodePChargingFunctionAddressesList(
                   IN MsgMgr*									pMsgMgr,
                   IN HRPOOL									hPool,
                   IN HPAGE										hPage,
                   IN RvSipCommonListHandle						hList,
				   IN HRPOOL									hListPool,
                   IN HPAGE										hListPage,
				   IN RvBool									bInUrlHeaders,
				   IN RvSipPChargingFunctionAddressesListType	eListType,
                   OUT RvUint32*								length)

{
    RvStatus								status;
	RvInt									elementType;
	void*									element;
	RvSipCommonListElemHandle				hRelative = NULL;
	MsgPChargingFunctionAddressesListElem*	hElement;
	RvBool									bIsFirstElement = RV_TRUE;

	if(hList == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	/* get the first element from the list */
	status = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
									&elementType, &element, &hRelative);
	while((RV_OK == status) && (NULL != element))
	{
		hElement = (MsgPChargingFunctionAddressesListElem*)element;
		
		if(hElement->strValue > UNDEFINED)
		{
			/* Place a semicolon between the elements */
			if(RV_TRUE == bIsFirstElement)
			{
				bIsFirstElement = RV_FALSE;
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
			}

			/* set ccf/ecf */
			if(RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF == eListType)
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ccf", length);
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ecf", length);
			}

			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetEqualChar(bInUrlHeaders), length);
			
			status = MsgUtilsEncodeString(pMsgMgr,
											hPool,
											hPage,
											hListPool,
											hListPage,
											hElement->strValue,
											length);
		}
		else
		{
			RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
			"MsgUtilsEncodePChargingFunctionAddressesList - Illegal empty element in list."));
			return RV_ERROR_BADPARAM;
		}

		/* get the next element from the list */
		status = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
										&elementType, &element, &hRelative);
	}

	return status;
}

/***************************************************************************
 * MsgUtilsEncodePChargingVectorInfoList
 * ------------------------------------------------------------------------
 * General: The function adds the string of the list of pdp-info or dsl-bearer-info to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage	- Handle of the memory page for encoded buffer.
 *			hList	- handle for the list.
 *			eListType - enumeration for the type of the list.
 *			hListPage - Handle of the memory page for the list.
 * Output: length	- The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodePChargingVectorInfoList(
                                   IN MsgMgr*					pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipCommonListHandle		hList,
								   IN HRPOOL                    hListPool,
                                   IN HPAGE                     hListPage,
								   IN RvBool					bInUrlHeaders,
								   IN RvSipPChargingVectorInfoListType eListType,
                                   OUT RvUint32*				length)

{
    RvStatus						status;
	RvInt							elementType;
	void*							element;
	RvSipCommonListElemHandle		hRelative = NULL;
	MsgPChargingVectorInfoListElem*	hElement;
	RvBool							bIsFirstElement = RV_TRUE;

	if(hList == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	/* get the first element from the list */
	status = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
									&elementType, &element, &hRelative);
	while((RV_OK == status) && (NULL != element))
	{
		hElement = (MsgPChargingVectorInfoListElem*)element;
		
		if(hElement->nItem > UNDEFINED && hElement->strCid > UNDEFINED)
		{
			RvChar strHelper[16];

			/* Place a comma between the elements */
			if(RV_TRUE == bIsFirstElement)
			{
				bIsFirstElement = RV_FALSE;
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetCommaChar(bInUrlHeaders), length);
			}

			/* set pdp-item/dsl-bearer-item */
			if(RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_PDP == eListType)
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "pdp-item", length);
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "dsl-bearer-item", length);
			}

			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetEqualChar(bInUrlHeaders), length);
			
			MsgUtils_itoa(hElement->nItem, strHelper);
			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool, hPage, strHelper, length);
			
			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
			
			/* set pdp-sig/dsl-bearer-sig */
			if(RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_PDP == eListType)
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "pdp-sig", length);
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "dsl-bearer-sig", length);
			}

			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetEqualChar(bInUrlHeaders), length);
			status = MsgUtilsEncodeExternalString(pMsgMgr,
									  hPool,
									  hPage,
									  (hElement->bSig == RV_TRUE ? "yes" : "no"),
									  length);
			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
			
			/* set gcid/dslcid */
			if(RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_PDP == eListType)
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "gcid", length);
			}
			else
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "dslcid", length);
			}

			status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetEqualChar(bInUrlHeaders), length);
			status = MsgUtilsEncodeString(pMsgMgr,
											hPool,
											hPage,
											hListPool,
											hListPage,
											hElement->strCid,
											length);
			/* set flow-id */
			if(hElement->strFlowID > UNDEFINED)
			{
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "flow-id", length);
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, MsgUtilsGetEqualChar(bInUrlHeaders), length);
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "(", length);
				status = MsgUtilsEncodeString(pMsgMgr,
												hPool,
												hPage,
												hListPool,
												hListPage,
												hElement->strFlowID,
												length);
				status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, ")", length);
			}
		}
		else
		{
			RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
			"MsgUtilsEncodePChargingVectorInfoList - Illegal empty element in list."));
			return RV_ERROR_BADPARAM;
		}

		/* get the next element from the list */
		status = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hRelative,
										&elementType, &element, &hRelative);
	}

	return status;
}

/***************************************************************************
 * MsgUtilsEncodeSecurityMechanismType
 * ------------------------------------------------------------------------
 * General: The function add the string of the Mechanism Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityMechanismType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityMechanismType	eMechanismType,
                                   IN HRPOOL						stringPool,
                                   IN HPAGE							stringPage,
                                   IN RvInt32						strMechanismType,
                                   OUT RvUint32*					length)

{
    RvStatus status;

    switch(eMechanismType)
    {
		case RVSIP_SECURITY_MECHANISM_TYPE_DIGEST:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "digest", length);
            return status;
        }
        case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "tls", length);
            return status;
        }
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ipsec-ike", length);
            return status;
        }
		case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ipsec-man", length);
            return status;
        }
		case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ipsec-3gpp", length);
            return status;
        }
        case RVSIP_SECURITY_MECHANISM_TYPE_OTHER:
        {
            if(strMechanismType == UNDEFINED)
			{
                return RV_ERROR_BADPARAM;
			}
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strMechanismType,
                                              length);
                return status;
            }
        }
        case RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityMechanismType - Undefined eMechanismType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityMechanismType - Unknown MechanismType value: %d", eMechanismType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeSecurityAlgorithmType
 * ------------------------------------------------------------------------
 * General: The function add the string of the Algorithm Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityAlgorithmType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityAlgorithmType	eAlgorithmType,
                                   OUT RvUint32*					length)

{
    RvStatus status;

    switch(eAlgorithmType)
    {
		case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "hmac-md5-96", length);
            return status;
        }
        case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "hmac-sha-1-96", length);
            return status;
        }
        case RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityAlgorithmType - Undefined eAlgorithmType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityAlgorithmType - Unknown Algorithm Type value: %d", eAlgorithmType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeSecurityProtocolType
 * ------------------------------------------------------------------------
 * General: The function add the string of the Protocol Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityProtocolType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityProtocolType		eProtocolType,
                                   OUT RvUint32*					length)

{
    RvStatus status;

    switch(eProtocolType)
    {
		case RVSIP_SECURITY_PROTOCOL_TYPE_ESP:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "esp", length);
            return status;
        }
        case RVSIP_SECURITY_PROTOCOL_TYPE_AH:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ah", length);
            return status;
        }
        case RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityProtocolType - Undefined eProtocolType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityProtocolType - Unknown Protocol Type value: %d", eProtocolType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeSecurityModeType
 * ------------------------------------------------------------------------
 * General: The function add the string of the Mode Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityModeType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityModeType			eModeType,
                                   OUT RvUint32*					length)

{
    RvStatus status;

    switch(eModeType)
    {
		case RVSIP_SECURITY_MODE_TYPE_TRANS:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "trans", length);
            return status;
        }
        case RVSIP_SECURITY_MODE_TYPE_TUN:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "tun", length);
            return status;
        }
        case RVSIP_SECURITY_MODE_TYPE_UDP_ENC_TUN:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "UDP-enc-tun", length);
            return status;
        }
		case RVSIP_SECURITY_MODE_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityModeType - Undefined eModeType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityModeType - Unknown Mode Type value: %d", eModeType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeSecurityEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General: The function add the string of the EncryptAlgorithm Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityEncryptAlgorithmType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipSecurityEncryptAlgorithmType	eEncryptAlgorithmType,
                                   OUT RvUint32*						length)

{
    RvStatus status;

    switch(eEncryptAlgorithmType)
    {
		case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "des-ede3-cbc", length);
            return status;
        }
		case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC:
		{
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "aes-cbc", length);
            return status;
        }
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "null", length);
            return status;
        }
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityEncryptAlgorithmType - Undefined eEncryptAlgorithmType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityEncryptAlgorithmType - Unknown Encrypt Algorithm Type value: %d", eEncryptAlgorithmType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeUriCPCType
 * ------------------------------------------------------------------------
 * General: The function add the string of the CPC Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeUriCPCType(
                                   IN MsgMgr*				pMsgMgr,
                                   IN HRPOOL				hPool,
                                   IN HPAGE					hPage,
                                   IN RvSipUriCPCType	    eCPCType,
                                   IN HRPOOL				stringPool,
                                   IN HPAGE					stringPage,
                                   IN RvInt32				strCPCType,
                                   OUT RvUint32*			length)

{
    RvStatus status;

    switch(eCPCType)
    {
		case RVSIP_CPC_TYPE_CELLULAR:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "cellular", length);
            return status;
        }
		case RVSIP_CPC_TYPE_CELLULAR_ROAMING:
		{
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "cellular-roaming", length);
            return status;
        }
        case RVSIP_CPC_TYPE_HOSPITAL:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "hospital", length);
            return status;
        }
        case RVSIP_CPC_TYPE_OPERATOR:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "operator", length);
            return status;
        }
		case RVSIP_CPC_TYPE_ORDINARY:
		{
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "ordinary", length);
            return status;
        }
        case RVSIP_CPC_TYPE_PAYPHONE:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "payphone", length);
            return status;
        }
        case RVSIP_CPC_TYPE_POLICE:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "police", length);
            return status;
        }
		case RVSIP_CPC_TYPE_PRISON:
		{
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "prison", length);
            return status;
        }
        case RVSIP_CPC_TYPE_TEST:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "test", length);
            return status;
        }
        case RVSIP_CPC_TYPE_UNKNOWN:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "unknown", length);
            return status;
        }
        case RVSIP_CPC_TYPE_OTHER:
        {
            if(strCPCType == UNDEFINED)
			{
                return RV_ERROR_BADPARAM;
			}
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strCPCType,
                                              length);
                return status;
            }
        }
        case RVSIP_CPC_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeUriCPCType - Undefined eCPCType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeUriCPCType - Unknown CPC Type value: %d", eCPCType));
            return RV_ERROR_BADPARAM;
        }
    }
}

/***************************************************************************
 * MsgUtilsEncodeAnswerType
 * ------------------------------------------------------------------------
 * General: The function add the string of the AnswerType to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eAnswerType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAnswerType(
                                   IN MsgMgr*                       pMsgMgr,
                                   IN HRPOOL                        hPool,
                                   IN HPAGE                         hPage,
                                   IN RvSipPAnswerStateAnswerType   eAnswerType,
                                   IN HRPOOL                        stringPool,
                                   IN HPAGE                         stringPage,
                                   IN RvInt32                       strAnswerType,
                                   OUT RvUint32*                    length)

{
    RvStatus status;

    switch(eAnswerType)
    {
        case RVSIP_ANSWER_TYPE_CONFIRMED:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "Confirmed", length);
            return status;
        }
        case RVSIP_ANSWER_TYPE_UNCONFIRMED:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "Unconfirmed", length);
            return status;
        }
        case RVSIP_ANSWER_TYPE_OTHER:
        {
            if(strAnswerType == UNDEFINED)
			{
                return RV_ERROR_BADPARAM;
			}
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strAnswerType,
                                              length);
                return status;
            }
        }
        case RVSIP_ANSWER_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAnswerType - Undefined strAnswerType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeAnswerType - Unknown AnswerType value: %d", eAnswerType));
            return RV_ERROR_BADPARAM;
        }
    }
}

 /***************************************************************************
 * MsgUtilsEncodeSessionCaseType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the SessionCaseType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eSessionCaseType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSessionCaseType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipPServedUserSessionCaseType   eSessionCaseType,
                                   OUT RvUint32*						length)
{
    RvStatus status;

    switch(eSessionCaseType)
    {
		case RVSIP_SESSION_CASE_TYPE_ORIG:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "orig", length);
            return status;
        }
        case RVSIP_SESSION_CASE_TYPE_TERM:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "term", length);
            return status;
        }
        case RVSIP_SESSION_CASE_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecuritySessionCaseType - Undefined eSessionCaseType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecuritySessionCaseType - Unknown SessionCase Type value: %d", eSessionCaseType));
            return RV_ERROR_BADPARAM;
        }
    }    
}

/***************************************************************************
 * MsgUtilsEncodeRegistrationStateType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the RegistrationStateType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eRegistrationStateType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeRegistrationStateType(
                                   IN MsgMgr*							    pMsgMgr,
                                   IN HRPOOL							    hPool,
                                   IN HPAGE								    hPage,
                                   IN RvSipPServedUserRegistrationStateType eRegistrationStateType,
                                   OUT RvUint32*						    length)
{
    RvStatus status;

    switch(eRegistrationStateType)
    {
		case RVSIP_REGISTRATION_STATE_TYPE_UNREG:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "unreg", length);
            return status;
        }
        case RVSIP_REGISTRATION_STATE_TYPE_REG:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "reg", length);
            return status;
        }
        case RVSIP_REGISTRATION_STATE_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityRegistrationStateType - Undefined eRegistrationStateType"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeSecurityRegistrationStateType - Unknown RegistrationState Type value: %d", eRegistrationStateType));
            return RV_ERROR_BADPARAM;
        }
    }    
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/***************************************************************************
 * MsgUtilsEncodeOSPSTag
 * ------------------------------------------------------------------------
 * General: The function add the string of the OSPSTag to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeOSPSTag (
                                   IN MsgMgr*			pMsgMgr,
                                   IN HRPOOL			hPool,
                                   IN HPAGE				hPage,
                                   IN RvSipOSPSTagType	eOSPSTag,
								   IN HRPOOL            stringPool,
                                   IN HPAGE             stringPage,
                                   IN RvInt32           strOSPSTag,
                                  OUT RvUint32*			length)
{
    RvStatus status;

    switch(eOSPSTag)
    {
		case RVSIP_P_DCS_OSPS_TAG_TYPE_BLV:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "BLV", length);
            return status;
        }
        case RVSIP_P_DCS_OSPS_TAG_TYPE_EI:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "EI", length);
            return status;
        }
		case RVSIP_P_DCS_OSPS_TAG_TYPE_RING:
        {
            status = MsgUtilsEncodeExternalString(pMsgMgr, hPool,hPage, "RING", length);
            return status;
        }
		case RVSIP_P_DCS_OSPS_TAG_TYPE_OTHER:
        {
            if(strOSPSTag == UNDEFINED)
			{
                return RV_ERROR_BADPARAM;
			}
            else
            {
                status = MsgUtilsEncodeString(pMsgMgr,
                                              hPool,
                                              hPage,
                                              stringPool,
                                              stringPage,
                                              strOSPSTag,
                                              length);
                return status;
            }
        }
        case RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeOSPSTag - Undefined eOSPSTag"));
            return RV_ERROR_BADPARAM;
        }
        default:
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgUtilsEncodeOSPSTag - Unknown OSPS Tag Type value: %d", eOSPSTag));
            return RV_ERROR_BADPARAM;
        }
    }
}		
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
/************************************************************************************
 * MsgUtilsFindCharInPage
 * ----------------------------------------------------------------------------------
 * General: try to finds a specific char in the header.
 * Return Value: RvBool - RV_TRUE if the char was found
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hPool   - handle to the pool to search the char
 *          hPage   - handle to the page to search the char
 *          offset    - offset in the page so we can get the pointer to start check.
 *          length  - the length of the buffer in the page to search in
 *          charToSearch - the char to find
 * output:    newOffset  - the pointer to the char after the "specific char"
 ***********************************************************************************/
RvBool RVCALLCONV MsgUtilsFindCharInPage(IN HRPOOL   hPool,
                                         IN HPAGE    hPage,
                                         IN RvInt32 offset,
                                         IN RvInt32 length,
                                         IN RvChar  charToSearch,
                                         OUT RvInt32 *newOffset)
{
    RvInt32 unfragSize;
    RvInt32 i;
    RvChar* headerToSearch;
    RvInt32 offsetInternal;

    headerToSearch = (RvChar*)RPOOL_GetPtr(hPool,
                                            hPage,
                                            offset);
    for (i=0; i<length ; i=i+unfragSize)
    {

        unfragSize= RPOOL_GetUnfragmentedSize(hPool,
                                              hPage,
                                              offset);


        if(unfragSize > length)
        {
            unfragSize = length;
        }
        /*begin in the byte after the l to try find :*/
        for (offsetInternal=0; offsetInternal<unfragSize; offsetInternal++)
        {
            if (headerToSearch[offsetInternal] == charToSearch)
            {
                *newOffset = offset + offsetInternal + 1;
                return RV_TRUE;
            }

        }
        offset += offsetInternal;
        /* update the pointer to */
        headerToSearch = (RvChar*)RPOOL_GetPtr(hPool,
                                                hPage,
                                                offset);
    }
    return RV_FALSE;
}

#endif /* RV_SIP_PRIMITIVES */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/


