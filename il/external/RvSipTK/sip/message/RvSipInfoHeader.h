/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipInfoHeader.h									  *
 *                                                                            *
 * The file defines the functions of the Info object:                         *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change parameters.                                                         *
 * The Info header object represents Error-Info, Call-Info and Alert-Info SIP *
 * headers                                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza      Aug 2005                                             *
 ******************************************************************************/

#ifndef RVSIPINFOHEADER_H
#define RVSIPINFOHEADER_H

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
 * RvSipInfoHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Info header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_INFO_OTHER_PARAMS,
    RVSIP_INFO_BAD_SYNTAX
}RvSipInfoHeaderStringName;

/*
 * RvSipInfoHeaderType
 * ----------------------
 * This enum defines whether the Info header represents an Alert-Info,
 * a Call-Info or an Error-Info header.
 */
typedef enum
{
    RVSIP_INFO_UNDEFINED_HEADER = -1,
    RVSIP_INFO_ALERT_INFO_HEADER,
    RVSIP_INFO_CALL_INFO_HEADER,
	RVSIP_INFO_ERROR_INFO_HEADER
}RvSipInfoHeaderType;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipInfoHeaderFieldName
 * ----------------------------
 * This enum defines all the Info header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_INFO_FIELD_ADDR_SPEC    = 0,
    RVSIP_INFO_FIELD_OTHER_PARMAS = 1,
	RVSIP_INFO_FIELD_BAD_SYNTAX   = 2
}RvSipInfoHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Info header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 *          Use RvSipInfoHeaderSetHeaderType to determine whether the
 *          object represents an Alert-Info, a Call-Info or an Error-Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Info 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipInfoHeaderHandle               *hHeader);

/***************************************************************************
 * RvSipInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Info Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 *          Use RvSipInfoHeaderSetHeaderType to determine whether the
 *          object represents an Alert-Info, a Call-Info or an Error-Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderConstruct(
                                         IN  RvSipMsgMgrHandle        hMsgMgr,
                                         IN  HRPOOL                   hPool,
                                         IN  HPAGE                    hPage,
                                         OUT RvSipInfoHeaderHandle    *hHeader);

/***************************************************************************
 * RvSipInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Info header object to a destination 
 *          Info header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Info header object.
 *    hSource      - Handle to the source Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderCopy(
                                     IN    RvSipInfoHeaderHandle hDestination,
                                     IN    RvSipInfoHeaderHandle hSource);

/***************************************************************************
 * RvSipInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Info header object to a textual Info header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Info header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderEncode(
                                          IN    RvSipInfoHeaderHandle      hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Info header into a Info header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Info header object.
 *    buffer    - Buffer containing a textual Info header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderParse(
                                     IN RvSipInfoHeaderHandle    hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Info header value into an Info header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Info header object.
 *    buffer    - The buffer containing a textual Info header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderParseValue(
                                     IN RvSipInfoHeaderHandle    hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Info header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderFix(
                                     IN RvSipInfoHeaderHandle     hHeader,
                                     IN RvChar*                   pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipInfoHeaderGetHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Info Header object.
 * Return Value: Returns the Info header type enumeration from the Info
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Info header object.
 ***************************************************************************/
RVAPI RvSipInfoHeaderType  RVCALLCONV RvSipInfoHeaderGetHeaderType(
											IN RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * RvSipInfoHeaderSetHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Info Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hInfoHeader       - Handle to the Info header object.
 *    eHeaderType       - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetHeaderType(
                                 IN    RvSipInfoHeaderHandle hHeader,
                                 IN    RvSipInfoHeaderType   eHeaderType);

/***************************************************************************
 * RvSipInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Info header fields are kept in a string format, for example, the
 *          CSeq other-params. In order to get such a field from the Info header object,
 *          your application should supply an adequate buffer to where the string will be
 *          copied.
 *          This function provides you with the length of the string to enable you to
 *          allocate an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Info header object.
 *    stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipInfoHeaderGetStringLength(
                                      IN  RvSipInfoHeaderHandle     hHeader,
                                      IN  RvSipInfoHeaderStringName stringName);

/***************************************************************************
 * RvSipInfoHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address-Spec field is held in the Info header object as an Address object.
 *          This function returns the handle to the Address object.
 * Return Value: Returns a handle to the Address object, or NULL if the Address
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Info header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipInfoHeaderGetAddrSpec
                                            (IN RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * RvSipInfoHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Addr-Spec address object in the Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Info header object.
 *    hAddrSpec - Handle to the Address-Spec object to be set in the 
 *                Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetAddrSpec
                                            (IN    RvSipInfoHeaderHandle  hHeader,
                                             IN    RvSipAddressHandle     hAddrSpec);

/***************************************************************************
 * RvSipInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the other-params string from the Info header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Info header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderGetOtherParams(
                                       IN RvSipInfoHeaderHandle    hHeader,
                                       IN RvChar*                  strBuffer,
                                       IN RvUint                   bufferLen,
                                       OUT RvUint*                 actualLen);

/***************************************************************************
 * RvSipInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other-params string in the Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Info header object.
 *    pText      - The other-params string to be set in the Info header. 
 *                 If NULL is supplied, the existing other-params is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetOtherParams(
                                     IN RvSipInfoHeaderHandle     hHeader,
                                     IN RvChar                   *pText);

/***************************************************************************
 * RvSipInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Info header,
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
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderGetStrBadSyntax(
                                     IN  RvSipInfoHeaderHandle           hHeader,
                                     IN  RvChar*						 strBuffer,
                                     IN  RvUint							 bufferLen,
                                     OUT RvUint*						 actualLen);

/***************************************************************************
 * RvSipInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Info header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *    strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipInfoHeaderSetStrBadSyntax(
                                  IN RvSipInfoHeaderHandle            hHeader,
                                  IN RvChar*						  strBadSyntax);



#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPINFOHEADER_H*/
