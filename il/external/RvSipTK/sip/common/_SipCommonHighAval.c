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
*********************************************************************************
*/

/*********************************************************************************
 *                              <_SipCommonHighAval.h>
 *
 *  The file holds High Availability utils functions to be used in all stack layers.
 *    Author                         Date
 *    ------                        ------
 *   Dikla Dror                   April 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_HIGHAVAL_ON

#include "rvansi.h"
#include "_SipCommonHighAval.h"
#include "_SipCommonUtils.h"
#include "RvSipVersion.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/* The internal and external seperetors are used for creating */
/* a formatted storing buffer for any Stack object            */
/* NOTE: These seperators MUSTN'T contain numeric characters  */
#define HIGH_AVAL_PARAM_INTERNAL_SEP     ":"
#define HIGH_AVAL_PARAM_EXTERNAL_SEP     "\n"
#define STORED_RV_BOOL_FORMAT            "%u"
#define STORED_RV_INT32_FORMAT           "%d"
#define HIGH_AVAL_BUFFER_BOUNDARY_SIGN   "#####"
#define HIGH_AVAL_BUFFER_SEP             "----"
#define HIGH_AVAL_FORMAT_LEN             20
#define HIGH_AVAL_SUFFIX_FORMAT_TEMPLATE "%s %%020d %s"
#define STORED_PARAM_LEN(_paramLen,_paramIDLen)                     \
                        (RvUint32)(_paramLen + _paramIDLen                +   \
                         2*strlen(HIGH_AVAL_PARAM_INTERNAL_SEP) +   \
                         strlen(HIGH_AVAL_PARAM_EXTERNAL_SEP))

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV HighAvalGetCurrentParamLen(
                                  IN  RvChar   *currBufPos,
                                  IN  RvUint32  currBytesLeft,
                                  OUT RvUint32 *pParamLen,
                                  OUT RvUint32 *pParamLenAdd);

static RvStatus RVCALLCONV HighAvalRestoreSingleParam(
									  IN RvChar                      *firstInternalSepPos,
									  IN RvUint32                     paramLen,
									  IN SipCommonHighAvalParameter  *paramsArray,
									  IN RvUint32                     arrayLen,
									  IN RvLogSource                 *pLogSrc);

static SipCommonHighAvalParameter *RVCALLCONV HighAvalGetParamIDArrayPos(
                                        IN RvChar                      *paramIDPos,
                                        IN RvUint32                     paramIDLen,
                                        IN SipCommonHighAvalParameter  *paramsArray,
                                        IN RvUint32                     arrayLen);

static RvStatus RVCALLCONV HighAvalVerifyFormatValidity(
                                        IN RvUint32   paramLen,
                                        IN RvChar    *firstInternalSepPos,
                                        IN RvChar    *secondInternalSepPos,
                                        IN RvChar    *externalSepPos);

static RvStatus RVCALLCONV HighAvalGetPrefixStr(INOUT RvUint32 *pPrefixLen,
                                                OUT   RvChar   *pPrefix);

static RvStatus RVCALLCONV HighAvalGetSuffixStr(IN    RvChar   *pBuff,
                                                IN    RvUint32  buffLen,
                                                INOUT RvUint32 *pSuffixLen,
                                                OUT   RvChar   *pSuffix);

static RvStatus RVCALLCONV HighAvalIsBufferSuffix(
                                        IN  RvChar   *pCurrBufPos,
                                        IN  RvUint32  leftBytes,
                                        OUT RvBool   *pbSuffix);

static RvInt RVCALLCONV HighAvalGetBufferCheckSum(
                                        IN RvChar   *pBuff,
                                        IN RvUint32  buffLen);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipCommonHighAvalGetTotalStoredParamLen
 * ------------------------------------------------------------------------
 * General: The function calculates the total stored param length (including 
 *          a preceding textual length and delimiter characters). The final 
 *          format of a param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  paramLen   - The length of the stored parameter 
 *         paramIdLen - The length of the stored parameter ID
 ***************************************************************************/
RvInt32 RVCALLCONV SipCommonHighAvalGetTotalStoredParamLen(
                                            IN RvUint32 paramLen,
                                            IN RvUint32 paramIDLen)
{
    RvChar   strParamLen[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    /* Add to the length the seperators length */
    RvUint32 currLen = STORED_PARAM_LEN(paramLen,paramIDLen);

    if (paramLen == 0)
    {
        return paramLen;
    }
    /* Add to the sum the length of the param length string */
    RvSprintf(strParamLen,"%d",currLen);
    currLen += (RvUint32)strlen(strParamLen);

    return currLen;
}

/***************************************************************************
 * SipCommonHighAvalGetTotalStoredBoolParamLen
 * ------------------------------------------------------------------------
 * General: The function calculates the total stored BOOL param length  
 *          (including a preceding textual length and delimiter characters).
 *           The final format of a param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  bParam     - The the stored boolean parameter.
 *         paramIdLen - The length of the stored parameter ID
 ***************************************************************************/
RvInt32 RVCALLCONV SipCommonHighAvalGetTotalStoredBoolParamLen(
                                                    IN RvBool   bParam,
                                                    IN RvUint32 paramIDLen)
{
    RvChar strBool[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];    

    RvSprintf(strBool,STORED_RV_BOOL_FORMAT,bParam);

    return SipCommonHighAvalGetTotalStoredParamLen(
                                            (RvUint32)strlen(strBool),paramIDLen);
}

/***************************************************************************
 * SipCommonHighAvalGetTotalStoredInt32ParamLen
 * ------------------------------------------------------------------------
 * General: The function calculates the total stored RvInt32 param length  
 *          (including a preceding textual length and delimiter characters).
 *           The final format of a param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  param      - The the stored RvInt32 parameter.
 *         paramIdLen - The length of the stored parameter ID
 ***************************************************************************/
RvInt32 RVCALLCONV SipCommonHighAvalGetTotalStoredInt32ParamLen(
                                                    IN RvInt32  param,
                                                    IN RvUint32 paramIDLen)
{
    RvChar strInt32[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];

    RvSprintf(strInt32,STORED_RV_INT32_FORMAT,param);

    return SipCommonHighAvalGetTotalStoredParamLen(
                                        (RvUint32)strlen(strInt32),paramIDLen);
}

/***************************************************************************
 * SipCommonHighAvalStoreSingleParamFromPage
 * ------------------------------------------------------------------------
 * General: The function stores an high availability single parameter in a 
 *          buffer, by using a utility function, which take a string from a 
 *          given rpool page, and set it in the buffer. 
 *          After copying the string, the function frees the page.
 *          The function also checks that there is enough space for the string
 *          in the buffer.
 *          The complete format of a stored param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strParamID     - The string that identifies the stored parameter.
 *          hPool          - Handle to the rpool.
 *          hPage          - Handle to the page holding the string.
 *          offset         - The offset of the page to be stored.
 *          bFreePage      - Indicates if the given page should be freed.
 *          maxBuffLen     - Length of the whole given buffer.
 *          encodedLen     - Length of the string need to copy to buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleParamFromPage(
                               IN    RvChar   *strParamID,
                               IN    HRPOOL    hPool,
                               IN    HPAGE     hPage,
                               IN    RvInt32   offset,
                               IN    RvBool    bFreePage,
                               IN    RvInt32   maxBuffLen,
                               IN    RvInt32   encodedLen,
                               INOUT RvChar  **ppCurrBuffPos,
                               INOUT RvUint32 *pCurrBuffLen)
{
    RvStatus rv;

    rv = SipCommonHighAvalStoreSingleParamPrefix(
            strParamID,maxBuffLen,encodedLen,ppCurrBuffPos,pCurrBuffLen);
    if (RV_OK != rv)
    {
        /* Free the hPage no matter if the store action failed */
        if (bFreePage == RV_TRUE)
        {
            RPOOL_FreePage(hPool, hPage);
        }
        return rv;
    }

    RPOOL_CopyToExternal(hPool,hPage,offset,(void*)*ppCurrBuffPos,encodedLen);
    
    *ppCurrBuffPos += encodedLen;    
    *pCurrBuffLen  += encodedLen;

    /* Free the hPage no matter if the store action failed */
    if (bFreePage == RV_TRUE)
    {
        RPOOL_FreePage(hPool, hPage);
    }

    rv = SipCommonHighAvalStoreSingleParamSuffix(maxBuffLen,ppCurrBuffPos,pCurrBuffLen);
    return rv;
}

/***************************************************************************
 * SipCommonHighAvalStoreSingleParamFromStr
 * ------------------------------------------------------------------------
 * General: The function stores an high availability a string parameter
 *          in a buffer.
 *          The function also checks that there is enough space for the string
 *          in the buffer.
 *          The complete format of a stored param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strParamID     - The string that identifies the stored parameter.
 *          strParam       - String that contains the parameter content.
 *          maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleParamFromStr(
                               IN          RvChar   *strParamID,
                               IN    const RvChar   *strParam,
                               IN          RvInt32   maxBuffLen,
                               INOUT       RvChar  **ppCurrBuffPos,
                               INOUT       RvUint32 *pCurrBuffLen)
{
    RvStatus rv;
    RvInt32  paramLen = (RvInt32)strlen(strParam);
    
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            strParamID,maxBuffLen,paramLen,ppCurrBuffPos,pCurrBuffLen);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Copy the parameter content and promote the pointer and counter */
    strncpy(*ppCurrBuffPos,strParam,paramLen);
    *ppCurrBuffPos += paramLen;
    *pCurrBuffLen  += paramLen;

    rv = SipCommonHighAvalStoreSingleParamSuffix(maxBuffLen,ppCurrBuffPos,pCurrBuffLen);
    
    return rv;
}

/***************************************************************************
 * SipCommonHighAvalStoreSingleBoolParam
 * ------------------------------------------------------------------------
 * General: The function stores an high availability a RvBool parameter
 *          in a buffer.
 *          The function also checks that there is enough space for the 
 *          value in the buffer.
 *          The complete format of a stored param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strParamID     - The string that identifies the stored parameter.
 *          bParam         - The stored boolean parameter.
 *          maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleBoolParam(
                                    IN    RvChar   *strParamID,
                                    IN    RvBool    bParam,
                                    IN    RvInt32   maxBuffLen,
                                    INOUT RvChar  **ppCurrBuffPos,
                                    INOUT RvUint32 *pCurrBuffLen)
{
    RvChar  strBool[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    
    RvSprintf(strBool,STORED_RV_BOOL_FORMAT,bParam);

    return SipCommonHighAvalStoreSingleParamFromStr(
                strParamID,strBool,maxBuffLen,ppCurrBuffPos,pCurrBuffLen);
}

/***************************************************************************
 * SipCommonHighAvalStoreSingleInt32Param
 * ------------------------------------------------------------------------
 * General: The function stores an high availability a RvInt32 parameter
 *          in a buffer.
 *          The function also checks that there is enough space for the 
 *          value in the buffer.
 *          The complete format of a stored param is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strParamID     - The string that identifies the stored parameter.
 *          param          - The RvInt32 stored parameter.
 *          maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleInt32Param(
                                    IN    RvChar   *strParamID,
                                    IN    RvInt32   param,
                                    IN    RvInt32   maxBuffLen,
                                    INOUT RvChar  **ppCurrBuffPos,
                                    INOUT RvUint32 *pCurrBuffLen)
{
    RvChar strInt32[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    
    RvSprintf(strInt32,STORED_RV_INT32_FORMAT,param);

    return SipCommonHighAvalStoreSingleParamFromStr(
                strParamID,strInt32,maxBuffLen,ppCurrBuffPos,pCurrBuffLen);
}

/***************************************************************************
 * SipCommonHighAvalStoreSingleParamPrefix
 * ------------------------------------------------------------------------
 * General: The function stores an high availability single param prefix in a 
 *          buffer.
 *          The format of a stored param prefix is:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strParamID     - The string that identifies the stored parameter.
 *          maxBuffLen     - Length of the whole given buffer.
 *          paramLen       - Length of the string need to copy to buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleParamPrefix(
                                IN    RvChar   *strParamID,
                                IN    RvUint32   maxBuffLen,
                                IN    RvInt32   paramLen,
                                INOUT RvChar  **ppCurrBuffPos,
                                INOUT RvUint32 *pCurrBuffLen)
{
    RvInt32 currStoredChars = 0;
    RvUint32 totalLen       = STORED_PARAM_LEN(paramLen,strlen(strParamID));

    if ((*pCurrBuffLen + totalLen) > maxBuffLen)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    currStoredChars = RvSprintf(*ppCurrBuffPos,
                                "%d%s%s%s",
                                totalLen,
                                HIGH_AVAL_PARAM_INTERNAL_SEP,
                                strParamID,
                                HIGH_AVAL_PARAM_INTERNAL_SEP);

    /* Promote the buffer position pointer and update the current length */
    *ppCurrBuffPos += currStoredChars;
    *pCurrBuffLen  += currStoredChars;

    return RV_OK;
}       

/***************************************************************************
 * SipCommonHighAvalStoreSingleParamSuffix
 * ------------------------------------------------------------------------
 * General: The function stores an high availability single param suffix in 
 *          a buffer.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreSingleParamSuffix(
                                IN    RvInt32   maxBuffLen,
                                INOUT RvChar  **ppCurrBuffPos,
                                INOUT RvUint32 *pCurrBuffLen)
{
    RvInt32 suffixLen = (RvInt32)strlen(HIGH_AVAL_PARAM_EXTERNAL_SEP);
    
    if ((RvInt32)(*pCurrBuffLen + suffixLen) > maxBuffLen)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    /* Use RvSnprintf in order to avoid the writing of */
    /* unnecessary terminating '\0' */
    strncpy(*ppCurrBuffPos,HIGH_AVAL_PARAM_EXTERNAL_SEP,suffixLen); 

    /* Promote the buffer position pointer and update the current length */
    *ppCurrBuffPos += suffixLen;
    *pCurrBuffLen  += suffixLen;

    return RV_OK;
}       

/***************************************************************************
 * SipCommonHighAvalGetGeneralStoredBuffPrefixLen
 * ------------------------------------------------------------------------
 * General: The function calculates the length of the general stored buffer
 *          header that contains an identification of the HighAval version.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Output: pPrefixLen - The prefix length.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalGetGeneralStoredBuffPrefixLen(
                                                OUT RvInt32 *pPrefixLen)
{
    RvChar   strPrefix[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    RvUint32 prefixLen = SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN;
    RvStatus rv;
    
    rv = HighAvalGetPrefixStr(&prefixLen,strPrefix);
    if (rv != RV_OK)
    {
        return rv;
    }

    *pPrefixLen = (RvInt32)prefixLen;

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalStoreGeneralPrefix
 * ------------------------------------------------------------------------
 * General: The function stores a general buffer header that
 *          contains an identification of the HighAval version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreGeneralPrefix(
                                     IN    RvInt32   maxBuffLen,
                                     INOUT RvChar  **ppCurrBuffPos,
                                     INOUT RvUint32 *pCurrBuffLen)
{
    RvUint32 prefixLen = maxBuffLen - *pCurrBuffLen;
    RvStatus rv;

    rv = HighAvalGetPrefixStr(&prefixLen,*ppCurrBuffPos);
    if (rv != RV_OK)
    {
        return rv;
    }
    *ppCurrBuffPos += prefixLen;
    *pCurrBuffLen  += prefixLen;

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalRestoreGeneralPrefix
 * ------------------------------------------------------------------------
 * General: The function restores a general buffer header that
 *          contains an identification of the HighAval version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  buffLen        - The left bytes number in the restoration buffer
 * InOut:  ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 * Output: pPrefixLen     - The found prefix length.
 *         pbUseOldVer    - Indicates if the buffer was stored, using an old
 *                         version of storing procedure. 
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalRestoreGeneralPrefix(
                                     IN    RvInt32    buffLen,
                                     INOUT RvChar   **ppCurrBuffPos,
                                     OUT   RvUint32  *pPrefixLen,
                                     OUT   RvBool    *pbUseOldVer)
{
    RvInt32 prefixLen;
    RvChar *pPrefixPos;

    *pPrefixLen = 0;

     /* If the first bytes don't equal HIGH_AVAL_BUFFER_BOUNDARY_SIGN   */
     /* the given buffer was stored by old version of storing procedure */
    if (strncmp(*ppCurrBuffPos,
                HIGH_AVAL_BUFFER_BOUNDARY_SIGN,
                strlen(HIGH_AVAL_BUFFER_BOUNDARY_SIGN))!= 0)
    {
        *pbUseOldVer = RV_TRUE;
        
        return RV_OK;
    }

    *pbUseOldVer = RV_FALSE;
    pPrefixPos   = SipCommonFindStrInMemBuff(*ppCurrBuffPos,buffLen,HIGH_AVAL_BUFFER_SEP);
    if (pPrefixPos == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    /* Calculate the prefix length by relative position, the added */
    /* expression sizeof(RvChar) is the length of the terminating  */
    /* char '\n' in the end of the prefix */
    prefixLen       = (RvInt32)(strlen(HIGH_AVAL_BUFFER_SEP) +
                                sizeof(RvChar) + 
                                pPrefixPos     - 
                                *ppCurrBuffPos); 
    *ppCurrBuffPos += prefixLen;
    *pPrefixLen    += prefixLen;
    
    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalGetGeneralStoredBuffSuffixLen
 * ------------------------------------------------------------------------
 * General: The function calculates the length of the general stored buffer
 *          suffix.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Output: pSuffixLen - The suffix length.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalGetGeneralStoredBuffSuffixLen(
                                                OUT RvInt32 *pSuffixLen)
{
    RvStatus rv;
    RvChar   strDummySuffix[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    RvUint32 dummySuffixLen = SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN;

    /* Create a Dummy suffix in order to get an estimation of */
    /* the required length */
    rv = HighAvalGetSuffixStr(NULL,0,&dummySuffixLen,strDummySuffix);
    if (rv != RV_OK)
    {
        return rv;
    }

    *pSuffixLen = dummySuffixLen;

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalStoreGeneralSuffix
 * ------------------------------------------------------------------------
 * General: The function stores a general buffer suffix.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   maxBuffLen     - Length of the whole given buffer.
 * InOut:   ppCurrBuffPos  - An updated pointer to the end of allocation in the buffer.
 *          pCurrBuffLen   - An updated integer, with the size of so far allocation.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalStoreGeneralSuffix(
                                     IN    RvInt32   maxBuffLen,
                                     INOUT RvChar  **ppCurrBuffPos,
                                     INOUT RvUint32 *pCurrBuffLen)
{
    RvUint32 suffixLen = maxBuffLen - *pCurrBuffLen; /* The max suffix length */
    RvStatus rv;

    rv = HighAvalGetSuffixStr(
            *ppCurrBuffPos-*pCurrBuffLen, /* The beginning of the buffer not    */
                                          /* including the prefix               */
            *pCurrBuffLen,                /* The length of the buffer until now */
            &suffixLen,                   /* The written suffix length          */
            *ppCurrBuffPos);              /* Where to add the suffix string     */ 
    if (rv != RV_OK)
    {
        return rv;
    }
    
    *ppCurrBuffPos += suffixLen;
    *pCurrBuffLen  += suffixLen;

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalRestoreFromBuffer
 * ------------------------------------------------------------------------
 * General: The function restores an object data by a memory buffer 
 *          according to the paramArray, which include set of parameter IDs,
 *          pointers to the restored objects and references to the utility
 *          functions that are used for setting the parameters restored 
 *          data in the suitable object members. 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strBuff     - Pointer to the buffer that includes the restored data.
 *        buffLen     - The length of the buffer. 
 *        paramsArray - The parameters array that relates a parameter ID with
 *                      a target object and suitbale 'set' function.
 *        arrayLen    - The number of elements in the paramsArray.
 *		  pLogSrc     - The module log-id. Used for printing messages.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalRestoreFromBuffer(
                            IN RvChar                      *strBuff,
                            IN RvUint32                     buffLen,
                            IN SipCommonHighAvalParameter  *paramsArray,
                            IN RvUint32                     arrayLen,
							IN RvLogSource                 *pLogSrc)
{
    RvChar   *currBufPos   = strBuff;
    RvUint32  currLen      = 0;
    RvUint32  currParamLen; 
    RvUint32  currParamLenAdd;
    RvBool    bIsSuffix;
    RvUint32  leftBuffBytes;
    RvStatus  rv;

    while (currLen < buffLen)
    {
        leftBuffBytes = buffLen - currLen; 
        /* Check if the end of the buffer is reached */
        rv = HighAvalIsBufferSuffix(currBufPos,leftBuffBytes,&bIsSuffix);
        if (rv != RV_OK)
        {
			RvLogError(pLogSrc,(pLogSrc,
				"SipCommonHighAvalRestoreFromBuffer - failed in HighAvalIsBufferSuffix"));
            return rv;
        }
		if (bIsSuffix == RV_TRUE) { return RV_OK; }

        rv = HighAvalGetCurrentParamLen(currBufPos,
                                        leftBuffBytes,
                                        &currParamLen,
                                        &currParamLenAdd);
        if (rv != RV_OK)
        {
			RvLogError(pLogSrc,(pLogSrc,
				"SipCommonHighAvalRestoreFromBuffer - failed in HighAvalGetCurrentParamLen"));
            return rv;
        }
        currBufPos += currParamLenAdd;
        currLen    += currParamLenAdd;

        rv = HighAvalRestoreSingleParam(currBufPos,currParamLen,paramsArray,arrayLen,pLogSrc);
        if (rv != RV_OK)
        {
			RvLogError(pLogSrc,(pLogSrc,
				"SipCommonHighAvalRestoreFromBuffer - failed in HighAvalRestoreSingleParam"));
            return rv;
        }
        currBufPos += currParamLen;
        currLen    += currParamLen;
    }

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalPrepareParamBufferForParse
 * ------------------------------------------------------------------------
 * General: The function prepares the parameter value string, which is part
 *          of a buffer, for parsing process.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pParamBuff  - Pointer to the parameter string value in a buffer.
 *         paramLen    - The length of the parameter string value.
 * Output: pTermChar   - The parameter value original terminating character
 *                       to be restored after parsing. 
 ***************************************************************************/
void RVCALLCONV SipCommonHighAvalPrepareParamBufferForParse(
                                                IN  RvChar   *pParamBuff,
                                                IN  RvUint32  paramLen,
                                                OUT RvChar   *pTermChar)
{
    RvChar *pTermBuffPos = pParamBuff + paramLen;

    *pTermChar    = *pTermBuffPos;
    *pTermBuffPos = '\0';
}

/***************************************************************************
 * SipCommonHighAvalRestoreParamBufferAfterParse
 * ------------------------------------------------------------------------
 * General: The function restores a parameter value string, which is part
 *          of a buffer, after parsing process.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  origTermChar  - The parameter value original terminating character
 *                         to be restored after parsing.
 *         paramLen      - The length of the parameter string value.
 * InOut:  pParamBuff    - Pointer to the parameter string value in a buffer.
 ***************************************************************************/
void RVCALLCONV SipCommonHighAvalRestoreParamBufferAfterParse(
                                                IN    RvChar   origTermChar,
                                                IN    RvUint32 paramLen,
                                                INOUT RvChar  *pParamBuff)
{
    RvChar *pTermBuffPos = pParamBuff + paramLen;

    *pTermBuffPos = origTermChar;
}

/***************************************************************************
 * SipCommonHighAvalRestoreBoolParam
 * ------------------------------------------------------------------------
 * General: The function restores a RvBool parameter value string in
 *          the pParamObj.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pParamID      - The ID of the configured parameter.
 *        paramIDLen    - The length of the parameter ID.
 *        pParamValue   - The value to be set in the paramObj data member. 
 *        paramValueLen - The length of the parameter value.
 * InOut: pParamObj     - A reference to a boolean object that is
 *                        affected by the parameter value.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalRestoreBoolParam(
                                           IN    RvChar   *pParamID,
                                           IN    RvUint32  paramIDLen,
                                           IN    RvChar   *pParamValue,
                                           IN    RvUint32  paramValueLen,
                                           INOUT void     *pParam)
{
    RvChar  termChar;
    RvBool *pbParam = (RvBool *)pParam;

    SipCommonHighAvalPrepareParamBufferForParse(pParamValue,paramValueLen,&termChar);
    RvSscanf(pParamValue,STORED_RV_BOOL_FORMAT,pbParam);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,paramValueLen,pParamValue);

    RV_UNUSED_ARG(pParamID);
    RV_UNUSED_ARG(paramIDLen);
    
    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalRestoreInt32Param
 * ------------------------------------------------------------------------
 * General: The function restores a Int32 parameter value string in
 *          the pParamObj.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pParamID      - The ID of the configured parameter.
 *        paramIDLen    - The length of the parameter ID.
 *        pParamValue   - The value to be set in the paramObj data member. 
 *        paramValueLen - The length of the parameter value.
 * InOut: pParamObj     - A reference to a boolean object that is
 *                        affected by the parameter value.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalRestoreInt32Param(
                                           IN    RvChar   *pParamID,
                                           IN    RvUint32  paramIDLen,
                                           IN    RvChar   *pParamValue,
                                           IN    RvUint32  paramValueLen,
                                           INOUT void     *pParam)
{
    RvChar   termChar;
    RvInt32 *pIntParam = (RvInt32 *)pParam;

    SipCommonHighAvalPrepareParamBufferForParse(pParamValue,paramValueLen,&termChar);
    RvSscanf(pParamValue,STORED_RV_INT32_FORMAT,pIntParam);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,paramValueLen,pParamValue);

    RV_UNUSED_ARG(pParamID);
    RV_UNUSED_ARG(paramIDLen);
    
    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalIsValidBuffer
 * ------------------------------------------------------------------------
 * General: The function checks if the given storage buffer is correct by 
 *          checking the checksum validity in the end of the buffer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pBuff   - Pointer to the checked buffer.
 *        buffLen - The buffer length.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalIsValidBuffer(
                                           IN   RvChar    *pBuff,
                                           IN   RvUint32   buffLen)
{
    RvChar  *pSepPos; 
    RvChar   suffixFormat[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    RvUint32 checkSumBuffLen;
    RvInt    bufferCheckSum;
    RvInt    actaulCheckSum;
    
    /* Looking for the HIGH_AVAL_BUFFER_SEP from the end of the buffer */
    /* This string start the suffix section of the storage buffer.     */
    pSepPos = SipCommonReverseFindStrInMemBuff(pBuff,buffLen,HIGH_AVAL_BUFFER_SEP);
    if (pSepPos == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    RvSnprintf(suffixFormat,
               SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN,
               HIGH_AVAL_SUFFIX_FORMAT_TEMPLATE,
               HIGH_AVAL_BUFFER_SEP,
               HIGH_AVAL_BUFFER_BOUNDARY_SIGN);

    if (RvSscanf(pSepPos,suffixFormat,&bufferCheckSum) <= 0)
    {
        return RV_ERROR_BADPARAM;
    }

 
    /* The checksum doesn't include the suffix itself */
    checkSumBuffLen = (RvUint32)(pSepPos - pBuff);
    actaulCheckSum  = HighAvalGetBufferCheckSum(pBuff,checkSumBuffLen);

    if (actaulCheckSum != bufferCheckSum)
    {
        return RV_ERROR_BADPARAM;
    }

    return RV_OK;
}

/***************************************************************************
 * SipCommonHighAvalHandleStandAloneBoundaries
 * ------------------------------------------------------------------------
 * General: The function reads the boundaries of the storage buffer and 
 *          check it validity.
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut: ppBuffPos - Pointer to current buffer position.
 *        pBuffLen  - Pointer to the current buffer length.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonHighAvalHandleStandAloneBoundaries(
                                               IN    RvUint32   buffLen,
                                               INOUT RvChar   **ppBuffPos,
                                               OUT   RvUint32  *pPrefixLen,
                                               OUT   RvBool    *pbUseOldVer)
{
    /* The pointer to the buffer is backuped for further use since it's gonna change */
    RvChar    *pBuffBackup = *ppBuffPos; 
    RvStatus   rv;

    *pbUseOldVer = RV_FALSE;

    rv = SipCommonHighAvalRestoreGeneralPrefix(
                    buffLen,ppBuffPos,pPrefixLen,pbUseOldVer);
    if (rv != RV_OK || *pbUseOldVer == RV_TRUE)
    {
        return rv;
    }
    
    /* Verify that the buffer contains the right checksum */
    /* The checksum contains the prefix */
    rv = SipCommonHighAvalIsValidBuffer((RvChar *)pBuffBackup,buffLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * HighAvalGetCurrentParamLen
 * ------------------------------------------------------------------------
 * General: The function retrieves the current restored parameter length
 *          from the given buffer accoring to the storing format:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  currBufPos    - Pointer to the buffer.
 *         currBytesLeft - The number of left bytes in the buffer.
 * Output: pParamLen     - The retrieved parameter length.
 *         pParamLenAdd  - The length of the parameter length field. This
 *                         value has to be taken into account while parsing
 *                         the total storage buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalGetCurrentParamLen(
                                  IN  RvChar   *currBufPos,
                                  IN  RvUint32  currBytesLeft,
                                  OUT RvUint32 *pParamLen,
                                  OUT RvUint32 *pParamLenAdd)
{
    RvChar  lenFormat[HIGH_AVAL_FORMAT_LEN];
    RvChar *internalSepPos = SipCommonFindStrInMemBuff(currBufPos,
                                                       currBytesLeft,
                                                       HIGH_AVAL_PARAM_INTERNAL_SEP);

    /* Initialize the OUT parameter */
    *pParamLen    = 0;

    /* Check if the internal separator was found in a 
       valid range of bytes (in the buffer) */
    *pParamLenAdd = (RvUint32)(internalSepPos - currBufPos);
    if (internalSepPos == NULL || 
        *pParamLenAdd   > currBytesLeft)
    {
        return RV_ERROR_UNKNOWN;
    }
    
    RvSprintf(lenFormat,
              "%%%uu%s",   /* For "%<num_len>u<in_sep>"  */
              *pParamLenAdd,
              HIGH_AVAL_PARAM_INTERNAL_SEP);
    RvSscanf(currBufPos,lenFormat,pParamLen);
    
    if ((*pParamLen + *pParamLenAdd) > currBytesLeft)
    {
        *pParamLen = 0;
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/***************************************************************************
 * HighAvalRestoreSingleParam
 * ------------------------------------------------------------------------
 * General: The function restores single parameter
 *          from the given buffer accoring to the storing format:
 *          <total param len(textual)><internal sep><param ID><internal sep>
 *          <param data><external sep>
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: firstInternalSepPos -
 *                      Pointer to the buffer that includes the restored data.
 *        buffLen     - The length of the buffer. 
 *        paramsArray - The parameters array that relates a parameter ID with
 *                      a target object and suitbale 'set' function.
 *        arrayLen    - The number of elements in the paramsArray.
 *		  pLogSrc     - The module log-id. Used for printing messages.
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalRestoreSingleParam(
                                IN RvChar                      *firstInternalSepPos,
                                IN RvUint32                     paramLen,
                                IN SipCommonHighAvalParameter  *paramsArray,
                                IN RvUint32                     arrayLen,
								IN RvLogSource                 *pLogSrc)
{
    RvChar                     *secondInternalSepPos;
    RvChar                     *externalSepPos;
    RvChar                     *paramIDPos;
    RvUint32                    paramIDLen;
    RvChar                     *paramValuePos;
    RvUint32                    paramValueLen;
    SipCommonHighAvalParameter *pParamArrayPos;
    RvStatus                    rv;

    paramIDPos           = firstInternalSepPos + strlen(HIGH_AVAL_PARAM_INTERNAL_SEP);
    secondInternalSepPos = SipCommonFindStrInMemBuff(paramIDPos,
                                                     (RvUint32)(paramLen - (paramIDPos-firstInternalSepPos)),
                                                     HIGH_AVAL_PARAM_INTERNAL_SEP);
    paramValuePos        = secondInternalSepPos + strlen(HIGH_AVAL_PARAM_INTERNAL_SEP);
    /* The external seperator position is calculated by the paramLen, */
    /* since the parameter might be nested and will contain several   */
    /* HIGH_AVAL_PARAM_INTERNAL_SEP strings */
    externalSepPos       = firstInternalSepPos + paramLen - strlen(HIGH_AVAL_PARAM_EXTERNAL_SEP);

    /* Check the validity of the buffer format */
    rv = HighAvalVerifyFormatValidity(paramLen,
                                      firstInternalSepPos,
                                      secondInternalSepPos,
                                      externalSepPos);
    if (rv != RV_OK)
    {
		RvLogError(pLogSrc,(pLogSrc,
			"HighAvalRestoreSingleParam - failed in HighAvalVerifyFormatValidity"));
        return rv;
    }

    paramIDLen     = (RvUint32)(secondInternalSepPos - paramIDPos);
    paramValueLen  = (RvUint32)(externalSepPos       - paramValuePos);
    pParamArrayPos = HighAvalGetParamIDArrayPos(
                        paramIDPos,paramIDLen,paramsArray,arrayLen);
    if (pParamArrayPos == NULL)
    {
		RvChar origChar       = *secondInternalSepPos; 
		*secondInternalSepPos = '\0'; /* Set '\0' in order to print the parameter name */
		RvLogWarning(pLogSrc,(pLogSrc,
			"HighAvalRestoreSingleParam - found unrecognized parameter=%s",paramIDPos));
		*secondInternalSepPos = origChar;

        return RV_OK;
    }

    if (pParamArrayPos->pfnSetParamValue != NULL)
    {
        rv = pParamArrayPos->pfnSetParamValue(
                                         paramIDPos,
                                         paramIDLen,
                                         paramValuePos,
                                         paramValueLen,
                                         pParamArrayPos->pParamObj);
    }
    else if (pParamArrayPos->subHighAvalParamsArray != NULL)
    {
        rv = SipCommonHighAvalRestoreFromBuffer(
                                         paramValuePos,
                                         paramValueLen,
                                         pParamArrayPos->subHighAvalParamsArray,
                                         pParamArrayPos->paramsArrayLen,
										 pLogSrc);
    }
    else
    {
		RvLogError(pLogSrc,(pLogSrc,
			"HighAvalRestoreSingleParam - the handling function of param=%s is NULL",
			pParamArrayPos->strParamID));
        /* Both of these parameters are NULL - Illegal */
        return RV_ERROR_UNKNOWN;
    }

    return rv;
}

/***************************************************************************
 * HighAvalGetParamIDArrayPos
 * ------------------------------------------------------------------------
 * General: The function retrieves the position of the given paramID in
 *          the input parameters array.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: paramIDPos  - The parameter ID position. 
 *        paramIDLen  - The length of the parameter ID. 
 *        paramsArray - The parameters array that relates a parameter ID with
 *                      a target object and suitbale 'set' function.
 *        arrayLen    - The number of elements in the paramsArray.
 ***************************************************************************/
static SipCommonHighAvalParameter *RVCALLCONV HighAvalGetParamIDArrayPos(
                                        IN RvChar                      *paramIDPos,
                                        IN RvUint32                     paramIDLen,
                                        IN SipCommonHighAvalParameter  *paramsArray,
                                        IN RvUint32                     arrayLen)
{
    RvUint32                    currParamNo    = 0;
    SipCommonHighAvalParameter *pParamArrayPos = paramsArray;

    while (currParamNo < arrayLen)
    {
        if (SipCommonMemBuffExactlyMatchStr(paramIDPos,
                                            paramIDLen,
                                            pParamArrayPos->strParamID) == RV_TRUE)
        {
            return pParamArrayPos;
        }
        /* Move to the next parameter element */
        pParamArrayPos += 1;
        currParamNo    += 1;
    }

    return NULL;
}

/***************************************************************************
 * HighAvalVerifyFormatValidity
 * ------------------------------------------------------------------------
 * General: The function verifes that the given parameter format is valid.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: paramLen   - The length of the parameter. 
 *        firstInternalSepPos  - Position of the 1st internal seperetor.
 *        secondInternalSepPos - Position of the 2nd internal seperetor.
 *        externalSepPos       - Position of the external seperator.
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalVerifyFormatValidity(
                                        IN RvUint32   paramLen,
                                        IN RvChar    *firstInternalSepPos,
                                        IN RvChar    *secondInternalSepPos,
                                        IN RvChar    *externalSepPos)
{
    RvChar *paramEndPos = firstInternalSepPos + paramLen;

    if (firstInternalSepPos  == NULL ||
        secondInternalSepPos == NULL ||
        externalSepPos       == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (firstInternalSepPos  > paramEndPos ||
        secondInternalSepPos > paramEndPos || 
        externalSepPos       > paramEndPos)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Verify that the parameter ends with the HIGH_AVAL_PARAM_EXTERNAL_SEP */
    if (strncmp(HIGH_AVAL_PARAM_EXTERNAL_SEP,
                externalSepPos,
                strlen(HIGH_AVAL_PARAM_EXTERNAL_SEP)) != 0)
    {
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/***************************************************************************
 * HighAvalGetPrefixStr
 * ------------------------------------------------------------------------
 * General: The function copies the prefix into a limited buffer. 
 *
 * Returns: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:   pPrefixLen   - The length of the prefix.
 * Output:  pPrefix      - The buffer, which the prefix is copied to.
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalGetPrefixStr(INOUT RvUint32 *pPrefixLen,
                                                OUT   RvChar   *pPrefix)
{
    RvUint32 totalLen = (RvUint32)strlen(HIGH_AVAL_BUFFER_BOUNDARY_SIGN) + 
                        (RvUint32)strlen(SIP_STACK_VERSION)              +
                        (RvUint32)strlen(HIGH_AVAL_BUFFER_SEP)           +
                        4; /* For the 2 spaces + 1 '\n' in the format + 1 '\0' at the end (not included in the final sum) */
    RvUint32 wroteBytes;
    
    if (*pPrefixLen < totalLen)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    
    wroteBytes = RvSnprintf(pPrefix,
                            totalLen, 
                            "%s %s %s\n",
                            HIGH_AVAL_BUFFER_BOUNDARY_SIGN,
                            SIP_STACK_VERSION,
                            HIGH_AVAL_BUFFER_SEP);

	if (wroteBytes == 0 || wroteBytes != (totalLen-1) /* -1 is for the '\0' which should be included */)
    {
        return RV_ERROR_UNKNOWN;
    }

    *pPrefixLen = wroteBytes;

    return RV_OK;
}

/***************************************************************************
 * HighAvalGetSuffixStr
 * ------------------------------------------------------------------------
 * General: The function copies the suffix into a limited buffer. 
 *
 * Returns: The length of the suffix buffer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuff       - The buffer which the suffix will be added to.
 *                        Its' value might be NULL.
 *          buffLen     - The buffer length.
 * InOut:   pSuffixLen  - The length of the suffix.
 * Output:  pSuffix     - The buffer, which the suffix is copied to.
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalGetSuffixStr(IN    RvChar   *pBuff,
                                                IN    RvUint32  buffLen,
                                                INOUT RvUint32 *pSuffixLen,
                                                OUT   RvChar   *pSuffix)
{
    RvInt    wroteBytes;
    RvInt    checkSum  = 0; 
    RvChar   suffixFormat[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];
    
    /* The buffer might be NULL in case of calling this function */
    /* from SipCommonHighAvalGetGeneralStoredBuffSuffixLen()     */
    if (pBuff != NULL)
    {
        checkSum = HighAvalGetBufferCheckSum(pBuff,buffLen);
    }
    
    RvSnprintf(suffixFormat,
               SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN,
               HIGH_AVAL_SUFFIX_FORMAT_TEMPLATE,
               HIGH_AVAL_BUFFER_SEP,
               HIGH_AVAL_BUFFER_BOUNDARY_SIGN);

    wroteBytes = RvSnprintf(pSuffix,*pSuffixLen,suffixFormat,checkSum);
    if (wroteBytes < 0)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    *pSuffixLen = wroteBytes;

    return RV_OK;
}

/***************************************************************************
 * HighAvalIsBufferSuffix
 * ------------------------------------------------------------------------
 * General: The function checks if the current buffer position points to 
 *          the buffer suffix. 
 *
 * Returns: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCurrBufPos - The current position in the buffer.
 *         leftBytes   - the number of bytes that are left in the buffer.
 * Output: pbSuffix    - Indicates if the current position points to suffix
 ***************************************************************************/
static RvStatus RVCALLCONV HighAvalIsBufferSuffix(
                                        IN  RvChar   *pCurrBufPos,
                                        IN  RvUint32  leftBytes,
                                        OUT RvBool   *pbSuffix)
{
    RvUint32 suffixSepLen = (RvUint32)strlen(HIGH_AVAL_BUFFER_SEP);
    
    if (memcmp(pCurrBufPos,HIGH_AVAL_BUFFER_SEP,suffixSepLen) != 0)
    {
        *pbSuffix = RV_FALSE;  
    }
    else if (suffixSepLen < leftBytes)
    {
        /* If the position points to a suffix string the left bytes */
        /* number must be more than the length of the suffix */
        *pbSuffix = RV_TRUE;
    }
    else
    {
        *pbSuffix = RV_FALSE;  

        return RV_ERROR_BADPARAM;
    }
    return RV_OK;
}

/***************************************************************************
 * HighAvalGetBufferCheckSum
 * ------------------------------------------------------------------------
 * General: The function checks if the current buffer position points to 
 *          the buffer suffix. 
 *
 * Returns: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCurrBufPos - The current position in the buffer.
 *         leftBytes   - the number of bytes that are left in the buffer.
 * Output: pbSuffix    - Indicates if the current position points to suffix
 ***************************************************************************/
static RvInt RVCALLCONV HighAvalGetBufferCheckSum(
                                        IN RvChar   *pBuff,
                                        IN RvUint32  buffLen)
{
    RvChar  *currBufPos = pBuff;
    RvUint32 leftBytes  = buffLen; 
    RvInt    currSum    = 0;
/* do not change this code because of lint warnings!!! */
    while (leftBytes > 0)
    {
        currSum    += (RvInt)*pBuff;
        currBufPos += 1;
        leftBytes  -= 1;
    }

    return currSum;
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */

