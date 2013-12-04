/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAcceptHeader.h									  *
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

#ifndef RVSIPACCEPTHEADER_H
#define RVSIPACCEPTHEADER_H

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
 * RvSipAcceptHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Accept header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_ACCEPT_MEDIATYPE,
	RVSIP_ACCEPT_MEDIASUBTYPE,
	RVSIP_ACCEPT_QVAL,
	RVSIP_ACCEPT_OTHER_PARAMS,
    RVSIP_ACCEPT_BAD_SYNTAX
}RvSipAcceptHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipAcceptHeaderFieldName
 * ----------------------------
 * This enum defines all the Accept header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_ACCEPT_FIELD_MEDIATYPE        = 0,
	RVSIP_ACCEPT_FIELD_MEDIASUBTYPE     = 1,
	RVSIP_ACCEPT_FIELD_QVAL             = 2,
	RVSIP_ACCEPT_FIELD_OTHER_PARAMS     = 3,
    RVSIP_ACCEPT_FIELD_BAD_SYNTAX       = 4
}RvSipAcceptHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Accept header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Accept 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAcceptHeaderHandle             *hHeader);


/***************************************************************************
 * RvSipAcceptHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Accept Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Accept header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderConstruct(
                                         IN  RvSipMsgMgrHandle				  hMsgMgr,
                                         IN  HRPOOL							  hPool,
                                         IN  HPAGE                            hPage,
                                         OUT RvSipAcceptHeaderHandle         *hHeader);

/***************************************************************************
 * RvSipAcceptHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Accept header object to a destination 
 *          Accept header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Accept header object.
 *    hSource      - Handle to the source Accept header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderCopy
                                    (IN    RvSipAcceptHeaderHandle hDestination,
                                     IN    RvSipAcceptHeaderHandle hSource);

/***************************************************************************
 * RvSipAcceptHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Accept header object to a textual Accept header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Accept header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderEncode(
                                          IN    RvSipAcceptHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipAcceptHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept header into a Accept header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Accept header object.
 *    buffer    - Buffer containing a textual Accept header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderParse(
                                     IN RvSipAcceptHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipAcceptHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Accept header value into an Accept header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAcceptHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Accept header object.
 *    buffer    - The buffer containing a textual Accept header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderParseValue(
                                     IN RvSipAcceptHeaderHandle          hHeader,
                                     IN RvChar*                          buffer);

/***************************************************************************
 * RvSipAcceptHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Accept header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderFix(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAcceptHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Accept header fields are kept in a string format, for
 *          example, the Accept header YYY string. In order to get such a field
 *          from the Accept header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Accept header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAcceptHeaderGetStringLength(
                                      IN  RvSipAcceptHeaderHandle     hHeader,
                                      IN  RvSipAcceptHeaderStringName eStringName);

/***************************************************************************
 * RvSipAcceptHeaderGetMediaType
 * ------------------------------------------------------------------------
 * General: Gets the media type enumeration value. If RVSIP_MEDIATYPE_OTHER
 *          is returned, you can receive the media type string using
 *          RvSipAcceptHeaderGetMediaTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaType RVCALLCONV RvSipAcceptHeaderGetMediaType(
                                   IN RvSipAcceptHeaderHandle   hHeader);

/***************************************************************************
 * RvSipAcceptHeaderGetStrMediaType
 * ------------------------------------------------------------------------
 * General: Copies the media type string from the Content-Type header into
 *          a given buffer. Use this function when the media type enumeration
 *          returned by RvSipAcceptGetMediaType() is RVSIP_MEDIATYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrMediaType(
                                    IN RvSipAcceptHeaderHandle       hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen);

/***************************************************************************
 * RvSipAcceptHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: Sets the media type field in the Content-Type header object.
 *          If the enumeration given by eMediaType is RVSIP_MEDIATYPE_OTHER,
 *          sets the media type string given by strMediaType in the
 *          Content-Type header object. Use RVSIP_MEDIATYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaType - The media type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIATYPE_UNDEFINED is supplied, the
 *               existing media type is removed from the header.
 *    strMediaType - The media type string to be set in the Content-Type header.
 *                (relevant when eMediaType is RVSIP_MEDIATYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetMediaType(
                                 IN  RvSipAcceptHeaderHandle     hHeader,
                                 IN  RvSipMediaType              eMediaType,
                                 IN  RvChar                     *strMediaType);

/***************************************************************************
 * RvSipAcceptHeaderGetMediaSubType
 * ------------------------------------------------------------------------
 * General: Gets the media sub type enumeration value. If
 *          RVSIP_MEDIASUBTYPE_OTHER is returned, you can receive the media
 *          sub type string using RvSipAcceptHeaderGetMediaSubTypeStr().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipMediaSubType RVCALLCONV RvSipAcceptHeaderGetMediaSubType(
                                   IN RvSipAcceptHeaderHandle   hHeader);

/***************************************************************************
 * RvSipAcceptHeaderGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: Copies the media sub type string from the Content-Type header
 *          into a given buffer. Use this function when the media sub type
 *          enumeration returned by RvSipAcceptGetMediaSubType() is
 *          RVSIP_MEDIASUBTYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer
 *          length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrMediaSubType(
                                    IN RvSipAcceptHeaderHandle       hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen);

/***************************************************************************
 * RvSipAcceptHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: Sets the media sub type field in the Content-Type header object.
 *          If the enumeration given by eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER,
 *          sets the media sub type string given by strMediaSubType in the
 *          Content-Type header object. Use RVSIP_MEDIASUBTYPE_OTHER when the
 *          media type you wish to set to the Content-Type does not have a
 *          matching enumeration value in the RvSipMediaSubType enumeration.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Content-Type header object.
 *    eMediaSubType - The media sub type enumeration to be set in the Content-Type
 *               header object. If RVSIP_MEDIASUBTYPE_UNDEFINED is supplied, the
 *               existing media sub type is removed from the header.
 *    strMediaSubType - The media sub type string to be set in the Content-Type
 *               header. (relevant when eMediaSubType is RVSIP_MEDIASUBTYPE_OTHER).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetMediaSubType(
                            IN  RvSipAcceptHeaderHandle      hHeader,
                            IN  RvSipMediaSubType            eMediaSubType,
                            IN  RvChar                      *strMediaSubType);

/***************************************************************************
 * RvSipAcceptHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the QVal string from the Accept header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetQVal(
                                       IN RvSipAcceptHeaderHandle          hHeader,
                                       IN RvChar*                          strBuffer,
                                       IN RvUint                           bufferLen,
                                       OUT RvUint*                         actualLen);

/***************************************************************************
 * RvSipAcceptHeaderSetQVal
 * ------------------------------------------------------------------------
 * General:Sets the QVal string in the Accept header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept header object.
 *    pQVal      - The QVal string to be set in the Accept header. 
 *                 If NULL is supplied, the existing QVal is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetQVal(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar                    *pQVal);

/***************************************************************************
 * RvSipAcceptHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Accept header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Accept header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetOtherParams(
                                       IN RvSipAcceptHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipAcceptHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Accept header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Accept header object.
 *    pText      - The other-params string to be set in the Accept header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetOtherParams(
                                     IN RvSipAcceptHeaderHandle   hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipAcceptHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Accept header,
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
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderGetStrBadSyntax(
                                     IN  RvSipAcceptHeaderHandle  hHeader,
                                     IN  RvChar*				  strBuffer,
                                     IN  RvUint					  bufferLen,
                                     OUT RvUint*				  actualLen);

/***************************************************************************
 * RvSipAcceptHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Accept header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAcceptHeaderSetStrBadSyntax(
                                  IN RvSipAcceptHeaderHandle    hHeader,
                                  IN RvChar*					strBadSyntax);

#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPACCEPTHEADER_H*/
