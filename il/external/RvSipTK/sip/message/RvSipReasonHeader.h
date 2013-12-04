/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipReasonHeader.h									  *
 *                                                                            *
 * The file defines the methods of the Reason header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Oren Libis         Jan. 2001                                            *
 ******************************************************************************/

#ifndef RVSIPREASONHEADER_H
#define RVSIPREASONHEADER_H

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
 * RvSipReasonHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Reason header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_REASON_PROTOCOL,
	RVSIP_REASON_TEXT,
	RVSIP_REASON_OTHER_PARAMS,
    RVSIP_REASON_BAD_SYNTAX
}RvSipReasonHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipReasonHeaderFieldName
 * ----------------------------
 * This enum defines all the Reason header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_REASON_FIELD_TEXT           = 0,
	RVSIP_REASON_FIELD_CAUSE          = 1,
	RVSIP_REASON_FIELD_PROTOCOL       = 2,
	RVSIP_REASON_FIELD_OTHER_PARAMS   = 3,
	RVSIP_REASON_FIELD_BAD_SYNTAX     = 4
}RvSipReasonHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReasonHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Reason header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Reason
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipReasonHeaderHandle             *hHeader);

/***************************************************************************
 * RvSipReasonHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Reason Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipReasonHeaderHandle *hHeader);

/***************************************************************************
 * RvSipReasonHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Reason header object to a destination
 *          Reason header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Reason header object.
 *    hSource      - Handle to the source Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderCopy(
                                     IN    RvSipReasonHeaderHandle hDestination,
                                     IN    RvSipReasonHeaderHandle hSource);

/***************************************************************************
 * RvSipReasonHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Reason header object to a textual Reason header. The
 *          textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a consecutive
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Reason header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderEncode(
                                          IN    RvSipReasonHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipReasonHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Reason header into a Reason header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Reason header object.
 *    buffer    - Buffer containing a textual Reason header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderParse(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipReasonHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Reason header value into an Reason header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipReasonHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Reason header object.
 *    buffer    - The buffer containing a textual Reason header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderParseValue(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipReasonHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Reason header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderFix(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);

/***************************************************************************
 * RvSipReasonHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Reason header fields are kept in a string format, for
 *          example, the Reason header next-nonce string. In order to get such a field
 *          from the Reason header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Reason header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipReasonHeaderGetStringLength(
                                      IN  RvSipReasonHeaderHandle     hHeader,
                                      IN  RvSipReasonHeaderStringName eStringName);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipReasonHeaderGetText
 * ------------------------------------------------------------------------
 * General: Copies the reason-text string from the Reason header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetText(
                                       IN RvSipReasonHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipReasonHeaderSetText
 * ------------------------------------------------------------------------
 * General:Sets the reason-text string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Reason header object.
 *    pText      - The reason-text string to be set in the Reason header.
 *                 If NULL is supplied, the existing reason-text is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetText(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipReasonHeaderGetCause
 * ------------------------------------------------------------------------
 * General: Gets the protocol-cause value from the Reason Header object.
 * Return Value: Returns the protocol-cause value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Reason header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipReasonHeaderGetCause(
                                    IN RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * RvSipReasonHeaderSetCause
 * ------------------------------------------------------------------------
 * General: Sets the protocol-cause value in the Reason Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Reason header object.
 *    cause             - The protocol-cause value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetCause(
                                 IN    RvSipReasonHeaderHandle hHeader,
                                 IN    RvInt32                 cause);

/***************************************************************************
 * RvSipReasonHeaderGetProtocol
 * ------------------------------------------------------------------------
 * General: Gets the protocol enumeration from the Reason object.
 * Return Value: The protocol from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Returns the protocol enumeration from the object.
 ***************************************************************************/
RVAPI RvSipReasonProtocolType  RVCALLCONV RvSipReasonHeaderGetProtocol(
                                      IN  RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * RvSipReasonHeaderGetStrProtocol
 * ------------------------------------------------------------------------
 * General: Copies the protocol string value of the Reason object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a
 *                     NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetStrProtocol(
                                       IN RvSipReasonHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipReasonHeaderSetProtocol
 * ------------------------------------------------------------------------
 * General:Sets the protocol string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Reason header object.
 *    eQop     - The protocol enumeration value to be set in the object.
 *    strQop   - You can use this parametere only if the eQop parameter is set to
 *               RVSIP_REASON_PROTOCOL_OTHER. In this case you can supply the
 *               protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetProtocol(
                                     IN RvSipReasonHeaderHandle  hHeader,
                                     IN RvSipReasonProtocolType  eProtocol,
									 IN RvChar *                 strProtocol);

/***************************************************************************
 * RvSipReasonHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Reason header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Reason header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetOtherParams(
                                       IN RvSipReasonHeaderHandle  hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipReasonHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Reason header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Reason header object.
 *    pText      - The other-params string to be set in the Reason header.
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetOtherParams(
                                     IN RvSipReasonHeaderHandle   hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipReasonHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Reason header,
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
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetStrBadSyntax(
                                     IN  RvSipReasonHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipReasonHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Reason header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetStrBadSyntax(
                                  IN RvSipReasonHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax);

/***************************************************************************
 * RvSipReasonHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Reason header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthInfoHeader - Handle to the Reason header object.
 *           eStringName    - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Reason header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderGetRpoolString(
                             IN    RvSipReasonHeaderHandle      hSipAuthInfoHeader,
                             IN    RvSipReasonHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr);

/***************************************************************************
 * RvSipReasonHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Reason header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Reason header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipReasonHeaderSetRpoolString(
                             IN RvSipReasonHeaderHandle       hHeader,
                             IN RvSipReasonHeaderStringName   eStringName,
                             IN RPOOL_Ptr                    *pRpoolPtr);


#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPREASONHEADER_H*/
