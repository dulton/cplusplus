/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPVisitedNetworkIDHeader.h                 *
 *                                                                            *
 * The file defines the methods of the PVisitedNetworkID header object:       *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/
#ifndef RVSIPPVISITEDNETWORKIDHEADER_H
#define RVSIPPVISITEDNETWORKIDHEADER_H

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
 * RvSipPVisitedNetworkIDHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PVisitedNetworkID header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_VISITED_NETWORK_ID_VNETWORK_SPEC,
    RVSIP_P_VISITED_NETWORK_ID_OTHER_PARAMS,
    RVSIP_P_VISITED_NETWORK_ID_BAD_SYNTAX
}RvSipPVisitedNetworkIDHeaderStringName;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PVisitedNetworkID header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PVisitedNetworkID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderConstructInMsg(
                               IN  RvSipMsgHandle						hSipMsg,
                               IN  RvBool								pushHeaderAtHead,
                               OUT RvSipPVisitedNetworkIDHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PVisitedNetworkID Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Visited-Network-ID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderConstruct(
                               IN  RvSipMsgMgrHandle					hMsgMgr,
                               IN  HRPOOL								hPool,
                               IN  HPAGE								hPage,
                               OUT RvSipPVisitedNetworkIDHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PVisitedNetworkID header object to a destination PVisitedNetworkID
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PVisitedNetworkID header object.
 *    hSource      - Handle to the destination PVisitedNetworkID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderCopy(
                                 INOUT RvSipPVisitedNetworkIDHeaderHandle hDestination,
                                 IN    RvSipPVisitedNetworkIDHeaderHandle hSource);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Visited-Network-ID header object to a textual PVisitedNetworkID header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Visited-Network-ID header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderEncode(
                              IN    RvSipPVisitedNetworkIDHeaderHandle	hHeader,
                              IN    HRPOOL								hPool,
                              OUT   HPAGE*								hPage,
                              OUT   RvUint32*							length);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PVisitedNetworkID header - for example,
 *          "P-Visited-Network-ID: other.net" - into a P-Visited-Network-ID header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PVisitedNetworkID header object.
 *    buffer    - Buffer containing a textual PVisitedNetworkID header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderParse(
                                 IN    RvSipPVisitedNetworkIDHeaderHandle	hHeader,
                                 IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PVisitedNetworkID header value into an PVisitedNetworkID header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPVisitedNetworkIDHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PVisitedNetworkID header object.
 *    buffer    - The buffer containing a textual PVisitedNetworkID header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderParseValue(
                             IN    RvSipPVisitedNetworkIDHeaderHandle	hHeader,
                             IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PVisitedNetworkID header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderFix(
                                 IN RvSipPVisitedNetworkIDHeaderHandle	hHeader,
                                 IN RvChar*								pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PVisitedNetworkID header fields are kept in a string format-for example, the
 *          P-Visited-Network-ID header display name. In order to get such a field from the PVisitedNetworkID header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PVisitedNetworkID header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPVisitedNetworkIDHeaderGetStringLength(
                                      IN  RvSipPVisitedNetworkIDHeaderHandle     hHeader,
                                      IN  RvSipPVisitedNetworkIDHeaderStringName stringName);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetVNetworkSpec
 * ------------------------------------------------------------------------
 * General: Copies the VNetworkSpec from the PVisitedNetworkID header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetVNetworkSpec(
                                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderSetVNetworkSpec
 * ------------------------------------------------------------------------
 * General:Sets the VNetworkSpec in the PVisitedNetworkID header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		  - Handle to the header object.
 *    strVNetworkSpec - The VNetworkSpec to be set in the PVisitedNetworkID header. If NULL is supplied, the
 *						existing VNetworkSpec is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetVNetworkSpec(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar*                 strVNetworkSpec);



/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PVisitedNetworkID header other params field of the PVisitedNetworkID header object into a
 *          given buffer.
 *          Not all the PVisitedNetworkID header parameters have separated fields in the PVisitedNetworkID
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PVisitedNetworkID header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetOtherParams(
                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                               IN RvChar*						     strBuffer,
                               IN RvUint							 bufferLen,
                              OUT RvUint*							 actualLen);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PVisitedNetworkID header object.
 *         Not all the PVisitedNetworkID header parameters have separated fields in the PVisitedNetworkID
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader					- Handle to the PVisitedNetworkID header object.
 *    strPVisitedNetworkIDParam - The extended parameters field to be set in the PVisitedNetworkID header. If NULL is
 *								  supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetOtherParams(
                         IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                         IN    RvChar *							  strPVisitedNetworkIDParam);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PVisitedNetworkID header,
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
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderGetStrBadSyntax(
                               IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                               IN RvChar*							 strBuffer,
                               IN RvUint							 bufferLen,
                              OUT RvUint*							 actualLen);

/***************************************************************************
 * RvSipPVisitedNetworkIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PVisitedNetworkID header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- The handle to the header object.
 *  strBadSyntax	- The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPVisitedNetworkIDHeaderSetStrBadSyntax(
                              IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                              IN RvChar*							strBadSyntax);


#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPVISITEDNETWORKIDHEADER_H */


