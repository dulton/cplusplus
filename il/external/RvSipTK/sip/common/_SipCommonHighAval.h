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
#ifndef SIP_COMMON_HIGH_AVAL_H
#define SIP_COMMON_HIGH_AVAL_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_HIGHAVAL_ON

#include "RV_ADS_DEF.h"
#include "rpool_API.h"
#include "rvlog.h"

/*-----------------------------------------------------------------------*/
/*                                 MACROS                                */
/*-----------------------------------------------------------------------*/
#define SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN 45

/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * HighAvalSetParamValue
 * ------------------------------------------------------------------------
 * General:  Sets the value in pValue to a data member in pParamObj.
 *           The implementation of this function must be type specific:
 *           It will read value from pValue according to its type, cast
 *           pParamObj accordingly, and asign pValue to one of pParamObj 
 *           data members.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pParamID      - The ID of the configured parameter.
 *        paramIDLen    - The length of the parameter ID.
 *        pParamValue   - The value to be set in the paramObj data member. 
 *        paramValueLen - The length of the parameter value.
 * InOut: pParamObj     - A reference to an object that might be 
 *                        affected by the parameter value.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * HighAvalSetParamValue)(
                                           IN    RvChar   *pParamID,
                                           IN    RvUint32  paramIDLen,
                                           IN    RvChar   *pParamValue,
                                           IN    RvUint32  paramValueLen,
                                           INOUT void     *pParamObj);

struct HighAvalParameter; /* Dummy declaration */

/* HighAvalParameter
 * ------------------------------------------------------------------------
 * This structure contains the details required by the high aval restoration
 * procedure for each parameter.
 *
 * strParamID       - The ID of the parameter. This name should appear in
 *                    the high aval buffer in the following format:
 *                <param_len><in_sep><paramID><in_sep><param_content><ex_sep>
 * pParamObj        - This is a reference to an object that might be 
 *                    affected by the parameter value.
 * pfnSetParamValue - This function sets the found parameter value in the
 *                    parameter object (by it's given reference). This 
 *                    struct member is used for plain HighAval parameters.
 * subHighAvalParamsArray - 
 *                    This parameter points to a sub-HighAval parameters
 *                    array. This array is useful in case of nested HighAval
 *                    parameter. 
 * paramsArrayLen         - The number of elements in the sub-HighAval 
 *                          parameters array.
 * NOTE: The parameters 'pfnSetParamValue' and 'subHighAvalParamsArray' 
 *       might be NULL. BUT when one parameter is NULL the other one MUSTN'T be
 *       NULL. These parameters are complementaries of each other.
 */
struct HighAvalParameter {
    RvChar                            *strParamID; 
    void                              *pParamObj;
    HighAvalSetParamValue              pfnSetParamValue;
    struct HighAvalParameter          *subHighAvalParamsArray;
    RvUint32                           paramsArrayLen;
};

typedef struct HighAvalParameter SipCommonHighAvalParameter;

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
                                            IN RvUint32 paramIDLen);

/***************************************************************************
 * SipCommonHighAvalGetTotalStoredBoolParamLen
 * ------------------------------------------------------------------------
 * General: The function calculates the total stored RvBool param length  
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
                                                    IN RvUint32 paramIDLen);

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
                                                    IN RvUint32 paramIDLen);

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
                               INOUT RvUint32 *pCurrBuffLen);

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
                               INOUT       RvUint32 *pCurrBuffLen);

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
                                    INOUT RvUint32 *pCurrBuffLen);

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
                                    INOUT RvUint32 *pCurrBuffLen);

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
                                IN    RvChar    *strParamID,
                                IN    RvUint32   maxBuffLen,
                                IN    RvInt32    paramLen,
                                INOUT RvChar   **ppCurrBuffPos,
                                INOUT RvUint32  *pCurrBuffLen);

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
                                INOUT RvUint32 *pCurrBuffLen);

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
                                                OUT RvInt32 *pPrefixLen);

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
                                     IN    RvInt32    maxBuffLen,
                                     INOUT RvChar   **ppCurrBuffPos,
                                     INOUT RvUint32  *pCurrBuffLen);

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
                                     OUT   RvBool    *pbUseOldVer);

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
                                                OUT RvInt32 *pSuffixLen);

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
                                     INOUT RvUint32 *pCurrBuffLen);

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
							IN RvLogSource                 *pLogSrc);

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
                                                OUT RvChar   *pTermChar);

/***************************************************************************
 * SipCommonHighAvalRestoreParamBufferAfterParse
 * ------------------------------------------------------------------------
 * General: The function restores a parameter value string, which is part
 *          of a buffer, after parsing process.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pOrigTermChar - The parameter value original terminating character
 *                         to be restored after parsing.
 *         paramLen      - The length of the parameter string value.
 * InOut:  pParamBuff    - Pointer to the parameter string value in a buffer.
 ***************************************************************************/
void RVCALLCONV SipCommonHighAvalRestoreParamBufferAfterParse(
                                                IN    RvChar   origTermChar,
                                                IN    RvUint32 paramLen,
                                                INOUT RvChar  *pParamBuff);

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
                                           INOUT void     *pParam);

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
                                           INOUT void     *pParam);

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
                                           IN   RvUint32   buffLen);

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
                                               OUT   RvBool    *pbUseOldVer);
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_COMMON_HIGH_AVAL_H */

