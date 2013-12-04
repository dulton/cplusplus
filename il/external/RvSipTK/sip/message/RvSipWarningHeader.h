/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipWarningHeader.h									  *
 *                                                                            *
 * The file defines the methods of the Warning header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Mar 2005                                             *
 ******************************************************************************/

#ifndef RVSIPWARNINGHEADER_H
#define RVSIPWARNINGHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/


/*
 * RvSipWarningHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Warning header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_WARNING_WARN_AGENT,
	RVSIP_WARNING_WARN_TEXT,
    RVSIP_WARNING_BAD_SYNTAX
}RvSipWarningHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipWarningHeaderFieldName
 * ----------------------------
 * This enum defines all the Warning header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_WARNING_FIELD_WARN_CODE        = 0,
	RVSIP_WARNING_FIELD_WARN_AGENT       = 1,
	RVSIP_WARNING_FIELD_WARN_TEXT        = 2,
	RVSIP_WARNING_FIELD_BAD_SYNTAX       = 3
}RvSipWarningHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipWarningHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Warning header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Warning
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipWarningHeaderHandle             *hHeader);

/***************************************************************************
 * RvSipWarningHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Warning Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipWarningHeaderHandle *hHeader);

/***************************************************************************
 * RvSipWarningHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Warning header object to a destination
 *          Warning header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Warning header object.
 *    hSource      - Handle to the source Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderCopy
                                    (IN    RvSipWarningHeaderHandle hDestination,
                                     IN    RvSipWarningHeaderHandle hSource);

/***************************************************************************
 * RvSipWarningHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Warning header object to a textual Warning header. The
 *          textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Warning header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderEncode(
                                          IN    RvSipWarningHeaderHandle   hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipWarningHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Warning header into a Warning header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Warning header object.
 *    buffer    - Buffer containing a textual Warning header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderParse(
                                     IN RvSipWarningHeaderHandle hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipWarningHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Warning header value into an Warning header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipWarningHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Warning header object.
 *    buffer    - The buffer containing a textual Warning header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderParseValue(
                                     IN RvSipWarningHeaderHandle hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipWarningHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Warning header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderFix(
                                     IN RvSipWarningHeaderHandle  hHeader,
                                     IN RvChar*                   pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipWarningHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Warning header fields are kept in a string format, for
 *          example, the Warning header warn-text string. In order to get such a field
 *          from the Warning header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Warning header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipWarningHeaderGetStringLength(
                                      IN  RvSipWarningHeaderHandle     hHeader,
                                      IN  RvSipWarningHeaderStringName eStringName);

/***************************************************************************
 * RvSipWarningHeaderGetWarnCode
 * ------------------------------------------------------------------------
 * General: Gets the warn-code value from the Warning Header object.
 * Return Value: Returns the warn-code value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Warning header object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipWarningHeaderGetWarnCode(
                                    IN RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * RvSipWarningHeaderSetWarnCode
 * ------------------------------------------------------------------------
 * General: Sets the warn-code value in the Warning Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Warning header object.
 *    warnCode          - The warn-code value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnCode(
                                 IN    RvSipWarningHeaderHandle hHeader,
                                 IN    RvInt16                  warnCode);

/***************************************************************************
 * RvSipWarningHeaderGetWarnAgent
 * ------------------------------------------------------------------------
 * General: Copies the Warn-Agent string from the Warning header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Warning header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetWarnAgent(
                                       IN RvSipWarningHeaderHandle hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipWarningHeaderSetWarnAgent
 * ------------------------------------------------------------------------
 * General:Sets the Warn-Agent string in the Warning header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Warning header object.
 *    pPseudonym - The pseudonym string to be set in the Warning header.
 *                 If NULL is supplied, the existing pseudonym is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnAgent(
                                     IN RvSipWarningHeaderHandle   hHeader,
                                     IN RvChar                    *pWarnAgent);

/***************************************************************************
 * RvSipWarningHeaderGetWarnText
 * ------------------------------------------------------------------------
 * General: Copies the warn-text string from the Warning header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Warning header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetWarnText(
                                       IN RvSipWarningHeaderHandle hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipWarningHeaderSetWarnText
 * ------------------------------------------------------------------------
 * General:Sets the warn-text string in the Warning header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Warning header object.
 *    pWarnText  - The warn-text string to be set in the Warning header.
 *                 If NULL is supplied, the existing warn-text is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetWarnText(
                                     IN RvSipWarningHeaderHandle   hHeader,
                                     IN RvChar                    *pWarnText);

/***************************************************************************
 * RvSipWarningHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Warning header,
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
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetStrBadSyntax(
                                     IN  RvSipWarningHeaderHandle             hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipWarningHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Warning header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetStrBadSyntax(
                                  IN RvSipWarningHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax);

/***************************************************************************
 * RvSipWarningHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Warning header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hWarningHeader  - Handle to the Warning header object.
 *           eStringName - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Warning header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderGetRpoolString(
                             IN    RvSipWarningHeaderHandle      hWarningHeader,
                             IN    RvSipWarningHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                    *pRpoolPtr);

/***************************************************************************
 * RvSipWarningHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Warning header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Warning header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                           new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipWarningHeaderSetRpoolString(
                             IN RvSipWarningHeaderHandle       hHeader,
                             IN RvSipWarningHeaderStringName   eStringName,
                             IN RPOOL_Ptr                      *pRpoolPtr);



#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPWARNINGHEADER_H*/
