/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPProfileKeyHeader.h                       *
 *                                                                            *
 * The file defines the methods of the PProfileKey header object:             *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Mar.2007                                             *
 ******************************************************************************/
#ifndef RVSIPPPROFILEKEYHEADER_H
#define RVSIPPPROFILEKEYHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipPProfileKeyHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PProfileKey header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_PROFILE_KEY_DISPLAYNAME,
    RVSIP_P_PROFILE_KEY_OTHER_PARAMS,
    RVSIP_P_PROFILE_KEY_BAD_SYNTAX
}RvSipPProfileKeyHeaderStringName;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPProfileKeyHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PProfileKey header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PProfileKey header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderConstructInMsg(
                                       IN  RvSipMsgHandle			hSipMsg,
                                       IN  RvBool					pushHeaderAtHead,
                                       OUT RvSipPProfileKeyHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPProfileKeyHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PProfileKey Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Profile-Key header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderConstruct(
                                           IN  RvSipMsgMgrHandle        hMsgMgr,
                                           IN  HRPOOL                   hPool,
                                           IN  HPAGE                    hPage,
                                           OUT RvSipPProfileKeyHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPProfileKeyHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PProfileKey header object to a destination PProfileKey
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PProfileKey header object.
 *    hSource      - Handle to the destination PProfileKey header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderCopy(
                                         INOUT RvSipPProfileKeyHeaderHandle hDestination,
                                         IN    RvSipPProfileKeyHeaderHandle hSource);


/***************************************************************************
 * RvSipPProfileKeyHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Profile-Key header object to a textual PProfileKey header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Profile-Key header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderEncode(
                                          IN    RvSipPProfileKeyHeaderHandle	hHeader,
                                          IN    HRPOOL                  hPool,
                                          OUT   HPAGE*                  hPage,
                                          OUT   RvUint32*               length);

/***************************************************************************
 * RvSipPProfileKeyHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PProfileKey header-for example,
 *          "PProfileKey:sip:172.20.5.3:5060"-into a P-Profile-Key header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PProfileKey header object.
 *    buffer    - Buffer containing a textual PProfileKey header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderParse(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipPProfileKeyHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PProfileKey header value into an PProfileKey header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPProfileKeyHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PProfileKey header object.
 *    buffer    - The buffer containing a textual PProfileKey header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderParseValue(
                                     IN    RvSipPProfileKeyHeaderHandle	hHeader,
                                     IN    RvChar*					buffer);

/***************************************************************************
 * RvSipPProfileKeyHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PProfileKey header with bad-syntax information.
 *          A SIP header has the following grammar:
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
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderFix(
                                     IN RvSipPProfileKeyHeaderHandle	hHeader,
                                     IN RvChar*                 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPProfileKeyHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PProfileKey header fields are kept in a string format-for example, the
 *          P-Profile-Key header display name. In order to get such a field from the PProfileKey header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PProfileKey header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPProfileKeyHeaderGetStringLength(
                                      IN  RvSipPProfileKeyHeaderHandle     hHeader,
                                      IN  RvSipPProfileKeyHeaderStringName stringName);

/***************************************************************************
 * RvSipPProfileKeyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PProfileKey header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen	 - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetDisplayName(
                                               IN RvSipPProfileKeyHeaderHandle	hHeader,
                                               IN RvChar*               strBuffer,
                                               IN RvUint                bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPProfileKeyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PProfileKey header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the PProfileKey header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetDisplayName(
                                     IN    RvSipPProfileKeyHeaderHandle	hHeader,
                                     IN    RvChar*					strDisplayName);

/***************************************************************************
 * RvSipPProfileKeyHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the P-Profile-Key header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PProfileKey header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPProfileKeyHeaderGetAddrSpec(
                                    IN RvSipPProfileKeyHeaderHandle	hHeader);

/***************************************************************************
 * RvSipPProfileKeyHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PProfileKey header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PProfileKey header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, 
 *				  the existing, Address Spec is removed from the PProfileKey header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetAddrSpec(
                                        IN    RvSipPProfileKeyHeaderHandle hHeader,
                                        IN    RvSipAddressHandle	pAddrSpec);

/***************************************************************************
 * RvSipPProfileKeyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PProfileKey header other params field of the PProfileKey header object into a
 *          given buffer.
 *          Not all the PProfileKey header parameters have separated fields in the PProfileKey
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PProfileKey header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetOtherParams(
                                               IN RvSipPProfileKeyHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPProfileKeyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PProfileKey header object.
 *         Not all the PProfileKey header parameters have separated fields in the PProfileKey
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the PProfileKey header object.
 *    strPProfileKeyParam	- The extended parameters field to be set in the PProfileKey header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetOtherParams(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar*				 strPProfileKeyParam);

/***************************************************************************
 * RvSipPProfileKeyHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad PProfileKey header,
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
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderGetStrBadSyntax(
                                               IN RvSipPProfileKeyHeaderHandle	hHeader,
                                               IN RvChar*				strBuffer,
                                               IN RvUint				bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPProfileKeyHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PProfileKey header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- The handle to the header object.
 *  strBadSyntax	- The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPProfileKeyHeaderSetStrBadSyntax(
                                  IN RvSipPProfileKeyHeaderHandle	 hHeader,
                                  IN RvChar*                 strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPPROFILEKEYHEADER_H */


