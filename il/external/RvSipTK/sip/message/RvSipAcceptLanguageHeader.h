/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptLanguageHeader.h									  *
 *                                                                            *
 * The file defines the methods of the Accept-Language header object:   	  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Aug 2005                                             *
 ******************************************************************************/

#ifndef RVSIPACCEPTLANGUAGEHEADER_H
#define RVSIPACCEPTLANGUAGEHEADER_H

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
 * RvSipAcceptLanguageHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Accept-Language header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_ACCEPT_LANGUAGE_LANGUAGE,
	RVSIP_ACCEPT_LANGUAGE_QVAL,
	RVSIP_ACCEPT_LANGUAGE_OTHER_PARAMS,
    RVSIP_ACCEPT_LANGUAGE_BAD_SYNTAX
}RvSipAcceptLanguageHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipAcceptLanguageHeaderFieldName
 * ----------------------------
 * This enum defines all the Accept-Language header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_ACCEPT_LANGUAGE_FIELD_LANGUAGE     = 0,
	RVSIP_ACCEPT_LANGUAGE_FIELD_QVAL         = 1,
	RVSIP_ACCEPT_LANGUAGE_FIELD_OTHER_PARAMS = 2,
    RVSIP_ACCEPT_LANGUAGE_FIELD_BAD_SYNTAX   = 3
}RvSipAcceptLanguageHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptLanguageHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept-Language header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept-Language 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptLanguageHeaderHandle      *hHeader);

/***************************************************************************
 * RvSipAcceptLanguageHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept-Language Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept-Language header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptLanguageHeaderHandle *hHeader);

/***************************************************************************
 * RvSipAcceptLanguageHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept-Language header object to a destination 
 *          AcceptLanguage header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AcceptLanguage header object.
 *    hSource      - Handle to the source AcceptLanguage header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderCopy
                                    (IN    RvSipAcceptLanguageHeaderHandle hDestination,
                                     IN    RvSipAcceptLanguageHeaderHandle hSource);

/***************************************************************************
 * RvSipAcceptLanguageHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept-Language header object to a textual Accept-Language header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept-Language header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderEncode(
                                          IN    RvSipAcceptLanguageHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipAcceptLanguageHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AcceptLanguage header into a AcceptLanguage header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AcceptLanguage header object.
 *    buffer    - Buffer containing a textual AcceptLanguage header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderParse(
                                     IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipAcceptLanguageHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept-Language header value into an Accept-Language header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptLanguageHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept-Language header object.
 *    buffer    - The buffer containing a textual AcceptLanguage header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderParseValue(
                                     IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN RvChar*                          buffer);

/***************************************************************************
 * RvSipAcceptLanguageHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AcceptLanguage header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderFix(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AcceptLanguage header fields are kept in a string format, for
 *          example, the AcceptLanguage header YYY string. In order to get such a field
 *          from the AcceptLanguage header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AcceptLanguage header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptLanguageHeaderGetStringLength(
                                      IN  RvSipAcceptLanguageHeaderHandle     hHeader,
                                      IN  RvSipAcceptLanguageHeaderStringName eStringName);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetLanguage
 * ------------------------------------------------------------------------
 * General: Copies the Language string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Language header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetLanguage(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen);

/***************************************************************************
 * RvSipAcceptLanguageHeaderSetLanguage
 * ------------------------------------------------------------------------
 * General:Sets the Language string in the Accept-Language header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptLanguage header object.
 *    pLanguage    - The Language string to be set in the Accept-Language header. 
 *                 If NULL is supplied, the existing Language is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetLanguage(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                           *pLanguage);

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Language header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetQVal(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen);

/***************************************************************************
 * RvSipAcceptLanguageHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept-Language header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept-Language header object.
 *    pQVal      - The QVal string to be set in the Accept-Language header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetQVal(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                           *pQVal);

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the AcceptLanguage header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AcceptLanguage header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetOtherParams(
                                       IN RvSipAcceptLanguageHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipAcceptLanguageHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the AcceptLanguage header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptLanguage header object.
 *    pText      - The other-params string to be set in the AcceptLanguage header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetOtherParams(
                                     IN RvSipAcceptLanguageHeaderHandle   hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipAcceptLanguageHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AcceptLanguage header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptLanguageHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipAcceptLanguageHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AcceptLanguage header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptLanguageHeaderSetStrBadSyntax(
                                  IN RvSipAcceptLanguageHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax);


#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPACCEPTLANGUAGEHEADER_H*/
