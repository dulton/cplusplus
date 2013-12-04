/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPDCSTracePartyIDHeader.h				  *
 *                                                                            *
 * The file defines the methods of the PDCSTracePartyID header object:		  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Dec.2005											  *
 ******************************************************************************/
#ifndef RVSIPPDCSTRACEPARTYIDHEADER_H
#define RVSIPPDCSTRACEPARTYIDHEADER_H

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
 * RvSipPDCSTracePartyIDHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PDCSTracePartyID header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_DCS_TRACE_PARTY_ID_DISPLAYNAME,
    RVSIP_P_DCS_TRACE_PARTY_ID_BAD_SYNTAX
}RvSipPDCSTracePartyIDHeaderStringName;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSTracePartyID header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSTracePartyID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderConstructInMsg(
                               IN  RvSipMsgHandle						hSipMsg,
                               IN  RvBool								pushHeaderAtHead,
                               OUT RvSipPDCSTracePartyIDHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSTracePartyID Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSTracePartyID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderConstruct(
                                   IN  RvSipMsgMgrHandle					hMsgMgr,
                                   IN  HRPOOL								hPool,
                                   IN  HPAGE								hPage,
                                   OUT RvSipPDCSTracePartyIDHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSTracePartyID header object to a destination PDCSTracePartyID
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSTracePartyID header object.
 *    hSource      - Handle to the destination PDCSTracePartyID header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderCopy(
                                 INOUT	RvSipPDCSTracePartyIDHeaderHandle hDestination,
                                 IN		RvSipPDCSTracePartyIDHeaderHandle hSource);


/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSTracePartyID header object to a textual PDCSTracePartyID header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSTracePartyID header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderEncode(
                              IN    RvSipPDCSTracePartyIDHeaderHandle	hHeader,
                              IN    HRPOOL								hPool,
                              OUT   HPAGE*								hPage,
                              OUT   RvUint32*							length);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSTracePartyID header - for example,
 *          "P-DCS-Trace-Party-ID:sip:172.20.5.3:5060" - into a PDCSTracePartyID header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSTracePartyID header object.
 *    buffer    - Buffer containing a textual PDCSTracePartyID header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderParse(
                             IN    RvSipPDCSTracePartyIDHeaderHandle hHeader,
                             IN    RvChar*							 buffer);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSTracePartyID header value into an PDCSTracePartyID header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSTracePartyIDHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSTracePartyID header object.
 *    buffer    - The buffer containing a textual PDCSTracePartyID header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderParseValue(
                             IN    RvSipPDCSTracePartyIDHeaderHandle hHeader,
                             IN    RvChar*							 buffer);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSTracePartyID header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderFix(
                             IN RvSipPDCSTracePartyIDHeaderHandle	hHeader,
                             IN RvChar*								pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSTracePartyID header fields are kept in a string format-for example, the
 *          PDCSTracePartyID header display name. In order to get such a field from the PDCSTracePartyID header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSTracePartyID header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSTracePartyIDHeaderGetStringLength(
                              IN  RvSipPDCSTracePartyIDHeaderHandle     hHeader,
                              IN  RvSipPDCSTracePartyIDHeaderStringName stringName);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PDCSTracePartyID header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderGetDisplayName(
                                   IN RvSipPDCSTracePartyIDHeaderHandle hHeader,
                                   IN RvChar*							strBuffer,
                                   IN RvUint							bufferLen,
                                   OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display name in the PDCSTracePartyID header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		 - Handle to the header object.
 *    strDisplayName - The display name to be set in the PDCSTracePartyID header. If NULL is supplied, the
 *					   existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderSetDisplayName(
                             IN    RvSipPDCSTracePartyIDHeaderHandle hHeader,
                             IN    RvChar*							 strDisplayName);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PDCSTracePartyID header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSTracePartyID header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPDCSTracePartyIDHeaderGetAddrSpec(
                                    IN RvSipPDCSTracePartyIDHeaderHandle hHeader);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PDCSTracePartyID header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PDCSTracePartyID header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the PDCSTracePartyID header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderSetAddrSpec(
                            IN    RvSipPDCSTracePartyIDHeaderHandle hHeader,
                            IN    RvSipAddressHandle				hAddrSpec);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSTracePartyID header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderGetStrBadSyntax(
                               IN RvSipPDCSTracePartyIDHeaderHandle hHeader,
                               IN RvChar*							strBuffer,
                               IN RvUint							bufferLen,
                               OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSTracePartyIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSTracePartyID header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	 - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSTracePartyIDHeaderSetStrBadSyntax(
                          IN RvSipPDCSTracePartyIDHeaderHandle  hHeader,
                          IN RvChar*						 	strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPDCSTRACEPARTYIDHEADER_H */


