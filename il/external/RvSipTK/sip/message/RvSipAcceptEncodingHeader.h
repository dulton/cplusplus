/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptEncodingHeader.h									  *
 *                                                                            *
 * The file defines the methods of the Accept-Encoding header object:   	  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Aug 2005                                             *
 ******************************************************************************/

#ifndef RVSIPACCEPTENCODINGHEADER_H
#define RVSIPACCEPTENCODINGHEADER_H

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
 * RvSipAcceptEncodingHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Accept-Encoding header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_ACCEPT_ENCODING_CODING,
	RVSIP_ACCEPT_ENCODING_QVAL,
	RVSIP_ACCEPT_ENCODING_OTHER_PARAMS,
    RVSIP_ACCEPT_ENCODING_BAD_SYNTAX
}RvSipAcceptEncodingHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipAcceptEncodingHeaderFieldName
 * ----------------------------
 * This enum defines all the Accept-Encoding header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_ACCEPT_ENCODING_FIELD_CODING       = 0,
	RVSIP_ACCEPT_ENCODING_FIELD_QVAL         = 1,
	RVSIP_ACCEPT_ENCODING_FIELD_OTHER_PARAMS = 2,
    RVSIP_ACCEPT_ENCODING_FIELD_BAD_SYNTAX   = 3
}RvSipAcceptEncodingHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptEncodingHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept-Encoding header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept-Encoding 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptEncodingHeaderHandle      *hHeader);

/***************************************************************************
 * RvSipAcceptEncodingHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept-Encoding Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept-Encoding header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptEncodingHeaderHandle *hHeader);

/***************************************************************************
 * RvSipAcceptEncodingHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept-Encoding header object to a destination 
 *          AcceptEncoding header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AcceptEncoding header object.
 *    hSource      - Handle to the source AcceptEncoding header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderCopy
                                    (IN    RvSipAcceptEncodingHeaderHandle hDestination,
                                     IN    RvSipAcceptEncodingHeaderHandle hSource);

/***************************************************************************
 * RvSipAcceptEncodingHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept-Encoding header object to a textual Accept-Encoding header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept-Encoding header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderEncode(
                                          IN    RvSipAcceptEncodingHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipAcceptEncodingHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AcceptEncoding header into a AcceptEncoding header object.
 *          All the textual fields are placed inside the object
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AcceptEncoding header object.
 *    buffer    - Buffer containing a textual AcceptEncoding header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderParse(
                                     IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipAcceptEncodingHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept-Encoding header value into an Accept-Encoding header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptEncodingHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept-Encoding header object.
 *    buffer    - The buffer containing a textual AcceptEncoding header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderParseValue(
                                     IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN RvChar*                          buffer);

/***************************************************************************
 * RvSipAcceptEncodingHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AcceptEncoding header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderFix(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AcceptEncoding header fields are kept in a string format, for
 *          example, the AcceptEncoding header YYY string. In order to get such a field
 *          from the AcceptEncoding header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AcceptEncoding header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptEncodingHeaderGetStringLength(
                                      IN  RvSipAcceptEncodingHeaderHandle     hHeader,
                                      IN  RvSipAcceptEncodingHeaderStringName eStringName);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetCoding
 * ------------------------------------------------------------------------
 * General: Copies the Coding string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Encoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetCoding(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen);

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetCoding
 * ------------------------------------------------------------------------
 * General:Sets the Coding string in the Accept-Encoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptEncoding header object.
 *    pCoding    - The Coding string to be set in the Accept-Encoding header. 
 *                 If NULL is supplied, the existing Coding is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetCoding(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                           *pCoding);

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept-Encoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetQVal(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen);

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept-Encoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept-Encoding header object.
 *    pQVal      - The QVal string to be set in the Accept-Encoding header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetQVal(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                           *pQVal);

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the AcceptEncoding header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AcceptEncoding header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetOtherParams(
                                       IN RvSipAcceptEncodingHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the AcceptEncoding header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AcceptEncoding header object.
 *    pText      - The other-params string to be set in the AcceptEncoding header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetOtherParams(
                                     IN RvSipAcceptEncodingHeaderHandle   hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipAcceptEncodingHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AcceptEncoding header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptEncodingHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipAcceptEncodingHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AcceptEncoding header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptEncodingHeaderSetStrBadSyntax(
                                  IN RvSipAcceptEncodingHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax);


#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPACCEPTENCODINGHEADER_H*/
