/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/*@****************************************************************************
 *                     RvSipContentIDHeader.h                                 *
 *                                                                            *
 * The file defines the methods of the Content-ID header object               *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Mickey Farkash    Jan 2007                                             *
 ****************************************************************************@*/


#ifndef RVSIPCONTENTIDHEADER_H
#define RVSIPCONTENTIDHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipContentIDHeaderStringName
 * ----------------------
 * Defines all Content-ID header object fields that are kept in
 * the object in a string format.
 */
typedef enum
{
    RVSIP_CONTENT_ID_BAD_SYNTAX
} RvSipContentIDHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipContentIDHeaderFieldName
 * ----------------------------
 * This enum defines all the Content-ID header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_CONTENT_ID_FIELD_BAD_SYNTAX     = 0
}RvSipContentIDHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/*@*****************************************************************************
 * RvSipContentIDHeaderConstructInBody
 * ------------------------------------------------------------------------
 * General: Constructs a Content-ID header object inside a given message
 *          body object.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hBody    - The handle to the message body object.
 * Output: phHeader - The handle to the newly constructed Content-ID header
 *                    object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderConstructInBody(
                               IN  RvSipBodyHandle              hBody,
                               OUT RvSipContentIDHeaderHandle  *phHeader);

/*@*****************************************************************************
 * RvSipContentIDHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Content-ID Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - The handle to the message manager.
 *         hPool    - The handle to the memory pool that the object will use.
 *         hPage    - The handle to the memory page that the object will use.
 * output: phHeader - The handle to the newly constructed Content-ID header
 *                   object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderConstruct(
                                    IN  RvSipMsgMgrHandle             hMsgMgr,
                                    IN  HRPOOL                        hPool,
                                    IN  HPAGE                         hPage,
                                    OUT RvSipContentIDHeaderHandle   *phHeader);

/*@*****************************************************************************
 * RvSipContentIDHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Content-ID header object to
 *          a destination Content-ID header object.
 *          You must construct the destination object before using this
 *          function.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - The handle to the destination Content-ID header object.
 *    hSource      - The handle to the source Content-ID header object.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderCopy(
                             INOUT RvSipContentIDHeaderHandle hDestination,
                             IN    RvSipContentIDHeaderHandle hSource);

/*@*****************************************************************************
 * RvSipContentIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Content-ID header object to a textual Content-ID
 *          header. The textual header is placed on a page taken from a
 *          specified pool. In order to copy the textual header from the
 *          page to a consecutive buffer, use RPOOL_CopyToExternal().
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - The handle to the Content-ID header object.
 *        hPool    - The handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderEncode(
                                IN    RvSipContentIDHeaderHandle hHeader,
                                IN    HRPOOL                       hPool,
                                OUT   HPAGE*                       hPage,
                                OUT   RvUint32*                   length);

/*@*****************************************************************************
 * RvSipContentIDHeaderParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Content-ID header - for example:
 *         "Content-ID: multipart/mixed; boundart=unique"
 *         - into a Content-ID header object. All the textual fields are
 *         placed inside the object.
 *          Note: Before performing the parse operation the stack
 *          resets all the header fields. Therefore, if the parse
 *          function fails, you will not be able to get the previous
 *          header field values.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Content-ID header object.
 *    buffer    - Buffer containing a textual Content-ID header.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderParse(
                                 IN    RvSipContentIDHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer);

/*@*****************************************************************************
 * RvSipContentIDHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Content-ID header value into an ContentID
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContentIDHeaderParse() function to parse
 *          strings that also include the header-name.
 *          Note: Before performing the parse operation the stack
 *          resets all the header fields. Therefore, if the parse
 *          function fails, you will not be able to get the previous
 *          header field values.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Content-ID header object.
 *    buffer    - The buffer containing a textual Content-ID header value.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderParseValue(
                                 IN    RvSipContentIDHeaderHandle  hHeader,
                                 IN    RvChar*                      buffer);

/*@*****************************************************************************
 * RvSipContentIDHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Content-ID header with bad-syntax information.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as it was.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderFix(
                                     IN RvSipContentIDHeaderHandle hHeader,
                                     IN RvChar*                     pFixedBuffer);


/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/*@*****************************************************************************
 * RvSipContentIDHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Content-ID header fields are kept in a string format
 *          - for example, the Content-ID header parameter boundary. In order
 *          to get such a field from the Content-ID header object, your
 *          application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader    - The handle to the Content-ID header object.
 *  stringName - Enumeration of the string name for which you require the
 *               length.
 * Return Value: Returns the length of the specified string.
 ****************************************************************************@*/
RVAPI RvUint RVCALLCONV RvSipContentIDHeaderGetStringLength(
                             IN  RvSipContentIDHeaderHandle     hHeader,
                             IN  RvSipContentIDHeaderStringName stringName);

/*@*****************************************************************************
 * RvSipContentIDHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the Content-ID header object as an Address object.
 *          This function returns the handle to the address object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - The handle to the Content-ID header object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 ****************************************************************************@*/
RVAPI RvSipAddressHandle RVCALLCONV RvSipContentIDHeaderGetAddrSpec(
                                IN RvSipContentIDHeaderHandle hHeader);

/*@*****************************************************************************
 * RvSipContentIDHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the Content-ID header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Content-ID header object.
 *    hAddrSpec - The handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the Content-ID header.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderSetAddrSpec(
                                IN    RvSipContentIDHeaderHandle hHeader,
                                IN    RvSipAddressHandle         hAddrSpec);

/*@*****************************************************************************
 * RvSipContentIDHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Content-ID header,
 *          and you wish to see the header-value.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer with which to fill the bad syntax string.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the bad syntax + 1, to include
 *                     a NULL value at the end of the parameter.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderGetStrBadSyntax(
                                    IN RvSipContentIDHeaderHandle   hHeader,
                                    IN RvChar*                       strBuffer,
                                    IN RvUint                        bufferLen,
                                    OUT RvUint*                      actualLen);

/*@*****************************************************************************
 * RvSipContentIDHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Content-ID header.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 * Return Value: Returns RvStatus.
 ****************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipContentIDHeaderSetStrBadSyntax(
                                  IN RvSipContentIDHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax);

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPCONTENTIDHEADER_H */
