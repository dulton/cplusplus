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
 *                              <ParserUtils.h>
 *
 * This file defines functions which are used from the parser to initialize sip
 * message or sip header.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Michal Mashiach                 DEC 2000
 *********************************************************************************/

#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AdsRpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "RV_SIP_DEF.h"
#include "ParserTypes.h"
#include "_SipParserManager.h"
#include "_SipMsg.h"

#if defined RV_SIP_LIGHT
#include "ParserEngineSipLight.h"
#elif defined RV_SIP_PRIMITIVES
#include "ParserEngineSipPrimitives.h"
#elif defined RV_SIP_JSR32_SUPPORT
#include "ParserEngineJSR32.h"
#elif defined RV_SIP_IMS_HEADER_SUPPORT
#include "ParserEngineIMS.h"
#elif defined RV_SIP_EXTENDED_HEADER_SUPPORT
#include "ParserEngineExtendedHeaders.h"
#elif defined RV_SIP_TEL_URI_SUPPORT
#include "ParserEngineTel.h"
#else
#include "ParserEngineClassic.h"
#endif
	
	

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
                        IN SipParser_pcb_type * pcb);

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
                        IN void * pcb_);
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
void ParserCleanExtParams(INOUT void * extParams);

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
                             OUT RvInt32  *pNum);

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
                             OUT RvInt32  *pNum);

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
                             OUT RvUint32  *pNum);

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
                             OUT RvUint32  *pNum);
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
                             OUT RvInt16  *pNum);
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
                               IN HRPOOL    destPool,
                               IN HPAGE     destPage,
                               IN HRPOOL    sourcePool,
                               IN HPAGE     sourcePage,
                               IN RvInt32  stringOffset,
                               IN RvUint32 stringSize);
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
                                 IN  RvUint32   sizeToCopy);
/***************************************************************************
 * ParserAllocAndSetString
 * ------------------------------------------------------------------------
 * General: The function is for setting the given string (with the NULL in the end)
 *          on a consecutive part of the page.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for sizeToCopy+1 because of that.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool      - Handle to the pool which is used to allocate from.
 *        hPage      - Handle to the page which is used to allocate from.
 *        pData      - The given string.
 *        sizeToCopy - The size of the string.
 * Ouput: AllocationOffset - Pointer to the offset on the page which the
 *                           string was allocated.
 ***************************************************************************/
RvStatus RVCALLCONV ParserAllocFromObjPage(
                                 IN  ParserMgr *pParserMgr,
                                 OUT RvInt32   *AllocationOffset,
                                 IN  HRPOOL      hPool,
                                 IN  HPAGE       hPage,
                                 IN  RvChar     *pData,
                                 IN  RvUint32   sizeToCopy);

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
                                                   IN  RvUint32 stringSize);
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
                     IN    void          *pExtParams);

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
                                           IN  void * pcb_);


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define PARSER_UTILS_H*/
