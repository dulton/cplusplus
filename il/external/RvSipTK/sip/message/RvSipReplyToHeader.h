/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReplyToHeader.h									  *
 *                                                                            *
 * The file defines the functions of the Reply-To object:                         *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change parameters.                                                         *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Aug 2005                                             *
 ******************************************************************************/

#ifndef RVSIPREPLYTOHEADER_H
#define RVSIPREPLYTOHEADER_H

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
 * RvSipReplyToHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Reply-To header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_REPLY_TO_DISPLAY_NAME,
	RVSIP_REPLY_TO_OTHER_PARAMS,
    RVSIP_REPLY_TO_BAD_SYNTAX
}RvSipReplyToHeaderStringName;


#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipReplyToHeaderFieldName
 * ----------------------------
 * This enum defines all the Reply-To header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_REPLY_TO_FIELD_ADDR_SPEC    = 0,
    RVSIP_REPLY_TO_FIELD_OTHER_PARMAS = 1,
	RVSIP_REPLY_TO_FIELD_BAD_SYNTAX   = 2
}RvSipReplyToHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReplyToHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an ReplyTo header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed ReplyTo 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipReplyToHeaderHandle             *hHeader);

/***************************************************************************
 * RvSipReplyToHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone ReplyTo Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed ReplyTo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipReplyToHeaderHandle *hHeader);

/***************************************************************************
 * RvSipReplyToHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source ReplyTo header object to a destination 
 *          ReplyTo header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination ReplyTo header object.
 *    hSource      - Handle to the source ReplyTo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderCopy
                                    (IN    RvSipReplyToHeaderHandle hDestination,
                                     IN    RvSipReplyToHeaderHandle hSource);

/***************************************************************************
 * RvSipReplyToHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an ReplyTo header object to a textual ReplyTo header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the ReplyTo header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderEncode(
                                          IN    RvSipReplyToHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipReplyToHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReplyTo header into a ReplyTo header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the ReplyTo header object.
 *    buffer    - Buffer containing a textual ReplyTo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderParse(
                                     IN RvSipReplyToHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipReplyToHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual ReplyTo header value into an ReplyTo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReplyToHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the ReplyTo header object.
 *    buffer    - The buffer containing a textual ReplyTo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderParseValue(
                                     IN RvSipReplyToHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipReplyToHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an ReplyTo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderFix(
                                     IN RvSipReplyToHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReplyToHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the ReplyTo header fields are kept in a string format, for
 *          example, the ReplyTo header YYY string. In order to get such a field
 *          from the ReplyTo header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the ReplyTo header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReplyToHeaderGetStringLength(
                                      IN  RvSipReplyToHeaderHandle     hHeader,
                                      IN  RvSipReplyToHeaderStringName eStringName);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipReplyToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the ReplyTo header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetDisplayName(
                                               IN  RvSipReplyToHeaderHandle   hHeader,
                                               IN  RvChar*                    strBuffer,
                                               IN  RvUint                     bufferLen,
                                               OUT RvUint*                    actualLen);

/***************************************************************************
 * RvSipReplyToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the ReplyTo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the header object.
 *    strDisplayName - The display name to be set in the ReplyTo header. If NULL is supplied, the existing
 *                   display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetDisplayName(
                                             IN    RvSipReplyToHeaderHandle hHeader,
                                             IN    RvChar *                 strDisplayName);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipReplyToHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address-Spec field is held in the Reply-To header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address object, or NULL if the Address
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Reply-To header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipReplyToHeaderGetAddrSpec
                                            (IN RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * RvSipReplyToHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address-Spec object in the Reply-To header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Reply-To header object.
 *    hAddrSpec - Handle to the Address-Spec object to be set in the 
 *                Reply-To header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetAddrSpec
                                            (IN    RvSipReplyToHeaderHandle  hHeader,
                                             IN    RvSipAddressHandle        hAddrSpec);

/***************************************************************************
 * RvSipReplyToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the ReplyTo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the ReplyTo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetOtherParams(
                                       IN RvSipReplyToHeaderHandle  hHeader,
                                       IN RvChar*                   strBuffer,
                                       IN RvUint                    bufferLen,
                                       OUT RvUint*                  actualLen);

/***************************************************************************
 * RvSipReplyToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the ReplyTo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the ReplyTo header object.
 *    pText      - The other-params string to be set in the ReplyTo header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetOtherParams(
                                     IN RvSipReplyToHeaderHandle    hHeader,
                                     IN RvChar                     *pText);

/***************************************************************************
 * RvSipReplyToHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad ReplyTo header,
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
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderGetStrBadSyntax(
                                     IN  RvSipReplyToHeaderHandle             hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipReplyToHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          ReplyTo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReplyToHeaderSetStrBadSyntax(
                                  IN RvSipReplyToHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax);


#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPREPLYTOHEADER_H*/
