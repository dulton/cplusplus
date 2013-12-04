 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *					RvSipPDCSRedirectHeader.h								  *
 *                                                                            *
 * The file defines the methods of the PDCSRedirect header object:			  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jan.2006                                             *
 ******************************************************************************/
#ifndef RVSIPPDCSREDIRECTHEADER_H
#define RVSIPPDCSREDIRECTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipPDCSRedirectHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PDCSRedirect header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_DCS_REDIRECT_OTHER_PARAMS,
    RVSIP_P_DCS_REDIRECT_BAD_SYNTAX
} RvSipPDCSRedirectHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSRedirectHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSRedirect header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderConstructInMsg(
                                   IN  RvSipMsgHandle					hSipMsg,
                                   IN  RvBool							pushHeaderAtHead,
                                   OUT RvSipPDCSRedirectHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSRedirectHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSRedirect Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-DCS-Redirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderConstruct(
                                   IN  RvSipMsgMgrHandle				hMsgMgr,
                                   IN  HRPOOL							hPool,
                                   IN  HPAGE							hPage,
                                   OUT RvSipPDCSRedirectHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSRedirectHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSRedirect header object to a destination PDCSRedirect
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSRedirect header object.
 *    hSource      - Handle to the destination PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderCopy(
                                 INOUT RvSipPDCSRedirectHeaderHandle hDestination,
                                 IN    RvSipPDCSRedirectHeaderHandle hSource);


/***************************************************************************
 * RvSipPDCSRedirectHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-DCS-Redirect header object to a textual PDCSRedirect header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-DCS-Redirect header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderEncode(
                                  IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                  IN    HRPOOL							hPool,
                                  OUT   HPAGE*							hPage,
                                  OUT   RvUint32*						length);

/***************************************************************************
 * RvSipPDCSRedirectHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSRedirect header - for example,
 *          "P-DCS-Redirect: "sip:bob@radvision.com""
 *			- into a P-DCS-Redirect header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSRedirect header object.
 *    buffer    - Buffer containing a textual PDCSRedirect header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderParse(
                                 IN		RvSipPDCSRedirectHeaderHandle	hHeader,
                                 IN		RvChar*							buffer);

/***************************************************************************
 * RvSipPDCSRedirectHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSRedirect header value into an PDCSRedirect header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSRedirectHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSRedirect header object.
 *    buffer    - The buffer containing a textual PDCSRedirect header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderParseValue(
                             IN    RvSipPDCSRedirectHeaderHandle hHeader,
                             IN    RvChar*							buffer);

/***************************************************************************
 * RvSipPDCSRedirectHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSRedirect header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderFix(
                             IN RvSipPDCSRedirectHeaderHandle	hHeader,
                             IN RvChar*							pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPDCSRedirectHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSRedirect header fields are kept in a string format-for example, the
 *          P-DCS-Redirect header OtherParams. In order to get such a field from the PDCSRedirect header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PDCSRedirect header object.
 *  stringName	- Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSRedirectHeaderGetStringLength(
                              IN  RvSipPDCSRedirectHeaderHandle     hHeader,
                              IN  RvSipPDCSRedirectHeaderStringName stringName);

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PDCSRedirect header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSRedirect header object.
 *	  ePDCSRedirectAddrType - Enumeration for the field from which to get the address.	
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPDCSRedirectHeaderGetAddrSpec(
                        IN RvSipPDCSRedirectHeaderHandle	hHeader,
						IN RvSipPDCSRedirectAddressType		ePDCSRedirectAddrType);

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PDCSRedirect header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PDCSRedirect header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *               Address Spec is removed from the PDCSRedirect header.
 *	  ePDCSRedirectAddrType - Enumeration for the field in which to set the address.	
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetAddrSpec(
                        IN	RvSipPDCSRedirectHeaderHandle	hHeader,
                        IN	RvSipAddressHandle				hAddrSpec,
						IN	RvSipPDCSRedirectAddressType	ePDCSRedirectAddrType);

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetCount
 * ------------------------------------------------------------------------
 * General: Gets the Count parameter from the PDCSRedirect header object.
 * Return Value: Returns the Count number, or UNDEFINED if the Count number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSRedirect header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSRedirectHeaderGetCount(
                                         IN RvSipPDCSRedirectHeaderHandle hHeader);

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetCount
 * ------------------------------------------------------------------------
 * General:  Sets Count parameter of the PDCSRedirect header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PDCSRedirect header object.
 *    count		- The Count number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetCount(
                                         IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                         IN    RvInt32							count);

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSRedirect header other params field of the PDCSRedirect header object into a
 *          given buffer.
 *          Not all the PDCSRedirect header parameters have separated fields in the PDCSRedirect
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSRedirect header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderGetOtherParams(
                               IN RvSipPDCSRedirectHeaderHandle	hHeader,
                               IN RvChar*							strBuffer,
                               IN RvUint							bufferLen,
                               OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSRedirect header object.
 *         Not all the PDCSRedirect header parameters have separated fields in the PDCSRedirect
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSRedirect header object.
 *    OtherParams - The extended parameters field to be set in the PDCSRedirect header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetOtherParams(
                             IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                             IN    RvChar *							strOtherParams);

/***************************************************************************
 * RvSipPDCSRedirectHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSRedirect header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderGetStrBadSyntax(
                               IN RvSipPDCSRedirectHeaderHandle	hHeader,
                               IN RvChar*							strBuffer,
                               IN RvUint							bufferLen,
                               OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSRedirectHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSRedirect header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSRedirectHeaderSetStrBadSyntax(
                          IN RvSipPDCSRedirectHeaderHandle	hHeader,
                          IN RvChar*						strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPDCSREDIRECTHEADER_H */


