/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPUriHeader.h							  *
 *                                                                            *
 * The file defines the methods of the PUri header object:					  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/
#ifndef RVSIPPURIHEADER_H
#define RVSIPPURIHEADER_H

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
 * RvSipPUriHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PUri header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_URI_DISPLAYNAME,
    RVSIP_P_URI_OTHER_PARAMS,
    RVSIP_P_URI_BAD_SYNTAX
}RvSipPUriHeaderStringName;

/*
 * RvSipPUriHeaderType
 * ----------------------
 * This enum defines whether the PUri header represents a P-Associated-Uri,
 * P-Called-Party-ID, P-Assecrted-Identity or a P-Preferred-Identity header.
 */
typedef enum
{
    RVSIP_P_URI_UNDEFINED_HEADER = -1,
    RVSIP_P_URI_ASSOCIATED_URI_HEADER,
	RVSIP_P_URI_CALLED_PARTY_ID_HEADER,
    RVSIP_P_URI_ASSERTED_IDENTITY_HEADER,
    RVSIP_P_URI_PREFERRED_IDENTITY_HEADER
}RvSipPUriHeaderType;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPUriHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PUri header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PUri header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderConstructInMsg(
                                       IN  RvSipMsgHandle			hSipMsg,
                                       IN  RvBool					pushHeaderAtHead,
                                       OUT RvSipPUriHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPUriHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PUri Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Uri header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderConstruct(
                                           IN  RvSipMsgMgrHandle        hMsgMgr,
                                           IN  HRPOOL                   hPool,
                                           IN  HPAGE                    hPage,
                                           OUT RvSipPUriHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPUriHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PUri header object to a destination PUri
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PUri header object.
 *    hSource      - Handle to the destination PUri header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderCopy(
                                         INOUT RvSipPUriHeaderHandle hDestination,
                                         IN    RvSipPUriHeaderHandle hSource);


/***************************************************************************
 * RvSipPUriHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Uri header object to a textual PUri header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Uri header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderEncode(
                                          IN    RvSipPUriHeaderHandle	hHeader,
                                          IN    HRPOOL                  hPool,
                                          OUT   HPAGE*                  hPage,
                                          OUT   RvUint32*               length);

/***************************************************************************
 * RvSipPUriHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PUri header-for example,
 *          "PUri:sip:172.20.5.3:5060"-into a P-Uri header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PUri header object.
 *    buffer    - Buffer containing a textual PUri header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderParse(
                                     IN    RvSipPUriHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipPUriHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PUri header value into an PUri header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPUriHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PUri header object.
 *    buffer    - The buffer containing a textual PUri header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderParseValue(
                                     IN    RvSipPUriHeaderHandle	hHeader,
                                     IN    RvChar*					buffer);

/***************************************************************************
 * RvSipPUriHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PUri header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderFix(
                                     IN RvSipPUriHeaderHandle	hHeader,
                                     IN RvChar*                 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPUriHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PUri header fields are kept in a string format-for example, the
 *          P-Uri header display name. In order to get such a field from the PUri header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PUri header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPUriHeaderGetStringLength(
                                      IN  RvSipPUriHeaderHandle     hHeader,
                                      IN  RvSipPUriHeaderStringName stringName);

/***************************************************************************
 * RvSipPUriHeaderGetPHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the PUri Header object.
 * Return Value: The PUri header type enumeration from the PUri
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PUri header object.
 ***************************************************************************/
RVAPI RvSipPUriHeaderType RVCALLCONV RvSipPUriHeaderGetPHeaderType(
                                   IN RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * RvSipPUriHeaderSetPHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the PUri Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- Handle to the PUri header object.
 *    ePHeaderType	- The header type enumeration to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderSetPHeaderType(
                                 IN    RvSipPUriHeaderHandle hHeader,
                                 IN    RvSipPUriHeaderType   ePHeaderType);

/***************************************************************************
 * RvSipPUriHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PUri header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderGetDisplayName(
                                               IN RvSipPUriHeaderHandle	hHeader,
                                               IN RvChar*               strBuffer,
                                               IN RvUint                bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPUriHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PUri header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the PUri header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderSetDisplayName(
                                     IN    RvSipPUriHeaderHandle	hHeader,
                                     IN    RvChar*					strDisplayName);

/***************************************************************************
 * RvSipPUriHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the P-Uri header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PUri header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPUriHeaderGetAddrSpec(
                                    IN RvSipPUriHeaderHandle	hHeader);

/***************************************************************************
 * RvSipPUriHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PUri header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PUri header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, 
 *				  the existing, Address Spec is removed from the PUri header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderSetAddrSpec(
                                        IN    RvSipPUriHeaderHandle hHeader,
                                        IN    RvSipAddressHandle	pAddrSpec);

/***************************************************************************
 * RvSipPUriHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PUri header other params field of the PUri header object into a
 *          given buffer.
 *          Not all the PUri header parameters have separated fields in the PUri
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PUri header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderGetOtherParams(
                                               IN RvSipPUriHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPUriHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PUri header object.
 *         Not all the PUri header parameters have separated fields in the PUri
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the PUri header object.
 *    strPUriParam	- The extended parameters field to be set in the PUri header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderSetOtherParams(
                                     IN    RvSipPUriHeaderHandle hHeader,
                                     IN    RvChar*				 strPUriParam);

/***************************************************************************
 * RvSipPUriHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PUri header,
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
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderGetStrBadSyntax(
                                               IN RvSipPUriHeaderHandle	hHeader,
                                               IN RvChar*				strBuffer,
                                               IN RvUint				bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPUriHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PUri header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- The handle to the header object.
 *  strBadSyntax	- The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPUriHeaderSetStrBadSyntax(
                                  IN RvSipPUriHeaderHandle	 hHeader,
                                  IN RvChar*                 strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPURIHEADER_H */


